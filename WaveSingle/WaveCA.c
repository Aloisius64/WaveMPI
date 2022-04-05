#include "WaveCA.h"
#include <stdlib.h>
#include <time.h>
#include <cal2DBuffer.h>
#include <math.h>

struct CALModel2D* life;
struct CALRun2D* life_simulation;
struct CALSubstate2Dr *Q_old;
struct CALSubstate2Dr *Q_old_2_steps;
CALreal max = 90.0f, min = 1.0f;

int main(int argc, char** argv)	{
	initializeCAandGraphic();
	calglSetDistanceCamera(-162.0f);
	calglStartProcessWindow2D(argc, argv);
	destroyAll();
	return 0;
}

struct CALModel2D* getWaveModel(){
	return life;
}

struct CALSubstate2Dr* getWaveMainSubstate(){
	return Q_old;
}

struct CALRun2D* getWaveRun(){
	return life_simulation;
}

void life_transition_function(struct CALModel2D* life, int i, int j){
	CALreal tmp = 0.0f;

	if(!(i==0 || j==0 || i==calglGetGlobalSettings()->rows-1 || j==calglGetGlobalSettings()->columns-1)){
		tmp = 2 * calGet2Dr(life, Q_old, i, j)
			- calGet2Dr(life, Q_old_2_steps, i, j)
			+ (calGetX2Dr(life, Q_old, i, j, 1)
			+ calGetX2Dr(life, Q_old, i, j, 4)
			+ calGetX2Dr(life, Q_old, i, j, 3)
			+ calGetX2Dr(life, Q_old, i, j, 2)
			- 4 * calGet2Dr(life, Q_old, i, j))/4;

		//tmp = tmp > max ? max : tmp;
		//tmp = tmp < min ? min : tmp;
		calSet2Dr(life, Q_old_2_steps, i, j, calGet2Dr(life, Q_old, i, j));
		calSet2Dr(life, Q_old, i, j, tmp);
	}
}

/*
5 | 1 | 8
---|---|---
2 | 0 | 3
---|---|---
6 | 4 | 7
*/
void life_smooth_function(struct CALModel2D* life, int i, int j){
	int k = 0, n = 5, discreteTmp = 0;
	CALreal tmp = 0.0f, sum = 0.0f;

	//tmp = floor(calGet2Dr(life, Q_old, i, j)+0.5f);
	//calSet2Dr(life, Q_old, i, j, tmp);

	if(!(i==0 || j==0 || i==calglGetGlobalSettings()->rows-1 || j==calglGetGlobalSettings()->columns-1)){
		if(life_simulation->step%10==0){
			for(k=0; k<n; k++){
				sum += calGetX2Dr(life, Q_old, i, j, k);
			}
			tmp = sum/n;
			//tmp = tmp > max ? max : tmp;
			//tmp = tmp < min ? min : tmp;
			calSet2Dr(life, Q_old, i, j, tmp);

			sum = 0.0f;
			for(k=0; k<n; k++){
				sum += calGetX2Dr(life, Q_old_2_steps, i, j, k);
			}
			tmp = sum/n;
			//tmp = tmp > max ? max : tmp;
			//tmp = tmp < min ? min : tmp;
			calSet2Dr(life, Q_old_2_steps, i, j, tmp);
		}
	}
}

void life_init(struct CALModel2D* life){
	//add cells to the set of active ones
	//calAddActiveCell2D(life, 0, 0);
	//calAddActiveCell2D(life, 0, 1);

	//this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(life);
}

void life_steering(struct CALModel2D* life){
	//int i, j, n;

	//if (life->A.cells)
	//	for (n=0; n<life->A.size_current; n++)
	//		calSet2Di(life, Qzito, life->A.cells[n].i, life->A.cells[n].j, (calGet2Di(life, Qzito, life->A.cells[n].i, life->A.cells[n].j) + 1) % 2);
	//else
	//	for (i=0; i<life->rows; i++)
	//		for (j=0; j<life->columns; j++)
	//			calSet2Di(life, Qzito, i, j, (calGet2Di(life, Qzito, i, j) + 1) % 2);

	////this call is needed only in case CAL_UPDATE_EXPLICIT
	//calUpdateSubstate2Di(life, Qzito);

	////this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(life);
}

void life_finalize(struct CALModel2D* life){
	//add cells to the set of active ones
	//calRemoveActiveCell2D(life, 0, 0);
	//calRemoveActiveCell2D(life, 0, 1);

	////this is needed only if one or more cells are added or eliminated from the computationally active cells
	//calUpdateActiveCells2D(life);
}

CALbyte lifeStopCondition(struct CALModel2D* model){
	if (life_simulation->step >= calglGetGlobalSettings()->step)
		return CAL_TRUE;
	return CAL_FALSE;
}

void initializeCAandGraphic(){
	struct CALDrawModel2D* drawModel = NULL;

	calglSetRowsAndColumnsGlobalSettings(500, 500);
	calglSetCellSizeGlobalSettings(20.0f);
	calglSetStepGlobalSettings(300);
	calglSetNoDataCALreal(-9999.99);
	calglSetRefreshTime(1.5f);
	calglEnableLights();

	//cadef and rundef
	life = calCADef2D (calglGetGlobalSettings()->rows, calglGetGlobalSettings()->columns, CAL_CUSTOM_NEIGHBORHOOD_2D, CAL_SPACE_FLAT, CAL_OPT_ACTIVE_CELLS);
	life_simulation = calRunDef2D(life, 1, CAL_RUN_LOOP, CAL_UPDATE_IMPLICIT);
	//add substates
	Q_old = calAddSubstate2Dr(life);
	Q_old_2_steps = calAddSubstate2Dr(life);
	//add transition function's elementary processes.
	calAddElementaryProcess2D(life, life_transition_function);
	calAddElementaryProcess2D(life, life_smooth_function);
	//add neighbors of the Moore neighborhood
	calAddNeighbor2D(life,   0,   0);	//this is the neighbor 0 (central cell)
	calAddNeighbor2D(life, - 1,   0);	//this is the neighbor 1
	calAddNeighbor2D(life,   0, - 1);	//this is the neighbor 2
	calAddNeighbor2D(life,   0, + 1);	//this is the neighbor 3
	calAddNeighbor2D(life, + 1,   0);	//this is the neighbor 4
	calAddNeighbor2D(life, - 1, - 1);	//this is the neighbor 5
	calAddNeighbor2D(life, + 1, - 1);	//this is the neighbor 6
	calAddNeighbor2D(life, + 1, + 1);	//this is the neighbor 7
	calAddNeighbor2D(life, - 1, + 1);	//this is the neighbor 8

	//simulation run
	calRunAddInitFunc2D(life_simulation, life_init);
	calRunInitSimulation2D(life_simulation);
	calRunAddSteeringFunc2D(life_simulation, life_steering);
	calRunAddFinalizeFunc2D(life_simulation, life_finalize);
	calRunAddStopConditionFunc2D(life_simulation, lifeStopCondition);

	//setting data
	calInitSubstate2Dr(life, Q_old, 1.0f);
	calInitSubstate2Dr(life, Q_old_2_steps, 1.0f);
	calSet2Dr(life, Q_old, calglGetGlobalSettings()->rows/2, calglGetGlobalSettings()->columns/2, max/2);
	calUpdate2D(life);

	drawModel = calglDefDrawModel2D(CALGL_DRAW_MODE_SURFACE, "Wave", life, life_simulation);
	calglAddToDrawModel2Dr(drawModel, NULL, &Q_old, CALGL_TYPE_INFO_VERTEX_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	calglColor2D(drawModel, 96.0f/255, 205.0f/255, 241.0f/255, 1.0f);	//	Water color
	calglAddToDrawModel2Dr(drawModel, Q_old, &Q_old, CALGL_TYPE_INFO_COLOR_DATA, CALGL_TYPE_INFO_USE_CONST_VALUE, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Dr(drawModel, Q_old, &Q_old, CALGL_TYPE_INFO_NORMAL_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	//calglInfoBar2Dr(drawModel, Q_old, "Q_old", CALGL_TYPE_INFO_USE_BLUE_SCALE, CALGL_INFO_BAR_VERTICAL);
}

void destroyAll(){
	calRunFinalize2D(life_simulation);
	calFinalize2D(life);
	calglDestroyGlobalSettings();
	calglCleanDrawModelList2D();
}

CALreal* waveGet2Dr(int i, int j){
	return &calGetMatrixElement(Q_old->current, life->columns, i, j);
}
