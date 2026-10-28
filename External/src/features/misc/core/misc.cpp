#include "misc.h"
#include "../walkspeed/walkspeed.h"
#include "../jumppower/jumppower.h"
#include "../gravity/gravity.h"
#include "../antiafk/antiafk.h"
#include "../nofall/nofall.h"
#include "../hitsound/hitsound.h"
#include "../noclip/noclip.h"
#include "../fly/fly.h"
#include "../spectate/spectate.h"
#include "../hipheight/hipheight.h"
#include "../infjump/infjump.h"
#include "../../visual/world/time/time.h"
namespace thenwefuckin::misc
{
    void run()
    {
        walkspeed::run();
        jumppower::run();
        gravity::run();
        antiafk::run();
        nofall::run();
        hitsound::run();
        noclip::run();
        fly::run();
        spectate::run();
        hipheight::run();
        infjump::run();
        world::time::run();
    }
}
