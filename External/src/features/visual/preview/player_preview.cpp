#define NOMINMAX
#include "player_preview.h"
#include "../players/avatars.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/stb/stb_image.h"
#include "../../../menu/core/gui/menu.h"
#include "../../../menu/core/overlay/overlay.h"
#include "../box/box.h"
#include "../health/health.h"
#include "../../../menu/overlay/settings.h"
#include "../../../menu/resources/font.h"
#include "../../../utility/cache/cache.h"
#include <windows.h>
#include <wininet.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <cctype>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#pragma comment(lib, "wininet.lib")
namespace thenwefuckin::visual::preview
{
    namespace
    {
        constexpr const char* kBackendHost = "rblxapi.vercel.app";
        constexpr const char* kBackendPassword = "a6707d0df1f04daa0f036a1d";
        constexpr const char* kBackendKey = "cs_JvDcYaEzH2RXAzTQ851hh605IXDne45Z1K-Mi4abh3U";
        constexpr DWORD kNetDialMs = 10000;
        constexpr DWORD kNetIoMs = 30000;
        constexpr int kFetchRounds = 30;
        constexpr UINT kRtSize = 512;
        constexpr size_t kMaxTextureBytes = 8ull << 20;
        constexpr size_t kMaxVerts = 200000;
        struct Jdom
        {
            enum class Type { Nul, Bool, Num, Str, Arr, Obj };
            Type type = Type::Nul;
            bool boolean = false;
            double num = 0.0;
            std::string str;
            std::vector<Jdom> arr;
            std::map<std::string, Jdom> obj;
            bool is_obj() const { return type == Type::Obj; }
            bool is_arr() const { return type == Type::Arr; }
            bool is_str() const { return type == Type::Str; }
            const Jdom* find(const std::string& k) const
            {
                if (type != Type::Obj) return nullptr;
                auto it = obj.find(k);
                return it != obj.end() ? &it->second : nullptr;
            }
            std::string str_or(const std::string& k, const std::string& dflt = "") const
            {
                const Jdom* v = find(k);
                return (v && v->is_str()) ? v->str : dflt;
            }
            double num_or(const std::string& k, double dflt = 0.0) const
            {
                const Jdom* v = find(k);
                if (!v) return dflt;
                if (v->type == Type::Num) return v->num;
                return dflt;
            }
        };
        struct JParser
        {
            const char* p;
            const char* end;
            bool fail = false;
            JParser(const std::string& s) : p(s.data()), end(s.data() + s.size()) {}
            void ws()
            {
                while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
            }
            bool lit(const char* s)
            {
                size_t n = strlen(s);
                if ((size_t)(end - p) < n || memcmp(p, s, n) != 0) { fail = true; return false; }
                p += n;
                return true;
            }
            std::string string()
            {
                std::string out;
                if (p >= end || *p != '"') { fail = true; return out; }
                ++p;
                while (p < end && *p != '"')
                {
                    if (*p == '\\' && p + 1 < end)
                    {
                        ++p;
                        switch (*p)
                        {
                        case '"': out += '"'; break;
                        case '\\': out += '\\'; break;
                        case '/': out += '/'; break;
                        case 'b': out += '\b'; break;
                        case 'f': out += '\f'; break;
                        case 'n': out += '\n'; break;
                        case 'r': out += '\r'; break;
                        case 't': out += '\t'; break;
                        case 'u':
                            out += '?';
                            if ((end - p) >= 5) p += 4;
                            break;
                        default: out += *p; break;
                        }
                        ++p;
                    }
                    else
                    {
                        out += *p++;
                    }
                }
                if (p >= end) { fail = true; return out; }
                ++p;
                return out;
            }
            double number()
            {
                char* e = nullptr;
                double v = strtod(p, &e);
                if (e == p) { fail = true; return 0.0; }
                p = e;
                return v;
            }
            Jdom value()
            {
                Jdom v;
                ws();
                if (p >= end) { fail = true; return v; }
                if (*p == '{')
                {
                    ++p;
                    v.type = Jdom::Type::Obj;
                    ws();
                    if (p < end && *p == '}') { ++p; return v; }
                    while (true)
                    {
                        ws();
                        std::string k = string();
                        if (fail) return v;
                        ws();
                        if (p >= end || *p != ':') { fail = true; return v; }
                        ++p;
                        v.obj[std::move(k)] = value();
                        if (fail) return v;
                        ws();
                        if (p >= end) { fail = true; return v; }
                        if (*p == ',') { ++p; continue; }
                        if (*p == '}') { ++p; break; }
                        fail = true;
                        return v;
                    }
                    return v;
                }
                if (*p == '[')
                {
                    ++p;
                    v.type = Jdom::Type::Arr;
                    ws();
                    if (p < end && *p == ']') { ++p; return v; }
                    while (true)
                    {
                        v.arr.push_back(value());
                        if (fail) return v;
                        ws();
                        if (p >= end) { fail = true; return v; }
                        if (*p == ',') { ++p; continue; }
                        if (*p == ']') { ++p; break; }
                        fail = true;
                        return v;
                    }
                    return v;
                }
                if (*p == '"') { v.type = Jdom::Type::Str; v.str = string(); return v; }
                if (*p == 't') { if (lit("true")) v.type = Jdom::Type::Bool, v.boolean = true; return v; }
                if (*p == 'f') { if (lit("false")) v.type = Jdom::Type::Bool; return v; }
                if (*p == 'n') { if (lit("null")) v.type = Jdom::Type::Nul; return v; }
                v.type = Jdom::Type::Num;
                v.num = number();
                return v;
            }
        };
        struct Http
        {
            static void arm(HINTERNET s)
            {
                DWORD c = kNetDialMs, io = kNetIoMs;
                InternetSetOptionA(s, INTERNET_OPTION_CONNECT_TIMEOUT, &c, sizeof(c));
                InternetSetOptionA(s, INTERNET_OPTION_SEND_TIMEOUT, &io, sizeof(io));
                InternetSetOptionA(s, INTERNET_OPTION_RECEIVE_TIMEOUT, &io, sizeof(io));
            }
            static bool login(std::string& jar)
            {
                jar.clear();
                HINTERNET s = InternetOpenA("AvatarViewer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
                if (!s) return false;
                arm(s);
                bool good = false;
                if (HINTERNET c = InternetConnectA(s, kBackendHost, INTERNET_DEFAULT_HTTPS_PORT,
                        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0))
                {
                    if (HINTERNET q = HttpOpenRequestA(c, "POST", "/api/auth/login", NULL, NULL, NULL,
                            INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0))
                    {
                        std::string payload = std::string("{\"password\":\"") + kBackendPassword + "\"}";
                        const char* hdrs = "Content-Type: application/json\r\n";
                        if (HttpSendRequestA(q, hdrs, (DWORD)strlen(hdrs), (LPVOID)payload.c_str(), (DWORD)payload.size()))
                        {
                            char raw[8192]; DWORD len = sizeof(raw), at = 0;
                            if (HttpQueryInfoA(q, HTTP_QUERY_SET_COOKIE, raw, &len, &at))
                            {
                                jar.assign(raw, len);
                                auto cut = jar.find(';');
                                if (cut != std::string::npos) jar.erase(cut);
                                good = !jar.empty();
                            }
                        }
                        InternetCloseHandle(q);
                    }
                    InternetCloseHandle(c);
                }
                InternetCloseHandle(s);
                return good;
            }
            struct Reply { DWORD code = 0; std::string body; };
            static Reply fetch_on(HINTERNET s, const std::string& url, const std::string& jar)
            {
                Reply out;
                std::string hdrs;
                if (url.find(kBackendHost) != std::string::npos)
                {
                    hdrs += std::string("x-api-key: ") + kBackendKey + "\r\n";
                    if (!jar.empty()) hdrs += "Cookie: " + jar + "\r\n";
                }
                hdrs += "Accept-Encoding: identity\r\n";
                auto open = [&](DWORD flags) {
                    return InternetOpenUrlA(s, url.c_str(),
                        hdrs.empty() ? NULL : hdrs.c_str(),
                        hdrs.empty() ? 0 : (DWORD)hdrs.size(), flags, 0);
                };
                HINTERNET h = open(INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE |
                    INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID);
                if (!h) h = open(INTERNET_FLAG_RELOAD);
                if (h)
                {
                    DWORD code = 0, clen = sizeof(code), ix = 0;
                    if (HttpQueryInfoA(h, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &code, &clen, &ix))
                        out.code = code;
                    char chunk[32768]; DWORD got = 0;
                    while (InternetReadFile(h, chunk, sizeof(chunk), &got) && got > 0)
                    {
                        if (out.body.size() + got > (kMaxTextureBytes + 65536))
                            break;
                        out.body.append(chunk, got);
                    }
                    InternetCloseHandle(h);
                }
                return out;
            }
            static Reply fetch(const std::string& url, const std::string& jar)
            {
                Reply out;
                HINTERNET s = InternetOpenA("AvatarViewer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
                if (!s) return out;
                arm(s);
                out = fetch_on(s, url, jar);
                InternetCloseHandle(s);
                return out;
            }
        };
        struct FaceRef { int p = 0; int t = 0; int n = 0; };
        enum class Phase { Calm, Fetch, Wait, Pull, Bake, Paint, Done, Broke };
        std::string rewrite_mtl(const std::string& src, const std::vector<std::pair<std::string, std::string>>& texs)
        {
            std::unordered_map<std::string, std::string> link;
            for (auto& t : texs)
                if (!link.count(t.first)) link[t.first] = t.second;
            std::istringstream in(src);
            std::ostringstream out;
            std::string ln;
            while (std::getline(in, ln))
            {
                if (!ln.empty() && ln.back() == '\r') ln.pop_back();
                if (ln.rfind("map_d ", 0) == 0) continue;
                std::string head, tail;
                if (ln.rfind("map_Kd ", 0) == 0) { head = "map_Kd "; tail = ln.substr(7); }
                else if (ln.rfind("map_Ka ", 0) == 0) { head = "map_Ka "; tail = ln.substr(7); }
                else if (ln.rfind("map_Ks ", 0) == 0) { head = "map_Ks "; tail = ln.substr(7); }
                if (!head.empty())
                {
                    auto a = tail.find_first_not_of(" \t"), b = tail.find_last_not_of(" \t");
                    if (a != std::string::npos)
                    {
                        std::string id = tail.substr(a, b - a + 1);
                        auto it = link.find(id);
                        if (it != link.end()) ln = head + it->second;
                    }
                }
                out << ln << '\n';
            }
            return out.str();
        }
        std::map<std::string, Surface> read_mtl(const std::string& src)
        {
            std::map<std::string, Surface> out;
            std::istringstream in(src);
            std::string ln, active;
            while (std::getline(in, ln))
            {
                if (!ln.empty() && ln.back() == '\r') ln.pop_back();
                std::istringstream ls(ln);
                std::string op;
                if (!(ls >> op)) continue;
                if (op == "newmtl") { ls >> active; out[active] = Surface{ active }; }
                else if (op == "Kd" && !active.empty()) ls >> out[active].base.x >> out[active].base.y >> out[active].base.z;
                else if (op == "map_Kd" && !active.empty()) ls >> out[active].tex;
            }
            return out;
        }
        inline int parse_idx_safe(const char* b, const char* e) noexcept
        {
            if (!b || b >= e) return 0;
            bool neg = false;
            if (*b == '+' || *b == '-') { neg = (*b == '-'); ++b; }
            if (b >= e) return 0;
            long v = 0;
            bool any = false;
            while (b < e && *b >= '0' && *b <= '9') { any = true; v = v * 10 + (*b - '0'); ++b; if (v > 2000000000L) break; }
            if (!any) return 0;
            return neg ? (int)-v : (int)v;
        }
        inline bool try_gunzip_inplace(std::string& data) noexcept
        {
            size_t n = data.size();
            if (n < 18) return false;
            const unsigned char* b = (const unsigned char*)data.data();
            if (b[0] != 0x1F || b[1] != 0x8B || b[2] != 0x08) return false;
            unsigned flags = b[3];
            size_t pos = 10;
            if (flags & 0x04)
            {
                if (pos + 2 > n) return false;
                unsigned xlen = (unsigned)b[pos] | ((unsigned)b[pos + 1] << 8);
                pos += 2 + xlen;
            }
            if (flags & 0x08)
            {
                while (pos < n && b[pos] != 0) ++pos;
                ++pos;
            }
            if (flags & 0x10)
            {
                while (pos < n && b[pos] != 0) ++pos;
                ++pos;
            }
            if (flags & 0x02) pos += 2;
            if (pos + 8 >= n) return false;
            size_t comp_len = n - pos - 8;
            if (comp_len == 0 || comp_len > (16ull << 20)) return false;
            int outlen = 0;
            char* out = nullptr;
            try { out = stbi_zlib_decode_noheader_malloc((const char*)(b + pos), (int)comp_len, &outlen); }
            catch (...) { out = nullptr; }
            if (!out || outlen <= 0 || outlen > (64 << 20)) { if (out) free(out); return false; }
            try { data.assign(out, out + outlen); }
            catch (...) { free(out); return false; }
            free(out);
            return true;
        }
        inline std::string describe_head(const std::string& body, size_t maxChars = 72)
        {
            size_t i = 0;
            while (i < body.size())
            {
                unsigned char c = (unsigned char)body[i];
                if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 0xEF || c == 0xBB || c == 0xBF || c == 0) { ++i; continue; }
                break;
            }
            if (i >= body.size()) return "empty";
            const unsigned char* b = (const unsigned char*)body.data();
            if (body.size() - i >= 2 && b[i] == 0x1F && b[i + 1] == 0x8B) return "gzip-compressed";
            if (body.size() - i >= 4 && b[i] == 'g' && b[i + 1] == 'l' && b[i + 2] == 'T' && b[i + 3] == 'F') return "glTF-binary";
            char c0 = body[i];
            if (c0 == '<') return "html/xml-error";
            if (c0 == '{' || c0 == '[') return "json-error";
            std::string head;
            for (size_t k = i; k < body.size() && head.size() < maxChars; ++k)
            {
                char c = body[k];
                if (c == '\r' || c == '\n') break;
                if (c < 32 || c > 126) c = '?';
                head += c;
            }
            if (head.empty()) return "binary";
            return "'" + head + "'";
        }
        FaceRef split_face(const std::string& tok) noexcept
        {
            FaceRef f;
            const char* s = tok.c_str();
            const char* e = s + tok.size();
            const char* s1 = nullptr;
            for (const char* p = s; p < e; ++p) if (*p == '/') { s1 = p; break; }
            if (!s1) { f.p = parse_idx_safe(s, e); return f; }
            f.p = parse_idx_safe(s, s1);
            const char* s2 = nullptr;
            for (const char* p = s1 + 1; p < e; ++p) if (*p == '/') { s2 = p; break; }
            if (!s2) { f.t = parse_idx_safe(s1 + 1, e); return f; }
            if (s2 > s1 + 1) f.t = parse_idx_safe(s1 + 1, s2);
            if (s2 + 1 < e) f.n = parse_idx_safe(s2 + 1, e);
            return f;
        }
        std::vector<BakedMesh> read_obj(const std::string& srcIn, size_t& vCount, size_t& fCount, size_t* outLines = nullptr, std::string* outFirstOp = nullptr)
        {
            vCount = 0;
            fCount = 0;
            if (outLines) *outLines = 0;
            if (outFirstOp) outFirstOp->clear();
            std::vector<V3> P, N;
            std::vector<V2> T;
            P.reserve(20000);
            std::map<std::string, BakedMesh> groups;
            std::map<std::string, std::map<std::string, uint32_t>> seen;
            std::string tag = "Default";
            std::string src;
            src.reserve(srcIn.size());
            size_t off = 0;
            if (srcIn.size() >= 3 &&
                (unsigned char)srcIn[0] == 0xEF && (unsigned char)srcIn[1] == 0xBB && (unsigned char)srcIn[2] == 0xBF)
                off = 3;
            size_t nulCount = 0;
            for (size_t i = off; i < srcIn.size(); ++i) if (srcIn[i] == '\0') ++nulCount;
            bool utf16ish = nulCount > srcIn.size() / 10 && srcIn.size() > 64;
            for (size_t i = off; i < srcIn.size(); ++i)
            {
                char c = srcIn[i];
                if (c == '\0') continue;
                if (c == '\r') c = '\n';
                src.push_back(c);
            }
            (void)utf16ish;
            std::istringstream in(src);
            std::string ln;
            size_t lines = 0;
            while (std::getline(in, ln))
            {
                ++lines;
                if (ln.size() > 1024) ln.resize(1024);
                std::istringstream ls(ln);
                std::string op;
                if (!(ls >> op)) continue;
                if (op.empty()) continue;
                if (op[0] == '#') continue;
                if (outFirstOp && outFirstOp->empty() && op.size() <= 16) *outFirstOp = op;
                if (op == "v")
                {
                    float x = 0, y = 0, z = 0;
                    if (ls >> x >> y >> z)
                    {
                        if (std::isfinite(x) && std::isfinite(y) && std::isfinite(z))
                        {
                            P.push_back(V3{ x, y, z });
                            ++vCount;
                        }
                    }
                }
                else if (op == "vt") { float x = 0, y = 0; if (ls >> x >> y) T.push_back(V2{ x, y }); }
                else if (op == "vn") { float x = 0, y = 0, z = 0; if (ls >> x >> y >> z) N.push_back(V3{ x, y, z }); }
                else if (op == "usemtl") { std::string nt; if (ls >> nt && !nt.empty() && nt.size() <= 128) tag = nt; }
                else if (op == "o" || op == "g") { std::string nt; if (ls >> nt && !nt.empty() && nt.size() <= 128) tag = nt; }
                else if (op == "f")
                {
                    std::vector<FaceRef> corners;
                    corners.reserve(4);
                    std::string tok;
                    while (ls >> tok)
                    {
                        if (tok.size() > 64) continue;
                        FaceRef fr = split_face(tok);
                        if (fr.p == 0) continue;
                        corners.push_back(fr);
                        if (corners.size() >= 16) break;
                    }
                    if (corners.size() < 3) continue;
                    ++fCount;
                    auto git = groups.find(tag);
                    if (git == groups.end()) git = groups.emplace(tag, BakedMesh{ tag }).first;
                    BakedMesh& g = git->second;
                    if (g.verts.size() > kMaxVerts) continue;
                    auto& memo = seen[tag];
                    auto bake = [&](const FaceRef& c) -> uint32_t {
                        std::string k = std::to_string(c.p) + "_" + std::to_string(c.t) + "_" + std::to_string(c.n);
                        auto hit = memo.find(k);
                        if (hit != memo.end()) return hit->second;
                        auto fix = [](int ix, size_t n) -> int {
                            if (ix > 0) return ix;
                            if (ix < 0 && (size_t)(-ix) <= n) return (int)n + ix + 1;
                            return 0;
                        };
                        int pi = fix(c.p, P.size());
                        int ti = fix(c.t, T.size());
                        int ni = fix(c.n, N.size());
                        GpuVertex v{};
                        if (pi > 0 && pi <= (int)P.size()) v.p = P[pi - 1];
                        if (ti > 0 && ti <= (int)T.size()) v.t = T[ti - 1];
                        if (ni > 0 && ni <= (int)N.size()) v.n = N[ni - 1];
                        uint32_t ix = (uint32_t)g.verts.size();
                        g.verts.push_back(v);
                        memo[k] = ix;
                        return ix;
                    };
                    uint32_t first = bake(corners[0]);
                    for (size_t i = 1; i + 1 < corners.size(); ++i)
                    {
                        g.idx.push_back(first);
                        g.idx.push_back(bake(corners[i]));
                        g.idx.push_back(bake(corners[i + 1]));
                    }
                }
            }
            if (outLines) *outLines = lines;
            std::vector<BakedMesh> out;
            for (auto& kv : groups)
                if (!kv.second.verts.empty()) out.push_back(std::move(kv.second));
            return out;
        }
        DecodedTex decode_bytes(const std::string& bytes)
        {
            DecodedTex t;
            if (bytes.empty() || bytes.size() > kMaxTextureBytes) return t;
            int w = 0, h = 0, ch = 0;
            unsigned char* px = stbi_load_from_memory((const stbi_uc*)bytes.data(), (int)bytes.size(), &w, &h, &ch, 4);
            if (!px) return t;
            if (w > 0 && h > 0 && (size_t)w * (size_t)h <= (16ull << 20))
            {
                t.w = w;
                t.h = h;
                t.px.assign(px, px + (size_t)w * (size_t)h * 4);
            }
            stbi_image_free(px);
            return t;
        }
        ID3D11ShaderResourceView* make_texture(ID3D11Device* dev, const DecodedTex& t)
        {
            if (t.px.empty() || t.w <= 0 || t.h <= 0) return nullptr;
            if ((size_t)t.w * (size_t)t.h * 4 != t.px.size()) return nullptr;
            D3D11_TEXTURE2D_DESC d{};
            d.Width = t.w; d.Height = t.h; d.MipLevels = 1; d.ArraySize = 1;
            d.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            d.SampleDesc.Count = 1;
            d.Usage = D3D11_USAGE_DEFAULT;
            d.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            D3D11_SUBRESOURCE_DATA init{};
            init.pSysMem = t.px.data();
            init.SysMemPitch = t.w * 4;
            ID3D11Texture2D* tex = nullptr;
            HRESULT hr = dev->CreateTexture2D(&d, &init, &tex);
            if (FAILED(hr) || !tex) return nullptr;
            ID3D11ShaderResourceView* srv = nullptr;
            hr = dev->CreateShaderResourceView(tex, nullptr, &srv);
            tex->Release();
            return srv;
        }
        static const char* kShaders = R"(
cbuffer Frame : register(b0)
{
    matrix W;
    matrix V;
    matrix P;
    float4 base;
    float mapped;
    float3 unused;
};
struct Vin { float3 p : POSITION; float3 n : NORMAL; float2 u : TEXCOORD; };
struct Pin { float4 p : SV_POSITION; float3 n : NORMAL; float2 u : TEXCOORD; };
Texture2D skin : register(t0);
SamplerState smp : register(s0);
Pin VS(Vin i)
{
    Pin o;
    float4 wp = mul(float4(i.p, 1.0f), W);
    wp = mul(wp, V);
    o.p = mul(wp, P);
    o.n = normalize(mul(i.n, (float3x3)W));
    o.u = float2(i.u.x, 1.0f - i.u.y);
    return o;
}
float4 PS(Pin i) : SV_Target
{
    float glow = max(0.35f, dot(i.n, normalize(float3(1.0f, 1.0f, -1.0f))));
    float4 col = base;
    if (mapped > 0.5f) col = skin.Sample(smp, i.u);
    return float4(col.rgb * glow, col.a);
}
)";
        struct State
        {
            std::string jar;
            std::atomic<Phase> phase{ Phase::Calm };
            std::string note = "Waiting for game...";
            std::mutex gate;
            std::vector<BakedMesh> pendingMeshes;
            std::map<std::string, Surface> pendingSurfaces;
            std::map<std::string, DecodedTex> pendingImages;
            std::atomic<bool> pending{ false };
            DecodedTex pendingThumb;
            std::atomic<bool> thumbPending{ false };
            ID3D11ShaderResourceView* thumbSrv = nullptr;
            int thumbW = 0, thumbH = 0;
            std::vector<LiveMesh> meshes;
            std::map<std::string, ID3D11ShaderResourceView*> images;
            std::map<std::string, Surface> surfaces;
            std::map<std::string, V3> partCenters;
            ID3D11VertexShader* vs = nullptr;
            ID3D11PixelShader* ps = nullptr;
            ID3D11InputLayout* layout = nullptr;
            ID3D11Buffer* ubo = nullptr;
            ID3D11SamplerState* sampler = nullptr;
            ID3D11RasterizerState* raster = nullptr;
            ID3D11DepthStencilState* depth = nullptr;
            ID3D11Texture2D* rt = nullptr;
            ID3D11RenderTargetView* rtv = nullptr;
            ID3D11ShaderResourceView* srv = nullptr;
            ID3D11Texture2D* zb = nullptr;
            ID3D11DepthStencilView* dsv = nullptr;
            UINT rtW = 0, rtH = 0;
            bool gfxReady = false;
            bool gfxFailed = false;
            int gfxRetryCd = 0;
            bool needFit = true;
            float fittedW = 0.0f, fittedH = 0.0f;
            float dist = 15.0f, yaw = 0.0f, pitch = 1.5f, fov = 70.0f;
            float homeYaw = 0.0f, homePitch = 1.5f;
            V3 focus{ 0, 0, 0 };
            V3 bbMin{ -5, -5, -5 }, bbMax{ 5, 5, 5 };
            bool spin = false;
            double lastTouch = -10.0;
            std::atomic<bool> busy{ false };
            int64_t pulledUid = 0;
            double lastTry = -100.0;
        };
        inline State s;
        constexpr int64_t kPreviewUid = 7846506923LL;
        bool compile_shaders(ID3D11Device* dev)
        {
            ID3DBlob* vsb = nullptr;
            ID3DBlob* psb = nullptr;
            HRESULT hr = D3DCompile(kShaders, strlen(kShaders), nullptr, nullptr, nullptr,
                "VS", "vs_4_0", 0, 0, &vsb, nullptr);
            if (FAILED(hr)) return false;
            hr = D3DCompile(kShaders, strlen(kShaders), nullptr, nullptr, nullptr,
                "PS", "ps_4_0", 0, 0, &psb, nullptr);
            if (FAILED(hr)) { vsb->Release(); return false; }
            hr = dev->CreateVertexShader(vsb->GetBufferPointer(), vsb->GetBufferSize(), nullptr, &s.vs);
            if (SUCCEEDED(hr))
                hr = dev->CreatePixelShader(psb->GetBufferPointer(), psb->GetBufferSize(), nullptr, &s.ps);
            D3D11_INPUT_ELEMENT_DESC fmt[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            };
            if (SUCCEEDED(hr))
                hr = dev->CreateInputLayout(fmt, 3, vsb->GetBufferPointer(), vsb->GetBufferSize(), &s.layout);
            vsb->Release();
            psb->Release();
            if (FAILED(hr)) return false;
            D3D11_BUFFER_DESC ub{};
            ub.ByteWidth = sizeof(FrameConsts);
            ub.Usage = D3D11_USAGE_DYNAMIC;
            ub.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            ub.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
            if (FAILED(dev->CreateBuffer(&ub, nullptr, &s.ubo))) return false;
            D3D11_SAMPLER_DESC sp{};
            sp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
            sp.AddressU = sp.AddressV = sp.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
            sp.ComparisonFunc = D3D11_COMPARISON_NEVER;
            sp.MaxLOD = D3D11_FLOAT32_MAX;
            if (FAILED(dev->CreateSamplerState(&sp, &s.sampler))) return false;
            D3D11_RASTERIZER_DESC rz{};
            rz.FillMode = D3D11_FILL_SOLID;
            rz.CullMode = D3D11_CULL_NONE;
            rz.DepthClipEnable = true;
            if (FAILED(dev->CreateRasterizerState(&rz, &s.raster))) return false;
            D3D11_DEPTH_STENCIL_DESC zd{};
            zd.DepthEnable = true;
            zd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
            zd.DepthFunc = D3D11_COMPARISON_LESS;
            if (FAILED(dev->CreateDepthStencilState(&zd, &s.depth))) return false;
            D3D11_TEXTURE2D_DESC rd{};
            rd.Width = kRtSize; rd.Height = kRtSize;
            rd.MipLevels = 1; rd.ArraySize = 1;
            rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rd.SampleDesc.Count = 1;
            rd.Usage = D3D11_USAGE_DEFAULT;
            rd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(dev->CreateTexture2D(&rd, nullptr, &s.rt))) return false;
            if (FAILED(dev->CreateRenderTargetView(s.rt, nullptr, &s.rtv))) return false;
            if (FAILED(dev->CreateShaderResourceView(s.rt, nullptr, &s.srv))) return false;
            D3D11_TEXTURE2D_DESC dd{};
            dd.Width = kRtSize; dd.Height = kRtSize;
            dd.MipLevels = 1; dd.ArraySize = 1;
            dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dd.SampleDesc.Count = 1;
            dd.Usage = D3D11_USAGE_DEFAULT;
            dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if (FAILED(dev->CreateTexture2D(&dd, nullptr, &s.zb))) return false;
            if (FAILED(dev->CreateDepthStencilView(s.zb, nullptr, &s.dsv))) return false;
            return true;
        }
        void commit_meshes(ID3D11Device* dev)
        {
            if (s.thumbPending.load(std::memory_order_acquire))
            {
                std::lock_guard<std::mutex> tl(s.gate);
                if (s.thumbPending.load(std::memory_order_relaxed))
                {
                    if (s.thumbSrv) { s.thumbSrv->Release(); s.thumbSrv = nullptr; s.thumbW = s.thumbH = 0; }
                    if (!s.pendingThumb.px.empty())
                    {
                        if (ID3D11ShaderResourceView* srv = make_texture(dev, s.pendingThumb))
                        {
                            s.thumbSrv = srv;
                            s.thumbW = s.pendingThumb.w;
                            s.thumbH = s.pendingThumb.h;
                        }
                    }
                    s.pendingThumb = DecodedTex{};
                    s.thumbPending.store(false, std::memory_order_release);
                }
            }
            if (!s.pending.load(std::memory_order_acquire)) return;
            std::lock_guard<std::mutex> l(s.gate);
            if (!s.pending.load(std::memory_order_relaxed)) return;
            for (auto& mm : s.meshes)
            {
                if (mm.vb) mm.vb->Release();
                if (mm.ib) mm.ib->Release();
            }
            s.meshes.clear();
            for (auto& kv : s.images)
                if (kv.second) kv.second->Release();
            s.images.clear();
            s.surfaces.clear();
            for (auto& kv : s.pendingImages)
                if (auto* srv = make_texture(dev, kv.second))
                    s.images[kv.first] = srv;
            s.surfaces = std::move(s.pendingSurfaces);
            for (auto& src : s.pendingMeshes)
            {
                if (src.verts.empty() || src.idx.empty() || src.verts.size() > kMaxVerts)
                    continue;
                LiveMesh mm;
                mm.tag = src.tag;
                mm.count = (uint32_t)src.idx.size();
                D3D11_BUFFER_DESC vd{};
                vd.ByteWidth = (UINT)(src.verts.size() * sizeof(GpuVertex));
                vd.Usage = D3D11_USAGE_IMMUTABLE;
                vd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
                D3D11_SUBRESOURCE_DATA vsrc{};
                vsrc.pSysMem = src.verts.data();
                if (FAILED(dev->CreateBuffer(&vd, &vsrc, &mm.vb))) continue;
                D3D11_BUFFER_DESC id{};
                id.ByteWidth = (UINT)(src.idx.size() * sizeof(uint32_t));
                id.Usage = D3D11_USAGE_IMMUTABLE;
                id.BindFlags = D3D11_BIND_INDEX_BUFFER;
                D3D11_SUBRESOURCE_DATA isrc{};
                isrc.pSysMem = src.idx.data();
                if (FAILED(dev->CreateBuffer(&id, &isrc, &mm.ib)))
                {
                    mm.vb->Release();
                    continue;
                }
                {
                    auto sit = s.surfaces.find(src.tag);
                    if (sit != s.surfaces.end())
                    {
                        mm.tint = DirectX::XMFLOAT4(sit->second.base.x, sit->second.base.y, sit->second.base.z, 1.0f);
                        if (!sit->second.tex.empty())
                        {
                            auto tit = s.images.find(sit->second.tex);
                            if (tit != s.images.end() && tit->second)
                                mm.srv = tit->second;
                        }
                    }
                }
                s.meshes.push_back(mm);
            }
            {
                s.partCenters.clear();
                bool any = false;
                V3 mn{ 1e9f, 1e9f, 1e9f }, mx{ -1e9f, -1e9f, -1e9f };
                for (auto& src : s.pendingMeshes)
                {
                    V3 psum{ 0, 0, 0 };
                    for (auto& v : src.verts)
                    {
                        if (v.p.x < mn.x) mn.x = v.p.x;
                        if (v.p.y < mn.y) mn.y = v.p.y;
                        if (v.p.z < mn.z) mn.z = v.p.z;
                        if (v.p.x > mx.x) mx.x = v.p.x;
                        if (v.p.y > mx.y) mx.y = v.p.y;
                        if (v.p.z > mx.z) mx.z = v.p.z;
                        psum.x += v.p.x;
                        psum.y += v.p.y;
                        psum.z += v.p.z;
                        any = true;
                    }
                    if (!src.verts.empty())
                    {
                        float inv = 1.0f / (float)src.verts.size();
                        s.partCenters[src.tag] = { psum.x * inv, psum.y * inv, psum.z * inv };
                    }
                }
                if (any && mx.x > mn.x && mx.y > mn.y && mx.z > mn.z)
                {
                    s.bbMin = mn;
                    s.bbMax = mx;
                    s.focus = { (mn.x + mx.x) * 0.5f, (mn.y + mx.y) * 0.5f, (mn.z + mx.z) * 0.5f };
                }
            }
            s.pendingMeshes.clear();
            s.pendingImages.clear();
            s.pending.store(false, std::memory_order_release);
            s.phase = Phase::Done;
            s.note = "Avatar is on screen.";
            s.needFit = true;
        }
        bool fetch_live_thumb(const std::string& uid, DecodedTex& out)
        {
            out = DecodedTex{};
            for (int attempt = 0; attempt < 10; ++attempt)
            {
                Http::Reply meta = Http::fetch(
                    "https://thumbnails.roblox.com/v1/users/avatar?userIds=" + uid +
                    "&size=720x720&format=Png&isCircular=false", "");
                if (!meta.body.empty())
                {
                    JParser pr(meta.body);
                    Jdom doc = pr.value();
                    const Jdom* data = (!pr.fail && doc.is_obj()) ? doc.find("data") : nullptr;
                    if (data && data->is_arr() && !data->arr.empty() && data->arr[0].is_obj())
                    {
                        const Jdom& e = data->arr[0];
                        std::string url = e.str_or("imageUrl");
                        if (!url.empty() && e.str_or("state") == "Completed")
                        {
                            Http::Reply img = Http::fetch(url, "");
                            if (img.body.size() >= 8 && memcmp(img.body.data(), "\x89PNG", 4) == 0)
                            {
                                out = decode_bytes(img.body);
                                if (!out.px.empty()) return true;
                            }
                            return false;
                        }
                    }
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            return false;
        }
        bool use_live_thumb_fallback(const std::string& uid, const std::string& diag)
        {
            s.phase = Phase::Fetch;
            s.note = "Avatar backend is down, fetching live Roblox avatar...";
            DecodedTex dt;
            if (!fetch_live_thumb(uid, dt))
            {
                s.phase = Phase::Broke;
                s.note = diag;
                s.busy = false;
                return false;
            }
            {
                std::lock_guard<std::mutex> l(s.gate);
                s.pendingThumb = std::move(dt);
                s.thumbPending.store(true, std::memory_order_release);
            }
            s.phase = Phase::Broke;
            s.note = "Avatar backend is down, showing live Roblox avatar.";
            s.busy = false;
            return true;
        }
        void worker_fetch(std::string uid)
        {
            {
                Http::Reply chk = Http::fetch("https://users.roblox.com/v1/users/" + uid, "");
                if (!chk.body.empty() && chk.code != 0)
                {
                    long long rid = 0;
                    try { rid = std::stoll(uid); } catch (...) { rid = 0; }
                    JParser cp(chk.body);
                    Jdom cdoc = cp.value();
                    const Jdom* cid = (!cp.fail && cdoc.is_obj()) ? cdoc.find("id") : nullptr;
                    long long got = (cid && cid->type == Jdom::Type::Num) ? (long long)cid->num : 0;
                    if (got != rid || rid <= 0)
                    {
                        s.phase = Phase::Broke;
                        s.note = "Bad Roblox ID (no such user).";
                        s.busy = false;
                        return;
                    }
                }
            }
            if (s.jar.empty())
            {
                s.phase = Phase::Fetch;
                s.note = "Signing in to the API...";
                Http::login(s.jar);
            }
            s.phase = Phase::Fetch;
            s.note = "Requesting avatar " + uid + "...";
            std::string payload;
            bool authed = false;
            for (int i = 0; i < kFetchRounds; ++i)
            {
                auto r = Http::fetch(std::string("https://") + kBackendHost + "/api/avatar?userId=" + uid, s.jar);
                if (r.code == 401 && !authed)
                {
                    authed = true;
                    s.jar.clear();
                    Http::login(s.jar);
                    continue;
                }
                if (r.body.empty())
                {
                    char diag[160];
                    snprintf(diag, sizeof(diag), "The API did not answer (HTTP %u).", r.code);
                    use_live_thumb_fallback(uid, diag);
                    return;
                }
                JParser pr(r.body);
                Jdom doc = pr.value();
                if (pr.fail || !doc.is_obj())
                {
                    char diag[256];
                    snprintf(diag, sizeof(diag), "Avatar backend is down (HTTP %u %s).",
                        r.code, describe_head(r.body).c_str());
                    use_live_thumb_fallback(uid, diag);
                    return;
                }
                const Jdom* err = doc.find("error");
                if (err && err->is_str() && !err->str.empty())
                {
                    s.phase = Phase::Broke;
                    s.note = std::string("API error: ") + err->str;
                    s.busy = false;
                    return;
                }
                const Jdom* data = doc.find("data");
                if (!data || !data->is_obj())
                {
                    s.phase = Phase::Broke;
                    s.note = "The API sent back something unexpected.";
                    s.busy = false;
                    return;
                }
                std::string st = data->str_or("state");
                if (st == "Completed") { payload = r.body; break; }
                s.phase = Phase::Wait;
                s.note = "Baking on Roblox side (" + st + "), retrying...";
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            if (payload.empty())
            {
                s.phase = Phase::Broke;
                s.note = "Timed out waiting on the avatar.";
                s.busy = false;
                return;
            }
            JParser pr(payload);
            Jdom doc = pr.value();
            const Jdom* data = (!pr.fail && doc.is_obj()) ? doc.find("data") : nullptr;
            const Jdom* mdl = (data && data->is_obj()) ? data->find("model") : nullptr;
            if (!mdl || !mdl->is_obj())
            {
                s.phase = Phase::Broke;
                s.note = "The API sent back something unexpected.";
                s.busy = false;
                return;
            }
            std::string objUrl = mdl->str_or("objUrl");
            std::string mtlUrl = mdl->str_or("mtlUrl");
            if (objUrl.empty() || mtlUrl.empty())
            {
                s.phase = Phase::Broke;
                s.note = "The API sent back something unexpected.";
                s.busy = false;
                return;
            }
            const Jdom* cam = mdl->find("camera");
            if (cam && cam->is_obj())
            {
                s.fov = (float)cam->num_or("fov", 70.0);
                const Jdom* cp = cam->find("position");
                V3 campos{ 0, 0, 0 };
                if (cp && cp->is_obj())
                {
                    campos.x = (float)cp->num_or("x", 0.0);
                    campos.y = (float)cp->num_or("y", 0.0);
                    campos.z = (float)cp->num_or("z", 0.0);
                }
                const Jdom* ab = mdl->find("aabb");
                V3 lo{ -5, -5, -5 }, hi{ 5, 5, 5 };
                if (ab && ab->is_obj())
                {
                    const Jdom* mn = ab->find("min");
                    const Jdom* mx = ab->find("max");
                    if (mn && mn->is_obj())
                    {
                        lo.x = (float)mn->num_or("x", lo.x);
                        lo.y = (float)mn->num_or("y", lo.y);
                        lo.z = (float)mn->num_or("z", lo.z);
                    }
                    if (mx && mx->is_obj())
                    {
                        hi.x = (float)mx->num_or("x", hi.x);
                        hi.y = (float)mx->num_or("y", hi.y);
                        hi.z = (float)mx->num_or("z", hi.z);
                    }
                }
                s.focus = { (lo.x + hi.x) * 0.5f, (lo.y + hi.y) * 0.5f, (lo.z + hi.z) * 0.5f };
                s.bbMin = lo;
                s.bbMax = hi;
                float ox = campos.x - s.focus.x, oy = campos.y - s.focus.y, oz = campos.z - s.focus.z;
                s.dist = sqrtf(ox * ox + oy * oy + oz * oz);
                if (s.dist < 1.0f) s.dist = 15.0f;
                s.yaw = atan2f(oz, ox);
                float c = oy / s.dist;
                if (c < -1.0f) c = -1.0f;
                if (c > 1.0f) c = 1.0f;
                s.pitch = acosf(c);
                s.homeYaw = s.yaw;
                s.homePitch = s.pitch;
            }
            std::vector<std::pair<std::string, std::string>> texList;
            const Jdom* texs = mdl->find("textures");
            if (texs && texs->is_arr())
            {
                for (auto& t : texs->arr)
                {
                    if (!t.is_obj()) continue;
                    std::string id = t.str_or("id");
                    std::string u = t.str_or("url");
                    if (!id.empty() && !u.empty())
                        texList.emplace_back(id, u);
                }
            }
            s.phase = Phase::Pull;
            s.note = "Fetching mesh + materials...";
            Http::Reply objR, mtlR;
            {
                std::thread to([&]() { objR = Http::fetch(objUrl, s.jar); });
                std::thread tm([&]() { mtlR = Http::fetch(mtlUrl, s.jar); });
                if (to.joinable()) to.join();
                if (tm.joinable()) tm.join();
            }
            auto looks_bad = [](const std::string& b) {
                size_t i = 0;
                while (i < b.size())
                {
                    unsigned char c = static_cast<unsigned char>(b[i]);
                    if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 0xEF || c == 0xBB || c == 0xBF || c == 0) { ++i; continue; }
                    break;
                }
                if (i >= b.size()) return true;
                unsigned char c0 = static_cast<unsigned char>(b[i]);
                if (c0 == '<') return true;
                if (c0 == '{' || c0 == '[') return true;
                if (i + 4 <= b.size() && b[i] == 'g' && b[i + 1] == 'l' && b[i + 2] == 'T' && b[i + 3] == 'F') return true;
                return false;
            };
            std::string obj = std::move(objR.body);
            std::string mtl = std::move(mtlR.body);
            bool objWasGzip = try_gunzip_inplace(obj);
            bool mtlWasGzip = try_gunzip_inplace(mtl);
            (void)objWasGzip; (void)mtlWasGzip;
            for (char& c : mtl) if (c == '\r') c = '\n';
            if ((objR.code != 200 && objR.code != 0) || (mtlR.code != 200 && mtlR.code != 0) ||
                obj.empty() || mtl.empty() || obj.size() > (kMaxTextureBytes + 65536) ||
                looks_bad(obj) || looks_bad(mtl))
            {
                s.phase = Phase::Broke;
                char diag[256];
                snprintf(diag, sizeof(diag), "Model download failed (obj %u/%lluB %s, mtl %u/%lluB %s).",
                    objR.code, (unsigned long long)obj.size(), describe_head(obj).c_str(),
                    mtlR.code, (unsigned long long)mtl.size(), describe_head(mtl).c_str());
                s.note = diag;
                s.busy = false;
                return;
            }
            s.note = "Fetching skin textures...";
            std::string fixed = rewrite_mtl(mtl, texList);
            auto mats = read_mtl(fixed);
            std::map<std::string, DecodedTex> blobs;
            std::mutex blobMtx;
            std::atomic<size_t> blobTotal{ 0 };
            std::atomic<size_t> blobDone{ 0 };
            size_t texCount = texList.size();
            unsigned workers = (unsigned)texList.size();
            if (workers > 8) workers = 8;
            if (workers < 1) workers = 1;
            std::atomic<int> alive{ (int)workers };
            std::vector<std::thread> pool;
            for (unsigned w = 0; w < workers; ++w)
            {
                try
                {
                    pool.emplace_back([&, w]() {
                        HINTERNET sess = InternetOpenA("AvatarViewer", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
                        if (sess)
                        {
                            DWORD c = kNetDialMs, iot = kNetIoMs;
                            InternetSetOptionA(sess, INTERNET_OPTION_CONNECT_TIMEOUT, &c, sizeof(c));
                            InternetSetOptionA(sess, INTERNET_OPTION_SEND_TIMEOUT, &iot, sizeof(iot));
                            InternetSetOptionA(sess, INTERNET_OPTION_RECEIVE_TIMEOUT, &iot, sizeof(iot));
                            for (size_t i = w; i < texList.size(); i += workers)
                            {
                                Http::Reply r = Http::fetch_on(sess, texList[i].second, s.jar);
                                if (r.body.empty() || r.body.size() > kMaxTextureBytes)
                                {
                                    ++blobDone;
                                    continue;
                                }
                                size_t prev = blobTotal.fetch_add(r.body.size());
                                if (prev > (kMaxTextureBytes * 4))
                                {
                                    ++blobDone;
                                    continue;
                                }
                                DecodedTex dt = decode_bytes(r.body);
                                if (!dt.px.empty())
                                {
                                    std::lock_guard<std::mutex> l(blobMtx);
                                    blobs[texList[i].second] = std::move(dt);
                                }
                                ++blobDone;
                            }
                            InternetCloseHandle(sess);
                        }
                        --alive;
                    });
                }
                catch (...)
                {
                    --alive;
                }
            }
            while (alive > 0)
            {
                char prog[96];
                snprintf(prog, sizeof(prog), "Fetching skin textures (%llu/%llu)...",
                    (unsigned long long)blobDone.load(), (unsigned long long)texCount);
                s.note = prog;
                std::this_thread::sleep_for(std::chrono::milliseconds(120));
            }
            for (auto& th : pool)
                if (th.joinable()) th.join();
            s.phase = Phase::Bake;
            s.note = "Assembling geometry...";
            std::vector<BakedMesh> baked;
            size_t objVerts = 0, objFaces = 0, objBytes = obj.size(), objLines = 0;
            std::string firstOp;
            try
            {
                baked = read_obj(obj, objVerts, objFaces, &objLines, &firstOp);
            }
            catch (...)
            {
                s.phase = Phase::Broke;
                s.note = "Could not parse the model.";
                s.busy = false;
                return;
            }
            if (baked.empty())
            {
                s.phase = Phase::Broke;
                std::string head = describe_head(obj);
                if (firstOp.empty()) firstOp = "none";
                char diag[256];
                snprintf(diag, sizeof(diag), "Model had no geometry (%lluB, %llu vtx, %llu faces, %llu lines, first=%s head=%s).",
                    (unsigned long long)objBytes, (unsigned long long)objVerts, (unsigned long long)objFaces,
                    (unsigned long long)objLines, firstOp.c_str(), head.c_str());
                s.note = diag;
                s.busy = false;
                return;
            }
            s.phase = Phase::Paint;
            s.note = "Sending everything to the GPU...";
            {
                std::lock_guard<std::mutex> l(s.gate);
                s.pendingMeshes = std::move(baked);
                s.pendingSurfaces = std::move(mats);
                s.pendingImages = std::move(blobs);
                s.pending.store(true, std::memory_order_release);
            }
            s.busy = false;
        }
        void pull_uid(const std::string& uid)
        {
            if (uid.empty() || s.busy) return;
            s.busy = true;
            try
            {
                std::thread([uid]() { worker_fetch(uid); }).detach();
            }
            catch (...)
            {
                s.busy = false;
            }
        }
        bool ensure_rt(ID3D11Device* dev, UINT w, UINT h)
        {
            if (w < 64) w = 64;
            if (h < 64) h = 64;
            if (w > 1024) w = 1024;
            if (h > 1024) h = 1024;
            w &= ~1u;
            h &= ~1u;
            if (s.rt && s.rtW == w && s.rtH == h)
                return true;
            if (s.rtv) { s.rtv->Release(); s.rtv = nullptr; }
            if (s.srv) { s.srv->Release(); s.srv = nullptr; }
            if (s.rt) { s.rt->Release(); s.rt = nullptr; }
            if (s.dsv) { s.dsv->Release(); s.dsv = nullptr; }
            if (s.zb) { s.zb->Release(); s.zb = nullptr; }
            D3D11_TEXTURE2D_DESC rd{};
            rd.Width = w; rd.Height = h;
            rd.MipLevels = 1; rd.ArraySize = 1;
            rd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            rd.SampleDesc.Count = 1;
            rd.Usage = D3D11_USAGE_DEFAULT;
            rd.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
            if (FAILED(dev->CreateTexture2D(&rd, nullptr, &s.rt))) return false;
            if (FAILED(dev->CreateRenderTargetView(s.rt, nullptr, &s.rtv))) return false;
            if (FAILED(dev->CreateShaderResourceView(s.rt, nullptr, &s.srv))) return false;
            D3D11_TEXTURE2D_DESC dd{};
            dd.Width = w; dd.Height = h;
            dd.MipLevels = 1; dd.ArraySize = 1;
            dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            dd.SampleDesc.Count = 1;
            dd.Usage = D3D11_USAGE_DEFAULT;
            dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
            if (FAILED(dev->CreateTexture2D(&dd, nullptr, &s.zb))) return false;
            if (FAILED(dev->CreateDepthStencilView(s.zb, nullptr, &s.dsv))) return false;
            s.rtW = w;
            s.rtH = h;
            return true;
        }
        static inline void DrawPreviewText(ImDrawList* dl, ImFont* font, float fontSize, float x, float y, ImU32 col, bool outline, ImU32 outlineCol, const char* text)
        {
            if (!dl || !text || !text[0]) return;
            if (!font) font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
            float ix = std::floor(x);
            float iy = std::floor(y);
            if (outline) {
                dl->AddText(font, fontSize, ImVec2(ix + 1.0f, iy + 1.0f), outlineCol, text);
            }
            dl->AddText(font, fontSize, ImVec2(ix, iy), col, text);
        }
        static void render_esp_visuals(
            ImDrawList* dl,
            const thenwefuckin::data::rect& rc,
            const ImVec2& headPt,
            const std::vector<std::pair<ImVec2, ImVec2>>& bones,
            bool hasHead,
            bool hasBones)
        {
            if (!dl) return;
            float animHp = 100.0f;
            if (Menu::g_previewAnimateHp)
            {
                float cycle = fmodf((float)ImGui::GetTime() * 0.35f, 1.0f);
                float norm = 1.0f;
                if (cycle < 0.20f)
                {
                    norm = 1.0f;
                }
                else if (cycle < 0.55f)
                {
                    float p = (cycle - 0.20f) / 0.35f;
                    float ease = 0.5f - 0.5f * cosf(p * 3.14159265f);
                    norm = 1.0f - ease * 0.95f;
                }
                else if (cycle < 0.65f)
                {
                    norm = 0.05f;
                }
                else
                {
                    float p = (cycle - 0.65f) / 0.35f;
                    float ease = 0.5f - 0.5f * cosf(p * 3.14159265f);
                    norm = 0.05f + ease * 0.95f;
                }
                animHp = std::clamp(norm * 100.0f, 1.0f, 100.0f);
            }
            if (thenwefuckin::settings::skeleton_enabled && hasBones)
            {
                ImU32 skelCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::skeleton_color[0], thenwefuckin::settings::skeleton_color[1], thenwefuckin::settings::skeleton_color[2], 1.0f));
                ImU32 skelOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::skeleton_outline_color[0], thenwefuckin::settings::skeleton_outline_color[1], thenwefuckin::settings::skeleton_outline_color[2], 1.0f));
                const float baseThick = 1.5f;
                const float outlineThick = baseThick + 1.0f;
                for (const auto& b : bones)
                {
                    if (thenwefuckin::settings::skeleton_outline)
                        dl->AddLine(b.first, b.second, skelOutlineCol, outlineThick);
                    dl->AddLine(b.first, b.second, skelCol, baseThick);
                }
            }
            if (thenwefuckin::settings::headdot_enabled && hasHead)
            {
                const float r = (std::max)(thenwefuckin::settings::headdot_thickness, 2.0f);
                ImU32 hcol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::headdot_color[0], thenwefuckin::settings::headdot_color[1], thenwefuckin::settings::headdot_color[2], 1.0f));
                ImU32 hoc = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::headdot_outline_color[0], thenwefuckin::settings::headdot_outline_color[1], thenwefuckin::settings::headdot_outline_color[2], 1.0f));
                if (thenwefuckin::settings::headdot_fill)
                {
                    if (thenwefuckin::settings::headdot_outline)
                        dl->AddCircleFilled(headPt, r + 1.0f, hoc);
                    dl->AddCircleFilled(headPt, r, hcol);
                }
                else if (thenwefuckin::settings::headdot_outline)
                {
                    dl->AddCircle(headPt, r, hoc, 0, 1.5f);
                }
            }
            if (thenwefuckin::settings::box_enabled)
            {
                if (thenwefuckin::settings::box_filled)
                {
                    if (thenwefuckin::settings::box_fill_gradient)
                    {
                        ImU32 c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::box_fill_color[0], thenwefuckin::settings::box_fill_color[1], thenwefuckin::settings::box_fill_color[2], thenwefuckin::settings::box_fill_color[3]));
                        ImU32 c2 = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::box_fill_color2[0], thenwefuckin::settings::box_fill_color2[1], thenwefuckin::settings::box_fill_color2[2], thenwefuckin::settings::box_fill_color2[3]));
                        thenwefuckin::visual::box::draw_fill_gradient(dl, rc.x0, rc.y0, rc.x1, rc.y1, c1, c2);
                    }
                    else
                    {
                        ImU32 c1 = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::box_fill_color[0], thenwefuckin::settings::box_fill_color[1], thenwefuckin::settings::box_fill_color[2], thenwefuckin::settings::box_fill_color[3]));
                        thenwefuckin::visual::box::draw_fill(dl, rc.x0, rc.y0, rc.x1, rc.y1, c1);
                    }
                }
                ImU32 boxCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::box_color[0], thenwefuckin::settings::box_color[1], thenwefuckin::settings::box_color[2], 1.0f));
                ImU32 boxOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::box_outline_color[0], thenwefuckin::settings::box_outline_color[1], thenwefuckin::settings::box_outline_color[2], 1.0f));
                if (thenwefuckin::settings::box_style == 1)
                {
                    thenwefuckin::visual::box::draw_corner(dl, rc.x0, rc.y0, rc.x1, rc.y1, boxCol, thenwefuckin::settings::box_outline, boxOutlineCol);
                }
                else
                {
                    thenwefuckin::visual::box::draw(dl, rc.x0, rc.y0, rc.x1, rc.y1, boxCol, thenwefuckin::settings::box_outline, boxOutlineCol);
                }
            }
            if (thenwefuckin::settings::health_enabled)
            {
                ImU32 hoc = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::health_outline_color[0], thenwefuckin::settings::health_outline_color[1], thenwefuckin::settings::health_outline_color[2], 1.0f));
                ImU32 hcol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::health_color[0], thenwefuckin::settings::health_color[1], thenwefuckin::settings::health_color[2], 1.0f));
                thenwefuckin::visual::health::draw(dl, rc, animHp, 100.0f, thenwefuckin::settings::health_outline, hoc, thenwefuckin::settings::health_gradient, thenwefuckin::settings::health_text, hcol, thenwefuckin::settings::box_enabled && thenwefuckin::settings::box_outline, 1.0f);
            }
            if (thenwefuckin::settings::name_enabled)
            {
                ImU32 nameCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::name_color[0], thenwefuckin::settings::name_color[1], thenwefuckin::settings::name_color[2], 1.0f));
                ImU32 nameOutlineCol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::name_outline_color[0], thenwefuckin::settings::name_outline_color[1], thenwefuckin::settings::name_outline_color[2], 1.0f));
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                std::string nameStr = "Preview";
                if (g_ctx.ready && !g_ctx.local_player.name().empty())
                    nameStr = g_ctx.local_player.name();
                ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, nameStr.c_str());
                float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - ts.x * 0.5f;
                float ty = rc.y0 - ts.y - 3.0f;
                DrawPreviewText(dl, font, fontSize, tx, ty, nameCol, thenwefuckin::settings::name_outline, nameOutlineCol, nameStr.c_str());
            }
            float distY = rc.y1 + 2.0f;
            if (thenwefuckin::settings::distance_enabled)
            {
                const char* distBuf = "15m";
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                ImVec2 distTs = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, distBuf);
                float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - distTs.x * 0.5f;
                float ty = distY;
                ImU32 dcol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::distance_color[0], thenwefuckin::settings::distance_color[1], thenwefuckin::settings::distance_color[2], 1.0f));
                ImU32 doc = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::distance_outline_color[0], thenwefuckin::settings::distance_outline_color[1], thenwefuckin::settings::distance_outline_color[2], 1.0f));
                DrawPreviewText(dl, font, fontSize, tx, ty, dcol, thenwefuckin::settings::distance_outline, doc, distBuf);
                distY += distTs.y + 2.0f;
            }
            if (thenwefuckin::settings::tool_enabled)
            {
                std::string toolStr = "Tool";
                if (g_ctx.ready && g_ctx.local_player.addr)
                {
                    node ch = g_ctx.get_character(g_ctx.local_player);
                    if (ch.addr)
                    {
                        node eq = ch.find_child_of_class("Tool");
                        if (eq && !eq.name().empty()) toolStr = eq.name();
                    }
                }
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                ImVec2 ts = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, toolStr.c_str());
                float tx = rc.x0 + (rc.x1 - rc.x0) * 0.5f - ts.x * 0.5f;
                float ty = distY;
                ImU32 tcol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::tool_color[0], thenwefuckin::settings::tool_color[1], thenwefuckin::settings::tool_color[2], 1.0f));
                ImU32 tocol = ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::tool_outline_color[0], thenwefuckin::settings::tool_outline_color[1], thenwefuckin::settings::tool_outline_color[2], 1.0f));
                DrawPreviewText(dl, font, fontSize, tx, ty, tcol, thenwefuckin::settings::tool_outline, tocol, toolStr.c_str());
                distY += ts.y + 2.0f;
            }
            if (thenwefuckin::settings::flags_enabled && (thenwefuckin::settings::flags_team || thenwefuckin::settings::flags_state || thenwefuckin::settings::flags_health))
            {
                ImFont* font = g_verdanaFont ? g_verdanaFont : ImGui::GetFont();
                float fontSize = 13.0f;
                float fx = rc.x1 + 4.0f;
                float fy = rc.y0;
                if (thenwefuckin::settings::flags_team)
                {
                    std::string teamStr = "Team";
                    if (g_ctx.ready && g_ctx.local_player.addr)
                    {
                        uintptr_t teamAddr = g_proc->read<uintptr_t>(g_ctx.local_player.addr + Offsets::Player::Team);
                        if (teamAddr)
                        {
                            std::string tm = node{ teamAddr }.name();
                            if (!tm.empty()) teamStr = tm;
                        }
                    }
                    ImU32 fcol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_color[0], thenwefuckin::settings::flags_color[1], thenwefuckin::settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_team_color[0], thenwefuckin::settings::flags_team_color[1], thenwefuckin::settings::flags_team_color[2], 1.0f));
                    ImU32 focol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_outline_color[0], thenwefuckin::settings::flags_outline_color[1], thenwefuckin::settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_team_outline_color[0], thenwefuckin::settings::flags_team_outline_color[1], thenwefuckin::settings::flags_team_outline_color[2], 1.0f));
                    bool use_outline = thenwefuckin::settings::flags_global ? thenwefuckin::settings::flags_outline : thenwefuckin::settings::flags_team_outline;
                    DrawPreviewText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, teamStr.c_str());
                    fy += fontSize + 2.0f;
                }
                if (thenwefuckin::settings::flags_state)
                {
                    const char* stateStr = "Standing";
                    ImU32 fcol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_color[0], thenwefuckin::settings::flags_color[1], thenwefuckin::settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_state_color[0], thenwefuckin::settings::flags_state_color[1], thenwefuckin::settings::flags_state_color[2], 1.0f));
                    ImU32 focol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_outline_color[0], thenwefuckin::settings::flags_outline_color[1], thenwefuckin::settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_state_outline_color[0], thenwefuckin::settings::flags_state_outline_color[1], thenwefuckin::settings::flags_state_outline_color[2], 1.0f));
                    bool use_outline = thenwefuckin::settings::flags_global ? thenwefuckin::settings::flags_outline : thenwefuckin::settings::flags_state_outline;
                    DrawPreviewText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, stateStr);
                    fy += fontSize + 2.0f;
                }
                if (thenwefuckin::settings::flags_health)
                {
                    char hpStr[16];
                    snprintf(hpStr, sizeof(hpStr), "%.0f%%", (double)animHp);
                    ImU32 fcol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_color[0], thenwefuckin::settings::flags_color[1], thenwefuckin::settings::flags_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_health_color[0], thenwefuckin::settings::flags_health_color[1], thenwefuckin::settings::flags_health_color[2], 1.0f));
                    ImU32 focol = thenwefuckin::settings::flags_global
                        ? ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_outline_color[0], thenwefuckin::settings::flags_outline_color[1], thenwefuckin::settings::flags_outline_color[2], 1.0f))
                        : ImGui::ColorConvertFloat4ToU32(ImVec4(thenwefuckin::settings::flags_health_outline_color[0], thenwefuckin::settings::flags_health_outline_color[1], thenwefuckin::settings::flags_health_outline_color[2], 1.0f));
                    bool use_outline = thenwefuckin::settings::flags_global ? thenwefuckin::settings::flags_outline : thenwefuckin::settings::flags_health_outline;
                    DrawPreviewText(dl, font, fontSize, fx, fy, fcol, use_outline, focol, hpStr);
                    fy += fontSize + 2.0f;
                }
            }
        }
    }
    static void render_backend(ImDrawList* dl, ImVec2 min, ImVec2 max);
    void render(ImDrawList* dl, ImVec2 min, ImVec2 max)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 m = io.MousePos;
        double now = ImGui::GetTime();
        int64_t uid = kPreviewUid;
        {
            static double lastUidCheck = -100.0;
            static int64_t cachedUid = kPreviewUid;
            static bool haveCached = false;
            if (now - lastUidCheck > 2.0)
            {
                lastUidCheck = now;
                int64_t live = 0;
                if (g_proc && g_proc->connected() && g_ctx.ready && g_ctx.local_player.addr)
                    live = g_proc->read<int64_t>(g_ctx.local_player.addr + Offsets::Player::UserId);
                if (live > 0) { cachedUid = live; haveCached = true; }
                else if (!haveCached) { cachedUid = kPreviewUid; }
            }
            uid = cachedUid;
        }
        if (uid > 0 && uid != s.pulledUid && !s.busy && now - s.lastTry > 1.0)
        {
            s.lastTry = now;
            s.pulledUid = uid;
            char buf[32];
            snprintf(buf, sizeof(buf), "%lld", (long long)uid);
            pull_uid(buf);
        }
        if (s.phase == Phase::Broke && !s.busy && now - s.lastTry > 20.0 && uid > 0
            && !s.thumbSrv && !s.thumbPending.load(std::memory_order_acquire))
        {
            s.lastTry = now;
            char buf[32];
            snprintf(buf, sizeof(buf), "%lld", (long long)uid);
            pull_uid(buf);
        }
        render_backend(dl, min, max);
    }
    static void render_backend(ImDrawList* dl, ImVec2 min, ImVec2 max)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 m = io.MousePos;
        double now = ImGui::GetTime();
        ID3D11Device* dev = Overlay::GetDeviceStatic();
        ID3D11DeviceContext* ctx = Overlay::GetContextStatic();
        if (dev && ctx && !s.gfxReady && !s.gfxFailed)
        {
            if (--s.gfxRetryCd <= 0)
            {
                s.gfxRetryCd = 300;
                if (compile_shaders(dev))
                    s.gfxReady = true;
                else
                {
                    s.gfxFailed = true;
                    s.note = "Preview renderer failed to start.";
                }
            }
        }
        if (dev && ctx && s.gfxReady)
            commit_meshes(dev);
        float vw = floorf(max.x - min.x);
        float vh = floorf(max.y - min.y);
        if (vw < 40.0f || vh < 40.0f)
            return;
        ImVec2 vpMin{ floorf(min.x + 2.0f), floorf(min.y + 2.0f) };
        ImVec2 vpMax{ floorf(max.x - 2.0f), floorf(max.y - 2.0f) };
        float vpw = vpMax.x - vpMin.x;
        float vph = vpMax.y - vpMin.y;
        bool modelShown = s.gfxReady && s.phase == Phase::Done && !s.meshes.empty() && s.srv;
        if (modelShown && !ensure_rt(dev, (UINT)vpw, (UINT)vph))
            modelShown = false;
        if (modelShown && (s.needFit || vpw != s.fittedW || vph != s.fittedH))
        {
            float fitH = s.bbMax.y - s.bbMin.y;
            if (fitH < 0.5f) fitH = 5.0f;
            float fitW = s.bbMax.x - s.bbMin.x;
            float fitD = s.bbMax.z - s.bbMin.z;
            if (fitD > fitW) fitW = fitD;
            if (fitW < 0.5f) fitW = 2.0f;
            float t = tanf(s.fov * 3.14159265f / 180.0f * 0.5f);
            if (t < 0.05f) t = 0.05f;
            float aspect = vph > 1.0f ? (vpw / vph) : 1.0f;
            float dH = (fitH * 0.5f) / t / 0.66f;
            float dW = (fitW * 0.5f) / t / aspect / 0.66f;
            s.dist = dH > dW ? dH : dW;
            if (s.dist < 2.0f) s.dist = 2.0f;
            if (s.dist > 60.0f) s.dist = 60.0f;
            s.needFit = false;
            s.fittedW = vpw;
            s.fittedH = vph;
        }
        if (modelShown)
        {
            bool hov = (m.x >= vpMin.x && m.x <= vpMax.x && m.y >= vpMin.y && m.y <= vpMax.y);
            bool modal = Menu::IsModalActive();
            static bool dragView = false;
            if (!dragView && hov && !modal && ImGui::IsMouseClicked(0))
                dragView = true;
            if (dragView)
            {
                if (ImGui::IsMouseDown(0) && !modal)
                {
                    ImVec2 d = ImGui::GetMouseDragDelta(0, 0.0f);
                    ImGui::ResetMouseDragDelta(0);
                    if (d.x != 0.0f || d.y != 0.0f)
                    {
                        s.yaw -= d.x * 0.005f;
                        s.pitch -= d.y * 0.005f;
                        if (s.pitch < 0.1f) s.pitch = 0.1f;
                        if (s.pitch > 3.04159f) s.pitch = 3.04159f;
                        s.spin = false;
                        s.lastTouch = now;
                    }
                }
                else
                {
                    dragView = false;
                }
            }
            static bool wasSpin = false;
            bool wantSpin = Menu::g_previewSpin;
            if (wantSpin)
                s.yaw += (float)io.DeltaTime * 0.4f;
            else if (wasSpin)
            {
                s.yaw = s.homeYaw;
                s.pitch = s.homePitch;
            }
            wasSpin = wantSpin;
            float sp = sinf(s.pitch), cp = cosf(s.pitch);
            float sy = sinf(s.yaw), cyy = cosf(s.yaw);
            DirectX::XMVECTOR at = DirectX::XMVectorSet(s.focus.x, s.focus.y, s.focus.z, 1.0f);
            DirectX::XMVECTOR eye = DirectX::XMVectorSet(
                s.focus.x + s.dist * sp * cyy,
                s.focus.y + s.dist * cp,
                s.focus.z + s.dist * sp * sy, 1.0f);
            DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);
            DirectX::XMMATRIX vwM = DirectX::XMMatrixLookAtLH(eye, at, up);
            DirectX::XMMATRIX pj = DirectX::XMMatrixPerspectiveFovLH(
                s.fov * 3.14159265f / 180.0f, (float)s.rtW / (float)s.rtH, 0.1f, 1000.0f);
            DirectX::XMMATRIX wd = DirectX::XMMatrixIdentity();
            static double lastScene = -100.0;
            bool animating = wantSpin || dragView;
            bool sceneDue = animating || (now - lastScene) >= (1.0 / 30.0);
            if (sceneDue)
            {
            lastScene = now;
            ID3D11RenderTargetView* oldRtv = nullptr;
            ID3D11DepthStencilView* oldDsv = nullptr;
            ctx->OMGetRenderTargets(1, &oldRtv, &oldDsv);
            UINT nvp = 1;
            D3D11_VIEWPORT oldVp{};
            ctx->RSGetViewports(&nvp, &oldVp);
            ID3D11DepthStencilState* oldDs = nullptr;
            UINT oldRef = 0;
            ctx->OMGetDepthStencilState(&oldDs, &oldRef);
            float bg[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            ctx->ClearRenderTargetView(s.rtv, bg);
            ctx->ClearDepthStencilView(s.dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
            D3D11_VIEWPORT vp{};
            vp.Width = (float)s.rtW; vp.Height = (float)s.rtH;
            vp.MaxDepth = 1.0f;
            ctx->RSSetViewports(1, &vp);
            ctx->OMSetRenderTargets(1, &s.rtv, s.dsv);
            ctx->OMSetDepthStencilState(s.depth, 1);
            ctx->IASetInputLayout(s.layout);
            ctx->VSSetShader(s.vs, nullptr, 0);
            ctx->PSSetShader(s.ps, nullptr, 0);
            ctx->PSSetSamplers(0, 1, &s.sampler);
            ctx->RSSetState(s.raster);
            ID3D11ShaderResourceView* blank = nullptr;
            ctx->PSSetShaderResources(0, 1, &blank);
            for (auto& mm : s.meshes)
            {
                DirectX::XMFLOAT4 col = mm.tint;
                float mapped = (mm.srv != nullptr) ? 1.0f : 0.0f;
                if (mm.srv)
                    ctx->PSSetShaderResources(0, 1, &mm.srv);
                else
                    ctx->PSSetShaderResources(0, 1, &blank);
                D3D11_MAPPED_SUBRESOURCE mmap{};
                if (SUCCEEDED(ctx->Map(s.ubo, 0, D3D11_MAP_WRITE_DISCARD, 0, &mmap)))
                {
                    auto* cb = (FrameConsts*)mmap.pData;
                    cb->w = DirectX::XMMatrixTranspose(wd);
                    cb->v = DirectX::XMMatrixTranspose(vwM);
                    cb->pr = DirectX::XMMatrixTranspose(pj);
                    cb->tint = col;
                    cb->textured = mapped;
                    ctx->Unmap(s.ubo, 0);
                }
                ctx->VSSetConstantBuffers(0, 1, &s.ubo);
                ctx->PSSetConstantBuffers(0, 1, &s.ubo);
                UINT stride = sizeof(GpuVertex), off = 0;
                ctx->IASetVertexBuffers(0, 1, &mm.vb, &stride, &off);
                ctx->IASetIndexBuffer(mm.ib, DXGI_FORMAT_R32_UINT, 0);
                ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                ctx->DrawIndexed(mm.count, 0, 0);
            }
            ctx->OMSetRenderTargets(1, &oldRtv, oldDsv);
            if (oldRtv) oldRtv->Release();
            if (oldDsv) oldDsv->Release();
            ctx->RSSetViewports(nvp, &oldVp);
            ctx->OMSetDepthStencilState(oldDs, oldRef);
            if (oldDs) oldDs->Release();
            ctx->PSSetShaderResources(0, 1, &blank);
            }
            ImVec2 wpos = ImGui::GetWindowPos();
            ImGui::SetCursorPos(ImVec2(vpMin.x - wpos.x, vpMin.y - wpos.y));
            ImGui::Image((ImTextureID)s.srv, ImVec2(vpw, vph));
            auto project_to_preview = [&](const V3& pt, ImVec2& out) -> bool {
                DirectX::XMVECTOR p = DirectX::XMVectorSet(pt.x, pt.y, pt.z, 1.0f);
                DirectX::XMVECTOR clip = DirectX::XMVector4Transform(p, DirectX::XMMatrixMultiply(vwM, pj));
                DirectX::XMFLOAT4 c;
                DirectX::XMStoreFloat4(&c, clip);
                if (c.w <= 0.001f)
                    return false;
                float invW = 1.0f / c.w;
                float ndcX = c.x * invW;
                float ndcY = c.y * invW;
                out.x = vpMin.x + (ndcX * 0.5f + 0.5f) * vpw;
                out.y = vpMin.y + (-ndcY * 0.5f + 0.5f) * vph;
                return true;
            };
            float hMin = s.bbMin.y;
            float hMax = s.bbMax.y;
            float hTotal = hMax - hMin;
            if (hTotal < 0.5f) hTotal = 5.0f;
            float wX = s.bbMax.x - s.bbMin.x;
            if (wX < 0.5f) wX = 2.0f;
            float cx = s.focus.x;
            float cz = s.focus.z;
            auto find_part = [&](const char* sub) -> V3* {
                size_t slen = strlen(sub);
                for (auto& kv : s.partCenters)
                {
                    const std::string& nm = kv.first;
                    if (nm.size() < slen) continue;
                    for (size_t i = 0; i + slen <= nm.size(); ++i)
                    {
                        size_t j = 0;
                        for (; j < slen; ++j)
                        {
                            char a = nm[i + j], b = sub[j];
                            if (a >= 'A' && a <= 'Z') a = (char)(a + 32);
                            if (b >= 'A' && b <= 'Z') b = (char)(b + 32);
                            if (a != b) break;
                        }
                        if (j == slen) return &kv.second;
                    }
                }
                return nullptr;
            };
            float skelDown = hTotal * 0.055f;
            V3* hpMesh = find_part("head");
            V3 headCenter = hpMesh ? *hpMesh : V3{ cx, hMin + hTotal * 0.88f, cz };
            headCenter.y -= skelDown;
            V3 headTop{ cx, hMax + 0.15f, cz };
            V3 neck{ cx, hMin + hTotal * 0.76f - skelDown, cz };
            V3* tpMesh = find_part("torso");
            V3 torso = tpMesh ? *tpMesh : V3{ cx, hMin + hTotal * 0.62f, cz };
            torso.y -= skelDown;
            V3 pelvis{ cx, hMin + hTotal * 0.48f - skelDown, cz };
            float shW = (std::max)(wX * 0.38f, 0.9f);
            float hipW = (std::max)(wX * 0.20f, 0.5f);
            V3 lShoulder{ cx - shW, hMin + hTotal * 0.74f - skelDown, cz };
            V3 rShoulder{ cx + shW, hMin + hTotal * 0.74f - skelDown, cz };
            V3 lElbow{ cx - shW * 1.15f, hMin + hTotal * 0.58f - skelDown, cz };
            V3 rElbow{ cx + shW * 1.15f, hMin + hTotal * 0.58f - skelDown, cz };
            V3 lHand{ cx - shW * 1.15f, hMin + hTotal * 0.42f - skelDown, cz };
            V3 rHand{ cx + shW * 1.15f, hMin + hTotal * 0.42f - skelDown, cz };
            V3 lHip{ cx - hipW, hMin + hTotal * 0.46f - skelDown, cz };
            V3 rHip{ cx + hipW, hMin + hTotal * 0.46f - skelDown, cz };
            V3 lKnee{ cx - hipW, hMin + hTotal * 0.24f - skelDown, cz };
            V3 rKnee{ cx + hipW, hMin + hTotal * 0.24f - skelDown, cz };
            V3 lFoot{ cx - hipW, hMin + hTotal * 0.03f - skelDown, cz };
            V3 rFoot{ cx + hipW, hMin + hTotal * 0.03f - skelDown, cz };
            thenwefuckin::data::rect rc{};
            if (thenwefuckin::settings::box_mode == 0)
            {
                ImVec2 topScreen, botScreen;
                if (project_to_preview(headTop, topScreen) && project_to_preview({ cx, hMin - 0.1f, cz }, botScreen))
                {
                    float hh = fabsf(botScreen.y - topScreen.y) + 6.0f;
                    float ww = hh * 0.55f;
                    float midX = (topScreen.x + botScreen.x) * 0.5f;
                    float minY = (std::min)(topScreen.y, botScreen.y) - 3.0f;
                    rc.x0 = std::floor(midX - ww * 0.5f);
                    rc.x1 = std::floor(midX + ww * 0.5f);
                    rc.y0 = std::floor(minY);
                    rc.y1 = std::floor(minY + hh);
                }
            }
            else
            {
                V3 corners[8] = {
                    { s.bbMin.x, s.bbMin.y, s.bbMin.z },
                    { s.bbMax.x, s.bbMin.y, s.bbMin.z },
                    { s.bbMin.x, s.bbMax.y, s.bbMin.z },
                    { s.bbMax.x, s.bbMax.y, s.bbMin.z },
                    { s.bbMin.x, s.bbMin.y, s.bbMax.z },
                    { s.bbMax.x, s.bbMin.y, s.bbMax.z },
                    { s.bbMin.x, s.bbMax.y, s.bbMax.z },
                    { s.bbMax.x, s.bbMax.y, s.bbMax.z }
                };
                float minX = 1e9f, maxX = -1e9f, minY = 1e9f, maxY = -1e9f;
                bool anyOk = false;
                for (int i = 0; i < 8; ++i)
                {
                    ImVec2 pt;
                    if (project_to_preview(corners[i], pt))
                    {
                        anyOk = true;
                        if (pt.x < minX) minX = pt.x;
                        if (pt.x > maxX) maxX = pt.x;
                        if (pt.y < minY) minY = pt.y;
                        if (pt.y > maxY) maxY = pt.y;
                    }
                }
                if (anyOk)
                {
                    rc.x0 = std::floor(minX - 2.0f);
                    rc.x1 = std::ceil(maxX + 2.0f);
                    rc.y0 = std::floor(minY - 2.0f);
                    rc.y1 = std::ceil(maxY + 2.0f);
                }
            }
            if (rc.empty())
            {
                float midX = vpMin.x + vpw * 0.5f;
                float midY = vpMin.y + vph * 0.5f;
                float hh = vph * 0.65f;
                float ww = hh * 0.48f;
                rc.x0 = std::floor(midX - ww * 0.5f);
                rc.x1 = std::floor(midX + ww * 0.5f);
                rc.y0 = std::floor(midY - hh * 0.5f);
                rc.y1 = std::floor(midY + hh * 0.5f);
            }
            ImVec2 headPt{};
            bool hasHead = project_to_preview(headCenter, headPt);
            std::pair<V3, V3> bone3D[] = {
                { headCenter, neck },
                { neck, torso },
                { torso, pelvis },
                { neck, lShoulder },
                { lShoulder, lElbow },
                { lElbow, lHand },
                { neck, rShoulder },
                { rShoulder, rElbow },
                { rElbow, rHand },
                { pelvis, lHip },
                { lHip, lKnee },
                { lKnee, lFoot },
                { pelvis, rHip },
                { rHip, rKnee },
                { rKnee, rFoot }
            };
            static thread_local std::vector<std::pair<ImVec2, ImVec2>> bones2D;
            if (bones2D.capacity() < 15) bones2D.reserve(15);
            bones2D.clear();
            for (const auto& b : bone3D)
            {
                ImVec2 sa, sb;
                if (project_to_preview(b.first, sa) && project_to_preview(b.second, sb))
                    bones2D.emplace_back(sa, sb);
            }
            render_esp_visuals(dl, rc, headPt, bones2D, hasHead, !bones2D.empty());
        }
        else
        {
            float midX = vpMin.x + vpw * 0.5f;
            float midY = vpMin.y + vph * 0.5f;
            float dummyH = std::clamp(vph * 0.62f, 90.0f, 220.0f);
            float dummyW = dummyH * 0.48f;
            thenwefuckin::data::rect rc;
            if (s.thumbSrv)
            {
                float aspect = (s.thumbW > 0 && s.thumbH > 0) ? ((float)s.thumbH / (float)s.thumbW) : 1.0f;
                float imgW = (std::min)(vpw - 8.0f, 220.0f);
                if (imgW < 40.0f) imgW = 40.0f;
                float imgH = imgW * aspect;
                float maxH = vph - 28.0f;
                if (maxH > 40.0f && imgH > maxH) { imgW *= maxH / imgH; imgH = maxH; }
                ImVec2 imgMin{ std::floor(midX - imgW * 0.5f), std::floor(vpMin.y + 4.0f) };
                ImVec2 wpos = ImGui::GetWindowPos();
                ImGui::SetCursorPos(ImVec2(imgMin.x - wpos.x, imgMin.y - wpos.y));
                ImGui::Image((ImTextureID)s.thumbSrv, ImVec2(imgW, imgH));
                dummyW = imgW;
                dummyH = imgH;
                rc.x0 = imgMin.x;
                rc.x1 = imgMin.x + imgW;
                rc.y0 = imgMin.y;
                rc.y1 = imgMin.y + imgH;
            }
            else
            {
                rc.x0 = std::floor(midX - dummyW * 0.5f);
                rc.x1 = std::floor(midX + dummyW * 0.5f);
                rc.y0 = std::floor(midY - dummyH * 0.5f);
                rc.y1 = std::floor(midY + dummyH * 0.5f);
                dl->AddRectFilled(ImVec2(rc.x0, rc.y0), ImVec2(rc.x1, rc.y1), IM_COL32(22, 22, 26, 120));
                dl->AddRect(ImVec2(rc.x0, rc.y0), ImVec2(rc.x1, rc.y1), IM_COL32(50, 50, 60, 160));
            }
            ImVec2 hs{ midX, rc.y0 + dummyH * 0.14f };
            ImVec2 n{ midX, rc.y0 + dummyH * 0.26f };
            ImVec2 t{ midX, rc.y0 + dummyH * 0.42f };
            ImVec2 p{ midX, rc.y0 + dummyH * 0.56f };
            ImVec2 ls{ midX - dummyW * 0.38f, rc.y0 + dummyH * 0.28f };
            ImVec2 rs{ midX + dummyW * 0.38f, rc.y0 + dummyH * 0.28f };
            ImVec2 le{ midX - dummyW * 0.46f, rc.y0 + dummyH * 0.44f };
            ImVec2 re{ midX + dummyW * 0.46f, rc.y0 + dummyH * 0.44f };
            ImVec2 lh{ midX - dummyW * 0.46f, rc.y0 + dummyH * 0.58f };
            ImVec2 rh{ midX + dummyW * 0.46f, rc.y0 + dummyH * 0.58f };
            ImVec2 l_hip{ midX - dummyW * 0.20f, rc.y0 + dummyH * 0.58f };
            ImVec2 r_hip{ midX + dummyW * 0.20f, rc.y0 + dummyH * 0.58f };
            ImVec2 lk{ midX - dummyW * 0.20f, rc.y0 + dummyH * 0.78f };
            ImVec2 rk{ midX + dummyW * 0.20f, rc.y0 + dummyH * 0.78f };
            ImVec2 lf{ midX - dummyW * 0.20f, rc.y1 - 2.0f };
            ImVec2 rf{ midX + dummyW * 0.20f, rc.y1 - 2.0f };
            static thread_local std::vector<std::pair<ImVec2, ImVec2>> dummyBones;
            if (dummyBones.empty())
            {
                dummyBones.reserve(15);
                dummyBones.assign({
                    { hs, n }, { n, t }, { t, p },
                    { n, ls }, { ls, le }, { le, lh },
                    { n, rs }, { rs, re }, { re, rh },
                    { p, l_hip }, { l_hip, lk }, { lk, lf },
                    { p, r_hip }, { r_hip, rk }, { rk, rf }
                });
            }
            else
            {
                dummyBones[0] = { hs, n }; dummyBones[1] = { n, t }; dummyBones[2] = { t, p };
                dummyBones[3] = { n, ls }; dummyBones[4] = { ls, le }; dummyBones[5] = { le, lh };
                dummyBones[6] = { n, rs }; dummyBones[7] = { rs, re }; dummyBones[8] = { re, rh };
                dummyBones[9] = { p, l_hip }; dummyBones[10] = { l_hip, lk }; dummyBones[11] = { lk, lf };
                dummyBones[12] = { p, r_hip }; dummyBones[13] = { r_hip, rk }; dummyBones[14] = { rk, rf };
            }
            render_esp_visuals(dl, rc, hs, dummyBones, true, true);
            ImFont* fs = g_fontSmall ? g_fontSmall : ImGui::GetFont();
            ImVec2 ts = fs->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, s.note.c_str());
            ImVec2 notePos{ floorf(vpMin.x + vpw * 0.5f - ts.x * 0.5f), floorf(vpMax.y - ts.y - 4.0f) };
            Menu::DrawOutlinedText(dl, fs, 11.0f, notePos, IM_COL32(160, 160, 170, 220), IM_COL32(0, 0, 0, 255), s.note.c_str());
        }
    }
    static void render_2d_fallback_with_esp(ImDrawList* dl, ImVec2 min, ImVec2 max, ID3D11ShaderResourceView* srv, int srvW, int srvH, const char* note)
    {

        float vw = floorf(max.x - min.x);
        float vh = floorf(max.y - min.y);
        if (vw < 40.0f || vh < 40.0f)
        {
            if (srv)
                dl->AddImage((ImTextureID)srv, ImVec2(min.x + 2.0f, min.y + 2.0f), ImVec2(max.x - 2.0f, max.y - 2.0f));
            return;
        }
        ImVec2 vpMin{ floorf(min.x + 2.0f), floorf(min.y + 2.0f) };
        ImVec2 vpMax{ floorf(max.x - 2.0f), floorf(max.y - 2.0f) };
        float vpw = vpMax.x - vpMin.x;
        float vph = vpMax.y - vpMin.y;
        float midX = vpMin.x + vpw * 0.5f;
        float midY = vpMin.y + vph * 0.5f;
        float dummyH = std::clamp(vph * 0.62f, 90.0f, 220.0f);
        float dummyW = dummyH * 0.48f;
        thenwefuckin::data::rect rc;
        if (srv)
        {
            float aspect = (srvW > 0 && srvH > 0) ? ((float)srvH / (float)srvW) : 1.0f;
            float imgW = (std::min)(vpw - 8.0f, 220.0f);
            if (imgW < 40.0f) imgW = 40.0f;
            float imgH = imgW * aspect;
            float maxH = vph - 28.0f;
            if (maxH > 40.0f && imgH > maxH) { imgW *= maxH / imgH; imgH = maxH; }
            ImVec2 imgMin{ std::floor(midX - imgW * 0.5f), std::floor(vpMin.y + 4.0f) };
            ImVec2 imgMax{ imgMin.x + imgW, imgMin.y + imgH };
            dl->AddImage((ImTextureID)srv, imgMin, imgMax);
            dummyW = imgW;
            dummyH = imgH;
            rc.x0 = imgMin.x;
            rc.x1 = imgMax.x;
            rc.y0 = imgMin.y;
            rc.y1 = imgMax.y;
        }
        else
        {
            rc.x0 = std::floor(midX - dummyW * 0.5f);
            rc.x1 = std::floor(midX + dummyW * 0.5f);
            rc.y0 = std::floor(midY - dummyH * 0.5f);
            rc.y1 = std::floor(midY + dummyH * 0.5f);
            dl->AddRectFilled(ImVec2(rc.x0, rc.y0), ImVec2(rc.x1, rc.y1), IM_COL32(22, 22, 26, 120));
            dl->AddRect(ImVec2(rc.x0, rc.y0), ImVec2(rc.x1, rc.y1), IM_COL32(50, 50, 60, 160));
        }
        ImVec2 hs{ midX, rc.y0 + dummyH * 0.14f };
        ImVec2 n{ midX, rc.y0 + dummyH * 0.26f };
        ImVec2 t{ midX, rc.y0 + dummyH * 0.42f };
        ImVec2 p{ midX, rc.y0 + dummyH * 0.56f };
        ImVec2 ls{ midX - dummyW * 0.38f, rc.y0 + dummyH * 0.28f };
        ImVec2 rs{ midX + dummyW * 0.38f, rc.y0 + dummyH * 0.28f };
        ImVec2 le{ midX - dummyW * 0.46f, rc.y0 + dummyH * 0.44f };
        ImVec2 re{ midX + dummyW * 0.46f, rc.y0 + dummyH * 0.44f };
        ImVec2 lh{ midX - dummyW * 0.46f, rc.y0 + dummyH * 0.58f };
        ImVec2 rh{ midX + dummyW * 0.46f, rc.y0 + dummyH * 0.58f };
        ImVec2 l_hip{ midX - dummyW * 0.20f, rc.y0 + dummyH * 0.58f };
        ImVec2 r_hip{ midX + dummyW * 0.20f, rc.y0 + dummyH * 0.58f };
        ImVec2 lk{ midX - dummyW * 0.20f, rc.y0 + dummyH * 0.78f };
        ImVec2 rk{ midX + dummyW * 0.20f, rc.y0 + dummyH * 0.78f };
        ImVec2 lf{ midX - dummyW * 0.20f, rc.y1 - 2.0f };
        ImVec2 rf{ midX + dummyW * 0.20f, rc.y1 - 2.0f };
        static thread_local std::vector<std::pair<ImVec2, ImVec2>> dummyBones;
        if (dummyBones.empty())
        {
            dummyBones.reserve(15);
            dummyBones.assign({
                { hs, n }, { n, t }, { t, p },
                { n, ls }, { ls, le }, { le, lh },
                { n, rs }, { rs, re }, { re, rh },
                { p, l_hip }, { l_hip, lk }, { lk, lf },
                { p, r_hip }, { r_hip, rk }, { rk, rf }
            });
        }
        else
        {
            dummyBones[0] = { hs, n }; dummyBones[1] = { n, t }; dummyBones[2] = { t, p };
            dummyBones[3] = { n, ls }; dummyBones[4] = { ls, le }; dummyBones[5] = { le, lh };
            dummyBones[6] = { n, rs }; dummyBones[7] = { rs, re }; dummyBones[8] = { re, rh };
            dummyBones[9] = { p, l_hip }; dummyBones[10] = { l_hip, lk }; dummyBones[11] = { lk, lf };
            dummyBones[12] = { p, r_hip }; dummyBones[13] = { r_hip, rk }; dummyBones[14] = { rk, rf };
        }
        render_esp_visuals(dl, rc, hs, dummyBones, true, true);
        if (note && note[0])
        {
            ImFont* fs = g_fontSmall ? g_fontSmall : ImGui::GetFont();
            ImVec2 ts = fs->CalcTextSizeA(11.0f, FLT_MAX, 0.0f, note);
            ImVec2 notePos{ floorf(vpMin.x + vpw * 0.5f - ts.x * 0.5f), floorf(vpMax.y - ts.y - 4.0f) };
            Menu::DrawOutlinedText(dl, fs, 11.0f, notePos, IM_COL32(160, 160, 170, 220), IM_COL32(0, 0, 0, 255), note);
        }
    }
    void render_for(int64_t uid, ImDrawList* dl, ImVec2 min, ImVec2 max)
    {
        if (uid <= 0)
            return;
        ID3D11Device* dev = Overlay::GetDeviceStatic();
        ID3D11DeviceContext* ctx = Overlay::GetContextStatic();
        if (dev && ctx && !s.gfxReady && !s.gfxFailed)
        {
            if (--s.gfxRetryCd <= 0)
            {
                s.gfxRetryCd = 300;
                if (compile_shaders(dev))
                    s.gfxReady = true;
                else
                {
                    s.gfxFailed = true;
                    s.note = "Preview renderer failed to start.";
                }
            }
        }
        if (dev && ctx && s.gfxReady)
            commit_meshes(dev);
        if (Menu::g_playerPreviewOpen)
        {

            ID3D11ShaderResourceView* srv = players::avatars::avatar_for((long long)uid);
            render_2d_fallback_with_esp(dl, min, max, srv, 1, 1, nullptr);
            return;
        }
        double now = ImGui::GetTime();
        if (uid != s.pulledUid && !s.busy && now - s.lastTry > 1.0)
        {
            s.lastTry = now;
            s.pulledUid = uid;
            char buf[32];
            snprintf(buf, sizeof(buf), "%lld", (long long)uid);
            pull_uid(buf);
        }
        if (s.phase == Phase::Broke && !s.busy && now - s.lastTry > 20.0
            && !s.thumbSrv && !s.thumbPending.load(std::memory_order_acquire))
        {
            s.lastTry = now;
            char buf[32];
            snprintf(buf, sizeof(buf), "%lld", (long long)uid);
            pull_uid(buf);
        }
        bool modelShown = s.gfxReady && s.phase == Phase::Done && !s.meshes.empty() && s.srv;
        if (!modelShown)
        {

            ID3D11ShaderResourceView* srv = nullptr;
            int tw = 1, th = 1;
            const char* note = nullptr;
            std::string ownedNote;
            if (s.pulledUid == uid && s.thumbSrv)
            {
                srv = s.thumbSrv;
                tw = s.thumbW;
                th = s.thumbH;
                note = s.note.c_str();
            }
            else
            {
                srv = players::avatars::avatar_for((long long)uid);
                if (s.pulledUid == uid)
                    note = s.note.c_str();
                else
                    note = "Fetching avatar...";
            }
            render_2d_fallback_with_esp(dl, min, max, srv, tw, th, note);
            return;
        }
        render_backend(dl, min, max);
    }
    void refresh()
    {
        s.pulledUid = 0;
        s.lastTry = 0.0;
    }
}
