#include "scan.h"
#include "../../../../utility/struct/offsets.h"
#include "../../../../utility/logger/logger.h"
#include <cmath>
#include <utility>
#include <vector>
namespace thenwefuckin::world
{
    bool match_node(const std::string& n, bool& plant, bool& bots, bool& animal, bool& tool, bool& crate, bool& keycard, int& idx)
    {
        plant = false;
        bots = false;
        animal = false;
        tool = false;
        crate = false;
        keycard = false;
        idx = -1;
        for (int i = 0; i < 3; ++i)
        {
            if (n != kOreNames[i])
                continue;
            bool on = (i == 0) ? settings::world_ore_stone
                : (i == 1) ? settings::world_ore_phosphate
                : settings::world_ore_metal;
            if (!on)
                return false;
            idx = i;
            return true;
        }
        for (int i = 0; i < 7; ++i)
        {
            if (n != kPlantNames[i])
                continue;
            if (!settings::world_plant_sel[i])
                return false;
            plant = true;
            idx = i;
            return true;
        }
        if (settings::world_bots_enabled || settings::aim_bots)
        {
            for (int i = 0; i < 4; ++i)
            {
                bool hit = false;
                if (i == 3)
                {

                    if (n.find("Heli") != std::string::npos &&
                        n.find("Rocket") == std::string::npos &&
                        n.find("Crate") == std::string::npos)
                        hit = true;
                    else if (n == kBotsNames[i])
                        hit = true;
                }
                else if (n == kBotsNames[i])
                    hit = true;
                if (!hit)
                    continue;
                if (!settings::world_bots_sel[i] && !settings::aim_bots)
                    return false;
                bots = true;
                idx = i;
                return true;
            }
        }
        if (settings::world_animals_enabled)
        {
            for (int i = 0; i < 3; ++i)
            {
                if (n != kAnimalNames[i])
                    continue;
                if (!settings::world_animals_sel[i])
                    return false;
                animal = true;
                idx = i;
                return true;
            }
        }
        if (settings::world_tools_enabled)
        {
            for (int i = 0; i < 7; ++i)
            {
                if (n != kToolNames[i])
                    continue;
                if (!settings::world_tools_sel[i])
                    return false;
                tool = true;
                idx = i;
                return true;
            }
        }
        if (settings::world_crates_enabled)
        {
            for (int i = 0; i < 8; ++i)
            {
                if (n != kCrateNames[i])
                    continue;
                if (!settings::world_crates_sel[i])
                    return false;
                crate = true;
                idx = i;
                return true;
            }
        }
        if (settings::world_keycards_enabled)
        {
            if (n != "Keycards")
            {
                for (int i = 0; i < 5; ++i)
                {
                    if (n.find(kKeycardNames[i]) == std::string::npos)
                        continue;
                    if (!settings::world_keycards_sel[i])
                        return false;
                    keycard = true;
                    idx = i;
                    return true;
                }
            }
        }
        return false;
    }
    math::vec3 instance_pos(node inst);
    math::vec3 keycard_pos(node inst)
    {
        if (!inst)
            return {};
        std::vector<std::pair<node, int>> st;
        st.emplace_back(inst, 0);
        int vis = 0;
        while (!st.empty() && vis < 60)
        {
            auto [nd, depth] = st.back();
            st.pop_back();
            ++vis;
            if (nd.name() == "HumanoidRootPart" &&
                (nd.class_name() == "Part" || nd.class_name() == "MeshPart"))
            {
                math::vec3 p = instance_pos(nd);
                if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                    return p;
                return {};
            }
            if (depth < 3)
            {
                for (auto& c : nd.children())
                    st.emplace_back(c, depth + 1);
            }
        }
        return {};
    }
    math::vec3 instance_pos(node inst)
    {
        if (!inst)
            return {};
        uintptr_t prim = inst.primitive();
        if (prim)
        {
            math::vec3 p = g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
            if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                return p;
        }
        uintptr_t pp = g_proc->read<uintptr_t>(inst.addr + Offsets::Model::PrimaryPart);
        if (pp)
        {
            uintptr_t pprim = g_proc->read<uintptr_t>(pp + Offsets::BasePart::Primitive);
            if (pprim)
            {
                math::vec3 p = g_proc->read<math::vec3>(pprim + Offsets::Primitive::Position);
                if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                    return p;
            }
        }
        for (auto& c : inst.children())
        {
            uintptr_t cp = c.primitive();
            if (cp)
            {
                math::vec3 p = g_proc->read<math::vec3>(cp + Offsets::Primitive::Position);
                if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                    return p;
            }
            for (auto& cc : c.children())
            {
                uintptr_t ccp = cc.primitive();
                if (!ccp)
                    continue;
                math::vec3 p = g_proc->read<math::vec3>(ccp + Offsets::Primitive::Position);
                if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                    return p;
            }
        }
        return {};
    }
    math::vec3 local_anchor()
    {
        if (g_ctx.local_root)
        {
            math::vec3 p = g_ctx.local_root.position();
            if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                return p;
        }
        if (g_ctx.camera)
            return g_proc->read<math::vec3>(g_ctx.camera.addr + Offsets::Camera::Position);
        return {};
    }
    math::vec3 part_pos(std::uintptr_t part)
    {
        if (!part)
            return {};
        uintptr_t prim = g_proc->read<std::uintptr_t>(part + Offsets::BasePart::Primitive);
        if (!prim)
            return {};
        return g_proc->read<math::vec3>(prim + Offsets::Primitive::Position);
    }
    void fast_refresh()
    {
        math::vec3 lp = local_anchor();
        for (auto it = g_entries.begin(); it != g_entries.end();)
        {
            std::uintptr_t par = g_proc->read<std::uintptr_t>(it->inst + Offsets::Instance::Parent);
            if (!par)
            {
                if (++it->zeroStreak >= 5)
                {
                    it = g_entries.erase(it);
                    continue;
                }
                ++it;
                continue;
            }
            bool useParts = (it->bots || it->animal) && it->rootPrim != 0;
            math::vec3 rp{};
            math::vec3 hp{};
            if (useParts)
            {
                if (it->rootPrim)
                    rp = g_proc->read<math::vec3>(it->rootPrim + Offsets::Primitive::Position);
                if (it->headPrim)
                    hp = g_proc->read<math::vec3>(it->headPrim + Offsets::Primitive::Position);
                bool rZero = (rp.x == 0.0f && rp.y == 0.0f && rp.z == 0.0f);
                bool hZero = (hp.x == 0.0f && hp.y == 0.0f && hp.z == 0.0f);
                if (rZero && hZero)
                {
                    if (++it->zeroStreak >= 5)
                    {
                        it = g_entries.erase(it);
                        continue;
                    }
                    ++it;
                    continue;
                }
                it->zeroStreak = 0;
                if (!hZero)
                    it->headPos = hp;
                if (!rZero)
                {
                    it->rootPos = rp;
                    it->pos = rp;
                    it->dist = (rp - lp).length();
                }
                else if (!hZero)
                {
                    it->pos = hp;
                    it->dist = (hp - lp).length();
                }
            }
            else
            {
                math::vec3 p = instance_pos(node{ it->inst });
                bool zero = (p.x == 0.0f && p.y == 0.0f && p.z == 0.0f);
                if (zero)
                {
                    if (++it->zeroStreak >= 5)
                    {
                        it = g_entries.erase(it);
                        continue;
                    }
                    ++it;
                    continue;
                }
                it->zeroStreak = 0;
                it->pos = p;
                it->dist = (p - lp).length();
                if (it->bots || it->animal)
                {
                    math::vec3 hp2 = part_pos(it->headPart);
                    math::vec3 rp2 = part_pos(it->rootPart);
                    if (!(hp2.x == 0.0f && hp2.y == 0.0f && hp2.z == 0.0f))
                        it->headPos = hp2;
                    if (!(rp2.x == 0.0f && rp2.y == 0.0f && rp2.z == 0.0f))
                    {
                        it->rootPos = rp2;
                        it->pos = rp2;
                        it->dist = (rp2 - lp).length();
                    }
                }
            }
            if ((it->animal || it->bots) && it->humanoid)
            {
                float ahp = g_proc->read<float>(it->humanoid + Offsets::Humanoid::Health);
                float amx = g_proc->read<float>(it->humanoid + Offsets::Humanoid::MaxHealth);
                if (std::isfinite(ahp) && ahp > 0.0f)
                {
                    it->hp = ahp;
                    it->hpValid = true;
                    if (std::isfinite(amx) && amx > 0.0f)
                        it->maxHp = amx;
                }
                else if (it->animal || (it->bots && std::isfinite(ahp)))
                {
                    it = g_entries.erase(it);
                    continue;
                }
            }
            else if (it->bots && it->hpValue)
            {
                float vhp = 0.0f;
                if (it->hpValueIsInt)
                    vhp = (float)g_proc->read<int32_t>(it->hpValue + Offsets::Value::Value);
                else
                    vhp = (float)g_proc->read<double>(it->hpValue + Offsets::Value::Value);
                if (std::isfinite(vhp) && vhp >= 0.0f && vhp < 1000000.0f)
                {
                    it->hp = vhp;
                    it->hpValid = true;
                    if (it->maxHpValue)
                    {
                        float vmx = 0.0f;
                        if (it->maxHpValueIsInt)
                            vmx = (float)g_proc->read<int32_t>(it->maxHpValue + Offsets::Value::Value);
                        else
                            vmx = (float)g_proc->read<double>(it->maxHpValue + Offsets::Value::Value);
                        if (std::isfinite(vmx) && vmx > 0.0f && vmx < 1000000.0f)
                            it->maxHp = vmx;
                    }
                }
            }
            ++it;
        }
    }
    void full_scan()
    {
        if (!g_ctx.workspace || !g_proc || !g_proc->connected())
        {
            g_scanning = false;
            return;
        }
        if (!g_scanning || g_ctx.workspace.addr != g_scanWs)
        {
            while (!g_scanQ.empty())
                g_scanQ.pop();
            g_scanOut.clear();
            g_scanOut.reserve(256);
            g_scanQ.push(g_ctx.workspace.addr);
            g_scanLp = local_anchor();
            g_scanWs = g_ctx.workspace.addr;
            g_scanned = 0;
            g_scanning = true;
            g_scanLocalChar = 0;
            if (g_ctx.local_player)
            {
                node lc = g_ctx.get_character(g_ctx.local_player);
                if (lc)
                    g_scanLocalChar = lc.addr;
            }
        }
        math::vec3 lp = g_scanLp;
        unsigned long long t0 = GetTickCount64();
        std::size_t n = 0;
        while (!g_scanQ.empty() && g_scanned < kMaxScan)
        {
            if (n >= kSliceNodes)
                break;
            if (n > 0 && GetTickCount64() - t0 >= 6)
                break;
            node cur{ g_scanQ.front() };
            g_scanQ.pop();
            ++g_scanned;
            ++n;
            if (!cur)
                continue;
            std::string cls = cur.class_name();
            bool isFolderCls = (cls == "Folder");
            bool isModelCls = (cls == "Model");
            bool isPartCls = (cls == "MeshPart" || cls == "Part");
            bool isWsCls = (cls == "Workspace");
            if (!isFolderCls && !isModelCls && !isPartCls && !isWsCls)
                continue;
            std::string nm = cur.name();
            bool isPlant = false;
            bool isBots = false;
            bool isAnimal = false;
            bool isTool = false;
            bool isCrate = false;
            bool isKeycard = false;
            int idx = -1;
            if (cls == "Folder" && nm != "Keycards" && settings::world_keycards_enabled)
            {
                for (int i = 0; i < 5; ++i)
                {
                    if (nm.find(kKeycardNames[i]) == std::string::npos)
                        continue;
                    if (!settings::world_keycards_sel[i])
                        break;
                    math::vec3 kp = keycard_pos(cur);
                    if (kp.x == 0.0f && kp.y == 0.0f && kp.z == 0.0f)
                        break;
                    float kd = (kp - lp).length();
                    if (kd < kMaxDist)
                    {
                        Entry e{};
                        e.name = nm;
                        e.pos = kp;
                        e.dist = kd;
                        e.idx = i;
                        e.keycard = true;
                        e.inst = cur.addr;
                        g_scanOut.push_back(std::move(e));
                    }
                    break;
                }
            }
            if ((cls == "Model" || cls == "MeshPart" || cls == "Part") && match_node(nm, isPlant, isBots, isAnimal, isTool, isCrate, isKeycard, idx))
            {

                if (g_scanLocalChar != 0)
                {
                    if (cur.addr == g_scanLocalChar)
                        continue;
                    std::uintptr_t mpar = g_proc->read<std::uintptr_t>(cur.addr + Offsets::Instance::Parent);
                    if (mpar == g_scanLocalChar)
                        continue;
                }
                math::vec3 p = instance_pos(cur);
                if (!(p.x == 0.0f && p.y == 0.0f && p.z == 0.0f))
                {
                    float d = (p - lp).length();
                    if (d < kMaxDist)
                    {
                        Entry e{};
                        e.name = isPlant ? kPlantNames[idx] : (isBots ? kBotsDisplay[idx] : (isAnimal ? kAnimalDisplay[idx] : (isTool ? kToolNames[idx] : (isCrate ? kCrateNames[idx] : (isKeycard ? nm : kOreDisplay[idx])))));
                        e.pos = p;
                        e.dist = d;
                        e.idx = idx;
                        e.plant = isPlant;
                        e.bots = isBots;
                        e.animal = isAnimal;
                        e.tool = isTool;
                        e.crate = isCrate;
                        e.keycard = isKeycard;
                        e.inst = cur.addr;
                        if (isBots || isAnimal)
                        {
                            std::uintptr_t headAddr = 0, hrpAddr = 0, torsoAddr = 0;
                            std::uintptr_t armLAddr = 0, armRAddr = 0, legLAddr = 0, legRAddr = 0;
                            std::uintptr_t humAddr = 0;
                            std::uintptr_t hpValAddr = 0, maxHpValAddr = 0;
                            bool hpIsInt = false, maxHpIsInt = false;
                            bool isR6guess = true;
                            for (auto& ch : cur.children())
                            {
                                std::string cc = ch.class_name();
                                bool isPart = (cc == "Part" || cc == "MeshPart");
                                bool isHpVal = (cc == "NumberValue" || cc == "IntValue");
                                if (cc != "Humanoid" && !isPart && !isHpVal)
                                    continue;
                                std::string cn = ch.name();
                                if (cc == "Humanoid" && !humAddr)
                                {
                                    humAddr = ch.addr;
                                    continue;
                                }
                                if (isHpVal)
                                {
                                    if (!hpValAddr && (cn == "Health" || cn == "HP" || cn == "HitPoints" || cn == "CurrentHealth"))
                                    {
                                        hpValAddr = ch.addr;
                                        hpIsInt = (cc == "IntValue");
                                    }
                                    else if (!maxHpValAddr && (cn == "MaxHealth" || cn == "MaxHP" || cn == "MaxHitPoints" || cn == "MaxHitPointsValue"))
                                    {
                                        maxHpValAddr = ch.addr;
                                        maxHpIsInt = (cc == "IntValue");
                                    }
                                    continue;
                                }
                                if (!isPart)
                                    continue;
                                if (cn == "Head" && !headAddr) headAddr = ch.addr;
                                else if (cn == "HumanoidRootPart" && !hrpAddr) hrpAddr = ch.addr;
                                else if (cn == "Torso" && !torsoAddr) { torsoAddr = ch.addr; }
                                else if (cn == "UpperTorso" && !torsoAddr) { torsoAddr = ch.addr; isR6guess = false; }
                                else if (cn == "LowerTorso") { isR6guess = false; }
                                else if ((cn == "Left Arm" || cn == "LeftArm" || cn == "LeftUpperArm" || cn == "LeftLowerArm" || cn == "LeftHand") && !armLAddr) armLAddr = ch.addr;
                                else if ((cn == "Right Arm" || cn == "RightArm" || cn == "RightUpperArm" || cn == "RightLowerArm" || cn == "RightHand") && !armRAddr) armRAddr = ch.addr;
                                else if ((cn == "Left Leg" || cn == "LeftLeg" || cn == "LeftUpperLeg" || cn == "LeftLowerLeg" || cn == "LeftFoot") && !legLAddr) legLAddr = ch.addr;
                                else if ((cn == "Right Leg" || cn == "RightLeg" || cn == "RightUpperLeg" || cn == "RightLowerLeg" || cn == "RightFoot") && !legRAddr) legRAddr = ch.addr;
                                if (headAddr && hrpAddr && torsoAddr && armLAddr && armRAddr && legLAddr && legRAddr && humAddr)
                                    break;
                            }
                            e.headPart = headAddr;
                            e.rootPart = hrpAddr;
                            if (headAddr)
                                e.headPrim = g_proc->read<std::uintptr_t>(headAddr + Offsets::BasePart::Primitive);
                            if (hrpAddr)
                                e.rootPrim = g_proc->read<std::uintptr_t>(hrpAddr + Offsets::BasePart::Primitive);
                            if (torsoAddr)
                                e.torsoPrim = g_proc->read<std::uintptr_t>(torsoAddr + Offsets::BasePart::Primitive);
                            if (armLAddr)
                                e.armLPrim = g_proc->read<std::uintptr_t>(armLAddr + Offsets::BasePart::Primitive);
                            if (armRAddr)
                                e.armRPrim = g_proc->read<std::uintptr_t>(armRAddr + Offsets::BasePart::Primitive);
                            if (legLAddr)
                                e.legLPrim = g_proc->read<std::uintptr_t>(legLAddr + Offsets::BasePart::Primitive);
                            if (legRAddr)
                                e.legRPrim = g_proc->read<std::uintptr_t>(legRAddr + Offsets::BasePart::Primitive);
                            e.npcR6 = isR6guess;
                            if (isBots && !e.rootPart)
                                continue;
                            math::vec3 hp{};
                            math::vec3 rp{};
                            if (e.headPrim)
                                hp = g_proc->read<math::vec3>(e.headPrim + Offsets::Primitive::Position);
                            else
                                hp = part_pos(e.headPart);
                            if (e.rootPrim)
                                rp = g_proc->read<math::vec3>(e.rootPrim + Offsets::Primitive::Position);
                            else
                                rp = part_pos(e.rootPart);
                            if (!(hp.x == 0.0f && hp.y == 0.0f && hp.z == 0.0f))
                                e.headPos = hp;
                            if (!(rp.x == 0.0f && rp.y == 0.0f && rp.z == 0.0f))
                            {
                                e.rootPos = rp;
                                e.pos = rp;
                                e.dist = (rp - lp).length();
                            }
                            else if (isBots)
                                continue;
                            if (humAddr)
                            {
                                e.humanoid = humAddr;
                                float bhp = g_proc->read<float>(humAddr + Offsets::Humanoid::Health);
                                float bmx = g_proc->read<float>(humAddr + Offsets::Humanoid::MaxHealth);
                                if (std::isfinite(bhp) && bhp > 0.0f)
                                {
                                    e.hp = bhp;
                                    e.maxHp = (std::isfinite(bmx) && bmx > 0.0f) ? bmx : 100.0f;
                                    e.hpValid = true;
                                }
                                else if (isBots && std::isfinite(bhp))
                                {
                                    continue;
                                }
                            }
                            if (!e.humanoid && hpValAddr)
                            {
                                e.hpValue = hpValAddr;
                                e.hpValueIsInt = hpIsInt;
                                e.maxHpValue = maxHpValAddr;
                                e.maxHpValueIsInt = maxHpIsInt;
                                float vhp = 0.0f;
                                if (hpIsInt)
                                    vhp = (float)g_proc->read<int32_t>(hpValAddr + Offsets::Value::Value);
                                else
                                    vhp = (float)g_proc->read<double>(hpValAddr + Offsets::Value::Value);
                                if (std::isfinite(vhp) && vhp > 0.0f && vhp < 1000000.0f)
                                {
                                    e.hp = vhp;
                                    e.maxHp = 100.0f;
                                    e.hpValid = true;
                                    if (maxHpValAddr)
                                    {
                                        float vmx = 0.0f;
                                        if (maxHpIsInt)
                                            vmx = (float)g_proc->read<int32_t>(maxHpValAddr + Offsets::Value::Value);
                                        else
                                            vmx = (float)g_proc->read<double>(maxHpValAddr + Offsets::Value::Value);
                                        if (std::isfinite(vmx) && vmx > 0.0f && vmx < 1000000.0f)
                                            e.maxHp = vmx;
                                    }
                                }
                            }
                            if (isBots && !e.humanoid && !e.hpValue && e.maxHp <= 0.0f)
                            {
                                e.hp = 100.0f;
                                e.maxHp = 100.0f;
                            }
                        }
                        if (isAnimal)
                        {
                            node hum = cur.find_child_of_class("Humanoid");
                            if (hum)
                            {
                                e.humanoid = hum.addr;
                                float ahp = g_proc->read<float>(hum.addr + Offsets::Humanoid::Health);
                                float amx = g_proc->read<float>(hum.addr + Offsets::Humanoid::MaxHealth);
                                if (std::isfinite(ahp) && ahp > 0.0f)
                                {
                                    e.hp = ahp;
                                    e.maxHp = (std::isfinite(amx) && amx > 0.0f) ? amx : 100.0f;
                                }
                                else
                                {
                                    continue;
                                }
                            }
                            else
                            {
                                e.hp = 100.0f;
                                e.maxHp = 100.0f;
                            }
                        }
                        g_scanOut.push_back(std::move(e));
                    }
                }
            }
            if (cls == "Workspace" || cls == "Folder" || cls == "Model")
            {
                for (auto& k : cur.children())
                {
                    if (g_scanQ.size() > kMaxQueue)
                        break;
                    g_scanQ.push(k.addr);
                }
            }
        }
        if (!g_scanQ.empty() && g_scanned < kMaxScan)
            return;
        int nOre = 0, nPlant = 0, nBots = 0, nAnimal = 0, nTool = 0, nCrate = 0, nKey = 0;
        for (auto& e : g_scanOut)
        {
            if (e.bots)
                ++nBots;
            else if (e.animal)
                ++nAnimal;
            else if (e.tool)
                ++nTool;
            else if (e.crate)
                ++nCrate;
            else if (e.keycard)
                ++nKey;
            else if (e.plant)
                ++nPlant;
            else
                ++nOre;
        }
        static int s_lastOre = -1, s_lastPlant = -1, s_lastBots = -1, s_lastAnimal = -1, s_lastTool = -1, s_lastCrate = -1, s_lastKey = -1;
        if (nOre != s_lastOre || nPlant != s_lastPlant || nBots != s_lastBots || nAnimal != s_lastAnimal || nTool != s_lastTool || nCrate != s_lastCrate || nKey != s_lastKey)
        {
            s_lastOre = nOre;
            s_lastPlant = nPlant;
            s_lastBots = nBots;
            s_lastAnimal = nAnimal;
            s_lastTool = nTool;
            s_lastCrate = nCrate;
            s_lastKey = nKey;
            LOGF("world", "scan: %d ore %d plant %d bots %d animals %d tools %d crates %d keys", nOre, nPlant, nBots, nAnimal, nTool, nCrate, nKey);
        }
        g_entries.swap(g_scanOut);
        g_scanning = false;
        g_lastScan = std::chrono::steady_clock::now();
    }
    void refresh_entries()
    {
        auto now = std::chrono::steady_clock::now();
        if (g_lastFast == std::chrono::steady_clock::time_point{} ||
            std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastFast).count() >= 100)
        {
            g_lastFast = now;
            if (!g_entries.empty())
                fast_refresh();
        }
        if (g_scanning)
        {
            full_scan();
            return;
        }
        if (g_lastScan == std::chrono::steady_clock::time_point{} ||
            std::chrono::duration_cast<std::chrono::milliseconds>(now - g_lastScan).count() >= 2000)
        {
            full_scan();
        }
    }
    void clear_entries()
    {
        g_entries.clear();
        while (!g_scanQ.empty())
            g_scanQ.pop();
        g_scanOut.clear();
        g_scanning = false;
        g_scanWs = 0;
        g_scanned = 0;
        g_lastScan = std::chrono::steady_clock::time_point{};
        g_lastFast = std::chrono::steady_clock::time_point{};
    }
}
