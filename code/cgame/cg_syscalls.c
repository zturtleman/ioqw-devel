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
 This file is only included when building a dll, cg_syscalls.asm is included instead when building a qvm.
**************************************************************************************************************************************/

#ifdef Q3_VM
#error "Do not use in VM build"
#endif
#include "cg_local.h"

static intptr_t(QDECL *syscall)(intptr_t arg, ...) = (intptr_t(QDECL *)(intptr_t, ...)) - 1;

/*
=======================================================================================================================================
dllEntry
=======================================================================================================================================
*/
Q_EXPORT void dllEntry(intptr_t(QDECL *syscallptr)(intptr_t arg, ...)) {
	syscall = syscallptr;
}

/*
=======================================================================================================================================
PASSFLOAT
=======================================================================================================================================
*/
int PASSFLOAT(float x) {
	floatint_t fi;

	fi.f = x;
	return fi.i;
}

/*
=======================================================================================================================================
trap_Print
=======================================================================================================================================
*/
void trap_Print(const char *fmt) {
	syscall(CG_PRINT, fmt);
}

/*
=======================================================================================================================================
trap_Error
=======================================================================================================================================
*/
void trap_Error(const char *fmt) {

	syscall(CG_ERROR, fmt);
	// shut up GCC warning about returning functions, because we know better
	exit(1);
}

/*
=======================================================================================================================================
trap_Milliseconds
=======================================================================================================================================
*/
int trap_Milliseconds(void) {
	return syscall(CG_MILLISECONDS);
}

/*
=======================================================================================================================================
trap_RealTime
=======================================================================================================================================
*/
int trap_RealTime(qtime_t *qtime) {
	return syscall(CG_REAL_TIME, qtime);
}

/*
=======================================================================================================================================
trap_SnapVector
=======================================================================================================================================
*/
void trap_SnapVector(float *v) {
	syscall(CG_SNAPVECTOR, v);
}

/*
=======================================================================================================================================
trap_Argc
=======================================================================================================================================
*/
int trap_Argc(void) {
	return syscall(CG_ARGC);
}

/*
=======================================================================================================================================
trap_Argv
=======================================================================================================================================
*/
void trap_Argv(int n, char *buffer, int bufferLength) {
	syscall(CG_ARGV, n, buffer, bufferLength);
}

/*
=======================================================================================================================================
trap_Args
=======================================================================================================================================
*/
void trap_Args(char *buffer, int bufferLength) {
	syscall(CG_ARGS, buffer, bufferLength);
}

/*
=======================================================================================================================================
trap_AddCommand
=======================================================================================================================================
*/
void trap_AddCommand(const char *cmdName) {
	syscall(CG_ADDCOMMAND, cmdName);
}

/*
=======================================================================================================================================
trap_RemoveCommand
=======================================================================================================================================
*/
void trap_RemoveCommand(const char *cmdName) {
	syscall(CG_REMOVECOMMAND, cmdName);
}

/*
=======================================================================================================================================
trap_Cmd_ExecuteText
=======================================================================================================================================
*/
void trap_Cmd_ExecuteText(int exec_when, const char *text) {
	syscall(CG_CMD_EXECUTETEXT, exec_when, text);
}

/*
=======================================================================================================================================
trap_Cvar_Register
=======================================================================================================================================
*/
void trap_Cvar_Register(vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags) {
	syscall(CG_CVAR_REGISTER, vmCvar, varName, defaultValue, flags);
}

/*
=======================================================================================================================================
trap_Cvar_Update
=======================================================================================================================================
*/
void trap_Cvar_Update(vmCvar_t *vmCvar) {
	syscall(CG_CVAR_UPDATE, vmCvar);
}

/*
=======================================================================================================================================
trap_Cvar_Set
=======================================================================================================================================
*/
void trap_Cvar_Set(const char *var_name, const char *value) {
	syscall(CG_CVAR_SET, var_name, value);
}

/*
=======================================================================================================================================
trap_Cvar_SetValue
=======================================================================================================================================
*/
void trap_Cvar_SetValue(const char *var_name, float value) {
	syscall(CG_CVAR_SET_VALUE, var_name, PASSFLOAT(value));
}

/*
=======================================================================================================================================
trap_Cvar_VariableValue
=======================================================================================================================================
*/
float trap_Cvar_VariableValue(const char *var_name) {
	floatint_t fi;

	fi.i = syscall(CG_CVAR_VARIABLE_VALUE, var_name);
	return fi.f;
}

/*
=======================================================================================================================================
trap_Cvar_VariableIntegerValue
=======================================================================================================================================
*/
int trap_Cvar_VariableIntegerValue(const char *var_name) {
	return syscall(CG_CVAR_VARIABLE_INTEGER_VALUE, var_name);
}

/*
=======================================================================================================================================
trap_Cvar_VariableStringBuffer
=======================================================================================================================================
*/
void trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize) {
	syscall(CG_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize);
}

/*
=======================================================================================================================================
trap_FS_FOpenFile
=======================================================================================================================================
*/
int trap_FS_FOpenFile(const char *qpath, fileHandle_t *f, fsMode_t mode) {
	return syscall(CG_FS_FOPENFILE, qpath, f, mode);
}

/*
=======================================================================================================================================
trap_FS_Read
=======================================================================================================================================
*/
void trap_FS_Read(void *buffer, int len, fileHandle_t f) {
	syscall(CG_FS_READ, buffer, len, f);
}

/*
=======================================================================================================================================
trap_FS_Write
=======================================================================================================================================
*/
void trap_FS_Write(const void *buffer, int len, fileHandle_t f) {
	syscall(CG_FS_WRITE, buffer, len, f);
}

/*
=======================================================================================================================================
trap_FS_Seek
=======================================================================================================================================
*/
int trap_FS_Seek(fileHandle_t f, long offset, int origin) {
	return syscall(CG_FS_SEEK, f, offset, origin);
}

/*
=======================================================================================================================================
trap_FS_FCloseFile
=======================================================================================================================================
*/
void trap_FS_FCloseFile(fileHandle_t f) {
	syscall(CG_FS_FCLOSEFILE, f);
}

/*
=======================================================================================================================================
trap_FS_GetFileList
=======================================================================================================================================
*/
int trap_FS_GetFileList(const char *path, const char *extension, char *listbuf, int bufsize) {
	return syscall(CG_FS_GETFILELIST, path, extension, listbuf, bufsize);
}

/*
=======================================================================================================================================
trap_PC_AddGlobalDefine
=======================================================================================================================================
*/
int trap_PC_AddGlobalDefine(const char *define) {
	return syscall(CG_PC_ADD_GLOBAL_DEFINE, define);
}

/*
=======================================================================================================================================
trap_PC_LoadSource
=======================================================================================================================================
*/
int trap_PC_LoadSource(const char *filename) {
	return syscall(CG_PC_LOAD_SOURCE, filename);
}

/*
=======================================================================================================================================
trap_PC_FreeSource
=======================================================================================================================================
*/
int trap_PC_FreeSource(int handle) {
	return syscall(CG_PC_FREE_SOURCE, handle);
}

/*
=======================================================================================================================================
trap_PC_ReadToken
=======================================================================================================================================
*/
int trap_PC_ReadToken(int handle, pc_token_t *pc_token) {
	return syscall(CG_PC_READ_TOKEN, handle, pc_token);
}

/*
=======================================================================================================================================
trap_PC_SourceFileAndLine
=======================================================================================================================================
*/
int trap_PC_SourceFileAndLine(int handle, char *filename, int *line) {
	return syscall(CG_PC_SOURCE_FILE_AND_LINE, handle, filename, line);
}

/*
=======================================================================================================================================
trap_GetGlconfig
=======================================================================================================================================
*/
void trap_GetGlconfig(glconfig_t *glconfig) {
	syscall(CG_GETGLCONFIG, glconfig);
}

/*
=======================================================================================================================================
trap_MemoryRemaining
=======================================================================================================================================
*/
int trap_MemoryRemaining(void) {
	return syscall(CG_MEMORY_REMAINING);
}

/*
=======================================================================================================================================
trap_UpdateScreen
=======================================================================================================================================
*/
void trap_UpdateScreen(void) {
	syscall(CG_UPDATESCREEN);
}

/*
=======================================================================================================================================
trap_GetGameState
=======================================================================================================================================
*/
void trap_GetGameState(gameState_t *gamestate) {
	syscall(CG_GETGAMESTATE, gamestate);
}

/*
=======================================================================================================================================
trap_GetCurrentSnapshotNumber
=======================================================================================================================================
*/
void trap_GetCurrentSnapshotNumber(int *snapshotNumber, int *serverTime) {
	syscall(CG_GETCURRENTSNAPSHOTNUMBER, snapshotNumber, serverTime);
}

/*
=======================================================================================================================================
trap_GetSnapshot
=======================================================================================================================================
*/
qboolean trap_GetSnapshot(int snapshotNumber, snapshot_t *snapshot) {
	return syscall(CG_GETSNAPSHOT, snapshotNumber, snapshot);
}

/*
=======================================================================================================================================
trap_GetServerCommand
=======================================================================================================================================
*/
qboolean trap_GetServerCommand(int serverCommandNumber) {
	return syscall(CG_GETSERVERCOMMAND, serverCommandNumber);
}

/*
=======================================================================================================================================
trap_GetCurrentCmdNumber
=======================================================================================================================================
*/
int trap_GetCurrentCmdNumber(void) {
	return syscall(CG_GETCURRENTCMDNUMBER);
}

/*
=======================================================================================================================================
trap_GetUserCmd
=======================================================================================================================================
*/
qboolean trap_GetUserCmd(int cmdNumber, usercmd_t *ucmd) {
	return syscall(CG_GETUSERCMD, cmdNumber, ucmd);
}

/*
=======================================================================================================================================
trap_SetUserCmdValue
=======================================================================================================================================
*/
void trap_SetUserCmdValue(int stateValue, float sensitivityScale) {
	syscall(CG_SETUSERCMDVALUE, stateValue, PASSFLOAT(sensitivityScale));
}

/*
=======================================================================================================================================
trap_SendClientCommand
=======================================================================================================================================
*/
void trap_SendClientCommand(const char *s) {
	syscall(CG_SENDCLIENTCOMMAND, s);
}

/*
=======================================================================================================================================
trap_CM_LoadMap
=======================================================================================================================================
*/
void trap_CM_LoadMap(const char *mapname) {
	syscall(CG_CM_LOADMAP, mapname);
}

/*
=======================================================================================================================================
trap_CM_NumInlineModels
=======================================================================================================================================
*/
int trap_CM_NumInlineModels(void) {
	return syscall(CG_CM_NUMINLINEMODELS);
}

/*
=======================================================================================================================================
trap_CM_InlineModel
=======================================================================================================================================
*/
clipHandle_t trap_CM_InlineModel(int index) {
	return syscall(CG_CM_INLINEMODEL, index);
}

/*
=======================================================================================================================================
trap_CM_MarkFragments
=======================================================================================================================================
*/
int trap_CM_MarkFragments(int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer) {
	return syscall(CG_CM_MARKFRAGMENTS, numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer);
}

/*
=======================================================================================================================================
trap_CM_PointContents
=======================================================================================================================================
*/
int trap_CM_PointContents(const vec3_t p, clipHandle_t model) {
	return syscall(CG_CM_POINTCONTENTS, p, model);
}

/*
=======================================================================================================================================
trap_CM_TransformedPointContents
=======================================================================================================================================
*/
int trap_CM_TransformedPointContents(const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles) {
	return syscall(CG_CM_TRANSFORMEDPOINTCONTENTS, p, model, origin, angles);
}

/*
=======================================================================================================================================
trap_CM_TempBoxModel
=======================================================================================================================================
*/
clipHandle_t trap_CM_TempBoxModel(const vec3_t mins, const vec3_t maxs) {
	return syscall(CG_CM_TEMPBOXMODEL, mins, maxs);
}

/*
=======================================================================================================================================
trap_CM_BoxTrace
=======================================================================================================================================
*/
void trap_CM_BoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask) {
	syscall(CG_CM_BOXTRACE, results, start, end, mins, maxs, model, brushmask);
}

/*
=======================================================================================================================================
trap_CM_TransformedBoxTrace
=======================================================================================================================================
*/
void trap_CM_TransformedBoxTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles) {
	syscall(CG_CM_TRANSFORMEDBOXTRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}

/*
=======================================================================================================================================
trap_CM_TempCapsuleModel
=======================================================================================================================================
*/
clipHandle_t trap_CM_TempCapsuleModel(const vec3_t mins, const vec3_t maxs) {
	return syscall(CG_CM_TEMPCAPSULEMODEL, mins, maxs);
}

/*
=======================================================================================================================================
trap_CM_CapsuleTrace
=======================================================================================================================================
*/
void trap_CM_CapsuleTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask) {
	syscall(CG_CM_CAPSULETRACE, results, start, end, mins, maxs, model, brushmask);
}

/*
=======================================================================================================================================
trap_CM_TransformedCapsuleTrace
=======================================================================================================================================
*/
void trap_CM_TransformedCapsuleTrace(trace_t *results, const vec3_t start, const vec3_t end, const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles) {
	syscall(CG_CM_TRANSFORMEDCAPSULETRACE, results, start, end, mins, maxs, model, brushmask, origin, angles);
}

/*
=======================================================================================================================================
trap_R_RegisterModel
=======================================================================================================================================
*/
qhandle_t trap_R_RegisterModel(const char *name) {
	return syscall(CG_R_REGISTERMODEL, name);
}

/*
=======================================================================================================================================
trap_R_RegisterShaderEx
=======================================================================================================================================
*/
qhandle_t trap_R_RegisterShaderEx(const char *name, int lightmapIndex, qboolean mipRawImage) {
	return syscall(CG_R_REGISTERSHADEREX, name, lightmapIndex, mipRawImage);
}

/*
=======================================================================================================================================
trap_R_RegisterShader
=======================================================================================================================================
*/
qhandle_t trap_R_RegisterShader(const char *name) {
	return syscall(CG_R_REGISTERSHADER, name);
}

/*
=======================================================================================================================================
trap_R_RegisterShaderNoMip
=======================================================================================================================================
*/
qhandle_t trap_R_RegisterShaderNoMip(const char *name) {
	return syscall(CG_R_REGISTERSHADERNOMIP, name);
}

/*
=======================================================================================================================================
trap_R_RegisterFont
=======================================================================================================================================
*/
void trap_R_RegisterFont(const char *fontName, int pointSize, float borderWidth, qboolean forceAutoHint, fontInfo_t *font) {
	syscall(CG_R_REGISTERFONT, fontName, pointSize, PASSFLOAT(borderWidth), forceAutoHint, font);
}

/*
=======================================================================================================================================
trap_R_RenderScene
=======================================================================================================================================
*/
void trap_R_RenderScene(const refdef_t *fd) {
	syscall(CG_R_RENDERSCENE, fd);
}

/*
=======================================================================================================================================
trap_R_ClearScene
=======================================================================================================================================
*/
void trap_R_ClearScene(void) {
	syscall(CG_R_CLEARSCENE);
}

/*
=======================================================================================================================================
trap_R_SetColor
=======================================================================================================================================
*/
void trap_R_SetColor(const float *rgba) {
	syscall(CG_R_SETCOLOR, rgba);
}

/*
=======================================================================================================================================
trap_R_LoadWorldMap
=======================================================================================================================================
*/
void trap_R_LoadWorldMap(const char *mapname) {
	syscall(CG_R_LOADWORLDMAP, mapname);
}

/*
=======================================================================================================================================
trap_R_inPVS
=======================================================================================================================================
*/
qboolean trap_R_inPVS(const vec3_t p1, const vec3_t p2) {
	return syscall(CG_R_INPVS, p1, p2);
}

/*
=======================================================================================================================================
trap_GetEntityToken
=======================================================================================================================================
*/
qboolean trap_GetEntityToken(char *buffer, int bufferSize) {
	return syscall(CG_GET_ENTITY_TOKEN, buffer, bufferSize);
}

/*
=======================================================================================================================================
trap_R_AddRefEntityToScene
=======================================================================================================================================
*/
void trap_R_AddRefEntityToScene(const refEntity_t *re) {
	syscall(CG_R_ADDREFENTITYTOSCENE, re, sizeof(refEntity_t));
}

/*
=======================================================================================================================================
trap_R_AddPolyRefEntityToScene
=======================================================================================================================================
*/
void trap_R_AddPolyRefEntityToScene(const refEntity_t *re, int numVerts, const polyVert_t *verts, int numPolys) {
	syscall(CG_R_ADDPOLYREFENTITYTOSCENE, re, sizeof(refEntity_t), numVerts, verts, numPolys);
}

/*
=======================================================================================================================================
trap_R_AddPolyToScene
=======================================================================================================================================
*/
void trap_R_AddPolyToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts, int bmodelNum, int sortLevel) {
	syscall(CG_R_ADDPOLYTOSCENE, hShader, numVerts, verts, bmodelNum, sortLevel);
}

/*
=======================================================================================================================================
trap_R_AddPolysToScene
=======================================================================================================================================
*/
void trap_R_AddPolysToScene(qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys, int bmodelNum, int sortLevel) {
	syscall(CG_R_ADDPOLYSTOSCENE, hShader, numVerts, verts, numPolys, bmodelNum, sortLevel);
}

/*
=======================================================================================================================================
trap_R_AddPolyBufferToScene
=======================================================================================================================================
*/
void trap_R_AddPolyBufferToScene(polyBuffer_t *pPolyBuffer) {
	syscall(CG_R_ADDPOLYBUFFERTOSCENE, pPolyBuffer);
}

/*
=======================================================================================================================================
trap_R_LightForPoint
=======================================================================================================================================
*/
int trap_R_LightForPoint(vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir) {
	return syscall(CG_R_LIGHTFORPOINT, point, ambientLight, directedLight, lightDir);
}

/*
=======================================================================================================================================
trap_R_AddLightToScene
=======================================================================================================================================
*/
void trap_R_AddLightToScene(const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader) {
	syscall(CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b), hShader);
}

/*
=======================================================================================================================================
trap_R_AddAdditiveLightToScene
=======================================================================================================================================
*/
void trap_R_AddAdditiveLightToScene(const vec3_t org, float radius, float intensity, float r, float g, float b) {
	syscall(CG_R_ADDADDITIVELIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b));
}

/*
=======================================================================================================================================
trap_R_AddVertexLightToScene
=======================================================================================================================================
*/
void trap_R_AddVertexLightToScene(const vec3_t org, float radius, float intensity, float r, float g, float b) {
	syscall(CG_R_ADDVERTEXLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b));
}

/*
=======================================================================================================================================
trap_R_AddJuniorLightToScene
=======================================================================================================================================
*/
void trap_R_AddJuniorLightToScene(const vec3_t org, float radius, float intensity, float r, float g, float b) {
	syscall(CG_R_ADDJUNIORLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b));
}

/*
=======================================================================================================================================
trap_R_AddDirectedLightToScene
=======================================================================================================================================
*/
void trap_R_AddDirectedLightToScene(const vec3_t normal, float intensity, float r, float g, float b) {
	syscall(CG_R_ADDDIRECTEDLIGHTTOSCENE, normal, PASSFLOAT(intensity), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b));
}

/*
=======================================================================================================================================
trap_R_AddCoronaToScene
=======================================================================================================================================
*/
void trap_R_AddCoronaToScene(const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible, qhandle_t hShader) {
	syscall(CG_R_ADDCORONATOSCENE, org, PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b), PASSFLOAT(scale), id, visible, hShader);
}

/*
=======================================================================================================================================
trap_R_GetGlobalFog
=======================================================================================================================================
*/
void trap_R_GetGlobalFog(fogType_t *type, vec3_t color, float *depthForOpaque, float *density, float *farClip) {
	syscall(CG_R_GET_GLOBAL_FOG, type, color, depthForOpaque, density, farClip);
}

/*
=======================================================================================================================================
trap_R_GetViewFog
=======================================================================================================================================
*/
void trap_R_GetViewFog(const vec3_t origin, fogType_t *type, vec3_t color, float *depthForOpaque, float *density, float *farClip, qboolean inwater) {
	syscall(CG_R_GET_VIEW_FOG, origin, type, color, depthForOpaque, density, farClip, inwater);
}

/*
=======================================================================================================================================
trap_R_ModelBounds
=======================================================================================================================================
*/
int trap_R_ModelBounds(clipHandle_t model, vec3_t mins, vec3_t maxs, int startFrame, int endFrame, float frac) {
	return syscall(CG_R_MODELBOUNDS, model, mins, maxs, startFrame, endFrame, PASSFLOAT(frac));
}

/*
=======================================================================================================================================
trap_R_LerpTag
=======================================================================================================================================
*/
int trap_R_LerpTag(orientation_t *tag, clipHandle_t mod, int startFrame, int endFrame, float frac, const char *tagName) {
	return syscall(CG_R_LERPTAG, tag, mod, startFrame, endFrame, PASSFLOAT(frac), tagName);
}

/*
=======================================================================================================================================
trap_R_LerpTagFrameModel
=======================================================================================================================================
*/
int trap_R_LerpTagFrameModel(orientation_t *tag, clipHandle_t mod, clipHandle_t frameModel, int startFrame, clipHandle_t endFrameModel, int endFrame, float frac, const char *tagName, int *tagIndex) {
	return syscall(CG_R_LERPTAG_FRAMEMODEL, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex);
}

/*
=======================================================================================================================================
trap_R_LerpTagTorso
=======================================================================================================================================
*/
int trap_R_LerpTagTorso(orientation_t *tag, clipHandle_t mod, clipHandle_t frameModel, int startFrame, clipHandle_t endFrameModel, int endFrame, float frac, const char *tagName, int *tagIndex, const vec3_t *torsoAxis, qhandle_t torsoFrameModel, int torsoFrame, qhandle_t oldTorsoFrameModel, int oldTorsoFrame, float torsoFrac) {
	return syscall(CG_R_LERPTAG_TORSO, tag, mod, frameModel, startFrame, endFrameModel, endFrame, PASSFLOAT(frac), tagName, tagIndex, torsoAxis, torsoFrameModel, torsoFrame, oldTorsoFrameModel, oldTorsoFrame, PASSFLOAT(torsoFrac));
}

/*
=======================================================================================================================================
trap_R_AllocSkinSurface
=======================================================================================================================================
*/
qhandle_t trap_R_AllocSkinSurface(const char *surface, qhandle_t hShader) {
	return syscall(CG_R_ALLOCSKINSURFACE, surface, hShader);
}

/*
=======================================================================================================================================
trap_R_AddSkinToFrame
=======================================================================================================================================
*/
qhandle_t trap_R_AddSkinToFrame(int numSurfaces, const qhandle_t *surfaces) {
	return syscall(CG_R_ADDSKINTOFRAME, numSurfaces, surfaces);
}

/*
=======================================================================================================================================
trap_R_SetClipRegion
=======================================================================================================================================
*/
void trap_R_SetClipRegion(const float *region) {
	syscall(CG_R_SETCLIPREGION, region);
}

/*
=======================================================================================================================================
trap_R_DrawStretchPic
=======================================================================================================================================
*/
void trap_R_DrawStretchPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader) {
	syscall(CG_R_DRAWSTRETCHPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader);
}

/*
=======================================================================================================================================
trap_R_DrawStretchPicGradient
=======================================================================================================================================
*/
void trap_R_DrawStretchPicGradient(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor) {
	syscall(CG_R_DRAWSTRETCHPIC_GRADIENT, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader, gradientColor);
}

/*
=======================================================================================================================================
trap_R_DrawRotatedPic
=======================================================================================================================================
*/
void trap_R_DrawRotatedPic(float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle) {
	syscall(CG_R_DRAWROTATEDPIC, PASSFLOAT(x), PASSFLOAT(y), PASSFLOAT(w), PASSFLOAT(h), PASSFLOAT(s1), PASSFLOAT(t1), PASSFLOAT(s2), PASSFLOAT(t2), hShader, PASSFLOAT(angle));
}

/*
=======================================================================================================================================
trap_R_Add2dPolys
=======================================================================================================================================
*/
void trap_R_Add2dPolys(polyVert_t *verts, int numverts, qhandle_t hShader) {
	syscall(CG_R_DRAW2DPOLYS, verts, numverts, hShader);
}

/*
=======================================================================================================================================
trap_R_RemapShader
=======================================================================================================================================
*/
void trap_R_RemapShader(const char *oldShader, const char *newShader, const char *timeOffset) {
	syscall(CG_R_REMAP_SHADER, oldShader, newShader, timeOffset);
}

/*
=======================================================================================================================================
trap_S_RegisterSound
=======================================================================================================================================
*/
sfxHandle_t trap_S_RegisterSound(const char *sample, qboolean compressed) {
	return syscall(CG_S_REGISTERSOUND, sample, compressed);
}

/*
=======================================================================================================================================
trap_S_StartLocalSound
=======================================================================================================================================
*/
void trap_S_StartLocalSound(sfxHandle_t sfx, int channelNum) {
	syscall(CG_S_STARTLOCALSOUND, sfx, channelNum);
}

/*
=======================================================================================================================================
trap_S_StartBackgroundTrack
=======================================================================================================================================
*/
void trap_S_StartBackgroundTrack(const char *intro, const char *loop) {
	syscall(CG_S_STARTBACKGROUNDTRACK, intro, loop);
}

/*
=======================================================================================================================================
trap_S_StopBackgroundTrack
=======================================================================================================================================
*/
void trap_S_StopBackgroundTrack(void) {
	syscall(CG_S_STOPBACKGROUNDTRACK);
}

/*
=======================================================================================================================================
trap_S_StartSound
=======================================================================================================================================
*/
void trap_S_StartSound(const vec3_t origin, int entityNum, int entchannel, sfxHandle_t sfx, int range) {
	syscall(CG_S_STARTSOUND, origin, entityNum, entchannel, sfx, range);
}

/*
=======================================================================================================================================
trap_S_ClearLoopingSounds
=======================================================================================================================================
*/
void trap_S_ClearLoopingSounds(qboolean killall) {
	syscall(CG_S_CLEARLOOPINGSOUNDS, killall);
}

/*
=======================================================================================================================================
trap_S_AddLoopingSound
=======================================================================================================================================
*/
void trap_S_AddLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range) {
	syscall(CG_S_ADDLOOPINGSOUND, entityNum, origin, velocity, sfx, range);
}

/*
=======================================================================================================================================
trap_S_AddRealLoopingSound
=======================================================================================================================================
*/
void trap_S_AddRealLoopingSound(int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range) {
	syscall(CG_S_ADDREALLOOPINGSOUND, entityNum, origin, velocity, sfx, range);
}

/*
=======================================================================================================================================
trap_S_StopLoopingSound
=======================================================================================================================================
*/
void trap_S_StopLoopingSound(int entityNum) {
	syscall(CG_S_STOPLOOPINGSOUND, entityNum);
}

/*
=======================================================================================================================================
trap_S_UpdateEntityPosition
=======================================================================================================================================
*/
void trap_S_UpdateEntityPosition(int entityNum, const vec3_t origin) {
	syscall(CG_S_UPDATEENTITYPOSITION, entityNum, origin);
}

/*
=======================================================================================================================================
trap_S_Respatialize
=======================================================================================================================================
*/
void trap_S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3], int inwater) {
	syscall(CG_S_RESPATIALIZE, entityNum, origin, axis, inwater);
}

/*
=======================================================================================================================================
trap_Key_GetCatcher
=======================================================================================================================================
*/
int trap_Key_GetCatcher(void) {
	return syscall(CG_KEY_GETCATCHER);
}

/*
=======================================================================================================================================
trap_Key_SetCatcher
=======================================================================================================================================
*/
void trap_Key_SetCatcher(int catcher) {
	syscall(CG_KEY_SETCATCHER, catcher);
}

/*
=======================================================================================================================================
trap_Key_IsDown
=======================================================================================================================================
*/
qboolean trap_Key_IsDown(int keynum) {
	return syscall(CG_KEY_ISDOWN, keynum);
}

/*
=======================================================================================================================================
trap_Key_GetKey
=======================================================================================================================================
*/
int trap_Key_GetKey(const char *binding) {
	return syscall(CG_KEY_GETKEY, binding);
}

/*
=======================================================================================================================================
trap_CIN_PlayCinematic

This returns a handle. arg0 is the name in the format "idlogo.roq", set arg1 to NULL, alteredstates to qfalse (do not alter gamestate).
=======================================================================================================================================
*/
int trap_CIN_PlayCinematic(const char *arg0, int xpos, int ypos, int width, int height, int bits) {
	return syscall(CG_CIN_PLAYCINEMATIC, arg0, xpos, ypos, width, height, bits);
}

/*
=======================================================================================================================================
trap_CIN_StopCinematic

Stops playing the cinematic and ends it. Should always return FMV_EOF. Cinematics must be stopped in reverse order of when they are
started.
=======================================================================================================================================
*/
e_status trap_CIN_StopCinematic(int handle) {
	return syscall(CG_CIN_STOPCINEMATIC, handle);
}

/*
=======================================================================================================================================
trap_CIN_RunCinematic

Will run a frame of the cinematic but will not draw it. Will return FMV_EOF if the end of the cinematic has been reached.
=======================================================================================================================================
*/
e_status trap_CIN_RunCinematic(int handle) {
	return syscall(CG_CIN_RUNCINEMATIC, handle);
}

/*
=======================================================================================================================================
trap_CIN_DrawCinematic

Draws the current frame.
=======================================================================================================================================
*/
void trap_CIN_DrawCinematic(int handle) {
	syscall(CG_CIN_DRAWCINEMATIC, handle);
}

/*
=======================================================================================================================================
trap_CIN_SetExtents

Allows you to resize the animation dynamically.
=======================================================================================================================================
*/
void trap_CIN_SetExtents(int handle, int x, int y, int w, int h) {
	syscall(CG_CIN_SETEXTENTS, handle, x, y, w, h);
}

/*
=======================================================================================================================================
trap_loadCamera
=======================================================================================================================================
*/
/*
qboolean trap_loadCamera(const char *name) {
	return syscall(CG_LOADCAMERA, name);
}
*/
/*
=======================================================================================================================================
trap_startCamera
=======================================================================================================================================
*/
/*
void trap_startCamera(int time) {
	syscall(CG_STARTCAMERA, time);
}
*/
/*
=======================================================================================================================================
trap_getCameraInfo
=======================================================================================================================================
*/
/*
qboolean trap_getCameraInfo(int time, vec3_t *origin, vec3_t *angles) {
	return syscall(CG_GETCAMERAINFO, time, origin, angles);
}
*/
