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
 Both games player movement code. Takes a playerstate and a usercmd as input and returns a modifed playerstate.
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t *pm;
pml_t pml;
// movement parameters
float pm_stopspeed = 100.0f;
float pm_duckScale = 0.25f;
float pm_swimScale = 0.15f;
float pm_accelerate = 10.0f;
float pm_airaccelerate = 1.0f;
float pm_wateraccelerate = 4.0f;
float pm_flyaccelerate = 8.0f;
float pm_friction = 6.0f;
float pm_waterfriction = 1.0f;
float pm_spectatorfriction = 5.0f;

int c_pmove = 0;

/*
=======================================================================================================================================
PM_AddEvent
=======================================================================================================================================
*/
void PM_AddEvent(int newEvent) {
	BG_AddPredictableEventToPlayerstate(newEvent, 0, pm->ps, -1);
}

/*
=======================================================================================================================================
PM_AddTouchEnt
=======================================================================================================================================
*/
void PM_AddTouchEnt(int entityNum) {
	int i;

	if (entityNum == ENTITYNUM_WORLD) {
		return;
	}

	if (pm->numtouch >= MAXTOUCH) {
		return;
	}
	// see if it is already added
	for (i = 0; i < pm->numtouch; i++) {
		if (pm->touchents[i] == entityNum) {
			return;
		}
	}
	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
=======================================================================================================================================
PM_StartTorsoAnim
=======================================================================================================================================
*/
static void PM_StartTorsoAnim(int anim) {

	if (pm->ps->pm_type >= PM_DEAD) {
		return;
	}

	pm->ps->torsoAnim = ((pm->ps->torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)|anim;
}

/*
=======================================================================================================================================
PM_StartLegsAnim
=======================================================================================================================================
*/
static void PM_StartLegsAnim(int anim) {

	if (pm->ps->pm_type >= PM_DEAD) {
		return;
	}

	if (pm->ps->legsTimer > 0) {
		return; // a high priority animation is running
	}

	pm->ps->legsAnim = ((pm->ps->legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)|anim;
}

/*
=======================================================================================================================================
PM_ContinueLegsAnim
=======================================================================================================================================
*/
static void PM_ContinueLegsAnim(int anim) {

	if ((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == anim) {
		return;
	}

	if (pm->ps->legsTimer > 0) {
		return; // a high priority animation is running
	}

	PM_StartLegsAnim(anim);
}

/*
=======================================================================================================================================
PM_ContinueTorsoAnim
=======================================================================================================================================
*/
static void PM_ContinueTorsoAnim(int anim) {

	if ((pm->ps->torsoAnim & ~ANIM_TOGGLEBIT) == anim) {
		return;
	}

	if (pm->ps->torsoTimer > 0) {
		return; // a high priority animation is running
	}

	PM_StartTorsoAnim(anim);
}

/*
=======================================================================================================================================
PM_ForceLegsAnim
=======================================================================================================================================
*/
static void PM_ForceLegsAnim(int anim) {

	pm->ps->legsTimer = 0;
	PM_StartLegsAnim(anim);
}

/*
=======================================================================================================================================
PM_ClipVelocity

Slide off of the impacting surface.
=======================================================================================================================================
*/
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce) {
	float backoff;
	float change;
	int i;

	backoff = DotProduct(in, normal);

	if (backoff < 0) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for (i = 0; i < 3; i++) {
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}
}

/*
=======================================================================================================================================
PM_Friction

Handles both ground friction and water friction.
=======================================================================================================================================
*/
static void PM_Friction(void) {
	vec3_t vec;
	float *vel;
	float speed, newspeed, control;
	float drop;

	vel = pm->ps->velocity;

	VectorCopy(vel, vec);

	if (pml.walking) {
		vec[2] = 0; // ignore slope movement
	}

	speed = VectorLength(vec);

	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0; // allow sinking underwater
		// FIXME: still have z friction underwater?
		if (pm->ps->pm_type == PM_SPECTATOR) {
			vel[2] = 0.0f; // no slow sinking/raising movements
		}

		return;
	}

	drop = 0;
	// apply ground friction
	if (pm->waterlevel <= 1) {
		if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK)) {
			// if getting knocked back, no friction
			if (!(pm->ps->pm_flags & PMF_TIME_KNOCKBACK)) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control * pm_friction * pml.frametime;
			}
		}
	}
	// apply water friction even if just wading
	if (pm->waterlevel) {
		drop += speed * pm_waterfriction * pm->waterlevel * pml.frametime;
	}

	if (pm->ps->pm_type == PM_SPECTATOR) {
		drop += speed * pm_spectatorfriction * pml.frametime;
	}
	// scale the velocity
	newspeed = speed - drop;

	if (newspeed < 0) {
		newspeed = 0;
	}

	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}

/*
=======================================================================================================================================
PM_Accelerate

Handles user intended acceleration.
=======================================================================================================================================
*/
static void PM_Accelerate(vec3_t wishdir, float wishspeed, float accel) {
#if 1
	// q2 style
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct(pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0) {
		return;
	}

	accelspeed = accel * pml.frametime * wishspeed;

	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++) {
		pm->ps->velocity[i] += accelspeed * wishdir[i];
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t wishVelocity;
	vec3_t pushDir;
	float pushLen;
	float canPush;

	VectorScale(wishdir, wishspeed, wishVelocity);
	VectorSubtract(wishVelocity, pm->ps->velocity, pushDir);

	pushLen = VectorNormalize(pushDir);
	canPush = accel * pml.frametime * wishspeed;

	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA(pm->ps->velocity, canPush, pushDir, pm->ps->velocity);
#endif
}

/*
=======================================================================================================================================
PM_CmdScale

Returns the scale factor to apply to cmd movements.
This allows the clients to use axial -127 to 127 values for all directions without getting a sqrt(2) distortion in speed.
=======================================================================================================================================
*/
static float PM_CmdScale(usercmd_t *cmd) {
	int max;
	float total;
	float scale;

	max = abs(cmd->forwardmove);

	if (abs(cmd->rightmove) > max) {
		max = abs(cmd->rightmove);
	}

	if (abs(cmd->upmove) > max) {
		max = abs(cmd->upmove);
	}

	if (!max) {
		return 0;
	}

	total = sqrt(cmd->forwardmove * cmd->forwardmove + cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove);
	scale = (float)pm->ps->speed * max / (127.0 * total);
	// ignore if in air
	if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {
		return scale;
	}
	// ignore if spectator
	if (pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return scale;
	}
	// apply speed scale for strafing and going backwards
	if (cmd->forwardmove < 0) {
		scale *= 0.75f;
	} else if (cmd->rightmove) {
		scale *= 0.9f;
	}
	// running
	if (!(pm->cmd.buttons & BUTTON_WALKING)) {
		// apply weapon speed scale
		switch (pm->ps->weapon) {
			case WP_GAUNTLET:
				scale *= 1.15f;
				break;
			case WP_RAILGUN:
			case WP_BFG:
				scale *= 0.9f;
				break;
			default:
				break;
		}
	// walking
	} else {
		scale *= 0.75f;
	}

	return scale;
}

/*
=======================================================================================================================================
PM_SetMovementDir

Determine the rotation of the legs relative to the facing dir.
=======================================================================================================================================
*/
static void PM_SetMovementDir(void) {

	if (pm->cmd.forwardmove || pm->cmd.rightmove) {
		if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0) {
			pm->ps->movementDir = 0;
		} else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0) {
			pm->ps->movementDir = 1;
		} else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0) {
			pm->ps->movementDir = 2;
		} else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0) {
			pm->ps->movementDir = 3;
		} else if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0) {
			pm->ps->movementDir = 4;
		} else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0) {
			pm->ps->movementDir = 5;
		} else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0) {
			pm->ps->movementDir = 6;
		} else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0) {
			pm->ps->movementDir = 7;
		}
	} else {
		// if they aren't actively going directly sideways, change the animation to the diagonal so they don't stop too crooked
		if (pm->ps->movementDir == 2) {
			pm->ps->movementDir = 1;
		} else if (pm->ps->movementDir == 6) {
			pm->ps->movementDir = 7;
		}
	}
}

/*
=======================================================================================================================================
PM_CheckJump
=======================================================================================================================================
*/
static qboolean PM_CheckJump(void) {

	if (pm->ps->pm_flags & PMF_RESPAWNED) {
		return qfalse; // don't allow jump until all buttons are up
	}

	if (pm->cmd.upmove < 10) {
		// not holding jump
		return qfalse;
	}
	// must wait for jump to be released
	if (pm->ps->pm_flags & PMF_JUMP_HELD) {
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	pml.groundPlane = qfalse; // jumping away
	pml.walking = qfalse;
	pm->ps->pm_flags |= PMF_JUMP_HELD;
	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pm->ps->velocity[2] = JUMP_VELOCITY;

	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
		pm->ps->velocity[2] *= SCOUT_SPEED_SCALE;
	}

	PM_AddEvent(EV_JUMP);

	if (pm->cmd.forwardmove >= 0) {
		PM_ForceLegsAnim(LEGS_JUMP);
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	} else {
		PM_ForceLegsAnim(LEGS_JUMPB);
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	return qtrue;
}

/*
=======================================================================================================================================
PM_CheckWaterJump
=======================================================================================================================================
*/
static qboolean PM_CheckWaterJump(void) {
	vec3_t spot;
	int cont;
	vec3_t flatforward;

	if (pm->ps->pm_time) {
		return qfalse;
	}
	// check for water jump
	if (pm->waterlevel != 2) {
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;

	VectorNormalize(flatforward);
	VectorMA(pm->ps->origin, 30, flatforward, spot);

	spot[2] += 6; // Tobias CHECK: compensate for the new viewheight to get out of water with ease again (but why do other games not need this?), AND I think this fixes the issue with bots hanging around in water (in q3dm12 BFG room).
	cont = pm->pointcontents(spot, pm->ps->clientNum);

	if (!(cont & CONTENTS_SOLID)) {
		return qfalse;
	}

	spot[2] += 24; // Tobias CHECK: compensate for the new viewheight to get out of water with ease again (but why do other games not need this?), AND I think this fixes the issue with bots hanging around in water (in q3dm12 BFG room).
	cont = pm->pointcontents(spot, pm->ps->clientNum);

	if (cont & (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)) {
		return qfalse;
	}
	// jump out of water
	VectorScale(pml.forward, 200, pm->ps->velocity);

	pm->ps->velocity[2] = 350;
	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

/*
=======================================================================================================================================
PM_WaterJumpMove

Flying out of the water.
=======================================================================================================================================
*/
static void PM_WaterJumpMove(void) {

	// waterjump has no control, but falls
	PM_StepSlideMove(qtrue);

	pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;

	if (pm->ps->velocity[2] < 0) {
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
=======================================================================================================================================
PM_WaterMove
=======================================================================================================================================
*/
static void PM_WaterMove(void) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;
	float vel;

	if (PM_CheckWaterJump()) {
		PM_WaterJumpMove();
		return;
	}

	if (pm->waterlevel == 3) {
		// jump = head for surface
		if (pm->cmd.upmove >= 10) {
			if (pm->ps->velocity[2] > -300) {
				if (pm->watertype & CONTENTS_LAVA) {
					pm->ps->velocity[2] = 10;
				} else {
					pm->ps->velocity[2] = 50;
				}
			}
		}
	}

	PM_Friction();

	scale = PM_CmdScale(&pm->cmd);
	// user intentions
	if (!scale) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		// sink towards bottom
		if (pm->watertype & CONTENTS_LAVA) {
			wishvel[2] = -10;
		} else {
			wishvel[2] = -60;
		}
	} else {
		for (i = 0; i < 3; i++) {
			wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > pm->ps->speed * pm_swimScale) {
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	if (pm->watertype & CONTENTS_LAVA) {
		wishspeed *= 0.5;
	}
	// make sure we can go up slopes easily under water
	if (pml.groundPlane && DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0) {
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
		// don't decrease velocity when going up or down a slope
		if (VectorLength(pm->ps->velocity) > 1) {
			VectorNormalize(pm->ps->velocity);
			VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
		}
	}

	PM_SlideMove(qfalse);
	PM_Accelerate(wishdir, wishspeed, pm_wateraccelerate);
}

/*
=======================================================================================================================================
PM_FlyMove
=======================================================================================================================================
*/
static void PM_FlyMove(void) {
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;

	// normal slowdown
	PM_Friction();

	scale = PM_CmdScale(&pm->cmd);
	// user intentions
	if (!scale) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for (i = 0; i < 3; i++) {
			wishvel[i] = scale * pml.forward[i] * pm->cmd.forwardmove + scale * pml.right[i] * pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate(wishdir, wishspeed, pm_flyaccelerate);
	PM_StepSlideMove(qfalse);
}

/*
=======================================================================================================================================
PM_AirMove
=======================================================================================================================================
*/
static void PM_AirMove(void) {
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	cmd = pm->cmd;
	scale = PM_CmdScale(&cmd);
	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();
	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;

	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 2; i++) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	wishvel[2] = 0;

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;
	// we may have a ground plane that is very steep, even though we don't have a groundentity
	// slide along the steep plane
	if (pml.groundPlane) {
		PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
	}

	PM_StepSlideMove(qtrue);
	// not on ground, so little effect on velocity
	PM_Accelerate(wishdir, wishspeed, pm_airaccelerate);
}

/*
=======================================================================================================================================
PM_WalkMove
=======================================================================================================================================
*/
static void PM_WalkMove(void) {
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;
	usercmd_t cmd;
	float accelerate;
	float vel;

	if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0) {
		// begin swimming
		PM_WaterMove();
		return;
	}

	if (PM_CheckJump()) {
		// jumped away
		if (pm->waterlevel > 1) {
			PM_WaterMove();
		} else {
			PM_AirMove();
		}

		return;
	}

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	cmd = pm->cmd;
	scale = PM_CmdScale(&cmd);
	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();
	// project moves down to flat plane
	pml.forward[2] = 0;
	pml.right[2] = 0;
	// project the forward and right directions onto the ground plane
	PM_ClipVelocity(pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
	PM_ClipVelocity(pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);

	VectorNormalize(pml.forward);
	VectorNormalize(pml.right);

	for (i = 0; i < 3; i++) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
	//wishvel[2] = 0;

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;
	// clamp the speed lower if ducking
	if (pm->ps->pm_flags & PMF_DUCKED) {
		if (wishspeed > pm->ps->speed * pm_duckScale) {
			wishspeed = pm->ps->speed * pm_duckScale;
		}
	}
	// clamp the speed lower if wading or walking on the bottom
	if (pm->waterlevel) {
		float waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - (1.0 - pm_swimScale) * waterScale;

		if (wishspeed > pm->ps->speed * waterScale) {
			wishspeed = pm->ps->speed * waterScale;
		}
	}
	// when a player gets hit, they temporarily lose full control, which allows them to be moved a bit
	if ((pml.groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
		accelerate = pm_airaccelerate;
	} else {
		accelerate = pm_accelerate;
	}

	PM_Accelerate(wishdir, wishspeed, accelerate);
	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ((pml.groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK) {
		pm->ps->velocity[2] -= pm->ps->gravity * pml.frametime;
	} else {
		// don't reset the z velocity for slopes
		//pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);
	// slide along the ground plane
	PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal, pm->ps->velocity, OVERCLIP);
	// don't decrease velocity when going up or down a slope
	if (VectorLength(pm->ps->velocity) > 1) {
		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}
	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1]) {
		return;
	}

	PM_StepSlideMove(qfalse);
	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));
}

/*
=======================================================================================================================================
PM_DeadMove
=======================================================================================================================================
*/
static void PM_DeadMove(void) {
	float forward;

	if (!pml.walking) {
		return;
	}
	// extra friction
	forward = VectorLength(pm->ps->velocity);
	forward -= 20;

	if (forward <= 0) {
		VectorClear(pm->ps->velocity);
	} else {
		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, forward, pm->ps->velocity);
	}
}

/*
=======================================================================================================================================
PM_NoclipMove
=======================================================================================================================================
*/
static void PM_NoclipMove(void) {
	float speed, drop, friction, control, newspeed;
	int i;
	vec3_t wishvel;
	float fmove, smove;
	vec3_t wishdir;
	float wishspeed;
	float scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	// friction
	speed = VectorLength(pm->ps->velocity);

	if (speed < 1) {
		VectorCopy(vec3_origin, pm->ps->velocity);
	} else {
		drop = 0;
		friction = pm_friction * 1.5; // extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control * friction * pml.frametime;
		// scale the velocity
		newspeed = speed - drop;

		if (newspeed < 0) {
			newspeed = 0;
		}

		newspeed /= speed;

		VectorScale(pm->ps->velocity, newspeed, pm->ps->velocity);
	}
	// accelerate
	scale = PM_CmdScale(&pm->cmd);
	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	for (i = 0; i < 3; i++) {
		wishvel[i] = pml.forward[i] * fmove + pml.right[i] * smove;
	}

	wishvel[2] += pm->cmd.upmove;

	VectorCopy(wishvel, wishdir);

	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate(wishdir, wishspeed, pm_accelerate);
	// move
	VectorMA(pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

/*
=======================================================================================================================================
PM_FootstepForSurface

Returns an event number appropriate for the groundsurface.
=======================================================================================================================================
*/
static int PM_FootstepForSurface(void) {

	if (pml.groundTrace.surfaceFlags & SURF_NOSTEPS) {
		return 0;
	}

	switch (pml.groundTrace.surfaceFlags & SURF_MATERIAL_MASK) {
		default:
		// sound defaults to hard, dry materials
/*
		case MAT_NONE:
		case MAT_STONE_GR_COL_01:
		case MAT_STONE_GR_COL_02:
		case MAT_STONE_GR_COL_03:
		case MAT_STONE_GR_COL_04:
		case MAT_STONE_HOT:
		case MAT_LAVACRACKS:
		case MAT_TILES_BROWN:
		case MAT_TILES_CYAN:
		case MAT_TILES_GREEN:
		case MAT_TILES_GREY:
		case MAT_TILES_RED:
		case MAT_TILES_YELLOW:
		case MAT_TILES_WHITE:
		case MAT_CONCRETE:
		case MAT_ASPHALT:
		case MAT_BRICK_BLACK:
		case MAT_BRICK_BROWN:
		case MAT_BRICK_GREY:
		case MAT_BRICK_RED:
		case MAT_METAL_SOLID:
		case MAT_METAL_SOLID_PAINTED:
		case MAT_POT:
		case MAT_INSULATION_01:
		case MAT_INSULATION_02:
		case MAT_WALLPAPER:
		case MAT_GLASS:
		case MAT_PLASTIC_HARD:
		case MAT_COMPUTER:
		case MAT_BONES:
		case MAT_PORCELAIN:
*/
			return EV_FOOTSTEP_HARD;
		case MAT_STONE_FROZEN:
		case MAT_TILES_BROWN_FROZEN:
		case MAT_TILES_CYAN_FROZEN:
		case MAT_TILES_GREEN_FROZEN:
		case MAT_TILES_GREY_FROZEN:
		case MAT_TILES_RED_FROZEN:
		case MAT_TILES_YELLOW_FROZEN:
		case MAT_TILES_WHITE_FROZEN:
		case MAT_CONCRETE_FROZEN:
		case MAT_ASPHALT_FROZEN:
		case MAT_METAL_SOLID_FROZEN:
		case MAT_WOOD_SOLID_DARK_FROZEN:
			return EV_FOOTSTEP_HARD_FROZEN;
		case MAT_STONE_SNOW:
		case MAT_TILES_BROWN_SNOW:
		case MAT_TILES_CYAN_SNOW:
		case MAT_TILES_GREEN_SNOW:
		case MAT_TILES_GREY_SNOW:
		case MAT_TILES_RED_SNOW:
		case MAT_TILES_YELLOW_SNOW:
		case MAT_TILES_WHITE_SNOW:
		case MAT_CONCRETE_SNOW:
		case MAT_ASPHALT_SNOW:
		case MAT_METAL_SOLID_SNOW:
		case MAT_WOOD_SOLID_DARK_SNOW:
		case MAT_SNOW_GR_COL_01:
		case MAT_SNOW_GR_COL_02:
		case MAT_SNOW_GR_COL_03:
		case MAT_SNOW_GR_COL_04:
			return EV_FOOTSTEP_HARD_SNOW;
		case MAT_STONE_SLUSH:
		case MAT_TILES_BROWN_SLUSH:
		case MAT_TILES_CYAN_SLUSH:
		case MAT_TILES_GREEN_SLUSH:
		case MAT_TILES_GREY_SLUSH:
		case MAT_TILES_RED_SLUSH:
		case MAT_TILES_YELLOW_SLUSH:
		case MAT_TILES_WHITE_SLUSH:
		case MAT_CONCRETE_SLUSH:
		case MAT_ASPHALT_SLUSH:
		case MAT_METAL_SOLID_SLUSH:
		case MAT_WOOD_SOLID_DARK_SLUSH:
			return EV_FOOTSTEP_HARD_SLUSH;
		case MAT_STONE_SPLASH:
		case MAT_TILES_BROWN_SPLASH:
		case MAT_TILES_CYAN_SPLASH:
		case MAT_TILES_GREEN_SPLASH:
		case MAT_TILES_GREY_SPLASH:
		case MAT_TILES_RED_SPLASH:
		case MAT_TILES_YELLOW_SPLASH:
		case MAT_TILES_WHITE_SPLASH:
		case MAT_CONCRETE_SPLASH:
		case MAT_ASPHALT_SPLASH:
		case MAT_METAL_SOLID_SPLASH:
		case MAT_WOOD_SOLID_DARK_SPLASH:
		case MAT_PUDDLE_GR_COL_01:
		case MAT_PUDDLE_GR_COL_02:
		case MAT_PUDDLE_GR_COL_03:
		case MAT_PUDDLE_GR_COL_04:
			return EV_FOOTSTEP_PUDDLE;
		case MAT_LEAVES_01_GR_COL_01:
		case MAT_LEAVES_01_GR_COL_02:
		case MAT_LEAVES_01_GR_COL_03:
		case MAT_LEAVES_01_GR_COL_04:
			return EV_FOOTSTEP_LEAVES;
		case MAT_BUSH_01_GR_COL_01:
		case MAT_BUSH_01_GR_COL_02:
		case MAT_BUSH_01_GR_COL_03:
		case MAT_BUSH_01_GR_COL_04:
		case MAT_BUSH_02_GR_COL_01:
		case MAT_BUSH_02_GR_COL_02:
		case MAT_BUSH_02_GR_COL_03:
		case MAT_BUSH_02_GR_COL_04:
			return EV_FOOTSTEP_BUSH;
		case MAT_SHORTGRASS_01_GR_COL_01:
		case MAT_SHORTGRASS_01_GR_COL_02:
		case MAT_SHORTGRASS_01_GR_COL_03:
		case MAT_SHORTGRASS_01_GR_COL_04:
		case MAT_SHORTGRASS_02_GR_COL_01:
		case MAT_SHORTGRASS_02_GR_COL_02:
		case MAT_SHORTGRASS_02_GR_COL_03:
		case MAT_SHORTGRASS_02_GR_COL_04:
			return EV_FOOTSTEP_GRASS;
		case MAT_LONGGRASS_01_GR_COL_01:
		case MAT_LONGGRASS_01_GR_COL_02:
		case MAT_LONGGRASS_01_GR_COL_03:
		case MAT_LONGGRASS_01_GR_COL_04:
		case MAT_LONGGRASS_02_GR_COL_01:
		case MAT_LONGGRASS_02_GR_COL_02:
		case MAT_LONGGRASS_02_GR_COL_03:
		case MAT_LONGGRASS_02_GR_COL_04:
			return EV_FOOTSTEP_LONGGRASS;
		case MAT_LONGGRASS_MUD_GR_COL_01:
		case MAT_LONGGRASS_MUD_GR_COL_02:
		case MAT_LONGGRASS_MUD_GR_COL_03:
		case MAT_LONGGRASS_MUD_GR_COL_04:
			return EV_FOOTSTEP_LONGGRASS_MUD;
		case MAT_SAND_GR_COL_01:
		case MAT_SAND_GR_COL_02:
		case MAT_SAND_GR_COL_03:
		case MAT_SAND_GR_COL_04:
			return EV_FOOTSTEP_SAND;
		case MAT_GRAVEL_GR_COL_01:
		case MAT_GRAVEL_GR_COL_02:
		case MAT_GRAVEL_GR_COL_03:
		case MAT_GRAVEL_GR_COL_04:
			return EV_FOOTSTEP_GRAVEL;
		case MAT_RUBBLE_GR_COL_01:
		case MAT_RUBBLE_GR_COL_02:
		case MAT_RUBBLE_GR_COL_03:
		case MAT_RUBBLE_GR_COL_04:
			return EV_FOOTSTEP_RUBBLE;
		case MAT_RUBBLE_WET_GR_COL_01:
		case MAT_RUBBLE_WET_GR_COL_02:
		case MAT_RUBBLE_WET_GR_COL_03:
		case MAT_RUBBLE_WET_GR_COL_04:
			return EV_FOOTSTEP_RUBBLE_WET;
		case MAT_SOIL_GR_COL_01:
		case MAT_SOIL_GR_COL_02:
		case MAT_SOIL_GR_COL_03:
		case MAT_SOIL_GR_COL_04:
			return EV_FOOTSTEP_SOIL;
		case MAT_MUD_GR_COL_01:
		case MAT_MUD_GR_COL_02:
		case MAT_MUD_GR_COL_03:
		case MAT_MUD_GR_COL_04:
			return EV_FOOTSTEP_MUD;
		case MAT_SNOW_DEEP_GR_COL_01:
		case MAT_SNOW_DEEP_GR_COL_02:
		case MAT_SNOW_DEEP_GR_COL_03:
		case MAT_SNOW_DEEP_GR_COL_04:
			return EV_FOOTSTEP_SNOW_DEEP;
		case MAT_ICE:
			return EV_FOOTSTEP_ICE;
		case MAT_METAL_HOLLOW:
		case MAT_METAL_HOLLOW_PAINTED:
		case MAT_METAL_COPPER:
		case MAT_BARREL:
			return EV_FOOTSTEP_METAL_HOLLOW;
		case MAT_METAL_HOLLOW_FROZEN:
			return EV_FOOTSTEP_METAL_HOLLOW_FROZEN;
		case MAT_METAL_HOLLOW_SNOW:
			return EV_FOOTSTEP_METAL_HOLLOW_SNOW;
		case MAT_METAL_HOLLOW_SLUSH:
			return EV_FOOTSTEP_METAL_HOLLOW_SLUSH;
		case MAT_METAL_HOLLOW_SPLASH:
			return EV_FOOTSTEP_METAL_HOLLOW_SPLASH;
		case MAT_GRATE_01:
			return EV_FOOTSTEP_GRATE_01;
		case MAT_GRATE_02:
			return EV_FOOTSTEP_GRATE_02;
		case MAT_DUCT:
			return EV_FOOTSTEP_DUCT;
		case MAT_PLATE:
			return EV_FOOTSTEP_PLATE;
		case MAT_FENCE:
			return EV_FOOTSTEP_FENCE;
		case MAT_WOOD_HOLLOW_DARK:
		case MAT_WOOD_HOLLOW_BRIGHT:
		case MAT_SHINGLES_WOOD:
			return EV_FOOTSTEP_WOOD_HOLLOW;
		case MAT_WOOD_HOLLOW_FROZEN:
			return EV_FOOTSTEP_WOOD_HOLLOW_FROZEN;
		case MAT_WOOD_HOLLOW_SNOW:
			return EV_FOOTSTEP_WOOD_HOLLOW_SNOW;
		case MAT_WOOD_HOLLOW_SLUSH:
			return EV_FOOTSTEP_WOOD_HOLLOW_SLUSH;
		case MAT_WOOD_HOLLOW_SPLASH:
			return EV_FOOTSTEP_WOOD_HOLLOW_SPLASH;
		case MAT_WOOD_SOLID_DARK:
		case MAT_WOOD_SOLID_BRIGHT:
			return EV_FOOTSTEP_WOOD_SOLID;
		case MAT_WOOD_CREAKING_DARK:
		case MAT_WOOD_CREAKING_BRIGHT:
			return EV_FOOTSTEP_WOOD_CREAKING;
		case MAT_ROOF:
			return EV_FOOTSTEP_ROOF;
		case MAT_SHINGLES_CERAMIC_RED:
		case MAT_SHINGLES_CERAMIC_GREY:
			return EV_FOOTSTEP_SHINGLES;
		case MAT_TEXTILES:
		case MAT_CARPET:
		case MAT_PLASTIC_SOFT:
		case MAT_CANVAS:
		case MAT_RUBBER:
		case MAT_FLESH:
			return EV_FOOTSTEP_SOFT;
		case MAT_GLASS_SHARDS:
			return EV_FOOTSTEP_GLASS_SHARDS;
		case MAT_TRASH_GLASS:
			return EV_FOOTSTEP_TRASH_GLASS;
		case MAT_TRASH_DEBRIS:
			return EV_FOOTSTEP_TRASH_DEBRIS;
		case MAT_TRASH_WIRE:
			return EV_FOOTSTEP_TRASH_WIRE;
		case MAT_TRASH_PACKING:
			return EV_FOOTSTEP_TRASH_PACKING;
		case MAT_TRASH_PLASTIC:
			return EV_FOOTSTEP_TRASH_PLASTIC;
	}
}

/*
=======================================================================================================================================
PM_CrashLand

Check for hard landings that generate sound events.
=======================================================================================================================================
*/
static void PM_CrashLand(void) {
	float delta;
	float dist;
	float vel, acc;
	float t;
	float a, b, c, den;
	int stunTime;

	stunTime = 0;
	// decide which landing animation to use
	if (pm->ps->pm_flags & PMF_BACKWARDS_JUMP) {
		PM_ForceLegsAnim(LEGS_LANDB);
	} else {
		PM_ForceLegsAnim(LEGS_LAND);
	}

	pm->ps->legsTimer = TIMER_LAND;
	// calculate the exact velocity on landing
	dist = pm->ps->origin[2] - pml.previous_origin[2];
	vel = pml.previous_velocity[2];
	acc = -pm->ps->gravity;
	a = acc * 0.5;
	b = vel;
	c = -dist;
	den = b * b - 4 * a * c;

	if (den < 0) {
		return;
	}

	t = (-b - sqrt(den)) / (2 * a);
	delta = vel + t * acc;
	delta = delta * delta * 0.0001;
	// ducking while falling doubles damage
	if (pm->ps->pm_flags & PMF_DUCKED) {
		delta *= 2;
	}
	// never take falling damage if completely underwater
	if (pm->waterlevel == 3) {
		return;
	}
	// reduce falling damage if there is standing water
	if (pm->waterlevel == 2) {
		delta *= 0.85;
	}
	// the scout powerup also reduces falling damage
	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
		delta *= 0.9;
	}

	if (delta < 1) {
		return;
	}
	// create a local entity event to play the sound
	// SURF_NODAMAGE is used for bounce pads where you don't want to take full damage or play a crunch sound
	if (!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE)) {
		// create a local entity event to play the sound
		if (delta > 84) { // Tobias NOTE: a delta of 84 = max_fallheight of 516 units for bots, and 529 units for humans (why can humans fall higher than bots?)
			PM_AddEvent(EV_FALL_DIE);
			stunTime = 1000;
		} else if (delta > 70) {
			PM_AddEvent(EV_FALL_DMG_50);
			stunTime = 1000;
		} else if (delta > 58) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_25);
			}

			stunTime = 250;
		} else if (delta > 48) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_15);
			}

			stunTime = 1000;
		} else if (delta > 38.75) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_10);
			}

			stunTime = 1000;
		} else if (delta > 28) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_5);
			}

			stunTime = 1000;
		} else if (delta > 7) {
			PM_AddEvent(EV_FALL_SHORT);
		} else {
			PM_AddEvent(PM_FootstepForSurface());
		}
	// Tobias NOTE: this simulates the old behavior, assuming old maps use SURF_NODAMAGE if needed
	} else {
		if (delta > 60) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_10);
			}

			stunTime = 1000;
		} else if (delta > 40) {
			// this is a pain grunt, so don't play it if dead
			if (pm->ps->stats[STAT_HEALTH] > 0) {
				PM_AddEvent(EV_FALL_DMG_5);
			}

			stunTime = 1000;
		} else if (delta > 7) {
			PM_AddEvent(EV_FALL_SHORT);
		} else {
			PM_AddEvent(PM_FootstepForSurface());
		}
	// Tobias END
	}
	// when landing from launch ramps don't stop so abruptly
	if (VectorLengthSquared(pm->ps->velocity) > 160000) {
		stunTime = 0;
	}

	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT) {
		stunTime = 0;
	}
	// when falling damage happens, velocity is cleared, but this needs to happen in pmove, not g_active (prediction will be wrong, otherwise)!
	if (stunTime) {
		pm->ps->pm_time = stunTime;
		pm->ps->pm_flags |= PMF_TIME_KNOCKBACK;
		VectorClear(pm->ps->velocity);
	}
	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=======================================================================================================================================
PM_CheckStuck
=======================================================================================================================================
*/
/*
void PM_CheckStuck(void) {
	trace_t trace;

	pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

	if (trace.allsolid) {
		//int shit = qtrue;
	}
}
*/
/*
=======================================================================================================================================
PM_CorrectAllSolid
=======================================================================================================================================
*/
static int PM_CorrectAllSolid(trace_t *trace) {
	int i, j, k;
	vec3_t point;

	if (pm->debugLevel) {
		Com_Printf("%i:allsolid\n", c_pmove);
	}
	// jitter around
	for (i = -1; i <= 1; i++) {
		for (j = -1; j <= 1; j++) {
			for (k = -1; k <= 1; k++) {
				VectorCopy(pm->ps->origin, point);

				point[0] += (float)i;
				point[1] += (float)j;
				point[2] += (float)k;

				pm->trace(trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);

				if (!trace->allsolid) {
					point[0] = pm->ps->origin[0];
					point[1] = pm->ps->origin[1];
					point[2] = pm->ps->origin[2] - 0.25;

					pm->trace(trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;
					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}

/*
=======================================================================================================================================
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall.
=======================================================================================================================================
*/
static void PM_GroundTraceMissed(void) {
	trace_t trace;
	vec3_t point;

	if (pm->ps->groundEntityNum != ENTITYNUM_NONE) {
		// we just transitioned into freefall
		if (pm->debugLevel) {
			Com_Printf("%i:lift\n", c_pmove);
		}
		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy(pm->ps->origin, point);

		point[2] -= 64;

		pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);

		if (trace.fraction == 1.0f) {
			if (pm->cmd.forwardmove >= 0) {
				PM_ForceLegsAnim(LEGS_JUMP);
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			} else {
				PM_ForceLegsAnim(LEGS_JUMPB);
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}

/*
=======================================================================================================================================
PM_GroundTrace
=======================================================================================================================================
*/
static void PM_GroundTrace(void) {
	vec3_t point;
	trace_t trace;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] - 0.25;

	pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;
	// do something corrective if the trace starts in a solid...
	if (trace.allsolid) {
		if (!PM_CorrectAllSolid(&trace)) {
			return;
		}
	}
	// if the trace didn't hit anything, we are in free fall
	if (trace.fraction == 1.0f) {
		PM_GroundTraceMissed();
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	// check if getting thrown off the ground
	if (pm->ps->velocity[2] > 0 && DotProduct(pm->ps->velocity, trace.plane.normal) > 10) {
		if (pm->debugLevel) {
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
		if (pm->cmd.forwardmove >= 0) {
			PM_ForceLegsAnim(LEGS_JUMP);
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		} else {
			PM_ForceLegsAnim(LEGS_JUMPB);
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}
	// slopes that are too steep will not be considered onground
	if (trace.plane.normal[2] < MIN_WALK_NORMAL) {
		if (pm->debugLevel) {
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	pml.walking = qtrue;
	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP|PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {
		// just hit the ground
		if (pm->debugLevel) {
			Com_Printf("%i:Land\n", c_pmove);
		}

		PM_CrashLand();
		// don't do landing time if we were just going down a slope
		if (pml.previous_velocity[2] < -200) {
			// don't allow another jump for a little while
			pm->ps->pm_flags |= PMF_TIME_LAND;
			pm->ps->pm_time = 250;
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;
	// don't reset the z velocity for slopes
	//pm->ps->velocity[2] = 0;

	PM_AddTouchEnt(trace.entityNum);
}

/*
=======================================================================================================================================
PM_SetWaterLevel

FIXME: Avoid this twice? Certainly if not moving.
=======================================================================================================================================
*/
static void PM_SetWaterLevel(void) {
	vec3_t point;
	int cont;
	int sample1;
	int sample2;

	// get waterlevel, accounting for ducking
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;
	cont = pm->pointcontents(point, pm->ps->clientNum);

	if (cont & MASK_WATER) {
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 * 0.5;
		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents(point, pm->ps->clientNum);

		if (cont & MASK_WATER) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents(point, pm->ps->clientNum);

			if (cont & MASK_WATER) {
				pm->waterlevel = 3;
			}
		}
	}
}

/*
=======================================================================================================================================
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight.
=======================================================================================================================================
*/
static void PM_CheckDuck(void) {
	trace_t trace;

	pm->mins[0] = -15;
	pm->mins[1] = -15;
	pm->maxs[0] = 15;
	pm->maxs[1] = 15;
	pm->mins[2] = MINS_Z;

	if (pm->ps->pm_type == PM_DEAD) {
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return;
	}

	if (pm->cmd.upmove < 0) { // duck
		pm->ps->pm_flags |= PMF_DUCKED;
	} else { // stand up if possible
		if (pm->ps->pm_flags & PMF_DUCKED) {
			// try to stand up
			pm->maxs[2] = 56; // 56 + 24 = 80 (80 * 2.5 = 200)
			pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

			if (!trace.allsolid) {
				pm->ps->pm_flags &= ~PMF_DUCKED;
			}
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED) {
		pm->maxs[2] = 42;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	} else {
		pm->maxs[2] = 56; // 56 + 24 = 80 (80 * 2.5 = 200)
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}
}

/*
=======================================================================================================================================
PM_Footsteps
=======================================================================================================================================
*/
static void PM_Footsteps(void) {
	float bobmove;
	int old;
	qboolean footstep;

	// calculate speed and cycle to be used for all cyclic walking effects
	pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0] + pm->ps->velocity[1] * pm->ps->velocity[1]);
	// in the air
	if (pm->ps->groundEntityNum == ENTITYNUM_NONE) {
		// airborne leaves position in cycle intact, but doesn't advance
		if (pm->waterlevel > 1) {
			PM_ContinueLegsAnim(LEGS_SWIM);
		}

		return;
	}
	// if not trying to move
	if (!pm->cmd.forwardmove && !pm->cmd.rightmove) {
		if (pm->xyspeed < 5) {
			pm->ps->bobCycle = 0; // start at beginning of cycle again

			if (pm->ps->pm_flags & PMF_DUCKED) {
				PM_ContinueLegsAnim(LEGS_IDLECR);
			} else {
				PM_ContinueLegsAnim(LEGS_IDLE);
			}
		}

		return;
	}

	footstep = qfalse;
	// ducked
	if (pm->ps->pm_flags & PMF_DUCKED) {
		bobmove = 0.5; // 0.65f

		if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
			PM_ContinueLegsAnim(LEGS_BACKCR);
		} else {
			PM_ContinueLegsAnim(LEGS_WALKCR);
		}
		// ducked characters never play footsteps
	/*
	} else if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
		if (!(pm->cmd.buttons & BUTTON_WALKING)) {
			bobmove = 0.4; // faster speeds bob faster
			footstep = qtrue;
		} else {
			bobmove = 0.3;
		}

		PM_ContinueLegsAnim(LEGS_BACK);
	*/
	} else {
		if (!(pm->cmd.buttons & BUTTON_WALKING)) {
			bobmove = 0.4f; // faster speeds bob faster

			if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
				PM_ContinueLegsAnim(LEGS_BACK);
			} else {
				PM_ContinueLegsAnim(LEGS_RUN);
			}

			footstep = qtrue;
		} else {
			bobmove = 0.3f; // walking bobs slow

			if (pm->ps->pm_flags & PMF_BACKWARDS_RUN) {
				PM_ContinueLegsAnim(LEGS_BACKWALK);
			} else {
				PM_ContinueLegsAnim(LEGS_WALK);
			}
		}
	}
	// check for footstep/splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)(old + bobmove * pml.msec) & 255;
	// if we just crossed a cycle boundary, play an appropriate footstep event
	if (((old + 64) ^ (pm->ps->bobCycle + 64)) & 128) {
		if (pm->waterlevel == 0) {
			// on ground will only play sounds if running
			if (footstep) {
				PM_AddEvent(PM_FootstepForSurface());
			}
		} else if (pm->waterlevel == 1) {
			// splashing
			PM_AddEvent(EV_FOOTSPLASH);
		} else if (pm->waterlevel == 2) {
			// wading/swimming at surface
			PM_AddEvent(EV_SWIM);
		} else if (pm->waterlevel == 3) {
			// no sound when completely underwater
		}
	}
}

/*
=======================================================================================================================================
PM_WaterEvents

Generate sound events for entering and leaving water.
=======================================================================================================================================
*/
static void PM_WaterEvents(void) { // FIXME?

	// if just entered a water volume, play a sound
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent(EV_WATER_TOUCH);
	}
	// if just completely exited a water volume, play a sound
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent(EV_WATER_LEAVE);
	}
	// check for head just going under water
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent(EV_WATER_UNDER);
	}
	// check for head just coming out of water
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent(EV_WATER_CLEAR);
	}
}

/*
=======================================================================================================================================
PM_BeginWeaponChange
=======================================================================================================================================
*/
static void PM_BeginWeaponChange(int weapon) {

	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS) {
		return;
	}

	if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon))) {
		return;
	}

	if (pm->ps->weaponstate == WEAPON_DROPPING) {
		return;
	}

	PM_AddEvent(EV_CHANGE_WEAPON);

	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += 200;

	PM_StartTorsoAnim(TORSO_DROP);
}

/*
=======================================================================================================================================
PM_FinishWeaponChange
=======================================================================================================================================
*/
static void PM_FinishWeaponChange(void) {
	int weapon;

	weapon = pm->cmd.weapon;

	if (weapon < WP_NONE || weapon >= WP_NUM_WEAPONS) {
		weapon = WP_NONE;
	}

	if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon))) {
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 250;

	PM_StartTorsoAnim(TORSO_RAISE);
}

/*
=======================================================================================================================================
PM_TorsoAnimation
=======================================================================================================================================
*/
static void PM_TorsoAnimation(void) {

	if (pm->ps->weaponstate == WEAPON_READY) {
		if (pm->ps->weapon == WP_GAUNTLET) {
			PM_ContinueTorsoAnim(TORSO_STAND2);
		} else {
			PM_ContinueTorsoAnim(TORSO_STAND);
		}

		return;
	}
}

/*
=======================================================================================================================================
PM_Weapon

Generates weapon events and modifes the weapon counter.
=======================================================================================================================================
*/
static void PM_Weapon(void) {
	int addTime;

	// don't allow attack until all buttons are up
	if (pm->ps->pm_flags & PMF_RESPAWNED) {
		return;
	}
	// ignore if spectator
	if (pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}
	// check for dead player
	if (pm->ps->stats[STAT_HEALTH] <= 0) {
		pm->ps->weapon = WP_NONE;
		return;
	}
	// check for item using
	if (pm->cmd.buttons & BUTTON_USE_HOLDABLE) {
		if (!(pm->ps->pm_flags & PMF_USE_ITEM_HELD)) {
			if (bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT && pm->ps->stats[STAT_HEALTH] >= 100) {
				// don't use medkit if at max health
			} else {
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent(EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag);
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}

			return;
		}
	} else {
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}
	// make weapon function
	if (pm->ps->weaponTime > 0) {
		pm->ps->weaponTime -= pml.msec;
	}
	// check for weapon change
	// can't change if weapon is firing, but can change again if lowering or raising
	if (pm->ps->weaponTime <= 0 || pm->ps->weaponstate != WEAPON_FIRING) {
		if (pm->ps->weapon != pm->cmd.weapon) {
			PM_BeginWeaponChange(pm->cmd.weapon);
		}
	}

	if (pm->ps->weaponTime > 0) {
		return;
	}
	// change weapon if time
	if (pm->ps->weaponstate == WEAPON_DROPPING) {
		PM_FinishWeaponChange();
		return;
	}

	if (pm->ps->weaponstate == WEAPON_RAISING) {
		pm->ps->weaponstate = WEAPON_READY;

		if (pm->ps->weapon == WP_GAUNTLET) {
			PM_StartTorsoAnim(TORSO_STAND2);
		} else {
			PM_StartTorsoAnim(TORSO_STAND);
		}

		return;
	}
	// check for fire
	if (!(pm->cmd.buttons & BUTTON_ATTACK)) {
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}
	// start the animation even if out of ammo
	if (pm->ps->weapon == WP_GAUNTLET) {
		// the gauntlet only "fires" when it actually hits something
		if (!pm->gauntletHit) {
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		}

		PM_StartTorsoAnim(TORSO_ATTACK2);
	} else {
		PM_StartTorsoAnim(TORSO_ATTACK);
	}

	pm->ps->weaponstate = WEAPON_FIRING;
	// check for out of ammo
	if (!pm->ps->ammo[pm->ps->weapon]) {
		PM_AddEvent(EV_NOAMMO);
		pm->ps->weaponTime += 500;
		return;
	}
	// take an ammo away if not infinite
	if (pm->ps->ammo[pm->ps->weapon] != -1) {
		pm->ps->ammo[pm->ps->weapon]--;
	}
	// fire weapon
	PM_AddEvent(EV_FIRE_WEAPON);

	switch (pm->ps->weapon) {
		default:
		case WP_GAUNTLET:
			addTime = 400;
			break;
		case WP_MACHINEGUN:
			addTime = 100;
			break;
		case WP_CHAINGUN:
			addTime = 30;
			break;
		case WP_SHOTGUN:
			addTime = 1000;
			break;
		case WP_NAILGUN:
			addTime = 1000;
			break;
		case WP_PROXLAUNCHER:
			addTime = 800;
			break;
		case WP_GRENADELAUNCHER:
			addTime = 800;
			break;
		case WP_NAPALMLAUNCHER:
			addTime = 1200;
			break;
		case WP_ROCKETLAUNCHER:
			addTime = 800;
			break;
		case WP_BEAMGUN:
			addTime = 50;
			break;
		case WP_RAILGUN:
			addTime = 1500;
			break;
		case WP_PLASMAGUN:
			addTime = 100;
			break;
		case WP_BFG:
			addTime = 200;
			break;
	}

	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN) {
		addTime /= 1.1;
	}

	pm->ps->weaponTime += addTime;
}

/*
=======================================================================================================================================
PM_Animate
=======================================================================================================================================
*/
static void PM_Animate(void) {

	if (pm->cmd.buttons & BUTTON_GESTURE) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_GESTURE);
			pm->ps->torsoTimer = TIMER_GESTURE;
			PM_AddEvent(EV_TAUNT);
		}
	} else if (pm->cmd.buttons & BUTTON_AFFIRMATIVE) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_AFFIRMATIVE);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	} else if (pm->cmd.buttons & BUTTON_NEGATIVE) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_NEGATIVE);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	} else if (pm->cmd.buttons & BUTTON_GETFLAG) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_GETFLAG);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	} else if (pm->cmd.buttons & BUTTON_GUARDBASE) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_GUARDBASE);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	} else if (pm->cmd.buttons & BUTTON_FOLLOWME) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_FOLLOWME);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	} else if (pm->cmd.buttons & BUTTON_PATROL) {
		if (pm->ps->torsoTimer == 0) {
			PM_StartTorsoAnim(TORSO_PATROL);
			pm->ps->torsoTimer = 600; // TIMER_GESTURE;
		}
	}
}

/*
=======================================================================================================================================
PM_DropTimers
=======================================================================================================================================
*/
static void PM_DropTimers(void) {

	// drop misc timing counter
	if (pm->ps->pm_time) {
		if (pml.msec >= pm->ps->pm_time) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}
	// drop animation counter
	if (pm->ps->legsTimer > 0) {
		pm->ps->legsTimer -= pml.msec;

		if (pm->ps->legsTimer < 0) {
			pm->ps->legsTimer = 0;
		}
	}

	if (pm->ps->torsoTimer > 0) {
		pm->ps->torsoTimer -= pml.msec;

		if (pm->ps->torsoTimer < 0) {
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
=======================================================================================================================================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles are being updated instead of a full move.
=======================================================================================================================================
*/
void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd) {
	short temp;
	int i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION) {
		return; // no view changes at all
	}

	if (ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0) {
		return; // no view changes at all
	}
	// circularly clamp the angles with deltas
	for (i = 0; i < 3; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];

		if (i == PITCH) {
			// don't let the player look up or down more than 90 degrees
			if (temp > 16000) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if (temp < -16000) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}

		ps->viewangles[i] = SHORT2ANGLE(temp);
	}
}

/*
=======================================================================================================================================
PmoveSingle
=======================================================================================================================================
*/
void PmoveSingle(pmove_t *pmove) {

	pm = pmove;
	// this counter lets us debug movement problems with a journal by setting a conditional breakpoint for the previous frame
	c_pmove++;
	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if (pm->ps->stats[STAT_HEALTH] <= 0) {
		pm->tracemask &= ~CONTENTS_BODY; // corpses can fly through bodies
	}
	// make sure walking button is clear if they are running, to avoid proxy no-footsteps cheats
	if (abs(pm->cmd.forwardmove) > 64 || abs(pm->cmd.rightmove) > 64) {
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}
	// set the talk balloon flag
	if (pm->cmd.buttons & BUTTON_TALK) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}
	// set the firing flag for continuous beam weapons
	if (!(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION && pm->ps->pm_type != PM_NOCLIP && (pm->cmd.buttons & BUTTON_ATTACK) && pm->ps->ammo[pm->ps->weapon]) {
		pm->ps->eFlags |= EF_FIRING;
	} else {
		pm->ps->eFlags &= ~EF_FIRING;
	}
	// clear the respawned flag if attack and use are cleared
	if (pm->ps->stats[STAT_HEALTH] > 0 && !(pm->cmd.buttons & (BUTTON_ATTACK|BUTTON_USE_HOLDABLE))) {
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}
	// if talk button is down, disallow all other input
	// this is to prevent any possible intercept proxy from adding fake talk balloons
	if (pmove->cmd.buttons & BUTTON_TALK) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}
	// clear all pmove local vars
	memset(&pml, 0, sizeof(pml));
	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;

	if (pml.msec < 1) {
		pml.msec = 1;
	} else if (pml.msec > 200) {
		pml.msec = 200;
	}

	pm->ps->commandTime = pmove->cmd.serverTime;
	// save old org in case we get stuck
	VectorCopy(pm->ps->origin, pml.previous_origin);
	// save old velocity for crashlanding
	VectorCopy(pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;
	// update the viewangles
	PM_UpdateViewAngles(pm->ps, &pm->cmd);
	AngleVectors(pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if (pm->cmd.upmove < 10) {
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}
	// decide if backpedaling animations should be used
	if (pm->cmd.forwardmove < 0) {
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	} else if (pm->cmd.forwardmove > 0 || (pm->cmd.forwardmove == 0 && pm->cmd.rightmove)) {
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if (pm->ps->pm_type >= PM_DEAD) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->ps->pm_type == PM_SPECTATOR) {
		PM_CheckDuck();
		PM_FlyMove();
		PM_DropTimers();
		return;
	}

	if (pm->ps->pm_type == PM_NOCLIP) {
		PM_NoclipMove();
		PM_DropTimers();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE) {
		return; // no movement at all
	}

	if (pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION) {
		return; // no movement at all
	}
	// set watertype, and waterlevel
	PM_SetWaterLevel();

	pml.previous_waterlevel = pmove->waterlevel;
	// set mins, maxs, and viewheight
	PM_CheckDuck();
	// set groundentity
	PM_GroundTrace();

	if (pm->ps->pm_type == PM_DEAD) {
		PM_DeadMove();
	}

	PM_DropTimers();

	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP) {
		PM_WaterJumpMove();
	} else if (pm->waterlevel > 1) {
		// swimming
		PM_WaterMove();
	} else if (pml.walking) {
		// walking on ground
		PM_WalkMove();
	} else {
		// airborne
		PM_AirMove();
	}

	PM_Animate();
	// set groundentity, watertype, and waterlevel
	PM_GroundTrace();
	PM_SetWaterLevel();
	// weapons
	PM_Weapon();
	// torso animation
	PM_TorsoAnimation();
	// footstep events/legs animations
	PM_Footsteps();
	// entering/leaving water splashes
	PM_WaterEvents();
	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector(pm->ps->velocity);
}

/*
=======================================================================================================================================
Pmove

Can be called by either the server or the client.
=======================================================================================================================================
*/
void Pmove(pmove_t *pmove) {
	int finalTime;

	finalTime = pmove->cmd.serverTime;

	if (finalTime < pmove->ps->commandTime) {
		return; // should not happen
	}

	if (finalTime > pmove->ps->commandTime + 1000) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount + 1) & ((1 << PS_PMOVEFRAMECOUNTBITS) - 1);
	// chop the move up if it is too long, to prevent framerate dependent behavior
	while (pmove->ps->commandTime != finalTime) {
		int msec;

		msec = finalTime - pmove->ps->commandTime;

		if (pmove->pmove_fixed) {
			if (msec > pmove->pmove_msec) {
				msec = pmove->pmove_msec;
			}
		} else {
			if (msec > 66) {
				msec = 66;
			}
		}

		pmove->cmd.serverTime = pmove->ps->commandTime + msec;

		PmoveSingle(pmove);

		if (pmove->ps->pm_flags & PMF_JUMP_HELD) {
			pmove->cmd.upmove = 20;
		}
	}

	//PM_CheckStuck();
}
