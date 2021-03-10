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
 Entity scripting that allows the designers to control the behaviour of entities according to each different scenario.
**************************************************************************************************************************************/

#include "g_local.h"
#include "../qcommon/q_shared.h"

vmCvar_t g_scriptDebug;
// action functions need to be declared here so they can be accessed in the scriptAction table
qboolean G_ScriptAction_GotoMarker(gentity_t *ent, char *params);
qboolean G_ScriptAction_Wait(gentity_t *ent, char *params);
qboolean G_ScriptAction_Trigger(gentity_t *ent, char *params);
qboolean G_ScriptAction_PlaySound(gentity_t *ent, char *params);
qboolean G_ScriptAction_PlayAnim(gentity_t *ent, char *params);
qboolean G_ScriptAction_AlertEntity(gentity_t *ent, char *params);
qboolean G_ScriptAction_RemoveEntity(gentity_t *ent, char *params);
qboolean G_ScriptAction_Accum(gentity_t *ent, char *params);
qboolean G_ScriptAction_MissionFailed(gentity_t *ent, char *params);
qboolean G_ScriptAction_MissionSuccess(gentity_t *ent, char *params);
qboolean G_ScriptAction_Print(gentity_t *ent, char *params);
qboolean G_ScriptAction_FaceAngles(gentity_t *ent, char *params);
qboolean G_ScriptAction_ResetScript(gentity_t *ent, char *params);
qboolean G_ScriptAction_TagConnect(gentity_t *ent, char *params);
qboolean G_ScriptAction_Halt(gentity_t *ent, char *params);
qboolean G_ScriptAction_StopSound(gentity_t *ent, char *params);
qboolean G_ScriptAction_StartCam(gentity_t *ent, char *params);
qboolean G_ScriptAction_StartCamBlack(gentity_t *ent, char *params);
qboolean G_ScriptAction_EntityScriptName(gentity_t *ent, char *params);
qboolean G_ScriptAction_AIScriptName(gentity_t *ent, char *params);
qboolean G_ScriptAction_MapDescription(gentity_t *ent, char *params);
qboolean G_ScriptAction_BlueRespawntime(gentity_t *ent, char *params);
qboolean G_ScriptAction_RedRespawntime(gentity_t *ent, char *params);
qboolean G_ScriptAction_NumberofObjectives(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveMet(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveBlueDesc(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveShortBlueDesc(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveRedDesc(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveShortRedDesc(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectiveImage(gentity_t *ent, char *params);
qboolean G_ScriptAction_SetWinner(gentity_t *ent, char *params);
qboolean G_ScriptAction_SetObjectiveStatus(gentity_t *ent, char *params);
qboolean G_ScriptAction_SetDefendingTeam(gentity_t *ent, char *params);
qboolean G_ScriptAction_Announce(gentity_t *ent, char *params);
qboolean G_ScriptAction_SetRoundTimelimit(gentity_t *ent, char *params);
qboolean G_ScriptAction_OverviewImage(gentity_t *ent, char *params);
qboolean G_ScriptAction_BackupScript(gentity_t *ent, char *params);
qboolean G_ScriptAction_RestoreScript(gentity_t *ent, char *params);
qboolean G_ScriptAction_SetHealth(gentity_t *ent, char *params);
qboolean G_ScriptAction_MusicStart(gentity_t *ent, char *params);
qboolean G_ScriptAction_MusicPlay(gentity_t *ent, char *params);
qboolean G_ScriptAction_MusicStop(gentity_t *ent, char *params);
qboolean G_ScriptAction_MusicFade(gentity_t *ent, char *params);
qboolean G_ScriptAction_MusicQueue(gentity_t *ent, char *params);
qboolean G_ScriptAction_ObjectivesNeeded(gentity_t *ent, char *params);
qboolean G_ScriptAction_NumSecrets(gentity_t *ent, char *params);
// these are the actions that each event can call
g_script_stack_action_t gScriptActions[] = {
	{"gotomarker", G_ScriptAction_GotoMarker},
	{"playsound", G_ScriptAction_PlaySound},
	{"playanim", G_ScriptAction_PlayAnim},
	{"wait", G_ScriptAction_Wait},
	{"trigger", G_ScriptAction_Trigger},
	{"alertentity", G_ScriptAction_AlertEntity},
	{"removeentity", G_ScriptAction_RemoveEntity},
	{"accum", G_ScriptAction_Accum},
	{"missionfailed", G_ScriptAction_MissionFailed},
	{"missionsuccess", G_ScriptAction_MissionSuccess},
	{"print", G_ScriptAction_Print},
	{"faceangles", G_ScriptAction_FaceAngles},
	{"resetscript", G_ScriptAction_ResetScript},
	{"attachtotag", G_ScriptAction_TagConnect},
	{"halt", G_ScriptAction_Halt},
	{"stopsound", G_ScriptAction_StopSound},
	{"startcam", G_ScriptAction_StartCam},
	{"startcamblack", G_ScriptAction_StartCamBlack},
	{"entityscriptname", G_ScriptAction_EntityScriptName},
	{"aiscriptname", G_ScriptAction_AIScriptName},
	{"objectivemet", G_ScriptAction_ObjectiveMet},
	// multiplayer scripting commands start with "wm_" (Wolf Multiplayer)
	{"wm_mapdescription", G_ScriptAction_MapDescription},
	{"wm_blue_respawntime", G_ScriptAction_BlueRespawntime},
	{"wm_red_respawntime", G_ScriptAction_RedRespawntime},
	{"wm_number_of_objectives", G_ScriptAction_NumberofObjectives},
	{"wm_objective_blue_desc", G_ScriptAction_ObjectiveBlueDesc},
	{"wm_objective_short_blue_desc", G_ScriptAction_ObjectiveShortBlueDesc},
	{"wm_objective_red_desc", G_ScriptAction_ObjectiveRedDesc},
	{"wm_objective_short_red_desc", G_ScriptAction_ObjectiveShortRedDesc},
	{"wm_objective_image", G_ScriptAction_ObjectiveImage},
	{"wm_setwinner", G_ScriptAction_SetWinner},
	{"wm_set_objective_status", G_ScriptAction_SetObjectiveStatus},
	{"wm_set_defending_team", G_ScriptAction_SetDefendingTeam},
	{"wm_announce", G_ScriptAction_Announce},
	{"wm_set_round_timelimit", G_ScriptAction_SetRoundTimelimit},
	{"wm_overview_image", G_ScriptAction_OverviewImage},
	{"backupscript", G_ScriptAction_BackupScript},
	{"restorescript", G_ScriptAction_RestoreScript},
	{"sethealth", G_ScriptAction_SetHealth},
	{"mu_start", G_ScriptAction_MusicStart}, // time to fadeup
	{"mu_play", G_ScriptAction_MusicPlay},
	{"mu_stop", G_ScriptAction_MusicStop}, // time to fadeout
	{"mu_fade", G_ScriptAction_MusicFade}, // time to fade to target
	{"mu_queue", G_ScriptAction_MusicQueue}, // music that will start when previous fades to 0
	{"objectivesneeded", G_ScriptAction_ObjectivesNeeded},
	{"numsecrets", G_ScriptAction_NumSecrets},
	{NULL, 0}
};

qboolean G_Script_EventMatch_StringEqual(g_script_event_t *event, char *eventParm);
qboolean G_Script_EventMatch_IntInRange(g_script_event_t *event, char *eventParm);

// the list of events that can start an action sequence
g_script_event_define_t gScriptEvents[] = {
	{"spawn", 0}, // called as each character is spawned into the game
	{"trigger", G_Script_EventMatch_StringEqual}, // something has triggered us (always followed by an identifier)
	{"pain", G_Script_EventMatch_IntInRange}, // we've been hurt
	{"death", 0}, // RIP
	{"activate", G_Script_EventMatch_StringEqual}, // something has triggered us (always followed by an identifier)
	{"stopcam", 0},
	{NULL, 0}
};

/*
=======================================================================================================================================
G_Script_EventMatch_StringEqual
=======================================================================================================================================
*/
qboolean G_Script_EventMatch_StringEqual(g_script_event_t *event, char *eventParm) {

	if (eventParm && !Q_strcasecmp(event->params, eventParm)) {
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=======================================================================================================================================
G_Script_EventMatch_IntInRange
=======================================================================================================================================
*/
qboolean G_Script_EventMatch_IntInRange(g_script_event_t *event, char *eventParm) {
	char *pString, *token;
	int int1, int2, eInt;

	// get the cast name
	pString = eventParm;
	token = COM_ParseExt(&pString, qfalse);
	int1 = atoi(token);
	token = COM_ParseExt(&pString, qfalse);
	int2 = atoi(token);
	eInt = atoi(event->params);

	if (eventParm && eInt > int1 && eInt <= int2) {
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=======================================================================================================================================
G_Script_EventForString
=======================================================================================================================================
*/
int G_Script_EventForString(char *string) {
	int i;

	for (i = 0; gScriptEvents[i].eventStr; i++) {
		if (!Q_strcasecmp(string, gScriptEvents[i].eventStr)) {
			return i;
		}
	}

	return -1;
}

/*
=======================================================================================================================================
G_Script_ActionForString
=======================================================================================================================================
*/
g_script_stack_action_t *G_Script_ActionForString(char *string) {
	int i;

	for (i = 0; gScriptActions[i].actionString; i++) {
		if (!Q_strcasecmp(string, gScriptActions[i].actionString)) {
			return &gScriptActions[i];
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
G_Script_ScriptLoad

Loads the script for the current level into the buffer.
=======================================================================================================================================
*/
void G_Script_ScriptLoad(void) {
	char filename[MAX_QPATH];
	vmCvar_t mapname;
	fileHandle_t f;
	int len;

	trap_Cvar_Register(&g_scriptDebug, "g_scriptDebug", "0", 0);

	level.scriptEntity = NULL;

	trap_Cvar_VariableStringBuffer("g_scriptName", filename, sizeof(filename));

	if (strlen(filename) > 0) {
		trap_Cvar_Register(&mapname, "g_scriptName", "", CVAR_ROM);
	} else {
		trap_Cvar_Register(&mapname, "mapname", "", CVAR_SERVERINFO|CVAR_ROM);
	}

	Q_strncpyz(filename, "maps/", sizeof(filename));
	Q_strcat(filename, sizeof(filename), mapname.string);
	Q_strcat(filename, sizeof(filename), ".script");

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	// make sure we clear out the temporary scriptname
	trap_Cvar_Set("g_scriptName", "");

	if (len < 0) {
		return;
	}

	level.scriptEntity = G_Alloc(len);

	trap_FS_Read(level.scriptEntity, len, f);
	trap_FS_FCloseFile(f);
}

#define MAX_SCRIPT_EVENTS 64
g_script_event_t g_temp_events[MAX_SCRIPT_EVENTS];
/*
=======================================================================================================================================
G_Script_ScriptParse

Parses the script for the given entity.
=======================================================================================================================================
*/
void G_Script_ScriptParse(gentity_t *ent) {
	char *pScript;
	char *token;
	qboolean wantName;
	qboolean inScript;
	int eventNum;
	int numEventItems;
	g_script_event_t *curEvent;
	char params[MAX_INFO_STRING];
	g_script_stack_action_t *action;
	int i;
	int bracketLevel;
	qboolean buildScript;

	if (!ent->scriptName) {
		return;
	}

	if (!level.scriptEntity) {
		return;
	}

	buildScript = qtrue;
	pScript = level.scriptEntity;
	wantName = qtrue;
	inScript = qfalse;

	COM_BeginParseSession("G_Script_ScriptParse");

	bracketLevel = 0;
	numEventItems = 0;

	memset(g_temp_events, 0, sizeof(g_temp_events));

	while (1) {
		token = COM_Parse(&pScript);

		if (!token[0]) {
			if (!wantName) {
				G_Error("G_Script_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine());
			}

			break;
		}
		// end of script
		if (token[0] == '}') {
			if (inScript) {
				break;
			}

			if (wantName) {
				G_Error("G_Script_ScriptParse(), Error (line %d): '}' found, but not expected.\n", COM_GetCurrentParseLine());
			}

			wantName = qtrue;
		} else if (token[0] == '{') {
			if (wantName) {
				G_Error("G_Script_ScriptParse(), Error (line %d): '{' found, NAME expected.\n", COM_GetCurrentParseLine());
			}
		} else if (wantName) {
			if (!Q_strcasecmp(ent->scriptName, token)) {
				inScript = qtrue;
				numEventItems = 0;
			}

			wantName = qfalse;
		} else if (inScript) {
			//if (!Q_strcasecmp(token, "attributes")) {
			//	// read in all the attributes
			//	G_Script_CheckLevelAttributes(cs, ent, &pScript);
			//	continue;
			//}

			eventNum = G_Script_EventForString(token);

			if (eventNum < 0) {
				G_Error("G_Script_ScriptParse(), Error (line %d): unknown event: %s.\n", COM_GetCurrentParseLine(), token);
			}

			if (numEventItems >= MAX_SCRIPT_EVENTS) {
				G_Error("G_Script_ScriptParse(), Error (line %d): MAX_SCRIPT_EVENTS reached (%d).\n", COM_GetCurrentParseLine(), MAX_SCRIPT_EVENTS);
			}

			curEvent = &g_temp_events[numEventItems];
			curEvent->eventNum = eventNum;

			memset(params, 0, sizeof(params));
			// parse any event params before the start of this event's actions
			while ((token = COM_Parse(&pScript)) && (token[0] != '{')) {
				if (!token[0]) {
					G_Error("G_Script_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine());
				}

				if (strlen(params)) { // add a space between each param
					Q_strcat(params, sizeof(params), " ");
				}

				Q_strcat(params, sizeof(params), token);
			}

			if (strlen(params)) { // copy the params into the event
				curEvent->params = G_Alloc(strlen(params) + 1);
				Q_strncpyz(curEvent->params, params, strlen(params) + 1);
			}
			// parse the actions for this event
			while ((token = COM_Parse(&pScript)) && (token[0] != '}')) {
				if (!token[0]) {
					G_Error("G_Script_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine());
				}

				action = G_Script_ActionForString(token);

				if (!action) {
					G_Error("G_Script_ScriptParse(), Error (line %d): unknown action: %s.\n", COM_GetCurrentParseLine(), token);
				}

				curEvent->stack.items[curEvent->stack.numItems].action = action;

				memset(params, 0, sizeof(params));

				token = COM_ParseExt(&pScript, qfalse);

				for (i = 0; token[0]; i++) {
					if (strlen(params)) { // add a space between each param
						Q_strcat(params, sizeof(params), " ");
					}

					if (i == 0) {
						// special case: playsound's need to be cached on startup to prevent in-game pauses
						if (!Q_stricmp(action->actionString, "playsound")) {
							G_SoundIndex(token);
						}

						if (buildScript && (!Q_stricmp(action->actionString, "mu_start") || !Q_stricmp(action->actionString, "mu_play") || !Q_stricmp(action->actionString, "mu_queue") || !Q_stricmp(action->actionString, "startcam") || !Q_stricmp(action->actionString, "startcamblack"))) {
							if (strlen(token)) { // we know there's a [0], but don't know if it's '0'
								trap_SendServerCommand(ent->s.number, va("addToBuild %s\n", token));
							}
						}
					}

					if (strrchr(token,' ')) { // need to wrap this param in quotes since it has more than one word
						Q_strcat(params, sizeof(params), "\"");
					}

					Q_strcat(params, sizeof(params), token);

					if (strrchr(token,' ')) { // need to wrap this param in quotes since it has more than one word
						Q_strcat(params, sizeof(params), "\"");
					}

					token = COM_ParseExt(&pScript, qfalse);
				}

				if (strlen(params)) { // copy the params into the event
					curEvent->stack.items[curEvent->stack.numItems].params = G_Alloc(strlen(params) + 1);
					Q_strncpyz(curEvent->stack.items[curEvent->stack.numItems].params, params, strlen(params) + 1);
				}

				curEvent->stack.numItems++;

				if (curEvent->stack.numItems >= G_MAX_SCRIPT_STACK_ITEMS) {
					G_Error("G_Script_ScriptParse(): script exceeded MAX_SCRIPT_ITEMS (%d), line %d.\n", G_MAX_SCRIPT_STACK_ITEMS, COM_GetCurrentParseLine());
				}
			}

			numEventItems++;
		} else { // skip this character completely
			// NOTE: gcc: suggest parentheses around assignment used as truth value
			while ((token = COM_Parse(&pScript))) {
				if (!token[0]) {
					G_Error("G_Script_ScriptParse(), Error (line %d): '}' expected, end of script found.\n", COM_GetCurrentParseLine());
				} else if (token[0] == '{') {
					bracketLevel++;
				} else if (token[0] == '}') {
					if (!--bracketLevel) {
						break;
					}
				}
			}
		}
	}
	// alloc and copy the events into the gentity_t for this cast
	if (numEventItems > 0) {
		ent->scriptEvents = G_Alloc(sizeof(g_script_event_t) * numEventItems);

		memcpy(ent->scriptEvents, g_temp_events, sizeof(g_script_event_t) * numEventItems);

		ent->numScriptEvents = numEventItems;
	}
}

qboolean G_Script_ScriptRun(gentity_t *ent);
/*
=======================================================================================================================================
G_Script_ScriptChange
=======================================================================================================================================
*/
void G_Script_ScriptChange(gentity_t *ent, int newScriptNum) {
	g_script_status_t scriptStatusBackup;

	// backup the current scripting
	memcpy(&scriptStatusBackup, &ent->scriptStatus, sizeof(g_script_status_t));
	// set the new script to this cast, and reset script status
	ent->scriptStatus.scriptEventIndex = newScriptNum;
	ent->scriptStatus.scriptStackHead = 0;
	ent->scriptStatus.scriptStackChangeTime = level.time;
	ent->scriptStatus.scriptId = scriptStatusBackup.scriptId + 1;
	// try and run the script, if it doesn't finish, then abort the current script (discard backup)
	if (G_Script_ScriptRun(ent)) {
		// completed successfully
		memcpy(&ent->scriptStatus, &scriptStatusBackup, sizeof(g_script_status_t));
	}
}

/*
=======================================================================================================================================
G_Script_ScriptEvent

An event has occured, for which a script may exist.
=======================================================================================================================================
*/
void G_Script_ScriptEvent(gentity_t *ent, char *eventStr, char *params) {
	int i, eventNum;

	eventNum = -1;
	// find out which event this is
	for (i = 0; gScriptEvents[i].eventStr; i++) {
		if (!Q_strcasecmp(eventStr, gScriptEvents[i].eventStr)) { // match found
			eventNum = i;
			break;
		}
	}

	if (eventNum < 0) {
		if (g_cheats.integer) { // dev mode
			G_Printf("devmode-> G_Script_ScriptEvent(), unknown event: %s.\n", eventStr);
		}

		return;
	}
	// see if this entity has this event
	for (i = 0; i < ent->numScriptEvents; i++) {
		if (ent->scriptEvents[i].eventNum == eventNum) {
			if ((!ent->scriptEvents[i].params) || (!gScriptEvents[eventNum].eventMatch || gScriptEvents[eventNum].eventMatch(&ent->scriptEvents[i], params))) {
				G_Script_ScriptChange(ent, i);
				break;
			}
		}
	}
}

/*
=======================================================================================================================================
G_Script_ScriptRun

Returns qtrue if the script completed.
=======================================================================================================================================
*/
qboolean G_Script_ScriptRun(gentity_t *ent) {
	g_script_stack_t *stack;

	if (strlen(g_missionStats.string) > 1) {
		return qfalse;
	}

	trap_Cvar_Update(&g_scriptDebug);

	if (!ent->scriptEvents) {
		ent->scriptStatus.scriptEventIndex = -1;
		return qtrue;
	}
	// if we are still doing a gotomarker, process the movement
	if (ent->scriptStatus.scriptFlags & SCFL_GOING_TO_MARKER) {
		G_ScriptAction_GotoMarker(ent, NULL);
	}
	// if we are animating, do the animation
	if (ent->scriptStatus.scriptFlags & SCFL_ANIMATING) {
		G_ScriptAction_PlayAnim(ent, ent->scriptStatus.animatingParams);
	}

	if (ent->scriptStatus.scriptEventIndex < 0) {
		return qtrue;
	}

	stack = &ent->scriptEvents[ent->scriptStatus.scriptEventIndex].stack;

	if (!stack->numItems) {
		ent->scriptStatus.scriptEventIndex = -1;
		return qtrue;
	}
	// show debugging info
	if (g_scriptDebug.integer && ent->scriptStatus.scriptStackChangeTime == level.time) {
		if (ent->scriptStatus.scriptStackHead < stack->numItems) {
			G_Printf("%i: (%s) GScript command: %s %s.\n", level.time, ent->scriptName, stack->items[ent->scriptStatus.scriptStackHead].action->actionString, (stack->items[ent->scriptStatus.scriptStackHead].params ? stack->items[ent->scriptStatus.scriptStackHead].params : ""));
		}
	}

	while (ent->scriptStatus.scriptStackHead < stack->numItems) {
		if (!stack->items[ent->scriptStatus.scriptStackHead].action->actionFunc(ent, stack->items[ent->scriptStatus.scriptStackHead].params)) {
			return qfalse;
		}
		// move to the next action in the script
		ent->scriptStatus.scriptStackHead++;
		// record the time that this new item became active
		ent->scriptStatus.scriptStackChangeTime = level.time;
		// show debugging info
		if (g_scriptDebug.integer) {
			if (ent->scriptStatus.scriptStackHead < stack->numItems) {
				G_Printf("%i: (%s) GScript command: %s %s.\n", level.time, ent->scriptName, stack->items[ent->scriptStatus.scriptStackHead].action->actionString, (stack->items[ent->scriptStatus.scriptStackHead].params ? stack->items[ent->scriptStatus.scriptStackHead].params : ""));
			}
		}
	}

	ent->scriptStatus.scriptEventIndex = -1;

	return qtrue;
}

/*
=======================================================================================================================================
script_linkentity
=======================================================================================================================================
*/
void script_linkentity(gentity_t *ent) {

	// this is required since non-solid brushes need to be linked but not solid
	trap_LinkEntity(ent);
}

/*
=======================================================================================================================================
script_mover_die
=======================================================================================================================================
*/
void script_mover_die(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod) {

	if (self->spawnflags & 4) {
		switch (mod) {
			case MOD_GRENADE_SPLASH:
			case MOD_ROCKET:
			case MOD_ROCKET_SPLASH:
			case MOD_BFG:
			case MOD_BFG_SPLASH:
				break;
			default: // no death from this weapon
				self->health += damage;
				return;
		}
	}

	G_Script_ScriptEvent(self, "death", "");

	self->die = 0;

	trap_UnlinkEntity(self);
	G_FreeEntity(self);
}

/*
=======================================================================================================================================
script_mover_pain
=======================================================================================================================================
*/
void script_mover_pain(gentity_t *self, gentity_t *attacker, int damage) {
	G_Script_ScriptEvent(self, "pain", va("%d %d", self->health, self->health + damage));
}

/*
=======================================================================================================================================
script_mover_spawn
=======================================================================================================================================
*/
void script_mover_spawn(gentity_t *ent) {

	if (ent->spawnflags & 2) {
		ent->clipmask = CONTENTS_SOLID;
		ent->r.contents = CONTENTS_SOLID;
	} else {
		ent->s.eFlags |= EF_NONSOLID_BMODEL;
		ent->clipmask = 0;
		ent->r.contents = 0;
	}

	script_linkentity(ent);
}

/*
=======================================================================================================================================
script_mover_use
=======================================================================================================================================
*/
void script_mover_use(gentity_t *ent, gentity_t *other, gentity_t *activator) {
	script_mover_spawn(ent);
}

/*
=======================================================================================================================================
script_mover_blocked
=======================================================================================================================================
*/
void script_mover_blocked(gentity_t *ent, gentity_t *other) {

	// remove it, we must not stop for anything or it will screw up script timing
	if (!other->client) {
		G_TempEntity(other->s.origin, EV_ITEM_POP);
		G_FreeEntity(other);
		return;
	}
	// FIXME: we could have certain entities stop us, thereby "pausing" movement until they move out the way. then we can just call the GotoMarker()
	// again, telling it that we are just now calling it for the first time, so it should start us on our way again (theoretically speaking)
	// kill them
	G_Damage(other, ent, ent, NULL, NULL, 9999, 0, MOD_CRUSH);
}

/*QUAKED script_mover (0.5 0.25 1.0) ? TRIGGERSPAWN SOLID EXPLOSIVEDAMAGEONLY
Scripted brush entity. A simplified means of moving brushes around based on events.

"modelscale" - Scale multiplier (defaults to 1, and scales uniformly).
"modelscale_vec" - Set scale per-axis. Overrides "modelscale", so if you have both the "modelscale" is ignored.
"model2" optional md3 to draw over the solid clip brush.
"scriptname" name used for scripting purposes (like aiName in AI scripting).
"health" optionally make this entity damagable.
*/
void SP_script_mover(gentity_t *ent) {
	float scale[3] = {1, 1, 1};
	vec3_t scalevec;

	if (!ent->model) {
		G_Error("SP_script_mover: script_model_med must have a \"model\".\n");
	}

	if (!ent->scriptName) {
		G_Error("SP_script_mover: script_model_med must have a \"scriptname\".\n");
	}

	ent->blocked = script_mover_blocked;
	// first position at start
	VectorCopy(ent->s.origin, ent->pos1);
	VectorCopy(ent->pos1, ent->pos2); // don't go anywhere just yet

	trap_SetBrushModel(ent, ent->model);
	InitMover(ent);

	ent->reached = 0;

	if (ent->spawnflags & 1) {
		ent->use = script_mover_use;
		trap_UnlinkEntity(ent); // make sure it's not visible
		return;
	}

	G_SetAngle(ent, ent->s.angles);
	G_SpawnInt("health", "0", &ent->health);

	if (ent->health) {
		ent->takedamage = qtrue;
	}

	ent->die = script_mover_die;
	ent->pain = script_mover_pain;
	// look for general scaling
	if (G_SpawnFloat("modelscale", "1", &scale[0])) {
		scale[2] = scale[1] = scale[0];
	}
	// look for axis specific scaling
	if (G_SpawnVector("modelscale_vec", "1 1 1", &scalevec[0])) {
		VectorCopy(scalevec, scale);
	}

	if (scale[0] != 1 || scale[1] != 1 || scale[2] != 1) {
		ent->s.density = ET_MOVERSCALED;
		// scale is stored in 'angles2'
		VectorCopy(scale, ent->s.angles2);
	}

	script_mover_spawn(ent);
}

/*
=======================================================================================================================================
script_model_med_spawn
=======================================================================================================================================
*/
void script_model_med_spawn(gentity_t *ent) {

	if (ent->spawnflags & 2) {
		ent->clipmask = CONTENTS_SOLID;
		ent->r.contents = CONTENTS_SOLID;
	}

	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = G_ModelIndex(ent->model);
	ent->s.frame = 0;

	VectorCopy(ent->s.origin, ent->s.pos.trBase);

	ent->s.pos.trType = TR_STATIONARY;

	trap_LinkEntity(ent);
}

/*
=======================================================================================================================================
script_model_med_use
=======================================================================================================================================
*/
void script_model_med_use(gentity_t *ent, gentity_t *other, gentity_t *activator) {
	script_model_med_spawn(ent);
}

/*QUAKED script_model_med (0.5 0.25 1.0) (-16 -16 -24) (16 16 64) TRIGGERSPAWN SOLID
MEDIUM SIZED scripted entity, used for animating a model, moving it around, etc.
SOLID spawnflag means this entity will clip the player and AI, otherwise they can walk straight through it.

"model" the full path of the model to use.
"scriptname" name used for scripting purposes (like aiName in AI scripting).
*/
void SP_script_model_med(gentity_t *ent) {

	if (!ent->model) {
		G_Error("SP_script_model_med: script_model_med %s must have a \"model\".\n", ent->scriptName);
	}
	if (!ent->scriptName) {
		G_Error("SP_script_model_med: script_model_med must have a \"scriptname\".\n");
	}

	ent->s.eType = ET_GENERAL;
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorClear(ent->s.apos.trDelta);

	if (ent->spawnflags & 1) {
		ent->use = script_model_med_use;
		trap_UnlinkEntity(ent); // make sure it's not visible
		return;
	}

	script_model_med_spawn(ent);
}

/*QUAKED script_camera (1.0 0.25 1.0) (-8 -8 -8) (8 8 8) TRIGGERSPAWN
This is a camera entity. Used by the scripting to show cinematics, via special camera commands. See scripting documentation.
"scriptname" name used for scripting purposes (like aiName in AI scripting)
*/
void SP_script_camera(gentity_t *ent) {

	if (!ent->scriptName) {
		G_Error("%s must have a \"scriptname\".\n", ent->classname);
	}

	ent->s.eType = ET_CAMERA;
	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;

	VectorCopy(ent->s.angles, ent->s.apos.trBase);
	VectorClear(ent->s.apos.trDelta);

	ent->s.frame = 0;
	ent->r.svFlags |= SVF_NOCLIENT; // only broadcast when in use
}

/*QUAKED script_multiplayer (1.0 0.25 1.0) (-8 -8 -8) (8 8 8)
This is used to script multiplayer maps. Entity not displayed in game.
"scriptname" name used for scripting purposes (REQUIRED)
*/
void SP_script_multiplayer(gentity_t *ent) {

	if (!ent->scriptName) {
		G_Error("%s must have a \"scriptname\".\n", ent->classname);
	}

	if (Q_stricmp(ent->scriptName, "game_manager")) {
		G_Error("%s must have a \"scriptname\" of 'game_manager'.\n", ent->classname);
	}

	ent->s.eType = ET_INVISIBLE;
	ent->r.svFlags |= SVF_NOCLIENT; // only broadcast when in use
}
