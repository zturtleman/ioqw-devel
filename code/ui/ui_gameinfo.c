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

#include "ui_local.h"

// arena and bot info
int ui_numBots;
static char *ui_botInfos[MAX_BOTS];
static int ui_numArenas;
static char *ui_arenaInfos[MAX_ARENAS];

/*
=======================================================================================================================================
UI_ParseInfos
=======================================================================================================================================
*/
static int UI_ParseInfos(char *buf, int max, char *infos[]) {
	char *token;
	int count;
	char info[MAX_INFO_STRING], key[MAX_TOKEN_CHARS];

	count = 0;

	while (1) {
		token = COM_Parse(&buf);

		if (!token[0]) {
			break;
		}

		if (strcmp(token, "{")) {
			Com_Printf("Missing { in info file\n");
			break;
		}

		if (count == max) {
			Com_Printf("Max infos exceeded\n");
			break;
		}

		info[0] = '\0';

		while (1) {
			token = COM_ParseExt(&buf, qtrue);

			if (!token[0]) {
				Com_Printf("Unexpected end of info file\n");
				break;
			}

			if (!strcmp(token, "}")) {
				break;
			}

			Q_strncpyz(key, token, sizeof(key));

			token = COM_ParseExt(&buf, qfalse);

			if (!token[0]) {
				strcpy(token, "<NULL>");
			}

			Info_SetValueForKey(info, key, token);
		}
		// NOTE: extra space for arena number
		infos[count] = UI_Alloc(strlen(info) + strlen("\\num\\") + strlen(va("%d", MAX_ARENAS)) + 1);

		if (infos[count]) {
			strcpy(infos[count], info);
			count++;
		}
	}

	return count;
}

/*
=======================================================================================================================================
UI_LoadArenasFromFile
=======================================================================================================================================
*/
static void UI_LoadArenasFromFile(const char *filename) {
	int len;
	fileHandle_t f;
	char buf[MAX_ARENAS_TEXT];

	len = trap_FS_FOpenFile(filename, &f, FS_READ);

	if (!f) {
		trap_Print(va(S_COLOR_RED "file not found: %s\n", filename));
		return;
	}

	if (len >= MAX_ARENAS_TEXT) {
		trap_Print(va(S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_ARENAS_TEXT));
		trap_FS_FCloseFile(f);
		return;
	}

	trap_FS_Read(buf, len, f);

	buf[len] = 0;

	trap_FS_FCloseFile(f);

	ui_numArenas += UI_ParseInfos(buf, MAX_ARENAS - ui_numArenas, &ui_arenaInfos[ui_numArenas]);
}

/*
=======================================================================================================================================
UI_LoadArenas
=======================================================================================================================================
*/
void UI_LoadArenas(void) {
	int numdirs, i, dirlen;
	vmCvar_t arenasFile;
	char dirlist[1024], filename[128];
	const char *dirptr;

	ui_numArenas = 0;

	trap_Cvar_Register(&arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM);

	if (*arenasFile.string) {
		UI_LoadArenasFromFile(arenasFile.string);
	} else {
		UI_LoadArenasFromFile("scripts/arenas.txt");
	}
	// get all arenas from .arena files
	numdirs = trap_FS_GetFileList("scripts", ".arena", dirlist, sizeof(dirlist));
	dirptr = dirlist;

	for (i = 0; i < numdirs; i++, dirptr += dirlen + 1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		UI_LoadArenasFromFile(filename);
	}

	trap_Print(va("%i arenas parsed\n", ui_numArenas));

	if (UI_OutOfMemory()) {
		trap_Print(S_COLOR_YELLOW "WARNING: not enough memory in pool to load all arenas\n");
	}
}

/*
=======================================================================================================================================
UI_LoadArenasIntoMapList
=======================================================================================================================================
*/
void UI_LoadArenasIntoMapList(void) {
	int n;
	const char *type, *str;

	uiInfo.mapCount = 0;

	for (n = 0; n < ui_numArenas; n++) {
		// determine type
		uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
		uiInfo.mapList[uiInfo.mapCount].mapLoadName = String_Alloc(Info_ValueForKey(ui_arenaInfos[n], "map"));
		uiInfo.mapList[uiInfo.mapCount].mapName = String_Alloc(Info_ValueForKey(ui_arenaInfos[n], "longname"));
		uiInfo.mapList[uiInfo.mapCount].levelShot = -1;
		uiInfo.mapList[uiInfo.mapCount].imageName = String_Alloc(va("levelshots/%s", uiInfo.mapList[uiInfo.mapCount].mapLoadName));
		uiInfo.mapList[uiInfo.mapCount].typeBits = 0;
		// set red respawn time
		str = Info_ValueForKey(ui_arenaInfos[n], "RedRespawnTime");

		if (*str) {
			uiInfo.mapList[uiInfo.mapCount].RedRespawnTime = atoi(str);
		} else {
			uiInfo.mapList[uiInfo.mapCount].RedRespawnTime = 0;
		}
		// set blue respawn time
		str = Info_ValueForKey(ui_arenaInfos[n], "BlueRespawnTime");

		if (*str) {
			uiInfo.mapList[uiInfo.mapCount].BlueRespawnTime = atoi(str);
		} else {
			uiInfo.mapList[uiInfo.mapCount].BlueRespawnTime = 0;
		}

		type = Info_ValueForKey(ui_arenaInfos[n], "type");
		// if no type specified, it will be treated as "ffa"
		if (*type) {
			if (strstr(type, "ffa")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
			}

			if (strstr(type, "tourney")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_TOURNAMENT);
			}

			if (strstr(type, "team")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_TEAM);
			}

			if (strstr(type, "ctf")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_CTF);
			}

			if (strstr(type, "oneflag")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_1FCTF);
			}

			if (strstr(type, "overload")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_OBELISK);
			}

			if (strstr(type, "harvester")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_HARVESTER);
			}

			if (strstr(type, "campaign")) {
				uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_CAMPAIGN);
			}
		} else {
			uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << GT_FFA);
		}

		uiInfo.mapCount++;

		if (uiInfo.mapCount >= MAX_MAPS) {
			break;
		}
	}
}

/*
=======================================================================================================================================
UI_LoadBotsFromFile
=======================================================================================================================================
*/
static void UI_LoadBotsFromFile(const char *filename) {
	int len;
	fileHandle_t f;
	char buf[MAX_BOTS_TEXT];

	len = trap_FS_FOpenFile(filename, &f, FS_READ);

	if (!f) {
		trap_Print(va(S_COLOR_RED "file not found: %s\n", filename));
		return;
	}

	if (len >= MAX_BOTS_TEXT) {
		trap_Print(va(S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_BOTS_TEXT));
		trap_FS_FCloseFile(f);
		return;
	}

	trap_FS_Read(buf, len, f);

	buf[len] = 0;

	trap_FS_FCloseFile(f);
	COM_Compress(buf);

	ui_numBots += UI_ParseInfos(buf, MAX_BOTS - ui_numBots, &ui_botInfos[ui_numBots]);
}

/*
=======================================================================================================================================
UI_LoadBots
=======================================================================================================================================
*/
void UI_LoadBots(void) {
	vmCvar_t botsFile;
	int numdirs, i, dirlen;
	char dirlist[1024], filename[128];
	const char *dirptr;

	ui_numBots = 0;

	trap_Cvar_Register(&botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM);

	if (*botsFile.string) {
		UI_LoadBotsFromFile(botsFile.string);
	} else {
		UI_LoadBotsFromFile("scripts/bots.txt");
	}
	// get all bots from .bot files
	numdirs = trap_FS_GetFileList("scripts", ".bot", dirlist, sizeof(dirlist));
	dirptr = dirlist;

	for (i = 0; i < numdirs; i++, dirptr += dirlen + 1) {
		dirlen = strlen(dirptr);
		strcpy(filename, "scripts/");
		strcat(filename, dirptr);
		UI_LoadBotsFromFile(filename);
	}

	trap_Print(va("%i bots parsed\n", ui_numBots));
}

/*
=======================================================================================================================================
UI_GetBotInfoByNumber
=======================================================================================================================================
*/
static const char *UI_GetBotInfoByNumber(int num) {

	if (num < 0 || num >= ui_numBots) {
		trap_Print(va(S_COLOR_RED "Invalid bot number: %i\n", num));
		return NULL;
	}

	return ui_botInfos[num];
}

/*
=======================================================================================================================================
UI_GetBotInfoByName
=======================================================================================================================================
*/
char *UI_GetBotInfoByName(const char *name) {
	int n;
	char *value;

	for (n = 0; n < ui_numBots; n++) {
		value = Info_ValueForKey(ui_botInfos[n], "name");

		if (!Q_stricmp(value, name)) {
			return ui_botInfos[n];
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
UI_GetNumBots
=======================================================================================================================================
*/
int UI_GetNumBots(void) {
	return ui_numBots;
}

/*
=======================================================================================================================================
UI_GetBotNameByNumber
=======================================================================================================================================
*/
char *UI_GetBotNameByNumber(int num) {
	const char *info;

	info = UI_GetBotInfoByNumber(num);

	if (info) {
		return Info_ValueForKey(info, "name");
	}

	return "Random";
}
