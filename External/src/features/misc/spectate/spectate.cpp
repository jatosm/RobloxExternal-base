#include "spectate.h"
#include "../../../utility/cache/cache.h"
#include "../../../utility/global/globals.h"
#include "../../../utility/struct/offsets.h"
#include "../../../menu/overlay/settings.h"
#include <cmath>
namespace thenwefuckin::misc::spectate
{
    namespace
    {
        uintptr_t s_target = 0;

        bool target_present(uintptr_t playerAddr)
        {
            if (!playerAddr)
                return false;
            for (auto& pl : g_ctx.player_list())
            {
                if (pl.addr == playerAddr)
                    return true;
            }
            return false;
        }

        uintptr_t subject_for(uintptr_t playerAddr)
        {
            node pl{ playerAddr };
            node ch = g_ctx.get_character(pl);
            if (!ch)
                return 0;
            node hum = find_live_humanoid(ch);
            if (hum)
                return hum.addr;
            return 0;
        }

        uintptr_t local_subject()
        {
            if (g_ctx.local_humanoid)
                return g_ctx.local_humanoid.addr;
            node hum = g_ctx.resolve_local_humanoid();
            if (hum)
                return hum.addr;
            return 0;
        }

        void write_subject(uintptr_t subject)
        {
            if (!g_ctx.camera)
                return;
            g_proc->write<uintptr_t>(g_ctx.camera.addr + Offsets::Camera::CameraSubject, subject);
        }
    }

    void toggle(uintptr_t playerAddr)
    {
        if (!playerAddr)
            return;
        if (s_target == playerAddr)
        {
            stop();
            return;
        }
        if (g_ctx.local_player && playerAddr == g_ctx.local_player.addr)
        {
            stop();
            return;
        }
        s_target = playerAddr;

        settings::freecam_enabled = false;
        if (g_proc && g_proc->connected() && g_ctx.ready && g_ctx.camera)
        {
            uintptr_t subj = subject_for(s_target);
            if (subj)
                write_subject(subj);
        }
    }

    void stop()
    {
        if (!s_target)
            return;
        s_target = 0;
        if (g_proc && g_proc->connected() && g_ctx.ready && g_ctx.camera)
        {
            uintptr_t back = local_subject();
            if (back)
                write_subject(back);
        }
    }

    bool active()
    {
        return s_target != 0;
    }

    bool is_spectating(uintptr_t playerAddr)
    {
        return playerAddr != 0 && s_target == playerAddr;
    }

    uintptr_t target()
    {
        return s_target;
    }

    bool teleport_to(uintptr_t playerAddr)
    {
        if (!playerAddr || !g_proc || !g_proc->connected() || !g_ctx.ready)
            return false;

        if (s_target)
            stop();
        node pl{ playerAddr };
        node ch = g_ctx.get_character(pl);
        if (!ch)
            return false;
        node anchor = ch.find_child("HumanoidRootPart");
        if (!anchor)
            anchor = ch.find_child("Head");
        if (!anchor)
            anchor = ch.find_child("Torso");
        if (!anchor)
            anchor = ch.find_child("UpperTorso");
        if (!anchor)
            return false;
        uintptr_t anchorPrim = anchor.primitive();
        if (!anchorPrim)
            return false;
        math::vec3 dst = g_proc->read<math::vec3>(anchorPrim + Offsets::Primitive::Position);
        if (dst.x == 0.0f && dst.y == 0.0f && dst.z == 0.0f)
            return false;
        if (!g_ctx.local_root)
            return false;
        uintptr_t myPrim = g_ctx.local_root.primitive();
        if (!myPrim)
            return false;

        math::vec3 land{ dst.x + 3.0f, dst.y + 4.0f, dst.z + 3.0f };
        g_proc->write<math::vec3>(myPrim + Offsets::Primitive::Position, land);
        math::vec3 zero{};
        g_proc->write<math::vec3>(myPrim + Offsets::Primitive::AssemblyLinearVelocity, zero);
        g_proc->write<math::vec3>(myPrim + Offsets::Primitive::AssemblyAngularVelocity, zero);
        return true;
    }

    void run()
    {
        if (!s_target)
            return;
        if (!g_proc || !g_proc->connected())
        {
            s_target = 0;
            return;
        }
        if (!g_ctx.ready || !g_ctx.camera)
            return;

        if (settings::freecam_enabled)
            return;
        if (g_ctx.local_player && s_target == g_ctx.local_player.addr)
        {
            s_target = 0;
            return;
        }
        if (!target_present(s_target))
        {
            stop();
            return;
        }
        uintptr_t subj = subject_for(s_target);
        if (!subj)
            return;
        write_subject(subj);
    }
}
