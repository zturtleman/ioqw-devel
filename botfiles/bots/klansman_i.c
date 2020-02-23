#include "inv.h"
// initial health/armor states
#define FS_HEALTH	3
#define FS_ARMOR	4
// initial weapon weights
//#define W_HANDGUN			60
#define W_MACHINEGUN		65
//#define W_HEAVY_MACHINEGUN	70
#define W_CHAINGUN			320
#define W_SHOTGUN			80
#define W_NAILGUN			290
//#define W_PHOSPHORGUN		295
#define W_PROXLAUNCHER		270
#define W_GRENADELAUNCHER	140
#define W_NAPALMLAUNCHER	145
#define W_ROCKETLAUNCHER	380
#define W_BEAMGUN			330
#define W_RAILGUN			200
#define W_PLASMAGUN			250
#define W_BFG10K			400
//#define W_MISSILELAUNCHER	405
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				45
#define GWW_MACHINEGUN			50
//#define GWW_HEAVY_MACHINEGUN	55
#define GWW_CHAINGUN			40
#define GWW_SHOTGUN				35
#define GWW_NAILGUN				70
//#define GWW_PHOSPHORGUN			75
#define GWW_PROXLAUNCHER		10
#define GWW_GRENADELAUNCHER		30
#define GWW_NAPALMLAUNCHER		35
#define GWW_ROCKETLAUNCHER		80
#define GWW_BEAMGUN				90
#define GWW_RAILGUN				50
#define GWW_PLASMAGUN			90
#define GWW_BFG10K				110
//#define GWW_MISSILELAUNCHER		120
// initial powerup weights
#define W_MEDKIT		100
#define W_KAMIKAZE		300
#define W_QUAD			200
#define W_INVISIBILITY	200
#define W_REGEN			200
#define W_AMMOREGEN		300
#define W_GUARD			400
#define W_DOUBLER		300
#define W_SCOUT			300

#include "fw_items.c"
