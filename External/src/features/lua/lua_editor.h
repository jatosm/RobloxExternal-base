#pragma once
#include "../../menu/imgui/imgui.h"
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <algorithm>

namespace thenwefuckin::lua_editor
{
struct EditorColors
{
    ImU32 bg = 0;
    ImU32 gutter_bg = 0;
    ImU32 line_num = 0;
    ImU32 line_num_active = 0;
    ImU32 cur_line = 0;
    ImU32 sel = 0;
    ImU32 cursor = 0;
    ImU32 text = 0;
    ImU32 keyword = 0;
    ImU32 func = 0;
    ImU32 str = 0;
    ImU32 comment = 0;
    ImU32 number = 0;
    ImU32 outline = IM_COL32(0, 0, 0, 255);
    ImU32 frame_border = 0;
};

enum TokKind { TK_TEXT = 0, TK_KEYWORD = 1, TK_FUNC = 2, TK_STRING = 3, TK_COMMENT = 4, TK_NUMBER = 5 };

inline ImU32 tok_color(const EditorColors& c, int kind)
{
    switch (kind)
    {
    case TK_KEYWORD: return c.keyword;
    case TK_FUNC: return c.func;
    case TK_STRING: return c.str;
    case TK_COMMENT: return c.comment;
    case TK_NUMBER: return c.number;
    default: return c.text;
    }
}

inline void add_outlined(ImDrawList* dl, ImFont* f, float fs, ImVec2 pos, ImU32 col, ImU32 outline, const char* b, const char* e)
{
    if (e > b)
    {
        dl->AddText(f, fs, ImVec2(pos.x + 1.0f, pos.y + 1.0f), outline, b, e);
        dl->AddText(f, fs, pos, col, b, e);
    }
}

inline constexpr size_t kMaxChars = 100000;

inline const char* default_sample()
{
    return "-- thenwefuckin lua -- paste or type below\n"
        "-- comments start with -- and stay green\n"
        "local speed = 16 -- walkspeed default\n"
        "local name = \"player_1\"\n"
        "\n"
        "local function greet(who)\n"
        "    if who == nil then return end\n"
        "    print(\"hello, \" .. who)\n"
        "    for i = 1, 10 do\n"
        "        print(i, 0xFF, 3.14)\n"
        "    end\n"
        "end\n"
        "\n"
        "greet(name)\n";
}

inline std::string& source()
{
    static std::string s = default_sample();
    return s;
}

inline size_t& cursor() { static size_t c = source().size(); return c; }
inline long long& sel_anchor() { static long long a = -1; return a; }
inline bool& focused() { static bool f = false; return f; }
inline bool& focus_request() { static bool f = false; return f; }

inline void clear_selection() { sel_anchor() = -1; }

inline bool has_selection()
{
    long long a = sel_anchor();
    return a >= 0 && (size_t)a != cursor() && a <= (long long)source().size();
}
inline void delete_selection()
{
    if (!has_selection()) return;
    size_t a = (size_t)sel_anchor(), b = cursor();
    if (a > b) std::swap(a, b);
    a = (std::min)(a, source().size());
    b = (std::min)(b, source().size());
    source().erase(a, b - a);
    cursor() = a;
    clear_selection();
}

inline std::vector<std::string> split_lines(const std::string& s)
{
    std::vector<std::string> out;
    out.reserve(64);
    size_t start = 0;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '\n')
        {
            out.emplace_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    out.emplace_back(s.substr(start));
    return out;
}

inline void offset_to_linecol(const std::vector<std::string>& lines, size_t off, int& line, int& col)
{
    size_t acc = 0;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        size_t len = lines[i].size();
        if (off <= acc + len)
        {
            line = (int)i + 1;
            col = (int)(off - acc) + 1;
            return;
        }
        acc += len + 1;
    }
    line = (int)lines.size();
    col = (int)lines.back().size() + 1;
}

inline size_t linecol_to_offset(const std::vector<std::string>& lines, int line, int col)
{
    line = (std::max)(1, (std::min)(line, (int)lines.size()));
    const std::string& l = lines[(size_t)line - 1];
    col = (std::max)(1, (std::min)(col, (int)l.size() + 1));
    size_t off = 0;
    for (int i = 0; i < line - 1; ++i) off += lines[(size_t)i].size() + 1;
    return off + (size_t)(col - 1);
}

inline bool is_word_char(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_'; }

inline bool is_keyword(const std::string& w)
{
    static const char* k[] = {
        "and", "break", "do", "else", "elseif", "end", "false", "for", "function",
        "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return",
        "then", "true", "until", "while", "continue", "type", "self", "export", nullptr
    };
    for (int i = 0; k[i]; ++i)
        if (w == k[i]) return true;
    return false;
}

inline bool is_builtin(const std::string& w)
{
    static const char* k[] = {
        "print", "warn", "require", "select", "pairs", "ipairs", "tostring", "tonumber",
        "pcall", "xpcall", "error", "assert", "type", "typeof", "rawget", "rawset",
        "rawequal", "rawlen", "next", "setmetatable", "getmetatable", "tick", "wait",
        "spawn", "delay", "game", "workspace", "script", "Instance", "Vector2", "Vector3",
        "CFrame", "Color3", "UDim", "UDim2", "Rect", "BrickColor", "TweenInfo", "task",
        "math", "string", "table", "os", "coroutine", "debug", "utf8", "buffer", "bit32",
        "shared", "_G", "_VERSION", nullptr
    };
    for (int i = 0; k[i]; ++i)
        if (w == k[i]) return true;
    return false;
}

inline int long_bracket_eq(const char* p, const char* end, bool dash_dash)
{
    const char* q = p;
    if (dash_dash)
    {
        if (end - q < 2 || q[0] != '-' || q[1] != '-') return -2;
        q += 2;
    }
    if (q >= end || *q != '[') return -2;
    ++q;
    int eq = 0;
    while (q < end && *q == '=') { ++eq; ++q; }
    if (q < end && *q == '[') return eq;
    return -2;
}

inline const char* find_long_close(const char* p, const char* end, int eq)
{
    for (const char* q = p; q < end; ++q)
    {
        if (*q != ']') continue;
        const char* r = q + 1;
        int n = 0;
        while (r < end && *r == '=' && n < eq) { ++r; ++n; }
        if (n == eq && r < end && *r == ']') return r + 1;
    }
    return nullptr;
}

struct Token { const char* b; const char* e; int kind; };

inline void tokenize_line(const char* b, const char* e, int& block_eq, int& str_eq, bool& in_str, std::vector<Token>& out)
{
    out.clear();
    const char* p = b;
    auto push = [&](const char* s, const char* f, int k) {
        if (f > s) out.push_back(Token{ s, f, k });
    };
    if (block_eq >= 0)
    {
        const char* c = find_long_close(p, e, block_eq);
        if (!c)
        {
            push(p, e, TK_COMMENT);
            return;
        }
        push(p, c, TK_COMMENT);
        p = c;
        block_eq = -1;
    }
    else if (in_str)
    {
        const char* c = find_long_close(p, e, str_eq);
        if (!c)
        {
            push(p, e, TK_STRING);
            return;
        }
        push(p, c, TK_STRING);
        p = c;
        in_str = false;
    }
    while (p < e)
    {
        if (p[0] == '-' && p + 1 < e && p[1] == '-')
        {
            int eq = long_bracket_eq(p, e, true);
            if (eq >= 0)
            {
                const char* c = find_long_close(p + 2 + 1 + eq + 1, e, eq);
                if (!c)
                {
                    push(p, e, TK_COMMENT);
                    block_eq = eq;
                    return;
                }
                push(p, c, TK_COMMENT);
                p = c;
                continue;
            }
            push(p, e, TK_COMMENT);
            return;
        }
        if (p[0] == '"' || p[0] == '\'')
        {
            char q = p[0];
            const char* r = p + 1;
            while (r < e && *r != q)
            {
                if (*r == '\\' && r + 1 < e) r += 2;
                else ++r;
            }
            if (r < e) ++r;
            push(p, r, TK_STRING);
            p = r;
            continue;
        }
        if (p[0] == '[')
        {
            int eq = long_bracket_eq(p, e, false);
            if (eq >= 0)
            {
                const char* c = find_long_close(p + 1 + eq + 1, e, eq);
                if (!c)
                {
                    push(p, e, TK_STRING);
                    in_str = true;
                    str_eq = eq;
                    return;
                }
                push(p, c, TK_STRING);
                p = c;
                continue;
            }
        }
        if ((p[0] >= '0' && p[0] <= '9') || (p[0] == '.' && p + 1 < e && p[1] >= '0' && p[1] <= '9'))
        {
            const char* r = p;
            if (*r == '0' && r + 1 < e && (r[1] == 'x' || r[1] == 'X'))
            {
                r += 2;
                while (r < e && ((r[0] >= '0' && r[0] <= '9') || (r[0] >= 'a' && r[0] <= 'f') || (r[0] >= 'A' && r[0] <= 'F') || r[0] == '_')) ++r;
            }
            else
            {
                while (r < e && ((r[0] >= '0' && r[0] <= '9') || r[0] == '_' || r[0] == '.' || r[0] == 'e' || r[0] == 'E' || r[0] == '+' || r[0] == '-'))
                {
                    if ((r[0] == 'e' || r[0] == 'E'))
                    {
                        ++r;
                        if (r < e && (r[0] == '+' || r[0] == '-')) ++r;
                        continue;
                    }
                    if (r[0] == '+' || r[0] == '-') break;
                    ++r;
                }
            }
            push(p, r, TK_NUMBER);
            p = r;
            continue;
        }
        if ((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z') || p[0] == '_')
        {
            const char* r = p + 1;
            while (r < e && is_word_char(*r)) ++r;
            std::string w(p, r);
            const char* look = r;
            while (look < e && (*look == ' ' || *look == '\t')) ++look;
            bool call = (look < e && *look == '(');
            int c = TK_TEXT;
            if (is_keyword(w)) c = TK_KEYWORD;
            else if (is_builtin(w) || call) c = TK_FUNC;
            else if (p > b && *(p - 1) == ':') c = TK_FUNC;
            push(p, r, c);
            p = r;
            continue;
        }
        push(p, p + 1, TK_TEXT);
        ++p;
    }
}

inline size_t word_start(const std::string& s, size_t idx)
{
    idx = (std::min)(idx, s.size());
    while (idx > 0 && is_word_char(s[idx - 1])) --idx;
    return idx;
}

inline size_t word_end(const std::string& s, size_t idx)
{
    idx = (std::min)(idx, s.size());
    while (idx < s.size() && is_word_char(s[idx])) ++idx;
    return idx;
}

struct Snap { std::string text; size_t cur; };
inline std::vector<Snap>& undo_stack() { static std::vector<Snap> v; return v; }
inline std::vector<Snap>& redo_stack() { static std::vector<Snap> v; return v; }
inline bool& typing_burst() { static bool b = false; return b; }
inline void push_undo()
{
    auto& u = undo_stack();
    u.push_back(Snap{ source(), cursor() });
    while (u.size() > 100) u.erase(u.begin());
    redo_stack().clear();
    typing_burst() = false;
}
inline void do_undo()
{
    auto& u = undo_stack();
    if (u.empty()) return;
    redo_stack().push_back(Snap{ source(), cursor() });
    Snap s = u.back(); u.pop_back();
    source() = s.text;
    cursor() = (std::min)(s.cur, source().size());
    clear_selection();
    typing_burst() = false;
}
inline void do_redo()
{
    auto& r = redo_stack();
    if (r.empty()) return;
    auto& u = undo_stack();
    u.push_back(Snap{ source(), cursor() });
    while (u.size() > 100) u.erase(u.begin());
    Snap s = r.back(); r.pop_back();
    source() = s.text;
    cursor() = (std::min)(s.cur, source().size());
    clear_selection();
    typing_burst() = false;
}

inline size_t line_start_of(const std::string& s, size_t idx)
{
    idx = (std::min)(idx, s.size());
    size_t p = idx;
    while (p > 0 && s[p - 1] != '\n') --p;
    return p;
}
inline size_t line_end_of(const std::string& s, size_t idx)
{
    idx = (std::min)(idx, s.size());
    size_t p = idx;
    while (p < s.size() && s[p] != '\n') ++p;
    return p;
}

inline void insert_text(std::string t)
{
    if (t.empty()) return;
    t.erase(std::remove(t.begin(), t.end(), '\r'), t.end());
    if (t.empty()) return;
    std::string& s = source();
    if (s.size() + t.size() > kMaxChars)
        t.resize(kMaxChars - s.size());
    if (t.empty()) return;
    push_undo();
    delete_selection();
    s.insert(cursor(), t);
    cursor() += t.size();
}
inline void clear_all()
{
    if (source().empty()) return;
    push_undo();
    source().clear();
    cursor() = 0;
    clear_selection();
}
inline void load_sample()
{
    push_undo();
    source() = default_sample();
    cursor() = source().size();
    clear_selection();
}
inline void handle_keys(bool can_edit)
{
    if (!focused() || !can_edit) return;
    ImGuiIO& io = ImGui::GetIO();
    std::string& s = source();
    size_t& cur = cursor();

    for (int i = 0; i < io.InputQueueCharacters.Size; ++i)
    {
        unsigned int c = (unsigned int)io.InputQueueCharacters[i];
        if (c == '\t') continue;
        if (c < 32 || c > 126) continue;
        if (s.size() + 1 > kMaxChars) break;
        if (!typing_burst())
        {
            push_undo();
            typing_burst() = true;
        }
        delete_selection();
        s.insert(cur, 1, (char)c);
        ++cur;
    }

    bool ctrl = io.KeyCtrl;
    bool shift = io.KeyShift;
    auto move_to = [&](size_t n, bool extend) {
        n = (std::min)(n, s.size());
        if (extend)
        {
            if (sel_anchor() < 0) sel_anchor() = (long long)cur;
            cur = n;
            if ((size_t)sel_anchor() == cur) clear_selection();
        }
        else
        {
            cur = n;
            clear_selection();
        }
    };

    if (ImGui::IsKeyPressed(ImGuiKey_Tab))
    {
        push_undo();
        if (shift)
        {
            size_t a = has_selection() ? (std::min)((size_t)sel_anchor(), cur) : cur;
            size_t b = has_selection() ? (std::max)((size_t)sel_anchor(), cur) : cur;
            std::vector<std::string> tl = split_lines(s);
            int l0 = 1, c0 = 1, l1 = 1, c1 = 1;
            offset_to_linecol(tl, a, l0, c0);
            offset_to_linecol(tl, b, l1, c1);
            if (c1 == 1 && l1 > l0) --l1;
            size_t removed_before_a = 0, removed_total = 0;
            for (int ln = l1; ln >= l0; --ln)
            {
                size_t ls = 0;
                for (int i = 0; i < ln - 1; ++i) ls += tl[(size_t)i].size() + 1;
                size_t n = 0;
                if (s.compare(ls, 1, "\t") == 0) n = 1;
                else while (n < 4 && ls + n < s.size() && s[ls + n] == ' ') ++n;
                if (n == 0) continue;
                s.erase(ls, n);
                removed_total += n;
                if (ln == l0) removed_before_a = (a >= ls + n) ? n : (a - ls);
            }
            cur = b - removed_total;
            sel_anchor() = (long long)(a - removed_before_a);
            if ((size_t)sel_anchor() == cur) clear_selection();
        }
        else if (has_selection())
        {
            size_t a = (std::min)((size_t)sel_anchor(), cur);
            size_t b = (std::max)((size_t)sel_anchor(), cur);
            std::vector<std::string> tl = split_lines(s);
            int l0 = 1, c0 = 1, l1 = 1, c1 = 1;
            offset_to_linecol(tl, a, l0, c0);
            offset_to_linecol(tl, b, l1, c1);
            if (c1 == 1 && l1 > l0) --l1;
            if (l1 > l0 && s.size() + (size_t)(l1 - l0 + 1) * 4 <= kMaxChars)
            {
                for (int ln = l1; ln >= l0; --ln)
                {
                    size_t ls = 0;
                    for (int i = 0; i < ln - 1; ++i) ls += tl[(size_t)i].size() + 1;
                    s.insert(ls, "    ");
                }
                sel_anchor() = (long long)(a + 4);
                cur = b + (size_t)(l1 - l0 + 1) * 4;
            }
            else
            {
                delete_selection();
                if (s.size() + 4 <= kMaxChars)
                {
                    s.insert(cur, "    ");
                    cur += 4;
                }
            }
        }
        else
        {
            delete_selection();
            if (s.size() + 4 <= kMaxChars)
            {
                s.insert(cur, "    ");
                cur += 4;
            }
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
    {
        push_undo();
        delete_selection();
        size_t ls = line_start_of(s, cur);
        size_t ie = ls;
        while (ie < s.size() && (s[ie] == ' ' || s[ie] == '\t')) ++ie;
        std::string ins = "\n" + s.substr(ls, ie - ls);
        if (s.size() + ins.size() <= kMaxChars)
        {
            s.insert(cur, ins);
            cur += ins.size();
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !ctrl)
    {
        push_undo();
        if (has_selection()) delete_selection();
        else if (cur > 0)
        {
            s.erase(cur - 1, 1);
            --cur;
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Backspace))
    {
        push_undo();
        if (has_selection()) delete_selection();
        else if (cur > 0)
        {
            size_t n = cur;
            while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\n')) --n;
            if (n > 0)
            {
                if (is_word_char(s[n - 1])) n = word_start(s, n);
                else --n;
            }
            s.erase(n, cur - n);
            cur = n;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && !ctrl)
    {
        push_undo();
        if (has_selection()) delete_selection();
        else if (cur < s.size()) s.erase(cur, 1);
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        push_undo();
        if (has_selection()) delete_selection();
        else if (cur < s.size())
        {
            size_t n = cur;
            while (n < s.size() && (s[n] == ' ' || s[n] == '\t' || s[n] == '\n')) ++n;
            if (n < s.size())
            {
                if (is_word_char(s[n])) n = word_end(s, n);
                else ++n;
            }
            s.erase(cur, n - cur);
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
    {
        typing_burst() = false;
        if (ctrl)
        {
            size_t n = cur;
            if (n > 0)
            {
                --n;
                while (n > 0 && !is_word_char(s[n])) --n;
                n = word_start(s, n + 1);
            }
            move_to(n, shift);
        }
        else if (cur > 0) move_to(cur - 1, shift);
        else if (!shift) clear_selection();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
    {
        typing_burst() = false;
        if (ctrl)
        {
            size_t n = word_end(s, cur);
            while (n < s.size() && !is_word_char(s[n])) ++n;
            move_to(n, shift);
        }
        else if (cur < s.size()) move_to(cur + 1, shift);
        else if (!shift) clear_selection();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow) || ImGui::IsKeyPressed(ImGuiKey_DownArrow))
    {
        typing_burst() = false;
        bool up = ImGui::IsKeyPressed(ImGuiKey_UpArrow);
        std::vector<std::string> lines = split_lines(s);
        int line = 1, col = 1;
        offset_to_linecol(lines, cur, line, col);
        size_t want = 0;
        {
            size_t acc = 0;
            for (int i = 0; i < line - 1; ++i) acc += lines[(size_t)i].size() + 1;
            want = cur - acc;
        }
        int nl = up ? line - 1 : line + 1;
        nl = (std::max)(1, (std::min)(nl, (int)lines.size()));
        size_t noff = 0;
        for (int i = 0; i < nl - 1; ++i) noff += lines[(size_t)i].size() + 1;
        move_to(noff + (std::min)(want, lines[(size_t)nl - 1].size()), shift);
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Home))
    {
        typing_burst() = false;
        if (ctrl) move_to(0, shift);
        else
        {
            std::vector<std::string> lines = split_lines(s);
            int line = 1, col = 1;
            offset_to_linecol(lines, cur, line, col);
            size_t ls = line_start_of(s, cur);
            size_t nb = ls;
            while (nb < s.size() && (s[nb] == ' ' || s[nb] == '\t')) ++nb;
            move_to(cur == nb ? ls : nb, shift);
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_End))
    {
        typing_burst() = false;
        if (ctrl) move_to(s.size(), shift);
        else
        {
            std::vector<std::string> lines = split_lines(s);
            int line = 1, col = 1;
            offset_to_linecol(lines, cur, line, col);
            move_to(linecol_to_offset(lines, line, (int)lines[(size_t)line - 1].size() + 1), shift);
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Z) && !shift) do_undo();
    if ((ctrl && ImGui::IsKeyPressed(ImGuiKey_Y)) || (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_Z))) do_redo();
    if (ctrl && shift && ImGui::IsKeyPressed(ImGuiKey_K))
    {
        push_undo();
        size_t a = has_selection() ? (std::min)((size_t)sel_anchor(), cur) : cur;
        size_t b = has_selection() ? (std::max)((size_t)sel_anchor(), cur) : cur;
        size_t ls = line_start_of(s, a);
        size_t le = line_end_of(s, b);
        if (le < s.size() && s[le] == '\n') ++le;
        else if (ls > 0 && s[ls - 1] == '\n') --ls;
        s.erase(ls, le - ls);
        cur = (std::min)(ls, s.size());
        clear_selection();
    }
    if (ctrl && !shift && ImGui::IsKeyPressed(ImGuiKey_D))
    {
        size_t a = has_selection() ? (std::min)((size_t)sel_anchor(), cur) : cur;
        size_t b = has_selection() ? (std::max)((size_t)sel_anchor(), cur) : cur;
        if (has_selection() && b > a)
        {
            std::string dup = s.substr(a, b - a);
            if (s.size() + dup.size() <= kMaxChars)
            {
                push_undo();
                s.insert(b, dup);
                sel_anchor() = (long long)b;
                cur = b + dup.size();
            }
        }
        else
        {
            size_t ls = line_start_of(s, cur);
            size_t le = line_end_of(s, cur);
            std::string dup;
            size_t at = 0;
            if (le < s.size() && s[le] == '\n') { dup = s.substr(ls, le - ls + 1); at = le + 1; }
            else { dup = "\n" + s.substr(ls, le - ls); at = le; }
            if (s.size() + dup.size() <= kMaxChars)
            {
                push_undo();
                s.insert(at, dup);
                cur = (std::min)(cur + dup.size(), s.size());
                clear_selection();
            }
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_Slash))
    {
        size_t a = has_selection() ? (std::min)((size_t)sel_anchor(), cur) : cur;
        size_t b = has_selection() ? (std::max)((size_t)sel_anchor(), cur) : cur;
        std::vector<std::string> tl = split_lines(s);
        int l0 = 1, c0 = 1, l1 = 1, c1 = 1;
        offset_to_linecol(tl, a, l0, c0);
        offset_to_linecol(tl, b, l1, c1);
        if (c1 == 1 && l1 > l0) --l1;
        bool all_commented = true;
        for (int ln = l0; ln <= l1; ++ln)
        {
            const std::string& t = tl[(size_t)ln - 1];
            size_t p = 0;
            while (p < t.size() && (t[p] == ' ' || t[p] == '\t')) ++p;
            if (p >= t.size() || t.compare(p, 2, "--") != 0) { all_commented = false; break; }
        }
        push_undo();
        for (int ln = l1; ln >= l0; --ln)
        {
            size_t ls = 0;
            for (int i = 0; i < ln - 1; ++i) ls += tl[(size_t)i].size() + 1;
            if (all_commented)
            {
                size_t p = ls;
                while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) ++p;
                if (p + 1 < s.size() && s[p] == '-' && s[p + 1] == '-')
                {
                    size_t n = 2;
                    if (p + 2 < s.size() && s[p + 2] == ' ') n = 3;
                    s.erase(p, n);
                }
            }
            else
            {
                size_t p = ls;
                while (p < s.size() && s[p] != '\n' && (s[p] == ' ' || s[p] == '\t')) ++p;
                if (p < s.size() && s[p] != '\n' && s.size() + 3 <= kMaxChars)
                    s.insert(p, "-- ");
            }
        }
        cur = line_start_of(s, cur);
        clear_selection();
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_A))
    {
        sel_anchor() = 0;
        cur = s.size();
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_C))
    {
        if (has_selection())
        {
            size_t a = (size_t)sel_anchor(), b = cur;
            if (a > b) std::swap(a, b);
            ImGui::SetClipboardText(s.substr(a, b - a).c_str());
        }
        else ImGui::SetClipboardText(s.c_str());
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_X))
    {
        if (has_selection())
        {
            size_t a = (size_t)sel_anchor(), b = cur;
            if (a > b) std::swap(a, b);
            ImGui::SetClipboardText(s.substr(a, b - a).c_str());
            push_undo();
            delete_selection();
        }
    }
    if (ctrl && ImGui::IsKeyPressed(ImGuiKey_V))
    {
        const char* clip = ImGui::GetClipboardText();
        if (clip && clip[0])
        {
            std::string t = clip;
            std::string n;
            n.reserve(t.size());
            for (char c : t)
            {
                if (c == '\r') continue;
                n.push_back(c);
            }
            if (s.size() + n.size() > kMaxChars) n.resize(kMaxChars - s.size());
            push_undo();
            delete_selection();
            s.insert(cur, n);
            cur += n.size();
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        focused() = false;
        clear_selection();
    }
}

inline size_t offset_from_mouse(ImFont* f, float fs, const std::vector<std::string>& lines, ImVec2 code_origin, float line_h, ImVec2 m)
{
    int line = (int)((m.y - code_origin.y) / line_h) + 1;
    line = (std::max)(1, (std::min)(line, (int)lines.size()));
    const std::string& t = lines[(size_t)line - 1];
    float x = m.x - code_origin.x;
    if (x <= 0) return linecol_to_offset(lines, line, 1);
    size_t lo = 0, hi = t.size();
    while (lo < hi)
    {
        size_t mid = (lo + hi + 1) / 2;
        float w = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, t.c_str(), t.c_str() + mid).x;
        if (w <= x) lo = mid;
        else hi = mid - 1;
    }
    if (lo < t.size())
    {
        float w0 = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, t.c_str(), t.c_str() + lo).x;
        float w1 = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, t.c_str(), t.c_str() + lo + 1).x;
        if (x > (w0 + w1) * 0.5f) ++lo;
    }
    return linecol_to_offset(lines, line, (int)lo + 1);
}

inline void render_editor_child(ImVec2 size, ImFont* f, float fs, bool can_edit, const EditorColors& c)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::BeginChild("##lua_code_scroll", size, false,
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 view_min = ImGui::GetWindowPos();
    ImVec2 view_sz = ImGui::GetWindowSize();
    ImVec2 view_max(view_min.x + view_sz.x, view_min.y + view_sz.y);

    ImGuiIO& io = ImGui::GetIO();
    if (focus_request())
    {
        focus_request() = false;
        focused() = true;
    }
    if (!can_edit) focused() = false;

    std::string& s = source();
    size_t& cur = cursor();
    cur = (std::min)(cur, s.size());
    if (sel_anchor() >= 0) sel_anchor() = (long long)(std::min)((size_t)sel_anchor(), s.size());

    std::vector<std::string> lines = split_lines(s);
    float line_h = floorf(fs + 4.0f);
    float pad = 4.0f;

    int digits = 1;
    for (int n = (int)lines.size(); n >= 10; n /= 10) ++digits;
    float digit_w = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, "8").x;
    float gutter_w = floorf((float)digits * digit_w + 12.0f);

    float max_w = 0.0f;
    for (auto& l : lines)
        max_w = (std::max)(max_w, f->CalcTextSizeA(fs, FLT_MAX, 0.0f, l.c_str()).x);
    ImVec2 content(floorf(gutter_w + pad + max_w + pad + 24.0f), floorf((float)lines.size() * line_h + pad * 2.0f));
    content.x = (std::max)(content.x, size.x);
    content.y = (std::max)(content.y, size.y);

    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##lua_edit_hit", content);
    bool hov = ImGui::IsItemHovered();
    ImVec2 box_min = view_min;
    ImVec2 box_max = view_max;
    if (hov) ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
    ImVec2 code0(origin.x + gutter_w + pad, origin.y + pad);
    if (hov && ImGui::IsMouseClicked(0) && can_edit)
    {
        focused() = true;
        typing_burst() = false;
        size_t off = offset_from_mouse(f, fs, lines, code0, line_h, io.MousePos);
        cur = off;
        sel_anchor() = (long long)off;
        if (ImGui::IsMouseDoubleClicked(0))
        {
            size_t a = word_start(s, off), b = word_end(s, off);
            sel_anchor() = (long long)a;
            cur = b;
        }
    }
    if (focused() && ImGui::IsMouseClicked(0) && !hov) focused() = false;
    if (focused() && ImGui::IsMouseDragging(0) && can_edit)
        cur = offset_from_mouse(f, fs, lines, code0, line_h, io.MousePos);
    if (!focused() && sel_anchor() >= 0 && !ImGui::IsMouseDown(0)) clear_selection();
    if (has_selection() && !focused()) clear_selection();

    handle_keys(can_edit && focused());

    lines = split_lines(s);
    cur = (std::min)(cur, s.size());

    dl->PushClipRect(box_min, box_max, true);
    dl->AddRectFilled(origin, ImVec2(origin.x + content.x, origin.y + content.y), c.bg);
    dl->AddRectFilled(origin, ImVec2(origin.x + gutter_w, origin.y + content.y), c.gutter_bg);

    int cur_line = 1, cur_col = 1;
    offset_to_linecol(lines, cur, cur_line, cur_col);

    size_t sel_a = cur, sel_b = cur;
    if (has_selection())
    {
        sel_a = (size_t)sel_anchor();
        sel_b = cur;
        if (sel_a > sel_b) std::swap(sel_a, sel_b);
    }

    size_t acc = 0;
    int block_eq = -1;
    bool in_str = false;
    int str_eq = 0;
    std::vector<Token> toks;
    toks.reserve(32);

    for (size_t li = 0; li < lines.size(); ++li)
    {
        float y = origin.y + pad + (float)li * line_h;
        if (y + line_h < box_min.y || y > box_max.y)
        {
            acc += lines[li].size() + 1;
            int be = block_eq;
            bool is = in_str;
            int se = str_eq;
            tokenize_line(lines[li].c_str(), lines[li].c_str() + lines[li].size(), be, se, is, toks);
            block_eq = be;
            in_str = is;
            str_eq = se;
            continue;
        }
        if ((int)li + 1 == cur_line && focused())
            dl->AddRectFilled(ImVec2(origin.x + gutter_w, floorf(y)), ImVec2(origin.x + content.x, floorf(y + line_h)), c.cur_line);

        char nb[16];
        snprintf(nb, sizeof(nb), "%d", (int)li + 1);
        ImVec2 nws = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, nb);
        add_outlined(dl, f, fs, ImVec2(floorf(origin.x + gutter_w - nws.x - 6.0f), floorf(y + (line_h - fs) * 0.5f)),
            ((int)li + 1 == cur_line) ? c.line_num_active : c.line_num, c.outline, nb, nb + strlen(nb));

        size_t loff = acc;
        size_t lend = loff + lines[li].size();
        if (has_selection() && sel_b > loff && sel_a < lend)
        {
            size_t a = (std::max)(sel_a, loff) - loff;
            size_t b = (std::min)(sel_b, lend) - loff;
            float xa = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, lines[li].c_str(), lines[li].c_str() + a).x;
            float xb = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, lines[li].c_str(), lines[li].c_str() + b).x;
            dl->AddRectFilled(
                ImVec2(origin.x + gutter_w + pad + xa, floorf(y)),
                ImVec2(origin.x + gutter_w + pad + xb, floorf(y + line_h)), c.sel);
        }

        const std::string& lt = lines[li];
        tokenize_line(lt.c_str(), lt.c_str() + lt.size(), block_eq, str_eq, in_str, toks);
        float ty = floorf(y + (line_h - fs) * 0.5f);
        for (auto& t : toks)
        {
            float xa = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, lt.c_str(), t.b).x;
            add_outlined(dl, f, fs, ImVec2(origin.x + gutter_w + pad + xa, ty), tok_color(c, t.kind), c.outline, t.b, t.e);
        }
        acc = lend + 1;
    }

    if (focused() && can_edit)
    {
        double t = ImGui::GetTime();
        if (fmod(t, 1.0) < 0.55)
        {
            float y = origin.y + pad + (float)(cur_line - 1) * line_h;
            const std::string& lt = lines[(size_t)cur_line - 1];
            size_t ci = (std::min)((size_t)(cur_col - 1), lt.size());
            float xa = f->CalcTextSizeA(fs, FLT_MAX, 0.0f, lt.c_str(), lt.c_str() + ci).x;
            dl->AddLine(
                ImVec2(origin.x + gutter_w + pad + xa, floorf(y + 1.0f)),
                ImVec2(origin.x + gutter_w + pad + xa, floorf(y + line_h - 1.0f)), c.cursor, 1.0f);
        }
    }

    dl->PopClipRect();
    dl->AddRect(view_min, view_max, c.outline);
    dl->AddRect(ImVec2(view_min.x + 1.0f, view_min.y + 1.0f), ImVec2(view_max.x - 1.0f, view_max.y - 1.0f), c.frame_border);
    ImGui::EndChild();
    ImGui::PopStyleVar(1);
}
}
