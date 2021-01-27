/*
=======================================================================================================================================
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of Spearmint Source Code.

Spearmint Source Code is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License, or (at your option) any later version.

Spearmint Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Spearmint Source Code.
If not, see <http://www.gnu.org/licenses/>.

In addition, Spearmint Source Code is also subject to certain additional terms. You should have received a copy of these additional
terms immediately following the terms and conditions of the GNU General Public License. If not, please request a copy in writing from
id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
=======================================================================================================================================
*/

/**************************************************************************************************************************************

	Allow a lot of command backups for very fast systems. Multiple commands may be combined into a single packet, so this needs to be
	larger than PACKET_BACKUP.

**************************************************************************************************************************************/

#define CMD_BACKUP 64
#define CMD_MASK (CMD_BACKUP - 1)

/**************************************************************************************************************************************

	Snapshots are a view of the server at a given time.

	Snapshots are generated at regular time intervals by the server, but they may not be sent if a client's rate level is exceeded, or
	they may be dropped by the network.

**************************************************************************************************************************************/

#define MAX_ENTITIES_IN_SNAPSHOT 256

typedef struct {
	int snapFlags;										// SNAPFLAG_RATE_DELAYED, etc.
	int ping;
	int serverTime;										// server time the message is valid for (in msec)
	byte areamask[MAX_MAP_AREA_BYTES];					// portalarea visibility bits
	playerState_t ps;									// complete information about the current player at this time
	int numEntities;
	entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];	// all of the entities that need to be presented at the time of this snapshot
	int numServerCommands;								// text based server commands to execute when this
	int serverCommandSequence;							// snapshot becomes current
} snapshot_t;

enum {
	CGAME_EVENT_NONE,
	CGAME_EVENT_TEAMMENU,
	CGAME_EVENT_SCOREBOARD,
	CGAME_EVENT_EDITHUD
};

/*
=======================================================================================================================================

	Functions imported from the main executable.

=======================================================================================================================================
*/

	// See sharedTraps_t in qcommon.h for TRAP_MEMSET = 0, etc.

typedef enum {
	//============== general Quake services ==================
	CG_PRINT = 20,
	CG_ERROR,
	CG_MILLISECONDS,
	CG_REAL_TIME,
	CG_SNAPVECTOR,
	CG_ARGC,
	CG_ARGV,
	CG_ARGS,
	CG_ADDCOMMAND,
	CG_REMOVECOMMAND,
	CG_CMD_EXECUTETEXT,
	CG_CVAR_REGISTER,
	CG_CVAR_UPDATE,
	CG_CVAR_SET,
	CG_CVAR_SET_VALUE,
	CG_CVAR_VARIABLE_VALUE,
	CG_CVAR_VARIABLE_INTEGER_VALUE,
	CG_CVAR_VARIABLE_STRING_BUFFER,
	CG_FS_FOPENFILE,
	CG_FS_READ,
	CG_FS_WRITE,
	CG_FS_SEEK,
	CG_FS_FCLOSEFILE,
	CG_FS_GETFILELIST,
	CG_PC_ADD_GLOBAL_DEFINE,
	CG_PC_LOAD_SOURCE,
	CG_PC_FREE_SOURCE,
	CG_PC_READ_TOKEN,
	CG_PC_SOURCE_FILE_AND_LINE,
	//=========== client game specific functionality =============
	CG_GETGLCONFIG = 100,
	CG_MEMORY_REMAINING,
	CG_UPDATESCREEN,

	CG_GETGAMESTATE = 150,
	CG_GETCURRENTSNAPSHOTNUMBER,
	CG_GETSNAPSHOT,
	CG_GETSERVERCOMMAND,
	CG_GETCURRENTCMDNUMBER,
	CG_GETUSERCMD,
	CG_SETUSERCMDVALUE,
	CG_SENDCLIENTCOMMAND,

	CG_CM_LOADMAP = 200,
	CG_CM_NUMINLINEMODELS,
	CG_CM_INLINEMODEL,
	CG_CM_MARKFRAGMENTS,
	CG_CM_POINTCONTENTS,
	CG_CM_TRANSFORMEDPOINTCONTENTS,
	CG_CM_TEMPBOXMODEL,
	CG_CM_BOXTRACE,
	CG_CM_TRANSFORMEDBOXTRACE,
	CG_CM_TEMPCAPSULEMODEL,
	CG_CM_CAPSULETRACE,
	CG_CM_TRANSFORMEDCAPSULETRACE,

	CG_R_REGISTERMODEL = 300,
	CG_R_REGISTERSHADEREX,
	CG_R_REGISTERSHADER,
	CG_R_REGISTERSHADERNOMIP,
	CG_R_REGISTERFONT,
	CG_R_RENDERSCENE,
	CG_R_CLEARSCENE,
	CG_R_SETCOLOR,
	CG_R_LOADWORLDMAP,
	CG_R_INPVS,
	CG_GET_ENTITY_TOKEN,
	CG_R_ADDREFENTITYTOSCENE,
	CG_R_ADDPOLYREFENTITYTOSCENE,
	CG_R_ADDPOLYTOSCENE,
	CG_R_ADDPOLYSTOSCENE,
	CG_R_ADDPOLYBUFFERTOSCENE,
	CG_R_LIGHTFORPOINT,
	CG_R_ADDLIGHTTOSCENE,
	CG_R_ADDADDITIVELIGHTTOSCENE,
	CG_R_ADDVERTEXLIGHTTOSCENE,
	CG_R_ADDJUNIORLIGHTTOSCENE,
	CG_R_ADDDIRECTEDLIGHTTOSCENE,
	CG_R_ADDCORONATOSCENE,
	CG_R_GET_GLOBAL_FOG,
	CG_R_GET_VIEW_FOG,
	CG_R_MODELBOUNDS,
	CG_R_LERPTAG,
	CG_R_LERPTAG_FRAMEMODEL,
	CG_R_LERPTAG_TORSO,
	CG_R_ALLOCSKINSURFACE,
	CG_R_ADDSKINTOFRAME,
	CG_R_SETCLIPREGION,
	CG_R_DRAWSTRETCHPIC,
	CG_R_DRAWSTRETCHPIC_GRADIENT,
	CG_R_DRAWROTATEDPIC,
	CG_R_DRAW2DPOLYS,
	CG_R_REMAP_SHADER,

	CG_S_REGISTERSOUND = 400,
	CG_S_STARTLOCALSOUND,
	CG_S_STARTBACKGROUNDTRACK,
	CG_S_STOPBACKGROUNDTRACK,
	CG_S_STARTSOUND,
	CG_S_CLEARLOOPINGSOUNDS,
	CG_S_ADDLOOPINGSOUND,
	CG_S_ADDREALLOOPINGSOUND,
	CG_S_STOPLOOPINGSOUND,
	CG_S_UPDATEENTITYPOSITION,
	CG_S_RESPATIALIZE,

	CG_KEY_GETCATCHER = 500,
	CG_KEY_SETCATCHER,
	CG_KEY_ISDOWN,
	CG_KEY_GETKEY,

	CG_CIN_PLAYCINEMATIC = 600,
	CG_CIN_STOPCINEMATIC,
	CG_CIN_RUNCINEMATIC,
	CG_CIN_DRAWCINEMATIC,
	CG_CIN_SETEXTENTS
/*
	CG_LOADCAMERA,
	CG_STARTCAMERA,
	CG_GETCAMERAINFO,
*/
} cgameImport_t;

/*
=======================================================================================================================================

	Functions exported to the main executable.

=======================================================================================================================================
*/

typedef enum {
	CG_INIT,
//	void CG_Init(int serverMessageNum, int serverCommandSequence, int clientNum)
	// called when the level loads or when the renderer is restarted
	// all media should be registered at this time
	// cgame will display loading status by calling SCR_Update, which will call CG_DrawInformation during the loading process
	// reliableCommandSequence will be 0 on fresh loads, but higher for demos, tourney restarts, or vid_restarts
	CG_SHUTDOWN,
//	void (*CG_Shutdown)(void);
	// opportunity to flush and close any open files
	CG_CONSOLE_COMMAND,
//	qboolean (*CG_ConsoleCommand)(void);
	// a console command has been issued locally that is not recognized by the main game system
	// use Cmd_Argc() / Cmd_Argv() to read the command, return qfalse if the command is not known to the game
	CG_DRAW_ACTIVE_FRAME,
//	void (*CG_DrawActiveFrame)(int serverTime, stereoFrame_t stereoView, qboolean demoPlayback);
	// generates and draws a game scene and status information at the given time
	// if demoPlayback is set, local movement prediction will not be enabled
	CG_CROSSHAIR_PLAYER,
//	int (*CG_CrosshairPlayer)(void);
	CG_LAST_ATTACKER,
//	int (*CG_LastAttacker)(void);
	CG_KEY_EVENT,
//	void (*CG_KeyEvent)(int key, qboolean down);
	CG_MOUSE_EVENT,
//	void (*CG_MouseEvent)(int dx, int dy);
	CG_EVENT_HANDLING
//	void (*CG_EventHandling)(int type);
} cgameExport_t;
