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
 AAS
**************************************************************************************************************************************/

#include "../qcommon/q_shared.h"
#include "l_memory.h"
#include "l_libvar.h"
#include "l_utils.h"
#include "l_script.h"
#include "l_precomp.h"
#include "l_struct.h"
#include "l_log.h"
#include "aasfile.h"
#include "botlib.h"
#include "be_aas.h"
#include "be_aas_funcs.h"
#include "be_interface.h"
#include "be_aas_def.h"

aas_t aasworlds[MAX_AAS_WORLDS];
aas_t *aasworld;

libvar_t *saveroutingcache;

/*
=======================================================================================================================================
AAS_Error
=======================================================================================================================================
*/
void QDECL AAS_Error(char *fmt, ...) {
	char str[1024];
	va_list arglist;

	va_start(arglist, fmt);
	Q_vsnprintf(str, sizeof(str), fmt, arglist);
	va_end(arglist);

	botimport.Print(PRT_FATAL, "%s", str);
}

/*
=======================================================================================================================================
AAS_Loaded
=======================================================================================================================================
*/
int AAS_Loaded(void) {
	return (*aasworld).loaded;
}

/*
=======================================================================================================================================
AAS_Initialized
=======================================================================================================================================
*/
int AAS_Initialized(void) {
	return (*aasworld).initialized;
}

/*
=======================================================================================================================================
AAS_SetInitialized
=======================================================================================================================================
*/
void AAS_SetInitialized(void) {

	(*aasworld).initialized = qtrue;
	botimport.Print(PRT_MESSAGE, "AAS initialized.\n");
#ifdef DEBUG
	// create all the routing cache
	//AAS_CreateAllRoutingCache();
	//AAS_RoutingInfo();
#endif
}

/*
=======================================================================================================================================
AAS_ContinueInit
=======================================================================================================================================
*/
void AAS_ContinueInit(float time) {

	// if no AAS file loaded
	if (!(*aasworld).loaded) {
		return;
	}
	// if AAS is already initialized
	if ((*aasworld).initialized) {
		return;
	}
	// calculate reachability, if not finished return
	if (AAS_ContinueInitReachability(time)) {
		return;
	}
	// initialize clustering for the new map
	AAS_InitClustering();
	// if reachability has been calculated and an AAS file should be written or there is a forced data optimization
	if ((*aasworld).savefile || ((int)LibVarGetValue("forcewrite"))) {
		// optimize the AAS data
		if ((int)LibVarValue("aasoptimize", "0")) {
			AAS_Optimize();
		}
		// save the AAS file
		if (AAS_WriteAASFile((*aasworld).filename)) {
			botimport.Print(PRT_MESSAGE, "%s written successfully\n", (*aasworld).filename);
		} else {
			botimport.Print(PRT_ERROR, "couldn't write %s\n", (*aasworld).filename);
		}
	}
	// initialize the routing
	AAS_InitRouting();
	// at this point AAS is initialized
	AAS_SetInitialized();
}

/*
=======================================================================================================================================
AAS_StartFrame

Called at the start of every frame.
=======================================================================================================================================
*/
int AAS_StartFrame(float time) {
	int i;

	// do each of the aasworlds
	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);

		(*aasworld).time = time;
		// unlink all entities that were not updated last frame
		AAS_UnlinkInvalidEntities();
		// invalidate the entities
		AAS_InvalidateEntities();
		// initialize AAS
		AAS_ContinueInit(time);

		(*aasworld).frameroutingupdates = 0;

		if (botDeveloper) {
			if (LibVarGetValue("showcacheupdates")) {
				AAS_RoutingInfo();
				LibVarSet("showcacheupdates", "0");
			}

			if (LibVarGetValue("showmemoryusage")) {
				PrintUsedMemorySize();
				LibVarSet("showmemoryusage", "0");
			}

			if (LibVarGetValue("memorydump")) {
				PrintMemoryLabels();
				LibVarSet("memorydump", "0");
			}
		}

		if (saveroutingcache->value) {
			AAS_WriteRouteCache();
			LibVarSet("saveroutingcache", "0");
		}
	}

	(*aasworld).numframes++;
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
AAS_Time
=======================================================================================================================================
*/
float AAS_Time(void) {
	return (*aasworld).time;
}

/*
=======================================================================================================================================
AAS_ProjectPointOntoVector
=======================================================================================================================================
*/
void AAS_ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj) {
	vec3_t pVec, vec;

	VectorSubtract(point, vStart, pVec);
	VectorSubtract(vEnd, vStart, vec);
	VectorNormalize(vec);
	// project onto the directional vector for this segment
	VectorMA(vStart, DotProduct(pVec, vec), vec, vProj);
}

/*
=======================================================================================================================================
AAS_LoadFiles
=======================================================================================================================================
*/
int AAS_LoadFiles(const char *mapname) {
	int errnum;
	char aasfile[MAX_QPATH];

	Q_strncpyz((*aasworld).mapname, mapname, sizeof((*aasworld).mapname));
	// NOTE: first reset the entity links into the AAS areas and BSP leaves
	// the AAS link heap and BSP link heap are reset after respectively the AAS file and BSP file are loaded
	AAS_ResetEntityLinks();
	// load bsp info
	AAS_LoadBSPFile();
	// load the aas file
	Com_sprintf(aasfile, sizeof(aasfile), "maps/%s.aas", mapname);

	errnum = AAS_LoadAASFile(aasfile);

	if (errnum != BLERR_NOERROR) {
		return errnum;
	}

	botimport.Print(PRT_MESSAGE, "loaded %s\n", aasfile);

	Q_strncpyz((*aasworld).filename, aasfile, sizeof((*aasworld).filename));
	return BLERR_NOERROR;
}

#define MAPNAME_LEN 256
/*
=======================================================================================================================================
AAS_LoadMap

Called every time a map changes.
=======================================================================================================================================
*/
int AAS_LoadMap(const char *mapname) {
	int missingErrNum, i, errnum;
	char this_mapname[MAPNAME_LEN], intstr[4];
	qboolean loaded;

	loaded = qfalse;
	missingErrNum = 0;

	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);

		Q_strncpyz(this_mapname, mapname, sizeof(this_mapname));
		Q_strcat(this_mapname, sizeof(this_mapname) - strlen(this_mapname) - 1, "_b");
		Com_sprintf(intstr, sizeof(intstr), "%i", i);
		Q_strcat(this_mapname, sizeof(this_mapname) - strlen(this_mapname) - 1, intstr);
		// if no mapname is provided then the string indexes are updated
		if (!mapname) {
			return 0;
		}

		(*aasworld).initialized = qfalse;
		// NOTE: free the routing caches before loading a new map because to free the caches the old number of areas, number of clusters
		// and number of areas in a clusters must be available
		AAS_FreeRoutingCaches();
		// load the map
		errnum = AAS_LoadFiles(this_mapname);

		if (errnum != BLERR_NOERROR) {
			(*aasworld).loaded = qfalse;
			// we are allowed to skip one of the files, but not both
			missingErrNum = errnum;
			continue;
		}

		loaded = qtrue;

		AAS_InitSettings();
		// initialize the AAS link heap for the new map
		AAS_InitAASLinkHeap();
		// initialize the AAS linked entities for the new map
		AAS_InitAASLinkedEntities();
		// initialize reachability for the new map
		AAS_InitReachability();
		// initialize the alternative routing
		AAS_InitAlternativeRouting();
	}

	if (!loaded) {
		return missingErrNum;
	}
	// everything went ok
	return 0;
}

/*
=======================================================================================================================================
AAS_Setup

Called when the library is first loaded.
=======================================================================================================================================
*/
int AAS_Setup(void) {

	// just use the default world for entities
	AAS_SetCurrentWorld(0);

	(*aasworlds).maxclients = (int)LibVarValue("maxclients", "128");
	(*aasworlds).maxentities = (int)LibVarValue("maxentities", "2048");
	// as soon as it's set to 1 the routing cache will be saved
	saveroutingcache = LibVar("saveroutingcache", "0");
	// allocate memory for the entities
	if ((*aasworld).entities) {
		FreeMemory((*aasworld).entities);
	}

	(*aasworld).entities = (aas_entity_t *)GetClearedHunkMemory((*aasworld).maxentities * sizeof(aas_entity_t));
	// invalidate all the entities
	AAS_InvalidateEntities();
	// force some recalculations
	//LibVarSet("forceclustering", "1"); // force clustering calculation
	//LibVarSet("forcereachability", "1"); // force reachability calculation
	(*aasworld).numframes = 0;
	return BLERR_NOERROR;
}

/*
=======================================================================================================================================
AAS_SetCurrentWorld

Multiple AAS worlds.
=======================================================================================================================================
*/
void AAS_SetCurrentWorld(int index) {

	if (index >= MAX_AAS_WORLDS || index < 0) {
		AAS_Error("AAS_SetCurrentWorld: index out of range\n");
		return;
	}
	// set the current world pointer
	aasworld = &aasworlds[index];
}

/*
=======================================================================================================================================
AAS_Shutdown
=======================================================================================================================================
*/
void AAS_Shutdown(void) {
	int i;

	// do each of the worlds
	for (i = 0; i < MAX_AAS_WORLDS; i++) {
		AAS_SetCurrentWorld(i);
		AAS_ShutdownAlternativeRouting();
		AAS_DumpBSPData();
		// free routing caches
		AAS_FreeRoutingCaches();
		// free aas link heap
		AAS_FreeAASLinkHeap();
		// free aas linked entities
		AAS_FreeAASLinkedEntities();
		// free the aas data
		AAS_DumpAASData();

		if (i == 0) {
			// free the entities
			if ((*aasworld).entities) {
				FreeMemory((*aasworld).entities);
			}
		}
		// clear the (*aasworld) structure
		Com_Memset(&(*aasworld), 0, sizeof(aas_t));
		// aas has not been initialized
		(*aasworld).initialized = qfalse;
	}
	// NOTE: as soon as a new .bsp file is loaded the .bsp file memory is freed and reallocated, so there's no need to free that memory here
	// print shutdown
	botimport.Print(PRT_MESSAGE, "AAS shutdown.\n");
}
