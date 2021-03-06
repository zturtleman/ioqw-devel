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
 Misc utility functions for game module.
**************************************************************************************************************************************/

#include "g_local.h"

typedef struct {
	char oldShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	float timeOffset;
} shaderRemap_t;

#define MAX_SHADER_REMAPS 128

int remapCount = 0;
shaderRemap_t remappedShaders[MAX_SHADER_REMAPS];

/*
=======================================================================================================================================
AddRemap
=======================================================================================================================================
*/
void AddRemap(const char *oldShader, const char *newShader, float timeOffset) {
	int i;

	for (i = 0; i < remapCount; i++) {
		if (Q_stricmp(oldShader, remappedShaders[i].oldShader) == 0) {
			// found it, just update this one
			strcpy(remappedShaders[i].newShader, newShader);
			remappedShaders[i].timeOffset = timeOffset;
			return;
		}
	}

	if (remapCount < MAX_SHADER_REMAPS) {
		strcpy(remappedShaders[remapCount].newShader, newShader);
		strcpy(remappedShaders[remapCount].oldShader, oldShader);
		remappedShaders[remapCount].timeOffset = timeOffset;
		remapCount++;
	}
}

/*
=======================================================================================================================================
BuildShaderStateConfig
=======================================================================================================================================
*/
const char *BuildShaderStateConfig(void) {
	static char buff[MAX_STRING_CHARS * 4];
	char out[(MAX_QPATH * 2) + 5];
	int i;

	memset(buff, 0, sizeof(buff));

	for (i = 0; i < remapCount; i++) {
		Com_sprintf(out, (MAX_QPATH * 2) + 5, "%s=%s:%5.2f@", remappedShaders[i].oldShader, remappedShaders[i].newShader, remappedShaders[i].timeOffset);
		Q_strcat(buff, sizeof(buff), out);
	}

	return buff;
}

/*
=======================================================================================================================================

	Model/Sound configstring indexes.

=======================================================================================================================================
*/

/*
=======================================================================================================================================
G_FindConfigstringIndex
=======================================================================================================================================
*/
int G_FindConfigstringIndex(const char *name, int start, int max, qboolean create) {
	int i;
	char s[MAX_STRING_CHARS];

	if (!name || !name[0]) {
		return 0;
	}

	for (i = 1; i < max; i++) {
		trap_GetConfigstring(start + i, s, sizeof(s));

		if (!s[0]) {
			break;
		}

		if (!strcmp(s, name)) {
			return i;
		}
	}

	if (!create) {
		return 0;
	}

	if (i == max) {
		G_Error("G_FindConfigstringIndex: overflow");
	}

	trap_SetConfigstring(start + i, name);

	return i;
}

/*
=======================================================================================================================================
G_ModelIndex
=======================================================================================================================================
*/
int G_ModelIndex(const char *name) {
	return G_FindConfigstringIndex(name, CS_MODELS, MAX_MODELS, qtrue);
}

/*
=======================================================================================================================================
G_SoundIndex
=======================================================================================================================================
*/
int G_SoundIndex(const char *name) {
	return G_FindConfigstringIndex(name, CS_SOUNDS, MAX_SOUNDS, qtrue);
}

/*
=======================================================================================================================================
G_TeamCommand

Broadcasts a command to only a specific team.
=======================================================================================================================================
*/
void G_TeamCommand(team_t team, const char *cmd) {
	int i;

	for (i = 0; i < level.maxclients; i++) {
		if (level.clients[i].pers.connected == CON_CONNECTED) {
			if (level.clients[i].sess.sessionTeam == team) {
				trap_SendServerCommand(i, va("%s", cmd));
			}
		}
	}
}

/*
=======================================================================================================================================
G_Find

Searches all active entities for the next one that holds the matching string at fieldofs (use the FOFS() macro) in the structure.
Searches beginning at the entity after from, or the beginning if NULL.
NULL will be returned if the end of the list is reached.
=======================================================================================================================================
*/
gentity_t *G_Find(gentity_t *from, int fieldofs, const char *match) {
	char *s;

	if (!from) {
		from = g_entities;
	} else {
		from++;
	}

	for (; from < &g_entities[level.num_entities]; from++) {
		if (!from->inuse) {
			continue;
		}

		s = *(char **)((byte *)from + fieldofs);

		if (!s) {
			continue;
		}

		if (!Q_stricmp(s, match)) {
			return from;
		}
	}

	return NULL;
}

#define MAXCHOICES 32
/*
=======================================================================================================================================
G_PickTarget

Selects a random entity from among the targets.
=======================================================================================================================================
*/
gentity_t *G_PickTarget(const char *targetname) {
	gentity_t *ent = NULL;
	int num_choices = 0;
	gentity_t *choice[MAXCHOICES];

	if (!targetname) {
		G_Printf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while (1) {
		ent = G_Find(ent, FOFS(targetname), targetname);

		if (!ent) {
			break;
		}

		choice[num_choices++] = ent;

		if (num_choices == MAXCHOICES) {
			break;
		}
	}

	if (!num_choices) {
		G_Printf("G_PickTarget: target %s not found\n", targetname);
		return NULL;
	}

	return choice[rand() % num_choices];
}

/*
=======================================================================================================================================
G_UseTargets

"activator" should be set to the entity that initiated the firing.
Search for (string) targetname in all entities that match (string) self.target and call their .use function.
=======================================================================================================================================
*/
void G_UseTargets(gentity_t *ent, gentity_t *activator) {
	gentity_t *t;
	float f;

	if (!ent) {
		return;
	}

	if (ent->targetShaderName && ent->targetShaderNewName) {
		f = level.time * 0.001f;

		AddRemap(ent->targetShaderName, ent->targetShaderNewName, f);
		trap_SetConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	}

	if (!ent->target) {
		return;
	}

	t = NULL;

	while ((t = G_Find(t, FOFS(targetname), ent->target)) != NULL) {
		if (t == ent) {
			G_Printf("WARNING: Entity used itself.\n");
		} else {
			if (t->use) {
				t->use(t, ent, activator);
			}
		}

		if (!ent->inuse) {
			G_Printf("entity was removed while using targets\n");
			return;
		}
	}
}

/*
=======================================================================================================================================
TempVector

This is just a convenience function for making temporary vectors for function calls.
=======================================================================================================================================
*/
float *TempVector(float x, float y, float z) {
	static int index;
	static vec3_t vecs[8];
	float *v;

	// use an array so that multiple vectors won't collide for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}

/*
=======================================================================================================================================
VectorToString

This is just a convenience function for printing vectors.
=======================================================================================================================================
*/
char *VectorToString(const vec3_t v) {
	static int index;
	static char str[8][32];
	char *s;

	// use an array so that multiple vectors won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf(s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);
	return s;
}

/*
=======================================================================================================================================
G_SetMovedir

The editor only specifies a single value for angles (yaw), but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction instead of an orientation.
=======================================================================================================================================
*/
void G_SetMovedir(vec3_t angles, vec3_t movedir) {

	SetMovedir(angles, movedir);
	VectorClear(angles);
}

/*
=======================================================================================================================================
VectorToYaw
=======================================================================================================================================
*/
float VectorToYaw(const vec3_t vec) {
	float yaw;

	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	} else {
		if (vec[PITCH]) {
			yaw = (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		} else if (vec[YAW] > 0) {
			yaw = 90;
		} else {
			yaw = 270;
		}

		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
=======================================================================================================================================
G_InitGentity
=======================================================================================================================================
*/
void G_InitGentity(gentity_t *e) {

	e->inuse = qtrue;
	e->classname = "noclass";
	e->s.number = e - g_entities;
	e->r.ownerNum = ENTITYNUM_NONE;
	// init scripting
	e->scriptStatus.scriptEventIndex = -1;
}

/*
=======================================================================================================================================
G_Spawn

Either finds a free entity, or allocates a new one.
The slots from 0 to MAX_CLIENTS - 1 are always reserved for clients, and will never be used by anything else.
Try to avoid reusing an entity that was recently freed, because it can cause the client to think the entity morphed into something else
instead of being removed and recreated, which can cause interpolated angles and bad trails.
=======================================================================================================================================
*/
gentity_t *G_Spawn(void) {
	int i, force;
	gentity_t *e;

	e = NULL; // shut up warning

	for (force = 0; force < 2; force++) {
		// if we go through all entities and can't find one to free, override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];

		for (i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
			if (e->inuse) {
				continue;
			}
			// the first couple seconds of server time can involve a lot of freeing and allocating, so relax the replacement policy
			if (!force && e->freetime > level.startTime + 2000 && level.time - e->freetime < 1000) {
				continue;
			}
			// reuse this slot
			G_InitGentity(e);
			return e;
		}

		if (level.num_entities < ENTITYNUM_MAX_NORMAL) {
			break;
		}
	}

	if (level.num_entities == ENTITYNUM_MAX_NORMAL) {
		for (i = 0; i < MAX_GENTITIES; i++) {
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}

		G_Error("G_Spawn: no free entities");
	}
	// open up a new slot
	level.num_entities++;
	// let the server system know that there are more entities
	trap_LocateGameData(level.gentities, level.num_entities, sizeof(gentity_t), &level.clients[0].ps, sizeof(level.clients[0]));
	G_InitGentity(e);
	return e;
}

/*
=======================================================================================================================================
G_EntitiesFree
=======================================================================================================================================
*/
qboolean G_EntitiesFree(void) {
	int i;
	gentity_t *e;

	if (level.num_entities < ENTITYNUM_MAX_NORMAL) {
		// can open a new slot if needed
		return qtrue;
	}

	e = &g_entities[MAX_CLIENTS];

	for (i = MAX_CLIENTS; i < level.num_entities; i++, e++) {
		if (e->inuse) {
			continue;
		}
		// slot available
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
G_FreeEntity

Marks the entity as free.
=======================================================================================================================================
*/
void G_FreeEntity(gentity_t *ent) {

	trap_UnlinkEntity(ent); // unlink from world

	if (ent->neverFree) {
		return;
	}

	memset(ent, 0, sizeof(*ent));

	ent->classname = "freed";
	ent->freetime = level.time;
	ent->inuse = qfalse;
}

/*
=======================================================================================================================================
G_TempEntity

Spawns an event entity that will be auto-removed. The origin will be snapped to save net bandwidth, so care must be taken if the origin
is right on a surface (snap towards start vector first).
=======================================================================================================================================
*/
gentity_t *G_TempEntity(const vec3_t origin, int event) {
	gentity_t *e;
	vec3_t snapped;

	e = G_Spawn();
	e->s.eType = ET_EVENTS + event;
	e->classname = "tempEntity";
	e->eventTime = level.time;
	e->freeAfterEvent = qtrue;

	VectorCopy(origin, snapped);
	SnapVector(snapped); // save network bandwidth
	G_SetOrigin(e, snapped);
	// find cluster for PVS
	trap_LinkEntity(e);

	return e;
}

/*
=======================================================================================================================================
G_KillBox

Kills all entities that would touch the proposed new positioning of ent. Ent should be unlinked before calling this!
=======================================================================================================================================
*/
void G_KillBox(gentity_t *ent) {
	int i, num;
	int touch[MAX_GENTITIES];
	gentity_t *hit;
	vec3_t mins, maxs;

	VectorAdd(ent->client->ps.origin, ent->r.mins, mins);
	VectorAdd(ent->client->ps.origin, ent->r.maxs, maxs);

	num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

	for (i = 0; i < num; i++) {
		hit = &g_entities[touch[i]];

		if (!hit->client) {
			continue;
		}
		// nail it
		G_Damage(hit, ent, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);
	}
}

/*
=======================================================================================================================================
G_AddPredictableEvent

Use for non-pmove events that would also be predicted on the client side: jumppads and item pickups.
Adds an event + parm and twiddles the event counter.
=======================================================================================================================================
*/
void G_AddPredictableEvent(gentity_t *ent, int event, int eventParm) {

	if (!ent->client) {
		return;
	}

	BG_AddPredictableEventToPlayerstate(event, eventParm, &ent->client->ps);
}

/*
=======================================================================================================================================
G_AddEvent

Adds an event + parm and twiddles the event counter.
=======================================================================================================================================
*/
void G_AddEvent(gentity_t *ent, int event, int eventParm) {
	int bits;

	if (!event) {
		G_Printf("G_AddEvent: zero event added for entity %i\n", ent->s.number);
		return;
	}
	// clients need to add the event in playerState_t instead of entityState_t
	if (ent->client) {
		bits = ent->client->ps.externalEvent & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;

		ent->client->ps.externalEvent = event|bits;
		ent->client->ps.externalEventParm = eventParm;
	} else {
		bits = ent->s.event & EV_EVENT_BITS;
		bits = (bits + EV_EVENT_BIT1) & EV_EVENT_BITS;

		ent->s.event = event|bits;
		ent->s.eventParm = eventParm;
	}

	ent->eventTime = level.time;
}

/*
=======================================================================================================================================
G_Sound
=======================================================================================================================================
*/
void G_Sound(gentity_t *ent, int channel, int soundIndex) {
	gentity_t *te;

	te = G_TempEntity(ent->r.currentOrigin, EV_GENERAL_SOUND);
	te->s.eventParm = soundIndex;
}

/*
=======================================================================================================================================
G_SetOrigin

Sets the pos trajectory for a fixed position.
=======================================================================================================================================
*/
void G_SetOrigin(gentity_t *ent, vec3_t origin) {

	VectorCopy(origin, ent->s.pos.trBase);

	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;

	VectorClear(ent->s.pos.trDelta);
	VectorCopy(origin, ent->r.currentOrigin);
}

/*
=======================================================================================================================================
G_SetAngle
=======================================================================================================================================
*/
void G_SetAngle(gentity_t *ent, vec3_t angle) {

	VectorCopy(angle, ent->s.apos.trBase);

	ent->s.apos.trType = TR_STATIONARY;
	ent->s.apos.trTime = 0;
	ent->s.apos.trDuration = 0;

	VectorClear(ent->s.apos.trDelta);
	VectorCopy(angle, ent->r.currentAngles);
}

/*
=======================================================================================================================================
G_GetEntityPlayerState
=======================================================================================================================================
*/
playerState_t *G_GetEntityPlayerState(const gentity_t *ent) {

	if (ent->client) {
		return &ent->client->ps;
	}
/* // Tobias NOTE: enable this!
	if (ent->monster) {
		return &ent->monster->ps;
	}
*/
	return NULL;
}

/*
=======================================================================================================================================
G_GetEntityEventSoundCoefficient

Return a value for how loud events are.

FIXME: WHY CAN'T WE USE 'BotCheckEvents'? is there any easier access to events? It seems like id planned to add the sounds in
       'BotCheckEvents'. If using 'BotCheckEvents' "event only entity" events aren't working.
FIXME: Why are closing doors not heard?

TODO: Adjust the sound distances.
      Ignore some sounds from teammates.
      Don't let us inspect EVERY projectile :)
      Enable the footstep sounds.

NOTE: Another approach of a similar function is 'BotGetEntitySurfaceSoundCoefficient' from ET (wasn't used there). This function will
      support all methods of 'hearing' awareness for different AI (Wolfenstein - Enemy Territory (broken by default),
      Return to Castle Wolfenstein (cs->attributes), and even HUNT monsters. OmniBot isn't tested...
=======================================================================================================================================
*/
int G_GetEntityEventSoundCoefficient(const gentity_t *ent) {

	if (ent->s.eType < ET_EVENTS) {
		switch (ent->s.eType) {
			case ET_PLAYER:
			case ET_MOVER:
				// FIXME: why are closing doors not heard?
				break;
			case ET_MISSILE:
				switch (ent->s.weapon) {
					case WP_ROCKETLAUNCHER:
					case WP_PLASMAGUN:
					case WP_GRENADELAUNCHER:
					case WP_NAILGUN:
					case WP_BFG:
						return 1000;
				}

				break;
			default:
				return 0;
		}
	}

	if (ent->client) {
		if (ent->s.eFlags & EF_FIRING) {
			return 1000;
		}
		// weapon 'hum' sounds
		switch (ent->s.weapon) {
			case WP_BEAMGUN:
			case WP_RAILGUN:
			case WP_BFG:
				return 500;
			default:
				return 0;
		}
	}

	if (ent->eventTime < level.time - 900) {
		return 0;
	}

	switch (ent->s.event & ~EV_EVENT_BITS) {
		case EV_KAMIKAZE:
			return 10000;
		case EV_MISSILE_MISS:
			// explosions are really loud (FIXME: Explosions without impact, e.g.: Proxy mine)
			return 5000;
		case EV_BULLET_HIT_WALL:
			// bullet impacts should attract the bot to inspect (FIXME: only, if the shooter isn't seen or heard)
			return 1750;
		case EV_PLAYER_TELEPORT_IN:
		case EV_PLAYER_TELEPORT_OUT:
			return 1000;
		case EV_PROXIMITY_MINE_STICK:
		case EV_GRENADE_BOUNCE:
		case EV_PAIN:
		case EV_DEATH1:
		case EV_DEATH2:
		case EV_DEATH3:
		case EV_FALL_DIE:
		case EV_FALL_DMG_50:
		case EV_FALL_DMG_25:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_5:
		case EV_FALL_SHORT:
		case EV_JUMP:
			return 500;
		case EV_STEP_4:
		case EV_STEP_8:
		case EV_STEP_12:
		case EV_STEP_16:
			return 400;
/*
		case EV_NOAMMO:
		case EV_CHANGE_WEAPON:
		case EV_FOOTSTEP_HARD:
		case EV_FOOTSTEP_HARD_FROZEN:
		case EV_FOOTSTEP_HARD_SNOW:
		case EV_FOOTSTEP_HARD_SLUSH:
		case EV_FOOTSTEP_PUDDLE:
		case EV_FOOTSTEP_LEAVES:
		case EV_FOOTSTEP_BUSH:
		case EV_FOOTSTEP_GRASS:
		case EV_FOOTSTEP_LONGGRASS:
		case EV_FOOTSTEP_LONGGRASS_MUD:
		case EV_FOOTSTEP_SAND:
		case EV_FOOTSTEP_GRAVEL:
		case EV_FOOTSTEP_RUBBLE:
		case EV_FOOTSTEP_RUBBLE_WET:
		case EV_FOOTSTEP_SOIL:
		case EV_FOOTSTEP_MUD:
		case EV_FOOTSTEP_SNOW_DEEP:
		case EV_FOOTSTEP_ICE:
		case EV_FOOTSTEP_METAL_HOLLOW:
		case EV_FOOTSTEP_METAL_HOLLOW_FROZEN:
		case EV_FOOTSTEP_METAL_HOLLOW_SNOW:
		case EV_FOOTSTEP_METAL_HOLLOW_SLUSH:
		case EV_FOOTSTEP_METAL_HOLLOW_SPLASH:
		case EV_FOOTSTEP_GRATE_01:
		case EV_FOOTSTEP_GRATE_02:
		case EV_FOOTSTEP_DUCT:
		case EV_FOOTSTEP_PLATE:
		case EV_FOOTSTEP_FENCE:
		case EV_FOOTSTEP_WOOD_HOLLOW:
		case EV_FOOTSTEP_WOOD_HOLLOW_FROZEN:
		case EV_FOOTSTEP_WOOD_HOLLOW_SNOW:
		case EV_FOOTSTEP_WOOD_HOLLOW_SLUSH:
		case EV_FOOTSTEP_WOOD_HOLLOW_SPLASH:
		case EV_FOOTSTEP_WOOD_SOLID:
		case EV_FOOTSTEP_WOOD_CREAKING:
		case EV_FOOTSTEP_ROOF:
		case EV_FOOTSTEP_SHINGLES:
		case EV_FOOTSTEP_SOFT:
		case EV_FOOTSTEP_GLASS_SHARDS:
		case EV_FOOTSTEP_TRASH_GLASS:
		case EV_FOOTSTEP_TRASH_DEBRIS:
		case EV_FOOTSTEP_TRASH_WIRE:
		case EV_FOOTSTEP_TRASH_PACKING:
		case EV_FOOTSTEP_TRASH_PLASTIC:
		case EV_FOOTSPLASH:
		case EV_FOOTWADE:
		case EV_SWIM:
		case EV_WATER_TOUCH:			// foot touches
		case EV_WATER_LEAVE:			// foot leaves
		case EV_WATER_UNDER:			// head touches
		case EV_WATER_CLEAR:			// head leaves
			return 200;
*/
		default:
			return 0;
	}
}

/*
=======================================================================================================================================
G_ProcessTagConnect
=======================================================================================================================================
*/
void G_ProcessTagConnect(gentity_t *ent, qboolean clearAngles) {

	if (!ent->tagName) {
		G_Error("G_ProcessTagConnect: NULL ent->tagName\n");
	}

	if (!ent->tagParent) {
		G_Error("G_ProcessTagConnect: NULL ent->tagParent\n");
	}

	G_FindConfigstringIndex(va("%i %i %s", ent->s.number, ent->tagParent->s.number, ent->tagName), CS_TAGCONNECTS, MAX_TAGCONNECTS, qtrue);

	ent->s.eFlags |= EF_TAGCONNECT;

	if (clearAngles) {
		// clear out the angles so it always starts out facing the tag direction
		VectorClear(ent->s.angles);
		VectorCopy(ent->s.angles, ent->s.apos.trBase);

		ent->s.apos.trTime = level.time;
		ent->s.apos.trDuration = 0;
		ent->s.apos.trType = TR_STATIONARY;

		VectorClear(ent->s.apos.trDelta);
		VectorClear(ent->r.currentAngles);
	}
}

/*
=======================================================================================================================================
DebugLine

Debug polygons only work when running a local game with r_debugSurface set to 2.
=======================================================================================================================================
*/
int DebugLine(vec3_t start, vec3_t end, int color) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy(start, points[0]);
	VectorCopy(start, points[1]);
	//points[1][2] -= 2;
	VectorCopy(end, points[2]);
	//points[2][2] -= 2;
	VectorCopy(end, points[3]);
	VectorSubtract(end, start, dir);
	VectorNormalize(dir);

	dot = DotProduct(dir, up);

	if (dot > 0.99 || dot < -0.99) {
		VectorSet(cross, 1, 0, 0);
	} else {
		CrossProduct(dir, up, cross);
	}

	VectorNormalize(cross);

	VectorMA(points[0], 2, cross, points[0]);
	VectorMA(points[1], -2, cross, points[1]);
	VectorMA(points[2], -2, cross, points[2]);
	VectorMA(points[3], 2, cross, points[3]);

	return trap_DebugPolygonCreate(color, 4, points);
}

/*
=======================================================================================================================================
IsPlayerEnt

Returns whether or not the passed entity is a player.
=======================================================================================================================================
*/
qboolean IsPlayerEnt(gentity_t *ent) {

	if (ent&& ent->inuse && ent->client && ent->aiName && !(ent->r.svFlags & SVF_CASTAI) && !(ent->r.svFlags & SVF_BOT) && !Q_stricmp(ent->aiName, "player")) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
ScriptEventForPlayer

Call script event if the passed in entity is a player.
=======================================================================================================================================
*/
qboolean ScriptEventForPlayer( gentity_t *activator, char *eventStr, char *params) {

	if (IsPlayerEnt(activator)) {
		//AICast_ScriptEvent(AICast_GetCastState(activator->s.number), eventStr, params); // Tobias FIXME
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
GetFirstValidPlayer

Returns the first valid player. For triggers/actions that are entity independent.
=======================================================================================================================================
*/
gentity_t *GetFirstValidPlayer(qboolean checkHealth) {
	gentity_t *trav;
	int i;

	for (trav = g_entities, i = 0; i < g_maxclients.integer; i++, trav++) {
		if (!IsPlayerEnt(trav)) {
			continue;
		}

		if (checkHealth && trav->health <= 0) {
			continue;
		}

		return trav;
	}

	return NULL;
}

/*
=======================================================================================================================================
GetFirstValidBluePlayer
=======================================================================================================================================
*/
gentity_t *GetFirstValidBluePlayer(qboolean checkHealth) {
	gentity_t *entity = GetFirstValidPlayer(checkHealth);

	if (entity && G_IsClientOnTeam(entity, TEAM_BLUE)) {
		return entity;
	}

	return NULL;
}
