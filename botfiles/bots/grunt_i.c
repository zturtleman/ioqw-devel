#include "inv.h"
// initial health/armor states
#define FS_HEALTH	1
#define FS_ARMOR	1
// initial weapon weights
//#define W_HANDGUN			60
#define W_MACHINEGUN		65
//#define W_HEAVY_MACHINEGUN	70
#define W_CHAINGUN			140
#define W_SHOTGUN			50
#define W_NAILGUN			170
//#define W_PHOSPHORGUN		175
#define W_PROXLAUNCHER		20
#define W_GRENADELAUNCHER	40
#define W_NAPALMLAUNCHER	45
#define W_ROCKETLAUNCHER	220
#define W_BEAMGUN			50
#define W_RAILGUN			385
#define W_PLASMAGUN			50
#define W_BFG10K			30
//#define W_MISSILELAUNCHER	35
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				45
#define GWW_MACHINEGUN			50
//#define GWW_HEAVY_MACHINEGUN	55
#define GWW_CHAINGUN			140
#define GWW_SHOTGUN				35
#define GWW_NAILGUN				170
//#define GWW_PHOSPHORGUN			175
#define GWW_PROXLAUNCHER		100
#define GWW_GRENADELAUNCHER		30
#define GWW_NAPALMLAUNCHER		35
#define GWW_ROCKETLAUNCHER		190
#define GWW_BEAMGUN				40
#define GWW_RAILGUN				325
#define GWW_PLASMAGUN			40
#define GWW_BFG10K				41
//#define GWW_MISSILELAUNCHER		42
// initial powerup weights
#define W_MEDKIT		200
#define W_KAMIKAZE		200
#define W_QUAD			90
#define W_INVISIBILITY	80
#define W_REGEN			80
#define W_AMMOREGEN		80
#define W_GUARD			100
#define W_DOUBLER		280
#define W_SCOUT			400

#include "fw_items.c"
