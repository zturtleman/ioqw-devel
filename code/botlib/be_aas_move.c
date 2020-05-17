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
 AAS movement.
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "../qcommon/surfaceflags.h" // for CONTENTS_WATER, CONTENTS_LAVA, CONTENTS_SLIME etc.
#include "l_memory.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_libvar.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_aas_def.h"

extern botlib_import_t botimport;

aas_settings_t aassettings;

/*
=======================================================================================================================================
AAS_DropToFloor
=======================================================================================================================================
*/
int AAS_DropToFloor(vec3_t origin, vec3_t mins, vec3_t maxs) {
	vec3_t end;
	bsp_trace_t trace;

	// mappers like to put them exactly on the floor, but being coplanar will sometimes show up as starting in solid, so lif it up one pixel
	origin[2] += 1;

	VectorCopy(origin, end);

	end[2] -= 4096;

	trace = AAS_Trace(origin, mins, maxs, end, 0, CONTENTS_SOLID);

	if (trace.startsolid) {
		origin[2] -= 1;
		return qfalse;
	}

	VectorCopy(trace.endpos, origin);
	return qtrue;
}

/*
=======================================================================================================================================
AAS_InitSettings
=======================================================================================================================================
*/
void AAS_InitSettings(void) {

	aassettings.phys_gravitydirection[0] = 0;
	aassettings.phys_gravitydirection[1] = 0;
	aassettings.phys_gravitydirection[2] = -1;
	aassettings.phys_friction = LibVarValue("phys_friction", "6");
	aassettings.phys_stopspeed = LibVarValue("phys_stopspeed", "100");
	aassettings.phys_gravity = LibVarValue("phys_gravity", "800");
	aassettings.phys_waterfriction = LibVarValue("phys_waterfriction", "1");
	aassettings.phys_watergravity = LibVarValue("phys_watergravity", "400");
	aassettings.phys_maxvelocity = LibVarValue("phys_maxvelocity", "260");
	aassettings.phys_maxscoutvelocity = LibVarValue("phys_maxscoutvelocity", "390");
	aassettings.phys_maxwalkvelocity = LibVarValue("phys_maxwalkvelocity", "280");
	aassettings.phys_maxcrouchvelocity = LibVarValue("phys_maxcrouchvelocity", "100");
	aassettings.phys_maxswimvelocity = LibVarValue("phys_maxswimvelocity", "45");
	aassettings.phys_walkaccelerate = LibVarValue("phys_walkaccelerate", "10");
	aassettings.phys_airaccelerate = LibVarValue("phys_airaccelerate", "1");
	aassettings.phys_swimaccelerate = LibVarValue("phys_swimaccelerate", "4");
	aassettings.phys_maxstep = LibVarValue("phys_maxstep", "19");
	aassettings.phys_maxsteepness = LibVarValue("phys_maxsteepness", "0.7");
	aassettings.phys_maxwaterjump = LibVarValue("phys_maxwaterjump", "20");
	aassettings.phys_maxbarrier = LibVarValue("phys_maxbarrier", "43");
	aassettings.phys_maxscoutbarrier = LibVarValue("phys_maxscoutbarrier", "73");
	aassettings.phys_jumpvel = LibVarValue("phys_jumpvel", "200");
	aassettings.phys_jumpvelscout = LibVarValue("phys_jumpvelscout", "300");
	aassettings.phys_falldelta5 = LibVarValue("phys_falldelta5", "40");
	aassettings.phys_falldelta10 = LibVarValue("phys_falldelta10", "60");
	aassettings.rs_waterjump = LibVarValue("rs_waterjump", "400");
	aassettings.rs_teleport = LibVarValue("rs_teleport", "50");
	aassettings.rs_barrierjump = LibVarValue("rs_barrierjump", "100");
	aassettings.rs_startcrouch = LibVarValue("rs_startcrouch", "300");
	aassettings.rs_startwalkoffledge = LibVarValue("rs_startwalkoffledge", "70");
	aassettings.rs_startjump = LibVarValue("rs_startjump", "300");
	aassettings.rs_rocketjump = LibVarValue("rs_rocketjump", "500");
	aassettings.rs_jumppad = LibVarValue("rs_jumppad", "250");
	aassettings.rs_aircontrolledjumppad = LibVarValue("rs_aircontrolledjumppad", "300");
	aassettings.rs_funcbob = LibVarValue("rs_funcbob", "300");
	aassettings.rs_startelevator = LibVarValue("rs_startelevator", "50");
	aassettings.rs_falldamage5 = LibVarValue("rs_falldamage5", "300");
	aassettings.rs_falldamage10 = LibVarValue("rs_falldamage10", "500");
	aassettings.rs_maxfallheight = LibVarValue("rs_maxfallheight", "512");
	aassettings.rs_maxjumpfallheight = LibVarValue("rs_maxjumpfallheight", "450");
}

/*
=======================================================================================================================================
AAS_AgainstLadder

Returns qtrue if the bot is against a ladder.
=======================================================================================================================================
*/
int AAS_AgainstLadder(vec3_t origin) {
	int areanum, i, facenum, side;
	vec3_t org;
	aas_plane_t *plane;
	aas_face_t *face;
	aas_area_t *area;

	VectorCopy(origin, org);

	areanum = AAS_PointAreaNum(org);

	if (!areanum) {
		org[0] += 1;
		areanum = AAS_PointAreaNum(org);

		if (!areanum) {
			org[1] += 1;
			areanum = AAS_PointAreaNum(org);

			if (!areanum) {
				org[0] -= 2;
				areanum = AAS_PointAreaNum(org);

				if (!areanum) {
					org[1] -= 2;
					areanum = AAS_PointAreaNum(org);
				}
			}
		}
	}
	// if in solid... wrrr shouldn't happen
	if (!areanum) {
		return qfalse;
	}
	// if not in a ladder area
	if (!(aasworld.areasettings[areanum].areaflags & AREA_LADDER)) {
		return qfalse;
	}
	// if a crouch only area
	if (!(aasworld.areasettings[areanum].presencetype & PRESENCE_NORMAL)) {
		return qfalse;
	}

	area = &aasworld.areas[areanum];

	for (i = 0; i < area->numfaces; i++) {
		facenum = aasworld.faceindex[area->firstface + i];
		side = facenum < 0;
		face = &aasworld.faces[abs(facenum)];
		// if the face isn't a ladder face
		if (!(face->faceflags & FACE_LADDER)) {
			continue;
		}
		// get the plane the face is in
		plane = &aasworld.planes[face->planenum ^ side];
		// if the origin is pretty close to the plane
		if (fabsf(DotProduct(plane->normal, origin) - plane->dist) < 3) {
			if (AAS_PointInsideFace(abs(facenum), origin, 0.1f)) {
				return qtrue;
			}
		}
	}

	return qfalse;
}

/*
=======================================================================================================================================
AAS_OnGround

Returns qtrue if the bot is on the ground.
=======================================================================================================================================
*/
int AAS_OnGround(vec3_t origin, int presencetype, int passent) {
	aas_trace_t trace;
	vec3_t end, up = {0, 0, 1};
	aas_plane_t *plane;
	float phys_maxsteepness;

	VectorCopy(origin, end);

	end[2] -= 10;

	trace = AAS_TraceClientBBox(origin, end, presencetype, passent);
	// if in solid
	if (trace.startsolid) {
		return qfalse;
	}
	// if nothing hit at all
	if (trace.fraction >= 1.0) {
		return qfalse;
	}
	// if too far from the hit plane
	if (origin[2] - trace.endpos[2] > 10) {
		return qfalse;
	}
	// check if the plane isn't too steep
	plane = &trace.plane; //AAS_PlaneFromNum(trace.planenum);
	phys_maxsteepness = aassettings.phys_maxsteepness;

	if (DotProduct(plane->normal, up) < phys_maxsteepness) {
		return qfalse;
	}
	// the bot is on the ground
	return qtrue;
}

/*
=======================================================================================================================================
AAS_Swimming

Returns qtrue if a bot at the given position is swimming.
=======================================================================================================================================
*/
int AAS_Swimming(vec3_t origin) {
	vec3_t testorg;

	VectorCopy(origin, testorg);

	testorg[2] -= 2;

	if (AAS_PointContents(testorg) & (CONTENTS_LAVA|CONTENTS_SLIME|CONTENTS_WATER)) {
		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
AAS_JumpReachRunStart
=======================================================================================================================================
*/
void AAS_JumpReachRunStart(aas_reachability_t *reach, vec3_t runstart) {
	vec3_t hordir, start, cmdmove;
	aas_clientmove_t move;

	hordir[0] = reach->start[0] - reach->end[0];
	hordir[1] = reach->start[1] - reach->end[1];
	hordir[2] = 0;

	VectorNormalize(hordir);
	// start point
	VectorCopy(reach->start, start);

	start[2] += 1;
	// get command movement
	VectorScale(hordir, 400, cmdmove);
	// movement prediction
	AAS_PredictClientMovement(&move, -1, start, PRESENCE_NORMAL, qtrue, qfalse, vec3_origin, cmdmove, 1, 2, 0.1f, SE_ENTERWATER|SE_ENTERSLIME|SE_ENTERLAVA|SE_HITGROUNDDAMAGE|SE_GAP, 0, qfalse);
	VectorCopy(move.endpos, runstart);
	// don't enter slime or lava and don't fall from too high
	if (move.stopevent & (SE_ENTERSLIME|SE_ENTERLAVA|SE_HITGROUNDDAMAGE)) {
		VectorCopy(start, runstart);
	}
}

/*
=======================================================================================================================================
AAS_ScoutJumpReachRunStart
=======================================================================================================================================
*/
void AAS_ScoutJumpReachRunStart(aas_reachability_t *reach, vec3_t runstart) {
	vec3_t hordir, start, cmdmove;
	aas_clientmove_t move;

	hordir[0] = reach->start[0] - reach->end[0];
	hordir[1] = reach->start[1] - reach->end[1];
	hordir[2] = 0;

	VectorNormalize(hordir);
	// start point
	VectorCopy(reach->start, start);

	start[2] += 1;
	// get command movement
	VectorScale(hordir, 400, cmdmove);
	// movement prediction
	AAS_PredictClientMovement(&move, -1, start, PRESENCE_NORMAL, qtrue, qtrue, vec3_origin, cmdmove, 1, 2, 0.1f, SE_ENTERWATER|SE_ENTERSLIME|SE_ENTERLAVA|SE_HITGROUNDDAMAGE|SE_GAP, 0, qfalse);
	VectorCopy(move.endpos, runstart);
	// don't enter slime or lava and don't fall from too high
	if (move.stopevent & (SE_ENTERSLIME|SE_ENTERLAVA|SE_HITGROUNDDAMAGE)) {
		VectorCopy(start, runstart);
	}
}

/*
=======================================================================================================================================
AAS_WeaponJumpZVelocity

Returns the Z velocity when rocket jumping at the origin.
=======================================================================================================================================
*/
float AAS_WeaponJumpZVelocity(vec3_t origin, float radiusdamage) {
	vec3_t kvel, v, start, end, forward, right, viewangles, dir;
	float mass, knockback, points, phys_jumpvel;
	vec3_t rocketoffset = {8, 8, -8};
	vec3_t botmins = {-16, -16, -24};
	vec3_t botmaxs = {16, 16, 56};
	bsp_trace_t bsptrace;

	// look down (90 degrees)
	viewangles[PITCH] = 90;
	viewangles[YAW] = 0;
	viewangles[ROLL] = 0;
	// get the start point shooting from
	VectorCopy(origin, start);

	start[2] += 8; // view offset Z

	AngleVectorsForwardRight(viewangles, forward, right);

	start[0] += forward[0] * rocketoffset[0] + right[0] * rocketoffset[1];
	start[1] += forward[1] * rocketoffset[0] + right[1] * rocketoffset[1];
	start[2] += forward[2] * rocketoffset[0] + right[2] * rocketoffset[1] + rocketoffset[2];
	// end point of the trace
	VectorMA(start, 500, forward, end);
	// trace a line to get the impact point
	bsptrace = AAS_Trace(start, NULL, NULL, end, 1, CONTENTS_SOLID);
	// calculate the damage the bot will get from the rocket impact
	VectorAdd(botmins, botmaxs, v);
	VectorMA(origin, 0.5, v, v);
	VectorSubtract(bsptrace.endpos, v, v);

	points = radiusdamage - 0.5 * VectorLength(v);

	if (points < 0) {
		points = 0;
	}
	// mass of the bot (g_client.c: PutClientInServer)
	mass = 200;
	// knockback is the same as the damage points
	knockback = points;
	// direction of the damage (from trace.endpos to bot origin)
	VectorSubtract(origin, bsptrace.endpos, dir);
	VectorNormalize(dir);
	// damage velocity
	VectorScale(dir, 1600.0 * (float)knockback / mass, kvel); // the rocket jump hack...
	// rocket impact velocity + jump velocity
	phys_jumpvel = aassettings.phys_jumpvel;
	return kvel[2] + phys_jumpvel;
}

/*
=======================================================================================================================================
AAS_RocketJumpZVelocity
=======================================================================================================================================
*/
float AAS_RocketJumpZVelocity(vec3_t origin) {
	// rocket radius damage is 120 (g_weapon.c: Weapon_RocketLauncher_Fire)
	return AAS_WeaponJumpZVelocity(origin, 120);
}

/*
=======================================================================================================================================
AAS_Accelerate
=======================================================================================================================================
*/
void AAS_Accelerate(vec3_t velocity, float frametime, vec3_t wishdir, float wishspeed, float accel) {
	// q2 style
	int i;
	float addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct(velocity, wishdir);
	addspeed = wishspeed - currentspeed;

	if (addspeed <= 0) {
		return;
	}

	accelspeed = accel * frametime * wishspeed;

	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}

	for (i = 0; i < 3; i++) {
		velocity[i] += accelspeed * wishdir[i];
	}
}

/*
=======================================================================================================================================
AAS_ApplyFriction

Applies ground friction to the given velocity.
=======================================================================================================================================
*/
void AAS_ApplyFriction(vec3_t vel, float friction, float stopspeed, float frametime) {
	float speed, control, newspeed;

	// horizontal speed
	speed = sqrt(vel[0] * vel[0] + vel[1] * vel[1]);

	if (speed) {
		control = speed < stopspeed ? stopspeed : speed;
		newspeed = speed - frametime * control * friction;

		if (newspeed < 0) {
			newspeed = 0;
		}

		newspeed /= speed;
		vel[0] *= newspeed;
		vel[1] *= newspeed;
	}
}

/*
=======================================================================================================================================
AAS_ClipToBBox
=======================================================================================================================================
*/
static qboolean AAS_ClipToBBox(aas_trace_t *trace, const vec3_t start, const vec3_t end, int presencetype, const vec3_t mins, const vec3_t maxs) {
	int i, j, side;
	float front, back, frac, planedist;
	vec3_t bboxmins, bboxmaxs, absmins, absmaxs, dir, mid;

	AAS_PresenceTypeBoundingBox(presencetype, bboxmins, bboxmaxs);
	VectorSubtract(mins, bboxmaxs, absmins);
	VectorSubtract(maxs, bboxmins, absmaxs);
	VectorCopy(end, trace->endpos);

	trace->fraction = 1;

	for (i = 0; i < 3; i++) {
		if (start[i] < absmins[i] && end[i] < absmins[i]) {
			return qfalse;
		}

		if (start[i] > absmaxs[i] && end[i] > absmaxs[i]) {
			return qfalse;
		}
	}
	// check bounding box collision
	VectorSubtract(end, start, dir);

	frac = 1;

	for (i = 0; i < 3; i++) {
		if (fabsf(dir[i]) < 0.001f) { // this may cause denormalization or division by zero
			//botimport.Print(PRT_MESSAGE, S_COLOR_BLUE "AAS_ClipToBBox: division by zero fix!\n");
			continue;
		}
		// get plane to test collision with for the current axis direction
		if (dir[i] > 0) {
			planedist = absmins[i];
		} else {
			planedist = absmaxs[i];
		}
		// calculate collision fraction
		front = start[i] - planedist;
		back = end[i] - planedist;
		frac = front / (front - back);
		// check if between bounding planes of next axis
		side = i + 1;

		if (side > 2) {
			side = 0;
		}

		mid[side] = start[side] + dir[side] * frac;

		if (mid[side] > absmins[side] && mid[side] < absmaxs[side]) {
			// check if between bounding planes of next axis
			side++;

			if (side > 2) {
				side = 0;
			}

			mid[side] = start[side] + dir[side] * frac;

			if (mid[side] > absmins[side] && mid[side] < absmaxs[side]) {
				mid[i] = planedist;
				break;
			}
		}
	}
	// if there was a collision
	if (i != 3) {
		trace->startsolid = qfalse;
		trace->fraction = frac;
		trace->ent = 0;
		trace->area = 0;
		trace->lastarea = 0;
		trace->planenum = 0;
		// ZTM: TODO: use the plane of collision
		trace->plane = aasworld.planes[trace->planenum];
		// trace endpos
		for (j = 0; j < 3; j++) {
			trace->endpos[j] = start[j] + dir[j] * frac;
		}

		return qtrue;
	}

	return qfalse;
}

/*
=======================================================================================================================================
AAS_ClientMovementPrediction

Predicts the movement. Assumes regular bounding box sizes.
NOTE: out of water jumping is not included.

Parameter:	origin			: origin to start with.
			presencetype	: presence type to start with.
			velocity		: velocity to start with.
			cmdmove			: client command movement.
			cmdframes		: number of frame cmdmove is valid.
			maxframes		: maximum number of predicted frames.
			frametime		: duration of one predicted frame.
			stopevent		: events that stop the prediction.
			stopareanum		: stop as soon as entered this area.
Returns: aas_clientmove_t
=======================================================================================================================================
*/
static int AAS_ClientMovementPrediction(aas_clientmove_t *move, int entnum, const vec3_t origin, int presencetype, int onground, int scoutmove, const vec3_t velocity, const vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, const vec3_t mins, const vec3_t maxs, int visualize) {
	float phys_friction, phys_stopspeed, phys_gravity, phys_waterfriction;
	float phys_watergravity;
	float phys_walkaccelerate, phys_airaccelerate, phys_swimaccelerate;
	float phys_maxwalkvelocity, phys_maxcrouchvelocity, phys_maxswimvelocity;
	float phys_maxstep, phys_maxsteepness, phys_maxbarrier, phys_maxscoutbarrier, phys_jumpvel, phys_jumpvelscout, friction;
	float gravity, delta, maxvel, wishspeed, accelerate;
	//float velchange, newvel;
	//int ax;
	int n, i, j, pc, step, swimming, crouch, event, jump_frame, areanum;
	int areas[20], numareas;
	vec3_t points[20];
	vec3_t org, end, feet, start, stepend, lastorg, wishdir;
	vec3_t frame_test_vel, old_frame_test_vel, left_test_vel, savevel;
	vec3_t up = {0, 0, 1};
	aas_plane_t *plane, *plane2, *lplane;
	aas_trace_t trace, steptrace;

	if (frametime <= 0) {
		frametime = 0.1f;
	}

	phys_friction = aassettings.phys_friction;
	phys_stopspeed = aassettings.phys_stopspeed;
	phys_gravity = aassettings.phys_gravity;
	phys_waterfriction = aassettings.phys_waterfriction;
	phys_watergravity = aassettings.phys_watergravity;
	phys_maxwalkvelocity = aassettings.phys_maxwalkvelocity; //* frametime;
	phys_maxcrouchvelocity = aassettings.phys_maxcrouchvelocity; //* frametime;
	phys_maxswimvelocity = aassettings.phys_maxswimvelocity; //* frametime;
	phys_walkaccelerate = aassettings.phys_walkaccelerate;
	phys_airaccelerate = aassettings.phys_airaccelerate;
	phys_swimaccelerate = aassettings.phys_swimaccelerate;
	phys_maxstep = aassettings.phys_maxstep;
	phys_maxsteepness = aassettings.phys_maxsteepness;
	phys_maxbarrier = aassettings.phys_maxbarrier;
	phys_maxscoutbarrier = aassettings.phys_maxscoutbarrier;
	phys_jumpvel = aassettings.phys_jumpvel * frametime;
	phys_jumpvelscout = aassettings.phys_jumpvelscout * frametime;

	Com_Memset(move, 0, sizeof(*move));
	Com_Memset(&trace, 0, sizeof(trace));
	// start at the current origin
	VectorCopy(origin, org);

	org[2] += 0.25;
	// velocity to test for the first frame
	VectorScale(velocity, frametime, frame_test_vel);

	jump_frame = -1;
	lplane = NULL;
	// predict a maximum of 'maxframes' ahead
	for (n = 0; n < maxframes; n++) {
		swimming = AAS_Swimming(org);
		// get gravity depending on swimming or not
		gravity = swimming ? phys_watergravity : phys_gravity;
		// apply gravity at the START of the frame
		frame_test_vel[2] = frame_test_vel[2] - (gravity * 0.1 * frametime);
		// if on the ground or swimming
		if (onground || swimming) {
			friction = swimming ? phys_waterfriction : phys_friction;
			// apply friction
			VectorScale(frame_test_vel, 1 / frametime, frame_test_vel);
			AAS_ApplyFriction(frame_test_vel, friction, phys_stopspeed, frametime);
			VectorScale(frame_test_vel, frametime, frame_test_vel);
		}

		crouch = qfalse;
		// apply command movement
		if (cmdframes < 0) {
			// cmdmove is the destination, we should keep moving towards it
			VectorSubtract(cmdmove, org, wishdir);
			VectorNormalize(wishdir);
			VectorScale(wishdir, phys_maxwalkvelocity, wishdir);
			VectorCopy(frame_test_vel, savevel);
			VectorScale(wishdir, frametime, frame_test_vel);

			if (!swimming) {
				frame_test_vel[2] = savevel[2];
			}
		} else if (n < cmdframes) {
			// ax = 0;
			maxvel = phys_maxwalkvelocity;
			accelerate = phys_airaccelerate;

			VectorCopy(cmdmove, wishdir);

			if (onground) {
				if (cmdmove[2] < -300) {
					crouch = qtrue;
					maxvel = phys_maxcrouchvelocity;
				}
				// if not swimming and upmove is positive then jump
				if (!swimming && cmdmove[2] > 1) {
					// jump velocity minus the gravity for one frame + 5 for safety
					if (!scoutmove) {
						frame_test_vel[2] = phys_jumpvel - (gravity * 0.1 * frametime) + 5;
					} else {
						frame_test_vel[2] = phys_jumpvelscout - (gravity * 0.1 * frametime) + 5;
					}

					jump_frame = n;
					// jumping so air accelerate
					accelerate = phys_airaccelerate;
				} else {
					accelerate = phys_walkaccelerate;
				}
				// ax = 2;
			}

			if (swimming) {
				maxvel = phys_maxswimvelocity;
				accelerate = phys_swimaccelerate;
				// ax = 3;
			} else {
				wishdir[2] = 0;
			}

			wishspeed = VectorNormalize(wishdir);

			if (wishspeed > maxvel) {
				wishspeed = maxvel;
			}

			VectorScale(frame_test_vel, 1 / frametime, frame_test_vel);
			AAS_Accelerate(frame_test_vel, frametime, wishdir, wishspeed, accelerate);
			VectorScale(frame_test_vel, frametime, frame_test_vel);
			/*
			for (i = 0; i < ax; i++) {
				velchange = (cmdmove[i] * frametime) - frame_test_vel[i];

				if (velchange > phys_maxacceleration) {
					velchange = phys_maxacceleration;
				} else if (velchange < -phys_maxacceleration) {
					velchange = -phys_maxacceleration;
				}

				newvel = frame_test_vel[i] + velchange;

				if (frame_test_vel[i] <= maxvel && newvel > maxvel) {
					frame_test_vel[i] = maxvel;
				} else if (frame_test_vel[i] >= -maxvel && newvel < -maxvel) {
					frame_test_vel[i] = -maxvel;
				} else {
					frame_test_vel[i] = newvel;
				}
			}
			*/
		}

		if (crouch) {
			presencetype = PRESENCE_CROUCH;
		} else if (presencetype == PRESENCE_CROUCH) {
			if (AAS_PointPresenceType(org) & PRESENCE_NORMAL) {
				presencetype = PRESENCE_NORMAL;
			}
		}
		// save the current origin
		VectorCopy(org, lastorg);
		// move linear during one frame
		VectorCopy(frame_test_vel, left_test_vel);

		j = 0;

		do {
			VectorAdd(org, left_test_vel, end);
			// trace a bounding box
			trace = AAS_TraceClientBBox(org, end, presencetype, entnum);
//#ifdef AAS_MOVE_DEBUG
			if (visualize) {
				if (trace.startsolid) {
					botimport.Print(PRT_MESSAGE, "PredictMovement: start solid\n");
				}

				AAS_DebugLine(org, trace.endpos, LINECOLOR_RED);
			}
//#endif // AAS_MOVE_DEBUG
			if (stopevent & (SE_ENTERAREA|SE_TOUCHJUMPPAD|SE_TOUCHTELEPORTER|SE_TOUCHCLUSTERPORTAL)) {
				numareas = AAS_TraceAreas(org, trace.endpos, areas, points, 20);

				for (i = 0; i < numareas; i++) {
					if (stopevent & SE_ENTERAREA) {
						if (areas[i] == stopareanum) {
							VectorCopy(points[i], move->endpos);
							VectorScale(frame_test_vel, 1 / frametime, move->velocity);

							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = SE_ENTERAREA;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = n * frametime;
							move->frames = n;
							return qtrue;
						}
					}
					// NOTE: if not the first frame
					if ((stopevent & SE_TOUCHJUMPPAD) && n) {
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_JUMPPAD) {
							VectorCopy(points[i], move->endpos);
							VectorScale(frame_test_vel, 1 / frametime, move->velocity);

							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = SE_TOUCHJUMPPAD;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = n * frametime;
							move->frames = n;
							return qtrue;
						}
					}

					if (stopevent & SE_TOUCHTELEPORTER) {
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_TELEPORTER) {
							VectorCopy(points[i], move->endpos);
							VectorScale(frame_test_vel, 1 / frametime, move->velocity);

							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = SE_TOUCHTELEPORTER;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = n * frametime;
							move->frames = n;
							return qtrue;
						}
					}

					if (stopevent & SE_TOUCHCLUSTERPORTAL) {
						if (aasworld.areasettings[areas[i]].contents & AREACONTENTS_CLUSTERPORTAL) {
							VectorCopy(points[i], move->endpos);
							VectorScale(frame_test_vel, 1 / frametime, move->velocity);

							move->endarea = areas[i];
							move->trace = trace;
							move->stopevent = SE_TOUCHCLUSTERPORTAL;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = n * frametime;
							move->frames = n;
							return qtrue;
						}
					}
				}
			}

			if (stopevent & SE_HITBOUNDINGBOX) {
				if (AAS_ClipToBBox(&trace, org, trace.endpos, presencetype, mins, maxs)) {
					VectorCopy(trace.endpos, move->endpos);
					VectorScale(frame_test_vel, 1 / frametime, move->velocity);

					move->endarea = AAS_PointAreaNum(move->endpos);
					move->trace = trace;
					move->stopevent = SE_HITBOUNDINGBOX;
					move->presencetype = presencetype;
					move->endcontents = 0;
					move->time = n * frametime;
					move->frames = n;
					return qtrue;
				}
			}
			// move the entity to the trace end point
			VectorCopy(trace.endpos, org);
			// if there was a collision
			if (trace.fraction < 1.0) {
				// get the plane the bounding box collided with
				plane = &trace.plane; //AAS_PlaneFromNum(trace.planenum);

				if (stopevent & SE_HITGROUNDAREA) {
					if (DotProduct(plane->normal, up) > phys_maxsteepness) {
						VectorCopy(org, start);

						start[2] += 0.5;

						if (AAS_PointAreaNum(start) == stopareanum) {
							VectorCopy(start, move->endpos);
							VectorScale(frame_test_vel, 1 / frametime, move->velocity);

							move->endarea = stopareanum;
							move->trace = trace;
							move->stopevent = SE_HITGROUNDAREA;
							move->presencetype = presencetype;
							move->endcontents = 0;
							move->time = n * frametime;
							move->frames = n;
							return qtrue;
						}
					}
				}
				// assume there's no step
				step = qfalse;
				// if it is a vertical plane and the bot didn't jump recently
				if (plane->normal[2] == 0 && (jump_frame < 0 || n - jump_frame > 2)) {
					// check for a step
					VectorMA(org, -0.25, plane->normal, start);
					VectorCopy(start, stepend);

					start[2] += phys_maxstep;
					steptrace = AAS_TraceClientBBox(start, stepend, presencetype, entnum);

					if (!steptrace.startsolid) {
						plane2 = &steptrace.plane; //AAS_PlaneFromNum(steptrace.planenum);

						if (DotProduct(plane2->normal, up) > phys_maxsteepness) {
							VectorSubtract(end, steptrace.endpos, left_test_vel);

							left_test_vel[2] = 0;
							frame_test_vel[2] = 0;
//#ifdef AAS_MOVE_DEBUG
							if (visualize) {
								if (steptrace.endpos[2] - org[2] > 0.125) {
									VectorCopy(org, start);

									start[2] = steptrace.endpos[2];

									AAS_DebugLine(org, start, LINECOLOR_BLUE);
								}
							}
//#endif // AAS_MOVE_DEBUG
							org[2] = steptrace.endpos[2];
							step = qtrue;
						}
					}
				}

				if (!step) {
					// velocity left to test for this frame is the projection of the current test velocity into the hit plane
					VectorMA(left_test_vel, -DotProduct(left_test_vel, plane->normal), plane->normal, left_test_vel);
					// if this is the same plane we hit before, nudge velocity out along it, which fixes some epsilon issues with non-axial planes
					if (lplane && DotProduct(lplane->normal, plane->normal) > 0.99) {
						VectorAdd(plane->normal, left_test_vel, left_test_vel);
					}

					lplane = plane;
					// store the old velocity for landing check
					VectorCopy(frame_test_vel, old_frame_test_vel);
					// test velocity for the next frame is the projection of the velocity of the current frame into the hit plane
					VectorMA(frame_test_vel, -DotProduct(frame_test_vel, plane->normal), plane->normal, frame_test_vel);
					// check for a landing on an almost horizontal floor
					if (DotProduct(plane->normal, up) > phys_maxsteepness) {
						onground = qtrue;
					}

					if (stopevent & SE_HITGROUNDDAMAGE) {
						delta = 0;

						if (old_frame_test_vel[2] < 0 && frame_test_vel[2] > old_frame_test_vel[2] && !onground) {
							delta = old_frame_test_vel[2];
						} else if (onground) {
							delta = frame_test_vel[2] - old_frame_test_vel[2];
						}

						if (delta) {
							delta = delta * 10;
							delta = delta * delta * 0.0001;

							if (swimming) {
								delta = 0;
							}
							// never take falling damage if completely underwater
							/*
							if (ent->waterlevel == 3) {
								return;
							}

							if (ent->waterlevel == 2) {
								delta *= 0.25;
							}

							if (ent->waterlevel == 1) {
								delta *= 0.5;
							}
							*/
							if (delta > 40) {
								VectorCopy(org, move->endpos);
								VectorCopy(frame_test_vel, move->velocity);

								move->endarea = AAS_PointAreaNum(org);
								move->trace = trace;
								move->stopevent = SE_HITGROUNDDAMAGE;
								move->presencetype = presencetype;
								move->endcontents = 0;
								move->time = n * frametime;
								move->frames = n;
								return qtrue;
							}
						}
					}
				}
			}
			// extra check to prevent endless loop
			if (++j > 20) {
				return qfalse;
			}
		// while there is a plane hit
		} while (trace.fraction < 1.0);
		// if going down
		if (frame_test_vel[2] <= 10) {
			// check for a liquid at the feet of the bot
			VectorCopy(org, feet);

			feet[2] -= 22;
			pc = AAS_PointContents(feet);
			// get event from pc
			event = SE_NONE;

			if (pc & CONTENTS_WATER) {
				event |= SE_ENTERWATER;
			}

			if (pc & CONTENTS_LAVA) {
				event |= SE_ENTERLAVA;
			}

			if (pc & CONTENTS_SLIME) {
				event |= SE_ENTERSLIME;
			}

			areanum = AAS_PointAreaNum(org);

			if (aasworld.areasettings[areanum].contents & AREACONTENTS_WATER) {
				event |= SE_ENTERWATER;
			}

			if (aasworld.areasettings[areanum].contents & AREACONTENTS_LAVA) {
				event |= SE_ENTERLAVA;
			}

			if (aasworld.areasettings[areanum].contents & AREACONTENTS_SLIME) {
				event |= SE_ENTERSLIME;
			}
			// if in lava or slime
			if (event & stopevent) {
				VectorCopy(org, move->endpos);
				VectorScale(frame_test_vel, 1 / frametime, move->velocity);

				move->endarea = areanum;
				move->trace = trace;
				move->stopevent = event & stopevent;
				move->presencetype = presencetype;
				move->endcontents = pc;
				move->time = n * frametime;
				move->frames = n;
				return qtrue;
			}
		}

		onground = AAS_OnGround(org, presencetype, entnum);
		// if onground and on the ground for at least one whole frame
		if (onground) {
			if (stopevent & SE_HITGROUND) {
				VectorCopy(org, move->endpos);
				VectorScale(frame_test_vel, 1 / frametime, move->velocity);

				move->endarea = AAS_PointAreaNum(org);
				move->trace = trace;
				move->stopevent = SE_HITGROUND;
				move->presencetype = presencetype;
				move->endcontents = 0;
				move->time = n * frametime;
				move->frames = n;
				return qtrue;
			}
		} else if (stopevent & SE_LEAVEGROUND) {
			VectorCopy(org, move->endpos);
			VectorScale(frame_test_vel, 1 / frametime, move->velocity);

			move->endarea = AAS_PointAreaNum(org);
			move->trace = trace;
			move->stopevent = SE_LEAVEGROUND;
			move->presencetype = presencetype;
			move->endcontents = 0;
			move->time = n * frametime;
			move->frames = n;
			return qtrue;
		} else if (stopevent & SE_GAP) {
			aas_trace_t gaptrace;

			VectorCopy(org, start);

			start[2] += 24;

			VectorCopy(start, end);

			end[2] -= 48 + phys_maxbarrier;
			gaptrace = AAS_TraceClientBBox(start, end, PRESENCE_CROUCH, entnum);
			// if solid is found the bot cannot walk any further and will not fall into a gap
			if (!gaptrace.startsolid) {
				// if it is a gap (lower than phys_maxbarrier height)
				if (gaptrace.endpos[2] < org[2] - phys_maxbarrier) {
					if (!(AAS_PointContents(end) & CONTENTS_WATER)) {
						VectorCopy(lastorg, move->endpos);
						VectorScale(frame_test_vel, 1 / frametime, move->velocity);

						move->endarea = AAS_PointAreaNum(lastorg);
						move->trace = trace;
						move->stopevent = SE_GAP;
						move->presencetype = presencetype;
						move->endcontents = 0;
						move->time = n * frametime;
						move->frames = n;
						return qtrue;
					}
				}
			}
		}
	}

	VectorCopy(org, move->endpos);
	VectorScale(frame_test_vel, 1 / frametime, move->velocity);

	move->endarea = AAS_PointAreaNum(org);
	move->trace = trace;
	move->stopevent = SE_NONE;
	move->presencetype = presencetype;
	move->endcontents = 0;
	move->time = n * frametime;
	move->frames = n;
	return qtrue;
}

/*
=======================================================================================================================================
AAS_PredictClientMovement
=======================================================================================================================================
*/
int AAS_PredictClientMovement(struct aas_clientmove_s *move, int entnum, vec3_t origin, int presencetype, int onground, int scoutmove, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, int stopevent, int stopareanum, int visualize) {
	vec3_t mins, maxs;
	return AAS_ClientMovementPrediction(move, entnum, origin, presencetype, onground, scoutmove, velocity, cmdmove, cmdframes, maxframes, frametime, stopevent, stopareanum, mins, maxs, visualize);
}

/*
=======================================================================================================================================
AAS_ClientMovementHitBBox
=======================================================================================================================================
*/
int AAS_ClientMovementHitBBox(struct aas_clientmove_s *move, int entnum, vec3_t origin, int presencetype, int onground, int scoutmove, vec3_t velocity, vec3_t cmdmove, int cmdframes, int maxframes, float frametime, vec3_t mins, vec3_t maxs, int visualize) {
	return AAS_ClientMovementPrediction(move, entnum, origin, presencetype, onground, scoutmove, velocity, cmdmove, cmdframes, maxframes, frametime, SE_HITBOUNDINGBOX, 0, mins, maxs, visualize);
}

/*
=======================================================================================================================================
AAS_TestMovementPrediction
=======================================================================================================================================
*/
void AAS_TestMovementPrediction(int entnum, vec3_t origin, vec3_t dir) {
	vec3_t velocity, cmdmove;
	aas_clientmove_t move;

	VectorClear(velocity);

	if (!AAS_Swimming(origin)) {
		dir[2] = 0;
	}

	VectorNormalize(dir);
	VectorScale(dir, 400, cmdmove);

	cmdmove[2] = 224;

	AAS_ClearShownDebugLines();
	// movement prediction
	AAS_PredictClientMovement(&move, entnum, origin, PRESENCE_NORMAL, qtrue, qfalse, velocity, cmdmove, 13, 13, 0.1f, SE_HITGROUND, 0, qtrue); //SE_LEAVEGROUND

	if (move.stopevent & SE_LEAVEGROUND) {
		botimport.Print(PRT_MESSAGE, "leave ground\n");
	}
}

/*
=======================================================================================================================================
AAS_HorizontalVelocityForJump

Calculates the horizontal velocity needed to perform a jump from start to end.

Parameter:	zvel	: z velocity for jump.
			start	: start position of jump.
			end		: end position of jump.
			*speed	: returned speed for jump.
Returns: qfalse if too high or too far from start to end.
=======================================================================================================================================
*/
int AAS_HorizontalVelocityForJump(float zvel, vec3_t start, vec3_t end, float *velocity) {
	float phys_gravity, phys_maxvelocity;
	float maxjump, height2fall, t, top;
	vec3_t dir;

	phys_gravity = aassettings.phys_gravity;
	phys_maxvelocity = aassettings.phys_maxvelocity;
	// maximum height a player can jump with the given initial z velocity
	maxjump = 0.5 * phys_gravity * (zvel / phys_gravity) * (zvel / phys_gravity);
	// top of the parabolic jump
	top = start[2] + maxjump;
	// height the bot will fall from the top
	height2fall = top - end[2];
	// if the goal is to high to jump to
	if (height2fall < 0) {
		*velocity = phys_maxvelocity;
		return 0;
	}
	// time a player takes to fall the height
	t = sqrt(height2fall / (0.5 * phys_gravity));
	// direction from start to end
	VectorSubtract(end, start, dir);

	if ((t + zvel / phys_gravity) == 0.0f) {
		*velocity = phys_maxvelocity;
		return 0;
	}
	// calculate horizontal speed
	*velocity = sqrt(dir[0] * dir[0] + dir[1] * dir[1]) / (t + zvel / phys_gravity);
	// the horizontal speed must be lower than the max speed
	if (*velocity > phys_maxvelocity) {
		*velocity = phys_maxvelocity;
		return 0;
	}

	return 1;
}

/*
=======================================================================================================================================
AAS_HorizontalVelocityForScoutJump

Calculates the horizontal velocity needed to perform a jump from start to end using the scout powerup.

Parameter:	zvel	: z velocity for jump.
			start	: start position of jump.
			end		: end position of jump.
			*speed	: returned speed for jump.
Returns: qfalse if too high or too far from start to end.
=======================================================================================================================================
*/
int AAS_HorizontalVelocityForScoutJump(float zvel, vec3_t start, vec3_t end, float *velocity) {
	float phys_gravity, phys_maxscoutvelocity;
	float maxscoutjump, height2fall, t, top;
	vec3_t dir;

	phys_gravity = aassettings.phys_gravity;
	phys_maxscoutvelocity = aassettings.phys_maxscoutvelocity;
	// maximum height a player can jump with the given initial z velocity
	maxscoutjump = 0.5 * phys_gravity * (zvel / phys_gravity) * (zvel / phys_gravity);
	// top of the parabolic jump
	top = start[2] + maxscoutjump;
	// height the bot will fall from the top
	height2fall = top - end[2];
	// if the goal is to high to jump to
	if (height2fall < 0) {
		*velocity = phys_maxscoutvelocity;
		return 0;
	}
	// time a player takes to fall the height
	t = sqrt(height2fall / (0.5 * phys_gravity));
	// direction from start to end
	VectorSubtract(end, start, dir);

	if ((t + zvel / phys_gravity) == 0.0f) {
		*velocity = phys_maxscoutvelocity;
		return 0;
	}
	// calculate horizontal speed
	*velocity = sqrt(dir[0] * dir[0] + dir[1] * dir[1]) / (t + zvel / phys_gravity);
	// the horizontal speed must be lower than the max speed
	if (*velocity > phys_maxscoutvelocity) {
		*velocity = phys_maxscoutvelocity;
		return 0;
	}

	return 1;
}
