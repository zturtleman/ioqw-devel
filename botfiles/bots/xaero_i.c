#include "inv.h"
// initial health/armor states
#define FS_HEALTH	2
#define FS_ARMOR	3
// initial weapon weights
//#define W_HANDGUN			60
#define W_MACHINEGUN		65
//#define W_HEAVY_MACHINEGUN	70
#define W_CHAINGUN			85
#define W_SHOTGUN			50
#define W_NAILGUN			70
//#define W_PHOSPHORGUN		75
#define W_PROXLAUNCHER		15
#define W_GRENADELAUNCHER	40
#define W_NAPALMLAUNCHER	45
#define W_ROCKETLAUNCHER	120
#define W_BEAMGUN			80
#define W_RAILGUN			285
#define W_PLASMAGUN			80
#define W_BFG10K			130
//#define W_MISSILELAUNCHER	140
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				45
#define GWW_MACHINEGUN			50
//#define GWW_HEAVY_MACHINEGUN	55
#define GWW_CHAINGUN			85
#define GWW_SHOTGUN				35
#define GWW_NAILGUN				20
//#define GWW_PHOSPHORGUN			25
#define GWW_PROXLAUNCHER		15
#define GWW_GRENADELAUNCHER		30
#define GWW_NAPALMLAUNCHER		35
#define GWW_ROCKETLAUNCHER		90
#define GWW_BEAMGUN				80
#define GWW_RAILGUN				225
#define GWW_PLASMAGUN			80
#define GWW_BFG10K				101
//#define GWW_MISSILELAUNCHER		102
// initial powerup weights
#define W_MEDKIT		350
#define W_KAMIKAZE		800
#define W_QUAD			400
#define W_INVISIBILITY	400
#define W_REGEN			3050
#define W_AMMOREGEN		40
#define W_GUARD			400
#define W_DOUBLER		400
#define W_SCOUT			400

#include "fw_items.c"
