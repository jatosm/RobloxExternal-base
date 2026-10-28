#include "friends.h"
#include <unordered_set>
namespace thenwefuckin::visual::players::friends
{
    namespace
    {
        std::unordered_set<uintptr_t> g_friends;
    }
    void set_friend(uintptr_t playerAddr, uintptr_t modelAddr, bool friended)
    {
        if (friended)
        {
            if (playerAddr)
                g_friends.insert(playerAddr);
            if (modelAddr)
                g_friends.insert(modelAddr);
        }
        else
        {
            if (playerAddr)
                g_friends.erase(playerAddr);
            if (modelAddr)
                g_friends.erase(modelAddr);
        }
    }
    bool is_friend(uintptr_t addr)
    {
        if (!addr)
            return false;
        return g_friends.find(addr) != g_friends.end();
    }
    void clear()
    {
        g_friends.clear();
    }
}
