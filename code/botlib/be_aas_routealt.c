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
 AAS routing.
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "l_utils.h"
#include "l_memory.h"
#include "l_log.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

#define ENABLE_ALTROUTING
//#define ALTROUTE_DEBUG

typedef struct midrangearea_s {
	int valid;
	unsigned short starttime;
	unsigned short goaltime;
} midrangearea_t;

midrangearea_t *midrangeareas;
int *clusterareas;
int numclusterareas;

/*
=======================================================================================================================================
AAS_AltRoutingFloodCluster_r
=======================================================================================================================================
*/
void AAS_AltRoutingFloodCluster_r(int areanum) {
	int i, otherareanum;
	aas_area_t *area;
	aas_face_t *face;

	// add the current area to the areas of the current cluster
	clusterareas[numclusterareas] = areanum;
	numclusterareas++;
	// remove the area from the mid range areas
	midrangeareas[areanum].valid = qfalse;
	// flood to other areas through the faces of this area
	area = &aasworld.areas[areanum];

	for (i = 0; i < area->numfaces; i++) {
		face = &aasworld.faces[abs(aasworld.faceindex[area->firstface + i])];
		// get the area at the other side of the face
		if (face->frontarea == areanum) {
			otherareanum = face->backarea;
		} else {
			otherareanum = face->frontarea;
		}
		// if there is an area at the other side of this face
		if (!otherareanum) {
			continue;
		}
		// if the other area is not a midrange area
		if (!midrangeareas[otherareanum].valid) {
			continue;
		}

		AAS_AltRoutingFloodCluster_r(otherareanum);
	}
}

/*
=======================================================================================================================================
AAS_AlternativeRouteGoals
=======================================================================================================================================
*/
int AAS_AlternativeRouteGoals(vec3_t start, int startareanum, vec3_t goal, int goalareanum, int travelflags, aas_altroutegoal_t *altroutegoals, int maxaltroutegoals, int type) {
#ifndef ENABLE_ALTROUTING
	return 0;
#else
	int i, j, bestareanum;
	int numaltroutegoals, nummidrangeareas;
	int starttime, goaltime, goaltraveltime;
	float dist, bestdist;
	vec3_t mid, dir;
#ifdef ALTROUTE_DEBUG
	int startmillisecs;

	startmillisecs = botimport.MilliSeconds();
#endif
	if (!startareanum || !goalareanum) {
		return 0;
	}
	// travel time towards the goal area
	goaltraveltime = AAS_AreaTravelTimeToGoalArea(startareanum, start, goalareanum, travelflags);
	// clear the midrange areas
	Com_Memset(midrangeareas, 0, aasworld.numareas * sizeof(midrangearea_t));

	numaltroutegoals = 0;
	nummidrangeareas = 0;

	for (i = 1; i < aasworld.numareas; i++) {
		if (!(type & ALTROUTEGOAL_ALL)) {
			if (!(type & ALTROUTEGOAL_CLUSTERPORTALS && (aasworld.areasettings[i].contents & AREACONTENTS_CLUSTERPORTAL))) {
				if (!(type & ALTROUTEGOAL_VIEWPORTALS && (aasworld.areasettings[i].contents & AREACONTENTS_VIEWPORTAL))) {
					continue;
				}
			}
		}
		// if the area has no reachabilities
		if (!AAS_AreaReachability(i)) {
			continue;
		}
		// travel time from the area to the start area
		starttime = AAS_AreaTravelTimeToGoalArea(startareanum, start, i, travelflags);

		if (!starttime) {
			continue;
		}
		// if the travel time from the start to the area is greater than the shortest goal travel time
		if (starttime > (float)1.1 * goaltraveltime) {
			continue;
		}
		// travel time from the area to the goal area
		goaltime = AAS_AreaTravelTimeToGoalArea(i, NULL, goalareanum, travelflags);

		if (!goaltime) {
			continue;
		}
		// if the travel time from the area to the goal is greater than the shortest goal travel time
		if (goaltime > (float)0.8 * goaltraveltime) {
			continue;
		}
		// this is a mid range area
		midrangeareas[i].valid = qtrue;
		midrangeareas[i].starttime = starttime;
		midrangeareas[i].goaltime = goaltime;

		Log_Write("%d midrange area %d", nummidrangeareas, i);

		nummidrangeareas++;
	}

	for (i = 1; i < aasworld.numareas; i++) {
		if (!midrangeareas[i].valid) {
			continue;
		}
		// get the areas in one cluster
		numclusterareas = 0;

		AAS_AltRoutingFloodCluster_r(i);
		// now we've got a cluster with areas through which an alternative route could go
		// get the 'center' of the cluster
		VectorClear(mid);

		for (j = 0; j < numclusterareas; j++) {
			VectorAdd(mid, aasworld.areas[clusterareas[j]].center, mid);
		}

		VectorScale(mid, 1.0 / numclusterareas, mid);
		// get the area closest to the center of the cluster
		bestdist = 999999;
		bestareanum = 0;

		for (j = 0; j < numclusterareas; j++) {
			VectorSubtract(mid, aasworld.areas[clusterareas[j]].center, dir);

			dist = VectorLength(dir);

			if (dist < bestdist) {
				bestdist = dist;
				bestareanum = clusterareas[j];
			}
		}
		// now we've got an area for an alternative route
		// FIXME: add alternative goal origin
		VectorCopy(aasworld.areas[bestareanum].center, altroutegoals[numaltroutegoals].origin);

		altroutegoals[numaltroutegoals].areanum = bestareanum;
		altroutegoals[numaltroutegoals].starttraveltime = midrangeareas[bestareanum].starttime;
		altroutegoals[numaltroutegoals].goaltraveltime = midrangeareas[bestareanum].goaltime;
		altroutegoals[numaltroutegoals].extratraveltime = (midrangeareas[bestareanum].starttime + midrangeareas[bestareanum].goaltime) - goaltraveltime;

		numaltroutegoals++;
#ifdef ALTROUTE_DEBUG
		AAS_ShowAreaPolygons(bestareanum, 1, qtrue);
#endif
		// don't return more than the maximum alternative route goals
		if (numaltroutegoals >= maxaltroutegoals) {
			break;
		}
	}
#ifdef ALTROUTE_DEBUG
	botimport.Print(PRT_MESSAGE, "alternative route goals in %d msec\n", botimport.MilliSeconds() - startmillisecs);
#endif
	return numaltroutegoals;
#endif
}

/*
=======================================================================================================================================
AAS_InitAlternativeRouting
=======================================================================================================================================
*/
void AAS_InitAlternativeRouting(void) {
#ifdef ENABLE_ALTROUTING
	if (midrangeareas) {
		FreeMemory(midrangeareas);
	}

	midrangeareas = (midrangearea_t *)GetMemory(aasworld.numareas * sizeof(midrangearea_t));

	if (clusterareas) {
		FreeMemory(clusterareas);
	}

	clusterareas = (int *)GetMemory(aasworld.numareas * sizeof(int));
#endif
}

/*
=======================================================================================================================================
AAS_ShutdownAlternativeRouting
=======================================================================================================================================
*/
void AAS_ShutdownAlternativeRouting(void) {
#ifdef ENABLE_ALTROUTING
	if (midrangeareas) {
		FreeMemory(midrangeareas);
	}

	midrangeareas = NULL;

	if (clusterareas) {
		FreeMemory(clusterareas);
	}

	clusterareas = NULL;
	numclusterareas = 0;
#endif
}
