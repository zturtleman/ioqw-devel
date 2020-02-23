#include "inv.h"
// initial health/armor states
#define FS_HEALTH	3
#define FS_ARMOR	3
// initial weapon weights
//#define W_HANDGUN			35
#define W_MACHINEGUN		40
//#define W_HEAVY_MACHINEGUN	45
#define W_CHAINGUN			140
#define W_SHOTGUN			150
#define W_NAILGUN			20
//#define W_PHOSPHORGUN		25
#define W_PROXLAUNCHER		10
#define W_GRENADELAUNCHER	90
#define W_NAPALMLAUNCHER	95
#define W_ROCKETLAUNCHER	200
#define W_BEAMGUN			10
#define W_RAILGUN			10
#define W_PLASMAGUN			20
#define W_BFG10K			400
//#define W_MISSILELAUNCHER	405
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				25
#define GWW_MACHINEGUN			30
//#define GWW_HEAVY_MACHINEGUN	35
#define GWW_CHAINGUN			540
#define GWW_SHOTGUN				35
#define GWW_NAILGUN				30
//#define GWW_PHOSPHORGUN			35
#define GWW_PROXLAUNCHER		15
#define GWW_GRENADELAUNCHER		30
#define GWW_NAPALMLAUNCHER		35
#define GWW_ROCKETLAUNCHER		70
#define GWW_BEAMGUN				30
#define GWW_RAILGUN				15
#define GWW_PLASMAGUN			10
#define GWW_BFG10K				590
//#define GWW_MISSILELAUNCHER		600
// initial powerup weights
#define W_MEDKIT		100
#define W_KAMIKAZE		200
#define W_QUAD			400
#define W_INVISIBILITY	100
#define W_REGEN			400
#define W_AMMOREGEN		200
#define W_GUARD			400
#define W_DOUBLER		300
#define W_SCOUT			10

#include "fw_items.c"
