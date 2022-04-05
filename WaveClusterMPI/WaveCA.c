#include "WaveCA.h"
#include <stdlib.h>
#include <time.h>

struct CALModel2D* wave;
struct CALRun2D* wave_simulation;
struct CALSubstate2Dr *Q_old;
struct CALSubstate2Dr *Q_old_2_steps;
struct CALSubstate2Dr *Q_new;
CALreal max = 90.0f, min = 1.0f;

struct CALModel2D* getWaveModel(){
	return wave;
}

struct CALSubstate2Dr* getWaveMainSubstate(){
	return Q_old;
}

struct CALRun2D* getWaveRun(){
	return wave_simulation;
}

/*
5 | 1 | 8
---|---|---
2 | 0 | 3
---|---|---
6 | 4 | 7
*/
void wave_transition_function(struct CALModel2D* wave, int i, int j){
	CALreal tmp = 0.0f;

	if(!(i==0 || j==0 || i==calglGetGlobalSettings()->rows-1 || j==calglGetGlobalSettings()->columns-1)){
		tmp = 2 * calGet2Dr(wave, Q_old, i, j)
			- calGet2Dr(wave, Q_old_2_steps, i, j)
			+ (calGetX2Dr(wave, Q_old, i, j, 1)
			+ calGetX2Dr(wave, Q_old, i, j, 4)
			+ calGetX2Dr(wave, Q_old, i, j, 3)
			+ calGetX2Dr(wave, Q_old, i, j, 2)
			- 4 * calGet2Dr(wave, Q_old, i, j))/4;

		calSet2Dr(wave, Q_old_2_steps, i, j, calGet2Dr(wave, Q_old, i, j));
		calSet2Dr(wave, Q_old, i, j, tmp);
		calSet2Dr(wave, Q_new, i, j, 0.0);
	}
}

void wave_smooth_function(struct CALModel2D* life, int i, int j){
	int k = 0, n = 5, discreteTmp = 0;
	CALreal tmp = 0.0f, sum = 0.0f;

	if(!(i==0 || j==0 || i==calglGetGlobalSettings()->rows-1 || j==calglGetGlobalSettings()->columns-1)){
		if(wave_simulation->step%10==0){
			for(k=0; k<n; k++){
				sum += calGetX2Dr(life, Q_old, i, j, k);
			}
			tmp = sum/n;
			calSet2Dr(life, Q_old, i, j, tmp);

			sum = 0.0f;
			for(k=0; k<n; k++){
				sum += calGetX2Dr(life, Q_old_2_steps, i, j, k);
			}
			tmp = sum/n;
			calSet2Dr(life, Q_old_2_steps, i, j, tmp);
		}
	}
}

void wave_init(struct CALModel2D* wave){
	////add cells to the set of active ones
	//calAddActiveCell2D(wave, 0, 0);
	//calAddActiveCell2D(wave, 0, 1);

	////this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(wave);
}

void wave_steering(struct CALModel2D* wave){
	//int i, j, n;

	//if (wave->A.cells)
	//	for (n=0; n<wave->A.size_current; n++)
	//		calSet2Di(wave, Qzito, wave->A.cells[n].i, wave->A.cells[n].j, (calGet2Di(wave, Qzito, wave->A.cells[n].i, wave->A.cells[n].j) + 1) % 2);
	//else
	//	for (i=0; i<wave->rows; i++)
	//		for (j=0; j<wave->columns; j++)
	//			calSet2Di(wave, Qzito, i, j, (calGet2Di(wave, Qzito, i, j) + 1) % 2);

	////this call is needed only in case CAL_UPDATE_EXPLICIT
	//calUpdateSubstate2Di(wave, Qzito);

	////this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(wave);
}

void wave_finalize(struct CALModel2D* wave){
	//add cells to the set of active ones
	//calRemoveActiveCell2D(wave, 0, 0);
	//calRemoveActiveCell2D(wave, 0, 1);

	////this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(wave);
}

CALbyte waveStopCondition(struct CALModel2D* model){
	if (wave_simulation->step >= calglGetGlobalSettings()->step)
		return CAL_TRUE;
	return CAL_FALSE;
}

void initializeCAandGraphic(){
	struct CALDrawModel2D* drawModel = NULL;

	//setup global parameters
	calglSetCellSizeGlobalSettings(20.0f);
	calglSetNoDataCALreal(-9999.99);
	calglSetRefreshTime(1.0f);
	calglEnableLights();

	//cadef and rundef
	wave = calCADef2D (calglGetGlobalSettings()->rows, calglGetGlobalSettings()->columns, CAL_CUSTOM_NEIGHBORHOOD_2D, CAL_SPACE_FLAT, CAL_OPT_ACTIVE_CELLS); // CAL_NO_OPT
	wave_simulation = calRunDef2D(wave, 1, CAL_RUN_LOOP, CAL_UPDATE_IMPLICIT);

	//add substates
	Q_old = calAddSubstate2Dr(wave);
	Q_old_2_steps = calAddSubstate2Dr(wave);
	Q_new = calAddSubstate2Dr(wave);

	//init substates
	calInitSubstate2Dr(wave, Q_old, 0.0f);
	calInitSubstate2Dr(wave, Q_old_2_steps, 0.0f);
	calInitSubstate2Dr(wave, Q_new, 0.0f);

	//add transition function's elementary processes.
	calAddElementaryProcess2D(wave, wave_transition_function);
	calAddElementaryProcess2D(wave, wave_smooth_function);

	//add neighbors of the Moore neighborhood
	calAddNeighbor2D(wave,   0,   0);	//this is the neighbor 0 (central cell)
	calAddNeighbor2D(wave, - 1,   0);	//this is the neighbor 1
	calAddNeighbor2D(wave,   0, - 1);	//this is the neighbor 2
	calAddNeighbor2D(wave,   0, + 1);	//this is the neighbor 3
	calAddNeighbor2D(wave, + 1,   0);	//this is the neighbor 4
	calAddNeighbor2D(wave, - 1, - 1);	//this is the neighbor 5
	calAddNeighbor2D(wave, + 1, - 1);	//this is the neighbor 6
	calAddNeighbor2D(wave, + 1, + 1);	//this is the neighbor 7
	calAddNeighbor2D(wave, - 1, + 1);	//this is the neighbor 8

	//simulation run
	calRunAddInitFunc2D(wave_simulation, wave_init);
	calRunAddSteeringFunc2D(wave_simulation, wave_steering);
	calRunAddFinalizeFunc2D(wave_simulation, wave_finalize);
	calRunAddStopConditionFunc2D(wave_simulation, waveStopCondition);

	drawModel = calglDefDrawModel2D(CALGL_DRAW_MODE_SURFACE, "Wave", wave, wave_simulation);
	calglAddToDrawModel2Dr(drawModel, NULL, &Q_old, CALGL_TYPE_INFO_VERTEX_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	calglColor2D(drawModel, 96.0f/255, 205.0f/255, 241.0f/255, 1.0f);	//	Water color
	calglAddToDrawModel2Dr(drawModel, Q_old, &Q_old, CALGL_TYPE_INFO_COLOR_DATA, CALGL_TYPE_INFO_USE_CONST_VALUE, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(drawModel, Q_old, &Q_old, CALGL_TYPE_INFO_NORMAL_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
}

void destroyAll(){
	calRunFinalize2D(wave_simulation);
	calFinalize2D(wave);
	calglDestroyGlobalSettings();
	calglCleanDrawModelList2D();
}

CALreal* waveGet2Dr(int i, int j){
	return &calGetMatrixElement(Q_old->current, wave->columns, i, j);
}
