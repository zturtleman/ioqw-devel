#include "inv.h"
// initial health/armor states
#define FS_HEALTH	3
#define FS_ARMOR	3
// initial weapon weights
//#define W_HANDGUN			60
#define W_MACHINEGUN		65
//#define W_HEAVY_MACHINEGUN	70
#define W_CHAINGUN			250
#define W_SHOTGUN			55
#define W_NAILGUN			30
//#define W_PHOSPHORGUN		35
#define W_PROXLAUNCHER		150
#define W_GRENADELAUNCHER	140
#define W_NAPALMLAUNCHER	145
#define W_ROCKETLAUNCHER	250
#define W_BEAMGUN			20
#define W_RAILGUN			15
#define W_PLASMAGUN			40
#define W_BFG10K			250
//#define W_MISSILELAUNCHER	255
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				45
#define GWW_MACHINEGUN			50
//#define GWW_HEAVY_MACHINEGUN	55
#define GWW_CHAINGUN			430
#define GWW_SHOTGUN				70
#define GWW_NAILGUN				70
//#define GWW_PHOSPHORGUN			75
#define GWW_PROXLAUNCHER		75
#define GWW_GRENADELAUNCHER		85
#define GWW_NAPALMLAUNCHER		90
#define GWW_ROCKETLAUNCHER		90
#define GWW_BEAMGUN				400
#define GWW_RAILGUN				25
#define GWW_PLASMAGUN			80
#define GWW_BFG10K				590
//#define GWW_MISSILELAUNCHER		600
// initial powerup weights
#define W_MEDKIT		350
#define W_KAMIKAZE		800
#define W_QUAD			400
#define W_INVISIBILITY	100
#define W_REGEN			400
#define W_AMMOREGEN		300
#define W_GUARD			400
#define W_DOUBLER		200
#define W_SCOUT			10

#include "fw_items.c"
