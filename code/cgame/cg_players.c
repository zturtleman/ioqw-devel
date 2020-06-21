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
 Handle the media and animation for player entities.
**************************************************************************************************************************************/

#include "cg_local.h"

char *cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	// default death
	"*dd1.wav",
	"*dd2.wav",
	"*dd3.wav",
	// drown
	"*dr1.wav",
	// fall death
	"*df1.wav",
	// falling far
	"*ff1.wav",
	"*ff2.wav",
	"*ff3.wav",
	"*ff4.wav",
	"*ff5.wav",
	// fall into void
	"*fv1.wav",
	// gasp
	"*gp1.wav",
	// default jump
	"*jd1.wav",
	// default pain
	"*pd1.wav",
	"*pd2.wav",
	"*pd3.wav",
	"*pd4.wav",
	// taunt
	"*ta1.wav"
};

/*
=======================================================================================================================================
CG_CustomSound
=======================================================================================================================================
*/
sfxHandle_t CG_CustomSound(int clientNum, const char *soundName) {
	clientInfo_t *ci;
	int i;

	if (soundName[0] != '*') {
		return trap_S_RegisterSound(soundName, qfalse);
	}

	if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
		clientNum = 0;
	}

	ci = &cgs.clientinfo[clientNum];

	for (i = 0; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i]; i++) {
		if (!strcmp(soundName, cg_customSoundNames[i])) {
			return ci->sounds[i];
		}
	}

	CG_Error("Unknown custom sound: %s", soundName);
	return 0;
}

/*
=======================================================================================================================================
CG_CachePlayerSounds

Used to cache player sounds at start up.
=======================================================================================================================================
*/
void CG_CachePlayerSounds(const char *modelName) {
	char filename[MAX_QPATH];
	const char *s;
	int i;

	for (i = 0; i < MAX_CUSTOM_SOUNDS; i++) {
		s = cg_customSoundNames[i];

		if (!s) {
			break;
		}

		Com_sprintf(filename, sizeof(filename), "snd/c/%s/%s", modelName, s + 1);
		trap_S_RegisterSound(filename, qfalse);
	}
}

/*
=======================================================================================================================================
CG_CachePlayerModels

Used to cache player models at start up.
=======================================================================================================================================
*/
void CG_CachePlayerModels(const char *modelName, const char *headModelName) {
	char filename[MAX_QPATH];

	Com_sprintf(filename, sizeof(filename), "models/players/%s/lower.md3", modelName);
	trap_R_RegisterModel(filename);

	Com_sprintf(filename, sizeof(filename), "models/players/%s/upper.md3", modelName);
	trap_R_RegisterModel(filename);

	if (headModelName[0] == '*') {
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1]);
	} else {
		Com_sprintf(filename, sizeof(filename), "models/players/%s/head.md3", headModelName);
	}

	if (!trap_R_RegisterModel(filename) && headModelName[0] != '*') {
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", headModelName, headModelName);
		trap_R_RegisterModel(filename);
	}
}

/*
=======================================================================================================================================

	CLIENT INFO

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_ParseAnimationFile

Read a configuration file containing animation counts and rates, models/players/visor/animation.cfg, etc.
=======================================================================================================================================
*/
static qboolean CG_ParseAnimationFile(const char *filename, clientInfo_t *ci) {
	char *text_p, *prev;
	int len;
	int i;
	char *token;
	float fps;
	int skip;
	char text[20000];
	fileHandle_t f;
	animation_t *animations;

	animations = ci->animations;
	// load the file
	len = trap_FS_FOpenFile(filename, &f, FS_READ);

	if (len <= 0) {
		return qfalse;
	}

	if (len >= sizeof(text) - 1) {
		CG_Printf("File %s too long\n", filename);
		trap_FS_FCloseFile(f);
		return qfalse;
	}

	trap_FS_Read(text, len, f);

	text[len] = 0;

	trap_FS_FCloseFile(f);
	// parse the text
	text_p = text;
	skip = 0; // quite the compiler warning

	VectorClear(ci->headOffset);

	ci->foottype = FOOTTYPE_DEFAULT;
	ci->gender = GENDER_MALE;
	ci->fixedlegs = qfalse;
	ci->fixedtorso = qfalse;
	// read optional parameters
	while (1) {
		prev = text_p; // so we can unget
		token = COM_Parse(&text_p);

		if (!token[0]) {
			break;
		}

		if (!Q_stricmp(token, "foottype")) {
			token = COM_Parse(&text_p);

			if (!token[0]) {
				break;
			}

			if (!Q_stricmp(token, "default")) {
				ci->foottype = FOOTTYPE_DEFAULT;
			} else if (!Q_stricmp(token, "boot_01")) {
				ci->foottype = FOOTTYPE_BOOT_01;
			} else if (!Q_stricmp(token, "boot_02")) {
				ci->foottype = FOOTTYPE_BOOT_02;
			} else if (!Q_stricmp(token, "boot_03")) {
				ci->foottype = FOOTTYPE_BOOT_03;
			} else if (!Q_stricmp(token, "flesh_01")) {
				ci->foottype = FOOTTYPE_FLESH_01;
			} else if (!Q_stricmp(token, "flesh_02")) {
				ci->foottype = FOOTTYPE_FLESH_02;
			} else if (!Q_stricmp(token, "heels_01")) {
				ci->foottype = FOOTTYPE_HEELS_01;
			} else if (!Q_stricmp(token, "heels_02")) {
				ci->foottype = FOOTTYPE_HEELS_02;
			} else if (!Q_stricmp(token, "heels_03")) {
				ci->foottype = FOOTTYPE_HEELS_03;
			} else if (!Q_stricmp(token, "sandals_01")) {
				ci->foottype = FOOTTYPE_SANDALS_01;
			} else if (!Q_stricmp(token, "step_01")) {
				ci->foottype = FOOTTYPE_STEP_01;
			} else if (!Q_stricmp(token, "step_02")) {
				ci->foottype = FOOTTYPE_STEP_02;
			} else if (!Q_stricmp(token, "step_03")) {
				ci->foottype = FOOTTYPE_STEP_03;
			} else if (!Q_stricmp(token, "strogg_01")) {
				ci->foottype = FOOTTYPE_STROGG_01;
			} else if (!Q_stricmp(token, "klesk")) {
				ci->foottype = FOOTTYPE_SPEC_KLESK;
			} else if (!Q_stricmp(token, "sorlag")) {
				ci->foottype = FOOTTYPE_SPEC_SORLAG;
			} else if (!Q_stricmp(token, "t2m")) {
				ci->foottype = FOOTTYPE_T2_MEDIUM;
			} else if (!Q_stricmp(token, "t2h")) {
				ci->foottype = FOOTTYPE_T2_HEAVY;
			} else if (!Q_stricmp(token, "t2s")) {
				ci->foottype = FOOTTYPE_T2_SMALL;
			} else {
				CG_Printf("Bad foottype parm in %s: %s\n", filename, token);
			}

			continue;
		} else if (!Q_stricmp(token, "headoffset")) {
			for (i = 0; i < 3; i++) {
				token = COM_Parse(&text_p);

				if (!token[0]) {
					break;
				}

				ci->headOffset[i] = atof(token);
			}

			continue;
		} else if (!Q_stricmp(token, "sex")) {
			token = COM_Parse(&text_p);

			if (!token[0]) {
				break;
			}

			if (token[0] == 'f' || token[0] == 'F') {
				ci->gender = GENDER_FEMALE;
			} else if (token[0] == 'n' || token[0] == 'N') {
				ci->gender = GENDER_NEUTER;
			} else {
				ci->gender = GENDER_MALE;
			}

			continue;
		} else if (!Q_stricmp(token, "fixedlegs")) {
			ci->fixedlegs = qtrue;
			continue;
		} else if (!Q_stricmp(token, "fixedtorso")) {
			ci->fixedtorso = qtrue;
			continue;
		}
		// if it is a number, start parsing animations
		if (token[0] >= '0' && token[0] <= '9') {
			text_p = prev; // unget the token
			break;
		}

		Com_Printf("unknown token '%s' in %s\n", token, filename);
	}
	// read information for each frame
	for (i = 0; i < MAX_ANIMATIONS; i++) {
		token = COM_Parse(&text_p);

		if (!token[0]) {
			if (i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = qfalse;
				animations[i].flipflop = qfalse;
				continue;
			}

			break;
		}

		animations[i].firstFrame = atoi(token);
		// leg only frames are adjusted to not count the upper body only frames
		if (i == LEGS_WALKCR) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}

		if (i >= LEGS_WALKCR && i < TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse(&text_p);

		if (!token[0]) {
			break;
		}

		animations[i].numFrames = atoi(token);
		animations[i].reversed = qfalse;
		animations[i].flipflop = qfalse;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = qtrue;
		}

		token = COM_Parse(&text_p);

		if (!token[0]) {
			break;
		}

		animations[i].loopFrames = atoi(token);
		token = COM_Parse(&text_p);

		if (!token[0]) {
			break;
		}

		fps = atof(token);

		if (fps == 0) {
			fps = 1;
		}

		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if (i != MAX_ANIMATIONS) {
		CG_Printf("Error parsing animation file: %s\n", filename);
		return qfalse;
	}
	// crouch backward animation
	memcpy(&animations[LEGS_BACKCR], &animations[LEGS_WALKCR], sizeof(animation_t));

	animations[LEGS_BACKCR].reversed = qtrue;
	// walk backward animation
	memcpy(&animations[LEGS_BACKWALK], &animations[LEGS_WALK], sizeof(animation_t));

	animations[LEGS_BACKWALK].reversed = qtrue;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = qfalse;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = qfalse;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = qtrue;
	// new anims changes
//	animations[TORSO_GETFLAG].flipflop = qtrue;
//	animations[TORSO_GUARDBASE].flipflop = qtrue;
//	animations[TORSO_PATROL].flipflop = qtrue;
//	animations[TORSO_AFFIRMATIVE].flipflop = qtrue;
//	animations[TORSO_NEGATIVE].flipflop = qtrue;
	return qtrue;
}

/*
=======================================================================================================================================
CG_FileExists
=======================================================================================================================================
*/
static qboolean CG_FileExists(const char *filename) {
	int len;

	len = trap_FS_FOpenFile(filename, NULL, FS_READ);

	if (len > 0) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
CG_FindClientModelFile
=======================================================================================================================================
*/
static qboolean CG_FindClientModelFile(char *filename, int length, clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *base, const char *ext) {
	char *team;
	int i;

	if (cgs.gametype > GT_TOURNAMENT) {
		switch (ci->team) {
			case TEAM_RED: {
				team = "red";
				break;
			}

			default: {
				team = "blue";
				break;
			}
		}
	} else {
		team = "default";
	}

	for (i = 0; i < 2; i++) {
		if (i == 0 && teamName && *teamName) {
			//								"models/players/james/stroggs/lower_lily_red.skin"
			Com_sprintf(filename, length, "models/players/%s/%s%s_%s_%s.%s", modelName, teamName, base, skinName, team, ext);
		} else {
			//								"models/players/james/lower_lily_red.skin"
			Com_sprintf(filename, length, "models/players/%s/%s_%s_%s.%s", modelName, base, skinName, team, ext);
		}

		if (CG_FileExists(filename)) {
			return qtrue;
		}

		if (cgs.gametype > GT_TOURNAMENT) {
			if (i == 0 && teamName && *teamName) {
				//								"models/players/james/stroggs/lower_red.skin"
				Com_sprintf(filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, team, ext);
			} else {
				//								"models/players/james/lower_red.skin"
				Com_sprintf(filename, length, "models/players/%s/%s_%s.%s", modelName, base, team, ext);
			}
		} else {
			if (i == 0 && teamName && *teamName) {
				//								"models/players/james/stroggs/lower_lily.skin"
				Com_sprintf(filename, length, "models/players/%s/%s%s_%s.%s", modelName, teamName, base, skinName, ext);
			} else {
				//								"models/players/james/lower_lily.skin"
				Com_sprintf(filename, length, "models/players/%s/%s_%s.%s", modelName, base, skinName, ext);
			}
		}

		if (CG_FileExists(filename)) {
			return qtrue;
		}

		if (!teamName || !*teamName) {
			break;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
CG_FindClientHeadFile
=======================================================================================================================================
*/
static qboolean CG_FindClientHeadFile(char *filename, int length, clientInfo_t *ci, const char *teamName, const char *headModelName, const char *headSkinName, const char *base, const char *ext) {
	char *team, *headsFolder;
	int i;

	if (cgs.gametype > GT_TOURNAMENT) {
		switch (ci->team) {
			case TEAM_RED: {
				team = "red";
				break;
			}

			default: {
				team = "blue";
				break;
			}
		}
	} else {
		team = "default";
	}

	if (headModelName[0] == '*') {
		headsFolder = "heads/";
		headModelName++;
	} else {
		headsFolder = "";
	}

	while (1) {
		for (i = 0; i < 2; i++) {
			if (i == 0 && teamName && *teamName) {
				Com_sprintf(filename, length, "models/players/%s%s/%s/%s%s_%s.%s", headsFolder, headModelName, headSkinName, teamName, base, team, ext);
			} else {
				Com_sprintf(filename, length, "models/players/%s%s/%s/%s_%s.%s", headsFolder, headModelName, headSkinName, base, team, ext);
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);

				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if (CG_FileExists(filename)) {
				return qtrue;
			}

			if (cgs.gametype > GT_TOURNAMENT) {
				if (i == 0 && teamName && *teamName) {
					Com_sprintf(filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, team, ext);
				} else {
					Com_sprintf(filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, team, ext);
				}
			} else {
				if (i == 0 && teamName && *teamName) {
					Com_sprintf(filename, length, "models/players/%s%s/%s%s_%s.%s", headsFolder, headModelName, teamName, base, headSkinName, ext);
				} else {
					Com_sprintf(filename, length, "models/players/%s%s/%s_%s.%s", headsFolder, headModelName, base, headSkinName, ext);
				}
			}

			if (Q_stricmpn(ext, "$image", 6) == 0) {
				COM_StripExtension(filename, filename, length);

				if (trap_R_RegisterShaderNoMip(filename)) {
					return qtrue;
				}
			} else if (CG_FileExists(filename)) {
				return qtrue;
			}

			if (!teamName || !*teamName) {
				break;
			}
		}
		// if tried the heads folder first
		if (headsFolder[0]) {
			break;
		}

		headsFolder = "heads/";
	}

	return qfalse;
}

/*
=======================================================================================================================================
CG_AddSkinToFrame
=======================================================================================================================================
*/
qhandle_t CG_AddSkinToFrame(const cgSkin_t *skin, entityState_t *state) {
	qhandle_t surfaces[MAX_CG_SKIN_SURFACES];
	int i, index;
	float skinFraction;

	if (!skin || !skin->numMeshes) {
		return 0;
	}

	skinFraction = state ? state->skinFraction : 0.0f;

	for (i = 0; i < skin->numMeshes; i++) {
		if (skinFraction >= 1.0f) {
			index = skin->meshes[i].numShaders - 1;
		} else if (skinFraction <= 0.0f) {
			index = 0;
		} else { // > 0 && < 1
			index = skinFraction * skin->meshes[i].numShaders;
		}

		surfaces[i] = skin->meshes[i].surfaces[index];
	}

	return trap_R_AddSkinToFrame(skin->numMeshes, surfaces);
}

/*
=======================================================================================================================================
CG_RegisterSkin
=======================================================================================================================================
*/
qboolean CG_RegisterSkin(const char *name, cgSkin_t *skin, qboolean append) {
	char *text_p;
	int len;
	char *token;
	char text[20000];
	fileHandle_t f;
	char surfName[MAX_QPATH];
	char shaderName[MAX_QPATH];
	qhandle_t hShader;
	int initialSurfaces;
	int totalSurfaces;

	if (!name || !name[0]) {
		Com_Printf("Empty name passed to RE_RegisterSkin\n");
		return 0;
	}

	if (strlen(name) >= MAX_QPATH) {
		Com_Printf("Skin name exceeds MAX_QPATH\n");
		return 0;
	}

	if (!COM_CompareExtension(name, ".skin")) {
		Com_Printf("WARNING: CG_RegisterSkin ignoring '%s', must have \".skin\" extension\n", name);
		return 0;
	}

	if (!append) {
		skin->numMeshes = 0;
	}

	initialSurfaces = skin->numMeshes;
	totalSurfaces = skin->numMeshes;
	// load the file
	len = trap_FS_FOpenFile(name, &f, FS_READ);

	if (len <= 0) {
		return qfalse;
	}

	if (len >= sizeof(text) - 1) {
		CG_Printf("File %s too long\n", name);
		trap_FS_FCloseFile(f);
		return qfalse;
	}

	trap_FS_Read(text, len, f);

	text[len] = 0;

	trap_FS_FCloseFile(f);
	// parse the text
	text_p = text;

	while (text_p && *text_p) {
		// get surface name
		token = COM_ParseExt2(&text_p, qtrue, ',');

		Q_strncpyz(surfName, token, sizeof(surfName));

		if (!token[0]) {
			break;
		}

		if (*text_p == ',') {
			text_p++;
		}

		if (!Q_stricmpn(token, "tag_", 4)) {
			SkipRestOfLine(&text_p);
			continue;
		}
		// skip RTCW/ET skin settings
		if (!Q_stricmpn(token, "md3_", 4) || !Q_stricmp(token, "playerscale")) {
			SkipRestOfLine(&text_p);
			continue;
		}

		if (skin->numMeshes < MAX_CG_SKIN_SURFACES) {
			int numShaders;

			for (numShaders = 0; numShaders < MAX_CG_SKIN_SURFACE_SHADERS; numShaders++) {
				if (*text_p == ',') {
					text_p++;
				}
				// parse the shader name
				token = COM_ParseExt2(&text_p, qfalse, ',');
				Q_strncpyz(shaderName, token, sizeof(shaderName));

				if (!token[0]) {
					// end of line
					break;
				}

				hShader = trap_R_RegisterShaderEx(shaderName, LIGHTMAP_NONE, qtrue);
				// for compatibility with quake3 skins, don't render missing shaders listed in skins
				if (!hShader) {
					hShader = cgs.media.nodrawShader;
				}

				skin->meshes[skin->numMeshes].surfaces[numShaders] = trap_R_AllocSkinSurface(surfName, hShader);
			}

			skin->meshes[skin->numMeshes].numShaders = numShaders;
			skin->numMeshes++;
		}

		totalSurfaces++;
	}

	if (totalSurfaces > MAX_CG_SKIN_SURFACES) {
		CG_Printf("WARNING: Ignoring excess surfaces (found %d, max is %d) in skin '%s'!\n", totalSurfaces - initialSurfaces, MAX_CG_SKIN_SURFACES - initialSurfaces, name);
	}
	// failed to load surfaces
	if (!skin->numMeshes) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
CG_RegisterClientSkin
=======================================================================================================================================
*/
static qboolean CG_RegisterClientSkin(clientInfo_t *ci, const char *teamName, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName) {
	char filename[MAX_QPATH];
	qboolean legsSkin, torsoSkin, headSkin;

	legsSkin = torsoSkin = headSkin = qfalse;
	/*
	Com_sprintf(filename, sizeof(filename), "models/players/%s/%slower_%s.skin", modelName, teamName, skinName);

	ci->legsSkin = trap_R_RegisterSkin(filename);

	if (!ci->legsSkin) {
		Com_Printf("Leg skin load failure: %s\n", filename);
	}

	Com_sprintf(filename, sizeof(filename), "models/players/%s/%supper_%s.skin", modelName, teamName, skinName);

	ci->torsoSkin = trap_R_RegisterSkin(filename);

	if (!ci->torsoSkin) {
		Com_Printf("Torso skin load failure: %s\n", filename);
	}
	*/
	if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "lower", "skin")) {
		legsSkin = CG_RegisterSkin(filename, &ci->modelSkin, qfalse);
	}

	if (!legsSkin) {
		Com_Printf("Leg skin load failure: %s\n", filename);
	}

	if (CG_FindClientModelFile(filename, sizeof(filename), ci, teamName, modelName, skinName, "upper", "skin")) {
		torsoSkin = CG_RegisterSkin(filename, &ci->modelSkin, qtrue);
	}

	if (!torsoSkin) {
		Com_Printf("Torso skin load failure: %s\n", filename);
	}

	if (CG_FindClientHeadFile(filename, sizeof(filename), ci, teamName, headModelName, headSkinName, "head", "skin")) {
		headSkin = CG_RegisterSkin(filename, &ci->modelSkin, qtrue);
	}

	if (!headSkin) {
		Com_Printf("Head skin load failure: %s\n", filename);
	}
	// if any skins failed to load
	if (!legsSkin || !torsoSkin || !headSkin) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
CG_RegisterClientModelname
=======================================================================================================================================
*/
static qboolean CG_RegisterClientModelname(clientInfo_t *ci, const char *modelName, const char *skinName, const char *headModelName, const char *headSkinName, const char *teamName) {
	char filename[MAX_QPATH];
	const char *headName;
	char newTeamName[MAX_QPATH];

	if (headModelName[0] == '\0') {
		headName = modelName;
	} else {
		headName = headModelName;
	}

	Com_sprintf(filename, sizeof(filename), "models/players/%s/lower.md3", modelName);

	ci->legsModel = trap_R_RegisterModel(filename);

	if (!ci->legsModel) {
		Com_Printf("Failed to load model file %s\n", filename);
		return qfalse;
	}

	Com_sprintf(filename, sizeof(filename), "models/players/%s/upper.md3", modelName);

	ci->torsoModel = trap_R_RegisterModel(filename);

	if (!ci->torsoModel) {
		Com_Printf("Failed to load model file %s\n", filename);
		return qfalse;
	}

	if (headName[0] == '*') {
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", &headModelName[1], &headModelName[1]);
	} else {
		Com_sprintf(filename, sizeof(filename), "models/players/%s/head.md3", headName);
	}

	ci->headModel = trap_R_RegisterModel(filename);
	// if the head model could not be found and we didn't load from the heads folder try to load from there
	if (!ci->headModel && headName[0] != '*') {
		Com_sprintf(filename, sizeof(filename), "models/players/heads/%s/%s.md3", headModelName, headModelName);
		ci->headModel = trap_R_RegisterModel(filename);
	}

	if (!ci->headModel) {
		Com_Printf("Failed to load model file %s\n", filename);
		return qfalse;
	}
	// if any skins failed to load, return failure
	if (!CG_RegisterClientSkin(ci, teamName, modelName, skinName, headName, headSkinName)) {
		if (teamName && *teamName) {
			Com_Printf("Failed to load skin file: %s : %s : %s, %s : %s\n", teamName, modelName, skinName, headName, headSkinName);

			if (ci->team == TEAM_BLUE) {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_BLUETEAM_NAME);
			} else {
				Com_sprintf(newTeamName, sizeof(newTeamName), "%s/", DEFAULT_REDTEAM_NAME);
			}

			if (!CG_RegisterClientSkin(ci, newTeamName, modelName, skinName, headName, headSkinName)) {
				Com_Printf("Failed to load skin file: %s : %s : %s, %s : %s\n", newTeamName, modelName, skinName, headName, headSkinName);
				return qfalse;
			}
		} else {
			Com_Printf("Failed to load skin file: %s : %s, %s : %s\n", modelName, skinName, headName, headSkinName);
			return qfalse;
		}
	}
	// load the animations
	Com_sprintf(filename, sizeof(filename), "models/players/%s/animation.cfg", modelName);

	if (!CG_ParseAnimationFile(filename, ci)) {
		Com_Printf("Failed to load animation file %s\n", filename);
		return qfalse;
	}

	if (CG_FindClientHeadFile(filename, sizeof(filename), ci, teamName, headName, headSkinName, "icon", "$image")) {
		ci->modelIcon = trap_R_RegisterShaderNoMip(filename);
	} else {
		ci->modelIcon = 0;
	}

	if (!ci->modelIcon) {
		Com_Printf("Failed to load icon for %s/%s\n", headName, headSkinName);

		if (cg_buildScript.integer) {
			return qfalse;
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
CG_ColorFromIndex
=======================================================================================================================================
*/
void CG_ColorFromIndex(int val, vec3_t color) {

	switch (val) {
		case 1: // blue
		case 2: // green
		case 3: // cyan
		case 4: // red
		case 5: // magenta
		case 6: // yellow
		case 7: // white
			VectorClear(color);

			if (val & 1) {
				color[2] = 1.0f;
			}

			if (val & 2) {
				color[1] = 1.0f;
			}

			if (val & 4) {
				color[0] = 1.0f;
			}

			break;
		case 8: // orange
			VectorSet(color, 1, 0.5f, 0);
			break;
		case 9: // lime
			VectorSet(color, 0.5f, 1, 0);
			break;
		case 10: // vivid green
			VectorSet(color, 0, 1, 0.5f);
			break;
		case 11: // light blue
			VectorSet(color, 0, 0.5f, 1);
			break;
		case 12: // purple
			VectorSet(color, 0.5f, 0, 1);
			break;
		case 13: // pink
			VectorSet(color, 1, 0, 0.5f);
			break;
		default: // fall back to white
			VectorSet(color, 1, 1, 1);
			break;
	}
}

/*
=======================================================================================================================================
CG_ColorFromString
=======================================================================================================================================
*/
static void CG_ColorFromString(const char *v, vec3_t color) {
	int val;

	val = atoi(v);

	CG_ColorFromIndex(val, color);
}

/*
=======================================================================================================================================
CG_LoadClientInfo

Load it now, taking the disk hits. This will usually be deferred to a safe time.
=======================================================================================================================================
*/
static void CG_LoadClientInfo(int clientNum, clientInfo_t *ci) {
	const char *dir, *fallback;
	int i, modelloaded;
	const char *s;
	char teamname[MAX_QPATH];

	teamname[0] = 0;

	if (cgs.gametype > GT_TOURNAMENT) {
		if (ci->team == TEAM_BLUE) {
			Q_strncpyz(teamname, cg_blueTeamName.string, sizeof(teamname));
		} else {
			Q_strncpyz(teamname, cg_redTeamName.string, sizeof(teamname));
		}
	}

	if (teamname[0]) {
		strcat(teamname, "/");
	}

	modelloaded = qtrue;

	if (!CG_RegisterClientModelname(ci, ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname)) {
		if (cg_buildScript.integer) {
			CG_Error("CG_RegisterClientModelname(%s, %s, %s, %s %s) failed", ci->modelName, ci->skinName, ci->headModelName, ci->headSkinName, teamname);
		}
		// fall back to default team name
		if (cgs.gametype > GT_TOURNAMENT) {
			// keep skin name
			if (ci->team == TEAM_BLUE) {
				Q_strncpyz(teamname, DEFAULT_BLUETEAM_NAME, sizeof(teamname));
			} else {
				Q_strncpyz(teamname, DEFAULT_REDTEAM_NAME, sizeof(teamname));
			}

			if (!CG_RegisterClientModelname(ci, DEFAULT_TEAM_MODEL, ci->skinName, DEFAULT_TEAM_HEAD, ci->skinName, teamname)) {
				CG_Error("DEFAULT_TEAM_MODEL/skin (%s/%s) failed to register", DEFAULT_TEAM_MODEL, ci->skinName);
			}
		} else {
			if (!CG_RegisterClientModelname(ci, DEFAULT_MODEL, "default", DEFAULT_HEAD, "default", teamname)) {
				CG_Error("DEFAULT_MODEL (%s) failed to register", DEFAULT_MODEL);
			}
		}

		modelloaded = qfalse;
	}

	ci->newAnims = qfalse;

	if (ci->torsoModel) {
		orientation_t tag;
		// if the torso model has the "tag_flag"
		if (trap_R_LerpTag(&tag, ci->torsoModel, 0, 0, 1, "tag_flag")) {
			ci->newAnims = qtrue;
		}
	}
	// sounds
	dir = ci->modelName;

	if (cgs.gametype > GT_TOURNAMENT) {
		fallback = (ci->gender == GENDER_FEMALE) ? DEFAULT_TEAM_MODEL_FEMALE : DEFAULT_TEAM_MODEL_MALE;
	} else {
		fallback = (ci->gender == GENDER_FEMALE) ? DEFAULT_MODEL_FEMALE : DEFAULT_MODEL_MALE;
	}

	for (i = 0; i < MAX_CUSTOM_SOUNDS; i++) {
		s = cg_customSoundNames[i];

		if (!s) {
			break;
		}

		ci->sounds[i] = 0;
		// if the model didn't load use the sounds of the default model
		if (modelloaded) {
			ci->sounds[i] = trap_S_RegisterSound(va("snd/c/%s/%s", dir, s + 1), qfalse);
		}

		if (!ci->sounds[i]) {
			ci->sounds[i] = trap_S_RegisterSound(va("snd/c/%s/%s", fallback, s + 1), qfalse);
		}
	}

	ci->deferred = qfalse;
	// reset any existing players and bodies, because they might be in bad frames for this new model
	for (i = 0; i < MAX_GENTITIES; i++) {
		if (cg_entities[i].currentState.clientNum == clientNum && cg_entities[i].currentState.eType == ET_PLAYER) {
			CG_ResetPlayerEntity(&cg_entities[i]);
		}
	}
}

/*
=======================================================================================================================================
CG_CopyClientInfoModel
=======================================================================================================================================
*/
static void CG_CopyClientInfoModel(clientInfo_t *from, clientInfo_t *to) {

	VectorCopy(from->headOffset, to->headOffset);

	to->foottype = from->foottype;
	to->gender = from->gender;
	to->legsModel = from->legsModel;
	to->torsoModel = from->torsoModel;
	to->headModel = from->headModel;
	to->modelSkin = from->modelSkin;
	to->modelIcon = from->modelIcon;
	to->newAnims = from->newAnims;

	memcpy(to->animations, from->animations, sizeof(to->animations));
	memcpy(to->sounds, from->sounds, sizeof(to->sounds));
}

/*
=======================================================================================================================================
CG_ScanForExistingClientInfo
=======================================================================================================================================
*/
static qboolean CG_ScanForExistingClientInfo(clientInfo_t *ci) {
	int i;
	clientInfo_t *match;

	for (i = 0; i < cgs.maxclients; i++) {
		match = &cgs.clientinfo[i];

		if (!match->infoValid) {
			continue;
		}

		if (match->deferred) {
			continue;
		}

		if (!Q_stricmp(ci->modelName, match->modelName) && !Q_stricmp(ci->skinName, match->skinName) && !Q_stricmp(ci->headModelName, match->headModelName)
			&& !Q_stricmp(ci->headSkinName, match->headSkinName) && !Q_stricmp(ci->blueTeam, match->blueTeam) && !Q_stricmp(ci->redTeam, match->redTeam)
			&& (cgs.gametype < GT_TEAM || ci->team == match->team)) {
			// this clientinfo is identical, so use its handles
			ci->deferred = qfalse;

			CG_CopyClientInfoModel(match, ci);
			return qtrue;
		}
	}
	// nothing matches, so defer the load
	return qfalse;
}

/*
=======================================================================================================================================
CG_SetDeferredClientInfo

We aren't going to load it now, so grab some other client's info to use until we have some spare time.
=======================================================================================================================================
*/
static void CG_SetDeferredClientInfo(int clientNum, clientInfo_t *ci) {
	int i;
	clientInfo_t *match;

	// if someone else is already the same models and skins we can just load the client info
	for (i = 0; i < cgs.maxclients; i++) {
		match = &cgs.clientinfo[i];

		if (!match->infoValid || match->deferred) {
			continue;
		}

		if (Q_stricmp(ci->skinName, match->skinName) || Q_stricmp(ci->modelName, match->modelName) ||
//			Q_stricmp(ci->headModelName, match->headModelName) ||
//			Q_stricmp(ci->headSkinName, match->headSkinName) ||
			(cgs.gametype > GT_TOURNAMENT && ci->team != match->team)) {
			continue;
		}
		// just load the real info cause it uses the same models and skins
		CG_LoadClientInfo(clientNum, ci);
		return;
	}
	// if we are in teamplay, only grab a model if the skin is correct
	if (cgs.gametype > GT_TOURNAMENT) {
		for (i = 0; i < cgs.maxclients; i++) {
			match = &cgs.clientinfo[i];

			if (!match->infoValid || match->deferred) {
				continue;
			}

			if (Q_stricmp(ci->skinName, match->skinName) || (cgs.gametype > GT_TOURNAMENT && ci->team != match->team)) {
				continue;
			}

			ci->deferred = qtrue;

			CG_CopyClientInfoModel(match, ci);
			return;
		}
		// load the full model, because we don't ever want to show an improper team skin. This will cause a hitch for the first
		// player, when the second enters. Combat shouldn't be going on yet, so it shouldn't matter
		CG_LoadClientInfo(clientNum, ci);
		return;
	}
	// find the first valid clientinfo and grab its stuff
	for (i = 0; i < cgs.maxclients; i++) {
		match = &cgs.clientinfo[i];

		if (!match->infoValid) {
			continue;
		}

		ci->deferred = qtrue;
		CG_CopyClientInfoModel(match, ci);
		return;
	}
	// we should never get here...
	CG_Printf("CG_SetDeferredClientInfo: no valid clients!\n");
	CG_LoadClientInfo(clientNum, ci);
}

/*
=======================================================================================================================================
CG_NewClientInfo
=======================================================================================================================================
*/
void CG_NewClientInfo(int clientNum) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char *configstring;
	const char *v;
	char *slash;

	ci = &cgs.clientinfo[clientNum];
	configstring = CG_ConfigString(clientNum + CS_PLAYERS);

	if (!configstring[0]) {
		memset(ci, 0, sizeof(*ci));
		return; // player just left
	}
	// build into a temp buffer so the defer checks can use the old value
	memset(&newInfo, 0, sizeof(newInfo));
	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz(newInfo.name, v, sizeof(newInfo.name));
	// team
	v = Info_ValueForKey(configstring, "t");
	newInfo.team = atoi(v);
	// team task
	v = Info_ValueForKey(configstring, "tt");
	newInfo.teamTask = atoi(v);
	// team leader
	v = Info_ValueForKey(configstring, "tl");
	newInfo.teamLeader = atoi(v);

	Q_strncpyz(newInfo.redTeam, cg_redTeamName.string, MAX_TEAMNAME);
	Q_strncpyz(newInfo.blueTeam, cg_blueTeamName.string, MAX_TEAMNAME);
	// model
	v = Info_ValueForKey(configstring, "model");

	if (cg_forceModel.integer) {
		// forcemodel makes everyone use a single model to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if (cgs.gametype > GT_TOURNAMENT) {
			Q_strncpyz(newInfo.modelName, DEFAULT_TEAM_MODEL, sizeof(newInfo.modelName));
			Q_strncpyz(newInfo.skinName, "default", sizeof(newInfo.skinName));
		} else {
			trap_Cvar_VariableStringBuffer("model", modelStr, sizeof(modelStr));

			if ((skin = strchr(modelStr, '/')) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz(newInfo.modelName, modelStr, sizeof(newInfo.modelName));
			Q_strncpyz(newInfo.skinName, skin, sizeof(newInfo.skinName));
		}

		if (cgs.gametype > GT_TOURNAMENT) {
			// keep skin name
			slash = strchr(v, '/');

			if (slash) {
				Q_strncpyz(newInfo.skinName, slash + 1, sizeof(newInfo.skinName));
			}
		}
	} else {
		Q_strncpyz(newInfo.modelName, v, sizeof(newInfo.modelName));

		slash = strchr(newInfo.modelName, '/');

		if (!slash) {
			// modelName did not include a skin name
			Q_strncpyz(newInfo.skinName, "default", sizeof(newInfo.skinName));
		} else {
			Q_strncpyz(newInfo.skinName, slash + 1, sizeof(newInfo.skinName));
			// truncate modelName
			*slash = 0;
		}
	}
	// head model
	v = Info_ValueForKey(configstring, "hmodel");

	if (cg_forceModel.integer) {
		// forcemodel makes everyone use a single model to prevent load hitches
		char modelStr[MAX_QPATH];
		char *skin;

		if (cgs.gametype > GT_TOURNAMENT) {
			Q_strncpyz(newInfo.headModelName, DEFAULT_TEAM_HEAD, sizeof(newInfo.headModelName));
			Q_strncpyz(newInfo.headSkinName, "default", sizeof(newInfo.headSkinName));
		} else {
			trap_Cvar_VariableStringBuffer("headmodel", modelStr, sizeof(modelStr));

			if ((skin = strchr(modelStr, '/')) == NULL) {
				skin = "default";
			} else {
				*skin++ = 0;
			}

			Q_strncpyz(newInfo.headModelName, modelStr, sizeof(newInfo.headModelName));
			Q_strncpyz(newInfo.headSkinName, skin, sizeof(newInfo.headSkinName));
		}

		if (cgs.gametype > GT_TOURNAMENT) {
			// keep skin name
			slash = strchr(v, '/');

			if (slash) {
				Q_strncpyz(newInfo.headSkinName, slash + 1, sizeof(newInfo.headSkinName));
			}
		}
	} else {
		Q_strncpyz(newInfo.headModelName, v, sizeof(newInfo.headModelName));

		slash = strchr(newInfo.headModelName, '/');

		if (!slash) {
			// modelName did not include a skin name
			Q_strncpyz(newInfo.headSkinName, "default", sizeof(newInfo.headSkinName));
		} else {
			Q_strncpyz(newInfo.headSkinName, slash + 1, sizeof(newInfo.headSkinName));
			// truncate modelName
			*slash = 0;
		}
	}
	// scan for an existing clientinfo that matches this modelname so we can avoid loading checks if possible
	if (!CG_ScanForExistingClientInfo(&newInfo)) {
		qboolean forceDefer;

		forceDefer = trap_MemoryRemaining() < 4000000;
		// if we are deferring loads, just have it pick the first valid
		if (forceDefer || (cg_deferPlayers.integer && !cg_buildScript.integer && !cg.loading)) {
			// keep whatever they had if it won't violate team skins
			CG_SetDeferredClientInfo(clientNum, &newInfo);
			// if we are low on memory, leave them with this model
			if (forceDefer) {
				CG_Printf("Memory is low. Using deferred model.\n");
				newInfo.deferred = qfalse;
			}
		} else {
			CG_LoadClientInfo(clientNum, &newInfo);
		}
	}
	// colors
	v = Info_ValueForKey(configstring, "c1");
	CG_ColorFromString(v, newInfo.color1);

	newInfo.c1RGBA[0] = 255 * newInfo.color1[0];
	newInfo.c1RGBA[1] = 255 * newInfo.color1[1];
	newInfo.c1RGBA[2] = 255 * newInfo.color1[2];
	newInfo.c1RGBA[3] = 255;

	v = Info_ValueForKey(configstring, "c2");
	CG_ColorFromString(v, newInfo.color2);

	newInfo.c2RGBA[0] = 255 * newInfo.color2[0];
	newInfo.c2RGBA[1] = 255 * newInfo.color2[1];
	newInfo.c2RGBA[2] = 255 * newInfo.color2[2];
	newInfo.c2RGBA[3] = 255;
	// wins
	v = Info_ValueForKey(configstring, "w");
	newInfo.wins = atoi(v);
	// losses
	v = Info_ValueForKey(configstring, "l");
	newInfo.losses = atoi(v);
	// bot skill
	v = Info_ValueForKey(configstring, "skill");
	newInfo.botSkill = atoi(v);
	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
}

/*
=======================================================================================================================================
CG_LoadDeferredPlayers

Called each frame when a player is dead and the scoreboard is up, so deferred players can be loaded.
=======================================================================================================================================
*/
void CG_LoadDeferredPlayers(void) {
	int i;
	clientInfo_t *ci;

	// scan for a deferred player to load
	for (i = 0, ci = cgs.clientinfo; i < cgs.maxclients; i++, ci++) {
		if (ci->infoValid && ci->deferred) {
			// if we are low on memory, leave it deferred
			if (trap_MemoryRemaining() < 4000000) {
				CG_Printf("Memory is low. Using deferred model.\n");
				ci->deferred = qfalse;
				continue;
			}

			CG_LoadClientInfo(i, ci);
//			break;
		}
	}
}

/*
=======================================================================================================================================

	PLAYER ANIMATION

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_SetLerpFrameAnimation

May include ANIM_TOGGLEBIT.
=======================================================================================================================================
*/
static void CG_SetLerpFrameAnimation(clientInfo_t *ci, lerpFrame_t *lf, int newAnimation) {
	animation_t *anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if (newAnimation < 0 || newAnimation >= MAX_TOTALANIMATIONS) {
		CG_Error("Bad animation number: %i", newAnimation);
	}

	anim = &ci->animations[newAnimation];

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if (cg_debugAnim.integer) {
		CG_Printf("Anim: %i\n", newAnimation);
	}
}

/*
=======================================================================================================================================
CG_RunLerpFrame

Sets cg.snap, cg.oldFrame, and cg.backlerp. cg.time should be between oldFrameTime and frameTime after exit.
=======================================================================================================================================
*/
static void CG_RunLerpFrame(clientInfo_t *ci, lerpFrame_t *lf, int newAnimation, float speedScale) {
	int f, numFrames;
	animation_t *anim;

	// debugging tool to get no animations
	if (cg_animSpeed.integer == 0) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}
	// see if the animation sequence is switching
	if (newAnimation != lf->animationNumber || !lf->animation) {
		CG_SetLerpFrameAnimation(ci, lf, newAnimation);
	}
	// if we have passed the current frame, move it to oldFrame and calculate a new frame
	if (cg.time >= lf->frameTime) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;
		// get the next frame based on the animation
		anim = lf->animation;

		if (!anim->frameLerp) {
			return; // shouldn't happen
		}

		if (cg.time < lf->animationTime) {
			lf->frameTime = lf->animationTime; // initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}

		f = (lf->frameTime - lf->animationTime) / anim->frameLerp;
		f *= speedScale; // adjust for scout, etc.

		numFrames = anim->numFrames;

		if (anim->flipflop) {
			numFrames *= 2;
		}

		if (f >= numFrames) {
			f -= numFrames;

			if (anim->loopFrames) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it can immediately transition to another sequence
				lf->frameTime = cg.time;
			}
		}

		if (anim->reversed) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		} else if (anim->flipflop && f >= anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		} else {
			lf->frame = anim->firstFrame + f;
		}

		if (cg.time > lf->frameTime) {
			lf->frameTime = cg.time;

			if (cg_debugAnim.integer) {
				CG_Printf("Clamp lf->frameTime\n");
			}
		}
	}

	if (lf->frameTime > cg.time + 200) {
		lf->frameTime = cg.time;
	}

	if (lf->oldFrameTime > cg.time) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if (lf->frameTime == lf->oldFrameTime) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)(cg.time - lf->oldFrameTime) / (lf->frameTime - lf->oldFrameTime);
	}
}

/*
=======================================================================================================================================
CG_ClearLerpFrame
=======================================================================================================================================
*/
static void CG_ClearLerpFrame(clientInfo_t *ci, lerpFrame_t *lf, int animationNumber) {

	lf->frameTime = lf->oldFrameTime = cg.time;

	CG_SetLerpFrameAnimation(ci, lf, animationNumber);

	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}

/*
=======================================================================================================================================
CG_PlayerAnimation
=======================================================================================================================================
*/
static void CG_PlayerAnimation(centity_t *cent, int *legsOld, int *legs, float *legsBackLerp, int *torsoOld, int *torso, float *torsoBackLerp) {
	clientInfo_t *ci;
	int clientNum;
	float speedScale;

	clientNum = cent->currentState.clientNum;

	if (cg_noPlayerAnims.integer) {
		*legsOld = *legs = *torsoOld = *torso = 0;
		return;
	}

	if (cent->currentState.powerups & (1 << PW_SCOUT)) {
		speedScale = SCOUT_SPEED_SCALE;
	} else {
		speedScale = 1;
	}

	ci = &cgs.clientinfo[clientNum];
	// do the shuffle turn frames locally
	if (cent->pe.legs.yawing && (cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) == LEGS_IDLE) {
		CG_RunLerpFrame(ci, &cent->pe.legs, LEGS_TURN, speedScale);
	} else {
		CG_RunLerpFrame(ci, &cent->pe.legs, cent->currentState.legsAnim, speedScale);
	}

	*legsOld = cent->pe.legs.oldFrame;
	*legs = cent->pe.legs.frame;
	*legsBackLerp = cent->pe.legs.backlerp;

	CG_RunLerpFrame(ci, &cent->pe.torso, cent->currentState.torsoAnim, speedScale);

	*torsoOld = cent->pe.torso.oldFrame;
	*torso = cent->pe.torso.frame;
	*torsoBackLerp = cent->pe.torso.backlerp;
}

/*
=======================================================================================================================================

	PLAYER ANGLES

=======================================================================================================================================
*/

/*
=======================================================================================================================================
CG_SwingAngles
=======================================================================================================================================
*/
static void CG_SwingAngles(float destination, float swingTolerance, float clampTolerance, float speed, float *angle, qboolean *swinging) {
	float swing;
	float move;
	float scale;

	if (!*swinging) {
		// see if a swing should be started
		swing = AngleSubtract(*angle, destination);

		if (swing > swingTolerance || swing < -swingTolerance) {
			*swinging = qtrue;
		}
	}

	if (!*swinging) {
		return;
	}
	// modify the speed depending on the delta so it doesn't seem so linear
	swing = AngleSubtract(destination, *angle);
	scale = fabs(swing);

	if (scale < swingTolerance * 0.5) {
		scale = 0.5;
	} else if (scale < swingTolerance) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}
	// swing towards the destination angle
	if (swing >= 0) {
		move = cg.frametime * scale * speed;

		if (move >= swing) {
			move = swing;
			*swinging = qfalse;
		}

		*angle = AngleMod(*angle + move);
	} else if (swing < 0) {
		move = cg.frametime * scale * -speed;

		if (move <= swing) {
			move = swing;
			*swinging = qfalse;
		}

		*angle = AngleMod(*angle + move);
	}
	// clamp to no more than tolerance
	swing = AngleSubtract(destination, *angle);

	if (swing > clampTolerance) {
		*angle = AngleMod(destination - (clampTolerance - 1));
	} else if (swing < -clampTolerance) {
		*angle = AngleMod(destination + (clampTolerance - 1));
	}
}

/*
=======================================================================================================================================
CG_AddPainTwitch
=======================================================================================================================================
*/
static void CG_AddPainTwitch(centity_t *cent, vec3_t torsoAngles) {
	int t;
	float f;

	t = cg.time - cent->pe.painTime;

	if (t >= PAIN_TWITCH_TIME) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if (cent->pe.painDirection) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}

/*
=======================================================================================================================================
CG_PlayerAngles

Handles separate torso motion.

Legs pivot based on direction of movement. Head always looks exactly at cent->lerpAngles.

If motion < 20 degrees, show in head only. If < 45 degrees, also show in torso.
=======================================================================================================================================
*/
static void CG_PlayerAngles(centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3]) {
	vec3_t legsAngles, torsoAngles, headAngles;
	float dest;
	static int movementOffsets[8] = {0, 22, 45, -22, 0, 22, -45, -22};
#ifndef BASEGAME
	vec3_t velocity;
	float speed;
#endif
	int dir, clientNum;
	clientInfo_t *ci;

	VectorCopy(cent->lerpAngles, headAngles);
	headAngles[YAW] = AngleMod(headAngles[YAW]);
	VectorClear(legsAngles);
	VectorClear(torsoAngles);

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ((cent->currentState.legsAnim & ~ANIM_TOGGLEBIT) != LEGS_IDLE || ((cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND && (cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT) != TORSO_STAND2)) {
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue; // always center
		cent->pe.torso.pitching = qtrue; // always center
		cent->pe.legs.yawing = qtrue; // always center
	}
	// adjust legs for movement dir
	if (cent->currentState.eFlags & EF_DEAD) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];

		if (dir < 0 || dir > 7) {
			CG_Error("Bad player movement angle");
		}
	}

	legsAngles[YAW] = headAngles[YAW] + movementOffsets[dir];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[dir];
	// torso
	CG_SwingAngles(torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing);
	CG_SwingAngles(legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing);

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if (headAngles[PITCH] > 180) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}

	CG_SwingAngles(dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching);

	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;
	clientNum = cent->currentState.clientNum;

	if (clientNum >= 0 && clientNum < MAX_CLIENTS) {
		ci = &cgs.clientinfo[clientNum];

		if (ci->fixedtorso) {
			torsoAngles[PITCH] = 0.0f;
		}
	}
#ifndef BASEGAME
	// --------- roll -------------

	// lean towards the direction of travel
	VectorCopy(cent->currentState.pos.trDelta, velocity);

	speed = VectorNormalize(velocity);

	if (speed) {
		vec3_t axis[3];
		float side;

		speed *= 0.05f;

		AnglesToAxis(legsAngles, axis);

		side = speed * DotProduct(velocity, axis[1]);
		legsAngles[ROLL] -= side;

		side = speed * DotProduct(velocity, axis[0]);
		legsAngles[PITCH] += side;
	}
#endif
	clientNum = cent->currentState.clientNum;

	if (clientNum >= 0 && clientNum < MAX_CLIENTS) {
		ci = &cgs.clientinfo[clientNum];

		if (ci->fixedlegs) {
			legsAngles[YAW] = torsoAngles[YAW];
			legsAngles[PITCH] = 0.0f;
			legsAngles[ROLL] = 0.0f;
		}
	}
	// pain twitch
	CG_AddPainTwitch(cent, torsoAngles);
	// pull the angles back out of the hierarchical chain
	AnglesSubtract(headAngles, torsoAngles, headAngles);
	AnglesSubtract(torsoAngles, legsAngles, torsoAngles);
	AnglesToAxis(legsAngles, legs);
	AnglesToAxis(torsoAngles, torso);
	AnglesToAxis(headAngles, head);
}

/*
=======================================================================================================================================
CG_BreathPuff
=======================================================================================================================================
*/
static void CG_BreathPuff(int clientNum, vec3_t origin, vec3_t *axis) {
	int contents;
	localEntity_t *puffs[3 + 5] = {0};

	contents = CG_PointContents(origin, 0);

	if (contents & (CONTENTS_WATER|CONTENTS_SLIME)) {
		CG_SpawnBubbles(puffs, origin, 2, (int)(3 + random() * 5));
	} else {
		if (cg_enableBreath.integer) {
			vec3_t up;

			VectorSet(up, 0, 0, 8);

			puffs[0] = CG_SmokePuff(origin, up, 16, 1, 1, 1, 0.66f, 1500, cg.time, cg.time + 400, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
		}
	}
}

/*
=======================================================================================================================================
CG_AddBreathPuffs
=======================================================================================================================================
*/
static void CG_AddBreathPuffs(centity_t *cent, refEntity_t *head) {
	clientInfo_t *ci;
	vec3_t origin;

	if (cent->currentState.number >= MAX_CLIENTS) {
		return;
	}

	ci = &cgs.clientinfo[cent->currentState.number];

	if (cent->currentState.eFlags & EF_DEAD) {
		return;
	}

	if (ci->breathPuffTime > cg.time) {
		return;
	}
	// add first person effects
	if (cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson) {
		VectorMA(cg.refdef.vieworg, 20, cg.refdef.viewaxis[0], origin);
		VectorMA(origin, -4, cg.refdef.viewaxis[2], origin);
		CG_BreathPuff(cent->currentState.number, origin, cg.refdef.viewaxis);
	}
	// add third person effects for mirrors
	VectorMA(head->origin, 8, head->axis[0], origin);
	VectorMA(origin, -4, head->axis[2], origin);
	CG_BreathPuff(cent->currentState.number, origin, head->axis);

	ci->breathPuffTime = cg.time + 2000;
}

/*
=======================================================================================================================================
CG_DustTrail
=======================================================================================================================================
*/
static void CG_DustTrail(centity_t *cent) {
	int anim;
	vec3_t end, vel;
	trace_t tr;

	if (!cg_enableDust.integer) {
		return;
	}

	if (cent->dustTrailTime > cg.time) {
		return;
	}

	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;

	if (anim != LEGS_LANDB && anim != LEGS_LAND) {
		return;
	}

	cent->dustTrailTime += 40;

	if (cent->dustTrailTime < cg.time) {
		cent->dustTrailTime = cg.time;
	}

	VectorCopy(cent->currentState.pos.trBase, end);

	end[2] -= 64;

	CG_Trace(&tr, cent->currentState.pos.trBase, NULL, NULL, end, cent->currentState.number, MASK_PLAYERSOLID);
// Tobias FIXME: do some simplifications here (like 'isSoftMaterial' etc.), after everything is done...
	if ((tr.surfaceFlags & SURF_MATERIAL_MASK) != MAT_SAND_GR_COL_01 && (tr.surfaceFlags & SURF_MATERIAL_MASK) != MAT_SAND_GR_COL_02 && (tr.surfaceFlags & SURF_MATERIAL_MASK) != MAT_SAND_GR_COL_03 && (tr.surfaceFlags & SURF_MATERIAL_MASK) != MAT_SAND_GR_COL_04) {
		return;
	}
// Tobias END
	VectorCopy(cent->currentState.pos.trBase, end);

	end[2] -= 16;

	VectorSet(vel, 0, 0, -30);
	CG_SmokePuff(end, vel, 24, .8f, .8f, 0.7f, 0.33f, 500, cg.time, 0, 0, cgs.media.dustPuffShader);
}

/*
=======================================================================================================================================
CG_TrailItem
=======================================================================================================================================
*/
static void CG_TrailItem(centity_t *cent, qhandle_t hModel) {
	refEntity_t ent;
	vec3_t angles;
	vec3_t axis[3];

	VectorCopy(cent->lerpAngles, angles);

	angles[PITCH] = 0;
	angles[ROLL] = 0;

	AnglesToAxis(angles, axis);

	memset(&ent, 0, sizeof(ent));

	VectorMA(cent->lerpOrigin, -16, axis[0], ent.origin);

	ent.origin[2] += 16;
	angles[YAW] += 90;

	AnglesToAxis(angles, ent.axis);

	ent.hModel = hModel;

	trap_R_AddRefEntityToScene(&ent);
}

/*
=======================================================================================================================================
CG_PlayerFlag
=======================================================================================================================================
*/
static void CG_PlayerFlag(centity_t *cent, const cgSkin_t *skin, refEntity_t *torso) {
	clientInfo_t *ci;
	refEntity_t pole;
	refEntity_t flag;
	vec3_t angles, dir;
	int legsAnim, flagAnim, updateangles;
	float angle, d;

	// show the flag pole model
	memset(&pole, 0, sizeof(pole));

	pole.hModel = cgs.media.flagPoleModel;

	VectorCopy(torso->lightingOrigin, pole.lightingOrigin);

	pole.shadowPlane = torso->shadowPlane;
	pole.renderfx = torso->renderfx;

	CG_PositionEntityOnTag(&pole, torso, torso->hModel, "tag_flag");
	trap_R_AddRefEntityToScene(&pole);
	// show the flag model
	memset(&flag, 0, sizeof(flag));

	flag.hModel = cgs.media.flagFlapModel;
	flag.customSkin = CG_AddSkinToFrame(skin, &cent->currentState);

	VectorCopy(torso->lightingOrigin, flag.lightingOrigin);

	flag.shadowPlane = torso->shadowPlane;
	flag.renderfx = torso->renderfx;

	VectorClear(angles);

	updateangles = qfalse;
	legsAnim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

	if (legsAnim == LEGS_IDLE || legsAnim == LEGS_IDLECR) {
		flagAnim = FLAG_STAND;
	} else if (legsAnim == LEGS_WALK || legsAnim == LEGS_WALKCR) {
		flagAnim = FLAG_STAND;
		updateangles = qtrue;
	} else {
		flagAnim = FLAG_RUN;
		updateangles = qtrue;
	}

	if (updateangles) {
		VectorCopy(cent->currentState.pos.trDelta, dir);
		// add gravity
		dir[2] += 100;
		VectorNormalize(dir);
		d = DotProduct(pole.axis[2], dir);
		// if there is enough movement orthogonal to the flag pole
		if (fabs(d) < 0.9) {
			d = DotProduct(pole.axis[0], dir);

			if (d > 1.0f) {
				d = 1.0f;
			} else if (d < -1.0f) {
				d = -1.0f;
			}

			angle = acos(d);
			d = DotProduct(pole.axis[1], dir);

			if (d < 0) {
				angles[YAW] = 360 - angle * 180 / M_PI;
			} else {
				angles[YAW] = angle * 180 / M_PI;
			}

			if (angles[YAW] < 0) {
				angles[YAW] += 360;
			}

			if (angles[YAW] > 360) {
				angles[YAW] -= 360;
			}

			//VectorToAngles(cent->currentState.pos.trDelta, tmpangles);
			//angles[YAW] = tmpangles[YAW] + 45 - cent->pe.torso.yawAngle;
			// change the yaw angle
			CG_SwingAngles(angles[YAW], 25, 90, 0.15f, &cent->pe.flag.yawAngle, &cent->pe.flag.yawing);
		}
		/*
		d = DotProduct(pole.axis[2], dir);
		angle = Q_acos(d);

		d = DotProduct(pole.axis[1], dir);

		if (d < 0) {
			angle = 360 - angle * 180 / M_PI;
		} else {
			angle = angle * 180 / M_PI;
		}

		if (angle > 340 && angle < 20) {
			flagAnim = FLAG_RUNUP;
		}

		if (angle > 160 && angle < 200) {
			flagAnim = FLAG_RUNDOWN;
		}
		*/
	}
	// set the yaw angle
	angles[YAW] = cent->pe.flag.yawAngle;
	// lerp the flag animation frames
	ci = &cgs.clientinfo[cent->currentState.clientNum];

	CG_RunLerpFrame(ci, &cent->pe.flag, flagAnim, 1);

	flag.oldframe = cent->pe.flag.oldFrame;
	flag.frame = cent->pe.flag.frame;
	flag.backlerp = cent->pe.flag.backlerp;

	AnglesToAxis(angles, flag.axis);
	CG_PositionRotatedEntityOnTag(&flag, &pole, pole.hModel, "tag_flag");
	trap_R_AddRefEntityToScene(&flag);
}

/*
=======================================================================================================================================
CG_PlayerTokens
=======================================================================================================================================
*/
static void CG_PlayerTokens(centity_t *cent, int renderfx) {
	int tokens, i, j;
	float angle;
	refEntity_t ent;
	vec3_t dir, origin;
	skulltrail_t *trail;

	if (cent->currentState.number >= MAX_CLIENTS) {
		return;
	}

	trail = &cg.skulltrails[cent->currentState.number];
	tokens = cent->currentState.tokens;

	if (!tokens) {
		trail->numpositions = 0;
		return;
	}

	if (tokens > MAX_SKULLTRAIL) {
		tokens = MAX_SKULLTRAIL;
	}
	// add skulls if there are more than last time
	for (i = 0; i < tokens - trail->numpositions; i++) {
		for (j = trail->numpositions; j > 0; j--) {
			VectorCopy(trail->positions[j - 1], trail->positions[j]);
		}

		VectorCopy(cent->lerpOrigin, trail->positions[0]);
	}

	trail->numpositions = tokens;
	// move all the skulls along the trail
	VectorCopy(cent->lerpOrigin, origin);

	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(trail->positions[i], origin, dir);

		if (VectorNormalize(dir) > 30) {
			VectorMA(origin, 30, dir, trail->positions[i]);
		}

		VectorCopy(trail->positions[i], origin);
	}

	memset(&ent, 0, sizeof(ent));

	if (cgs.clientinfo[cent->currentState.clientNum].team == TEAM_BLUE) {
		ent.hModel = cgs.media.redCubeModel;
	} else {
		ent.hModel = cgs.media.blueCubeModel;
	}

	ent.renderfx = renderfx;

	VectorCopy(cent->lerpOrigin, origin);

	for (i = 0; i < trail->numpositions; i++) {
		VectorSubtract(origin, trail->positions[i], ent.axis[0]);

		ent.axis[0][2] = 0;

		VectorNormalize(ent.axis[0]);
		VectorSet(ent.axis[2], 0, 0, 1);
		CrossProduct(ent.axis[0], ent.axis[2], ent.axis[1]);
		VectorCopy(trail->positions[i], ent.origin);

		angle = (((cg.time + 500 * MAX_SKULLTRAIL - 500 * i) / 16) & 255) * (M_PI * 2) / 255;
		ent.origin[2] += sin(angle) * 10;

		trap_R_AddRefEntityToScene(&ent);
		VectorCopy(trail->positions[i], origin);
	}
}

/*
=======================================================================================================================================
CG_PlayerPowerups
=======================================================================================================================================
*/
static void CG_PlayerPowerups(centity_t *cent, refEntity_t *torso) {
	int powerups;
	clientInfo_t *ci;

	powerups = cent->currentState.powerups;

	if (!powerups) {
		return;
	}
	// quad gives a dlight
	if (powerups & (1 << PW_QUAD)) {
		trap_R_AddLightToScene(cent->lerpOrigin, 200 + (rand()&31), 1.0f, 0.2f, 0.2f, 1, 0);
	}

	ci = &cgs.clientinfo[cent->currentState.clientNum];
	// redflag
	if (powerups & (1 << PW_REDFLAG)) {
		if (ci->newAnims) {
			CG_PlayerFlag(cent, &cgs.media.redFlagFlapSkin, torso);
		} else {
			CG_TrailItem(cent, cgs.media.redFlagModel);
		}
	}
	// blueflag
	if (powerups & (1 << PW_BLUEFLAG)) {
		if (ci->newAnims) {
			CG_PlayerFlag(cent, &cgs.media.blueFlagFlapSkin, torso);
		} else {
			CG_TrailItem(cent, cgs.media.blueFlagModel);
		}
	}
	// neutralflag
	if (powerups & (1 << PW_NEUTRALFLAG)) {
		if (ci->newAnims) {
			CG_PlayerFlag(cent, &cgs.media.neutralFlagFlapSkin, torso);
		} else {
			CG_TrailItem(cent, cgs.media.neutralFlagModel);
		}
	}
}

/*
=======================================================================================================================================
CG_PlayerFloatSprite

Float a sprite over the player's head.
=======================================================================================================================================
*/
static void CG_PlayerFloatSprite(vec3_t origin, int rf, qhandle_t shader) {
	refEntity_t ent;

	memset(&ent, 0, sizeof(ent));

	VectorCopy(origin, ent.origin);

	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;

	trap_R_AddRefEntityToScene(&ent);
}

/*
=======================================================================================================================================
CG_PlayerSprites

Float sprites over the player's head.
=======================================================================================================================================
*/
static void CG_PlayerSprites(centity_t *cent, const refEntity_t *parent) {
	int friendFlags, thirdPersonFlags, team;
	vec3_t origin;
// Tobias DEBUG
	clientInfo_t *ci;
	int clientNum;

	clientNum = cent->currentState.clientNum;
	ci = &cgs.clientinfo[clientNum];
// Tobias END
	VectorCopy(parent->origin, origin);

	origin[2] += 42;

	if (cent->currentState.number == cg.snap->ps.clientNum) {
		// current player's team sprite should only be shown in mirrors
		friendFlags = RF_ONLY_MIRROR;

		if (!cg.renderingThirdPerson) {
			thirdPersonFlags = RF_ONLY_MIRROR;
		} else {
			thirdPersonFlags = 0;
		}
	} else {
		friendFlags = thirdPersonFlags = 0;
	}
// Tobias DEBUG
	if (cg_drawDebug.integer) {
		if (cg_drawStatusDebug.integer) {
			if (cgs.gametype > GT_TOURNAMENT) {
				if (ci) {
					qhandle_t h;

					h = CG_StatusHandle(ci->teamTask);
					CG_PlayerFloatSprite(origin, thirdPersonFlags, h);
					return;
				}
			}
		}
#ifdef TOBIAS_OBSTACLEDEBUG
		else if (cg_drawObstacleDebug.integer) {
			if (ci) {
				qhandle_t h;

				h = CG_ObstacleHandle(ci->teamTask);
				CG_PlayerFloatSprite(origin, thirdPersonFlags, h);
				return;
			}
		}
#endif
	}
// Tobias END
	if (cent->currentState.eFlags & EF_CONNECTION) {
		CG_PlayerFloatSprite(origin, thirdPersonFlags, cgs.media.connectionShader);
		return;
	}

	if (cent->currentState.eFlags & EF_TALK) {
		CG_PlayerFloatSprite(origin, thirdPersonFlags, cgs.media.balloonShader);
		return;
	}

	team = cgs.clientinfo[cent->currentState.clientNum].team;

	if (!(cent->currentState.eFlags & EF_DEAD) && cg.snap->ps.persistant[PERS_TEAM] == team && cgs.gametype > GT_TOURNAMENT) {
		if (cg_drawFriend.integer) {
			if (team == TEAM_BLUE) {
				CG_PlayerFloatSprite(origin, friendFlags, cgs.media.blueFriendShader);
			} else {
				CG_PlayerFloatSprite(origin, friendFlags, cgs.media.redFriendShader);
			}
		}

		return;
	}
}

#define SHADOW_DISTANCE 128
/*
=======================================================================================================================================
CG_PlayerShadow

Returns the Z component of the surface being shadowed. Should it return a full plane instead of a Z?
=======================================================================================================================================
*/
static qboolean CG_PlayerShadow(centity_t *cent, float *shadowPlane) {
	vec3_t end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t trace;
	float alpha;

	*shadowPlane = 0;

	if (cg_shadows.integer == 0) {
		return qfalse;
	}
	// no shadows when invisible
	if (cent->currentState.powerups & (1 << PW_INVIS)) {
		return qfalse;
	}
	// send a trace down from the player to the ground
	VectorCopy(cent->lerpOrigin, end);

	end[2] -= SHADOW_DISTANCE;

	trap_CM_BoxTrace(&trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID);
	// no shadow if too high
	if (trace.fraction == 1.0 || trace.startsolid || trace.allsolid) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if (cg_shadows.integer != 1) { // no mark for stencil or projection shadows
		return qtrue;
	}
	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;
	// hack/FPE - bogus planes?
	//assert(DotProduct(trace.plane.normal, trace.plane.normal) != 0.0f)
	// add the mark as a temporary, so it goes directly to the renderer without taking a spot in the cg_marks array
	CG_ImpactMark(cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, cent->pe.legs.yawAngle, alpha, alpha, alpha, 1, qfalse, 24, qtrue);
	return qtrue;
}

/*
=======================================================================================================================================
CG_PlayerSplash

Draw a mark at the water surface.
=======================================================================================================================================
*/
static void CG_PlayerSplash(centity_t *cent) {
	vec3_t start, end;
	trace_t trace;
	int contents;
	polyVert_t verts[4];

	if (!cg_shadows.integer) {
		return;
	}

	VectorCopy(cent->lerpOrigin, end);

	end[2] -= 24;
	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = CG_PointContents(end, 0);

	if (!(contents & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA))) {
		return;
	}

	VectorCopy(cent->lerpOrigin, start);

	start[2] += 32;
	// if the head isn't out of liquid, don't make a mark
	contents = CG_PointContents(start, 0);

	if (contents & (CONTENTS_SOLID|CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) {
		return;
	}
	// trace down to find the surface
	trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA));

	if (trace.fraction == 1.0) {
		return;
	}
	// create a mark polygon
	VectorCopy(trace.endpos, verts[0].xyz);
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[1].xyz);
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[2].xyz);
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy(trace.endpos, verts[3].xyz);
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.wakeMarkShader, 4, verts, 0, 0);
}

/*
=======================================================================================================================================
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups. Also called by CG_Missile for quad rockets, but nobody can tell...
=======================================================================================================================================
*/
void CG_AddRefEntityWithPowerups(refEntity_t *ent, entityState_t *state) {

	if (state->powerups & (1 << PW_INVIS)) {
		if (state->team == TEAM_RED) {
			ent->customShader = cgs.media.invisRedShader;
		} else if (state->team == TEAM_BLUE) {
			ent->customShader = cgs.media.invisBlueShader;
		} else {
			ent->customShader = cgs.media.invisShader;
		}

		trap_R_AddRefEntityToScene(ent);
	} else {
		trap_R_AddRefEntityToScene(ent);

		if (state->powerups & (1 << PW_QUAD)) {
			ent->customShader = cgs.media.quadShader;
			trap_R_AddRefEntityToScene(ent);
		}

		if (state->powerups & (1 << PW_REGEN)) {
			if (((cg.time / 100) % 10) == 1) {
				ent->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene(ent);
			}
		}
	}
}

/*
=======================================================================================================================================
CG_LightVerts
=======================================================================================================================================
*/
int CG_LightVerts(vec3_t normal, int numVerts, polyVert_t *verts) {
	int i, j;
	float incoming;
	vec3_t ambientLight;
	vec3_t lightDir;
	vec3_t directedLight;

	trap_R_LightForPoint(verts[0].xyz, ambientLight, directedLight, lightDir);

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct(normal, lightDir);

		if (incoming <= 0) {
			verts[i].modulate[0] = ambientLight[0];
			verts[i].modulate[1] = ambientLight[1];
			verts[i].modulate[2] = ambientLight[2];
			verts[i].modulate[3] = 255;
			continue;
		}

		j = (ambientLight[0] + incoming * directedLight[0]);

		if (j > 255) {
			j = 255;
		}

		verts[i].modulate[0] = j;

		j = (ambientLight[1] + incoming * directedLight[1]);

		if (j > 255) {
			j = 255;
		}

		verts[i].modulate[1] = j;

		j = (ambientLight[2] + incoming * directedLight[2]);

		if (j > 255) {
			j = 255;
		}

		verts[i].modulate[2] = j;
		verts[i].modulate[3] = 255;
	}

	return qtrue;
}

/*
=======================================================================================================================================
CG_Player
=======================================================================================================================================
*/
void CG_Player(centity_t *cent) {
	clientInfo_t *ci;
	refEntity_t legs;
	refEntity_t torso;
	refEntity_t head;
	int clientNum;
	int renderfx;
	qboolean shadow;
	float shadowPlane;
	refEntity_t skull;
	refEntity_t powerup;
	int t;
	float c;
	float angle;
	vec3_t dir, angles;

	// the client number is stored in clientNum. It can't be derived from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;

	if (clientNum < 0 || clientNum >= MAX_CLIENTS) {
		CG_Error("Bad clientNum on player entity");
	}

	ci = &cgs.clientinfo[clientNum];
	// it is possible to see corpses from disconnected players that may not have valid clientinfo
	if (!ci->infoValid) {
		return;
	}
	// get the player model information
	renderfx = 0;

	if (cent->currentState.number == cg.snap->ps.clientNum) {
		if (!cg.renderingThirdPerson) {
			renderfx = RF_ONLY_MIRROR; // only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}

	memset(&legs, 0, sizeof(legs));
	memset(&torso, 0, sizeof(torso));
	memset(&head, 0, sizeof(head));
	// get the rotation information
	CG_PlayerAngles(cent, legs.axis, torso.axis, head.axis);
// Tobias HACK: make player models (visually) bigger until we have new models... this way we can test our new model specific hit boxes without looking too silly
	if (!strcmp(ci->modelName, "razor")) {
		VectorScale(legs.axis[0], 1.39, legs.axis[0]);
		VectorScale(legs.axis[1], 1.39, legs.axis[1]);
		VectorScale(legs.axis[2], 1.39, legs.axis[2]);
		cent->lerpOrigin[2] += 9;
	} else if ((!strcmp(ci->modelName, "janet")) || (!strcmp(ci->modelName, "james"))) {
		VectorScale(legs.axis[0], 1.12, legs.axis[0]);
		VectorScale(legs.axis[1], 1.12, legs.axis[1]);
		VectorScale(legs.axis[2], 1.12, legs.axis[2]);
		cent->lerpOrigin[2] += 3;
	} else if ((!strcmp(ci->modelName, "major")) || (!strcmp(ci->modelName, "pi")) || (!strcmp(ci->modelName, "morgan")) || (!strcmp(ci->modelName, "sorlag"))) {
		VectorScale(legs.axis[0], 1.20, legs.axis[0]);
		VectorScale(legs.axis[1], 1.20, legs.axis[1]);
		VectorScale(legs.axis[2], 1.20, legs.axis[2]);
		cent->lerpOrigin[2] += 5;
	} else if ((!strcmp(ci->modelName, "fritzkrieg")) || (!strcmp(ci->modelName, "biker"))) {
		VectorScale(legs.axis[0], 1.18, legs.axis[0]);
		VectorScale(legs.axis[1], 1.18, legs.axis[1]);
		VectorScale(legs.axis[2], 1.18, legs.axis[2]);
		cent->lerpOrigin[2] += 4;
	} else if ((!strcmp(ci->modelName, "vex")) || (!strcmp(ci->modelName, "slash"))) {
		VectorScale(legs.axis[0], 0.97, legs.axis[0]);
		VectorScale(legs.axis[1], 0.97, legs.axis[1]);
		VectorScale(legs.axis[2], 0.97, legs.axis[2]);
		cent->lerpOrigin[2] -= 1;
	} else if (!strcmp(ci->modelName, "tarantula")) {
		VectorScale(legs.axis[0], 0.25, legs.axis[0]);
		VectorScale(legs.axis[1], 0.25, legs.axis[1]);
		VectorScale(legs.axis[2], 0.25, legs.axis[2]);
		cent->lerpOrigin[2] -= 17;
	} else {
		VectorScale(legs.axis[0], 1.25, legs.axis[0]);
		VectorScale(legs.axis[1], 1.25, legs.axis[1]);
		VectorScale(legs.axis[2], 1.25, legs.axis[2]);
		cent->lerpOrigin[2] += 6;
	}

	legs.nonNormalizedAxes = qtrue;
// Tobias END
	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation(cent, &legs.oldframe, &legs.frame, &legs.backlerp, &torso.oldframe, &torso.frame, &torso.backlerp);
	// add the shadow
	shadow = CG_PlayerShadow(cent, &shadowPlane);
	// add a water splash if partially in and out of water
	CG_PlayerSplash(cent);

	if (cg_shadows.integer == 3 && shadow) {
		renderfx |= RF_SHADOW_PLANE;
	}

	renderfx |= RF_LIGHTING_ORIGIN; // use the same origin for all

	if (cgs.gametype == GT_HARVESTER) {
		CG_PlayerTokens(cent, renderfx);
	}
	// add the legs
	legs.hModel = ci->legsModel;
	legs.customSkin = CG_AddSkinToFrame(&ci->modelSkin, &cent->currentState);

	VectorCopy(cent->lerpOrigin, legs.origin);
	VectorCopy(cent->lerpOrigin, legs.lightingOrigin);

	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;

	VectorCopy(legs.origin, legs.oldorigin); // don't positionally lerp at all
	Byte4Copy(ci->c1RGBA, legs.shaderRGBA);

	legs.shaderRGBA[3] = cent->currentState.skinFraction * 255;

	CG_AddRefEntityWithPowerups(&legs, &cent->currentState);
	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}
	// add the torso
	torso.hModel = ci->torsoModel;

	if (!torso.hModel) {
		return;
	}

	torso.customSkin = legs.customSkin; // Tobias CHECK: legs?

	VectorCopy(cent->lerpOrigin, torso.lightingOrigin);
	CG_PositionRotatedEntityOnTag(&torso, &legs, ci->legsModel, "tag_torso");

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	Byte4Copy(ci->c1RGBA, torso.shaderRGBA);

	torso.shaderRGBA[3] = cent->currentState.skinFraction * 255;

	CG_AddRefEntityWithPowerups(&torso, &cent->currentState);

	torso.shaderRGBA[3] = 255; // leave powerup entity alpha alone

	t = cg.time - ci->medkitUsageTime;

	if (ci->medkitUsageTime && t < 500) {
		memcpy(&powerup, &torso, sizeof(torso));

		powerup.hModel = cgs.media.medkitUsageModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		// always draw
		powerup.renderfx &= ~RF_ONLY_MIRROR;
		VectorClear(angles);
		AnglesToAxis(angles, powerup.axis);
		VectorCopy(cent->lerpOrigin, powerup.origin);
		powerup.origin[2] += -24 + (float)t * 80 / 500;

		if (t > 400) {
			c = (float)(t - 1000) * 0xff / 100;
			powerup.shaderRGBA[0] = 0xff - c;
			powerup.shaderRGBA[1] = 0xff - c;
			powerup.shaderRGBA[2] = 0xff - c;
			powerup.shaderRGBA[3] = 0xff - c;
		} else {
			powerup.shaderRGBA[0] = 0xff;
			powerup.shaderRGBA[1] = 0xff;
			powerup.shaderRGBA[2] = 0xff;
			powerup.shaderRGBA[3] = 0xff;
		}

		trap_R_AddRefEntityToScene(&powerup);
	}

	if (cent->currentState.eFlags & EF_KAMIKAZE) {
		memset(&skull, 0, sizeof(skull));

		VectorCopy(cent->lerpOrigin, skull.lightingOrigin);

		skull.shadowPlane = shadowPlane;
		skull.renderfx = renderfx;

		if (cent->currentState.eFlags & EF_DEAD) {
			// one skull bobbing above the dead body
			angle = ((cg.time / 7) & 255) * (M_PI * 2) / 255;

			if (angle > M_PI * 2) {
				angle -= (float)M_PI * 2;
			}

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[2] = 15 + sin(angle) * 8;

			VectorAdd(torso.origin, dir, skull.origin);

			dir[2] = 0;

			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;

			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;

			trap_R_AddRefEntityToScene(&skull);
		} else {
			// three skulls spinning around the player
			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255;
			dir[0] = cos(angle) * 20;
			dir[1] = sin(angle) * 20;
			dir[2] = cos(angle) * 20;

			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = sin(angle) * 30;
			angles[1] = (angle * 180 / M_PI) + 90;

			if (angles[1] > 360) {
				angles[1] -= 360;
			}

			angles[2] = 0;

			AnglesToAxis(angles, skull.axis);
			/*
			dir[2] = 0;
			VectorInverse(dir);
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/
			skull.hModel = cgs.media.kamikazeHeadModel;

			trap_R_AddRefEntityToScene(&skull);
			// flip the trail because this skull is spinning in the other direction
			VectorInverse(skull.axis[1]);

			skull.hModel = cgs.media.kamikazeHeadTrail;

			trap_R_AddRefEntityToScene(&skull);

			angle = ((cg.time / 4) & 255) * (M_PI * 2) / 255 + M_PI;

			if (angle > M_PI * 2) {
				angle -= (float)M_PI * 2;
			}

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = cos(angle) * 20;

			VectorAdd(torso.origin, dir, skull.origin);

			angles[0] = cos(angle - 0.5 * M_PI) * 30;
			angles[1] = 360 - (angle * 180 / M_PI);

			if (angles[1] > 360) {
				angles[1] -= 360;
			}

			angles[2] = 0;

			AnglesToAxis(angles, skull.axis);
			/*
			dir[2] = 0;
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);
			*/
			skull.hModel = cgs.media.kamikazeHeadModel;

			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;

			trap_R_AddRefEntityToScene(&skull);

			angle = ((cg.time / 3) & 255) * (M_PI * 2) / 255 + 0.5 * M_PI;

			if (angle > M_PI * 2) {
				angle -= (float)M_PI * 2;
			}

			dir[0] = sin(angle) * 20;
			dir[1] = cos(angle) * 20;
			dir[2] = 0;

			VectorAdd(torso.origin, dir, skull.origin);
			VectorCopy(dir, skull.axis[1]);
			VectorNormalize(skull.axis[1]);
			VectorSet(skull.axis[2], 0, 0, 1);
			CrossProduct(skull.axis[1], skull.axis[2], skull.axis[0]);

			skull.hModel = cgs.media.kamikazeHeadModel;

			trap_R_AddRefEntityToScene(&skull);

			skull.hModel = cgs.media.kamikazeHeadTrail;

			trap_R_AddRefEntityToScene(&skull);
		}
	}

	if (cent->currentState.powerups & (1 << PW_AMMOREGEN)) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.ammoRegenPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}

	if (cent->currentState.powerups & (1 << PW_GUARD)) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.guardPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}

	if (cent->currentState.powerups & (1 << PW_DOUBLER)) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.doublerPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}

	if (cent->currentState.powerups & (1 << PW_SCOUT)) {
		memcpy(&powerup, &torso, sizeof(torso));
		powerup.hModel = cgs.media.scoutPowerupModel;
		powerup.frame = 0;
		powerup.oldframe = 0;
		powerup.customSkin = 0;
		trap_R_AddRefEntityToScene(&powerup);
	}
	// add the talk baloon or disconnect icon
	CG_PlayerSprites(cent, &torso);
	// add the head
	head.hModel = ci->headModel;

	if (!head.hModel) {
		return;
	}

	head.customSkin = legs.customSkin; // Tobias CHECK: legs?

	VectorCopy(cent->lerpOrigin, head.lightingOrigin);
	CG_PositionRotatedEntityOnTag(&head, &torso, ci->torsoModel, "tag_head");

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	Byte4Copy(ci->c1RGBA, head.shaderRGBA);

	head.shaderRGBA[3] = cent->currentState.skinFraction * 255;

	CG_AddRefEntityWithPowerups(&head, &cent->currentState);
	CG_AddBreathPuffs(cent, &head);
	CG_DustTrail(cent);
	// add the gun/barrel/flash
	CG_AddPlayerWeapon(&torso, NULL, cent, ci->team);
	// add powerups floating behind the player
	CG_PlayerPowerups(cent, &torso);
}

/*
=======================================================================================================================================
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info.
=======================================================================================================================================
*/
void CG_ResetPlayerEntity(centity_t *cent) {

	cent->errorTime = -99999; // guarantee no error decay added

	CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.legs, cent->currentState.legsAnim);
	CG_ClearLerpFrame(&cgs.clientinfo[cent->currentState.clientNum], &cent->pe.torso, cent->currentState.torsoAnim);

	BG_EvaluateTrajectory(&cent->currentState.pos, cg.time, cent->lerpOrigin);
	BG_EvaluateTrajectory(&cent->currentState.apos, cg.time, cent->lerpAngles);

	VectorCopy(cent->lerpOrigin, cent->rawOrigin);
	VectorCopy(cent->lerpAngles, cent->rawAngles);

	memset(&cent->pe.legs, 0, sizeof(cent->pe.legs));

	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset(&cent->pe.torso, 0, sizeof(cent->pe.torso));

	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	if (cg_debugPosition.integer) {
		CG_Printf("%i ResetPlayerEntity yaw = %f\n", cent->currentState.number, cent->pe.torso.yawAngle);
	}
}
