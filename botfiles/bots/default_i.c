#include "inv.h"
// initial health/armor states
#define FS_HEALTH	2
#define FS_ARMOR	3
// initial weapon weights
//#define W_HANDGUN			480
#define W_MACHINEGUN		490
//#define W_HEAVY_MACHINEGUN	500
#define W_CHAINGUN			560
#define W_SHOTGUN			530
#define W_NAILGUN			580
//#define W_PHOSPHORGUN		585
#define W_PROXLAUNCHER		520
#define W_GRENADELAUNCHER	505
#define W_NAPALMLAUNCHER	510
#define W_ROCKETLAUNCHER	540
#define W_BEAMGUN			570
#define W_RAILGUN			590
#define W_PLASMAGUN			550
#define W_BFG10K			600
//#define W_MISSILELAUNCHER	605
// the bot has the weapons, so the weights change a little bit
//#define GWW_HANDGUN				425
#define GWW_MACHINEGUN			430
//#define GWW_HEAVY_MACHINEGUN	435
#define GWW_CHAINGUN			490
#define GWW_SHOTGUN				410
#define GWW_NAILGUN				460
//#define GWW_PHOSPHORGUN			465
#define GWW_PROXLAUNCHER		400
#define GWW_GRENADELAUNCHER		420
#define GWW_NAPALMLAUNCHER		425
#define GWW_ROCKETLAUNCHER		440
#define GWW_BEAMGUN				480
#define GWW_RAILGUN				450
#define GWW_PLASMAGUN			470
#define GWW_BFG10K				500
//#define GWW_MISSILELAUNCHER		510
// initial powerup weights
#define W_MEDKIT		800
#define W_KAMIKAZE		800
#define W_QUAD			800
#define W_INVISIBILITY	800
#define W_REGEN			800
#define W_AMMOREGEN		800
#define W_GUARD			800
#define W_DOUBLER		800
#define W_SCOUT			800

#include "fw_items.c"
