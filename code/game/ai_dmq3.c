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

#include "g_local.h"
#include "../botlib/aasfile.h"
#include "../botlib/botlib.h"
#include "../botlib/be_aas.h"
#include "../botlib/be_ea.h"
#include "../botlib/be_ai_char.h"
#include "../botlib/be_ai_chat.h"
#include "../botlib/be_ai_gen.h"
#include "../botlib/be_ai_goal.h"
#include "../botlib/be_ai_move.h"
#include "../botlib/be_ai_weap.h"
#include "ai_main.h"
#include "ai_dmq3.h"
#include "ai_chat.h"
#include "ai_cmd.h"
#include "ai_dmnet.h"
#include "ai_team.h"
#include "ai_vcmd.h"
#include "chars.h" // characteristics
#include "inv.h" // indexes into the inventory
#include "syn.h" // synonyms
#include "match.h" // string matching types and vars
#include "../../ui/menudef.h" // for the voice chats

#define MAX_WAYPOINTS 128

bot_waypoint_t botai_waypoints[MAX_WAYPOINTS];
bot_waypoint_t *botai_freewaypoints;
// NOTE: not using a cvar which can be updated because the game should be reloaded anyway
int gametype; // game type

vmCvar_t g_spSkill;

extern vmCvar_t bot_developer;

vec3_t lastteleport_origin; // last teleport event origin
float lastteleport_time; // last teleport event time
int max_bspmodelindex; // maximum BSP model index
// CTF flag goals
bot_goal_t ctf_redflag;
bot_goal_t ctf_blueflag;
bot_goal_t ctf_neutralflag;
bot_goal_t redobelisk;
bot_goal_t blueobelisk;
bot_goal_t neutralobelisk;

#define MAX_ALTROUTEGOALS 32

int altroutegoals_setup;
aas_altroutegoal_t red_altroutegoals[MAX_ALTROUTEGOALS];
int red_numaltroutegoals;
aas_altroutegoal_t blue_altroutegoals[MAX_ALTROUTEGOALS];
int blue_numaltroutegoals;

byte botCheckedAreas[65536];

/*
=======================================================================================================================================
BotSetUserInfo
=======================================================================================================================================
*/
void BotSetUserInfo(bot_state_t *bs, char *key, char *value) {
	char userinfo[MAX_INFO_STRING];

	trap_GetUserinfo(bs->client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, key, value);
	trap_SetUserinfo(bs->client, userinfo);
	ClientUserinfoChanged(bs->client);
}

/*
=======================================================================================================================================
BotCTFCarryingFlag
=======================================================================================================================================
*/
int BotCTFCarryingFlag(bot_state_t *bs) {

	if (gametype != GT_CTF) {
		return CTF_FLAG_NONE;
	}

	if (bs->inventory[INVENTORY_REDFLAG] > 0) {
		return CTF_FLAG_RED;
	} else if (bs->inventory[INVENTORY_BLUEFLAG] > 0) {
		return CTF_FLAG_BLUE;
	}

	return CTF_FLAG_NONE;
}

/*
=======================================================================================================================================
BotTeam
=======================================================================================================================================
*/
int BotTeam(bot_state_t *bs) {

	if (bs->client < 0 || bs->client >= MAX_CLIENTS) {
		return qfalse;
	}

	if (level.clients[bs->client].sess.sessionTeam == TEAM_RED) {
		return TEAM_RED;
	} else if (level.clients[bs->client].sess.sessionTeam == TEAM_BLUE) {
		return TEAM_BLUE;
	}

	return TEAM_FREE;
}

/*
=======================================================================================================================================
BotOppositeTeam
=======================================================================================================================================
*/
int BotOppositeTeam(bot_state_t *bs) {

	switch (BotTeam(bs)) {
		case TEAM_RED:
			return TEAM_BLUE;
		case TEAM_BLUE:
			return TEAM_RED;
		default:
			return TEAM_FREE;
	}
}

/*
=======================================================================================================================================
BotEnemyFlag
=======================================================================================================================================
*/
bot_goal_t *BotEnemyFlag(bot_state_t *bs) {

	if (BotTeam(bs) == TEAM_RED) {
		return &ctf_blueflag;
	} else {
		return &ctf_redflag;
	}
}

/*
=======================================================================================================================================
BotTeamFlag
=======================================================================================================================================
*/
bot_goal_t *BotTeamFlag(bot_state_t *bs) {

	if (BotTeam(bs) == TEAM_RED) {
		return &ctf_redflag;
	} else {
		return &ctf_blueflag;
	}
}

/*
=======================================================================================================================================
EntityIsDead
=======================================================================================================================================
*/
qboolean EntityIsDead(aas_entityinfo_t *entinfo) {
	gentity_t *ent;
	playerState_t *ps;

	// if attacking an obelisk
	if (entinfo->number >= MAX_CLIENTS && (entinfo->number == redobelisk.entitynum || entinfo->number == blueobelisk.entitynum)) {
		// if the obelisk is respawning
		if (g_entities[entinfo->number].activator && g_entities[entinfo->number].activator->s.frame == 2) {
			return qtrue;
		}

		return qfalse;
	}

	if (entinfo->number >= 0 && entinfo->number < level.num_entities) {
		ent = &g_entities[entinfo->number];

		if (!ent->inuse) {
			return qtrue;
		}

		if (!ent->r.linked) {
			return qtrue;
		}

		ps = G_GetEntityPlayerState(ent);

		if (!ps) {
			return qtrue;
		}

		if (ps->pm_type != PM_NORMAL) {
			return qtrue;
		}

		if (ps->stats[STAT_HEALTH] <= 0) {
			return qtrue;
		}

		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
EntityCarriesFlag
=======================================================================================================================================
*/
qboolean EntityCarriesFlag(aas_entityinfo_t *entinfo) {

	if (entinfo->powerups & (1 << PW_REDFLAG)) {
		return qtrue;
	}

	if (entinfo->powerups & (1 << PW_BLUEFLAG)) {
		return qtrue;
	}

	if (entinfo->powerups & (1 << PW_NEUTRALFLAG)) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityIsShooting
=======================================================================================================================================
*/
qboolean EntityIsShooting(aas_entityinfo_t *entinfo) {

	if (entinfo->flags & EF_FIRING) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityIsAlreadyMined
=======================================================================================================================================
*/
int EntityIsAlreadyMined(aas_entityinfo_t *entinfo) {

	if (entinfo->flags & EF_TICKING) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityIsChatting
=======================================================================================================================================
*/
qboolean EntityIsChatting(aas_entityinfo_t *entinfo) {

	if (entinfo->flags & EF_TALK) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityHasQuad
=======================================================================================================================================
*/
qboolean EntityHasQuad(aas_entityinfo_t *entinfo) {

	if (entinfo->powerups & (1 << PW_QUAD)) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityHasKamikaze
=======================================================================================================================================
*/
qboolean EntityHasKamikaze(aas_entityinfo_t *entinfo) {

	if (entinfo->flags & EF_KAMIKAZE) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityCarriesCubes
=======================================================================================================================================
*/
qboolean EntityCarriesCubes(aas_entityinfo_t *entinfo) {
	entityState_t state;

	if (gametype != GT_HARVESTER) {
		return qfalse;
	}
	// FIXME: get this info from the aas_entityinfo_t?
	BotAI_GetEntityState(entinfo->number, &state);

	if (state.tokens > 0) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
EntityIsInvisible
=======================================================================================================================================
*/
qboolean EntityIsInvisible(aas_entityinfo_t *entinfo) {

	// if player is invisible
	if (entinfo->powerups & (1 << PW_INVIS)) {
		// a shooting player is always visible
		if (EntityIsShooting(entinfo)) {
			return qfalse;
		}
		// the flag is always visible
		if (EntityCarriesFlag(entinfo)) {
			return qfalse;
		}
		// cubes are always visible
		if (EntityCarriesCubes(entinfo)) {
			return qfalse;
		}
		// the kamikaze is always visible
		if (EntityHasKamikaze(entinfo)) {
			return qfalse;
		}
		// invisible
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
Bot1FCTFCarryingFlag
=======================================================================================================================================
*/
int Bot1FCTFCarryingFlag(bot_state_t *bs) {

	if (gametype != GT_1FCTF) {
		return qfalse;
	}

	if (bs->inventory[INVENTORY_NEUTRALFLAG] > 0) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotHarvesterCarryingCubes
=======================================================================================================================================
*/
int BotHarvesterCarryingCubes(bot_state_t *bs) {

	if (gametype != GT_HARVESTER) {
		return qfalse;
	}

	if (bs->inventory[INVENTORY_REDCUBE] > 0) {
		return qtrue;
	}

	if (bs->inventory[INVENTORY_BLUECUBE] > 0) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotRememberLastOrderedTask
=======================================================================================================================================
*/
void BotRememberLastOrderedTask(bot_state_t *bs) {

	if (!bs->ordered) {
		return;
	}

	bs->lastgoal_decisionmaker = bs->decisionmaker;
	bs->lastgoal_ltgtype = bs->ltgtype;

	memcpy(&bs->lastgoal_teamgoal, &bs->teamgoal, sizeof(bot_goal_t));

	bs->lastgoal_teammate = bs->teammate;
}

/*
=======================================================================================================================================
BotSetTeamStatus
=======================================================================================================================================
*/
void BotSetTeamStatus(bot_state_t *bs) {
	int teamtask;
	aas_entityinfo_t entinfo;

	teamtask = TEAMTASK_NONE; // Tobias DEBUG

	switch (bs->ltgtype) {
		case LTG_GETFLAG:
			teamtask = TEAMTASK_OFFENSE;
			break;
		case LTG_ATTACKENEMYBASE:
			teamtask = TEAMTASK_OFFENSE;
			break;
		case LTG_HARVEST:
			teamtask = TEAMTASK_OFFENSE;
			break;
		case LTG_DEFENDKEYAREA:
			teamtask = TEAMTASK_DEFENSE;
			break;
		case LTG_RUSHBASE:
			teamtask = TEAMTASK_DEFENSE;
			break;
		case LTG_RETURNFLAG:
			teamtask = TEAMTASK_RETRIEVE;
			break;
		case LTG_TEAMHELP:
			break;
		case LTG_TEAMACCOMPANY:
			// get the entity information
			BotEntityInfo(bs->teammate, &entinfo);
			// if the entity information is valid
			if (!entinfo.valid) {
				break;
			}

			if (((gametype == GT_CTF || gametype == GT_1FCTF) && EntityCarriesFlag(&entinfo)) || (gametype == GT_HARVESTER && EntityCarriesCubes(&entinfo))) {
				teamtask = TEAMTASK_ESCORT;
			} else {
				teamtask = TEAMTASK_FOLLOW;
			}

			break;
		case LTG_CAMP:
		case LTG_CAMPORDER:
			teamtask = TEAMTASK_CAMP;
			break;
// Tobias DEBUG
		case LTG_PATROL:
		case LTG_GETITEM:
		case LTG_KILL:
			teamtask = TEAMTASK_PATROL;
			break;
		default:
			teamtask = TEAMTASK_NONE; // roaming
			break;
// Tobias END
	}

	BotSetUserInfo(bs, "teamtask", va("%d", teamtask));
}

/*
=======================================================================================================================================
BotSetLastOrderedTask
=======================================================================================================================================
*/
int BotSetLastOrderedTask(bot_state_t *bs) {
	bot_goal_t *tb, *eb;
	int tt, et;

	if (gametype == GT_CTF) {
		// don't go back to returning the flag if it's at the base
		if (bs->lastgoal_ltgtype == LTG_RETURNFLAG) {
			if (BotTeam(bs) == TEAM_RED) {
				if (bs->redflagstatus == 0) {
					bs->lastgoal_ltgtype = 0;
				}
			} else {
				if (bs->blueflagstatus == 0) {
					bs->lastgoal_ltgtype = 0;
				}
			}
		}
	}

	if (bs->lastgoal_ltgtype) {
		memcpy(&bs->teamgoal, &bs->lastgoal_teamgoal, sizeof(bot_goal_t));
		// the teammate
		bs->teammate = bs->lastgoal_teammate;
		// the teammate who ordered
		bs->decisionmaker = bs->lastgoal_decisionmaker;
		bs->ordered = qtrue;
		bs->order_time = 0; // Tobias CHECK: was missing 'order_time'?
		// set the ltg type
		bs->ltgtype = bs->lastgoal_ltgtype;
		// set the team goal time
		bs->teamgoal_time = FloatTime() + 300;
		// get an alternate route in ctf
		if (gametype == GT_CTF) {
			if (bs->ltgtype == LTG_GETFLAG) {
				tb = BotTeamFlag(bs);
				eb = BotEnemyFlag(bs);
				tt = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, tb->areanum, TFL_DEFAULT);
				et = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, eb->areanum, TFL_DEFAULT);
				// if the travel time towards the enemy base is larger than towards our base
				if (et > tt) {
					// get an alternative route goal towards the enemy base
					BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
				}
			}
		}
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);

		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotRefuseOrder
=======================================================================================================================================
*/
void BotRefuseOrder(bot_state_t *bs) {

	if (!bs->ordered) {
		return;
	}
	// if the bot was ordered to do something
	if (bs->order_time && bs->order_time > FloatTime() - 10) {
		bs->order_time = 0;
		trap_EA_Action(bs->client, ACTION_NEGATIVE);
		BotVoiceChat(bs, bs->decisionmaker, VOICECHAT_NO);
	}
}

/*
=======================================================================================================================================
BotCTFSeekGoals
=======================================================================================================================================
*/
void BotCTFSeekGoals(bot_state_t *bs) {
	float rnd, l1, l2;
	int flagstatus, c;
	vec3_t dir;
	aas_entityinfo_t entinfo;

	// when carrying a flag in ctf the bot should rush to the base
	if (BotCTFCarryingFlag(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;

			switch (BotTeam(bs)) {
				case TEAM_RED:
					VectorSubtract(ctf_blueflag.origin, bs->origin, dir);
					break;
				case TEAM_BLUE:
					VectorSubtract(ctf_redflag.origin, bs->origin, dir);
					break;
				default:
					VectorSet(dir, 999, 999, 999);
					break;
			}
			// if the bot picked up the flag very close to the enemy base
			if (VectorLength(dir) < 128) {
				// get an alternative route goal through the enemy base
				BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
			} else {
				// don't use any alt route goal, just get the hell out of the base
				bs->altroutegoal.areanum = 0;
			}

			BotSetUserInfo(bs, "teamtask", va("%d", TEAMTASK_OFFENSE));
			BotVoiceChat(bs, -1, VOICECHAT_IHAVEFLAG);
		} else if (bs->rushbaseaway_time > FloatTime()) {
			if (BotTeam(bs) == TEAM_RED) {
				flagstatus = bs->redflagstatus;
			} else {
				flagstatus = bs->blueflagstatus;
			}
			// if the flag is back
			if (flagstatus == 0) {
				// away from rushing to base
				bs->rushbaseaway_time = 0;
			}
		}

		return;
	}
	// if the bot decided to follow someone
	if (bs->ltgtype == LTG_TEAMACCOMPANY && !bs->ordered) {
		// get the entity information
		BotEntityInfo(bs->teammate, &entinfo);
		// if the teammate being accompanied no longer carries the flag
		if (!EntityCarriesFlag(&entinfo)) {
			bs->ltg_time = 0;
			bs->ltgtype = 0;
		}
	}

	if (BotTeam(bs) == TEAM_RED) {
		flagstatus = bs->redflagstatus * 2 + bs->blueflagstatus;
	} else {
		flagstatus = bs->blueflagstatus * 2 + bs->redflagstatus;
	}
	// if our team has the enemy flag and our flag is at the base
	if (flagstatus == 1) {
		if (bs->owndecision_time < FloatTime()) {
			// if not defending the base already
			if (!(bs->ltgtype == LTG_DEFENDKEYAREA && (bs->teamgoal.number == ctf_redflag.number || bs->teamgoal.number == ctf_blueflag.number))) {
				// if there is a visible teammate flag carrier
				c = BotTeamFlagCarrierVisible(bs);
				// if not already following the teammate flag carrier
				if (c >= 0 && (bs->ltgtype != LTG_TEAMACCOMPANY || bs->teammate != c)) {
					BotRefuseOrder(bs);
					// the teammate
					bs->teammate = c;
					// the bot made its own decision
					bs->decisionmaker = bs->client;
					bs->ordered = qfalse;
					// set the ltg type
					bs->ltgtype = LTG_TEAMACCOMPANY;
					// set the team goal time
					bs->teamgoal_time = FloatTime() + TEAM_ACCOMPANY_TIME;
					// last time the teammate was visible
					bs->teammatevisible_time = FloatTime();
					// no arrive message
					bs->arrive_time = 1;
					// set the formation intervening space
					bs->formation_dist = BotSetTeamFormationDist(bs);
					// set the team status (offense, defense etc.)
					BotSetTeamStatus(bs);
					// follow the flag carrier
					BotVoiceChat(bs, bs->teammate, VOICECHAT_ONFOLLOW);
					// no message
					bs->teammessage_time = 0;
					// time the bot made its own decision
					bs->owndecision_time = FloatTime() + 5;
				}
			}
		}

		return;
	// if the enemy has our flag
	} else if (flagstatus == 2) {
		if (bs->owndecision_time < FloatTime()) {
			// if enemy flag carrier is visible
			c = BotEnemyFlagCarrierVisible(bs);

			if (c >= 0) {
				// FIXME: fight enemy flag carrier
			}
			// if not already doing something important
			if (bs->ltgtype != LTG_GETFLAG && bs->ltgtype != LTG_RETURNFLAG && bs->ltgtype != LTG_TEAMHELP && bs->ltgtype != LTG_TEAMACCOMPANY && bs->ltgtype != LTG_CAMPORDER && bs->ltgtype != LTG_PATROL && bs->ltgtype != LTG_GETITEM) {
				BotRefuseOrder(bs);
				// the bot made its own decision
				bs->decisionmaker = bs->client;
				bs->ordered = qfalse;
				// set the ltg type
				if (random() < 0.5) {
					// go for the enemy flag
					bs->ltgtype = LTG_GETFLAG;
				} else {
					bs->ltgtype = LTG_RETURNFLAG;
				}
				// set the time the bot will stop getting the flag
				bs->teamgoal_time = FloatTime() + TEAM_GETFLAG_TIME;
				// get an alternative route goal towards the enemy base
				BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
				// set the team status (offense, defense etc.)
				BotSetTeamStatus(bs);
				// no team message
				bs->teammessage_time = 0;
				// time the bot made its own decision
				bs->owndecision_time = FloatTime() + 5;
			}
		}

		return;
	// if both flags not at their bases
	} else if (flagstatus == 3) {
		if (bs->owndecision_time < FloatTime()) {
			// if not trying to return the flag and not following the team flag carrier
			if (bs->ltgtype != LTG_RETURNFLAG && bs->ltgtype != LTG_TEAMACCOMPANY) {
				// if there is a visible teammate flag carrier
				c = BotTeamFlagCarrierVisible(bs);

				if (c >= 0) {
					BotRefuseOrder(bs);
					// the teammate
					bs->teammate = c;
					// the bot made its own decision
					bs->decisionmaker = bs->client;
					bs->ordered = qfalse;
					// set the ltg type
					bs->ltgtype = LTG_TEAMACCOMPANY;
					// set the team goal time
					bs->teamgoal_time = FloatTime() + TEAM_ACCOMPANY_TIME;
					// last time the teammate was visible
					bs->teammatevisible_time = FloatTime();
					// no arrive message
					bs->arrive_time = 1;
					// set the formation intervening space
					bs->formation_dist = BotSetTeamFormationDist(bs);
					// set the team status (offense, defense etc.)
					BotSetTeamStatus(bs);
					// follow the flag carrier
					BotVoiceChat(bs, bs->teammate, VOICECHAT_ONFOLLOW);
					// no message
					bs->teammessage_time = 0;
					// time the bot made its own decision
					bs->owndecision_time = FloatTime() + 5;
				} else {
					BotRefuseOrder(bs);
					// the bot made its own decision
					bs->decisionmaker = bs->client;
					bs->ordered = qfalse;
					// set the ltg type
					bs->ltgtype = LTG_RETURNFLAG;
					// set the time the bot will stop returning the flag
					bs->teamgoal_time = FloatTime() + TEAM_RETURNFLAG_TIME;
					// get an alternative route goal towards the enemy base
					BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
					// set the team status (offense, defense etc.)
					BotSetTeamStatus(bs);
					// set the time to send a message to the teammates
					bs->teammessage_time = FloatTime() + 2 * random();
					// time the bot made its own decision
					bs->owndecision_time = FloatTime() + 5;
				}
			}
		}

		return;
	}
	// don't just do something wait for the bot team leader to give orders
	if (BotTeamLeader(bs)) {
		return;
	}
	// if the bot is ordered to do something
	if (bs->lastgoal_ltgtype) {
		bs->teamgoal_time += 60;
	}
	// if the bot decided to do something on its own and has a last ordered goal
	if (!bs->ordered && bs->lastgoal_ltgtype) {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
	}
	// if the bot already has a CTF or team goal
	if (bs->ltgtype == LTG_GETFLAG || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_RETURNFLAG || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) {
		return;
	}
	// if ordered to do something
	if (BotSetLastOrderedTask(bs)) {
		return;
	}

	if (bs->owndecision_time > FloatTime()) {
		return;
	}
	// if the bot is roaming
	if (bs->teamroam_time > FloatTime()) {
		return;
	}
	// if the bot has enough aggression to decide what to do
	if (BotAggression(bs) < 50) {
		return;
	}

	if (bs->teamtaskpreference & (TEAMTP_ATTACKER|TEAMTP_DEFENDER)) {
		if (bs->teamtaskpreference & TEAMTP_ATTACKER) {
			l1 = 0.7f;
		} else {
			l1 = 0.2f;
		}

		l2 = 0.9f;
	} else {
		l1 = 0.4f;
		l2 = 0.7f;
	}
	// get the flag or defend the base
	rnd = random();

	if (rnd < l1 && ctf_redflag.areanum && ctf_blueflag.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;
		// set the ltg type
		bs->ltgtype = LTG_GETFLAG;
		// set the time the bot will stop getting the flag
		bs->teamgoal_time = FloatTime() + TEAM_GETFLAG_TIME;
		// get an alternative route goal towards the enemy base
		BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else if (rnd < l2 && ctf_redflag.areanum && ctf_blueflag.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		if (BotTeam(bs) == TEAM_RED) {
			memcpy(&bs->teamgoal, &ctf_redflag, sizeof(bot_goal_t));
		} else {
			memcpy(&bs->teamgoal, &ctf_blueflag, sizeof(bot_goal_t));
		}
		// set the ltg type
		bs->ltgtype = LTG_DEFENDKEYAREA;
		// set the time the bot stops defending the base
		bs->teamgoal_time = FloatTime() + TEAM_DEFENDKEYAREA_TIME;
		// away from defending
		bs->defendaway_time = 0;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
		// set the time the bot will stop roaming
		bs->teamroam_time = FloatTime() + TEAM_ROAM_TIME;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	}
	// set the time to send a message to the teammates
	bs->teammessage_time = FloatTime() + 2 * random();
	// time the bot made its own decision
	bs->owndecision_time = FloatTime() + 5;
#ifdef DEBUG
	BotPrintTeamGoal(bs);
#endif // DEBUG
}

/*
=======================================================================================================================================
BotCTFRetreatGoals
=======================================================================================================================================
*/
void BotCTFRetreatGoals(bot_state_t *bs) {

	// when carrying a flag in ctf the bot should rush to the base
	if (BotCTFCarryingFlag(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
		}
	}
}

/*
=======================================================================================================================================
Bot1FCTFSeekGoals
=======================================================================================================================================
*/
void Bot1FCTFSeekGoals(bot_state_t *bs) {
	aas_entityinfo_t entinfo;
	float rnd, l1, l2;
	int c;

	// when carrying the flag in 1ctf the bot should rush to the enemy base
	if (Bot1FCTFCarryingFlag(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;
			// get an alternative route goal towards the enemy base
			BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
			BotVoiceChat(bs, -1, VOICECHAT_IHAVEFLAG);
		}

		return;
	}
	// if the bot decided to follow someone
	if (bs->ltgtype == LTG_TEAMACCOMPANY && !bs->ordered) {
		// get the entity information
		BotEntityInfo(bs->teammate, &entinfo);
		// if the teammate being accompanied no longer carries the flag
		if (!EntityCarriesFlag(&entinfo)) {
			bs->ltg_time = 0;
			bs->ltgtype = 0;
		}
	}
	// our team has the flag
	if (bs->neutralflagstatus == 1) {
		if (bs->owndecision_time < FloatTime()) {
			// if not already following someone
			if (bs->ltgtype != LTG_TEAMACCOMPANY) {
				// if there is a visible teammate flag carrier
				c = BotTeamFlagCarrierVisible(bs);

				if (c >= 0) {
					BotRefuseOrder(bs);
					// the teammate
					bs->teammate = c;
					// the bot made its own decision
					bs->decisionmaker = bs->client;
					bs->ordered = qfalse;
					// set the ltg type
					bs->ltgtype = LTG_TEAMACCOMPANY;
					// set the team goal time
					bs->teamgoal_time = FloatTime() + TEAM_ACCOMPANY_TIME;
					// last time the teammate was visible
					bs->teammatevisible_time = FloatTime();
					// no arrive message
					bs->arrive_time = 1;
					// set the formation intervening space
					bs->formation_dist = BotSetTeamFormationDist(bs);
					// set the team status (offense, defense etc.)
					BotSetTeamStatus(bs);
					// follow the flag carrier
					BotVoiceChat(bs, bs->teammate, VOICECHAT_ONFOLLOW);
					// no message
					bs->teammessage_time = 0;
					// time the bot made its own decision
					bs->owndecision_time = FloatTime() + 5;
					return;
				}
			}
			// if the bot already has a CTF or team goal
			if (bs->ltgtype == LTG_GETFLAG || bs->ltgtype == LTG_ATTACKENEMYBASE || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) {
				return;
			}
			// if not already attacking the enemy base
			if (bs->ltgtype != LTG_ATTACKENEMYBASE) {
				BotRefuseOrder(bs);
				// the bot made its own decision
				bs->decisionmaker = bs->client;
				bs->ordered = qfalse;

				if (BotTeam(bs) == TEAM_RED) {
					memcpy(&bs->teamgoal, &ctf_blueflag, sizeof(bot_goal_t));
				} else {
					memcpy(&bs->teamgoal, &ctf_redflag, sizeof(bot_goal_t));
				}
				// set the ltg type
				bs->ltgtype = LTG_ATTACKENEMYBASE;
				// set the time the bot will stop getting the flag
				bs->teamgoal_time = FloatTime() + TEAM_ATTACKENEMYBASE_TIME;
				// set the team status (offense, defense etc.)
				BotSetTeamStatus(bs);
				// time the bot made its own decision
				bs->owndecision_time = FloatTime() + 5;
			}
		}

		return;
	// enemy team has the flag
	} else if (bs->neutralflagstatus == 2) {
		if (bs->owndecision_time < FloatTime()) {
			c = BotEnemyFlagCarrierVisible(bs);

			if (c >= 0) {
				// FIXME: attack enemy flag carrier
			}
			// if the bot already has a CTF or team goal
			if (bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) {
				return;
			}
			// if not already defending the base
			if (bs->ltgtype != LTG_DEFENDKEYAREA) {
				BotRefuseOrder(bs);
				// the bot made its own decision
				bs->decisionmaker = bs->client;
				bs->ordered = qfalse;

				if (BotTeam(bs) == TEAM_RED) {
					memcpy(&bs->teamgoal, &ctf_redflag, sizeof(bot_goal_t));
				} else {
					memcpy(&bs->teamgoal, &ctf_blueflag, sizeof(bot_goal_t));
				}
				// set the ltg type
				bs->ltgtype = LTG_DEFENDKEYAREA;
				// set the time the bot stops defending the base
				bs->teamgoal_time = FloatTime() + TEAM_DEFENDKEYAREA_TIME;
				// away from defending
				bs->defendaway_time = 0;
				// set the team status (offense, defense etc.)
				BotSetTeamStatus(bs);
				// time the bot made its own decision
				bs->owndecision_time = FloatTime() + 5;
			}
		}

		return;
	}
	// don't just do something wait for the bot team leader to give orders
	if (BotTeamLeader(bs)) {
		return;
	}
	// if the bot is ordered to do something
	if (bs->lastgoal_ltgtype) {
		bs->teamgoal_time += 60;
	}
	// if the bot decided to do something on its own and has a last ordered goal
	if (!bs->ordered && bs->lastgoal_ltgtype) {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
	}
	// if the bot already has a CTF or team goal
	if (bs->ltgtype == LTG_GETFLAG || bs->ltgtype == LTG_ATTACKENEMYBASE || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_RETURNFLAG || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) { // Tobias CHECK: LTG_RETURNFLAG?
		return;
	}
	// if ordered to do something
	if (BotSetLastOrderedTask(bs)) {
		return;
	}

	if (bs->owndecision_time > FloatTime()) {
		return;
	}
	// if the bot is roaming
	if (bs->teamroam_time > FloatTime()) {
		return;
	}
	// if the bot has enough aggression to decide what to do
	if (BotAggression(bs) < 50) {
		return;
	}

	if (bs->teamtaskpreference & (TEAMTP_ATTACKER|TEAMTP_DEFENDER)) {
		if (bs->teamtaskpreference & TEAMTP_ATTACKER) {
			l1 = 0.7f;
		} else {
			l1 = 0.2f;
		}

		l2 = 0.9f;
	} else {
		l1 = 0.4f;
		l2 = 0.7f;
	}
	// get the flag or defend the base
	rnd = random();

	if (rnd < l1 && ctf_neutralflag.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;
		// set the ltg type
		bs->ltgtype = LTG_GETFLAG;
		// set the time the bot will stop getting the flag
		bs->teamgoal_time = FloatTime() + TEAM_GETFLAG_TIME;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else if (rnd < l2 && ctf_redflag.areanum && ctf_blueflag.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		if (BotTeam(bs) == TEAM_RED) {
			memcpy(&bs->teamgoal, &ctf_redflag, sizeof(bot_goal_t));
		} else {
			memcpy(&bs->teamgoal, &ctf_blueflag, sizeof(bot_goal_t));
		}
		// set the ltg type
		bs->ltgtype = LTG_DEFENDKEYAREA;
		// set the time the bot stops defending the base
		bs->teamgoal_time = FloatTime() + TEAM_DEFENDKEYAREA_TIME;
		// away from defending
		bs->defendaway_time = 0;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
		// set the time the bot will stop roaming
		bs->teamroam_time = FloatTime() + TEAM_ROAM_TIME;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	}
	// set the time to send a message to the teammates
	bs->teammessage_time = FloatTime() + 2 * random();
	// time the bot made its own decision
	bs->owndecision_time = FloatTime() + 5;
#ifdef DEBUG
	BotPrintTeamGoal(bs);
#endif // DEBUG
}

/*
=======================================================================================================================================
Bot1FCTFRetreatGoals
=======================================================================================================================================
*/
void Bot1FCTFRetreatGoals(bot_state_t *bs) {

	// when carrying the flag in 1ctf the bot should rush to the enemy base
	if (Bot1FCTFCarryingFlag(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;
			// get an alternative route goal towards the enemy base
			BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
		}
	}
}

/*
=======================================================================================================================================
BotObeliskSeekGoals
=======================================================================================================================================
*/
void BotObeliskSeekGoals(bot_state_t *bs) {
	float rnd, l1, l2;

	// don't just do something wait for the bot team leader to give orders
	if (BotTeamLeader(bs)) {
		return;
	}
	// if the bot is ordered to do something
	if (bs->lastgoal_ltgtype) {
		bs->teamgoal_time += 60;
	}
	// if the bot already has a CTF or team goal
	if (bs->ltgtype == LTG_ATTACKENEMYBASE || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || /*bs->ltgtype == LTG_GETFLAG || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_RETURNFLAG || */bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) { // Tobias CHECK!
		return;
	}
	// if ordered to do something
	if (BotSetLastOrderedTask(bs)) {
		return;
	}
	// if the bot is roaming
	if (bs->teamroam_time > FloatTime()) {
		return;
	}
	// if the bot has enough aggression to decide what to do
	if (BotAggression(bs) < 50) {
		return;
	}

	if (bs->teamtaskpreference & (TEAMTP_ATTACKER|TEAMTP_DEFENDER)) {
		if (bs->teamtaskpreference & TEAMTP_ATTACKER) {
			l1 = 0.7f;
		} else {
			l1 = 0.2f;
		}

		l2 = 0.9f;
	} else {
		l1 = 0.4f;
		l2 = 0.7f;
	}
	// attack the enemy base or defend the base
	rnd = random();

	if (rnd < l1 && redobelisk.areanum && blueobelisk.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		if (BotTeam(bs) == TEAM_RED) {
			memcpy(&bs->teamgoal, &blueobelisk, sizeof(bot_goal_t));
		} else {
			memcpy(&bs->teamgoal, &redobelisk, sizeof(bot_goal_t));
		}
		// set the ltg type
		bs->ltgtype = LTG_ATTACKENEMYBASE;
		// set the time the bot will stop attacking the enemy base
		bs->teamgoal_time = FloatTime() + TEAM_ATTACKENEMYBASE_TIME;
		// get an alternate route goal towards the enemy base
		BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else if (rnd < l2 && redobelisk.areanum && blueobelisk.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		if (BotTeam(bs) == TEAM_RED) {
			memcpy(&bs->teamgoal, &redobelisk, sizeof(bot_goal_t));
		} else {
			memcpy(&bs->teamgoal, &blueobelisk, sizeof(bot_goal_t));
		}
		// set the ltg type
		bs->ltgtype = LTG_DEFENDKEYAREA;
		// set the time the bot stops defending the base
		bs->teamgoal_time = FloatTime() + TEAM_DEFENDKEYAREA_TIME;
		// away from defending
		bs->defendaway_time = 0;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
		// set the time the bot will stop roaming
		bs->teamroam_time = FloatTime() + TEAM_ROAM_TIME;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	}
	// set the time to send a message to the teammates
	bs->teammessage_time = FloatTime() + 2 * random();
}

/*
=======================================================================================================================================
BotGoHarvest
=======================================================================================================================================
*/
void BotGoHarvest(bot_state_t *bs) {

	if (BotTeam(bs) == TEAM_RED) {
		memcpy(&bs->teamgoal, &blueobelisk, sizeof(bot_goal_t));
	} else {
		memcpy(&bs->teamgoal, &redobelisk, sizeof(bot_goal_t));
	}
	// set the ltg type
	bs->ltgtype = LTG_HARVEST;
	// set the time the bot will stop harvesting
	bs->teamgoal_time = FloatTime() + TEAM_HARVEST_TIME;
	// away from harvesting
	bs->harvestaway_time = 0;
	// set the team status (offense, defense etc.)
	BotSetTeamStatus(bs);
}

/*
=======================================================================================================================================
BotObeliskRetreatGoals
=======================================================================================================================================
*/
void BotObeliskRetreatGoals(bot_state_t *bs) {
	// nothing special
}

/*
=======================================================================================================================================
BotHarvesterSeekGoals
=======================================================================================================================================
*/
void BotHarvesterSeekGoals(bot_state_t *bs) {
	aas_entityinfo_t entinfo;
	float rnd, l1, l2;
	int c;

	// when carrying cubes in harvester the bot should rush to the enemy base
	if (BotHarvesterCarryingCubes(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;
			// get an alternative route goal towards the enemy base
			BotGetAlternateRouteGoal(bs, BotOppositeTeam(bs));
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
		}

		return;
	}
	// don't just do something wait for the bot team leader to give orders
	if (BotTeamLeader(bs)) {
		return;
	}
	// if the bot decided to follow someone
	if (bs->ltgtype == LTG_TEAMACCOMPANY && !bs->ordered) {
		// get the entity information
		BotEntityInfo(bs->teammate, &entinfo);
		// if the teammate being accompanied no longer carries cubes
		if (!EntityCarriesCubes(&entinfo)) {
			bs->ltg_time = 0;
			bs->ltgtype = 0;
		}
	}
	// if the bot is ordered to do something
	if (bs->lastgoal_ltgtype) {
		bs->teamgoal_time += 60;
	}
	// if the bot already has a CTF or team goal
	if (bs->ltgtype == LTG_ATTACKENEMYBASE || bs->ltgtype == LTG_HARVEST || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM) { // Tobias CHECK: also added LTG_RUSHBASE
		return;
	}
	// if ordered to do something
	if (BotSetLastOrderedTask(bs)) {
		return;
	}
	// if the bot is roaming
	if (bs->teamroam_time > FloatTime()) {
		return;
	}
	// if the bot has enough aggression to decide what to do
	if (BotAggression(bs) < 50) {
		return;
	}

	c = BotEnemyCubeCarrierVisible(bs);

	if (c >= 0) {
		// FIXME: attack enemy cube carrier
	}

	if (bs->ltgtype != LTG_TEAMACCOMPANY) {
		// if there is a visible teammate carrying cubes
		c = BotTeamCubeCarrierVisible(bs);

		if (c >= 0) {
			// the teammate
			bs->teammate = c;
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_TEAMACCOMPANY;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_ACCOMPANY_TIME;
			// last time the teammate was visible
			bs->teammatevisible_time = FloatTime();
			// no arrive message
			bs->arrive_time = 1;
			// set the formation intervening space
			bs->formation_dist = BotSetTeamFormationDist(bs);
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
			// follow the teammate carrying cubes
			BotVoiceChat(bs, bs->teammate, VOICECHAT_ONFOLLOW);
			// no message
			bs->teammessage_time = 0;
			return;
		}
	}

	if (bs->teamtaskpreference & (TEAMTP_ATTACKER|TEAMTP_DEFENDER)) {
		if (bs->teamtaskpreference & TEAMTP_ATTACKER) {
			l1 = 0.7f;
		} else {
			l1 = 0.2f;
		}

		l2 = 0.9f;
	} else {
		l1 = 0.4f;
		l2 = 0.7f;
	}

	rnd = random();

	if (rnd < l1 && redobelisk.areanum && blueobelisk.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		BotGoHarvest(bs);
	} else if (rnd < l2 && redobelisk.areanum && blueobelisk.areanum) {
		// the bot made its own decision
		bs->decisionmaker = bs->client;
		bs->ordered = qfalse;

		if (BotTeam(bs) == TEAM_RED) {
			memcpy(&bs->teamgoal, &redobelisk, sizeof(bot_goal_t));
		} else {
			memcpy(&bs->teamgoal, &blueobelisk, sizeof(bot_goal_t));
		}
		// set the ltg type
		bs->ltgtype = LTG_DEFENDKEYAREA;
		// set the time the bot stops defending the base
		bs->teamgoal_time = FloatTime() + TEAM_DEFENDKEYAREA_TIME;
		// away from defending
		bs->defendaway_time = 0;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	} else {
		bs->ltg_time = 0;
		bs->ltgtype = 0;
		// set the time the bot will stop roaming
		bs->teamroam_time = FloatTime() + TEAM_ROAM_TIME;
		// set the team status (offense, defense etc.)
		BotSetTeamStatus(bs);
	}
	// set the time to send a message to the teammates
	bs->teammessage_time = FloatTime() + 2 * random();
}

/*
=======================================================================================================================================
BotHarvesterRetreatGoals
=======================================================================================================================================
*/
void BotHarvesterRetreatGoals(bot_state_t *bs) {

	// when carrying cubes in harvester the bot should rush to the enemy base
	if (BotHarvesterCarryingCubes(bs)) {
		// if not already rushing to the base
		if (bs->ltgtype != LTG_RUSHBASE) {
			BotRefuseOrder(bs);
			// the bot made its own decision
			bs->decisionmaker = bs->client;
			bs->ordered = qfalse;
			// set the ltg type
			bs->ltgtype = LTG_RUSHBASE;
			// set the team goal time
			bs->teamgoal_time = FloatTime() + TEAM_RUSHBASE_TIME;
			// away from rushing to base
			bs->rushbaseaway_time = 0;
			// set the team status (offense, defense etc.)
			BotSetTeamStatus(bs);
		}

		return;
	}
}

/*
=======================================================================================================================================
BotTeamGoals
=======================================================================================================================================
*/
void BotTeamGoals(bot_state_t *bs, int retreat) {

	if (retreat) {
		if (gametype == GT_CTF) {
			BotCTFRetreatGoals(bs);
		} else if (gametype == GT_1FCTF) {
			Bot1FCTFRetreatGoals(bs);
		} else if (gametype == GT_OBELISK) {
			BotObeliskRetreatGoals(bs);
		} else if (gametype == GT_HARVESTER) {
			BotHarvesterRetreatGoals(bs);
		}
	} else {
		if (gametype == GT_CTF) {
			// decide what to do in CTF mode
			BotCTFSeekGoals(bs);
		} else if (gametype == GT_1FCTF) {
			Bot1FCTFSeekGoals(bs);
		} else if (gametype == GT_OBELISK) {
			BotObeliskSeekGoals(bs);
		} else if (gametype == GT_HARVESTER) {
			BotHarvesterSeekGoals(bs);
		}
	}
	// reset the order time which is used to see if we decided to refuse an order
	bs->order_time = 0;
}

/*
=======================================================================================================================================
BotFirstReachabilityArea
=======================================================================================================================================
*/
int BotFirstReachabilityArea(int entnum, vec3_t origin, int *areas, int numareas, qboolean distCheck) {
	int i, best;
	vec3_t center;
	float bestDist, dist;
	bsp_trace_t trace;
	vec3_t mins, maxs;

	best = 0;
	bestDist = 999999;

	if (entnum >= 0 && entnum < level.maxclients) {
		trap_AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
		mins[2] += STEPSIZE;
	} else {
		VectorCopy(vec3_origin, mins);
		VectorCopy(vec3_origin, maxs);
	}

	for (i = 0; i < numareas; i++) {
		if (botCheckedAreas[areas[i]]) {
			continue;
		}

		botCheckedAreas[areas[i]] = 1;

		if (trap_AAS_AreaReachability(areas[i])) {
			// make sure this area is visible
			//if (!trap_AAS_AreaWaypoint(areas[i], center)) {
				trap_AAS_AreaCenter(areas[i], center);
			//}

			if (distCheck) {
				dist = VectorDistance(center, origin);

				if (center[2] > origin[2]) {
					dist += 32 * (center[2] - origin[2]);
				}

				if (dist < bestDist) {
					trap_Trace(&trace, origin, mins, maxs, center, -1, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP);
					// if no solids were found
					if (!trace.startsolid && !trace.allsolid && trace.fraction >= 1.0f) {
						bestDist = dist;
						best = areas[i];
						break;
					}
				}
			} else {
				trap_Trace(&trace, origin, mins, maxs, center, -1, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP);
				// if no solids were found
				if (!trace.startsolid && !trace.allsolid && trace.fraction >= 1.0f) {
					best = areas[i];
					break;
				}
			}
		}
	}

	return best;
}

/*
=======================================================================================================================================
BotPointAreaNum
=======================================================================================================================================
*/
int BotPointAreaNum(int entnum, vec3_t origin) {
	int areanum, numareas, areas[50], bestarea, i;
	vec3_t end, start, ofs, mins, maxs;
	float f;
	gentity_t *ent;

	bestarea = 0;

	if (entnum >= 0 && VectorCompare(origin, g_entities[entnum].botAreaPos)) {
		return g_entities[entnum].botAreaNum;
	}

	memset(botCheckedAreas, 0, sizeof(botCheckedAreas));

	ent = NULL;

	if (entnum >= 0) {
		ent = &g_entities[entnum];
	}/*
	// if this is a bot, and it's touching a ladder, do special handling
	if (ent && ent->client && ent->client->ps.pm_flags & PMF_LADDER) {
		// use the point only if its a ladder area
		areanum = trap_AAS_PointAreaNum(origin);

		if (areanum && !trap_AAS_AreaLadder(areanum)) {
			areanum = 0;
		}

		if (areanum) {
			bestarea = areanum;

			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
		// try a small box, and take a ladder area as preference
		maxs[0] = 8;
		maxs[1] = 8;
		maxs[2] = 4;

		VectorSubtract(origin, maxs, mins);
		VectorAdd(origin, maxs, maxs);

		numareas = trap_AAS_BBoxAreas(mins, maxs, areas, 50);

		if (numareas > 0) {
			bestarea = BotFirstLadderArea(entnum, areas, numareas);
		}

		if (bestarea) {
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
		// try the actual point
		areanum = trap_AAS_PointAreaNum(origin);

		if (areanum && !trap_AAS_AreaReachability(areanum)) {
			areanum = 0;
		}

		if (areanum) {
			bestarea = areanum;
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
	} else */{
		areanum = trap_AAS_PointAreaNum(origin);

		if (areanum && !trap_AAS_AreaReachability(areanum)) {
			areanum = 0;
		}

		if (areanum) {
			bestarea = areanum;

			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
		// trace a line from below us, upwards, finding the first area the line touches
		VectorCopy(origin, start);
		VectorCopy(origin, end);

		start[2] -= 30;

		if (entnum >= 0 && g_entities[entnum].inuse && g_entities[entnum].client) {
			end[2] += g_entities[entnum].client->ps.viewheight;
		}

		numareas = trap_AAS_TraceAreas(start, end, areas, NULL, 50);

		if (numareas > 0) {
			bestarea = BotFirstReachabilityArea(entnum, origin, areas, numareas, qfalse);
		}

		if (bestarea) {
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
		// try a small box around the origin
		maxs[0] = 4;
		maxs[1] = 4;
		maxs[2] = 4;

		VectorSubtract(origin, maxs, mins);
		VectorAdd(origin, maxs, maxs);

		numareas = trap_AAS_BBoxAreas(mins, maxs, areas, 50);

		if (numareas > 0) {
			bestarea = BotFirstReachabilityArea(entnum, origin, areas, numareas, qtrue);
		}

		if (bestarea) {
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
	}
	// try using the players bounding box
	if (entnum >= 0 && g_entities[entnum].inuse && g_entities[entnum].client) {
		numareas = trap_AAS_BBoxAreas(g_entities[entnum].r.absmin, g_entities[entnum].r.absmax, areas, 50);

		if (numareas > 0) {
			bestarea = BotFirstReachabilityArea(entnum, origin, areas, numareas, qtrue);
		}

		if (bestarea) {
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
	}
	// TODO: the following code seems to often cause bogus areanums to be returned. They are offset from the real areas, and this causes all sorts of bot stickiness
	// try half size first
	for (f = 0.1f; f <= 1.0f; f += 0.45f) {
		VectorCopy(origin, end);

		end[2] += 80;

		VectorCopy(origin, ofs);

		ofs[2] -= 60;

		for (i = 0; i < 2; i++) {
			end[i] += BOTAREA_BOX_DIST * f;
		}

		for (i = 0; i < 2; i++) {
			ofs[i] -= BOTAREA_BOX_DIST * f;
		}

		numareas = trap_AAS_BBoxAreas(ofs, end, areas, 50);

		if (numareas > 0) {
			bestarea = BotFirstReachabilityArea(entnum, origin, areas, numareas, qtrue);
		}

		if (bestarea) {
			if (entnum >= 0) {
				VectorCopy(origin, g_entities[entnum].botAreaPos);
				g_entities[entnum].botAreaNum = bestarea;
			}

			return bestarea;
		}
	}

	return bestarea;
}

/*
=======================================================================================================================================
ClientSkin
=======================================================================================================================================
*/
char *ClientSkin(int client, char *skin, int size) {
	char buf[MAX_INFO_STRING];

	if (client < 0 || client >= MAX_CLIENTS) {
		BotAI_Print(PRT_ERROR, "ClientSkin: client out of range\n");
		return "[client out of range]";
	}

	trap_GetConfigstring(CS_PLAYERS + client, buf, sizeof(buf));
	strncpy(skin, Info_ValueForKey(buf, "model"), size - 1);

	skin[size - 1] = '\0';
	return skin;
}

/*
=======================================================================================================================================
ClientName
=======================================================================================================================================
*/
char *ClientName(int client, char *name, int size) {
	char buf[MAX_INFO_STRING];

	if (client < 0 || client >= MAX_CLIENTS) {
		BotAI_Print(PRT_ERROR, "ClientName: client out of range\n");
		return "[client out of range]";
	}

	trap_GetConfigstring(CS_PLAYERS + client, buf, sizeof(buf));
	strncpy(name, Info_ValueForKey(buf, "n"), size - 1);

	name[size - 1] = '\0';

	Q_CleanStr(name);
	return name;
}

/*
=======================================================================================================================================
ClientFromName
=======================================================================================================================================
*/
int ClientFromName(char *name) {
	int i;
	char buf[MAX_INFO_STRING];

	for (i = 0; i < level.maxclients; i++) {
		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		Q_CleanStr(buf);

		if (!Q_stricmp(Info_ValueForKey(buf, "n"), name)) {
			return i;
		}
	}

	return -1;
}

/*
=======================================================================================================================================
ClientOnSameTeamFromName
=======================================================================================================================================
*/
int ClientOnSameTeamFromName(bot_state_t *bs, char *name) {
	int i;
	char buf[MAX_INFO_STRING];

	for (i = 0; i < level.maxclients; i++) {
		if (!BotSameTeam(bs, i)) {
			continue;
		}

		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		Q_CleanStr(buf);

		if (!Q_stricmp(Info_ValueForKey(buf, "n"), name)) {
			return i;
		}
	}

	return -1;
}

/*
=======================================================================================================================================
stristr
=======================================================================================================================================
*/
char *stristr(char *str, char *charset) {
	int i;

	while (*str) {
		for (i = 0; charset[i] && str[i]; i++) {
			if (toupper(charset[i]) != toupper(str[i])) {
				break;
			}
		}

		if (!charset[i]) {
			return str;
		}

		str++;
	}

	return NULL;
}

/*
=======================================================================================================================================
EasyClientName
=======================================================================================================================================
*/
char *EasyClientName(int client, char *buf, int size) {
	int i;
	char *str1, *str2, *ptr, c;
	char name[128] = {0};

	ClientName(client, name, sizeof(name));

	for (i = 0; name[i]; i++) {
		name[i] &= 127;
	}
	// remove all spaces
	for (ptr = strstr(name, " "); ptr; ptr = strstr(name, " ")) {
		memmove(ptr, ptr + 1, strlen(ptr + 1) + 1);
	}
	// check for [x] and ]x[ clan names
	str1 = strstr(name, "[");
	str2 = strstr(name, "]");

	if (str1 && str2) {
		if (str2 > str1) {
			memmove(str1, str2 + 1, strlen(str2 + 1) + 1);
		} else {
			memmove(str2, str1 + 1, strlen(str1 + 1) + 1);
		}
	}
	// remove Mr and Ms prefix
	if ((name[0] == 'm' || name[0] == 'M') && (name[1] == 'r' || name[1] == 'R' || name[1] == 's' || name[1] == 'S')) {
		memmove(name, name + 2, strlen(name + 2) + 1);
	}
	// only allow lower case alphabet characters
	ptr = name;
	// allow first character being upper case
	if (*ptr >= 'A' && *ptr <= 'Z') {
		ptr++;
	}

	while (*ptr) {
		c = *ptr;

		if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
			ptr++;
		} else if (c >= 'A' && c <= 'Z') {
			*ptr += 'a' - 'A';
			ptr++;
		} else {
			memmove(ptr, ptr + 1, strlen(ptr + 1) + 1);
		}
	}

	strncpy(buf, name, size - 1);

	buf[size - 1] = '\0';
	return buf;
}

/*
=======================================================================================================================================
BotSynonymContext
=======================================================================================================================================
*/
int BotSynonymContext(bot_state_t *bs) {
	int context;

	context = CONTEXT_NORMAL|CONTEXT_NEARBYITEM|CONTEXT_NAMES;

	if (gametype == GT_CTF || gametype == GT_1FCTF) {
		if (BotTeam(bs) == TEAM_RED) {
			context |= CONTEXT_CTFREDTEAM;
		} else {
			context |= CONTEXT_CTFBLUETEAM;
		}
	} else if (gametype == GT_OBELISK) {
		if (BotTeam(bs) == TEAM_RED) {
			context |= CONTEXT_OBELISKREDTEAM;
		} else {
			context |= CONTEXT_OBELISKBLUETEAM;
		}
	} else if (gametype == GT_HARVESTER) {
		if (BotTeam(bs) == TEAM_RED) {
			context |= CONTEXT_HARVESTERREDTEAM;
		} else {
			context |= CONTEXT_HARVESTERBLUETEAM;
		}
	}

	return context;
}

/*
=======================================================================================================================================
BotUsesLongRangeInstantHitWeapon
=======================================================================================================================================
*/
/*
static const qboolean BotUsesLongRangeInstantHitWeapon(bot_state_t *bs) {

	switch (bs->weaponnum) {
		case WP_RAILGUN:
			return qtrue;
		default:
			return qfalse;
	}
}
*/
/*
=======================================================================================================================================
BotUsesMidRangeWeapon
=======================================================================================================================================
*/
static const qboolean BotUsesMidRangeWeapon(bot_state_t *bs) {

	switch (bs->weaponnum) {
		case WP_MACHINEGUN:
		case WP_CHAINGUN:
		case WP_NAPALMLAUNCHER:
		case WP_ROCKETLAUNCHER:
		case WP_BEAMGUN:
		case WP_RAILGUN:
		case WP_PLASMAGUN:
		case WP_BFG:
			return qtrue;
		default:
			return qfalse;
	}
}

/*
=======================================================================================================================================
BotUsesGravityAffectedProjectileWeapon
=======================================================================================================================================
*/
static const qboolean BotUsesGravityAffectedProjectileWeapon(bot_state_t *bs) {

	switch (bs->weaponnum) {
		case WP_PROXLAUNCHER:
		case WP_GRENADELAUNCHER:
		case WP_NAPALMLAUNCHER:
			return qtrue;
		default:
			return qfalse;
	}
}

/*
=======================================================================================================================================
BotUsesInstantHitWeapon
=======================================================================================================================================
*/
static const qboolean BotUsesInstantHitWeapon(bot_state_t *bs) {

	switch (bs->weaponnum) {
		case WP_GAUNTLET:
		case WP_MACHINEGUN:
		case WP_CHAINGUN:
		case WP_SHOTGUN:
		case WP_BEAMGUN:
		case WP_RAILGUN:
			return qtrue;
		default:
			return qfalse;
	}
}

/*
=======================================================================================================================================
BotUsesCloseCombatWeapon
=======================================================================================================================================
*/
static const qboolean BotUsesCloseCombatWeapon(bot_state_t *bs) {

	switch (bs->weaponnum) {
		case WP_GAUNTLET:
			return qtrue;
		default:
			return qfalse;
	}
}

/*
=======================================================================================================================================
BotChooseWeapon
=======================================================================================================================================
*/
void BotChooseWeapon(bot_state_t *bs) {
	int newweaponnum;

	if (bs->cur_ps.weaponstate == WEAPON_RAISING || bs->cur_ps.weaponstate == WEAPON_DROPPING) {
		trap_EA_SelectWeapon(bs->client, bs->weaponnum);
	} else {
		newweaponnum = trap_BotChooseBestFightWeapon(bs->ws, bs->inventory);

		if (bs->weaponnum != newweaponnum) {
			bs->weaponchange_time = FloatTime();
		}

		bs->weaponnum = newweaponnum;
		//BotAI_Print(PRT_MESSAGE, "bs->weaponnum = %d\n", bs->weaponnum);
		trap_EA_SelectWeapon(bs->client, bs->weaponnum);
	}
}

/*
=======================================================================================================================================
BotWantsToWalk
=======================================================================================================================================
*/
qboolean BotWantsToWalk(bot_state_t *bs) {

	if (bot_nowalk.integer) {
		return qfalse;
	}

	if (bs->walker < 0.1f) {
		return qfalse;
	}

	switch (gametype) {
		case GT_SINGLE_PLAYER:
			break;
		case GT_FFA:
			break;
		case GT_TOURNAMENT:
			break;
		case GT_TEAM:
			break;
		case GT_CTF:
			// never walk if carrying a flag
			if (BotCTFCarryingFlag(bs)) {
				return qfalse;
			}

			break;
		case GT_1FCTF:
			// never walk if carrying the flag
			if (Bot1FCTFCarryingFlag(bs)) {
				return qfalse;
			}

			break;
		case GT_OBELISK:
			break;
		case GT_HARVESTER:
			// never walk if carrying cubes
			if (BotHarvesterCarryingCubes(bs)) {
				return qfalse;
			}

			break;
		default:
			break;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotSetupForMovement
=======================================================================================================================================
*/
void BotSetupForMovement(bot_state_t *bs) {
	bot_initmove_t initmove;

	memset(&initmove, 0, sizeof(bot_initmove_t));

	initmove.entitynum = bs->entitynum;
	initmove.client = bs->client;
	initmove.thinktime = bs->thinktime;
	// set presence type
	if (bs->cur_ps.pm_flags & PMF_DUCKED) {
		initmove.presencetype = PRESENCE_CROUCH;
	} else {
		initmove.presencetype = PRESENCE_NORMAL;
	}
	// set the onground flag
	if (bs->cur_ps.groundEntityNum != ENTITYNUM_NONE) {
		initmove.or_moveflags |= MFL_ONGROUND;
	}
	// set the walk flag
	if (BotWantsToWalk(bs)) {
		initmove.or_moveflags |= MFL_WALK;
	}
	// set the waterjump flag
	if ((bs->cur_ps.pm_flags & PMF_TIME_WATERJUMP) && (bs->cur_ps.pm_time > 0)) {
		initmove.or_moveflags |= MFL_WATERJUMP;
	}
	// set the scout flag
	if (BotHasScout(bs)) {
		initmove.or_moveflags |= MFL_SCOUT;
	}
	// set the teleported flag
	if ((bs->cur_ps.pm_flags & PMF_TIME_KNOCKBACK) && (bs->cur_ps.pm_time > 0)) {
		initmove.or_moveflags |= MFL_TELEPORTED;
	}

	VectorCopy(bs->cur_ps.origin, initmove.origin);
	VectorCopy(bs->cur_ps.velocity, initmove.velocity);
	VectorCopy(bs->viewangles, initmove.viewangles);

	trap_BotInitMoveState(bs->ms, &initmove);
}

/*
=======================================================================================================================================
BotCheckItemPickup
=======================================================================================================================================
*/
void BotCheckItemPickup(bot_state_t *bs, int *oldinventory) {
	int offence, leader;

	if (gametype < GT_CTF) {
		return;
	}

	offence = -1;
	// go into offence if picked up the kamikaze
	if (!oldinventory[INVENTORY_KAMIKAZE] && bs->inventory[INVENTORY_KAMIKAZE] >= 1) {
		offence = qtrue;
	}
	// if not already wearing the kamikaze
	if (!bs->inventory[INVENTORY_KAMIKAZE]) {
		if (!oldinventory[INVENTORY_SCOUT] && bs->inventory[INVENTORY_SCOUT] >= 1) {
			offence = qtrue;
		}

		if (!oldinventory[INVENTORY_GUARD] && bs->inventory[INVENTORY_GUARD] >= 1) {
			offence = qtrue;
		}

		if (!oldinventory[INVENTORY_DOUBLER] && bs->inventory[INVENTORY_DOUBLER] >= 1) {
			offence = qfalse;
		}

		if (!oldinventory[INVENTORY_AMMOREGEN] && bs->inventory[INVENTORY_AMMOREGEN] >= 1) {
			offence = qfalse;
		}
	}

	if (offence >= 0) {
		leader = ClientFromName(bs->teamleader);

		if (offence) {
			if (!(bs->teamtaskpreference & TEAMTP_ATTACKER)) {
				// if we have a bot team leader
				if (BotTeamLeader(bs)) {
					// tell the leader we want to be on offence
					BotVoiceChat(bs, leader, VOICECHAT_WANTONOFFENSE);
					//BotAI_BotInitialChat(bs, "wantoffence", NULL);
					//trap_BotEnterChat(bs->cs, leader, CHAT_TELL);
				} else if (g_spSkill.integer <= 3) {
					if (bs->ltgtype != LTG_GETFLAG && bs->ltgtype != LTG_ATTACKENEMYBASE && bs->ltgtype != LTG_HARVEST) {
						if ((gametype != GT_CTF || (bs->redflagstatus == 0 && bs->blueflagstatus == 0)) && (gametype != GT_1FCTF || bs->neutralflagstatus == 0)) {
							// tell the leader we want to be on offence
							BotVoiceChat(bs, leader, VOICECHAT_WANTONOFFENSE);
							//BotAI_BotInitialChat(bs, "wantoffence", NULL);
							//trap_BotEnterChat(bs->cs, leader, CHAT_TELL);
						}
					}
				}

				bs->teamtaskpreference |= TEAMTP_ATTACKER;
			}

			bs->teamtaskpreference &= ~TEAMTP_DEFENDER;
		} else {
			if (!(bs->teamtaskpreference & TEAMTP_DEFENDER)) {
				// if we have a bot team leader
				if (BotTeamLeader(bs)) {
					// tell the leader we want to be on defense
					BotVoiceChat(bs, -1, VOICECHAT_WANTONDEFENSE);
					//BotAI_BotInitialChat(bs, "wantdefence", NULL);
					//trap_BotEnterChat(bs->cs, leader, CHAT_TELL);
				} else if (g_spSkill.integer <= 3) {
					if (bs->ltgtype != LTG_DEFENDKEYAREA) {
						if ((gametype != GT_CTF || (bs->redflagstatus == 0 && bs->blueflagstatus == 0)) && (gametype != GT_1FCTF || bs->neutralflagstatus == 0)) {
							// tell the leader we want to be on defense
							BotVoiceChat(bs, -1, VOICECHAT_WANTONDEFENSE);
							//BotAI_BotInitialChat(bs, "wantdefence", NULL);
							//trap_BotEnterChat(bs->cs, leader, CHAT_TELL);
						}
					}
				}

				bs->teamtaskpreference |= TEAMTP_DEFENDER;
			}

			bs->teamtaskpreference &= ~TEAMTP_ATTACKER;
		}
	}
}

/*
=======================================================================================================================================
BotUpdateInventory
=======================================================================================================================================
*/
void BotUpdateInventory(bot_state_t *bs) {
	int oldinventory[MAX_ITEMS];

	memcpy(oldinventory, bs->inventory, sizeof(oldinventory));
	// health
	bs->inventory[INVENTORY_HEALTH] = bs->cur_ps.stats[STAT_HEALTH];
	// armor
	bs->inventory[INVENTORY_ARMOR] = bs->cur_ps.stats[STAT_ARMOR];
	// weapons
	bs->inventory[INVENTORY_GAUNTLET] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_GAUNTLET)) != 0;
	bs->inventory[INVENTORY_MACHINEGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_MACHINEGUN)) != 0;
	bs->inventory[INVENTORY_CHAINGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_CHAINGUN)) != 0;
	bs->inventory[INVENTORY_SHOTGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_SHOTGUN)) != 0;
	bs->inventory[INVENTORY_NAILGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_NAILGUN)) != 0;
	bs->inventory[INVENTORY_PROXLAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_PROXLAUNCHER)) != 0;
	bs->inventory[INVENTORY_GRENADELAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_GRENADELAUNCHER)) != 0;
	bs->inventory[INVENTORY_NAPALMLAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_NAPALMLAUNCHER)) != 0;
	bs->inventory[INVENTORY_ROCKETLAUNCHER] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_ROCKETLAUNCHER)) != 0;
	bs->inventory[INVENTORY_BEAMGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_BEAMGUN)) != 0;
	bs->inventory[INVENTORY_RAILGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_RAILGUN)) != 0;
	bs->inventory[INVENTORY_PLASMAGUN] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_PLASMAGUN)) != 0;
	bs->inventory[INVENTORY_BFG10K] = (bs->cur_ps.stats[STAT_WEAPONS] & (1 << WP_BFG)) != 0;
	// ammo
	bs->inventory[INVENTORY_BULLETS] = bs->cur_ps.ammo[WP_MACHINEGUN];
	bs->inventory[INVENTORY_BELT] = bs->cur_ps.ammo[WP_CHAINGUN];
	bs->inventory[INVENTORY_SHELLS] = bs->cur_ps.ammo[WP_SHOTGUN];
	bs->inventory[INVENTORY_NAILS] = bs->cur_ps.ammo[WP_NAILGUN];
	bs->inventory[INVENTORY_MINES] = bs->cur_ps.ammo[WP_PROXLAUNCHER];
	bs->inventory[INVENTORY_GRENADES] = bs->cur_ps.ammo[WP_GRENADELAUNCHER];
	bs->inventory[INVENTORY_CANISTERS] = bs->cur_ps.ammo[WP_NAPALMLAUNCHER];
	bs->inventory[INVENTORY_ROCKETS] = bs->cur_ps.ammo[WP_ROCKETLAUNCHER];
	bs->inventory[INVENTORY_BEAMGUN_AMMO] = bs->cur_ps.ammo[WP_BEAMGUN];
	bs->inventory[INVENTORY_SLUGS] = bs->cur_ps.ammo[WP_RAILGUN];
	bs->inventory[INVENTORY_CELLS] = bs->cur_ps.ammo[WP_PLASMAGUN];
	bs->inventory[INVENTORY_BFG_AMMO] = bs->cur_ps.ammo[WP_BFG];
	// holdables
	bs->inventory[INVENTORY_MEDKIT] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_MEDKIT;
	bs->inventory[INVENTORY_KAMIKAZE] = bs->cur_ps.stats[STAT_HOLDABLE_ITEM] == MODELINDEX_KAMIKAZE;
	// powerups
	bs->inventory[INVENTORY_QUAD] = bs->cur_ps.powerups[PW_QUAD] != 0;
	bs->inventory[INVENTORY_INVISIBILITY] = bs->cur_ps.powerups[PW_INVIS] != 0;
	bs->inventory[INVENTORY_REGEN] = bs->cur_ps.powerups[PW_REGEN] != 0;
	// persistant powerups
	bs->inventory[INVENTORY_AMMOREGEN] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_AMMOREGEN;
	bs->inventory[INVENTORY_GUARD] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_GUARD;
	bs->inventory[INVENTORY_DOUBLER] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_DOUBLER;
	bs->inventory[INVENTORY_SCOUT] = bs->cur_ps.stats[STAT_PERSISTANT_POWERUP] == MODELINDEX_SCOUT;
	// team items
	bs->inventory[INVENTORY_REDFLAG] = bs->cur_ps.powerups[PW_REDFLAG] != 0;
	bs->inventory[INVENTORY_BLUEFLAG] = bs->cur_ps.powerups[PW_BLUEFLAG] != 0;
	bs->inventory[INVENTORY_NEUTRALFLAG] = bs->cur_ps.powerups[PW_NEUTRALFLAG] != 0;

	if (BotTeam(bs) != TEAM_RED) {
		bs->inventory[INVENTORY_REDCUBE] = bs->cur_ps.tokens;
		bs->inventory[INVENTORY_BLUECUBE] = 0;
	} else {
		bs->inventory[INVENTORY_REDCUBE] = 0;
		bs->inventory[INVENTORY_BLUECUBE] = bs->cur_ps.tokens;
	}

	bs->inventory[BOT_IS_IN_HURRY] = BotOnlyPickupImportantItems(bs);

	BotCheckItemPickup(bs, oldinventory);
}

/*
=======================================================================================================================================
BotUpdateBattleInventory
=======================================================================================================================================
*/
void BotUpdateBattleInventory(bot_state_t *bs, int enemy) {
	vec3_t dir;
	aas_entityinfo_t entinfo;

	// get the entity information
	BotEntityInfo(enemy, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return;
	}

	VectorSubtract(entinfo.origin, bs->origin, dir);

	bs->inventory[ENEMY_HEIGHT] = (int)dir[2];
	dir[2] = 0;
	bs->inventory[ENEMY_HORIZONTAL_DIST] = (int)VectorLength(dir);
	bs->inventory[ENTITY_IS_AN_OBELISK] = (int)(bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) ? 1 : 0;
	// FIXME: add num visible enemies and num visible teammates to the inventory
}

/*
=======================================================================================================================================
BotWantsToUseKamikaze
=======================================================================================================================================
*/
qboolean BotWantsToUseKamikaze(bot_state_t *bs) {

	if (gametype == GT_OBELISK) {
		// if the bot is low on health and recently hurt
		if (bs->inventory[INVENTORY_HEALTH] < 60 && g_entities[bs->entitynum].client->lasthurt_time > level.time - 1000) { // Tobias NOTE: exclude falling damage
			return qtrue;
		}
		// if the bot has the ammoregen powerup
		if (bs->inventory[INVENTORY_AMMOREGEN] > 0) {
			return qfalse;
		}
		// if the bot can use the machine gun
		if (bs->inventory[INVENTORY_MACHINEGUN] > 0 && bs->inventory[INVENTORY_BULLETS] > 0) {
			return qfalse;
		}
		// if the bot can use the chain gun
		if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 0) {
			return qfalse;
		}
		// if the bot can use the shot gun
		if (bs->inventory[INVENTORY_SHOTGUN] > 0 && bs->inventory[INVENTORY_SHELLS] > 0) {
			return qfalse;
		}
		// if the bot can use the nail gun
		if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 0) {
			return qfalse;
		}
		// if the bot can place a mine
		if (bs->inventory[INVENTORY_PROXLAUNCHER] > 0 && bs->inventory[INVENTORY_MINES] > 0) {
			return qfalse;
		}
		// if the bot can use the grenade launcher
		if (bs->inventory[INVENTORY_GRENADELAUNCHER] > 0 && bs->inventory[INVENTORY_GRENADES] > 0) {
			return qfalse;
		}
		// if the bot can use the napalm launcher
		if (bs->inventory[INVENTORY_NAPALMLAUNCHER] > 0 && bs->inventory[INVENTORY_CANISTERS] > 0) {
			return qfalse;
		}
		// if the bot can use the rocket launcher
		if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 0) {
			return qfalse;
		}
		// if the bot can use the beam gun
		if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 0) {
			return qfalse;
		}
		// if the bot can use the rail gun
		if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 0) {
			return qfalse;
		}
		// if the bot can use the plasma gun
		if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 0) {
			return qfalse;
		}
		// if the bot can use the bfg
		if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 0) {
			return qfalse;
		}
	} else {
		// if the bot is low on health and recently hurt
		if (bs->inventory[INVENTORY_HEALTH] < 80 && g_entities[bs->entitynum].client->lasthurt_time > level.time - 1000) { // Tobias NOTE: exclude falling damage
			return qtrue;
		}
	}

	return qfalse;
}

#define KAMIKAZE_DIST 1024
/*
=======================================================================================================================================
BotUseKamikaze
=======================================================================================================================================
*/
void BotUseKamikaze(bot_state_t *bs) {
	int c, teammates, enemies;
	aas_entityinfo_t entinfo;
	vec3_t dir, target;
	bot_goal_t *goal;
	bsp_trace_t trace;

	// if the bot has no kamikaze
	if (bs->inventory[INVENTORY_KAMIKAZE] <= 0) {
		return;
	}

	if (bs->kamikaze_time > FloatTime()) {
		return;
	}

	bs->kamikaze_time = FloatTime() + 0.2;

	switch (gametype) {
		case GT_FFA:
			BotCountVisibleEnemies(bs, &enemies, KAMIKAZE_DIST);

			if (enemies > 0 && BotWantsToUseKamikaze(bs)) {
				trap_EA_Use(bs->client);
				return;
			}

			break;
		case GT_TEAM:
			BotCountVisibleTeamMatesAndEnemies(bs, &teammates, &enemies, KAMIKAZE_DIST);

			if (enemies > 2 && enemies > teammates + 1 && BotWantsToUseKamikaze(bs)) {
				trap_EA_Use(bs->client);
				return;
			}

			break;
		case GT_CTF:
			// never use the kamikaze if carrying a flag
			if (BotCTFCarryingFlag(bs)) {
				return;
			}
			// never use the kamikaze if the team flag carrier is visible
			c = BotTeamFlagCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					return;
				}
			}
			// always use the kamikaze if the enemy flag carrier is visible
			c = BotEnemyFlagCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					trap_EA_Use(bs->client);
					return;
				}
			}

			BotCountVisibleTeamMatesAndEnemies(bs, &teammates, &enemies, KAMIKAZE_DIST);

			if (enemies > 2 && enemies > teammates + 1 && BotWantsToUseKamikaze(bs)) {
				trap_EA_Use(bs->client);
				return;
			}

			break;
		case GT_1FCTF:
			// never use the kamikaze if carrying the flag
			if (Bot1FCTFCarryingFlag(bs)) {
				return;
			}
			// never use the kamikaze if the team flag carrier is visible
			c = BotTeamFlagCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					return;
				}
			}
			// always use the kamikaze if the enemy flag carrier is visible
			c = BotEnemyFlagCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					trap_EA_Use(bs->client);
					return;
				}
			}

			BotCountVisibleTeamMatesAndEnemies(bs, &teammates, &enemies, KAMIKAZE_DIST);

			if (enemies > 2 && enemies > teammates + 1 && BotWantsToUseKamikaze(bs)) {
				trap_EA_Use(bs->client);
				return;
			}

			break;
		case GT_OBELISK:
			switch (BotTeam(bs)) {
				case TEAM_RED:
					goal = &blueobelisk;
					break;
				default:
					goal = &redobelisk;
					break;
			}
			// if the obelisk is visible
			VectorCopy(goal->origin, target);

			target[2] += 1;

			VectorSubtract(target, bs->origin, dir);
			// don't use the kamikaze as long as it isn't really needed
			if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST * 0.7) && BotWantsToUseKamikaze(bs)) {
				BotAI_Trace(&trace, bs->eye, NULL, NULL, target, bs->client, CONTENTS_SOLID);
				// if a full trace or the hitent is hit
				if (trace.fraction >= 1.0f || trace.entityNum == goal->entitynum) {
					trap_EA_Use(bs->client);
					return;
				}
			}

			break;
		case GT_HARVESTER:
			// never use the kamikaze if carrying cubes
			if (BotHarvesterCarryingCubes(bs)) {
				return;
			}
			// never use the kamikaze if a teammate carrying cubes is visible
			c = BotTeamCubeCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					return;
				}
			}
			// always use the kamikaze if an enemy carrying cubes is visible
			c = BotEnemyCubeCarrierVisible(bs);

			if (c >= 0) {
				// get the entity information
				BotEntityInfo(c, &entinfo);
				VectorSubtract(entinfo.origin, bs->origin, dir);

				if (VectorLengthSquared(dir) < Square(KAMIKAZE_DIST)) {
					trap_EA_Use(bs->client);
					return;
				}
			}

			BotCountVisibleTeamMatesAndEnemies(bs, &teammates, &enemies, KAMIKAZE_DIST);

			if (enemies > 2 && enemies > teammates + 1 && BotWantsToUseKamikaze(bs)) {
				trap_EA_Use(bs->client);
				return;
			}

			break;
		default:
			break;
	}
}

/*
=======================================================================================================================================
BotBattleUseItems
=======================================================================================================================================
*/
void BotBattleUseItems(bot_state_t *bs) {

	if (bs->inventory[INVENTORY_HEALTH] < 60) {
		if (bs->inventory[INVENTORY_MEDKIT] > 0) {
			trap_EA_Use(bs->client);
		}
	}

	BotUseKamikaze(bs);
}

/*
=======================================================================================================================================
BotSetTeleportTime
=======================================================================================================================================
*/
void BotSetTeleportTime(bot_state_t *bs) {

	if ((bs->cur_ps.eFlags ^ bs->last_eFlags) & EF_TELEPORT_BIT) {
		bs->teleport_time = FloatTime();
	}

	bs->last_eFlags = bs->cur_ps.eFlags;
}

/*
=======================================================================================================================================
BotIsDead
=======================================================================================================================================
*/
qboolean BotIsDead(bot_state_t *bs) {
	return (bs->cur_ps.pm_type == PM_DEAD);
}

/*
=======================================================================================================================================
BotIsObserver
=======================================================================================================================================
*/
qboolean BotIsObserver(bot_state_t *bs) {
	char buf[MAX_INFO_STRING];

	if (bs->cur_ps.pm_type == PM_SPECTATOR) {
		return qtrue;
	}

	trap_GetConfigstring(CS_PLAYERS + bs->client, buf, sizeof(buf));

	if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotIntermission
=======================================================================================================================================
*/
qboolean BotIntermission(bot_state_t *bs) {

	// NOTE: we shouldn't be looking at the game code...
	if (level.intermissiontime) {
		return qtrue;
	}

	return (bs->cur_ps.pm_type == PM_FREEZE || bs->cur_ps.pm_type == PM_INTERMISSION);
}

/*
=======================================================================================================================================
BotInLavaOrSlime
=======================================================================================================================================
*/
qboolean BotInLavaOrSlime(bot_state_t *bs) {
	vec3_t feet;

	VectorCopy(bs->origin, feet);

	feet[2] -= 23;
	return (trap_AAS_PointContents(feet) & (CONTENTS_LAVA|CONTENTS_SLIME));
}

/*
=======================================================================================================================================
BotCreateWayPoint
=======================================================================================================================================
*/
bot_waypoint_t *BotCreateWayPoint(char *name, vec3_t origin, int areanum) {
	bot_waypoint_t *wp;
	vec3_t waypointmins = {-8, -8, -8}, waypointmaxs = {8, 8, 8};

	wp = botai_freewaypoints;

	if (!wp) {
		BotAI_Print(PRT_WARNING, "BotCreateWayPoint: Out of waypoints\n");
		return NULL;
	}

	botai_freewaypoints = botai_freewaypoints->next;

	Q_strncpyz(wp->name, name, sizeof(wp->name));

	VectorCopy(origin, wp->goal.origin);
	VectorCopy(waypointmins, wp->goal.mins);
	VectorCopy(waypointmaxs, wp->goal.maxs);

	wp->goal.areanum = areanum;
	wp->next = NULL;
	wp->prev = NULL;
	return wp;
}

/*
=======================================================================================================================================
BotFindWayPoint
=======================================================================================================================================
*/
bot_waypoint_t *BotFindWayPoint(bot_waypoint_t *waypoints, char *name) {
	bot_waypoint_t *wp;

	for (wp = waypoints; wp; wp = wp->next) {
		if (!Q_stricmp(wp->name, name)) {
			return wp;
		}
	}

	return NULL;
}

/*
=======================================================================================================================================
BotFreeWaypoints
=======================================================================================================================================
*/
void BotFreeWaypoints(bot_waypoint_t *wp) {
	bot_waypoint_t *nextwp;

	for (; wp; wp = nextwp) {
		nextwp = wp->next;
		wp->next = botai_freewaypoints;
		botai_freewaypoints = wp;
	}
}

/*
=======================================================================================================================================
BotInitWaypoints
=======================================================================================================================================
*/
void BotInitWaypoints(void) {
	int i;

	botai_freewaypoints = NULL;

	for (i = 0; i < MAX_WAYPOINTS; i++) {
		botai_waypoints[i].next = botai_freewaypoints;
		botai_freewaypoints = &botai_waypoints[i];
	}
}

/*
=======================================================================================================================================
TeamPlayIsOn
=======================================================================================================================================
*/
int TeamPlayIsOn(void) {
	return (gametype > GT_TOURNAMENT);
}

/*
=======================================================================================================================================
BotCanCamp
=======================================================================================================================================
*/
qboolean BotCanCamp(bot_state_t *bs) {

	// if the bot's team does not lead
	if (gametype > GT_TOURNAMENT && bs->ownteamscore < bs->enemyteamscore) {
		return qfalse;
	}
	// if the enemy is located way higher than the bot
	if (bs->inventory[ENEMY_HEIGHT] > 200) {
		return qfalse;
	}
	// if the bot is very low on health
	if (bs->inventory[INVENTORY_HEALTH] < 60) {
		return qfalse;
	}
	// if the bot is low on health
	if (bs->inventory[INVENTORY_HEALTH] < 80) {
		// if the bot has insufficient armor
		if (bs->inventory[INVENTORY_ARMOR] < 40) {
			return qfalse;
		}
	}
	// if the bot has the quad powerup
	if (bs->inventory[INVENTORY_QUAD]) {
		return qfalse;
	}
	// if the bot has the invisibility powerup
	if (bs->inventory[INVENTORY_INVISIBILITY]) {
		return qfalse;
	}
	// if the bot has the regen powerup
	if (bs->inventory[INVENTORY_REGEN]) {
		return qfalse;
	}
	// the bot should have at least have a good weapon with some ammo
	if (!(bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 80)
		&& !(bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 10)
		&& !(bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 10)
		&& !(bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 80)
		&& !(bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 10)) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotAggression

qfalse -> bots want to retreat (not affected if the bot is carrying a flag or the enemy is carrying a flag).
qtrue  -> bots want to chase (not affected if the bot is carrying a flag or the enemy is carrying a flag).
qtrue  -> bots can decide what to do.
=======================================================================================================================================
*/
const int BotAggression(bot_state_t *bs) {
	float aggression, selfpreservation;
	int value;
	aas_entityinfo_t entinfo;
	vec3_t dir, angles;
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	aggression = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AGGRESSION, 0, 1);
	selfpreservation = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_SELFPRESERVATION, 0, 1);

	if (!bot_alt_aggressive.integer) {
		// if the enemy is located way higher than the bot
		if (bs->inventory[ENEMY_HEIGHT] > 200) {
			return 0;
		}
		// if the bot is using the napalmlauncher
		if (bs->weaponnum == WP_NAPALMLAUNCHER) {
			return 0;
		}
		// if the bot is using the grenadelauncher
		if (bs->weaponnum == WP_GRENADELAUNCHER) {
			return 0;
		}
		// if the bot is using the proxylauncher
		if (bs->weaponnum == WP_PROXLAUNCHER) {
			return 0;
		}
		// current enemy
		if (bs->enemy >= 0) {
			// get the entity information
			BotEntityInfo(bs->enemy, &entinfo);
			// if the entity information is valid
			if (entinfo.valid) {
				// if the bot is using the gauntlet
				if (bs->weaponnum == WP_GAUNTLET) {
					// if attacking an obelisk
					if (bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) {
						return 0;
					}
					// if the enemy is located higher than the bot can jump on
					if (bs->inventory[ENEMY_HEIGHT] > 42) {
						return 0;
					}
					// an extremely aggressive bot will less likely retreat
					if (aggression > 0.9) {
						return 100;
					}
					// if the enemy is really near
					if (bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
						return 100;
					}
					// if the enemy is far away, check if we can attack the enemy from behind
					if (aggression > 0.5 && bs->inventory[ENEMY_HORIZONTAL_DIST] < 500) {
						VectorSubtract(bs->origin, entinfo.origin, dir);
						VectorToAngles(dir, angles);
						// if not in the enemy's field of vision, attack!
						if (!InFieldOfVision(entinfo.angles, 180, angles)) {
							return 100;
						}
					}
					// if currently using the gauntlet, retreat!
					return 0;
				}
				// if the enemy is using the gauntlet
				if (entinfo.weapon == WP_GAUNTLET) {
					return 100;
				}
				// an extremely aggressive bot will less likely retreat
				if (aggression > 0.9 && bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
					return 100;
				}
				// if the enemy is using the bfg
				if (entinfo.weapon == WP_BFG) {
					return 0;
				}
				// if the enemy is using the napalmlauncher
				if (entinfo.weapon == WP_NAPALMLAUNCHER) {
					return 0;
				}
				// if the enemy is using the grenadelauncher
				if (entinfo.weapon == WP_GRENADELAUNCHER) {
					return 0;
				}
				// if the enemy is using the proxylauncher
				if (entinfo.weapon == WP_PROXLAUNCHER) {
					return 0;
				}
				// if the enemy has the quad damage
				if (entinfo.powerups & (1 << PW_QUAD)) {
					return 0;
				}
				// if the bot is out for revenge
				if (bs->enemy == bs->revenge_enemy && bs->revenge_kills > 0) {
					return 100;
				}
			}
		}
		// if the bot has the quad damage powerup
		if (bs->inventory[INVENTORY_QUAD]) {
			return 100;
		}
		// if the bot has the invisibility powerup
		if (bs->inventory[INVENTORY_INVISIBILITY]) {
			return 100;
		}
		// if the bot has the regeneration powerup
		if (bs->inventory[INVENTORY_REGEN]) {
			return 100;
		}
		// if the bot has the guard powerup
		if (bs->inventory[INVENTORY_GUARD]) {
			return 100;
		}
		// if the bot is very low on health.
		if (bs->inventory[INVENTORY_HEALTH] < 100 * selfpreservation) {
			return 0;
		}
		// if the bot is low on health.
		if (bs->inventory[INVENTORY_HEALTH] < 100 * selfpreservation + 20) {
			// if the bot has insufficient armor
			if (bs->inventory[INVENTORY_ARMOR] < 40) {
				return 0;
			}
		}
		// if the bot has the ammoregen powerup
		if (bs->inventory[INVENTORY_AMMOREGEN]) {
			return 100;
		}
		// if the bot has the doubler powerup
		if (bs->inventory[INVENTORY_DOUBLER]) {
			return 100;
		}
		// if the bot has the scout powerup
		if (bs->inventory[INVENTORY_SCOUT]) {
			return 100;
		}
		// if the bot can use the machine gun
		if (bs->inventory[INVENTORY_MACHINEGUN] > 0 && bs->inventory[INVENTORY_BULLETS] > 40) {
			return 100;
		}
		// if the bot can use the chain gun
		if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 50) {
			return 100;
		}
		// if the bot can use the shot gun
		if (bs->inventory[INVENTORY_SHOTGUN] > 0 && bs->inventory[INVENTORY_SHELLS] > 5) {
			return 100;
		}
		// if the bot can use the nail gun
		if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 5) {
			return 100;
		}
		// if the bot can use the rocket launcher
		if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 5) {
			return 100;
		}
		// if the bot can use the beam gun
		if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 50) {
			return 100;
		}
		// if the bot can use the railgun
		if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 3) {
			return 100;
		}
		// if the bot can use the plasma gun
		if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 40) {
			return 100;
		}
		// if the bot can use the bfg
		if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 5) {
			return 100;
		}
		// otherwise the bot is not feeling too good
		return 0;
	} else {
		value = 0;
		// if the enemy is located way higher than the bot
		if (bs->inventory[ENEMY_HEIGHT] > 200) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy higher than 200 (%i).\n", netname, value);
#endif
			return 0;
		}
		// if the bot is using the napalmlauncher
		if (bs->weaponnum == WP_NAPALMLAUNCHER) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Using Napalmlauncher (%i).\n", netname, value);
#endif
			return 0;
		}
		// if the bot is using the grenadelauncher
		if (bs->weaponnum == WP_GRENADELAUNCHER) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Using Grenadelauncher (%i).\n", netname, value);
#endif
			return 0;
		}
		// if the bot is using the proxylauncher
		if (bs->weaponnum == WP_PROXLAUNCHER) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Using Proxylauncher (%i).\n", netname, value);
#endif
			return 0;
		}
		// current enemy
		if (bs->enemy >= 0) {
			// get the entity information
			BotEntityInfo(bs->enemy, &entinfo);
			// if the entity information is valid
			if (entinfo.valid) {
				// if the bot is using the gauntlet
				if (bs->weaponnum == WP_GAUNTLET) {
					// if attacking an obelisk
					if (bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) {
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Using Gauntlet enemy Obelisk (%i).\n", netname, value);
#endif
						return 0;
					}
					// if the enemy is located higher than the bot can jump on
					if (bs->inventory[ENEMY_HEIGHT] > 42) {
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Using Gauntlet enemy higher 42 (%i).\n", netname, value);
#endif
						return 0;
					}
					// an extremely aggressive bot will less likely retreat
					if (aggression > 0.9) {
						value += 50;
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Using Gauntlet char aggression +50 (%i).\n", netname, value);
#endif
					}
					// if the enemy is really near
					if (bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
						value += 50;
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Using Gauntlet dist < 200 +50 (%i).\n", netname, value);
#endif
					}
					// if the enemy is far away, check if we can attack the enemy from behind
					if (aggression > 0.5 && bs->inventory[ENEMY_HORIZONTAL_DIST] < 500) {
						VectorSubtract(bs->origin, entinfo.origin, dir);
						VectorToAngles(dir, angles);
						// if not in the enemy's field of vision, attack!
						if (!InFieldOfVision(entinfo.angles, 180, angles)) {
							value += 50;
#ifdef DEBUG
							BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Using Gauntlet attacking from behind +50 (%i).\n", netname, value);
#endif
						}
					}
				}
				// if the enemy is using the gauntlet
				if (entinfo.weapon == WP_GAUNTLET) {
					value += 50;
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy Gauntlet +50 (%i).\n", netname, value);
#endif
				}
				// an extremely aggressive bot will less likely retreat
				if (aggression > 0.9 && bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
					value += 50;
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy Gauntlet char aggression +50 (%i).\n", netname, value);
#endif
				}
				// if the enemy is using the bfg
				if (entinfo.weapon == WP_BFG) {
					// if the enemy is really near
					if (bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
						value += 100;
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy BFG < 200 +100 (%i).\n", netname, value);
#endif
					} else {
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy BFG > 200 (%i).\n", netname, value);
#endif
						return 0;
					}
				}
				// if the enemy is using the napalmlauncher
				if (entinfo.weapon == WP_NAPALMLAUNCHER) {
					// if the enemy is really near
					if (bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
						value += 100;
					} else {
						return 0;
					}
				}
				// if the enemy is using the grenadelauncher
				if (entinfo.weapon == WP_GRENADELAUNCHER) {
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy Grenadelauncher (%i).\n", netname, value);
#endif
					return 0;
				}
				// if the enemy is using the proxylauncher
				if (entinfo.weapon == WP_PROXLAUNCHER) {
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy Proxylauncher (%i).\n", netname, value);
#endif
					return 0;
				}
				// if the enemy has the quad damage
				if (entinfo.powerups & (1 << PW_QUAD)) {
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy Quad (%i).\n", netname, value);
#endif
					return 0;
				}
				// if the bot is out for revenge
				if (bs->enemy == bs->revenge_enemy && bs->revenge_kills > 0) {
					value += 20;
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Out for revenge +20 (%i).\n", netname, value);
#endif
				}
			}
		}
		// if the bot has the quad damage powerup
		if (bs->inventory[INVENTORY_QUAD]) {
			value += 70;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Quad +70 (%i).\n", netname, value);
#endif
		}
		// if the bot has the invisibility powerup
		if (bs->inventory[INVENTORY_INVISIBILITY]) {
			value += 70;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Invis +70 (%i).\n", netname, value);
#endif
		}
		// if the bot has the regeneration powerup
		if (bs->inventory[INVENTORY_REGEN]) {
			value += bs->inventory[INVENTORY_HEALTH];
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Regen +Health (%i).\n", netname, value);
#endif
		}
		// if the bot has the guard powerup
		if (bs->inventory[INVENTORY_GUARD]) {
			value += bs->inventory[INVENTORY_HEALTH];
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Guard +Health (%i).\n", netname, value);
#endif
		}
		// if the bot is very low on health
		if (bs->inventory[INVENTORY_HEALTH] < 100 * selfpreservation) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Low Health (%i).\n", netname, value);
#endif
			return 0;
		}
		// if the bot is low on health
		if (bs->inventory[INVENTORY_HEALTH] < 100 * selfpreservation + 20) {
			// if the bot has insufficient armor
			if (bs->inventory[INVENTORY_ARMOR] < 40) {
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Low Health and Armor (%i).\n", netname, value);
#endif
				return 0;
			}
		}
		// if the bot has the ammoregen powerup
		if (bs->inventory[INVENTORY_AMMOREGEN]) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Ammoregen +50 (%i).\n", netname, value);
#endif
		}
		// if the bot has the doubler powerup
		if (bs->inventory[INVENTORY_DOUBLER]) {
			value += 100;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Doubler +100 (%i).\n", netname, value);
#endif
		}
		// if the bot has the scout powerup
		if (bs->inventory[INVENTORY_SCOUT]) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Scout +50 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the machine gun
		if (bs->inventory[INVENTORY_MACHINEGUN] > 0 && bs->inventory[INVENTORY_BULLETS] > 40) {
			value += 30;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Machinegun +30 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the chain gun
		if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 50) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Chaingun +50 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the shot gun
		if (bs->inventory[INVENTORY_SHOTGUN] > 0 && bs->inventory[INVENTORY_SHELLS] > 5) {
			value += 30;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Shotgun +30 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the nail gun
		if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 5) {
			value += 30;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Nailgun +30 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the rocket launcher
		if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 5) {
			value += 30;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Rocketlauncher +30 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the beam gun
		if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 50) {
			value += 30;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Beamgun +30 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the railgun
		if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 3) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Railgun +50 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the plasma gun
		if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 40) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Plasmagun +50 (%i).\n", netname, value);
#endif
		}
		// if the bot can use the bfg
		if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 5) {
			value += 50;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: BFG +50 (%i).\n", netname, value);
#endif
		}
		// otherwise the bot is not feeling too good
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: AGGRESSION VALUE = %i.\n", netname, value);
#endif
		return (int)value;
	}
}

/*
=======================================================================================================================================
BotFeelingBad
=======================================================================================================================================
*/
float BotFeelingBad(bot_state_t *bs) {

	if (bs->weaponnum == WP_GAUNTLET) {
		return 100;
	}

	if (bs->inventory[INVENTORY_HEALTH] < 40) {
		return 100;
	}

	if (bs->weaponnum == WP_MACHINEGUN) {
		return 90;
	}

	if (bs->inventory[INVENTORY_HEALTH] < 60) {
		return 80;
	}

	return 0;
}

/*
=======================================================================================================================================
BotTeammateNeedsNBG

NOTE: This function takes information into account a human player can't derive from his display. I justify this by assuming that an
endangered player screams for help and tells the needed information.
=======================================================================================================================================
*/
static qboolean BotTeammateNeedsNBG(int clientNum, const playerState_t *ps) {

	// a teammate carrying a flag should be preferred
	if (ps->powerups[PW_REDFLAG] || ps->powerups[PW_BLUEFLAG] || ps->powerups[PW_NEUTRALFLAG]) { // Tobias FIXME: add skulls!
		return qtrue;
	}
	// if the teammate has the chaingun with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_CHAINGUN)) && ps->ammo[WP_CHAINGUN] >= 50) {
		return qfalse;
	}
	// if the teammate has the nailgun with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_NAILGUN)) && ps->ammo[WP_NAILGUN] >= 5) {
		return qfalse;
	}
	// if the teammate has the rocketlauncher with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_ROCKETLAUNCHER)) && ps->ammo[WP_ROCKETLAUNCHER] >= 5) {
		return qfalse;
	}
	// if the teammate has the railgun with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_RAILGUN)) && ps->ammo[WP_RAILGUN] >= 5) {
		return qfalse;
	}
	// if the teammate has the plasmagun with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_PLASMAGUN)) && ps->ammo[WP_PLASMAGUN] >= 15) {
		return qfalse;
	}
	// if the teammate has the bfg with some ammo
	if ((ps->stats[STAT_WEAPONS] & (1 << WP_BFG)) && ps->ammo[WP_BFG] >= 5) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotAvoidItemPickup

The bot leaves the item to someone else.
=======================================================================================================================================
*/
static qboolean BotAvoidItemPickup(bot_state_t *bs, bot_goal_t *goal) {
	float obtrusiveness;
	int i;
	gentity_t *ent;
	playerState_t ps;
	vec3_t dir, angles, v1;
	aas_entityinfo_t entinfo;
// Tobias DEBUG
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	if (!bot_teamavoidpickup.integer) {
		return qfalse;
	}
// Tobias END
	if (gametype < GT_TEAM) {
		return qfalse;
	}
	// always pick up items if using the gauntlet
	if (bs->weaponnum == WP_GAUNTLET) {
		return qfalse;
	}
	// always pick up items if carrying a flag or skulls
	if (BotCTFCarryingFlag(bs) || Bot1FCTFCarryingFlag(bs) || BotHarvesterCarryingCubes(bs)) {
		return qfalse;
	}
	// always pick up team items
	if (g_entities[goal->entitynum].item->giType == IT_TEAM) {
		//BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Picking up a team goal!\n", netname);
		return qfalse;
	}

	obtrusiveness = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_OBTRUSIVENESS, 0, 1);

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}

		if (!BotAI_GetClientState(i, &ps)) {
			continue;
		}
		// if on the same team
		if (!BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if the entity isn't dead
		if (EntityIsDead(&entinfo)) {
			continue;
		}

		ent = &g_entities[i];
		// ignore non-moving teammates
		if (VectorLengthSquared(ent->client->ps.velocity) <= 0) {
			continue;
		}

		VectorSubtract(bs->origin, entinfo.origin, dir);
		VectorToAngles(dir, angles);
		// humans are prefered
		if (!(ent->r.svFlags & SVF_BOT)) {
			// ignore distant teammates
			if (VectorLength(dir) > 650 - (500 * obtrusiveness)) {
				continue;
			}
			// if the bot isn't in the fov of the teammate, ignore the teammate
			if (!InFieldOfVision(entinfo.angles, 180, angles)) {
				continue;
			}
		} else {
			// always pick up items if using the machinegun
			if (bs->weaponnum == WP_MACHINEGUN) { // Tobias NOTE: move this up (outside 'for')
				continue;
			}
			// always pick up items if using the proxylauncher
			if (bs->weaponnum == WP_PROXLAUNCHER) {
				continue;
			}
			// ignore distant teammates
			if (VectorLength(dir) > 500 - (400 * obtrusiveness)) {
				continue;
			}
			// if the bot isn't in the fov of the teammate, ignore the teammate
			if (!InFieldOfVision(entinfo.angles, 120, angles)) {
				continue;
			}
		}
		// always pick up health if the health is lower than the one from the teammate
		if (g_entities[goal->entitynum].item->giType == IT_HEALTH && ent->client->ps.stats[STAT_HEALTH] > bs->inventory[INVENTORY_HEALTH]) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Picking up Health. Own health: %i, Health of teammate: %i.\n", netname, bs->inventory[INVENTORY_HEALTH], ent->client->ps.stats[STAT_HEALTH]);
			continue;
		}
		// always pick up armor if the armor is lower than the one from the teammate
		if (g_entities[goal->entitynum].item->giType == IT_ARMOR && ent->client->ps.stats[STAT_ARMOR] > bs->inventory[INVENTORY_ARMOR]) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Picking up Armor. Own armor: %i, Armor of teammate: %i.\n", netname, bs->inventory[INVENTORY_ARMOR], ent->client->ps.stats[STAT_ARMOR]);
			continue;
		}
		// always pick up holdable items if the teammate already has one
		if (g_entities[goal->entitynum].item->giType == IT_HOLDABLE && ent->client->ps.stats[STAT_HOLDABLE_ITEM] > 0) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Picking up a holdable item. The teammate already has one.\n", netname);
			continue;
		}
		// always pick up persistant powerups if the teammate already has one
		if (g_entities[goal->entitynum].item->giType == IT_PERSISTANT_POWERUP && ent->client->ps.stats[STAT_PERSISTANT_POWERUP] > 0) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Picking up a persistant powerup. The teammate already has one.\n", netname);
			continue;
		}

		if (!BotEntityVisible(&bs->cur_ps, 90, i)) {
			if (VectorLength(dir) > 200) {
				continue;
			}
		} else {
			VectorNormalize2(ent->client->ps.velocity, v1);

			if (DotProduct(v1, dir) < 0.0) {
				continue;
			}
		}
		// if the teammate is in danger, or needs this item
		if (BotTeammateNeedsNBG(i, &ps)) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Avoid picking up item, a teammate needs this NBG!\n", netname);
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotAIWaiting
=======================================================================================================================================
*/
qboolean BotAIWaiting(bot_state_t *bs, bot_goal_t *goal) {

	// never wait if there is an enemy
	if (bs->enemy >= 0) {
		return qfalse;
	}
	// never wait when the health is decreasing
	if (level.clients[bs->client].lasthurt_time > level.time - 1000) {
		return qfalse;
	}
	// never wait when standing in lava or slime
	if (BotInLavaOrSlime(bs)) {
		return qfalse;
	}
	// never wait if the bot is in water
	if (trap_AAS_PointContents(bs->eye) & CONTENTS_WATER) {
		return qfalse;
	}
	// if the bot is waiting for a teammate to pick up items
	if (BotAvoidItemPickup(bs, goal)) {
		// pop the current goal from the stack
		trap_BotPopGoal(bs->gs); // Tobias NOTE: without this we get an "goal heap overflow" error, why?
		// reset the avoid goals and the avoid reach
		trap_BotResetAvoidGoals(bs->gs); // Tobias NOTE: is this really needed?
		trap_BotResetAvoidReach(bs->ms); // Tobias NOTE: is this really needed?
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotHasEmergencyGoal

The bot is in hurry sometimes, he shouldn't pick up every single item on it's way.

1.DONE: If the bot carries the enmemy CTF flag and the own flag is at base.
1.DONE: If the bot is trying to get the enemy CTF flag and the own flag is NOT at base.
3.DONE: If the bot carries the 1CTF flag.
4.DONE: If the bot carries cubes.

TODO: If the bot is rushing the enemy base and the bot isn't in the fov of an enemy (or no enemy at their home base).
TODO: If the enemy obelsik (GT_OBELISK) is seriously damaged and will explode very likely.
=======================================================================================================================================
*/
static qboolean BotHasEmergencyGoal(bot_state_t *bs) {

	switch (gametype) {
		case GT_CTF:
			// if the bot carries a flag and the own flag is at base
			if (BotCTFCarryingFlag(bs)) {
				if (BotTeam(bs) == TEAM_RED) {
					if (bs->redflagstatus == 0) {
						return qtrue;
					}
				} else {
					if (bs->blueflagstatus == 0) {
						return qtrue;
					}
				}
			}
			// if the bot is trying to get the flag and the own flag is NOT at base
			if (bs->ltgtype == LTG_GETFLAG) {
				if (BotTeam(bs) == TEAM_RED) {
					if (bs->redflagstatus == 1) {
						return qtrue;
					}
				} else {
					if (bs->blueflagstatus == 1) {
						return qtrue;
					}
				}
			}

			break;
		case GT_1FCTF:
			// if the bot carries the flag
			if (Bot1FCTFCarryingFlag(bs)) {
				return qtrue;
			}

			break;
		case GT_OBELISK:
			break;
		case GT_HARVESTER:
			// if the bot carries cubes
			if (BotHarvesterCarryingCubes(bs)) {
				return qtrue;
			}

			break;
		default:
			break;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotNearbyGoalPickupRange_NoLTG

Used to determine the range for how far the bot should check for picking up items.
Used for AI node 'BATTLE FIGHT' and AI node 'BATTLE CHASE'.
=======================================================================================================================================
*/
const int BotNearbyGoalPickupRange_NoLTG(bot_state_t *bs) {
	float selfpreservation, nbg_multiplier;
	int range;
	aas_entityinfo_t entinfo;
	vec3_t dir, angles;
// Tobias DEBUG
#ifdef DEBUG
	char buf1[MAX_INFO_STRING];
	char action[MAX_MESSAGE_SIZE];
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));

	nbg_multiplier = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_GOAL_MULTIPLIER, 0, 100);

	if (nbg_multiplier == 0.0) {
		range = 150;
		// FIGHT is drawn in RED
		if (bs->ainode == AINode_Battle_Fight) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Range = %i.\n", netname, range);
		// CHASE is drawn in CYAN
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Range = %i.\n", netname, range);
		}

		return range;
	}
#endif
// Tobias END
	selfpreservation = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_SELFPRESERVATION, 0, 1);
	nbg_multiplier = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_GOAL_MULTIPLIER, 0, 100);
	// default range
	range = 100;
	// current enemy
	if (bs->enemy >= 0) {
		// get the entity information
		BotEntityInfo(bs->enemy, &entinfo);
		// if the entity information is valid
		if (entinfo.valid) {
			// if the bot is using the gauntlet
			if (bs->weaponnum == WP_GAUNTLET) {
				// if the enemy is near enough, check if we can attack the enemy from behind
				if (bs->inventory[ENEMY_HORIZONTAL_DIST] < 200) {
					VectorSubtract(bs->origin, entinfo.origin, dir);
					VectorToAngles(dir, angles);
					// if not in the enemy's field of vision, attack!
					if (!InFieldOfVision(entinfo.angles, 180, angles)) {
// Tobias DEBUG
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: BACKSTABBING! Range = %i.\n", netname, range);
#endif
// Tobias END
						return 10;
					}
				}
			}
			// if the bot is out for revenge
			if (bs->enemy == bs->revenge_enemy && bs->revenge_kills > 0) {
				range -= 20;
			}
		}
	}
	// if the bot is using the gauntlet
	if (bs->weaponnum == WP_GAUNTLET) {
		return 300;
	}
	// if the bot has the quad damage powerup
	if (bs->inventory[INVENTORY_QUAD]) {
		range -= 50;
	}
	// if the bot has the invisibility powerup
	if (bs->inventory[INVENTORY_INVISIBILITY]) {
		range -= 50;
	}
	// if the bot has the doubler powerup
	if (bs->inventory[INVENTORY_DOUBLER]) {
		range -= 50;
	}
	// if the bot has the scout powerup
	if (bs->inventory[INVENTORY_SCOUT]) {
		range += 50;
	}
	// if the bot can use the machine gun
	if (bs->inventory[INVENTORY_MACHINEGUN] > 0 && bs->inventory[INVENTORY_BULLETS] > 40) {
		range -= 10;
	}
	// if the bot can use the chain gun
	if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 50) {
		range -= 50;
	}
		// if the bot can use the shot gun
	if (bs->inventory[INVENTORY_SHOTGUN] > 0 && bs->inventory[INVENTORY_SHELLS] > 5) {
		range -= 10;
	}
	// if the bot can use the nail gun
	if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 5) {
		range -= 10;
	}
	// if the bot can use the rocket launcher
	if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 5) {
		range -= 10;
	}
	// if the bot can use the beam gun
	if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 50) {
		range -= 10;
	}
	// if the bot can use the railgun
	if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 3) {
		range -= 50;
	}
	// if the bot can use the plasma gun
	if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 40) {
		range -= 10;
	}
	// if the bot can use the bfg
	if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 5) {
		range -= 10;
	}
	// if the bot is low on health
	if (selfpreservation < 0.9) {
		range -= bs->inventory[INVENTORY_HEALTH];
	}
	// if the bot accompanies someone
	if (bs->ltgtype == LTG_TEAMACCOMPANY && !BotTeamFlagCarrierVisible(bs)) {
		range *= 4;
		// if the teammate isn't visible for quite some time
		if (bs->teammatevisible_time < FloatTime() - 5) {
			range *= 0.25;
		}
	}

	range *= nbg_multiplier;
	// just for safety sake
	if (range < 10) {
		range = 10;
	}
// Tobias DEBUG
#ifdef DEBUG
	// FIGHT is drawn in RED
	if (bs->ainode == AINode_Battle_Fight) {
		if (bot_report.integer) {
			trap_GetConfigstring(CS_BOTINFO + bs->client, buf1, sizeof(buf1));
			Q_strncpyz(action, Info_ValueForKey(buf1, "a"), sizeof(action));

			if (!*buf1) {
				BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Range = %i.\n", netname, range);
			} else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: %s Range = %i.\n", netname, action, range);
			}
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Range = %i.\n", netname, range);
		}
	// CHASE is drawn in CYAN
	} else {
		if (bot_report.integer) {
			trap_GetConfigstring(CS_BOTINFO + bs->client, buf1, sizeof(buf1));
			Q_strncpyz(action, Info_ValueForKey(buf1, "a"), sizeof(action));

			if (!*buf1) {
				BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Range = %i.\n", netname, range);
			} else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: %s Range = %i.\n", netname, action, range);
			}
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Range = %i.\n", netname, range);
		}
	}
#endif
// Tobias END
	return (int)range;
}

/*
=======================================================================================================================================
BotNearbyGoalPickupRange_LTG

Used to determine the range for how far the bot should check for picking up items.
Used for AI node 'SEEK LTG' and AI node 'BATTLE RETREAT'.
=======================================================================================================================================
*/
const int BotNearbyGoalPickupRange_LTG(bot_state_t *bs) {
	float selfpreservation, nbg_multiplier;
	int range;
// Tobias DEBUG
#ifdef DEBUG
	char buf1[MAX_INFO_STRING];
	char action[MAX_MESSAGE_SIZE];
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));

	nbg_multiplier = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_GOAL_MULTIPLIER, 0, 100);

	if (nbg_multiplier == 0.0) {
		range = 150;
		// RETREAT is drawn in YELLOW
		if (bs->ainode == AINode_Battle_Retreat) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Range = %i.\n", netname, range);
		// SEEK LTG is drawn in GREEN
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Range = %i.\n", netname, range);
		}

		return range;
	}
#endif
// Tobias END
	selfpreservation = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_SELFPRESERVATION, 0, 1);
	nbg_multiplier = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_GOAL_MULTIPLIER, 0, 100);
	// Tobias NOTE: this represents the old behaviour (FIXME: remove this?)
	// if the bot is carrying a flag or cubes and should rush to base as fast as possible
	if (BotHasEmergencyGoal(bs)) {
		return 50;
	} else if (bs->ltgtype == LTG_DEFENDKEYAREA) {
		return 400;
	} else {
		range = 200;
	}
	// Tobias END
	// if the bot is using the gauntlet
	if (bs->weaponnum == WP_GAUNTLET) {
		return 300;
	}
	// if the bot has the quad damage powerup
	if (bs->inventory[INVENTORY_QUAD]) {
		range -= 50;
	}
	// if the bot has the invisibility powerup
	if (bs->inventory[INVENTORY_INVISIBILITY]) {
		range -= 50;
	}
	// if the bot has the doubler powerup
	if (bs->inventory[INVENTORY_DOUBLER]) {
		range -= 50;
	}
	// if the bot has the scout powerup
	if (bs->inventory[INVENTORY_SCOUT]) {
		range += 50;
	}
	// if the bot can use the machine gun
	if (bs->inventory[INVENTORY_MACHINEGUN] > 0 && bs->inventory[INVENTORY_BULLETS] > 40) {
		range -= 10;
	}
	// if the bot can use the chain gun
	if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 50) {
		range -= 50;
	}
		// if the bot can use the shot gun
	if (bs->inventory[INVENTORY_SHOTGUN] > 0 && bs->inventory[INVENTORY_SHELLS] > 5) {
		range -= 10;
	}
	// if the bot can use the nail gun
	if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 5) {
		range -= 10;
	}
	// if the bot can use the rocket launcher
	if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 5) {
		range -= 10;
	}
	// if the bot can use the beam gun
	if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 50) {
		range -= 10;
	}
	// if the bot can use the railgun
	if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 3) {
		range -= 50;
	}
	// if the bot can use the plasma gun
	if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 40) {
		range -= 10;
	}
	// if the bot can use the bfg
	if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 5) {
		range -= 10;
	}
	// if the bot is low on health
	if (selfpreservation < 0.9) {
		range -= bs->inventory[INVENTORY_HEALTH];
	}
	// if the bot accompanies someone
	if (bs->ltgtype == LTG_TEAMACCOMPANY && !BotTeamFlagCarrierVisible(bs)) {
		range *= 2;
		// if the teammate isn't visible for quite some time
		if (bs->teammatevisible_time < FloatTime() - 5) {
			range *= 0.5;
		}
	}

	range *= nbg_multiplier;
	// just for safety sake
	if (range < 10) {
		range = 10;
	}
// Tobias DEBUG
#ifdef DEBUG
	// RETREAT is drawn in YELLOW
	if (bs->ainode == AINode_Battle_Retreat) {
		if (bot_report.integer) {
			trap_GetConfigstring(CS_BOTINFO + bs->client, buf1, sizeof(buf1));
			Q_strncpyz(action, Info_ValueForKey(buf1, "a"), sizeof(action));

			if (!*buf1) {
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Range = %i.\n", netname, range);
			} else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: %s Range = %i.\n", netname, action, range);
			}
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Range = %i.\n", netname, range);
		}
	// SEEK LTG is drawn in GREEN
	} else {
		if (bot_report.integer) {
			trap_GetConfigstring(CS_BOTINFO + bs->client, buf1, sizeof(buf1));
			Q_strncpyz(action, Info_ValueForKey(buf1, "a"), sizeof(action));

			if (!*buf1) {
				BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Range = %i.\n", netname, range);
			} else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: %s Range = %i.\n", netname, action, range);
			}
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Range = %i.\n", netname, range);
		}
	}
#endif
// Tobias END
	return (int)range;
}

/*
=======================================================================================================================================
BotOnlyPickupImportantItems

Only pick up items that are really needed (health items if low on health, useful ammo etc.). The goal is to not waste time by picking
up 'useless' items (bots will not collect ammo for weapons they don't own etc). This will attract bots to move faster to their own base
if they are carrying a flag. They will also less likely lose line of sight when chasing an enemy.
In those cases the bot will only pick up items that are useful and he will only pick up those items if they are in a closer range.

1.DONE: If the bot is standing in lava or slime.
2.DONE: If the bot needs air.
3.DONE: If the bot is carrying a flag or cubes and should rush to base as fast as possible (CHECK TODO's).

TODO: If chasing the enemy and view will be lost when going for NBGs.
TODO: Only if healthy enough, and at least some good weapon with ammo (per gametype?).
=======================================================================================================================================
*/
qboolean BotOnlyPickupImportantItems(bot_state_t *bs) {

	// when standing in lava or slime, get out of this hell
	if (BotInLavaOrSlime(bs)) {
		return qtrue;
	}
	// if the bot needs air
	if (trap_AAS_PointContents(bs->eye) & CONTENTS_WATER) {
		if (bs->lastair_time < FloatTime() - 15) {
			return qtrue;
		}
	}
	// if the bot is carrying a flag or cubes and should rush to base as fast as possible
	if (BotHasEmergencyGoal(bs)) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotWantsToRetreat
=======================================================================================================================================
*/
const int BotWantsToRetreat(bot_state_t *bs) {
	aas_entityinfo_t entinfo;

	// retreat when standing in lava or slime
	if (BotInLavaOrSlime(bs)) {
		return qtrue;
	}
	// not enough air, so retreat
	if (trap_AAS_PointContents(bs->eye) & CONTENTS_WATER) {
		if (bs->lastair_time < FloatTime() - 15) {
			return qtrue;
		}
	}

	if (bs->enemy >= 0) {
		// get the entity information
		BotEntityInfo(bs->enemy, &entinfo);
	}

	switch (gametype) {
		case GT_CTF:
			// always retreat when carrying a CTF flag
			if (BotCTFCarryingFlag(bs)) {
				return qtrue;
			}
			// if the enemy is carrying a flag
			if (EntityCarriesFlag(&entinfo)) {
				return qfalse;
			}
			// if the bot is getting the flag
			if (bs->ltgtype == LTG_GETFLAG) {
				return qtrue;
			}

			break;
		case GT_1FCTF:
			// if carrying the flag then always retreat
			if (Bot1FCTFCarryingFlag(bs)) {
				return qtrue;
			}
			// if the enemy is carrying a flag
			if (EntityCarriesFlag(&entinfo)) {
				return qfalse;
			}
			// if the bot is getting the flag
			if (bs->ltgtype == LTG_GETFLAG) {
				return qtrue;
			}

			break;
		case GT_OBELISK:
			// the bots should be dedicated to attacking the enemy obelisk
			if (bs->ltgtype == LTG_ATTACKENEMYBASE) {
				if (BotFeelingBad(bs) > 50) {
					return qtrue;
				}
				// if this enemy is NOT an obelisk
				if (bs->enemy >= MAX_CLIENTS && (bs->enemy != redobelisk.entitynum && bs->enemy != blueobelisk.entitynum)) {
					return qtrue;
				}

				return qfalse;
			}

			break;
		case GT_HARVESTER:
			// if carrying cubes then always retreat
			if (BotHarvesterCarryingCubes(bs)) {
				return qtrue;
			}
			// if the enemy is carrying cubes
			if (EntityCarriesCubes(&entinfo)) {
				return qfalse;
			}

			break;
		default:
			break;
	}

	if (BotAggression(bs) < 50) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotWantsToChase
=======================================================================================================================================
*/
const int BotWantsToChase(bot_state_t *bs) {
	aas_entityinfo_t entinfo;

	// don't chase if in lava or slime
	if (BotInLavaOrSlime(bs)) {
		return qfalse;
	}
	// don't chase if not enough air
	if (trap_AAS_PointContents(bs->eye) & CONTENTS_WATER) {
		if (bs->lastair_time < FloatTime() - 15) {
			return qfalse;
		}
	}

	if (bs->enemy >= 0) {
		// get the entity information
		BotEntityInfo(bs->enemy, &entinfo);
	}

	switch (gametype) {
		case GT_CTF:
			// never chase when carrying a CTF flag
			if (BotCTFCarryingFlag(bs)) {
				return qfalse;
			}
			// always chase if the enemy is carrying a flag
			if (EntityCarriesFlag(&entinfo)) {
				return qtrue;
			}
			// if the bot is getting the flag
			if (bs->ltgtype == LTG_GETFLAG) {
				return qfalse;
			}

			break;
		case GT_1FCTF:
			// never chase if carrying the flag
			if (Bot1FCTFCarryingFlag(bs)) {
				return qfalse;
			}
			// always chase if the enemy is carrying a flag
			if (EntityCarriesFlag(&entinfo)) {
				return qtrue;
			}
			// if the bot is getting the flag
			if (bs->ltgtype == LTG_GETFLAG) {
				return qfalse;
			}

			break;
		case GT_OBELISK:
			// the bots should be dedicated to attacking the enemy obelisk
			if (bs->ltgtype == LTG_ATTACKENEMYBASE) {
				if (BotFeelingBad(bs) > 50) {
					return qfalse;
				}
				// if this enemy is an obelisk
				if (bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) {
					return qtrue;
				}

				return qfalse;
			}

			break;
		case GT_HARVESTER:
			// never chase if carrying cubes
			if (BotHarvesterCarryingCubes(bs)) {
				return qfalse;
			}
			// always chase if the enemy is carrying cubes
			if (EntityCarriesCubes(&entinfo)) {
				return qtrue;
			}

			break;
		default:
			break;
	}

	if (BotAggression(bs) > 50) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotWantsToHelp
=======================================================================================================================================
*/
int BotWantsToHelp(bot_state_t *bs) {
	return qtrue;
}

/*
=======================================================================================================================================
BotCanAndWantsToRocketJump
=======================================================================================================================================
*/
int BotCanAndWantsToRocketJump(bot_state_t *bs) {
	float rocketjumper;

	// if rocket jumping is disabled
	if (!bot_rocketjump.integer) {
		return qfalse;
	}
	// if no rocket launcher
	if (bs->inventory[INVENTORY_ROCKETLAUNCHER] <= 0) {
		return qfalse;
	}
	// if low on rockets
	if (bs->inventory[INVENTORY_ROCKETS] < 3) {
		return qfalse;
	}
	// never rocket jump with the quad damage powerup
	if (bs->inventory[INVENTORY_QUAD]) {
		if (bs->inventory[INVENTORY_HEALTH] < 200 && bs->inventory[INVENTORY_ARMOR] < 120) {
			return qfalse;
		}
	}
	// never rocket jump with the doubler powerup
	if (bs->inventory[INVENTORY_DOUBLER]) {
		if (bs->inventory[INVENTORY_HEALTH] < 100 && bs->inventory[INVENTORY_ARMOR] < 120) {
			return qfalse;
		}
	}
	// if low on health
	if (bs->inventory[INVENTORY_HEALTH] < 100 && bs->inventory[INVENTORY_ARMOR] < 10) {
		return qfalse;
	}

	rocketjumper = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_WEAPONJUMPING, 0, 1);

	if (rocketjumper < 0.5) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotHasScout
=======================================================================================================================================
*/
int BotHasScout(bot_state_t *bs) {

	// if no scout powerup
	if (!bs->inventory[INVENTORY_SCOUT]) {
		return qfalse;
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotHasPersistantPowerupAndWeapon
=======================================================================================================================================
*/
int BotHasPersistantPowerupAndWeapon(bot_state_t *bs) {

	// if the bot does not have a persistant powerup
	if (!bs->inventory[INVENTORY_SCOUT] && !bs->inventory[INVENTORY_GUARD] && !bs->inventory[INVENTORY_DOUBLER] && !bs->inventory[INVENTORY_AMMOREGEN]) {
		return qfalse;
	}
	// if the bot is very low on health
	if (bs->inventory[INVENTORY_HEALTH] < 60) {
		return qfalse;
	}
	// if the bot is low on health
	if (bs->inventory[INVENTORY_HEALTH] < 80) {
		// if the bot has insufficient armor
		if (bs->inventory[INVENTORY_ARMOR] < 40) {
			return qfalse;
		}
	}
	// if the bot can use the chain gun
	if (bs->inventory[INVENTORY_CHAINGUN] > 0 && bs->inventory[INVENTORY_BELT] > 60) {
		return qtrue;
	}
	// if the bot can use the nail gun
	if (bs->inventory[INVENTORY_NAILGUN] > 0 && bs->inventory[INVENTORY_NAILS] > 5) {
		return qtrue;
	}
	// if the bot can use the napalm launcher
	if (bs->inventory[INVENTORY_NAPALMLAUNCHER] > 0 && bs->inventory[INVENTORY_CANISTERS] > 5) {
		return qtrue;
	}
	// if the bot can use the rocket launcher
	if (bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 5) {
		return qtrue;
	}
	// if the bot can use the beam gun
	if (bs->inventory[INVENTORY_BEAMGUN] > 0 && bs->inventory[INVENTORY_BEAMGUN_AMMO] > 50) {
		return qtrue;
	}
	// if the bot can use the railgun
	if (bs->inventory[INVENTORY_RAILGUN] > 0 && bs->inventory[INVENTORY_SLUGS] > 5) {
		return qtrue;
	}
	// if the bot can use the plasma gun
	if (bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 20) {
		return qtrue;
	}
	// if the bot can use the bfg
	if (bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 7) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotGoCamp
=======================================================================================================================================
*/
void BotGoCamp(bot_state_t *bs, bot_goal_t *goal) {
	float camper;

	// the teammate that requested the camping
	bs->teammate = 0;
	// the bot made its own decision
	bs->decisionmaker = bs->client;
	bs->ordered = qfalse;
	// set the ltg type
	bs->ltgtype = LTG_CAMP;
	// set the team goal
	memcpy(&bs->teamgoal, goal, sizeof(bot_goal_t));

	camper = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CAMPER, 0, 1);
	// set the team goal time
	if (camper > 0.99) {
		bs->teamgoal_time = FloatTime() + 99999;
	} else {
		bs->teamgoal_time = FloatTime() + 120 + 180 * camper + random() * 15;
	}
	// set the last time the bot started camping
	bs->camp_time = FloatTime();
	// do NOT type arrive message
	bs->arrive_time = 1;
	// set message time to zero so bot will NOT show any message
	bs->teammessage_time = 0;
}

/*
=======================================================================================================================================
BotWantsToCamp
=======================================================================================================================================
*/
int BotWantsToCamp(bot_state_t *bs) {
	float camper;
	int cs, traveltime, besttraveltime;
	bot_goal_t goal, bestgoal;

	camper = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CAMPER, 0, 1);

	if (camper < 0.1) {
		return qfalse;
	}
	// if the bot already has a CTF or team goal
	if (bs->ltgtype == LTG_GETFLAG || bs->ltgtype == LTG_ATTACKENEMYBASE || bs->ltgtype == LTG_HARVEST || bs->ltgtype == LTG_DEFENDKEYAREA || bs->ltgtype == LTG_RUSHBASE || bs->ltgtype == LTG_RETURNFLAG || bs->ltgtype == LTG_TEAMHELP || bs->ltgtype == LTG_TEAMACCOMPANY || bs->ltgtype == LTG_CAMP || bs->ltgtype == LTG_CAMPORDER || bs->ltgtype == LTG_PATROL || bs->ltgtype == LTG_GETITEM || bs->ltgtype == LTG_KILL) {
		return qfalse;
	}
	// if camped recently
	if (bs->camp_time > FloatTime() - 60 + 300 * (1 - camper)) {
		return qfalse;
	}

	if (random() > camper) {
		bs->camp_time = FloatTime();
		return qfalse;
	}
	// if the bot isn't healthy enough
	if (!BotCanCamp(bs)) {
		return qfalse;
	}
	// find the closest camp spot
	besttraveltime = 99999;

	for (cs = trap_BotGetNextCampSpotGoal(0, &goal); cs; cs = trap_BotGetNextCampSpotGoal(cs, &goal)) {
		traveltime = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, goal.areanum, TFL_DEFAULT);

		if (traveltime && traveltime < besttraveltime) {
			besttraveltime = traveltime;
			memcpy(&bestgoal, &goal, sizeof(bot_goal_t));
		}
	}

	if (besttraveltime > 150) {
		return qfalse;
	}
	// ok found a camp spot, go camp there
	BotGoCamp(bs, &bestgoal);

	return qtrue;
}

/*
=======================================================================================================================================
BotDontAvoid
=======================================================================================================================================
*/
void BotDontAvoid(bot_state_t *bs, char *itemname) {
	bot_goal_t goal;
	int num;

	num = trap_BotGetLevelItemGoal(-1, itemname, &goal);

	while (num >= 0) {
		trap_BotRemoveFromAvoidGoals(bs->gs, goal.number);
		num = trap_BotGetLevelItemGoal(num, itemname, &goal);
	}
}

/*
=======================================================================================================================================
BotGoForPowerups
=======================================================================================================================================
*/
void BotGoForPowerups(bot_state_t *bs) {

	// don't avoid any of the powerups anymore
	BotDontAvoid(bs, "Quad Damage");
	BotDontAvoid(bs, "Invisibility");
	BotDontAvoid(bs, "Regeneration");
	// reset the long term goal time so the bot will go for the powerup
	// NOTE: the long term goal type doesn't change
	bs->ltg_time = 0;
}

/*
=======================================================================================================================================
BotRoamGoal
=======================================================================================================================================
*/
void BotRoamGoal(bot_state_t *bs, vec3_t goal) {
	int pc, i;
	float len, rnd;
	vec3_t dir, bestorg, belowbestorg;
	bsp_trace_t trace;

	for (i = 0; i < 10; i++) {
		// start at the bot origin
		VectorCopy(bs->origin, bestorg);

		rnd = random();

		if (rnd > 0.25) {
			// add a random value to the x-coordinate
			if (random() < 0.5) {
				bestorg[0] -= 800 * random() + 100;
			} else {
				bestorg[0] += 800 * random() + 100;
			}
		}

		if (rnd < 0.75) {
			// add a random value to the y-coordinate
			if (random() < 0.5) {
				bestorg[1] -= 800 * random() + 100;
			} else {
				bestorg[1] += 800 * random() + 100;
			}
		}
		// add a random value to the z-coordinate (NOTE: 48 = maxjump?)
		bestorg[2] += 96 * crandom();
		// trace a line from the origin to the roam target
		BotAI_Trace(&trace, bs->origin, NULL, NULL, bestorg, bs->entitynum, MASK_SOLID);
		// direction and length towards the roam target
		VectorSubtract(trace.endpos, bs->origin, dir);

		len = VectorNormalize(dir);
		// if the roam target is far away enough
		if (len > 200) {
			// the roam target is in the given direction before walls
			VectorScale(dir, len * trace.fraction - 40, dir);
			VectorAdd(bs->origin, dir, bestorg);
			// get the coordinates of the floor below the roam target
			belowbestorg[0] = bestorg[0];
			belowbestorg[1] = bestorg[1];
			belowbestorg[2] = bestorg[2] - 800;

			BotAI_Trace(&trace, bestorg, NULL, NULL, belowbestorg, bs->entitynum, MASK_SOLID);

			if (!trace.startsolid) {
				trace.endpos[2]++;

				pc = trap_PointContents(trace.endpos, bs->entitynum);

				if (!(pc & (CONTENTS_LAVA|CONTENTS_SLIME))) {
					VectorCopy(bestorg, goal);
					return;
				}
			}
		}
	}

	VectorCopy(bestorg, goal);
}

/*
=======================================================================================================================================
BotAttackMove
=======================================================================================================================================
*/
bot_moveresult_t BotAttackMove(bot_state_t *bs, int tfl) {
	int movetype, i, attackentity, attack_dist, attack_range;
	float attack_skill, jumper, croucher, dist, selfpreservation, strafechange_time;
	vec3_t forward, backward, sideward, start, hordir, up = {0, 0, 1}, mins = {-4, -4, -4}, maxs = {4, 4, 4};
	aas_entityinfo_t entinfo;
	bot_moveresult_t moveresult;
	bot_goal_t goal;
	bsp_trace_t bsptrace;

	attackentity = bs->enemy;

	if (attackentity < 0) {
		memset(&moveresult, 0, sizeof(moveresult));
		return moveresult;
	}

	if (bs->attackchase_time > FloatTime()) {
		// create the chase goal
		goal.entitynum = attackentity;
		goal.areanum = bs->lastenemyareanum;

		VectorCopy(bs->lastenemyorigin, goal.origin);
		VectorSet(goal.mins, -8, -8, -8);
		VectorSet(goal.maxs, 8, 8, 8);
		// move towards the goal
		trap_BotMoveToGoal(&moveresult, bs->ms, &goal, tfl);
		return moveresult;
	}

	memset(&moveresult, 0, sizeof(bot_moveresult_t));

	attack_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ATTACK_SKILL, 0, 1);
	jumper = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_JUMPER, 0, 1);
	croucher = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CROUCHER, 0, 1);
	selfpreservation = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_SELFPRESERVATION, 0, 1);
	// if the bot is in the air
	if (bs->cur_ps.groundEntityNum == ENTITYNUM_NONE) {
		return moveresult;
	}
	// if the bot is really stupid
	if (attack_skill < 0.2) {
		// check if the bot is blocking teammates
		BotCheckBlockedTeammates(bs);
		return moveresult;
	}
	// get the entity information
	BotEntityInfo(attackentity, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return moveresult;
	}
	// direction towards the enemy
	VectorSubtract(entinfo.origin, bs->origin, forward);
	// the distance towards the enemy
	dist = VectorNormalize(forward);

	VectorNegate(forward, backward);
	// walk, crouch or jump
	movetype = MOVE_WALK;
	// for long range attacks the bots should crouch more often
	if (bs->croucher > 0.1f && dist > 4096 - (2048 * bs->croucher)) {
		croucher += 0.5f;
	} else if (BotWantsToRetreat(bs)) {
		croucher = 0.0f;
	}
	// don't jump if the enemy is quite far away
	if (dist > 1024 + (1024 * bs->jumper)) {
		jumper = 0.0f;
	}
	// don't crouch when swimming
	if (trap_AAS_Swimming(bs->origin)) {
		bs->crouch_time = FloatTime() - 1;
	}

	if (bs->crouch_time < FloatTime() - 1) {
		if (random() < jumper) {
			movetype = MOVE_JUMP;
		// wait at least one second before crouching again
		} else if (bs->crouch_time < FloatTime() - 1 && random() < croucher) {
			bs->crouch_time = FloatTime() + croucher * 5;
		}
	}
	// if the bot wants to crouch
	if (bs->crouch_time > FloatTime()) {
		// get the start point shooting from
		VectorCopy(bs->origin, start);
		// get the crouch view height
		start[2] += CROUCH_VIEWHEIGHT;
		// only try to crouch if the enemy remains visible
		BotAI_Trace(&bsptrace, start, mins, maxs, entinfo.origin, bs->client, MASK_SHOT);
		// if the enemy remains visible from the predicted position
		if (bsptrace.fraction >= 1.0f || bsptrace.entityNum == attackentity) {
			movetype = MOVE_CROUCH;
		}
	}
	// if the bot should jump
	if (movetype == MOVE_JUMP) {
		// if jumped last frame
		if (bs->attackjump_time > FloatTime()) {
			movetype = MOVE_WALK;
		} else {
			bs->attackjump_time = FloatTime() + 1;
		}
	}
	// if the bot is using a close combat weapon or if the enemy is using a weapon with splash damage, go closer
	if ((BotUsesCloseCombatWeapon(bs) && BotAggression(bs) > 50) || ((entinfo.weapon == WP_NAPALMLAUNCHER || entinfo.weapon == WP_ROCKETLAUNCHER || entinfo.weapon == WP_BFG) && dist < 200 && selfpreservation < 0.5 && movetype != MOVE_CROUCH)) {
		attack_dist = 0;
		attack_range = 0;
	// if the bot is using the napalmlauncher, or the enemy is using the napalmlauncher
	} else if (bs->cur_ps.weapon == WP_NAPALMLAUNCHER || entinfo.weapon == WP_NAPALMLAUNCHER) {
		attack_dist = 4500;
		attack_range = 250;
	// if the bot is using the grenadelauncher, or the enemy is using the grenadelauncher
	} else if (bs->cur_ps.weapon == WP_GRENADELAUNCHER || entinfo.weapon == WP_GRENADELAUNCHER) {
		attack_dist = 2000;
		attack_range = 150;
	// if the bot is using the proxylauncher, or the enemy is using the proxylauncher
	} else if (bs->cur_ps.weapon == WP_PROXLAUNCHER || entinfo.weapon == WP_PROXLAUNCHER) {
		attack_dist = 700;
		attack_range = 100;
	// if the bot is using the beam gun
	} else if (bs->cur_ps.weapon == WP_BEAMGUN) {
		attack_dist = 0.75 * BEAMGUN_RANGE;
		attack_range = 0.25 * BEAMGUN_RANGE;
	// if the enemy is using the beam gun, stay away
	} else if (entinfo.weapon == WP_BEAMGUN && bs->cur_ps.weapon != WP_BEAMGUN) {
		attack_dist = BEAMGUN_RANGE + 200;
		attack_range = 100;
	// attacking obelisks
	} else if (bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) {
		attack_dist = 300;
		attack_range = 150;
	// use the bots individual prefered attack distances
	} else {
		attack_dist = trap_Characteristic_BInteger(bs->character, CHARACTERISTIC_ATTACK_DISTANCE, 0, 1000);
		attack_range = trap_Characteristic_BInteger(bs->character, CHARACTERISTIC_ATTACK_RANGE, 0, 1000);
	}
	// if the bot is stupid
	if (attack_skill <= 0.4) {
		// just walk to or away from the enemy
		if (dist > attack_dist + attack_range) {
			if (trap_BotMoveInDirection(bs->ms, forward, 400, movetype)) {
				return moveresult;
			}
		}

		if (dist < attack_dist - attack_range) {
			if (trap_BotMoveInDirection(bs->ms, backward, 400, movetype)) {
				return moveresult;
			}
		}

		return moveresult;
	}
	// increase the strafe time
	bs->attackstrafe_time += bs->thinktime;
	// get the strafe change time
	strafechange_time = 0.4 + (1 - attack_skill) * 0.2;

	if (attack_skill > 0.7) {
		strafechange_time += crandom() * 0.2;
	}
	// close combat weapons
	if (BotUsesCloseCombatWeapon(bs) && BotAggression(bs) > 50) {
		bs->attackstrafe_time = 0;
	}
	// if the strafe direction should be changed
	if (bs->attackstrafe_time > strafechange_time) {
		// some magic number :)
		if (random() > 0.935) {
			// flip the strafe direction
			bs->flags ^= BFL_STRAFERIGHT;
			bs->attackstrafe_time = 0;
		}
	}

	for (i = 0; i < 2; i++) {
		hordir[0] = forward[0];
		hordir[1] = forward[1];
		hordir[2] = 0;

		VectorNormalize(hordir);
		// get the sideward vector
		CrossProduct(hordir, up, sideward);
		// reverse the vector depending on the strafe direction
		if (bs->flags & BFL_STRAFERIGHT) {
			VectorNegate(sideward, sideward);
		}
		// randomly go back a little
		if (random() > 0.9) {
			VectorAdd(sideward, backward, sideward);
		} else {
			// walk forward or backward to get at the ideal attack distance
			if (dist > attack_dist + attack_range) {
				VectorAdd(sideward, forward, sideward);
			} else if (dist < attack_dist - attack_range) {
				VectorAdd(sideward, backward, sideward);
			}
		}
		// perform the movement
		if (trap_BotMoveInDirection(bs->ms, sideward, 400, movetype)) {
			return moveresult;
		}
		// movement failed, flip the strafe direction
		bs->flags ^= BFL_STRAFERIGHT;
		bs->attackstrafe_time = 0;
	}
	// bot couldn't do any useful movement
	//bs->attackchase_time = AAS_Time() + 6;
	return moveresult;
}

/*
=======================================================================================================================================
BotSameTeam
=======================================================================================================================================
*/
int BotSameTeam(bot_state_t *bs, int entnum) {

	if (bs->client < 0 || bs->client >= MAX_CLIENTS) {
		return qfalse;
	}

	if (entnum < 0 || entnum >= MAX_CLIENTS) {
		return qfalse;
	}

	if (gametype > GT_TOURNAMENT) {
		if (level.clients[bs->client].sess.sessionTeam == level.clients[entnum].sess.sessionTeam) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
InFieldOfVision
=======================================================================================================================================
*/
qboolean InFieldOfVision(vec3_t viewangles, int fov, vec3_t angles) {
	int i;
	float diff, angle;

	for (i = 0; i < 2; i++) {
		angle = AngleMod(viewangles[i]);
		angles[i] = AngleMod(angles[i]);
		diff = angles[i] - angle;

		if (angles[i] > angle) {
			if (diff > 180.0) {
				diff -= 360.0;
			}
		} else {
			if (diff < -180.0) {
				diff += 360.0;
			}
		}

		if (diff > 0) {
			if (diff > fov * 0.5) {
				return qfalse;
			}
		} else {
			if (diff < -fov * 0.5) {
				return qfalse;
			}
		}
	}

	return qtrue;
}

/*
=======================================================================================================================================
BotEntityVisible

Returns visibility in the range [0, 1] taking fog and water surfaces into account.
=======================================================================================================================================
*/
qboolean BotEntityVisible(playerState_t *ps, float fov, int ent) {
	int viewer, visdist, i, contents_mask, passent, hitent, infog, inlava, inwater, otherinfog, otherinlava, otherinwater;
	float squaredfogdist, waterfactor, vis, bestvis;
	bsp_trace_t trace;
	aas_entityinfo_t entinfo;
	vec3_t viewangles, eye, dir, vec, right, entangles, start, end, middle;

	if (ent < 0) {
		return qfalse;
	}
	// get the entity information
	BotEntityInfo(ent, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return qfalse;
	}
	// calculate middle of bounding box
	VectorAdd(entinfo.mins, entinfo.maxs, middle);
	VectorScale(middle, 0.5, middle);
	VectorAdd(entinfo.origin, middle, middle);
	// calculate eye position
	VectorCopy(ps->origin, eye);

	eye[2] += ps->viewheight;

	VectorSubtract(middle, eye, dir);

	visdist = bot_visualrange.value;

	if (VectorLength(dir) > visdist) {
		return qfalse;
	}
	// check if entity is within field of vision
	VectorCopy(ps->viewangles, viewangles);
	VectorToAngles(dir, entangles);

	if (!InFieldOfVision(viewangles, fov, entangles)) {
		return qfalse;
	}

	if (EntityIsInvisible(&entinfo) && VectorLength(dir) > 300) {
		return qfalse;
	}
	// set the right vector
	VectorCopy(dir, vec);
	VectorNormalize(vec);

	right[0] = vec[1];
	right[1] = vec[0];
	right[2] = 0;

	viewer = ps->clientNum;
	passent = viewer;
	hitent = ent;
	contents_mask = CONTENTS_SOLID;
	infog = (trap_AAS_PointContents(eye) & CONTENTS_FOG);
	inlava = (trap_AAS_PointContents(eye) & CONTENTS_LAVA);
	inwater = (trap_AAS_PointContents(eye) & (CONTENTS_SLIME|CONTENTS_WATER));
	otherinfog = (trap_AAS_PointContents(middle) & CONTENTS_FOG);
	otherinlava = (trap_AAS_PointContents(middle) & CONTENTS_LAVA);
	otherinwater = (trap_AAS_PointContents(middle) & (CONTENTS_SLIME|CONTENTS_WATER));
	waterfactor = 1.0;
	bestvis = 0;
	// if the bot or the entity is in lava
	if (inlava || otherinlava) {
		return qfalse;
	}
	// if the bot or the entity is in water
	if (inwater || otherinwater) {
		waterfactor = 0.5;
	}

	for (i = 0; i < 5; i++) {
		VectorCopy(eye, start);
		VectorCopy(middle, end);
		// trace from start to end
		BotAI_Trace(&trace, start, NULL, NULL, end, passent, contents_mask);
		// if a full trace or the hitent was hit
		if (trace.fraction >= 1.0f || trace.entityNum == hitent) {
			// check for fog, assuming there's only one fog brush where either the viewer or the entity is in or both are in
			if (infog && otherinfog) {
				VectorSubtract(trace.endpos, eye, dir);
				squaredfogdist = VectorLengthSquared(dir);
			} else if (infog) {
				VectorCopy(trace.endpos, start);
				BotAI_Trace(&trace, start, NULL, NULL, eye, viewer, CONTENTS_FOG);
				VectorSubtract(eye, trace.endpos, dir); // Tobias CHECK: VectorSubtract reversed origins
				squaredfogdist = VectorLengthSquared(dir);
			} else if (otherinfog) {
				VectorCopy(trace.endpos, end);
				BotAI_Trace(&trace, eye, NULL, NULL, end, viewer, CONTENTS_FOG);
				VectorSubtract(end, trace.endpos, dir);
				squaredfogdist = VectorLengthSquared(dir);
			} else {
				// if the entity and the viewer are not in fog assume there's no fog in between
				squaredfogdist = 0;
			}
			// decrease visibility with the view distance through fog
			vis = 1 / ((squaredfogdist * 0.001) < 1 ? 1 : (squaredfogdist * 0.001));
			// if entering water visibility is reduced
			vis *= waterfactor;

			if (vis > bestvis) {
				bestvis = vis;
			}
			// if pretty much no fog (fogparms > 350)
			if (bestvis >= 0.01) {
				//BotAI_Print(PRT_MESSAGE, "Visibility = %f. Distance = %f.\n", bestvis, VectorLength(dir));
				return qtrue;
			}
		}
		// check bottom and top of bounding box as well
		if (i == 0) {
			middle[2] -= (entinfo.maxs[2] - entinfo.mins[2]) * 0.5;
		} else if (i == 1) {
			middle[2] += entinfo.maxs[2] - entinfo.mins[2];
		} else if (i == 2) { // right side
			middle[2] -= (entinfo.maxs[2] - entinfo.mins[2]) * 0.5;
			VectorMA(eye, entinfo.maxs[0] - 0.5, right, eye);
		} else if (i == 3) { // left side
			VectorMA(eye, -2.0 * (entinfo.maxs[0] - 0.5), right, eye);
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotEntityIndirectlyVisible
=======================================================================================================================================
*/
static qboolean BotEntityIndirectlyVisible(bot_state_t *bs, int ent) {
	static char entityVisStatus[MAX_CLIENTS][MAX_GENTITIES];
	static int entityVisStatusNextCheck[MAX_CLIENTS][MAX_GENTITIES];
	int i, teammate;
	gentity_t *tent;
	qboolean checkStatus, vis;

	// if not in teamplay mode
	if (gametype < GT_TEAM) {
		return qfalse;
	}

	if (ent < 0 || ent >= ENTITYNUM_MAX_NORMAL) {
		return qfalse;
	}
	// ignore new enemies if we already have one
	if (bs->enemy >= 0) {
		return qfalse;
	}
	// there is no need to have obelisks indirectly visible
	if (bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) {
		return qfalse;
	}

	BotDetermineVisibleTeammates(bs);

	checkStatus = qfalse;

	for (i = 0; i < bs->numvisteammates; i++) {
		teammate = bs->visteammates[i];

		if (teammate < 0 || teammate >= MAX_CLIENTS) {
			continue; // should not happen
		}

		if (entityVisStatusNextCheck[teammate][ent] <= level.time) {
			checkStatus = qtrue;
			continue;
		}

		if (entityVisStatus[teammate][ent]) {
			return qtrue;
		}
	}

	if (!checkStatus) {
		return qfalse;
	}

	for (i = 0; i < bs->numvisteammates; i++) {
		teammate = bs->visteammates[i];

		if (teammate < 0 || teammate >= MAX_CLIENTS) {
			continue; // should not happen
		}

		if (entityVisStatusNextCheck[teammate][ent] > level.time) {
			continue;
		}

		tent = &g_entities[teammate];

		if (!tent->inuse) {
			continue;
		}

		if (!tent->client) {
			continue;
		}

		if (tent->health <= 0) {
			continue;
		}

		vis = (BotEntityVisible(&tent->client->ps, 180, ent) > 0);
		entityVisStatus[teammate][ent] = vis;
		entityVisStatusNextCheck[teammate][ent] = level.time + 1000 + rand() % 2000;

		if (vis) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "Entity is indirect visible!\n");
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotFindEnemy
=======================================================================================================================================
*/
const int BotFindEnemy(bot_state_t *bs, int curenemy) {
	int i, squaredist, cursquaredist, f, healthdecrease, enemyArea;
	float /*alertness, */aggression, enemypreference, easyfragger;
	aas_entityinfo_t entinfo, curenemyinfo, curbotinfo;
	vec3_t dir, angles;
	qboolean foundEnemy;
#ifdef DEBUG
	//int curdist, dist;
	char netname1[MAX_NETNAME];
	char netname2[MAX_NETNAME];
	char netname3[MAX_NETNAME];

	ClientName(bs->client, netname1, sizeof(netname1));
#endif
	//alertness = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ALERTNESS, 0, 1);
	enemypreference = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ENEMY_PREFERENCE, 0, 1);
	easyfragger = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_EASY_FRAGGER, 0, 1);
	aggression = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AGGRESSION, 0, 1);
	// check if the health decreased by a reliable method (consider automatic decrease if health > max. health!)
	healthdecrease = g_entities[bs->entitynum].client->lasthurt_time > level.time - 1000;
	// remember the current health value
	bs->lasthealth = bs->inventory[INVENTORY_HEALTH];

	if (gametype == GT_OBELISK) {
		vec3_t target;
		bot_goal_t *goal;
		bsp_trace_t trace;

		if (BotTeam(bs) == TEAM_RED) {
			goal = &blueobelisk;
		} else {
			goal = &redobelisk;
		}
		// if the obelisk is respawning
		if (g_entities[goal->entitynum].activator && g_entities[goal->entitynum].activator->s.frame == 2) {
			// Tobias FIXME: qtrue if Proxylauncher in bag to place mines at respawning obelisk, otherwise -> node WAIT or collect ammo?
			return qfalse;
		}
		// if the obelisk is visible
		VectorCopy(goal->origin, target);

		target[2] += 1;

		BotAI_Trace(&trace, bs->eye, NULL, NULL, target, bs->client, CONTENTS_SOLID);
		// if a full trace or the hitent is hit
		if (trace.fraction >= 1.0f || trace.entityNum == goal->entitynum) {
			if (goal->entitynum == bs->enemy) {
				return qfalse;
			}

			bs->enemy = goal->entitynum;
			bs->enemysight_time = FloatTime();
			bs->enemysuicide = qfalse;
			bs->enemydeath_time = 0;
			bs->enemyvisible_time = FloatTime();
			return qtrue;
		}
	}
	// if the bot already has an enemy
	if (curenemy >= 0) {
		// get the entity information
		BotEntityInfo(curenemy, &curenemyinfo);
		// if the entity information is valid
		if (!curenemyinfo.valid) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "BotFindEnemy: !curenemyinfo.valid\n"); // Tobias CHECK: shouldn't happen?
			return qfalse;
		}
		// if the entity isn't the bot self
		if (curenemyinfo.number == bs->entitynum) {
			return qfalse;
		}
		// only concentrate on flag carrier if not carrying a flag
		if (EntityCarriesFlag(&curenemyinfo) && !BotCTFCarryingFlag(bs)) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: Enemy (%s) is carrying a flag.\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)));
#endif
			return qfalse;
		}
		// only concentrate on cube carrier if not carrying cubes
		if (EntityCarriesCubes(&curenemyinfo) && !BotHarvesterCarryingCubes(bs)) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: Enemy (%s) is carrying cubes.\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)));
#endif
			return qfalse;
		}
		// looking for revenge
		if (curenemy == bs->revenge_enemy && bs->revenge_kills > 0) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: Found revenge enemy (%s).\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)));
#endif
			return qfalse;
		}
		// stupid bots don't accept new enemies at all
		if (enemypreference < 0.2) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: I already have an enemy (%s).\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)));
#endif
			return qfalse;
		}
		// calculate the distance towards the enemy
		VectorSubtract(curenemyinfo.origin, bs->origin, dir);
#ifdef DEBUG
		curdist = VectorLength(dir);
#endif
		// less aggressive bots will immediatly stop firing if the enemy is dead
		if (EntityIsDead(&curenemyinfo) && aggression < 0.5) {
			//bs->enemy = -1; // Tobias CECK: Added but outcommented, because this causes an error now!?
			curenemy = -1;
			cursquaredist = 0;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy dead! Immediately check for another enemy.\n", netname1);
#endif
		} else {
			cursquaredist = VectorLengthSquared(dir);
		}
	} else {
		cursquaredist = 0;
	}

	foundEnemy = qfalse;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if it's the current enemy
		if (i == curenemy) {
			continue;
		}
		// if the enemy has targeting disabled
		if (g_entities[i].flags & FL_NOTARGET) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "BotFindEnemy: !entinfo.valid\n"); // Tobias CHECK: shouldn't happen?
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if the entity isn't dead
		if (EntityIsDead(&entinfo)) {
			continue;
		}
		// ignore invisible, mined and burning enemies if already fighting
		if (bs->enemy >= 0 && (EntityIsInvisible(&entinfo) || EntityIsAlreadyMined(&entinfo))) {
			continue;
		}
		// if not an easy fragger don't shoot at chatting players
		if (easyfragger < 0.5 && EntityIsChatting(&entinfo)) {
			continue;
		}

		if (lastteleport_time > FloatTime() - 3) {
			VectorSubtract(entinfo.origin, lastteleport_origin, dir);

			if (VectorLengthSquared(dir) < Square(70)) {
				continue;
			}
		}
		// calculate the distance towards the enemy
		VectorSubtract(entinfo.origin, bs->origin, dir);
#ifdef DEBUG
		dist = VectorLength(dir);
#endif
		squaredist = VectorLengthSquared(dir);
		// if this entity is not carrying a flag or cubes
		if (!EntityCarriesFlag(&entinfo) && !EntityCarriesCubes(&entinfo)) {
// Tobias NOTE: check, repair, and extend this!
/*
			// if the enemy is standing still, or is just moving slowly (could be a camper)
			if (enemypreference > 0.4 && squaredist < Square(1024) && VectorLength(g_entities[i].client->ps.velocity) < 10) {
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: NewEn (%s) Enemy isn't moving (Vel. = %f).\n", netname1, ClientName(i, netname3, sizeof(netname3)), VectorLength(g_entities[i].client->ps.velocity));
#endif
				squaredist *= 0.25;
			}
			// if this enemy has the quad damage or the regenration powerup
			if (enemypreference > 0.7 && (entinfo.powerups & (1 << PW_QUAD) || entinfo.powerups & (1 << PW_REGEN))) {
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s (%i): CurEn (%s (%i)) Dist = %i (%i). NewEn (%s (%i)) Dist = %i (%i). Enemy has powerup.\n", netname1, bs->lasthealth, ClientName(curenemy, netname2, sizeof(netname2)), g_entities[curenemy].health, cursquaredist, curdist * curdist, ClientName(i, netname3, sizeof(netname3)), g_entities[i].health, squaredist, dist * dist);
#endif
				squaredist *= 0.25;
			}
			// if using the missile launcher, prefer enemies on stationary weapons
			if (COM_BitCheck(bs->cur_ps.weapons, WP_MISSILELAUNCHER) && (g_entities[i].client->ps.eFlags & EF_STAT_MG_ACTIVE)) {
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s (%i): CurEn (%s (%i)) Dist = %i (%i). NewEn (%s (%i)) Dist = %i (%i). Enemy using stationary weapon.\n", netname1, bs->lasthealth, ClientName(curenemy, netname2, sizeof(netname2)), g_entities[curenemy].health, cursquaredist, curdist * curdist, ClientName(i, netname3, sizeof(netname3)), g_entities[i].health, squaredist, dist * dist);
#endif
				squaredist *= 0.5;
			}
			// if this is enemy is attempting to disarm dynamite/build, they take preference!
			if (g_entities[i].client->ps.weapon == WP_PLIERS && g_entities[i].client->touchingTOI) {
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s (%i): CurEn (%s (%i)) Dist = %i (%i). NewEn (%s (%i)) Dist = %i (%i). Enemy is constructing.\n", netname1, bs->lasthealth, ClientName(curenemy, netname2, sizeof(netname2)), g_entities[curenemy].health, cursquaredist, curdist * curdist, ClientName(i, netname3, sizeof(netname3)), g_entities[i].health, squaredist, dist * dist);
#endif
				squaredist *= 0.25;
			}
*/
// Tobias END
			// if the bot already has an enemy, compare both enemies
			if (curenemy >= 0) {
				// if this enemy is not too far away, is carrying no flags or skulls, and the bot has a precise weapon
				if (enemypreference > 0.6 && squaredist < Square(700) && BotUsesMidRangeWeapon(bs) && !BotCTFCarryingFlag(bs) && !BotHarvesterCarryingCubes(bs)/* && bs->lasthealth - 40 > g_entities[curenemy].health*/) { // Tobias NOTE: care must be taken -> BEAMGUN range!
					// if the current enemy is lower on health than this one
					if (g_entities[i].health > g_entities[curenemy].health + 20) {
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s (%i): CurEn (%s (%i)) NewEn (%s (%i)) Skip new enemy, current enemy is lower on health.\n", netname1, bs->lasthealth, ClientName(curenemy, netname2, sizeof(netname2)), g_entities[curenemy].health, ClientName(i, netname3, sizeof(netname3)), g_entities[i].health);
#endif
						continue;
					}
					// prefer targets near the goal
					if (enemypreference > 0.8 && BotAggression(bs) > 50 && bs->ltgtype != 0 && DistanceSquared(entinfo.origin, bs->teamgoal.origin) * 1.5 > DistanceSquared(curenemyinfo.origin, bs->teamgoal.origin)) {
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: CurEn (%s) NewEn (%s). Skip new enemy, prefer the enemy near goal.\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)), ClientName(i, netname3, sizeof(netname3)));
#endif
						continue;
					}
					// prefer enemies with higher scores
					if (enemypreference > 0.9 && g_entities[i].client->ps.persistant[PERS_SCORE] < g_entities[curenemy].client->ps.persistant[PERS_SCORE] && g_entities[curenemy].client->ps.persistant[PERS_SCORE] > bs->cur_ps.persistant[PERS_SCORE]) {
#ifdef DEBUG
						BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s (%i): CurEn (%s (%i)). NewEn (%s (%i)). Skip new enemy, score (%i) lower current enemy score (%i) higher own score (%i).\n", netname1, bs->cur_ps.persistant[PERS_SCORE], ClientName(curenemy, netname2, sizeof(netname2)), g_entities[curenemy].client->ps.persistant[PERS_SCORE], ClientName(i, netname3, sizeof(netname3)), g_entities[i].client->ps.persistant[PERS_SCORE], g_entities[i].client->ps.persistant[PERS_SCORE], g_entities[curenemy].client->ps.persistant[PERS_SCORE], bs->cur_ps.persistant[PERS_SCORE]);
#endif
						continue;
					}
				}
				// if this enemy is further away than the current one
				if (squaredist > cursquaredist) {
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: CurEn (%s) Dist = %i (%i). NewEn (%s) Dist = %i (%i). Skip new enemy, it's further away than the old one!\n", netname1, ClientName(curenemy, netname2, sizeof(netname2)), cursquaredist, curdist * curdist, ClientName(i, netname3, sizeof(netname3)), squaredist, dist * dist);
#endif
					continue;
				}
			}
		}
/*
		// if the bot has no
		if (squaredist > Square(900.0 + alertness * 4000.0)) {
			continue;
		}
*/
		// if the bot's health decreased or the enemy is shooting
		if (curenemy < 0 && (healthdecrease || EntityIsShooting(&entinfo))) {
			f = 360;
		} else {
			f = trap_Characteristic_BInteger(bs->character, CHARACTERISTIC_FOV, 0, 360);
		}
		// check if the enemy is visible
		if (!BotEntityVisible(&bs->cur_ps, f, i)) {
			if (bs->enemy >= 0) {
				continue;
			}

			if (curenemy >= 0) {
				continue;
			}

			if (!BotEntityIndirectlyVisible(bs, i)) {
				continue;
			}

			enemyArea = BotPointAreaNum(i, entinfo.origin); // Tobias CHECK: i?

			if (enemyArea <= 0) {
				continue;
			}

			if (!trap_AAS_AreaReachability(enemyArea)) {
				continue;
			}
		}
		// if the enemy is quite far away and doesn't have a flag or cubes and the bot is not damaged try to ignore this enemy
		if (curenemy < 0 && squaredist > Square(100) && !healthdecrease && !EntityCarriesFlag(&entinfo) && !EntityCarriesCubes(&entinfo)) {
			// get the entity information
			BotEntityInfo(bs->client, &curbotinfo);
			// if the bot is invisible and want to get the flag, ignore enemies
			if (EntityIsInvisible(&curbotinfo) && bs->ltgtype == LTG_GETFLAG) {
				continue;
			}
			// if trying to activate an entity, ignore enemies
			if (bs->ainode == AINode_Seek_ActivateEntity) {
				continue;
			}
			// check if we can avoid this enemy
			VectorSubtract(bs->origin, entinfo.origin, dir);
			VectorToAngles(dir, angles);
			// if the bot isn't in the fov of the enemy
			if (!InFieldOfVision(entinfo.angles, 90, angles)) {
				// update some stuff for this enemy
				BotUpdateBattleInventory(bs, i); // Tobias CHECK: delete this after ENEMY_HEIGHT in BotAggression is replaced by real values?
				// if the bot doesn't really want to fight
				if (BotWantsToRetreat(bs)) {
					continue;
				}
			}
		}
		// found an enemy
		foundEnemy = qtrue;
		curenemy = entinfo.number;
		cursquaredist = squaredist;
		curenemyinfo = entinfo;
	}

	if (foundEnemy) {
		if (bs->enemy < 0) {
			bs->enemysight_time = FloatTime();
		}

		bs->enemy = curenemy;
		bs->aimnotperfect_time = FloatTime();
		bs->enemyvisible_time = FloatTime();
		bs->enemydeath_time = 0;
		bs->enemysuicide = qfalse;

		VectorCopy(entinfo.origin, bs->lastenemyorigin);

		bs->lastenemyareanum = BotPointAreaNum(curenemy, entinfo.origin); // Tobias CHECK: curenemy?

		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotTeamCubeCarrierVisible
=======================================================================================================================================
*/
int BotTeamCubeCarrierVisible(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if the cube carrier is not on the same team
		if (!BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if this player is carrying cubes
		if (!EntityCarriesCubes(&entinfo)) {
			continue;
		}
		// if the cube carrier is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}

		return i;
	}

	return -1;
}

/*
=======================================================================================================================================
BotEnemyCubeCarrierVisible
=======================================================================================================================================
*/
int BotEnemyCubeCarrierVisible(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if the cube carrier is on the same team
		if (BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if this player is carrying cubes
		if (!EntityCarriesCubes(&entinfo)) {
			continue;
		}
		// if the cube carrier is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}

		return i;
	}

	return -1;
}

/*
=======================================================================================================================================
BotTeamFlagCarrierVisible
=======================================================================================================================================
*/
int BotTeamFlagCarrierVisible(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if the flag carrier is not on the same team
		if (!BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if this player is carrying a flag
		if (!EntityCarriesFlag(&entinfo)) {
			continue;
		}
		// if the flag carrier is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}

		return i;
	}

	return -1;
}

/*
=======================================================================================================================================
BotTeamFlagCarrier
=======================================================================================================================================
*/
int BotTeamFlagCarrier(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if the flag carrier is not on the same team
		if (!BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if this player is carrying a flag
		if (!EntityCarriesFlag(&entinfo)) {
			continue;
		}

		return i;
	}

	return -1;
}

/*
=======================================================================================================================================
BotEnemyFlagCarrierVisible
=======================================================================================================================================
*/
int BotEnemyFlagCarrierVisible(bot_state_t *bs) {
	int i;
	aas_entityinfo_t entinfo;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if the flag carrier is on the same team
		if (BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if this player is carrying a flag
		if (!EntityCarriesFlag(&entinfo)) {
			continue;
		}
		// if the flag carrier is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}

		return i;
	}

	return -1;
}

/*
=======================================================================================================================================
BotCountVisibleEnemies
=======================================================================================================================================
*/
void BotCountVisibleEnemies(bot_state_t *bs, int *enemies, float range) {
	int i;
	aas_entityinfo_t entinfo;
	vec3_t dir;

	if (enemies) {
		*enemies = 0;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if not within range
		VectorSubtract(entinfo.origin, bs->origin, dir);

		if (VectorLengthSquared(dir) > Square(range)) {
			continue;
		}
		// if the enemy is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}

		if (enemies) {
			(*enemies)++;
		}
	}
}

/*
=======================================================================================================================================
BotCountVisibleTeamMatesAndEnemies
=======================================================================================================================================
*/
void BotCountVisibleTeamMatesAndEnemies(bot_state_t *bs, int *teammates, int *enemies, float range) {
	int i;
	aas_entityinfo_t entinfo;
	vec3_t dir;

	if (teammates) {
		*teammates = 0;
	}

	if (enemies) {
		*enemies = 0;
	}

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}

		if (gametype == GT_CTF || gametype == GT_1FCTF) {
			// if this player is carrying a flag
			if (!EntityCarriesFlag(&entinfo)) {
				continue;
			}
		}
		// if not within range
		VectorSubtract(entinfo.origin, bs->origin, dir);

		if (VectorLengthSquared(dir) > Square(range)) {
			continue;
		}
		// if the enemy is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			if (teammates) {
				(*teammates)++;
			}
		} else {
			if (enemies) {
				(*enemies)++;
			}
		}
	}
}

/*
=======================================================================================================================================
BotCountAllTeamMates

Counts all teammates inside a specific range, regardless if they are visible or not.
=======================================================================================================================================
*/
int BotCountAllTeamMates(bot_state_t *bs, float range) {
	int teammates, i;
	aas_entityinfo_t entinfo;
	vec3_t dir;

	// if not in teamplay mode
	if (gametype < GT_TEAM) {
		return 0;
	}

	teammates = 0;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if not within range
		VectorSubtract(entinfo.origin, bs->origin, dir);

		if (VectorLengthSquared(dir) > Square(range)) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			teammates++;
		}
	}

	return teammates;
}
// Tobias HACK
/*
=======================================================================================================================================
BotEqualizeTeamScore
=======================================================================================================================================
*/
qboolean BotEqualizeTeamScore(bot_state_t *bs) {
// DEBUG
	if (!bot_equalize.integer) {
		return qfalse;
	}
// DEBUGEND
	// if not in team deathmatch mode
	if (gametype != GT_TEAM) {
		return qfalse;
	}

	if (bs->enemyteamscore + bot_equalizer_teambon.value < bs->ownteamscore) { // DEBUG: bot_equalizer_teambon
#if 0 // DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "EQUALIZE for B! (%s) Blue scores: %s, Red scores: %i\n", enemyteamscore, ownteamscore);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "EQUALIZE for R! (%s) Blue scores: %s, Red scores: %i\n", enemyteamscore, ownteamscore);
		}
#endif // DEBUGEND
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotEqualizeWeakestHumanTeamScore
=======================================================================================================================================
*/
qboolean BotEqualizeWeakestHumanTeamScore(bot_state_t *bs) {
	char buf[MAX_INFO_STRING], modelName[MAX_QPATH];
	int i, femaleClient, maleClient, femaleScores, maleScores, referenceScores;
	aas_entityinfo_t entinfo;
	playerState_t ps;
// DEBUG
#if 0
	char netname[MAX_NETNAME];
#endif
	if (!bot_equalize.integer) {
		return qfalse;
	}
// DEBUGEND
	if (!TeamPlayIsOn()) {
		return qfalse;
	}

	femaleClient = 0;
	maleClient = 0;
	referenceScores = 999999;
	femaleScores = 0;
	maleScores = 0;

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if on the same team
		if (BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "BotEqualizeWeakestHumanTeamScore -> !entinfo.valid\n"); // Tobias CHECK: shouldn't happen?
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}

		trap_GetConfigstring(CS_PLAYERS + i, buf, sizeof(buf));
		// if no config string or no name
		if (!buf[0] || !*Info_ValueForKey(buf, "n")) {
			continue;
		}
		// skip spectators
		if (atoi(Info_ValueForKey(buf, "t")) == TEAM_SPECTATOR) {
			continue;
		}
//#if 0 // DEBUG
		// skip bots
		if (g_entities[i].r.svFlags & SVF_BOT) {
			continue;
		}
//#endif// DEBUGEND
		if (BotAI_GetClientState(i, &ps)) {
			ClientSkin(i, modelName, sizeof(modelName));

			if (!Q_stricmp(modelName, "razor")) {
				referenceScores = ps.persistant[PERS_SCORE];
			}

			if (!Q_stricmp(modelName, "mynx")) { // DEBUG (major)
				femaleClient = i;
				femaleScores = ps.persistant[PERS_SCORE] - bot_equalizer_fembon.value; // DEBUG: bot_equalizer_fembon
			}

			if (!Q_stricmp(modelName, "james")) { // DEBUG (sarge)
				maleClient = i;
				maleScores = ps.persistant[PERS_SCORE];
			}
		}
	}

	if (referenceScores <= maleScores || referenceScores <= femaleScores) {
#if 0 // DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "NO CHANGES (R is NOT FIRST). R score: %i, M score: %i, F score: %i.\n", referenceScores, maleScores, femaleScores);
		}
#endif // DEBUGEND
		return qfalse;
	}

	if ((bs->enemy == femaleClient && femaleScores < maleScores) || (bs->enemy == maleClient && maleScores < femaleScores)) {
#if 0 // DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			if (maleScores < femaleScores) {
				BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "EQUALIZE for M! (%s) Enemy: %s, R score: %i, M score: %i, F score: %i.\n", ClientName(bs->client, netname, sizeof(netname)), ClientName(bs->enemy, modelName, sizeof(modelName)), referenceScores, maleScores, femaleScores);
			} else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "EQUALIZE for F! (%s) Enemy: %s, R score: %i, M score: %i, F score: %i.\n", ClientName(bs->client, netname, sizeof(netname)), ClientName(bs->enemy, modelName, sizeof(modelName)), referenceScores, maleScores, femaleScores);
			}
		}
#endif // DEBUGEND
		return qtrue;
	}

	return qfalse;
}
// Tobias END
/*
=======================================================================================================================================
BotAimAtEnemy
=======================================================================================================================================
*/
void BotAimAtEnemy(bot_state_t *bs) {
	int i, mask;
	float dist, f, aim_skill, aim_accuracy, speed, reactiontime, viewType, enemyHeight;
	vec3_t origin, dir, bestorigin, end, start, target, groundtarget, cmdmove, enemyvelocity, middleOfArc, topOfArc;
	static vec3_t rmins = {-4, -4, -4}, rmaxs = {4, 4, 4}; // rockets/missiles
//	static vec3_t bmins = {-6, -6, -6}, bmaxs = {6, 6, 6}; // satchel/dynamite/bombs
//	static vec3_t fmins = {-30, -30, -30}, fmaxs = {30, 30, 30}; // flame chunks
	float *mins, *maxs;
	weaponinfo_t wi;
	aas_entityinfo_t entinfo;
	aas_clientmove_t move;
	bot_goal_t goal;
	bsp_trace_t trace;
	playerState_t ps;
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	// if the bot has no enemy
	if (bs->enemy < 0) {
		return;
	}

	if (bs->weaponnum <= WP_NONE || bs->weaponnum >= WP_NUM_WEAPONS) {
		return;
	}
	// get the entity information
	BotEntityInfo(bs->enemy, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return;
	}
	// if this is not a player (could be an obelisk)
	if (bs->enemy >= MAX_CLIENTS) {
		// if the entity is visible
		VectorCopy(entinfo.origin, target);
		// if attacking an obelisk
		if (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum) {
			target[2] += OBELISK_TARGET_HEIGHT;
		}
		// aim at the entity
		VectorSubtract(target, bs->eye, dir);
		VectorToAngles(dir, bs->ideal_viewangles);
		// set the aim target before trying to attack
		VectorCopy(target, bs->aimtarget);
		return;
	}

	//BotAI_Print(PRT_MESSAGE, "%s: Aiming at client %d.\n", netname, bs->enemy);
	// get the weapon information
	trap_BotGetWeaponInfo(bs->ws, bs->weaponnum, &wi);
	// get the weapon specific aim accuracy and or aim skill
	switch (wi.number) {
		case WP_MACHINEGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MACHINEGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_CHAINGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_CHAINGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_SHOTGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_SHOTGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_NAILGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_NAILGUN, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_NAILGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_PROXLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PROXLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_GRENADELAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_GRENADELAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_GRENADELAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_NAPALMLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_NAPALMLAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_NAPALMLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_ROCKETLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_ROCKETLAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_ROCKETLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_BEAMGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BEAMGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_RAILGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_RAILGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_PLASMAGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_PLASMAGUN, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PLASMAGUN, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
		case WP_BFG:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_BFG10K, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BFG10K, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
		default:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
	}
// Tobias FIXME: this is nonsense, if reactiontime is 0 than this has no effect (0.5 * 0 = 0)
/*
	if (aim_skill > 0.95) {
		// don't aim too early
		reactiontime = 0.5 * trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 5);

		if (bs->enemysight_time > FloatTime() - reactiontime) {
			return;
		}

		if (bs->teleport_time > FloatTime() - reactiontime) {
			return;
		}
	}
*/
// Tobias END
	VectorSubtract(entinfo.origin, entinfo.lastvisorigin, enemyvelocity);
	VectorScale(enemyvelocity, 1 / entinfo.update_time, enemyvelocity);
	// enemy origin and velocity is remembered every 0.5 seconds
	if (bs->enemyposition_time < FloatTime()) {
		bs->enemyposition_time = FloatTime() + 0.5;
		VectorCopy(enemyvelocity, bs->enemyvelocity);
		VectorCopy(entinfo.origin, bs->enemyorigin);
	}
	// if not extremely skilled
	if (aim_skill < 0.9) {
		VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
		// if the enemy moved a bit
		if (VectorLengthSquared(dir) > Square(48)) {
			// if the enemy changed direction
			if (DotProduct(bs->enemyvelocity, enemyvelocity) < 0) {
				// aim accuracy should be worse now
				aim_accuracy *= 0.7f;
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, "%s: Enemy changed direction (*0.7): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
			}
		}
	}
	// if the enemy is invisible then shoot crappy most of the time
	if (EntityIsInvisible(&entinfo)) {
		if (random() > 0.1) {
			aim_accuracy *= 0.4f;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: Enemy invisible (*0.4): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
		}
	}
	// Tobias TODO: add prone ~ + 0.2f;
	// if the bot is crouching
	if (bs->cur_ps.pm_flags & PMF_DUCKED) {
		aim_accuracy += 0.2f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Crouching (+0.1): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
	}
	// if the bot is standing still
	if (VectorLengthSquared(bs->cur_ps.velocity) <= 0) {
		aim_accuracy += 0.3f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Standing still (+0.2): aim accuracy: %f, vel: %i.\n", netname, aim_accuracy, (int)VectorLengthSquared(bs->cur_ps.velocity));
#endif
	}
	// if the enemy is standing still
	f = VectorLength(bs->enemyvelocity);

	if (f > 200.0f) {
		f = 200.0f;
	}

	aim_accuracy += 0.2f * (0.5f - (f / 200.0f));
#ifdef DEBUG
	if (f <= 0) {
		BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy standing still (+0.2): aim accuracy: %f, Enemy speed: %i.\n", netname, aim_accuracy, (int)f);
	}
#endif
	// if the bot needs some time to react on the enemy, aiming gets better with time
	reactiontime = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);

	if (reactiontime > 1.75f) {
		f = FloatTime() - bs->enemysight_time;

		if (f > 2.0f) {
			f = 2.0f;
		}

		aim_accuracy += 0.1f * f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Time based aim accuracy: %f.\n", netname, aim_accuracy);
#endif
	}
	// consider enemy model specific attributes
	if (BotEqualizeWeakestHumanTeamScore(bs) || BotEqualizeTeamScore(bs)) {
		aim_accuracy *= bot_equalizer_aim.value; // DEBUG: bot_equalizer_aim
#ifdef DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Camouflage skin: EQUALIZE for BLUE! RED score = %i, BLUE score = %i, aim accuracy = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, aim_accuracy);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Camouflage skin: EQUALIZE for RED! BLUE score = %i, RED score = %i, aim accuracy = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, aim_accuracy);
		}
#endif
	}
	// keep a minimum accuracy
	if (aim_accuracy <= 0.0f) {
		aim_accuracy = 0.0001f;
	}
	// also set a maximum accuracy
	if (aim_accuracy > 1.0f) {
		aim_accuracy = 1.0f;
	}

	bs->allowHitWorld = qfalse;
	// if the enemy is NOT visible
	if (!BotEntityVisible(&bs->cur_ps, 360, bs->enemy)) {
		VectorCopy(bs->lastenemyorigin, bestorigin);

		bestorigin[2] += 8;
		// if the bot is skilled enough
		if (aim_skill > 0.5) {
			// do prediction shots around corners
			if (!BotUsesInstantHitWeapon(bs)) {
				// create the chase goal
				goal.entitynum = bs->client;
				goal.areanum = bs->areanum;

				VectorCopy(bs->eye, goal.origin);
				VectorSet(goal.mins, -8, -8, -8);
				VectorSet(goal.maxs, 8, 8, 8);

				if (trap_BotPredictVisiblePosition(bs->lastenemyorigin, bs->lastenemyareanum, &goal, TFL_DEFAULT, target)) {
					VectorSubtract(target, bs->eye, dir);
					// if the hitpoint is far enough from the bot
					if (VectorLengthSquared(dir) > Square(100)) {
						VectorCopy(target, bestorigin);
						// if the projectile does large radial damage try to aim at the ground in front of the enemy
						if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
							bestorigin[2] -= 20;
						}
					}
					// allow the bot to hit the world
					bs->allowHitWorld = qtrue;
					aim_accuracy = 1.0f;
				}
			}
		}

		VectorCopy(bestorigin, bs->aimtarget);
	// if the enemy is visible
	} else {
		// get the start point shooting from
		// NOTE: the x and y projectile start offsets are ignored
		VectorCopy(bs->origin, start);

		start[2] += bs->cur_ps.viewheight;
		start[2] += wi.offset[2];
		// get the end point aiming at
		VectorCopy(entinfo.origin, bestorigin);
		// try to aim at the head
		if (BotAI_GetClientState(bs->enemy, &ps)) {
			bestorigin[2] += ps.viewheight;
		}

		BotAI_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, mask);
		// if the enemy is NOT hit
		if (trace.fraction < 1.0f && trace.entityNum != entinfo.number) {
			// aim a bit higher
			bestorigin[2] += 8;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy NOT hit. Aiming higher!\n", netname);
#endif
			BotAI_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, mask);
			// if the enemy is still NOT hit
			if (trace.fraction < 1.0f && trace.entityNum != entinfo.number) {
				// aim a bit lower
				bestorigin[2] -= 32;
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy NOT hit. Aiming lower!\n", netname);
#endif
			}
		}
		// if it is not an instant hit weapon the bot might want to predict the enemy
		if (!BotUsesInstantHitWeapon(bs)) {
			// direction towards the enemy
			VectorSubtract(bestorigin, bs->origin, dir);
			// distance towards the enemy
			dist = VectorLength(dir);
			// direction the enemy is moving in
			VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
			// if the enemy is NOT pretty far away and strafing just small steps left and right
			if (!(dist > 100 && VectorLengthSquared(dir) < Square(32))) {
				// if skilled enough and if the weapon is ready to fire, do exact prediction
				if (aim_skill > 0.8 && bs->cur_ps.weaponstate == WEAPON_READY) {
					VectorSubtract(entinfo.origin, bs->origin, dir);
					// distance towards the enemy
					dist = VectorLength(dir);
					// direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					VectorScale(dir, 1 / entinfo.update_time, dir);
					VectorCopy(entinfo.origin, origin);

					origin[2] += 1;

					VectorClear(cmdmove);
					//AAS_ClearShownDebugLines();
					// movement prediction
					trap_AAS_PredictClientMovement(&move, bs->enemy, origin, PRESENCE_CROUCH, qfalse, qfalse, dir, cmdmove, 0, dist * 10 / wi.speed, 0.1f, 0, 0, qfalse);
					VectorCopy(move.endpos, bestorigin);
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Predicted speed = %f, Frames = %f.\n", netname, FloatTime(), VectorLength(dir), dist * 10 / wi.speed);
#endif
				// if not that skilled do linear prediction
				} else if (aim_skill > 0.4) {
					// direction towards the enemy
					VectorSubtract(entinfo.origin, bs->origin, dir);
					// distance towards the enemy
					dist = VectorLength(dir);
					// direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);

					dir[2] = 0;
					speed = VectorNormalize(dir) / entinfo.update_time;
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, "%s: Speed = %f, wi->speed = %f.\n", netname, speed, wi.speed);
#endif
					// best spot to aim at
					VectorMA(entinfo.origin, (dist / wi.speed) * speed, dir, bestorigin);
				}
			}
		}
		// if the projectile does large radial damage
		if (aim_skill > 0.6 && (wi.proj.damagetype & DAMAGETYPE_RADIAL) && wi.proj.radius > 50) {
			// if the enemy isn't standing significantly higher than the bot and isn't in water
			if (entinfo.origin[2] < bs->origin[2] + 16 && !(trap_AAS_PointContents(entinfo.origin) & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA))) {
				// try to aim at the ground in front of the enemy
				VectorCopy(entinfo.origin, end);

				end[2] -= 64;

				BotAI_Trace(&trace, entinfo.origin, mins, maxs, end, entinfo.number, mask);
				VectorCopy(bestorigin, groundtarget); // Tobias CHECK: is 'bestorigin' wrong now (changed above), or was it always strange to use 'bestorigin' instead of 'end' ?

				if (trace.startsolid) {
					groundtarget[2] = entinfo.origin[2] - 16;
				} else {
					groundtarget[2] = trace.endpos[2] - 8;
				}
				// trace a line from projectile start to ground target
				BotAI_Trace(&trace, start, mins, maxs, groundtarget, bs->entitynum, mask);
				// if hitpoint is not vertically too far from the ground target
				if (fabs(trace.endpos[2] - groundtarget[2]) < 50) {
					VectorSubtract(trace.endpos, groundtarget, dir);
					// if the hitpoint is near enough the ground target
					if (VectorLengthSquared(dir) < Square(60)) {
						VectorSubtract(trace.endpos, start, dir);
						// if the hitpoint is far enough from the bot
						if (VectorLengthSquared(dir) > Square(100)) {
							// check if the bot is visible from the ground target
							trace.endpos[2] += 1;

							BotAI_Trace(&trace, trace.endpos, mins, maxs, entinfo.origin, entinfo.number, mask);
							// if the projectile will not be blocked
							if (trace.fraction >= 1.0f) {
#ifdef DEBUG
								BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Aiming at ground.\n", netname, FloatTime());
#endif
								VectorCopy(groundtarget, bestorigin);
								// allow the bot to hit the world (shooting at the ground)
								bs->allowHitWorld = qtrue;
							}
						}
					}
				}
			}
		}
//#ifdef DEBUG
		if (!bot_noaccerror.integer) {
//#endif
			bestorigin[0] += 20 * crandom() * (1 - aim_accuracy);
			bestorigin[1] += 20 * crandom() * (1 - aim_accuracy);
			bestorigin[2] += 10 * crandom() * (1 - aim_accuracy);
//#ifdef DEBUG
		}
//#endif
		BotAI_Trace(&trace, bs->eye, mins, maxs, bestorigin, bs->entitynum, mask);
		VectorCopy(trace.endpos, bs->aimtarget);
	}
// Tobias NOTE: for developers...
/*
		(o = botai trace line)
		(* = projectile travel line)
                                                                                        o  <---- topOfArc[2] trace
                                                                                  o
                                                                            o
                                                                       o                *  <---- grenade arc is always below trace!
                                                                  o   *
                                                             o *
                                                        o*
                                                   o *
                                               o *
                                          o   *
                                     o     *
             trace start ----> o        *
                                      *
                            _______ *
                           |      *
                           |     * |
                           |    *  |
        projectile start --|-> *   |
                           |       |
                           |       |
                           |       |
                           |       |
NOTE: 1. We can't trace a parabola. That's why we trace a bit above the real arc of travel in straight line. If the projectile will hit some overhead ledge, the bot will fire the projectile straight ahead!
NOTE: 2. wi.proj.gravity, was never set in the botfiles, it MUST be set to make this work properly (an alternate would be to completely do all this without the need of weaponinfo). Without modified botfiles everything works as normally.
NOTE: 3. wi.proj.gravity is a simple configurable (pseudo)multiplier, this seems to be the fastest way to compute 100% accurate ballistics (e.g: projectile speed 700 -> gravity 0.3, projectile speed 10000 -> gravity 0.04, etc.).
NOTE: 4. It doesn't really make sense to use DEFAULT_GRAVITY or g_gravity (projectiles are not influenced by g_gravity at all in Q3a).
NOTE: 5. Although my method of computing the ballistics looks like magic, it is the simplest and fastest way I could think of (correct but slow math: -> https://en.wikipedia.org/wiki/Parabola)
         Computing the ballistics this way takes configurable projectile speed, configurable projectile gravity, dynamic enemy height and dynamic enemy distance into account.
NOTE: 6. This code becomes more precise the faster the projectile moves. Grenades are too slow in Q3a! Fix this: average H-Gren.: bolt speed 1300, = ~60 meters. (gravity 0.35)

WARNING 1: Accuracy is nearly 100% even with very fast projectiled weapons (e.g.: speed 20000 etc.), this means bots will always hit their opponents, even with a bow (eventually decrease the bots individual aim_accuracy)
WARNING 2: Bots will also throw grenades through windows even from distance, so be careful!
*/
	if (BotUsesGravityAffectedProjectileWeapon(bs)) {
		// direction towards the enemy
		VectorSubtract(bestorigin, bs->origin, dir);

		enemyHeight = dir[2]; // Tobias CHECK: enemyHeight = dist[2]?
		// distance towards the enemy
		dist = VectorNormalize(dir);
		// get the start point shooting from, for safety sake take overhead ledges into account (so we trace along the highest point of the arc, from start to middle)
		VectorCopy(bs->origin, start);

		start[2] += bs->cur_ps.viewheight + 20;
		// half the distance will be the middle of the projectile arc (highest point the projectile will travel)
		VectorMA(start, dist * 0.5, dir, middleOfArc);
		VectorCopy(middleOfArc, topOfArc);

		topOfArc[2] += (dist * wi.proj.gravity) + (enemyHeight > 0 ? enemyHeight * 0.1 : 0);
		// trace from start to middle, check if the projectile will be blocked by something
		BotAI_Trace(&trace, start, mins, maxs, topOfArc, entinfo.number, mask);
		// if the projectile will not be blocked
		if (trace.fraction >= 1.0f) {
			// get the end point (the projectiles impact point), for safety sake take overhead ledges into account (so we trace along the highest point of the arc, from middle to end)
			VectorCopy(entinfo.origin, end);

			end[2] += 20;
			// trace from middle to end, check if the projectile will be blocked by something
			BotAI_Trace(&trace, topOfArc, mins, maxs, end, entinfo.number, mask);
			// if the projectile will not be blocked
			if (trace.fraction >= 1.0f) {
				// take projectile speed, gravity and enemy height into account
				bestorigin[2] += (dist * dist / wi.speed * wi.proj.gravity) + (enemyHeight > 0 ? enemyHeight * 0.1 : 0);
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Predicted speed = %f, Frames = %f, Enemy height = %f.\n", netname, FloatTime(), VectorLength(dir), dist * 10 / wi.speed, enemyHeight);
#endif
			}
		}
	}
	// get aim direction
	VectorSubtract(bestorigin, bs->eye, dir);
//#ifdef DEBUG
	if (!bot_noaccerror.integer) {
//#endif
		if (BotUsesInstantHitWeapon(bs)) {
			// distance towards the enemy
			dist = VectorLength(dir);

			if (dist > 150) {
				dist = 150;
			}

			f = 0.6f + dist / 150 * 0.4f;
			aim_accuracy *= f;
		}
		// add some random stuff to the aim direction depending on the aim accuracy
		if (aim_accuracy < 0.8f) {
			VectorNormalize(dir);

			for (i = 0; i < 3; i++) {
				dir[i] += 0.3f * crandom() * (1 - aim_accuracy);
			}
		}
//#ifdef DEBUG
	}
//#endif
	// set the ideal view angles
	VectorToAngles(dir, bs->ideal_viewangles);
	// take the weapon spread into account for lower skilled bots
	bs->ideal_viewangles[PITCH] += 6 * wi.vspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[PITCH] = AngleMod(bs->ideal_viewangles[PITCH]);
	bs->ideal_viewangles[YAW] += 6 * wi.hspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[YAW] = AngleMod(bs->ideal_viewangles[YAW]);
	// if the bots should be really challenging
	viewType = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_TYPE, 0, 1);

	if (viewType > 0.9
//#ifdef DEBUG
		|| bot_challenge.integer
//#endif
		) {
		// if the bot is really accurate and has the enemy in view for some time
		if (aim_accuracy > 0.9f && bs->enemysight_time < FloatTime() - 1) {
			// set the view angles directly
			if (bs->ideal_viewangles[PITCH] > 180) {
				bs->ideal_viewangles[PITCH] -= 360;
			}

			VectorCopy(bs->ideal_viewangles, bs->viewangles);
			trap_EA_View(bs->client, bs->viewangles);
		}
	}
#ifdef DEBUG
	BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Final aim accuracy: %f.\n", netname, aim_accuracy);
#endif
}
// Tobias DEBUG
/*
=======================================================================================================================================
BotAimAtEnemy_New
=======================================================================================================================================
*/
void BotAimAtEnemy_New(bot_state_t *bs) {
	int i, mask;
	float dist, f, aim_skill, aim_accuracy, speed, reactiontime, viewType, enemyHeight;
	vec3_t origin, dir, bestorigin, end, start, target, groundtarget, cmdmove, enemyvelocity, middleOfArc, topOfArc;
	static vec3_t rmins = {-4, -4, -4}, rmaxs = {4, 4, 4}; // rockets/missiles
//	static vec3_t bmins = {-6, -6, -6}, bmaxs = {6, 6, 6}; // satchel/dynamite/bombs
//	static vec3_t fmins = {-30, -30, -30}, fmaxs = {30, 30, 30}; // flame chunks
	float *mins, *maxs;
	weaponinfo_t wi;
	aas_entityinfo_t entinfo;
	aas_clientmove_t move;
	bot_goal_t goal;
	bsp_trace_t trace;
	playerState_t ps;
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	// if the bot has no enemy
	if (bs->enemy < 0) {
		return;
	}

	if (bs->weaponnum <= WP_NONE || bs->weaponnum >= WP_NUM_WEAPONS) {
		return;
	}
	// get the entity information
	BotEntityInfo(bs->enemy, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return;
	}
	// if this is not a player (could be an obelisk)
	if (bs->enemy >= MAX_CLIENTS) {
		// if the entity is visible
		VectorCopy(entinfo.origin, target);
		// if attacking an obelisk
		if (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum) {
			target[2] += OBELISK_TARGET_HEIGHT;
		}
		// aim at the entity
		VectorSubtract(target, bs->eye, dir);
		VectorToAngles(dir, bs->ideal_viewangles);
		// set the aim target before trying to attack
		VectorCopy(target, bs->aimtarget);
		return;
	}

	//BotAI_Print(PRT_MESSAGE, "%s: Aiming at client %d.\n", netname, bs->enemy);
	// get the weapon information
	trap_BotGetWeaponInfo(bs->ws, bs->weaponnum, &wi);
	// get the weapon specific aim accuracy and or aim skill
	switch (wi.number) {
		case WP_MACHINEGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MACHINEGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_CHAINGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_CHAINGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_SHOTGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_SHOTGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_NAILGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_NAILGUN, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_NAILGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_PROXLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PROXLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_GRENADELAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_GRENADELAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_GRENADELAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_NAPALMLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_NAPALMLAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_NAPALMLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_ROCKETLAUNCHER:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_ROCKETLAUNCHER, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_ROCKETLAUNCHER, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			//mask = MASK_MISSILESHOT;
			mask = MASK_SHOT;
			break;
		case WP_BEAMGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BEAMGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_RAILGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_RAILGUN, 0, 1);
			mins = NULL;
			maxs = NULL;
			mask = MASK_SHOT;
			break;
		case WP_PLASMAGUN:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_PLASMAGUN, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PLASMAGUN, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
		case WP_BFG:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL_BFG10K, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BFG10K, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
		default:
			aim_skill = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_SKILL, 0, 1);
			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
			mins = rmins;
			maxs = rmaxs;
			mask = MASK_SHOT;
			break;
	}
// Tobias FIXME: this is nonsense, if reactiontime is 0 than this has no effect (0.5 * 0 = 0)
/*
	if (aim_skill > 0.95) {
		// don't aim too early
		reactiontime = 0.5 * trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 5);

		if (bs->enemysight_time > FloatTime() - reactiontime) {
			return;
		}

		if (bs->teleport_time > FloatTime() - reactiontime) {
			return;
		}
	}
*/
// Tobias END
	VectorSubtract(entinfo.origin, entinfo.lastvisorigin, enemyvelocity);
	VectorScale(enemyvelocity, 1 / entinfo.update_time, enemyvelocity);
	// enemy origin and velocity is remembered every 0.5 seconds
	if (bs->enemyposition_time < FloatTime()) {
		bs->enemyposition_time = FloatTime() + 0.5;
		VectorCopy(enemyvelocity, bs->enemyvelocity);
		VectorCopy(entinfo.origin, bs->enemyorigin);
	}
	// if not extremely skilled
	if (aim_skill < 0.9) {
		VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
		// if the enemy moved a bit
		if (VectorLengthSquared(dir) > Square(48)) {
			// if the enemy changed direction
			if (DotProduct(bs->enemyvelocity, enemyvelocity) < 0) {
				// aim accuracy should be worse now
				aim_accuracy *= 0.7f;
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, "%s: Enemy changed direction (*0.7): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
			}
		}
	}
	// if the enemy is invisible then shoot crappy most of the time
	if (EntityIsInvisible(&entinfo)) {
		if (random() > 0.1) {
			aim_accuracy *= 0.4f;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, "%s: Enemy invisible (*0.4): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
		}
	}
	// Tobias TODO: add prone ~ + 0.2f;
	// if the bot is crouching
	if (bs->cur_ps.pm_flags & PMF_DUCKED) {
		aim_accuracy += 0.2f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Crouching (+0.1): aim accuracy: %f.\n", netname, aim_accuracy);
#endif
	}
	// if the bot is standing still
	if (VectorLengthSquared(bs->cur_ps.velocity) <= 0) {
		aim_accuracy += 0.3f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Standing still (+0.2): aim accuracy: %f, vel: %i.\n", netname, aim_accuracy, (int)VectorLengthSquared(bs->cur_ps.velocity));
#endif
	}
	// if the enemy is standing still
	f = VectorLength(bs->enemyvelocity);

	if (f > 200.0f) {
		f = 200.0f;
	}

	aim_accuracy += 0.2f * (0.5f - (f / 200.0f));
#ifdef DEBUG
	if (f <= 0) {
		BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy standing still (+0.2): aim accuracy: %f, Enemy speed: %i.\n", netname, aim_accuracy, (int)f);
	}
#endif
	// if the bot needs some time to react on the enemy, aiming gets better with time
	reactiontime = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 1);

	if (reactiontime > 1.75f) {
		f = FloatTime() - bs->enemysight_time;

		if (f > 2.0f) {
			f = 2.0f;
		}

		aim_accuracy += 0.1f * f;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Time based aim accuracy: %f.\n", netname, aim_accuracy);
#endif
	}
	// consider enemy model specific attributes
	if (BotEqualizeWeakestHumanTeamScore(bs) || BotEqualizeTeamScore(bs)) {
		aim_accuracy *= bot_equalizer_aim.value; // DEBUG: bot_equalizer_aim
#ifdef DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Camouflage skin: EQUALIZE for BLUE! RED score = %i, BLUE score = %i, aim accuracy = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, aim_accuracy);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Camouflage skin: EQUALIZE for RED! BLUE score = %i, RED score = %i, aim accuracy = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, aim_accuracy);
		}
#endif
	}
	// keep a minimum accuracy
	if (aim_accuracy <= 0.0f) {
		aim_accuracy = 0.0001f;
	}
	// also set a maximum accuracy
	if (aim_accuracy > 1.0f) {
		aim_accuracy = 1.0f;
	}

	bs->allowHitWorld = qfalse;
	// if the enemy is NOT visible
	if (!BotEntityVisible(&bs->cur_ps, 360, bs->enemy)) {
		VectorCopy(bs->lastenemyorigin, bestorigin);

		bestorigin[2] += 8;
		// if the bot is skilled enough
		if (aim_skill > 0.5) {
			// do prediction shots around corners
			if (!BotUsesInstantHitWeapon(bs)) {
				// create the chase goal
				goal.entitynum = bs->client;
				goal.areanum = bs->areanum;

				VectorCopy(bs->eye, goal.origin);
				VectorSet(goal.mins, -8, -8, -8);
				VectorSet(goal.maxs, 8, 8, 8);

				if (trap_BotPredictVisiblePosition(bs->lastenemyorigin, bs->lastenemyareanum, &goal, TFL_DEFAULT, target)) {
					VectorSubtract(target, bs->eye, dir);
					// if the hitpoint is far enough from the bot
					if (VectorLengthSquared(dir) > Square(100)) {
						VectorCopy(target, bestorigin);
						// if the projectile does large radial damage try to aim at the ground in front of the enemy
						if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
							bestorigin[2] -= 20;
						}
					}
					// allow the bot to hit the world
					bs->allowHitWorld = qtrue;
					aim_accuracy = 1.0f;
				}
			}
		}

		VectorCopy(bestorigin, bs->aimtarget);
	// if the enemy is visible
	} else {
		// get the start point shooting from
		// NOTE: the x and y projectile start offsets are ignored
		VectorCopy(bs->origin, start);

		start[2] += bs->cur_ps.viewheight;
		start[2] += wi.offset[2];
		// get the end point aiming at
		VectorCopy(entinfo.origin, bestorigin);
		// try to aim at the head
		if (BotAI_GetClientState(bs->enemy, &ps)) {
			bestorigin[2] += ps.viewheight;
		}

		BotAI_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, mask);
		// if the enemy is NOT hit
		if (trace.fraction < 1.0f && trace.entityNum != entinfo.number) {
			// aim a bit higher
			bestorigin[2] += 8;
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Enemy NOT hit. Aiming higher!\n", netname);
#endif
			BotAI_Trace(&trace, start, mins, maxs, bestorigin, bs->entitynum, mask);
			// if the enemy is still NOT hit
			if (trace.fraction < 1.0f && trace.entityNum != entinfo.number) {
				// aim a bit lower
				bestorigin[2] -= 32;
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Enemy NOT hit. Aiming lower!\n", netname);
#endif
			}
		}
		// if it is not an instant hit weapon the bot might want to predict the enemy
		if (!BotUsesInstantHitWeapon(bs)) {
			// direction towards the enemy
			VectorSubtract(bestorigin, bs->origin, dir);
			// distance towards the enemy
			dist = VectorLength(dir);
			// direction the enemy is moving in
			VectorSubtract(entinfo.origin, bs->enemyorigin, dir);
			// if the enemy is NOT pretty far away and strafing just small steps left and right
			if (!(dist > 100 && VectorLengthSquared(dir) < Square(32))) {
				// if skilled enough and if the weapon is ready to fire, do exact prediction
				if (aim_skill > 0.8 && bs->cur_ps.weaponstate == WEAPON_READY) {
					VectorSubtract(entinfo.origin, bs->origin, dir);
					// distance towards the enemy
					dist = VectorLength(dir);
					// direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);
					VectorScale(dir, 1 / entinfo.update_time, dir);
					VectorCopy(entinfo.origin, origin);

					origin[2] += 1;

					VectorClear(cmdmove);
					//AAS_ClearShownDebugLines();
					// movement prediction
					trap_AAS_PredictClientMovement(&move, bs->enemy, origin, PRESENCE_CROUCH, qfalse, qfalse, dir, cmdmove, 0, dist * 10 / wi.speed, 0.1f, 0, 0, qfalse);
					VectorCopy(move.endpos, bestorigin);
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Predicted speed = %f, Frames = %f.\n", netname, FloatTime(), VectorLength(dir), dist * 10 / wi.speed);
#endif
				// if not that skilled do linear prediction
				} else if (aim_skill > 0.4) {
					// direction towards the enemy
					VectorSubtract(entinfo.origin, bs->origin, dir);
					// distance towards the enemy
					dist = VectorLength(dir);
					// direction the enemy is moving in
					VectorSubtract(entinfo.origin, entinfo.lastvisorigin, dir);

					dir[2] = 0;
					speed = VectorNormalize(dir) / entinfo.update_time;
#ifdef DEBUG
					BotAI_Print(PRT_MESSAGE, "%s: Speed = %f, wi->speed = %f.\n", netname, speed, wi.speed);
#endif
					// best spot to aim at
					VectorMA(entinfo.origin, (dist / wi.speed) * speed, dir, bestorigin);
				}
			}
		}
		// if the projectile does large radial damage
		if (aim_skill > 0.6 && (wi.proj.damagetype & DAMAGETYPE_RADIAL) && wi.proj.radius > 50) {
			// if the enemy isn't standing significantly higher than the bot and isn't in water
			if (entinfo.origin[2] < bs->origin[2] + 16 && !(trap_AAS_PointContents(entinfo.origin) & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA))) {
				// try to aim at the ground in front of the enemy
				VectorCopy(entinfo.origin, end);

				end[2] -= 64;

				BotAI_Trace(&trace, entinfo.origin, mins, maxs, end, entinfo.number, mask);
				VectorCopy(bestorigin, groundtarget); // Tobias CHECK: is 'bestorigin' wrong now (changed above), or was it always strange to use 'bestorigin' instead of 'end' ?

				if (trace.startsolid) {
					groundtarget[2] = entinfo.origin[2] - 16;
				} else {
					groundtarget[2] = trace.endpos[2] - 8;
				}
				// trace a line from projectile start to ground target
				BotAI_Trace(&trace, start, mins, maxs, groundtarget, bs->entitynum, mask);
				// if hitpoint is not vertically too far from the ground target
				if (fabs(trace.endpos[2] - groundtarget[2]) < 50) {
					VectorSubtract(trace.endpos, groundtarget, dir);
					// if the hitpoint is near enough the ground target
					if (VectorLengthSquared(dir) < Square(60)) {
						VectorSubtract(trace.endpos, start, dir);
						// if the hitpoint is far enough from the bot
						if (VectorLengthSquared(dir) > Square(100)) {
							// check if the bot is visible from the ground target
							trace.endpos[2] += 1;

							BotAI_Trace(&trace, trace.endpos, mins, maxs, entinfo.origin, entinfo.number, mask);
							// if the projectile will not be blocked
							if (trace.fraction >= 1.0f) {
#ifdef DEBUG
								BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Aiming at ground.\n", netname, FloatTime());
#endif
								VectorCopy(groundtarget, bestorigin);
								// allow the bot to hit the world (shooting at the ground)
								bs->allowHitWorld = qtrue;
							}
						}
					}
				}
			}
		}
//#ifdef DEBUG
		if (!bot_noaccerror.integer) {
//#endif
			bestorigin[0] += 20 * crandom() * (1 - aim_accuracy);
			bestorigin[1] += 20 * crandom() * (1 - aim_accuracy);
			bestorigin[2] += 10 * crandom() * (1 - aim_accuracy);
//#ifdef DEBUG
		}
//#endif
		BotAI_Trace(&trace, bs->eye, mins, maxs, bestorigin, bs->entitynum, mask);
		VectorCopy(trace.endpos, bs->aimtarget);
	}
// Tobias NOTE: for developers...
/*
		(o = botai trace line)
		(* = projectile travel line)
                                                                                        o  <---- topOfArc[2] trace
                                                                                  o
                                                                            o
                                                                       o                *  <---- grenade arc is always below trace!
                                                                  o   *
                                                             o *
                                                        o*
                                                   o *
                                               o *
                                          o   *
                                     o     *
             trace start ----> o        *
                                      *
                            _______ *
                           |      *
                           |     * |
                           |    *  |
        projectile start --|-> *   |
                           |       |
                           |       |
                           |       |
                           |       |
NOTE: 1. We can't trace a parabola. That's why we trace a bit above the real arc of travel in straight line. If the projectile will hit some overhead ledge, the bot will fire the projectile straight ahead!
NOTE: 2. wi.proj.gravity, was never set in the botfiles, it MUST be set to make this work properly (an alternate would be to completely do all this without the need of weaponinfo). Without modified botfiles everything works as normally.
NOTE: 3. wi.proj.gravity is a simple configurable (pseudo)multiplier, this seems to be the fastest way to compute 100% accurate ballistics (e.g: projectile speed 700 -> gravity 0.3, projectile speed 10000 -> gravity 0.04, etc.).
NOTE: 4. It doesn't really make sense to use DEFAULT_GRAVITY or g_gravity (projectiles are not influenced by g_gravity at all in Q3a).
NOTE: 5. Although my method of computing the ballistics looks like magic, it is the simplest and fastest way I could think of (correct but slow math: -> https://en.wikipedia.org/wiki/Parabola)
         Computing the ballistics this way takes configurable projectile speed, configurable projectile gravity, dynamic enemy height and dynamic enemy distance into account.
NOTE: 6. This code becomes more precise the faster the projectile moves. Grenades are too slow in Q3a! Fix this: average H-Gren.: bolt speed 1300, = ~60 meters. (gravity 0.35)

WARNING 1: Accuracy is nearly 100% even with very fast projectiled weapons (e.g.: speed 20000 etc.), this means bots will always hit their opponents, even with a bow (eventually decrease the bots individual aim_accuracy)
WARNING 2: Bots will also throw grenades through windows even from distance, so be careful!
*/
	if (BotUsesGravityAffectedProjectileWeapon(bs)) {
		// direction towards the enemy
		VectorSubtract(bestorigin, bs->origin, dir);

		enemyHeight = dir[2]; // Tobias CHECK: enemyHeight = dist[2]?
		// distance towards the enemy
		dist = VectorNormalize(dir);
		// get the start point shooting from, for safety sake take overhead ledges into account (so we trace along the highest point of the arc, from start to middle)
		VectorCopy(bs->origin, start);

		start[2] += bs->cur_ps.viewheight + 20;
		// half the distance will be the middle of the projectile arc (highest point the projectile will travel)
		VectorMA(start, dist * 0.5, dir, middleOfArc);
		VectorCopy(middleOfArc, topOfArc);

		topOfArc[2] += (dist * wi.proj.gravity) + (enemyHeight > 0 ? enemyHeight * 0.1 : 0);
		// trace from start to middle, check if the projectile will be blocked by something
		BotAI_Trace(&trace, start, mins, maxs, topOfArc, entinfo.number, mask);
		// if the projectile will not be blocked
		if (trace.fraction >= 1.0f) {
			// get the end point (the projectiles impact point), for safety sake take overhead ledges into account (so we trace along the highest point of the arc, from middle to end)
			VectorCopy(entinfo.origin, end);

			end[2] += 20;
			// trace from middle to end, check if the projectile will be blocked by something
			BotAI_Trace(&trace, topOfArc, mins, maxs, end, entinfo.number, mask);
			// if the projectile will not be blocked
			if (trace.fraction >= 1.0f) {
				// take projectile speed, gravity and enemy height into account
				bestorigin[2] += (dist * dist / wi.speed * wi.proj.gravity) + (enemyHeight > 0 ? enemyHeight * 0.1 : 0);
#ifdef DEBUG
				BotAI_Print(PRT_MESSAGE, "%s: Time = %1.1f Predicted speed = %f, Frames = %f, Enemy height = %f.\n", netname, FloatTime(), VectorLength(dir), dist * 10 / wi.speed, enemyHeight);
#endif
			}
		}
	}
	// get aim direction
	VectorSubtract(bestorigin, bs->eye, dir);
//#ifdef DEBUG
	if (!bot_noaccerror.integer) {
//#endif
		if (BotUsesInstantHitWeapon(bs)) {
			// distance towards the enemy
			dist = VectorLength(dir);

			if (dist > 150) {
				dist = 150;
			}

			f = 0.6f + dist / 150 * 0.4f;
			aim_accuracy *= f;
		}
		// add some random stuff to the aim direction depending on the aim accuracy
		if (aim_accuracy < 0.8f) {
			VectorNormalize(dir);

			for (i = 0; i < 3; i++) {
				dir[i] += 0.3f * crandom() * (1 - aim_accuracy);
			}
		}
//#ifdef DEBUG
	}
//#endif
	// set the ideal view angles
	VectorToAngles(dir, bs->ideal_viewangles);
	// take the weapon spread into account for lower skilled bots
	bs->ideal_viewangles[PITCH] += 6 * wi.vspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[PITCH] = AngleMod(bs->ideal_viewangles[PITCH]);
	bs->ideal_viewangles[YAW] += 6 * wi.hspread * crandom() * (1 - aim_accuracy);
	bs->ideal_viewangles[YAW] = AngleMod(bs->ideal_viewangles[YAW]);
	// if the bots should be really challenging
	viewType = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VIEW_TYPE, 0, 1);

	if (viewType > 0.9
//#ifdef DEBUG
		|| bot_challenge.integer
//#endif
		) {
		// if the bot is really accurate and has the enemy in view for some time
		if (aim_accuracy > 0.9f && bs->enemysight_time < FloatTime() - 1) {
			// set the view angles directly
			if (bs->ideal_viewangles[PITCH] > 180) {
				bs->ideal_viewangles[PITCH] -= 360;
			}

			VectorCopy(bs->ideal_viewangles, bs->viewangles);
			trap_EA_View(bs->client, bs->viewangles);
		}
	}
#ifdef DEBUG
	BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Final aim accuracy: %f.\n", netname, aim_accuracy);
#endif
}
// Tobias END
/*
=======================================================================================================================================
BotMayRadiusDamageTeamMate
=======================================================================================================================================
*/
static qboolean BotMayRadiusDamageTeamMate(bot_state_t *bs, vec3_t origin, float radius) {
	gentity_t *ent;
	int i, e, numListedEntities, entityList[MAX_GENTITIES];
	float teampreservation, dist;
	vec3_t mins, maxs, v;
	team_t team;
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	if (g_gametype.integer < GT_TEAM) {
		return qfalse;
	}

	if (!g_friendlyFire.integer) {
		return qfalse;
	}

	if (radius < 1) {
		radius = 1;
	}

	for (i = 0; i < 3; i++) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox(mins, maxs, entityList, MAX_GENTITIES);
	team = g_entities[bs->entitynum].client->sess.sessionTeam;
	teampreservation = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_TEAMPRESERVATION, 0, 1);

	for (e = 0; e < numListedEntities; e++) {
		// if this is the bot self
		if (e == bs->client) {
			continue;
		}

		ent = &g_entities[entityList[e]];

		if (!ent->takedamage) {
			continue;
		}

		if (!ent->client) {
			continue;
		}

		if (ent->client->sess.sessionTeam != team) {
			continue;
		}

		if (ent->client->ps.stats[STAT_HEALTH] <= 0) {
			continue;
		}
		// find the distance from the edge of the bounding box
		for (i = 0; i < 3; i++) {
			if (origin[i] < ent->r.absmin[i]) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if (origin[i] > ent->r.absmax[i]) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength(v);

		if (dist >= radius * 2 * teampreservation) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Team Damage: ignoring teammate, not inside radius.\n", netname);
#endif
			continue;
#ifdef DEBUG
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Team Damage: NO ATTACK, teammate will be hit!\n", netname);
#endif
		}

		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotCheckAttack

Tobias NOTE: This new version only uses one trace call, instead of two!
			 The trace call using 'bs->aimtarget' was merged into the existing one below.
Tobias FIXME: Due to fixing an old idtech3 bug now 'weaponfov' contradicts 'aim_accuracy' -> the bot isn't firing the gun because of
			  'bs->aimtarget' isn't used anymore.
			  Although this is theoretically correct, it feels wrong :(
			  The goal of the bugfix was to not fire rockets near walls etc., not to don't fire the weapon when aim is not perfect...
			  Either tweak this via aim_accuracy or through a list of weapons that allow imprecision.
			  Anyways this was always a bug, but by default the bug wasn't so likely to happen.

Tobias TODO: Add a quick 'qtrue' check for instant shooting (carrier near base, obelisk splash dmg, low 'attack_accuracy' value etc.).
Tobias TODO: Make use of self/team preservation.
Tobias TODO: Re-enable better use of 'bs->aimtarget' again?
=======================================================================================================================================
*/
qboolean BotCheckAttack(bot_state_t *bs) {
	float attack_accuracy, aim_accuracy, reactiontime, firethrottle, *mins, *maxs;
	int attackentity, fov, weaponfov, weaponrange, mask;
	//float selfpreservation;
	vec3_t forward, right, start, end, targetpoint, dir, angles;
	weaponinfo_t wi;
	bsp_trace_t trace;
	aas_entityinfo_t entinfo;
	static vec3_t rmins = {-4, -4, -4}, rmaxs = {4, 4, 4}; // rockets/missiles
//	static vec3_t bmins = {-6, -6, -6}, bmaxs = {6, 6, 6}; // satchel/dynamite/bombs
//	static vec3_t fmins = {-30, -30, -30}, fmaxs = {30, 30, 30}; // flame chunks
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
	attackentity = bs->enemy;

	if (attackentity < 0) {
		return qfalse;
	}

	if (bs->weaponnum <= WP_NONE || bs->weaponnum >= WP_NUM_WEAPONS) {
		return qfalse;
	}
/*
	// don't attack with the flamethrower if moving faster than normal
	if (bs->weaponnum == WP_FLAMETHROWER && VectorLengthSquared(bs->cur_ps.velocity) > 160000) {
		return qfalse;
	}
*/
	// get the entity information
	BotEntityInfo(attackentity, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return qfalse;
	}
	// if the entity isn't dead
	if (EntityIsDead(&entinfo)) {
		return qfalse;
	}
	// if attacking an obelisk
	if (attackentity >= MAX_CLIENTS && (entinfo.number == redobelisk.entitynum || entinfo.number == blueobelisk.entitynum)) {
		// if the obelisk is respawning
		if (g_entities[entinfo.number].activator && g_entities[entinfo.number].activator->s.frame == 2) {
			return qfalse;
		}
	}

	attack_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_ATTACK_ACCURACY, 0, 1);
	// get the reaction time
	reactiontime = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_REACTIONTIME, 0, 5);
	// if the enemy is invisible
	if (EntityIsInvisible(&entinfo)) {
		reactiontime += 1.5f;
		// limit the reactiontime
		if (reactiontime > 2.5f) {
			reactiontime = 2.5f;
		}
	}
	// consider enemy model specific attributes
	if (BotEqualizeWeakestHumanTeamScore(bs) || BotEqualizeTeamScore(bs)) {
		reactiontime += bot_equalizer_react.value; // DEBUG: bot_equalizer_react
#ifdef DEBUG
		if (BotTeam(bs) == TEAM_RED) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Camouflage skin: EQUALIZE for BLUE! RED score = %i, BLUE score = %i, reactiontime = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, reactiontime);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Camouflage skin: EQUALIZE for RED! BLUE score = %i, RED score = %i, reactiontime = %f.\n", netname, bs->ownteamscore, bs->enemyteamscore, reactiontime);
		}
#endif
	}
	// wait until we have had time to react
	if (bs->enemysight_time > FloatTime() - reactiontime) {
		bs->aimnotperfect_time = FloatTime();
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: No attack: bs->enemysight_time > FloatTime!\n", netname);
#endif
		return qfalse;
	}

	if (bs->teleport_time > FloatTime() - reactiontime) {
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: No attack: bs->teleport_time > FloatTime!\n", netname);
#endif
		return qfalse;
	}

	VectorSubtract(bs->aimtarget, bs->eye, dir);
	// if using a close combat weapon and the enemy is too far away
	if (BotUsesCloseCombatWeapon(bs) && BotWantsToRetreat(bs) && VectorLengthSquared(dir) > Square(60)) {
		return qfalse;
	}
	// if changing weapons
	if (bs->weaponchange_time > FloatTime() - 0.1) {
		return qfalse;
	}
/*
	BotAI_Trace(&bsptrace, bs->eye, mins, maxs, bs->aimtarget, bs->client, mask);

	if (!bs->allowHitWorld && bsptrace.fraction < 1.0f && bsptrace.entityNum != attackentity) {
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: No attack: trace won't hit!\n", netname);
#endif
		return qfalse;
	}
*/
	// check fire throttle characteristic
	if (bs->firethrottlewait_time > FloatTime()) {
		return qfalse;
	}

	firethrottle = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_FIRETHROTTLE, 0, 1);
	// if attacking an obelisk or if the bot wants to retreat and using the grenadelauncher
	if ((bs->enemy >= MAX_CLIENTS && (bs->enemy == redobelisk.entitynum || bs->enemy == blueobelisk.entitynum)) || (bs->weaponnum == WP_GRENADELAUNCHER && BotWantsToRetreat(bs))) {
		firethrottle = 0;
	}

	if (bs->firethrottleshoot_time < FloatTime()) {
		if (random() > firethrottle) {
			bs->firethrottlewait_time = FloatTime() + firethrottle;
			bs->firethrottleshoot_time = 0;
		} else {
			bs->firethrottleshoot_time = FloatTime() + 1 - firethrottle;
			bs->firethrottlewait_time = 0;
		}
	}
	// Tobias NOTE: (FIXME: remove this?) an attack_accuracy of 0.5 restores the old (incorrect) behaviour of bots shooting accidently at walls (they hit the trigger to soon, and they release the trigger too late (-> lot of suicides by rockets near walls etc.)!
	if (attack_accuracy != 0.5) {
		// get some weapon specific attack values
		switch (bs->weaponnum) {
			case WP_GAUNTLET:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
				weaponfov = 90;
				weaponrange = 42;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_MACHINEGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_MACHINEGUN, 0, 1);
				weaponfov = 20;
				weaponrange = 100000;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_CHAINGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_CHAINGUN, 0, 1);
				weaponfov = 80;
				weaponrange = 100000;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_SHOTGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_SHOTGUN, 0, 1);
				weaponfov = 20;
				weaponrange = 500;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_NAILGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_NAILGUN, 0, 1);
				weaponfov = 40; // 30 (pre-aiming?)
				weaponrange = 500;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_PROXLAUNCHER:
			case WP_GRENADELAUNCHER:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_GRENADELAUNCHER, 0, 1);
				weaponfov = 120;
				weaponrange = 2000;
				mins = rmins;
				maxs = rmaxs;
				//mask = MASK_MISSILESHOT;
				mask = MASK_SHOT;
				break;
			case WP_NAPALMLAUNCHER:
			case WP_ROCKETLAUNCHER:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_ROCKETLAUNCHER, 0, 1);
				weaponfov = 60;
				weaponrange = 1000;
				mins = rmins;
				maxs = rmaxs;
				mask = MASK_SHOT;
				break;
			case WP_BEAMGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BEAMGUN, 0, 1);
				weaponfov = 80;
				weaponrange = BEAMGUN_RANGE;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_RAILGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_RAILGUN, 0, 1);
				weaponfov = 20;
				weaponrange = 100000;
				mins = NULL;
				maxs = NULL;
				mask = MASK_SHOT;
				break;
			case WP_PLASMAGUN:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_PLASMAGUN, 0, 1);
				weaponfov = 20;
				weaponrange = 1000;
				mins = rmins;
				maxs = rmaxs;
				mask = MASK_SHOT;
				break;
			case WP_BFG:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY_BFG10K, 0, 1);
				weaponfov = 20;
				weaponrange = 1000;
				mins = rmins;
				maxs = rmaxs;
				mask = MASK_SHOT;
				break;
			default:
				aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);
				weaponfov = 50;
				weaponrange = 1000;
				mins = rmins;
				maxs = rmaxs;
				mask = MASK_SHOT;
				break;
		}
		// Tobias FIXME(?): some bots really have a very bad aim accuracy, in this case the bots view shakes so the enemy is outside their fov(!), extend the fov in this case, otherwise bots won't hit the trigger.
		weaponfov += 30 - (30 * aim_accuracy);
	// simulate old style Q3A Gladiator bot behaviour
	} else {
		weaponfov = 50;
		weaponrange = 1000;
		mins = NULL;
		maxs = NULL;
		mask = MASK_SHOT;
	}

	if (VectorLengthSquared(dir) < Square(100)) { // Tobias NOTE: hmm, I still don't see a reason for this (keep it for spin-up weapons)?
		fov = 120;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: Dist < 100, FOV: %i.\n", netname, fov);
#endif
	} else {
		fov = weaponfov;
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Dist > 100, FOV: %i.\n", netname, fov);
#endif
	}

	VectorToAngles(dir, angles);

	if (attack_accuracy > 0.6) {
		// some weapons don't accept inprecision (could be dangerous for ourself etc.)
		if (!InFieldOfVision(bs->viewangles, 0.5 * fov, angles)) {
			bs->aimnotperfect_time = FloatTime();
		}
		// so don't shoot too early with those weapons
		if (bs->weaponnum == WP_RAILGUN && (bs->aimnotperfect_time > FloatTime() - 0.1)) {
#ifdef DEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: (> 0.6) No attack: aimnotperfect_time!\n", netname);
#endif
			return qfalse;
		}
	}

	if (!InFieldOfVision(bs->viewangles, fov, angles)) {
#ifdef DEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: No attack: not in fov!\n", netname);
#endif
		return qfalse;
	}
	// get the start point shooting from
	VectorCopy(bs->origin, start);

	start[2] += bs->cur_ps.viewheight;

	AngleVectorsForwardRight(bs->viewangles, forward, right);
	// get the weapon info
	trap_BotGetWeaponInfo(bs->ws, bs->weaponnum, &wi);

	start[0] += forward[0] * wi.offset[0] + right[0] * wi.offset[1];
	start[1] += forward[1] * wi.offset[0] + right[1] * wi.offset[1];
	start[2] += forward[2] * wi.offset[0] + right[2] * wi.offset[1] + wi.offset[2];
	// a little back to make sure not inside a very close enemy
	VectorMA(start, -8, forward, start);
	// end point aiming at
	VectorMA(start, weaponrange, forward, end); // Tobias NOTE: 262144 (default Railgun range see g_weapon.c) does NOT work with the (unmodified/default) broken code for radial damage projectiles from below!
	// Tobias NOTE: (FIXME: remove this?) an attack_accuracy of 0.5 restores the old (incorrect) behaviour of bots shooting accidently at walls (they hit the trigger to soon, and they release the trigger too late (-> lot of suicides by rockets near walls etc.)!
	if (attack_accuracy > 0.5) {
		VectorCopy(end, targetpoint); // Tobias FIXME(?): some bots really have a very bad aim accuracy, in this case the bots view shakes so the enemy is outside their fov(!), extend the fov in this case, otherwise bots won't hit the trigger.
	// simulate old style Q3A Gladiator bot behaviour
	} else {
		VectorCopy(bs->aimtarget, targetpoint);
	}

	BotAI_Trace(&trace, start, mins, maxs, targetpoint, bs->entitynum, mask);

	if (!bs->allowHitWorld && trace.fraction < 1.0f && trace.entityNum != attackentity) {
#ifdef DEBUG
		if (attack_accuracy != 0.5) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: (> 0.5 Fixed) No attack: trace won't hit!\n", netname);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: (== 0.5 Default) No attack: trace won't hit!\n", netname);
		}
#endif
		return qfalse;
	}
	// if the entity is a client
	if (trace.entityNum >= 0 && trace.entityNum < MAX_CLIENTS) {
		if (trace.entityNum != attackentity) { // Tobias CHECK: isn't this already checke above? Remove this twice?
			// if a teammate is hit
			if (BotSameTeam(bs, trace.entityNum)) {
				return qfalse;
			}
		}
	}
/*
	// if won't hit the enemy or not attacking a player (obelisk)
	if (trace.entityNum != attackentity || attackentity >= MAX_CLIENTS) {
		// if the projectile does radial damage
		if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
			if (trace.fraction * 1000 < wi.proj.radius) {
				points = (wi.proj.damage - 0.5 * trace.fraction * 1000) * 0.5;

				if (points > 0) {
					return qfalse;
				}
			}
			// FIXME: check if a teammate gets radial damage
		}
	}
*/
	// check if a teammate gets radial damage
	if (wi.proj.damagetype & DAMAGETYPE_RADIAL) {
		if (BotMayRadiusDamageTeamMate(bs, trace.endpos, wi.proj.radius)) {
			return qfalse;
		}
	}
// Tobias DEBUG
	if (bot_noshoot.integer) {
		return qfalse;
	}
#ifdef DEBUG
	BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Final Reactiontime = %f.\n", netname, reactiontime);
#endif
// DEBUG
	// if fire has to be release to activate weapon
	if (wi.flags & WFL_FIRERELEASED) {
		if (bs->flags & BFL_ATTACKED) {
			trap_EA_Attack(bs->client);
		}
	} else {
		trap_EA_Attack(bs->client);
	}

	bs->flags ^= BFL_ATTACKED;

	return qfalse;
}

/*
=======================================================================================================================================
BotMapScripts
=======================================================================================================================================
*/
void BotMapScripts(bot_state_t *bs) {
	char info[1024];
	char mapname[128];
	int i, shootbutton;
	float aim_accuracy;
	aas_entityinfo_t entinfo;
	vec3_t dir;

	trap_GetServerinfo(info, sizeof(info));
	strncpy(mapname, Info_ValueForKey(info, "mapname"), sizeof(mapname) - 1);

	mapname[sizeof(mapname) - 1] = '\0';

	if (!Q_stricmp(mapname, "q3tourney6") || !Q_stricmp(mapname, "q3tourney6_ctf") || !Q_stricmp(mapname, "mpq3tourney6")) {
		vec3_t mins = {694, 200, 480}, maxs = {968, 472, 680};
		vec3_t buttonorg = {304, 352, 920};
		// NOTE: NEVER use the func_bobbing in q3tourney6
		bs->tfl &= ~TFL_FUNCBOB;
		// crush area is higher in mpq3tourney6
		if (!Q_stricmp(mapname, "mpq3tourney6")) {
			mins[2] += 64;
			maxs[2] += 64;
		}
		// if the bot is in the bounding box of the crush area
		if (bs->origin[0] > mins[0] && bs->origin[0] < maxs[0]) {
			if (bs->origin[1] > mins[1] && bs->origin[1] < maxs[1]) {
				if (bs->origin[2] > mins[2] && bs->origin[2] < maxs[2]) {
					return;
				}
			}
		}

		shootbutton = qfalse;
		// if an enemy is in the bounding box then shoot the button
		for (i = 0; i < level.maxclients; i++) {
			if (i == bs->client) {
				continue;
			}
			// get the entity information
			BotEntityInfo(i, &entinfo);
			// if the entity information is valid
			if (!entinfo.valid) {
				//BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "BotMapScripts -> !entinfo.valid\n"); // Tobias CHECK: shouldn't happen?
				continue;
			}
			// if the entity isn't the bot self
			if (entinfo.number == bs->entitynum) {
				continue;
			}
			// if the entity isn't dead
			if (EntityIsDead(&entinfo)) {
				continue;
			}

			if (entinfo.origin[0] > mins[0] && entinfo.origin[0] < maxs[0]) {
				if (entinfo.origin[1] > mins[1] && entinfo.origin[1] < maxs[1]) {
					if (entinfo.origin[2] > mins[2] && entinfo.origin[2] < maxs[2]) {
						// if there's a teammate below the crusher
						if (BotSameTeam(bs, i)) {
							shootbutton = qfalse;
							break;
						} else if (bs->enemy == i) {
							shootbutton = qtrue;
						}
					}
				}
			}
		}

		if (shootbutton) {
			bs->flags |= BFL_IDEALVIEWSET;

			VectorSubtract(buttonorg, bs->eye, dir);
			VectorToAngles(dir, bs->ideal_viewangles);

			aim_accuracy = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_AIM_ACCURACY, 0, 1);

			bs->ideal_viewangles[PITCH] += 8 * crandom() * (1 - aim_accuracy);
			bs->ideal_viewangles[PITCH] = AngleMod(bs->ideal_viewangles[PITCH]);
			bs->ideal_viewangles[YAW] += 8 * crandom() * (1 - aim_accuracy);
			bs->ideal_viewangles[YAW] = AngleMod(bs->ideal_viewangles[YAW]);

			if (InFieldOfVision(bs->viewangles, 20, bs->ideal_viewangles)) {
				trap_EA_Attack(bs->client);
			}
		}
	} else if (!Q_stricmp(mapname, "mpq3tourney6")) {
		// NOTE: NEVER use the func_bobbing in mpq3tourney6
		// or avoid bots from jumping (during attack) on platforms moving along the x or y axis (-> the bot will fall into the void)
		// https://github.com/zturtleman/spearmint/issues/247
		bs->tfl &= ~TFL_FUNCBOB;
	}
}

/*
=======================================================================================================================================
BotModelMinsMaxs

This is ugly.
=======================================================================================================================================
*/
int BotModelMinsMaxs(int modelindex, int eType, int contents, vec3_t mins, vec3_t maxs) {
	gentity_t *ent;
	int i;

	ent = &g_entities[0];

	for (i = 0; i < level.num_entities; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}

		if (eType && ent->s.eType != eType) {
			continue;
		}

		if (contents && ent->r.contents != contents) {
			continue;
		}

		if (ent->s.modelindex == modelindex) {
			if (mins) {
				VectorAdd(ent->r.currentOrigin, ent->r.mins, mins);
			}

			if (maxs) {
				VectorAdd(ent->r.currentOrigin, ent->r.maxs, maxs);
			}

			return i;
		}
	}

	if (mins) {
		VectorClear(mins);
	}

	if (maxs) {
		VectorClear(maxs);
	}

	return 0;
}

/*
=======================================================================================================================================
BotFuncButtonActivateGoal
=======================================================================================================================================
*/
int BotFuncButtonActivateGoal(bot_state_t *bs, int bspent, bot_activategoal_t *activategoal) {
	int i, areas[10], numareas, modelindex, entitynum;
	char model[128];
	float lip, dist, health, angle;
	vec3_t size, start, end, mins, maxs, angles, points[10];
	vec3_t movedir, origin, goalorigin, bboxmins, bboxmaxs;
	vec3_t extramins = {1, 1, 1}, extramaxs = {-1, -1, -1};
	bsp_trace_t bsptrace;

	activategoal->shoot = qfalse;

	VectorClear(activategoal->target);
	// create a bot goal towards the button
	trap_AAS_ValueForBSPEpairKey(bspent, "model", model, sizeof(model));

	if (!*model) {
		return qfalse;
	}

	modelindex = atoi(model + 1);

	if (!modelindex) {
		return qfalse;
	}

	entitynum = BotModelMinsMaxs(modelindex, ET_MOVER, 0, mins, maxs);
	// get the lip of the button
	trap_AAS_FloatForBSPEpairKey(bspent, "lip", &lip);

	if (!lip) {
		lip = 4;
	}
	// get the move direction from the angle
	trap_AAS_FloatForBSPEpairKey(bspent, "angle", &angle);
	VectorSet(angles, 0, angle, 0);
	SetMovedir(angles, movedir);
	// button size
	VectorSubtract(maxs, mins, size);
	// button origin
	VectorAdd(mins, maxs, origin);
	VectorScale(origin, 0.5, origin);
	// touch distance of the button
	dist = fabs(movedir[0]) * size[0] + fabs(movedir[1]) * size[1] + fabs(movedir[2]) * size[2] - lip;

	trap_AAS_FloatForBSPEpairKey(bspent, "health", &health);
	// if the button is shootable
	if (health) {
		// calculate the shoot target
		VectorMA(origin, -dist, movedir, goalorigin);
		VectorCopy(goalorigin, activategoal->target);

		activategoal->shoot = qtrue;

		BotAI_Trace(&bsptrace, bs->eye, NULL, NULL, goalorigin, bs->entitynum, MASK_SHOT);
		// if the button is visible from the current position
		if (bsptrace.fraction >= 1.0f || bsptrace.entityNum == entitynum) {
			activategoal->goal.entitynum = entitynum; // NOTE: this is the entity number of the shootable button
			activategoal->goal.number = 0;
			activategoal->goal.flags = 0;
			VectorCopy(bs->origin, activategoal->goal.origin);
			activategoal->goal.areanum = bs->areanum;
			VectorSet(activategoal->goal.mins, -8, -8, -8);
			VectorSet(activategoal->goal.maxs, 8, 8, 8);
			return qtrue;
		} else {
			// create a goal from where the button is visible and shoot at the button from there
			// add bounding box size to the dist
			trap_AAS_PresenceTypeBoundingBox(PRESENCE_CROUCH, bboxmins, bboxmaxs);

			for (i = 0; i < 3; i++) {
				if (movedir[i] < 0) {
					dist += fabs(movedir[i]) * fabs(bboxmaxs[i]);
				} else {
					dist += fabs(movedir[i]) * fabs(bboxmins[i]);
				}
			}
			// calculate the goal origin
			VectorMA(origin, -dist, movedir, goalorigin);
			VectorCopy(goalorigin, start);

			start[2] += 24;

			VectorCopy(start, end);

			end[2] -= 512;
			numareas = trap_AAS_TraceAreas(start, end, areas, points, 10);

			for (i = numareas - 1; i >= 0; i--) {
				if (trap_AAS_AreaReachability(areas[i])) {
					break;
				}
			}

			if (i < 0) {
				// FIXME: trace forward and maybe in other directions to find a valid area
			}

			if (i >= 0) {
				VectorCopy(points[i], activategoal->goal.origin);
				activategoal->goal.areanum = areas[i];
				VectorSet(activategoal->goal.mins, 8, 8, 8);
				VectorSet(activategoal->goal.maxs, -8, -8, -8);

				for (i = 0; i < 3; i++) {
					if (movedir[i] < 0) {
						activategoal->goal.maxs[i] += fabs(movedir[i]) * fabs(extramaxs[i]);
					} else {
						activategoal->goal.mins[i] += fabs(movedir[i]) * fabs(extramins[i]);
					}
				}

				activategoal->goal.entitynum = entitynum;
				activategoal->goal.number = 0;
				activategoal->goal.flags = 0;
				return qtrue;
			}
		}

		return qfalse;
	} else {
		// add bounding box size to the dist
		trap_AAS_PresenceTypeBoundingBox(PRESENCE_CROUCH, bboxmins, bboxmaxs);

		for (i = 0; i < 3; i++) {
			if (movedir[i] < 0) {
				dist += fabs(movedir[i]) * fabs(bboxmaxs[i]);
			} else {
				dist += fabs(movedir[i]) * fabs(bboxmins[i]);
			}
		}
		// calculate the goal origin
		VectorMA(origin, -dist, movedir, goalorigin);
		VectorCopy(goalorigin, start);

		start[2] += 24;

		VectorCopy(start, end);

		end[2] -= 100;
		numareas = trap_AAS_TraceAreas(start, end, areas, NULL, 10);

		for (i = 0; i < numareas; i++) {
			if (trap_AAS_AreaReachability(areas[i])) {
				break;
			}
		}

		if (i < numareas) {
			VectorCopy(origin, activategoal->goal.origin);
			activategoal->goal.areanum = areas[i];
			VectorSubtract(mins, origin, activategoal->goal.mins);
			VectorSubtract(maxs, origin, activategoal->goal.maxs);

			for (i = 0; i < 3; i++) {
				if (movedir[i] < 0) {
					activategoal->goal.maxs[i] += fabs(movedir[i]) * fabs(extramaxs[i]);
				} else {
					activategoal->goal.mins[i] += fabs(movedir[i]) * fabs(extramins[i]);
				}
			}

			activategoal->goal.entitynum = entitynum;
			activategoal->goal.number = 0;
			activategoal->goal.flags = 0;
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotFuncDoorActivateGoal
=======================================================================================================================================
*/
int BotFuncDoorActivateGoal(bot_state_t *bs, int bspent, bot_activategoal_t *activategoal) {
	int modelindex, entitynum;
	char model[MAX_INFO_STRING];
	vec3_t mins, maxs, origin;

	// shoot at the shootable door
	trap_AAS_ValueForBSPEpairKey(bspent, "model", model, sizeof(model));

	if (!*model) {
		return qfalse;
	}

	modelindex = atoi(model + 1);

	if (!modelindex) {
		return qfalse;
	}

	entitynum = BotModelMinsMaxs(modelindex, ET_MOVER, 0, mins, maxs);
	// door origin
	VectorAdd(mins, maxs, origin);
	VectorScale(origin, 0.5, origin);
	VectorCopy(origin, activategoal->target);

	activategoal->shoot = qtrue;
	activategoal->goal.entitynum = entitynum; // NOTE: this is the entity number of the shootable door
	activategoal->goal.number = 0;
	activategoal->goal.flags = 0;

	VectorCopy(bs->origin, activategoal->goal.origin);

	activategoal->goal.areanum = bs->areanum;

	VectorSet(activategoal->goal.mins, -8, -8, -8);
	VectorSet(activategoal->goal.maxs, 8, 8, 8);
	return qtrue;
}

/*
=======================================================================================================================================
BotTriggerMultipleActivateGoal
=======================================================================================================================================
*/
int BotTriggerMultipleActivateGoal(bot_state_t *bs, int bspent, bot_activategoal_t *activategoal) {
	int i, areas[10], numareas, modelindex, entitynum;
	char model[128];
	vec3_t start, end, mins, maxs;
	vec3_t origin, goalorigin;

	activategoal->shoot = qfalse;

	VectorClear(activategoal->target);
	// create a bot goal towards the trigger
	trap_AAS_ValueForBSPEpairKey(bspent, "model", model, sizeof(model));

	if (!*model) {
		return qfalse;
	}

	modelindex = atoi(model + 1);

	if (!modelindex) {
		return qfalse;
	}

	entitynum = BotModelMinsMaxs(modelindex, 0, CONTENTS_TRIGGER, mins, maxs);
	// trigger origin
	VectorAdd(mins, maxs, origin);
	VectorScale(origin, 0.5, origin);
	VectorCopy(origin, goalorigin);
	VectorCopy(goalorigin, start);

	start[2] += 24;

	VectorCopy(start, end);

	end[2] -= 100;
	numareas = trap_AAS_TraceAreas(start, end, areas, NULL, 10);

	for (i = 0; i < numareas; i++) {
		if (trap_AAS_AreaReachability(areas[i])) {
			break;
		}
	}

	if (i < numareas) {
		VectorCopy(origin, activategoal->goal.origin);
		activategoal->goal.areanum = areas[i];
		VectorSubtract(mins, origin, activategoal->goal.mins);
		VectorSubtract(maxs, origin, activategoal->goal.maxs);

		activategoal->goal.entitynum = entitynum;
		activategoal->goal.number = 0;
		activategoal->goal.flags = 0;
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotPopFromActivateGoalStack
=======================================================================================================================================
*/
int BotPopFromActivateGoalStack(bot_state_t *bs) {

	if (!bs->activatestack) {
		return qfalse;
	}

	BotEnableActivateGoalAreas(bs->activatestack, qtrue);

	bs->activatestack->inuse = qfalse;
	bs->activatestack->justused_time = FloatTime();
	bs->activatestack = bs->activatestack->next;
	return qtrue;
}

/*
=======================================================================================================================================
BotPushOntoActivateGoalStack
=======================================================================================================================================
*/
int BotPushOntoActivateGoalStack(bot_state_t *bs, bot_activategoal_t *activategoal) {
	int i, best;
	float besttime;

	best = -1;
	besttime = FloatTime() + 9999;

	for (i = 0; i < MAX_ACTIVATESTACK; i++) {
		if (!bs->activategoalheap[i].inuse) {
			if (bs->activategoalheap[i].justused_time < besttime) {
				besttime = bs->activategoalheap[i].justused_time;
				best = i;
			}
		}
	}

	if (best != -1) {
		memcpy(&bs->activategoalheap[best], activategoal, sizeof(bot_activategoal_t));

		bs->activategoalheap[best].inuse = qtrue;
		bs->activategoalheap[best].next = bs->activatestack;
		bs->activatestack = &bs->activategoalheap[best];
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotClearActivateGoalStack
=======================================================================================================================================
*/
void BotClearActivateGoalStack(bot_state_t *bs) {

	while (bs->activatestack) {
		BotPopFromActivateGoalStack(bs);
	}
}

/*
=======================================================================================================================================
BotEnableActivateGoalAreas
=======================================================================================================================================
*/
void BotEnableActivateGoalAreas(bot_activategoal_t *activategoal, int enable) {
	int i;

	if (activategoal->areasdisabled == !enable) {
		return;
	}

	for (i = 0; i < activategoal->numareas; i++) {
		trap_AAS_EnableRoutingArea(activategoal->areas[i], enable);
	}

	activategoal->areasdisabled = !enable;
}

/*
=======================================================================================================================================
BotIsGoingToActivateEntity
=======================================================================================================================================
*/
int BotIsGoingToActivateEntity(bot_state_t *bs, int entitynum) {
	bot_activategoal_t *a;
	int i;

	for (a = bs->activatestack; a; a = a->next) {
		if (a->time < FloatTime()) {
			continue;
		}

		if (a->goal.entitynum == entitynum) {
			return qtrue;
		}
	}

	for (i = 0; i < MAX_ACTIVATESTACK; i++) {
		if (bs->activategoalheap[i].inuse) {
			continue;
		}

		if (bs->activategoalheap[i].goal.entitynum == entitynum) {
			// if the bot went for this goal less than 2 seconds ago
			if (bs->activategoalheap[i].justused_time > FloatTime() - 2) {
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotGetActivateGoal

Returns the number of the bsp entity to activate. 'goal->entitynum' will be set to the game entity to activate.
=======================================================================================================================================
*/
int BotGetActivateGoal(bot_state_t *bs, int entitynum, bot_activategoal_t *activategoal) {
	int i, ent, cur_entities[10], spawnflags, modelindex, areas[MAX_ACTIVATEAREAS * 2], numareas, t;
	char model[MAX_INFO_STRING], tmpmodel[128];
	char target[128], classname[128];
	float health;
	char targetname[10][128];
	aas_entityinfo_t entinfo;
	aas_areainfo_t areainfo;
	vec3_t origin, absmins, absmaxs;

	memset(activategoal, 0, sizeof(bot_activategoal_t));
	// get the entity information
	BotEntityInfo(entitynum, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return 0;
	}

	Com_sprintf(model, sizeof(model), "*%d", entinfo.modelindex);

	for (ent = trap_AAS_NextBSPEntity(0); ent; ent = trap_AAS_NextBSPEntity(ent)) {
		if (!trap_AAS_ValueForBSPEpairKey(ent, "model", tmpmodel, sizeof(tmpmodel))) {
			continue;
		}

		if (!strcmp(model, tmpmodel)) {
			break;
		}
	}

	if (!ent) {
		BotAI_Print(PRT_ERROR, "BotGetActivateGoal: no entity found with model %s\n", model);
		return 0;
	}

	trap_AAS_ValueForBSPEpairKey(ent, "classname", classname, sizeof(classname));

	if (!*classname) {
		BotAI_Print(PRT_ERROR, "BotGetActivateGoal: entity with model %s has no classname\n", model);
		return 0;
	}
	// if it is a door
	if (!strcmp(classname, "func_door")) {
		if (trap_AAS_FloatForBSPEpairKey(ent, "health", &health)) {
			// if the door has health then the door must be shot to open
			if (health) {
				BotFuncDoorActivateGoal(bs, ent, activategoal);
				return ent;
			}
		}

		trap_AAS_IntForBSPEpairKey(ent, "spawnflags", &spawnflags);
		// if the door starts open then just wait for the door to return
		if (spawnflags & 1) {
			return 0;
		}
		// get the door origin
		if (!trap_AAS_VectorForBSPEpairKey(ent, "origin", origin)) {
			VectorClear(origin);
		}
		// if the door is open or opening already
		if (!VectorCompare(origin, entinfo.origin)) {
			return 0;
		}
		// store all the areas the door is in
		trap_AAS_ValueForBSPEpairKey(ent, "model", model, sizeof(model));

		if (*model) {
			modelindex = atoi(model + 1);

			if (modelindex) {
				BotModelMinsMaxs(modelindex, ET_MOVER, 0, absmins, absmaxs);

				numareas = trap_AAS_BBoxAreas(absmins, absmaxs, areas, MAX_ACTIVATEAREAS * 2);
				// store the areas with reachabilities first
				for (i = 0; i < numareas; i++) {
					if (activategoal->numareas >= MAX_ACTIVATEAREAS) {
						break;
					}

					if (!trap_AAS_AreaReachability(areas[i])) {
						continue;
					}

					trap_AAS_AreaInfo(areas[i], &areainfo);

					if (areainfo.contents & AREACONTENTS_MOVER) {
						activategoal->areas[activategoal->numareas++] = areas[i];
					}
				}
				// store any remaining areas
				for (i = 0; i < numareas; i++) {
					if (activategoal->numareas >= MAX_ACTIVATEAREAS) {
						break;
					}

					if (trap_AAS_AreaReachability(areas[i])) {
						continue;
					}

					trap_AAS_AreaInfo(areas[i], &areainfo);

					if (areainfo.contents & AREACONTENTS_MOVER) {
						activategoal->areas[activategoal->numareas++] = areas[i];
					}
				}
			}
		}
	}
	// if the bot is blocked by or standing on top of a button
	if (!strcmp(classname, "func_button")) {
		return 0;
	}
	// get the targetname so we can find an entity with a matching target
	if (!trap_AAS_ValueForBSPEpairKey(ent, "targetname", targetname[0], sizeof(targetname[0]))) {
		if (bot_developer.integer) {
			BotAI_Print(PRT_ERROR, "BotGetActivateGoal: entity with model \"%s\" has no targetname\n", model);
		}

		return 0;
	}
	// allow tree-like activation
	cur_entities[0] = trap_AAS_NextBSPEntity(0);

	for (i = 0; i >= 0 && i < 10;) {
		for (ent = cur_entities[i]; ent; ent = trap_AAS_NextBSPEntity(ent)) {
			if (!trap_AAS_ValueForBSPEpairKey(ent, "target", target, sizeof(target))) {
				continue;
			}

			if (!strcmp(targetname[i], target)) {
				cur_entities[i] = trap_AAS_NextBSPEntity(ent);
				break;
			}
		}

		if (!ent) {
			if (bot_developer.integer) {
				BotAI_Print(PRT_ERROR, "BotGetActivateGoal: no entity with target \"%s\"\n", targetname[i]);
			}

			i--;
			continue;
		}

		if (!trap_AAS_ValueForBSPEpairKey(ent, "classname", classname, sizeof(classname))) {
			if (bot_developer.integer) {
				BotAI_Print(PRT_ERROR, "BotGetActivateGoal: entity with target \"%s\" has no classname\n", targetname[i]);
			}

			continue;
		}
		// BSP button model
		if (!strcmp(classname, "func_button")) {
			if (!BotFuncButtonActivateGoal(bs, ent, activategoal)) {
				continue;
			}
			// if the bot tries to activate this button already
			if (bs->activatestack && bs->activatestack->inuse && bs->activatestack->goal.entitynum == activategoal->goal.entitynum && bs->activatestack->time > FloatTime() && bs->activatestack->start_time < FloatTime() - 2) {
				continue;
			}
			// if the bot is in a reachability area
			if (trap_AAS_AreaReachability(bs->areanum)) {
				// disable all areas the blocking entity is in
				BotEnableActivateGoalAreas(activategoal, qfalse);

				t = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, activategoal->goal.areanum, bs->tfl);
				// if the button is not reachable
				if (!t) {
					continue;
				}

				activategoal->time = FloatTime() + t * 0.01 + 5;
			}

			return ent;
		// invisible trigger multiple box
		} else if (!strcmp(classname, "trigger_multiple")) {
			if (!BotTriggerMultipleActivateGoal(bs, ent, activategoal)) {
				continue;
			}
			// if the bot tries to activate this trigger already
			if (bs->activatestack && bs->activatestack->inuse && bs->activatestack->goal.entitynum == activategoal->goal.entitynum && bs->activatestack->time > FloatTime() && bs->activatestack->start_time < FloatTime() - 2) {
				continue;
			}
			// if the bot is in a reachability area
			if (trap_AAS_AreaReachability(bs->areanum)) {
				// disable all areas the blocking entity is in
				BotEnableActivateGoalAreas(activategoal, qfalse);

				t = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, activategoal->goal.areanum, bs->tfl);
				// if the trigger is not reachable
				if (!t) {
					continue;
				}

				activategoal->time = FloatTime() + t * 0.01 + 5;
			}

			return ent;
		} else if (!strcmp(classname, "func_timer")) {
			// just skip the func_timer
			continue;
		// the actual button or trigger might be linked through a target_relay or target_delay
		} else if (!strcmp(classname, "target_relay") || !strcmp(classname, "target_delay")) {
			if (trap_AAS_ValueForBSPEpairKey(ent, "targetname", targetname[i + 1], sizeof(targetname[0]))) {
				i++;
				cur_entities[i] = trap_AAS_NextBSPEntity(0);
			}
		}
	}
#ifdef OBSTACLEDEBUG
	BotAI_Print(PRT_ERROR, "BotGetActivateGoal: no valid activator for entity with target \"%s\"\n", targetname[0]);
#endif
	return 0;
}

/*
=======================================================================================================================================
BotGoForActivateGoal
=======================================================================================================================================
*/
int BotGoForActivateGoal(bot_state_t *bs, bot_activategoal_t *activategoal, bot_aienter_t aienter) {
	aas_entityinfo_t activateinfo;

	assert(aienter);

	activategoal->inuse = qtrue;

	if (!activategoal->time) {
		activategoal->time = FloatTime() + 10;
	}

	activategoal->start_time = FloatTime();
	activategoal->aienter = aienter;
	// get the entity information
	BotEntityInfo(activategoal->goal.entitynum, &activateinfo);
	// if the entity information is valid
	if (!activateinfo.valid) {
		AIEnter_Seek_LTG(bs, "BotGoForActivateGoal: goal ent invalid");
		return qtrue;
	}

	VectorCopy(activateinfo.origin, activategoal->origin);

	if (BotPushOntoActivateGoalStack(bs, activategoal)) {
		// enter the activate entity AI node
		AIEnter_Seek_ActivateEntity(bs, "BotGoForActivateGoal: entering activate ent.");
		return qtrue;
	} else {
		// enable any routing areas that were disabled
		BotEnableActivateGoalAreas(activategoal, qtrue);
		return qfalse;
	}
}

/*
=======================================================================================================================================
BotPrintActivateGoalInfo
=======================================================================================================================================
*/
void BotPrintActivateGoalInfo(bot_state_t *bs, bot_activategoal_t *activategoal, int bspent) {
	char netname[MAX_NETNAME];
	char classname[128];
	char buf[128];

	ClientName(bs->client, netname, sizeof(netname));
	trap_AAS_ValueForBSPEpairKey(bspent, "classname", classname, sizeof(classname));

	if (activategoal->shoot) {
		Com_sprintf(buf, sizeof(buf), "%s: I have to shoot at a %s from %1.1f %1.1f %1.1f in area %d\n", netname, classname, activategoal->goal.origin[0], activategoal->goal.origin[1], activategoal->goal.origin[2], activategoal->goal.areanum);
	} else {
		Com_sprintf(buf, sizeof(buf), "%s: I have to activate a %s at %1.1f %1.1f %1.1f in area %d\n", netname, classname, activategoal->goal.origin[0], activategoal->goal.origin[1], activategoal->goal.origin[2], activategoal->goal.areanum);
	}

	trap_EA_Say(bs->client, buf);
}

/*
=======================================================================================================================================
BotRandomMove
=======================================================================================================================================
*/
void BotRandomMove(bot_state_t *bs, bot_moveresult_t *moveresult, int speed, int movetype) {
	vec3_t dir, angles;
	int i;

	angles[0] = 0;
	angles[1] = random() * 360;
	angles[2] = 0;

	for (i = 0; i < 8; i++) {
		AngleVectorsForward(angles, dir);

		if (trap_BotMoveInDirection(bs->ms, dir, speed, movetype)) {
			break;
		}

		angles[1] = ((int)angles[1] + 45) % 360;
	}

	moveresult->failure = (i == 8);

	VectorCopy(dir, moveresult->movedir);
}

/*
=======================================================================================================================================
BotObstacleAvoidanceMoveExt
=======================================================================================================================================
*/
void BotObstacleAvoidanceMoveExt(bot_state_t *bs, bot_moveresult_t *moveresult, int speed, int movetype) {
#ifdef OBSTACLEDEBUG
	char netname[MAX_NETNAME];
#endif
// Tobias DEBUG
	int obstacleMove;
// Tobias END
	vec3_t dir2, mins, maxs, hordir, sideward, rightwards, leftwards, angles, up = {0, 0, 1};
	gentity_t *ent;
	bsp_trace_t trace;
// Tobias DEBUG
	obstacleMove = MOVEMENT_DEFAULT;
// Tobias END
#ifdef OBSTACLEDEBUG
	ClientName(bs->client, netname, sizeof(netname));
#endif
	// just some basic dynamic obstacle avoidance code
	hordir[0] = moveresult->movedir[0];
	hordir[1] = moveresult->movedir[1];
	hordir[2] = 0;
	// if no direction just take a random direction
	if (VectorNormalize(hordir) < 0.1) {
		VectorSet(angles, 0, 360 * random(), 0);
		AngleVectorsForward(angles, hordir);
	}
	// try to crouch or jump over barrier
	if (!trap_BotMoveInDirection(bs->ms, hordir, speed, movetype)) {
		// get the (right) sideward vector
		CrossProduct(hordir, up, sideward);
		// get the direction the blocking obstacle is moving
		ent = &g_entities[moveresult->blockentity];
		dir2[2] = 0;

		VectorCopy(ent->client->ps.velocity, dir2);
		// if the blocking obstacle is moving
		if (VectorLengthSquared(dir2) > 0) {
			// we start moving to our right side, but if the blocking entity is also moving towards our right side flip the direction and move to the left side
			if (DotProduct(dir2, sideward) > 50.0f) {
				// flip the direction
				VectorNegate(sideward, sideward);
// Tobias DEBUG
				obstacleMove = MOVEMENT_BARRIER_WALK_LEFT;
// Tobias END
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Forward movement failed. Flipped right side because dir2 = %1.1f.\n", netname, DotProduct(dir2, sideward));
#endif
			} else {
// Tobias DEBUG
				obstacleMove = MOVEMENT_BARRIER_WALK_RIGHT;
// Tobias END
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Forward movement failed. Keep right side.\n", netname);
#endif
			}
			// move sidwards
			if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
				// flip the direction
				VectorNegate(sideward, sideward);
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: 1st sidewards movement failed, flipped direction.\n", netname);
#endif
				// move in the other direction
				if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
					// move in a random direction in the hope to get out
					BotRandomMove(bs, moveresult, speed, movetype);
// Tobias DEBUG
					obstacleMove = MOVEMENT_RANDOM;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: 2nd sidewards movement failed, ending up using random move.\n", netname);
#endif
				}
			}
		// if the blocking obstacle is not moving at all
		} else {
// Tobias NOTE: this version was working, but was inaccurate!
/*
			vec3_t start, end;

			trap_AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
			VectorMA(bs->origin, 32, sideward, start);
			VectorMA(ent->client->ps.origin, 32, sideward, end);

			BotAI_Trace(&trace, start, mins, maxs, end, bs->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP|CONTENTS_BODY|CONTENTS_CORPSE);
			// if something is hit
			if (trace.startsolid || trace.fraction < 1.0f) {
				// flip the direction
				VectorNegate(sideward, sideward);
// Tobias DEBUG
				obstacleMove = MOVEMENT_BARRIER_WALK_LEFT;
// Tobias END
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Forward movement failed. Flipped right side because found solid.\n", netname);
#endif
			} else {
// Tobias DEBUG
				obstacleMove = MOVEMENT_BARRIER_WALK_RIGHT;
// Tobias END
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Forward movement failed. Keep right side.\n", netname);
#endif
			}
*/
// Tobias FIXME: enhance/complete this!
			vec3_t blocked_dir, avoidAngles, avoidRight_dir, avoidLeft_dir, block_pos;
			float blocked_dist, rightSucc, leftSucc, yaw, avoidRadius, arcAngle;
			aas_entityinfo_t entinfo;

			trap_AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);

			mins[2] += STEPSIZE;
			// get the blocked direction
			yaw = VectorToYaw(blocked_dir);
			// get the avoid radius
			avoidRadius = sqrt((ent->r.maxs[0] * ent->r.maxs[0]) + (ent->r.maxs[1] * ent->r.maxs[1])) + sqrt((maxs[0] * maxs[0]) + (maxs[1] * maxs[1]));
			// get info for the entity that is blocking the bot
			BotEntityInfo(moveresult->blockentity, &entinfo);
			// see if we're inside our avoidance radius
			VectorSubtract(entinfo.origin, bs->origin, blocked_dir);

			blocked_dist = VectorNormalize(blocked_dir);
			arcAngle = (blocked_dist <= avoidRadius) ? 135 : ((avoidRadius / blocked_dist) * 90);

			VectorClear(avoidAngles);
			// test right
			avoidAngles[YAW] = AngleNormalize360(yaw + arcAngle);

			AngleVectorsForward(avoidAngles, avoidRight_dir);
			VectorMA(bs->origin, blocked_dist, avoidRight_dir, block_pos);
			BotAI_Trace(&trace, bs->origin, mins, maxs, block_pos, bs->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP|CONTENTS_BODY|CONTENTS_CORPSE);

			if (!trace.allsolid && !trace.startsolid) {
				if (trace.fraction >= 1.0f) {
					// all clear, go for it (favor the right if both are equal)
					VectorCopy(avoidRight_dir, rightwards);
					//self->NPC->lastSideStepSide = 1;
					//self->NPC->sideStepHoldTime = level.time + 2000;
					//return qtrue;
// Tobias DEBUG
					obstacleMove = MOVEMENT_BARRIER_WALK_RIGHT;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Forward movement failed. Keep right side.\n", netname);
#endif
				}

				rightSucc = trace.fraction;
			} else {
				rightSucc = 0.0f;
			}
			// now test left
			arcAngle *= -1;

			avoidAngles[YAW] = AngleNormalize360(yaw + arcAngle);

			AngleVectorsForward(avoidAngles, avoidLeft_dir);
			VectorMA(bs->origin, blocked_dist, avoidLeft_dir, block_pos);
			BotAI_Trace(&trace, bs->origin, mins, maxs, block_pos, bs->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP|CONTENTS_BODY|CONTENTS_CORPSE);

			if (!trace.allsolid && !trace.startsolid) {
				if (trace.fraction >= 1.0f) {
					// all clear, go for it (right side would have already succeeded if as good as this)
					VectorCopy(avoidLeft_dir, leftwards);
					//self->NPC->lastSideStepSide = -1;
					//self->NPC->sideStepHoldTime = level.time + 2000;
					//return qtrue;
// Tobias DEBUG
					obstacleMove = MOVEMENT_BARRIER_WALK_LEFT;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Forward movement failed. Flipped right side because found solid.\n", netname);
#endif
				}

				leftSucc = trace.fraction;
			} else {
				leftSucc = 0.0f;
			}
			// both sides failed
			if (leftSucc == 0.0f && rightSucc == 0.0f) {
				//return qfalse;
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: **** BOTH SIDES FAILED! ****.\n", netname);
#endif
			}
			// if the traces hit something, but got a relatively good distance
			if (rightSucc * blocked_dist >= avoidRadius || leftSucc * blocked_dist >= avoidRadius) {
				// favor the right, all things being equal
				if (rightSucc >= leftSucc) {
					VectorCopy(avoidRight_dir, rightwards);
					//self->NPC->lastSideStepSide = 1;
					//self->NPC->sideStepHoldTime = level.time + 2000;
// Tobias DEBUG
					obstacleMove = MOVEMENT_BARRIER_WALK_RIGHT;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Forward movement failed. Keep right side.\n", netname);
#endif
				} else {
					VectorCopy(avoidLeft_dir, leftwards);
					//self->NPC->lastSideStepSide = -1;
					//self->NPC->sideStepHoldTime = level.time + 2000;
// Tobias DEBUG
					obstacleMove = MOVEMENT_BARRIER_WALK_LEFT;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: Forward movement failed. Flipped right side because found solid.\n", netname);
#endif
				}

				//return qtrue;
			}
			// move sidwards
			if (!trap_BotMoveInDirection(bs->ms, rightwards, speed, movetype)) {
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: 1st sidewards movement failed, try left direction.\n", netname);
#endif
				// move in the other direction
				if (!trap_BotMoveInDirection(bs->ms, leftwards, speed, movetype)) {
					// move in a random direction in the hope to get out
					BotRandomMove(bs, moveresult, speed, movetype);
// Tobias DEBUG
					obstacleMove = MOVEMENT_RANDOM;
// Tobias END
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: 2nd sidewards movement failed, ending up using random move.\n", netname);
#endif
				}
			}
		}
	}
// Tobias DEBUG
	BotSetUserInfo(bs, "obstacleMove", va("%d", obstacleMove));
// Tobias END
}

/*
=======================================================================================================================================
BotObstacleAvoidanceMove
=======================================================================================================================================
*/
void BotObstacleAvoidanceMove(bot_state_t *bs, bot_moveresult_t *moveresult, int speed, int movetype) {
	vec3_t dir2, hordir, sideward, angles, up = {0, 0, 1};
	gentity_t *ent;
// Tobias DEBUG
#ifdef OBSTACLEDEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
// Tobias END
	// just some basic dynamic obstacle avoidance code
	hordir[0] = moveresult->movedir[0];
	hordir[1] = moveresult->movedir[1];
	hordir[2] = 0;
	// if no direction just take a random direction
	if (VectorNormalize(hordir) < 0.1) {
		VectorSet(angles, 0, 360 * random(), 0);
		AngleVectorsForward(angles, hordir);
	}
	// get the (right) sideward vector
	CrossProduct(hordir, up, sideward);
	// get the direction the blocking obstacle is moving
	ent = &g_entities[moveresult->blockentity];
	dir2[2] = 0;

	VectorCopy(ent->client->ps.velocity, dir2);
	// we start moving to our right side, but if the blocking entity is also moving towards our right side or if the right side is blocked move to the left side
	if (DotProduct(dir2, sideward) > 50.0f || moveresult->flags & MOVERESULT_BARRIER_WALK_LEFT) {
		// flip the direction
		VectorNegate(sideward, sideward);
#ifdef OBSTACLEDEBUG
		if (moveresult->flags & MOVERESULT_BARRIER_WALK_LEFT) {
			BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: Flipped right side because right side is blocked.\n", netname);
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: Flipped right side because dir2 = %1.1f.\n", netname, DotProduct(dir2, sideward));
		}
	} else {
		BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Keep right side.\n", netname);
#endif
	}
	// move sidwards
	if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
		// move in a random direction in the hope to get out
		BotRandomMove(bs, moveresult, speed, movetype);
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: Sidewards movement failed, ending up using random move.\n", netname);
#endif
	}
/*
	// if the blocking obstacle is not moving at all
	} else {
		if (moveresult->flags & MOVERESULT_BARRIER_WALK_LEFT) {
			VectorNegate(sideward, sideward);
#ifdef OBSTACLEDEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "%s: (Static obstacle) Flipped right side because dir2 = %1.1f.\n", netname, DotProduct(dir2, sideward));
#endif
		} else {
#ifdef OBSTACLEDEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_BLUE "%s: (Static obstacle) Keep right side.\n", netname);
#endif
		}
		// move sidwards
		if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
			// flip the direction
			VectorNegate(sideward, sideward); // Tobias CHECK: really needed?
#ifdef OBSTACLEDEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_YELLOW "%s: (Static obstacle) 1st sidewards movement failed, flipped direction.\n", netname);
#endif
			// move in the other direction
			if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
				// move in a random direction in the hope to get out
				BotRandomMove(bs, moveresult, speed, movetype); // Tobias CHECK: only if NOT teammate?
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: (Static obstacle) 2nd sidewards movement failed, ending up using random move.\n", netname);
#endif
			}
		}
	}
*/
}

/*
=======================================================================================================================================
BotObstacleAvoidanceMoveFast
=======================================================================================================================================
*/
void BotObstacleAvoidanceMoveFast(bot_state_t *bs, bot_moveresult_t *moveresult, int movetype) {
	vec3_t hordir, angles;

	// just some basic dynamic obstacle avoidance code
	hordir[0] = moveresult->movedir[0];
	hordir[1] = moveresult->movedir[1];
	hordir[2] = 0;
	// if no direction just take a random direction
	if (VectorNormalize(hordir) < 0.1) {
		VectorSet(angles, 0, 360 * random(), 0);
		AngleVectorsForward(angles, hordir);
	}

	trap_BotMoveInDirection(bs->ms, hordir, 400, movetype);
}

/*
=======================================================================================================================================
BotAIBlocked

Very basic handling of bots being blocked by other entities. Check what kind of entity is blocking the bot and try to activate it.
If that's not an option then try to walk around or over the entity.
Before the bot ends in this part of the AI it should predict which doors to open, which buttons to activate etc.
=======================================================================================================================================
*/
void BotAIBlocked(bot_state_t *bs, bot_moveresult_t *moveresult, bot_aienter_t activatedonefunc) {
#ifdef OBSTACLEDEBUG
	char netname[MAX_NETNAME];
#endif
// Tobias DEBUG
	int obstacleMove;
// Tobias END
	int movetype, bspent, speed;
	vec3_t dir1, dir2, mins, maxs, end;
	gentity_t *ent;
	aas_entityinfo_t entinfo;
	bot_activategoal_t activategoal;
	bsp_trace_t trace;
// Tobias DEBUG
	float blockcvar;

	blockcvar = bot_blocktime.value;
// Tobias END
	// if the bot is not blocked by anything
	if (!moveresult->blocked) {
		bs->notblocked_time = FloatTime();
// Tobias DEBUG
		BotSetUserInfo(bs, "obstacleMove", va("%d", MOVEMENT_DEFAULT));
// Tobias END
		return;
	}
// Tobias DEBUG
	if (moveresult->flags & MOVERESULT_BARRIER_JUMP) {
		obstacleMove = MOVEMENT_BARRIER_JUMP;
	} else if (moveresult->flags & MOVERESULT_BARRIER_CROUCH) {
		obstacleMove = MOVEMENT_BARRIER_CROUCH;
	} else if (moveresult->flags & MOVERESULT_BARRIER_WALK_LEFT) {
		obstacleMove = MOVEMENT_BARRIER_WALK_LEFT;
	} else if (moveresult->flags & MOVERESULT_BARRIER_WALK_RIGHT) {
		obstacleMove = MOVEMENT_BARRIER_WALK_RIGHT;
	} else {
		obstacleMove = MOVEMENT_DEFAULT;
	}

	BotSetUserInfo(bs, "obstacleMove", va("%d", obstacleMove));
// Tobias END
#ifdef OBSTACLEDEBUG
	ClientName(bs->client, netname, sizeof(netname));
#endif
	if (!BotWantsToWalk(bs)) {
		speed = 400;
	} else {
		speed = 200;
	}

	if (moveresult->flags & MOVERESULT_BARRIER_JUMP) {
		movetype = MOVE_JUMP;
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "BotAIBlocked: movetype = Jump\n");
#endif
	} else if (moveresult->flags & MOVERESULT_BARRIER_CROUCH) {
		movetype = MOVE_CROUCH;
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "BotAIBlocked: movetype = Crouch\n");
#endif
	} else {
		movetype = MOVE_WALK;
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_CYAN "BotAIBlocked: movetype = Walk\n");
#endif
	}
	// if stuck in a solid area
	if (moveresult->type == RESULTTYPE_INSOLIDAREA) {
		// move in a random direction in the hope to get out
		BotRandomMove(bs, moveresult, speed, movetype);
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_MAGENTA "%s: IN SOLID AREA!\n", netname);
#endif
		return;
	}
	// get info for the entity that is blocking the bot
	BotEntityInfo(moveresult->blockentity, &entinfo);
	// if the entity information is valid
	if (!entinfo.valid) {
		return;
	}

	ent = &g_entities[moveresult->blockentity];
	// if blocked by a bsp model
	if (!ent->client) {
		if (entinfo.modelindex > 0 && entinfo.modelindex <= max_bspmodelindex) {
			// a closed door without a targetname will operate automatically
			if (!strcmp(ent->classname, "func_door") && (ent->moverState == MOVER_POS1)) {
				// if no targetname and not a shootable door
				if (!ent->targetname && !ent->health) {
#ifdef OBSTACLEDEBUG
					BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Ignoring blocking model %d (Door)!\n", netname, entinfo.modelindex);
#endif
					return;
				}
			}
			// buttons will operate on contact
			if (!strcmp(ent->classname, "func_button") && (ent->moverState == MOVER_POS1)) {
#ifdef OBSTACLEDEBUG
				BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Ignoring blocking model %d (Button)!\n", netname, entinfo.modelindex);
#endif
				return;
			}
			// if the bot wants to activate the bsp entity
			if (activatedonefunc != NULL) {
				// find the bsp entity which should be activated in order to get the blocking entity out of the way
				bspent = BotGetActivateGoal(bs, entinfo.number, &activategoal);

				if (bspent) {
					if (bs->activatestack && !bs->activatestack->inuse) {
						bs->activatestack = NULL;
					}
					// if not already trying to activate this entity
					if (!BotIsGoingToActivateEntity(bs, activategoal.goal.entitynum)) {
						BotGoForActivateGoal(bs, &activategoal, activatedonefunc);
					}
					// if ontop of an obstacle or if the bot is not in a reachability area it'll still need some dynamic obstacle avoidance, otherwise return
					if (!(moveresult->flags & MOVERESULT_ONTOPOF_OBSTACLE) && trap_AAS_AreaReachability(bs->areanum)) {
						return;
					}
				} else {
					// enable any routing areas that were disabled
					BotEnableActivateGoalAreas(&activategoal, qtrue);
					// try to crouch through or jump over obstacles
					BotObstacleAvoidanceMoveFast(bs, moveresult, movetype); // Tobias NOTE: why has this to be done here, inside (bspent/activatedonefunc)?
					return;
				}
			}
		}
	// if blocked by a player
	} else {
		VectorSubtract(entinfo.origin, bs->origin, dir1);
		// Tobias HACK?: accompanying teammates are less concerned about blocking teammates
		// Tobias NOTE: also do this for multiple teammates trying to camp at the same spot (camp_dist)
		if (bs->ltgtype == LTG_TEAMACCOMPANY && BotSameTeam(bs, moveresult->blockentity) && VectorLengthSquared(dir1) > Square(bs->formation_dist) * 1.25) { // Tobias NOTE: repalac 1.25 by a more advanced value (in fov etc.)
#ifdef OBSTACLEDEBUG
			BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "Ignoring blocking teammate outside formation space!\n");
#endif
			return;
#ifdef OBSTACLEDEBUG
		} else {
			BotAI_Print(PRT_MESSAGE, S_COLOR_RED "BLOCKED inside formation space!\n");
#endif
		}

		VectorNormalize(dir1);
		VectorNormalize2(ent->client->ps.velocity, dir2);
		// if the blocking entity is moving away from us (or moving along the same direction), or if it is an enemy farther away than 24 units
		if (DotProduct(dir2, dir1) > 0.0 || !BotSameTeam(bs, moveresult->blockentity)) {
			trap_AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
			VectorMA(bs->origin, 24, dir1, end);
			BotAI_TraceEntities(&trace, bs->origin, mins, maxs, end, bs->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP|CONTENTS_BODY|CONTENTS_CORPSE);
			// if nothing is hit
			if (trace.fraction >= 1.0f) {
				return;
			}
#ifdef OBSTACLEDEBUG
			else {
				BotAI_Print(PRT_MESSAGE, S_COLOR_GREEN "%s: Ignoring blocking player moving along the same direction, or distant enemy!\n", netname);
			}
#endif
		}
	}
	// just some basic dynamic obstacle avoidance code
	BotObstacleAvoidanceMove(bs, moveresult, speed, movetype);

	if (activatedonefunc == NULL) {
		if (bs->notblocked_time < FloatTime() - blockcvar) {
			// just reset goals and hope the bot will go into another direction?
			// is this still needed??
			if (bs->ainode == AINode_Seek_NBG) {
				bs->nbg_time = 0;
			} else if (bs->ainode == AINode_Seek_LTG) {
				bs->ltg_time = 0;
			}
		}
	}
}

/*
=======================================================================================================================================
BotAIPredictObstacles

Predict the route towards the goal and check if the bot will be blocked by certain obstacles. When the bot has obstacles on its path
the bot should figure out if they can be removed by activating certain entities.
=======================================================================================================================================
*/
int BotAIPredictObstacles(bot_state_t *bs, bot_goal_t *goal, bot_aienter_t activatedonefunc) {
	int modelnum, entitynum, bspent;
	bot_activategoal_t activategoal;
	aas_predictroute_t route;

	if (!bot_predictobstacles.integer) {
		return qfalse;
	}
	// always predict when the goal change or at regular intervals
	if (bs->predictobstacles_goalareanum == goal->areanum && bs->predictobstacles_time > FloatTime() - 0.5) {
		return qfalse;
	}

	bs->predictobstacles_goalareanum = goal->areanum;
	bs->predictobstacles_time = FloatTime();
	// predict at most 100 areas or 1 second ahead
	trap_AAS_PredictRoute(&route, bs->areanum, bs->origin, goal->areanum, bs->tfl, 100, 1000, RSE_USETRAVELTYPE|RSE_ENTERCONTENTS, AREACONTENTS_MOVER, TFL_BRIDGE, 0);
	// if bot has to travel through an area with a mover
	if (route.stopevent & RSE_ENTERCONTENTS) {
		// if the bot will run into a mover
		if (route.endcontents & AREACONTENTS_MOVER) {
			// NOTE: this only works with bspc 2.1 or higher
			modelnum = (route.endcontents & AREACONTENTS_MODELNUM) >> AREACONTENTS_MODELNUMSHIFT;

			if (modelnum) {
				entitynum = BotModelMinsMaxs(modelnum, ET_MOVER, 0, NULL, NULL);

				if (entitynum) {
					// NOTE: BotGetActivateGoal already checks if the door is open or not
					bspent = BotGetActivateGoal(bs, entitynum, &activategoal);

					if (bspent) {
						if (bs->activatestack && !bs->activatestack->inuse) {
							bs->activatestack = NULL;
						}
						// if not already trying to activate this entity
						if (!BotIsGoingToActivateEntity(bs, activategoal.goal.entitynum)) {
							//BotAI_Print(PRT_MESSAGE, "blocked by mover model %d, entity %d ?\n", modelnum, entitynum);
							BotGoForActivateGoal(bs, &activategoal, activatedonefunc);
							return qtrue;
						} else {
							// enable any routing areas that were disabled
							BotEnableActivateGoalAreas(&activategoal, qtrue);
						}
					}
				}
			}
		}
	} else if (route.stopevent & RSE_USETRAVELTYPE) {
		if (route.endtravelflags & TFL_BRIDGE) {
			// FIXME: check if the bridge is available to travel over
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotCheckConsoleMessages
=======================================================================================================================================
*/
void BotCheckConsoleMessages(bot_state_t *bs) {
	char botname[MAX_NETNAME], message[MAX_MESSAGE_SIZE], netname[MAX_NETNAME], *ptr;
	float chat_reply;
	int context, handle;
	bot_consolemessage_t m;
	bot_match_t match;

	// the name of this bot
	ClientName(bs->client, botname, sizeof(botname));

	while ((handle = trap_BotNextConsoleMessage(bs->cs, &m)) != 0) {
		// if the chat state is flooded with messages the bot will read them quickly
		if (trap_BotNumConsoleMessages(bs->cs) < 10) {
			// if it is a chat message the bot needs some time to read it
			if (m.type == CMS_CHAT && m.time > FloatTime() - (1 + random())) {
				break;
			}
		}

		ptr = m.message;
		// if it is a chat message then don't unify white spaces and don't replace synonyms in the netname
		if (m.type == CMS_CHAT) {
			if (trap_BotFindMatch(m.message, &match, MTCONTEXT_REPLYCHAT)) {
				ptr = m.message + match.variables[MESSAGE].offset;
			}
		}
		// unify the white spaces in the message
		trap_UnifyWhiteSpaces(ptr);
		// replace synonyms in the right context
		context = BotSynonymContext(bs);

		trap_BotReplaceSynonyms(ptr, context);
		// if there's no match
		if (!BotMatchMessage(bs, m.message)) {
			// if it is a chat message
			if (m.type == CMS_CHAT && !bot_nochat.integer) {
				if (!trap_BotFindMatch(m.message, &match, MTCONTEXT_REPLYCHAT)) {
					trap_BotRemoveConsoleMessage(bs->cs, handle);
					continue;
				}
				// don't use eliza chats with team messages
				if (match.subtype & ST_TEAM) {
					trap_BotRemoveConsoleMessage(bs->cs, handle);
					continue;
				}

				trap_BotMatchVariable(&match, NETNAME, netname, sizeof(netname));
				trap_BotMatchVariable(&match, MESSAGE, message, sizeof(message));
				// if this is a message from the bot self
				if (bs->client == ClientFromName(netname)) {
					trap_BotRemoveConsoleMessage(bs->cs, handle);
					continue;
				}
				// unify the message
				trap_UnifyWhiteSpaces(message);
				trap_Cvar_Update(&bot_testrchat);

				if (bot_testrchat.integer) {
					trap_BotLibVarSet("bot_testrchat", "1");
					// if bot replies with a chat message
					if (trap_BotReplyChat(bs->cs, message, context, CONTEXT_REPLY, NULL, NULL, NULL, NULL, NULL, NULL, botname, netname)) {
						BotAI_Print(PRT_MESSAGE, "------------------------\n");
					} else {
						BotAI_Print(PRT_MESSAGE, "**** no valid reply ****\n");
					}
				// if at a valid chat position and not chatting already and not in teamplay
				} else if (bs->ainode != AINode_Stand && BotValidChatPosition(bs) && !TeamPlayIsOn()) {
					chat_reply = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_CHAT_REPLY, 0, 1);

					if (random() < 1.5 / (NumBots() + 1) && random() < chat_reply) {
						// if bot replies with a chat message
						if (trap_BotReplyChat(bs->cs, message, context, CONTEXT_REPLY, NULL, NULL, NULL, NULL, NULL, NULL, botname, netname)) {
							// remove the console message
							trap_BotRemoveConsoleMessage(bs->cs, handle);
							break;
						}
					}
				}
			}
		}
		// remove the console message
		trap_BotRemoveConsoleMessage(bs->cs, handle);
	}
}

/*
=======================================================================================================================================
BotCheckBlockedTeammates


TODO: 1# better check for desired speed instead of actual speed :\
=======================================================================================================================================
*/
void BotCheckBlockedTeammates(bot_state_t *bs) {
	bot_moveresult_t moveresult;
	int movetype, i, squaredist, mindist, speed;
	aas_entityinfo_t entinfo;
	gentity_t *ent;
	float obtrusiveness;
	vec3_t dir, viewangles, mins, maxs, end, v3, v2, v1, sideward, angles, up = {0, 0, 1};
	bsp_trace_t trace;
#ifdef DEBUG
	char netname[MAX_NETNAME];

	ClientName(bs->client, netname, sizeof(netname));
#endif
// Tobias DEBUG
	if (!bot_teamblock.integer) {
		return;
	}
// Tobias END
	if (gametype < GT_TEAM) {
		return;
	}

	if (BotCTFCarryingFlag(bs)) {
		return;
	}

	if (Bot1FCTFCarryingFlag(bs)) {
		return;
	}

	if (BotHarvesterCarryingCubes(bs)) {
		return;
	}

	obtrusiveness = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_OBTRUSIVENESS, 0, 1);

	VectorSet(bs->notblocked_dir, 0, 0, 0);

	for (i = 0; i < level.maxclients; i++) {
		if (i == bs->client) {
			continue;
		}
		// if on the same team
		if (!BotSameTeam(bs, i)) {
			continue;
		}
		// get the entity information
		BotEntityInfo(i, &entinfo);
		// if the entity information is valid
		if (!entinfo.valid) {
			continue;
		}
		// if the entity isn't the bot self
		if (entinfo.number == bs->entitynum) {
			continue;
		}
		// if the entity isn't dead
		if (EntityIsDead(&entinfo)) {
			continue;
		}
		// if the teammate is not visible
		if (!BotEntityVisible(&bs->cur_ps, 360, i)) {
			continue;
		}
		// calculate the distance towards the teammate
		VectorSubtract(bs->origin, entinfo.origin, dir);
		VectorToAngles(dir, viewangles);

		squaredist = VectorLengthSquared(dir);
		ent = &g_entities[i];
		// if the teammate walks slowly and doesn't carrying a flag or skulls and if the teammate isn't dangerous
		if (VectorLengthSquared(ent->client->ps.velocity) < 40000 && !EntityCarriesFlag(&entinfo) && !EntityCarriesCubes(&entinfo) && (!(entinfo.flags & EF_TICKING))) { // TODO: 1# we need BUTON_RUN (ACTION) instead of ent->client->ps.velocity
			// if the teammate is far away enough
			if (squaredist > 16384) {
				continue;
			}
			// human players and facing teammates need more space
			if (!(ent->r.svFlags & SVF_BOT) || (BotEntityVisible(&bs->cur_ps, 60, i) && InFieldOfVision(entinfo.angles, 60, viewangles))) {
				mindist = 42;
			} else {
				// set minimum distance
				mindist = 8;
			}
			// set default speed
			speed = 200;
		} else {
			// if the teammate is far away enough
			if (squaredist > 65536) {
				continue;
			}

			mindist = 256 - (128 * obtrusiveness);
			// use max speed
			speed = 400;
		}
		// safety check, don't force to reach the goal
		if (mindist >= bs->formation_dist) {
			bs->formation_dist = mindist;
		}
#ifdef OBSTACLEDEBUG
		BotAI_Print(PRT_MESSAGE, S_COLOR_RED "%s: teammate speed = %f, dist = %i, required speed = %i, required mindist = %i\n", netname, VectorLengthSquared(ent->client->ps.velocity), squaredist, speed, mindist);
#endif
		// calculate the direction towards the teammate
		v2[2] = 0;

		VectorSubtract(entinfo.origin, bs->origin, v2);
		VectorNormalize(v2);
		// now check if the teammate is blocked, increase the distance accordingly
		trap_AAS_PresenceTypeBoundingBox(PRESENCE_NORMAL, mins, maxs);
		VectorMA(bs->origin, mindist, v2, end);
		BotAI_TraceEntities(&trace, bs->origin, mins, maxs, end, bs->entitynum, CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP|CONTENTS_BODY|CONTENTS_CORPSE);
		// if the teammate is too close (blocked)
		if (trace.entityNum == i && (trace.startsolid || trace.fraction < 1.0f)) {
			// stop crouching to gain speed
			bs->crouch_time = FloatTime() - 1;
			movetype = MOVE_WALK;
			// look into the direction of the blocked teammate
			VectorToAngles(v2, bs->ideal_viewangles);
			// get the sideward vector
			CrossProduct(up, v2, sideward);
			// get the direction the blocked player is moving
			v1[2] = 0;

			VectorNormalize2(ent->client->ps.velocity, v1);
			VectorCopy(ent->client->ps.velocity, v1);
			// the blocked player is moving to his left side, so move to his right side (and vice versa)
			if (DotProduct(v1, sideward) > -50.0f) {
				// flip the direction
				VectorNegate(sideward, sideward);
			}
			// also go backwards a little
			VectorMA(sideward, -1, v2, sideward);
			// move sidwards
			if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
				// flip the direction
				VectorNegate(sideward, sideward);
				// move in the other direction
				if (!trap_BotMoveInDirection(bs->ms, sideward, speed, movetype)) {
					// try to step back
					if (!trap_BotMoveInDirection(bs->ms, v2, speed, movetype)) {
						if (DotProduct(bs->notblocked_dir, bs->notblocked_dir) < 0.1) {
							VectorSet(angles, 0, 360 * random(), 0);
							AngleVectorsForward(angles, v3);
						} else {
							VectorCopy(bs->notblocked_dir, v3);
						}

						if (!trap_BotMoveInDirection(bs->ms, v3, speed, movetype)) {
							VectorSet(bs->notblocked_dir, 0, 0, 0);
							// move in a random direction in the hope to get out
							BotRandomMove(bs, &moveresult, speed, movetype);
						} else {
							VectorCopy(v3, bs->notblocked_dir);
						}
					}
				}
			}
		}
	}
}

/*
=======================================================================================================================================
BotCheckForGrenades
=======================================================================================================================================
*/
void BotCheckForGrenades(bot_state_t *bs, entityState_t *state) {

	// if this is not a grenade
	if (state->eType != ET_MISSILE || state->weapon != WP_GRENADELAUNCHER) {
		return;
	}
	// try to avoid the grenade
	trap_BotAddAvoidSpot(bs->ms, state->pos.trBase, 160, AVOID_ALWAYS);
}

/*
=======================================================================================================================================
BotCheckForProxMines
=======================================================================================================================================
*/
void BotCheckForProxMines(bot_state_t *bs, entityState_t *state) {

	// if this is not a prox mine
	if (state->eType != ET_MISSILE || state->weapon != WP_PROXLAUNCHER) {
		return;
	}
	// if this prox mine is from someone on our own team
	if (gametype > GT_TOURNAMENT && state->team == BotTeam(bs)) {
		return;
	}
	// if the bot doesn't have a weapon to deactivate the mine
	if (!(bs->inventory[INVENTORY_PLASMAGUN] > 0 && bs->inventory[INVENTORY_CELLS] > 0) && !(bs->inventory[INVENTORY_ROCKETLAUNCHER] > 0 && bs->inventory[INVENTORY_ROCKETS] > 0) && !(bs->inventory[INVENTORY_BFG10K] > 0 && bs->inventory[INVENTORY_BFG_AMMO] > 0)) {
		return;
	}
	// try to avoid the prox mine
	trap_BotAddAvoidSpot(bs->ms, state->pos.trBase, 160, AVOID_ALWAYS);

	if (bs->numproxmines >= MAX_PROXMINES) {
		return;
	}

	bs->proxmines[bs->numproxmines] = state->number;
	bs->numproxmines++;
}

/*
=======================================================================================================================================
BotCheckForKamikazeBody
=======================================================================================================================================
*/
void BotCheckForKamikazeBody(bot_state_t *bs, entityState_t *state) {

	// if this entity is not wearing the kamikaze
	if (!(state->eFlags & EF_KAMIKAZE)) {
		return;
	}
	// if this entity isn't dead
	if (!(state->eFlags & EF_DEAD)) {
		return;
	}
	// remember this kamikaze body
	bs->kamikazebody = state->number;
}

/*
=======================================================================================================================================
BotCheckEvents
=======================================================================================================================================
*/
void BotCheckEvents(bot_state_t *bs, entityState_t *state) {
	int event;
	char buf[128];
	aas_entityinfo_t entinfo;

	// NOTE: this sucks, we're accessing the gentity_t directly but there's no other fast way to do it right now
	if (bs->entityeventTime[state->number] == g_entities[state->number].eventTime) {
		return;
	}

	bs->entityeventTime[state->number] = g_entities[state->number].eventTime;
	// if it's an event only entity
	if (state->eType > ET_EVENTS) {
		event = (state->eType - ET_EVENTS) & ~EV_EVENT_BITS;
	} else {
		event = state->event & ~EV_EVENT_BITS;
	}

	switch (event) {
		case EV_GENERAL_SOUND:
		{
			// if this sound is played on the bot
			if (state->number == bs->client) {
				if (state->eventParm < 0 || state->eventParm >= MAX_SOUNDS) {
					BotAI_Print(PRT_ERROR, "EV_GENERAL_SOUND: eventParm (%d) out of range\n", state->eventParm);
					break;
				}
				// check out the sound
				trap_GetConfigstring(CS_SOUNDS + state->eventParm, buf, sizeof(buf));
				// if falling into a death pit
				if (!strcmp(buf, "*fv1.wav")) {
					// if the bot has a kamikaze
					if (bs->inventory[INVENTORY_KAMIKAZE] > 0) {
						// use the holdable item
						trap_EA_Use(bs->client);
					}
				}
			}

			break;
		}
		case EV_GLOBAL_SOUND:
		{
			if (state->eventParm < 0 || state->eventParm >= MAX_SOUNDS) {
				BotAI_Print(PRT_ERROR, "EV_GLOBAL_SOUND: eventParm (%d) out of range\n", state->eventParm);
				break;
			}

			trap_GetConfigstring(CS_SOUNDS + state->eventParm, buf, sizeof(buf));
			/*
			if (!strcmp(buf, "snd/m/flagret_red.wav")) {
				// red flag is returned
				bs->redflagstatus = 0;
				bs->flagstatuschanged = qtrue;
			} else if (!strcmp(buf, "snd/m/flagret_blu.wav")) {
				// blue flag is returned
				bs->blueflagstatus = 0;
				bs->flagstatuschanged = qtrue;
			} else
			*/
			if (!strcmp(buf, "snd/i/kam_sp.wav")) {
				// the kamikaze respawned so don't avoid it
				BotDontAvoid(bs, "Kamikaze");
			} else if (!strcmp(buf, "snd/i/psp.wav")) {
				// powerup respawned... go get it
				BotGoForPowerups(bs);
			}

			break;
		}
		case EV_GLOBAL_TEAM_SOUND:
		{
			if (gametype == GT_CTF) {
				switch (state->eventParm) {
					case GTS_RED_CAPTURE:
						bs->blueflagstatus = 0;
						bs->redflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break; // see BotMatch_CTF
					case GTS_BLUE_CAPTURE:
						bs->blueflagstatus = 0;
						bs->redflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break; // see BotMatch_CTF
					case GTS_RED_RETURN:
						// blue flag is returned
						bs->blueflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_BLUE_RETURN:
						// red flag is returned
						bs->redflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_RED_TAKEN:
						// blue flag is taken
						bs->blueflagstatus = 1;
						bs->flagstatuschanged = qtrue;
						break; // see BotMatch_CTF
					case GTS_BLUE_TAKEN:
						// red flag is taken
						bs->redflagstatus = 1;
						bs->flagstatuschanged = qtrue;
						break; // see BotMatch_CTF
				}
			} else if (gametype == GT_1FCTF) {
				switch (state->eventParm) {
					case GTS_RED_CAPTURE:
						bs->neutralflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_BLUE_CAPTURE:
						bs->neutralflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_RED_RETURN:
						// flag has returned
						bs->neutralflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_BLUE_RETURN:
						// flag has returned
						bs->neutralflagstatus = 0;
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_RED_TAKEN:
						bs->neutralflagstatus = BotTeam(bs) == TEAM_RED ? 2 : 1; // FIXME: check Team_TakeFlagSound in g_team.c
						bs->flagstatuschanged = qtrue;
						break;
					case GTS_BLUE_TAKEN:
						bs->neutralflagstatus = BotTeam(bs) == TEAM_BLUE ? 2 : 1; // FIXME: check Team_TakeFlagSound in g_team.c
						bs->flagstatuschanged = qtrue;
						break;
				}
			} else if (gametype == GT_OBELISK || gametype == GT_HARVESTER) {
				switch (state->eventParm) {
					case GTS_RED_CAPTURE:
					case GTS_BLUE_CAPTURE:
						bs->lastteamscore_time = FloatTime();
						break;
				}
			}

			break;
		}
		// client obituary event
		case EV_OBITUARY:
		{
			int target, attacker, mod;
			float vengefulness;
			qboolean getRevenge;

			target = state->otherEntityNum;
			attacker = state->otherEntityNum2;
			mod = state->eventParm;
			// does the bot want revenge?
			if (level.numPlayingClients < 3) {
				getRevenge = qfalse;
			} else {
				vengefulness = trap_Characteristic_BFloat(bs->character, CHARACTERISTIC_VENGEFULNESS, 0, 1);
				getRevenge = (random() < vengefulness);
			}
			// the bot was killed
			if (target == bs->client) {
				bs->botdeathtype = mod;
				bs->lastkilledby = attacker;

				if (target == attacker || target == ENTITYNUM_NONE || target == ENTITYNUM_WORLD) {
					bs->botsuicide = qtrue;
				} else {
					bs->botsuicide = qfalse;
				}

				bs->num_deaths++;

				if (!bs->botsuicide) {
					if (getRevenge) {
						if (attacker != bs->revenge_enemy) {
							bs->revenge_enemy = attacker;
							bs->revenge_kills = 0;
						}

						bs->revenge_kills++;
					} else {
						bs->revenge_enemy = -1;
						bs->revenge_kills = 0;
					}
				}
			// if this player was killed by the bot
			} else if (attacker == bs->client) {
				bs->enemydeathtype = mod;
				bs->lastkilledplayer = target;
				bs->killedenemy_time = FloatTime();
				bs->num_kills++;
				// revenge!
				if (target == bs->revenge_enemy) {
					if (getRevenge) {
						bs->revenge_kills--;
					} else {
						bs->revenge_kills = 0;
					}

					if (bs->revenge_kills <= 0) {
						bs->revenge_enemy = -1;
					}
				}
			} else if (attacker == bs->enemy && target == attacker) {
				bs->enemysuicide = qtrue;
			}

			if (gametype == GT_1FCTF) {
				// get the entity information
				BotEntityInfo(target, &entinfo);

				if (entinfo.powerups & (1 << PW_NEUTRALFLAG)) {
					if (!BotSameTeam(bs, target)) {
						bs->neutralflagstatus = 3; // enemy dropped the flag
						bs->flagstatuschanged = qtrue;
					}
				}
			}

			break;
		}
		case EV_SWIM:
		case EV_FALL_DIE:
		case EV_FALL_DMG_50:
		case EV_FALL_DMG_25:
		case EV_FALL_DMG_15:
		case EV_FALL_DMG_10:
		case EV_FALL_DMG_5:
		case EV_FALL_SHORT:
		case EV_STEP_4:
		case EV_STEP_8:
		case EV_STEP_12:
		case EV_STEP_16:
		case EV_JUMP_PAD:
		case EV_JUMP:
		case EV_TAUNT:
		case EV_WATER_TOUCH:
		case EV_WATER_LEAVE:
		case EV_WATER_UNDER:
		case EV_WATER_CLEAR:
		case EV_ITEM_PICKUP:
		case EV_GLOBAL_ITEM_PICKUP:
		case EV_NOAMMO:
		case EV_CHANGE_WEAPON:
		case EV_FIRE_WEAPON:
			// FIXME: either add to sound queue or mark player as someone making noise
			break;
		case EV_USE_ITEM0:
		case EV_USE_ITEM1:
		case EV_USE_ITEM2:
		case EV_USE_ITEM3:
		case EV_USE_ITEM4:
		case EV_USE_ITEM5:
		case EV_USE_ITEM6:
		case EV_USE_ITEM7:
		case EV_USE_ITEM8:
		case EV_USE_ITEM9:
		case EV_USE_ITEM10:
		case EV_USE_ITEM11:
		case EV_USE_ITEM12:
		case EV_USE_ITEM13:
		case EV_USE_ITEM14:
		case EV_USE_ITEM15:
			break;
		case EV_PLAYER_TELEPORT_IN:
		{
			VectorCopy(state->origin, lastteleport_origin);
			lastteleport_time = FloatTime();
			break;
		}
	}
}

/*
=======================================================================================================================================
BotCheckSnapshot
=======================================================================================================================================
*/
void BotCheckSnapshot(bot_state_t *bs) {
	int ent;
	entityState_t state;

	// remove all avoid spots
	trap_BotAddAvoidSpot(bs->ms, vec3_origin, 0, AVOID_CLEAR);
	// reset number of prox mines
	bs->numproxmines = 0;
	// reset kamikaze body
	bs->kamikazebody = 0;
	ent = 0;

	while ((ent = BotAI_GetSnapshotEntity(bs->client, ent, &state)) != -1) {
		// check the entity state for events
		BotCheckEvents(bs, &state);
		// check for grenades the bot should avoid
		BotCheckForGrenades(bs, &state);
		// check for proximity mines which the bot should deactivate
		BotCheckForProxMines(bs, &state);
		// check for dead bodies with the kamikaze effect which should be gibbed
		BotCheckForKamikazeBody(bs, &state);
	}
	// check the player state for events
	BotAI_GetEntityState(bs->client, &state);
	// copy the player state events to the entity state
	state.event = bs->cur_ps.externalEvent;
	state.eventParm = bs->cur_ps.externalEventParm;

	BotCheckEvents(bs, &state);
}

/*
=======================================================================================================================================
BotCheckTeamScores
=======================================================================================================================================
*/
void BotCheckTeamScores(bot_state_t *bs) {

	switch (bs->cur_ps.persistant[PERS_TEAM]) {
		case TEAM_RED:
			bs->enemyteamscore = level.teamScores[TEAM_BLUE];
			bs->ownteamscore = level.teamScores[TEAM_RED];
			break;
		case TEAM_BLUE:
			bs->enemyteamscore = level.teamScores[TEAM_RED];
			bs->ownteamscore = level.teamScores[TEAM_BLUE];
			break;
		default:
			return;
	}
}

/*
=======================================================================================================================================
BotCheckAir
=======================================================================================================================================
*/
void BotCheckAir(bot_state_t *bs) {

	if (bs->inventory[INVENTORY_REGEN] <= 0) {
		if (trap_AAS_PointContents(bs->eye) & (CONTENTS_WATER|CONTENTS_SLIME|CONTENTS_LAVA)) {
			return;
		}
	}

	bs->lastair_time = FloatTime();
}

/*
=======================================================================================================================================
BotAlternateRoute
=======================================================================================================================================
*/
bot_goal_t *BotAlternateRoute(bot_state_t *bs, bot_goal_t *goal) {
	int t;

	// if the bot has an alternative route goal
	if (bs->altroutegoal.areanum) {
		if (bs->reachedaltroutegoal_time) {
			return goal;
		}
		// travel time towards alternative route goal
		t = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, bs->altroutegoal.areanum, bs->tfl);

		if (t && t < 20) {
			//BotAI_Print(PRT_MESSAGE, "reached alternate route goal\n");
			bs->reachedaltroutegoal_time = FloatTime();
		}

		memcpy(goal, &bs->altroutegoal, sizeof(bot_goal_t));
		return &bs->altroutegoal;
	}

	return goal;
}

/*
=======================================================================================================================================
BotGetAlternateRouteGoal
=======================================================================================================================================
*/
int BotGetAlternateRouteGoal(bot_state_t *bs, int base) {
	aas_altroutegoal_t *altroutegoals;
	bot_goal_t *goal;
	int numaltroutegoals, rnd;

	if (base == TEAM_RED) {
		altroutegoals = red_altroutegoals;
		numaltroutegoals = red_numaltroutegoals;
	} else {
		altroutegoals = blue_altroutegoals;
		numaltroutegoals = blue_numaltroutegoals;
	}

	if (!numaltroutegoals) {
		return qfalse;
	}

	rnd = (float)random() * numaltroutegoals;

	if (rnd >= numaltroutegoals) {
		rnd = numaltroutegoals - 1;
	}

	goal = &bs->altroutegoal;
	goal->areanum = altroutegoals[rnd].areanum;

	VectorCopy(altroutegoals[rnd].origin, goal->origin);
	VectorSet(goal->mins, -8, -8, -8);
	VectorSet(goal->maxs, 8, 8, 8);

	goal->entitynum = 0;
	goal->iteminfo = 0;
	goal->number = 0;
	goal->flags = 0;

	bs->reachedaltroutegoal_time = 0;
	return qtrue;
}

/*
=======================================================================================================================================
BotSetupAlternativeRouteGoals
=======================================================================================================================================
*/
void BotSetupAlternativeRouteGoals(void) {

	if (altroutegoals_setup) {
		return;
	}

	if (gametype == GT_CTF) {
		if (trap_BotGetLevelItemGoal(-1, "Neutral Flag", &ctf_neutralflag) < 0) {
			BotAI_Print(PRT_WARNING, "No alt routes without Neutral Flag\n");
		}

		if (ctf_neutralflag.areanum) {
			red_numaltroutegoals = trap_AAS_AlternativeRouteGoals(ctf_neutralflag.origin, ctf_neutralflag.areanum, ctf_redflag.origin, ctf_redflag.areanum, TFL_DEFAULT, red_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
			blue_numaltroutegoals = trap_AAS_AlternativeRouteGoals(ctf_neutralflag.origin, ctf_neutralflag.areanum, ctf_blueflag.origin, ctf_blueflag.areanum, TFL_DEFAULT, blue_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
		}
	} else if (gametype == GT_1FCTF) {
		if (trap_BotGetLevelItemGoal(-1, "Neutral Obelisk", &neutralobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "One Flag CTF without Neutral Obelisk\n");
		}

		red_numaltroutegoals = trap_AAS_AlternativeRouteGoals(ctf_neutralflag.origin, ctf_neutralflag.areanum, ctf_redflag.origin, ctf_redflag.areanum, TFL_DEFAULT, red_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
		blue_numaltroutegoals = trap_AAS_AlternativeRouteGoals(ctf_neutralflag.origin, ctf_neutralflag.areanum, ctf_blueflag.origin, ctf_blueflag.areanum, TFL_DEFAULT, blue_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
	} else if (gametype == GT_OBELISK) {
		if (trap_BotGetLevelItemGoal(-1, "Neutral Obelisk", &neutralobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "No alt routes without Neutral Obelisk\n");
		}

		red_numaltroutegoals = trap_AAS_AlternativeRouteGoals(neutralobelisk.origin, neutralobelisk.areanum, redobelisk.origin, redobelisk.areanum, TFL_DEFAULT, red_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
		blue_numaltroutegoals = trap_AAS_AlternativeRouteGoals(neutralobelisk.origin, neutralobelisk.areanum, blueobelisk.origin, blueobelisk.areanum, TFL_DEFAULT, blue_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
	} else if (gametype == GT_HARVESTER) {
		if (trap_BotGetLevelItemGoal(-1, "Neutral Obelisk", &neutralobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Harvester without Neutral Obelisk\n");
		}

		red_numaltroutegoals = trap_AAS_AlternativeRouteGoals(neutralobelisk.origin, neutralobelisk.areanum, redobelisk.origin, redobelisk.areanum, TFL_DEFAULT, red_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
		blue_numaltroutegoals = trap_AAS_AlternativeRouteGoals(neutralobelisk.origin, neutralobelisk.areanum, blueobelisk.origin, blueobelisk.areanum, TFL_DEFAULT, blue_altroutegoals, MAX_ALTROUTEGOALS, ALTROUTEGOAL_CLUSTERPORTALS|ALTROUTEGOAL_VIEWPORTALS);
	}

	altroutegoals_setup = qtrue;
}

/*
=======================================================================================================================================
BotDeathmatchAI
=======================================================================================================================================
*/
void BotDeathmatchAI(bot_state_t *bs, float thinktime) {
	char gender[144], name[144];
	char userinfo[MAX_INFO_STRING];
	int i;

	// if the bot has just been setup
	if (bs->setupcount > 0) {
		bs->setupcount--;

		if (bs->setupcount > 0) {
			return;
		}
		// get the gender characteristic
		trap_Characteristic_String(bs->character, CHARACTERISTIC_GENDER, gender, sizeof(gender));
		// set the bot gender
		trap_GetUserinfo(bs->client, userinfo, sizeof(userinfo));
		Info_SetValueForKey(userinfo, "sex", gender);
		trap_SetUserinfo(bs->client, userinfo);
		// set the chat gender
		if (gender[0] == 'm') {
			trap_BotSetChatGender(bs->cs, CHAT_GENDERMALE);
		} else if (gender[0] == 'f') {
			trap_BotSetChatGender(bs->cs, CHAT_GENDERFEMALE);
		} else {
			trap_BotSetChatGender(bs->cs, CHAT_GENDERLESS);
		}
		// set the chat name
		ClientName(bs->client, name, sizeof(name));
		trap_BotSetChatName(bs->cs, name, bs->client);

		bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
		bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
		bs->setupcount = 0;

		BotSetupAlternativeRouteGoals();
	}
	// no ideal view set
	bs->flags &= ~BFL_IDEALVIEWSET;
	// if not in the intermission and not in observer mode
	if (!BotIntermission(bs) && !BotIsObserver(bs)) {
		bs->tfl = TFL_DEFAULT;
		// initialize the movement state
		BotSetupForMovement(bs);
		// check out the snapshot
		BotCheckSnapshot(bs);
		// update some inventory values
		BotUpdateInventory(bs);
		// choose the best weapon to fight with
		BotChooseWeapon(bs);
		// use holdable items
		BotBattleUseItems(bs);
		// do team AI
		BotTeamAI(bs);
		// set the teleport time
		BotSetTeleportTime(bs);
		// check for air
		BotCheckAir(bs);
	}
	// check the console messages
	BotCheckConsoleMessages(bs);
	// if the bot has no ai node
	if (!bs->ainode) {
		AIEnter_Seek_LTG(bs, "BotDeathmatchAI: no ai node");
	}
	// reset the node switches from the previous frame
	BotResetNodeSwitches();
	// execute AI nodes
	for (i = 0; i < MAX_NODESWITCHES; i++) {
		if (bs->ainode(bs)) {
			break;
		}
	}
	// if the bot removed itself :)
	if (!bs->inuse) {
		return;
	}
	// if the bot executed too many AI nodes
	if (i >= MAX_NODESWITCHES) {
		trap_BotDumpGoalStack(bs->gs);
		trap_BotDumpAvoidGoals(bs->gs);
		BotDumpNodeSwitches(bs);
		ClientName(bs->client, name, sizeof(name));
		BotAI_Print(PRT_ERROR, "%s at %1.1f switched more than %d AI nodes\n", name, FloatTime(), MAX_NODESWITCHES);
	}

	bs->lastframe_health = bs->inventory[INVENTORY_HEALTH];
	bs->lasthitcount = bs->cur_ps.persistant[PERS_HITS];
	// if the bot entered the game less than 8 seconds ago
	if (!bs->entergamechat && bs->entergame_time > FloatTime() - 8) {
		BotChat_EnterGame(bs);
		bs->entergamechat = qtrue;
	}
}

/*
=======================================================================================================================================
BotSetEntityNumForGoalWithModel
=======================================================================================================================================
*/
void BotSetEntityNumForGoalWithModel(bot_goal_t *goal, int eType, char *modelname) {
	gentity_t *ent;
	int i, modelindex;
	vec3_t dir;

	modelindex = G_ModelIndex(modelname);
	ent = &g_entities[0];

	for (i = 0; i < level.num_entities; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}

		if (eType && ent->s.eType != eType) {
			continue;
		}

		if (ent->s.modelindex != modelindex) {
			continue;
		}

		VectorSubtract(goal->origin, ent->s.origin, dir);

		if (VectorLengthSquared(dir) < Square(10)) {
			goal->entitynum = i;
			return;
		}
	}
}

/*
=======================================================================================================================================
BotSetEntityNumForGoal
=======================================================================================================================================
*/
void BotSetEntityNumForGoal(bot_goal_t *goal, char *classname) {
	gentity_t *ent;
	int i;
	vec3_t dir;

	ent = &g_entities[0];

	for (i = 0; i < level.num_entities; i++, ent++) {
		if (!ent->inuse) {
			continue;
		}

		if (Q_stricmp(ent->classname, classname) != 0) {
			continue;
		}

		VectorSubtract(goal->origin, ent->s.origin, dir);

		if (VectorLengthSquared(dir) < Square(10)) {
			goal->entitynum = i;
			return;
		}
	}
}

/*
=======================================================================================================================================
BotSetEntityNumForGoalWithActivator
=======================================================================================================================================
*/
void BotSetEntityNumForGoalWithActivator(bot_goal_t *goal, char *classname) {
	gentity_t *ent;
	int i;
	vec3_t dir;

	ent = &g_entities[0];

	for (i = 0; i < level.num_entities; i++, ent++) {
		if (!ent->inuse || !ent->activator) {
			continue;
		}

		if (Q_stricmp(ent->activator->classname, classname) != 0) {
			continue;
		}

		VectorSubtract(goal->origin, ent->s.origin, dir);

		if (VectorLengthSquared(dir) < Square(10)) {
			goal->entitynum = i;
			return;
		}
	}
}

/*
=======================================================================================================================================
BotGoalForBSPEntity
=======================================================================================================================================
*/
int BotGoalForBSPEntity(char *classname, bot_goal_t *goal) {
	char value[MAX_INFO_STRING];
	vec3_t origin, start, end;
	int ent, numareas, areas[10];

	memset(goal, 0, sizeof(bot_goal_t));

	for (ent = trap_AAS_NextBSPEntity(0); ent; ent = trap_AAS_NextBSPEntity(ent)) {
		if (!trap_AAS_ValueForBSPEpairKey(ent, "classname", value, sizeof(value))) {
			continue;
		}

		if (!strcmp(value, classname)) {
			if (!trap_AAS_VectorForBSPEpairKey(ent, "origin", origin)) {
				return qfalse;
			}

			VectorCopy(origin, goal->origin);
			VectorCopy(origin, start);

			start[2] -= 32;

			VectorCopy(origin, end);

			end[2] += 32;
			numareas = trap_AAS_TraceAreas(start, end, areas, NULL, 10);

			if (!numareas) {
				return qfalse;
			}

			goal->areanum = areas[0];
			return qtrue;
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
BotSetupDeathmatchAI
=======================================================================================================================================
*/
void BotSetupDeathmatchAI(void) {
	int ent, modelnum;
	char model[128];

	gametype = trap_Cvar_VariableIntegerValue("g_gametype");

	trap_Cvar_Register(&g_spSkill, "g_spSkill", "2", 0);

	if (gametype == GT_CTF) {
		if (trap_BotGetLevelItemGoal(-1, "Red Flag", &ctf_redflag) < 0) {
			BotAI_Print(PRT_WARNING, "CTF without Red Flag\n");
		}

		if (trap_BotGetLevelItemGoal(-1, "Blue Flag", &ctf_blueflag) < 0) {
			BotAI_Print(PRT_WARNING, "CTF without Blue Flag\n");
		}
	} else if (gametype == GT_1FCTF) {
		if (trap_BotGetLevelItemGoal(-1, "Neutral Flag", &ctf_neutralflag) < 0) {
			BotAI_Print(PRT_WARNING, "One Flag CTF without Neutral Flag\n");
		}

		if (trap_BotGetLevelItemGoal(-1, "Red Flag", &ctf_redflag) < 0) {
			BotAI_Print(PRT_WARNING, "One Flag CTF without Red Flag\n");
		}

		if (trap_BotGetLevelItemGoal(-1, "Blue Flag", &ctf_blueflag) < 0) {
			BotAI_Print(PRT_WARNING, "One Flag CTF without Blue Flag\n");
		}
	} else if (gametype == GT_OBELISK) {
		if (trap_BotGetLevelItemGoal(-1, "Red Obelisk", &redobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Overload without Red Obelisk\n");
		}

		BotSetEntityNumForGoalWithActivator(&redobelisk, "team_redobelisk");

		if (trap_BotGetLevelItemGoal(-1, "Blue Obelisk", &blueobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Overload without Blue Obelisk\n");
		}

		BotSetEntityNumForGoalWithActivator(&blueobelisk, "team_blueobelisk");
	} else if (gametype == GT_HARVESTER) {
		if (trap_BotGetLevelItemGoal(-1, "Red Obelisk", &redobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Harvester without Red Obelisk\n");
		}

		BotSetEntityNumForGoalWithActivator(&redobelisk, "team_redobelisk");

		if (trap_BotGetLevelItemGoal(-1, "Blue Obelisk", &blueobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Harvester without Blue Obelisk\n");
		}

		BotSetEntityNumForGoalWithActivator(&blueobelisk, "team_blueobelisk");

		if (trap_BotGetLevelItemGoal(-1, "Neutral Obelisk", &neutralobelisk) < 0) {
			BotAI_Print(PRT_WARNING, "Harvester without Neutral Obelisk\n");
		}

		BotSetEntityNumForGoalWithActivator(&neutralobelisk, "team_neutralobelisk");
	}

	max_bspmodelindex = 0;

	for (ent = trap_AAS_NextBSPEntity(0); ent; ent = trap_AAS_NextBSPEntity(ent)) {
		if (!trap_AAS_ValueForBSPEpairKey(ent, "model", model, sizeof(model))) {
			continue;
		}

		if (model[0] == '*') {
			modelnum = atoi(model + 1);

			if (modelnum > max_bspmodelindex) {
				max_bspmodelindex = modelnum;
			}
		}
	}
	// initialize the waypoint heap
	BotInitWaypoints();
}

/*
=======================================================================================================================================
BotShutdownDeathmatchAI
=======================================================================================================================================
*/
void BotShutdownDeathmatchAI(void) {
	altroutegoals_setup = qfalse;
}
