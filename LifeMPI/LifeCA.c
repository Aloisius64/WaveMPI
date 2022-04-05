#include "LifeCA.h"
#include <stdlib.h>
#include <time.h>
#include <cal2DBuffer.h>

struct CALModel2D* life;
struct CALRun2D* life_simulation;
struct CALSubstate2Di *Q;
struct CALSubstate2Di *Qzito;
CALParameterr pr = 0;

struct CALModel2D* getWaveModel(){
	return life;
}

struct CALSubstate2Di* getWaveMainSubstate(){
	return Q;
}

struct CALRun2D* getWaveRun(){
	return life_simulation;
}

void life_transition_function(struct CALModel2D* life, int i, int j){
	int sum = 0, n;
	for (n=1; n<life->sizeof_X; n++)
		sum += calGetX2Di(life, Q, i, j, n);
	if ((sum == 3) || (sum == 2 && calGet2Di(life, Q, i, j) == 1))
		calSet2Di(life, Q, i, j, 1);
	else
		calSet2Di(life, Q, i, j, 0);
}

void life_elementary_process_1(struct CALModel2D* life, int i, int j)
{
	calSet2Di(life, Qzito, i, j, rand() % 2);
}

void life_elementary_process_2(struct CALModel2D* life, int i, int j)
{
	calSet2Di(life, Qzito, i, j, rand() % 2);
}

void life_init(struct CALModel2D* life){
	//add cells to the set of active ones
	calAddActiveCell2D(life, 0, 0);
	calAddActiveCell2D(life, 0, 1);

	//this is needed only if one or more cells are added or eliminated from the computationally active cells
	calUpdateActiveCells2D(life);
}

void life_steering(struct CALModel2D* life){
	int i, j, n;

	if (life->A.cells)
		for (n=0; n<life->A.size_current; n++)
			calSet2Di(life, Qzito, life->A.cells[n].i, life->A.cells[n].j, (calGet2Di(life, Qzito, life->A.cells[n].i, life->A.cells[n].j) + 1) % 2);
	else
		for (i=0; i<life->rows; i++)
			for (j=0; j<life->columns; j++)
				calSet2Di(life, Qzito, i, j, (calGet2Di(life, Qzito, i, j) + 1) % 2);

	//this call is needed only in case CAL_UPDATE_EXPLICIT
	calUpdateSubstate2Di(life, Qzito);

	//this is needed only if one or more cells are added or eliminated from the computationally active cells
	calUpdateActiveCells2D(life);
}

void life_finalize(struct CALModel2D* life){
	//add cells to the set of active ones
	calRemoveActiveCell2D(life, 0, 0);
	calRemoveActiveCell2D(life, 0, 1);

	//this is needed only if one or more cells are added or eliminated from the computationally active cells
	calUpdateActiveCells2D(life);
}

CALbyte lifeStopCondition(struct CALModel2D* model){
	if (life_simulation->step >= calglGetGlobalSettings()->step)
		return CAL_TRUE;
	return CAL_FALSE;
}

void initializeCAandGraphic(){
	struct CALDrawModel2D* drawModel = NULL;

	calglSetCellSizeGlobalSettings(10);
	//calglEnableLights();

	//cadef and rundef
	life = calCADef2D (calglGetGlobalSettings()->rows, calglGetGlobalSettings()->columns, CAL_CUSTOM_NEIGHBORHOOD_2D, CAL_SPACE_FLAT, CAL_OPT_ACTIVE_CELLS); // CAL_NO_OPT
	life_simulation = calRunDef2D(life, 1, CAL_RUN_LOOP, CAL_UPDATE_IMPLICIT);  // CAL_UPDATE_EXPLICIT

	//initialize the random number function
	srand(0);

	//add transition function's elementary processes.
	calAddElementaryProcess2D(life, life_transition_function);
	calAddElementaryProcess2D(life, life_elementary_process_1);
	calAddElementaryProcess2D(life, life_elementary_process_2);

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

	//add substates
	Q = calAddSubstate2Di(life);
	Qzito = calAddSubstate2Di(life);

	//set the whole substate to 0
	calInitSubstate2Di(life, Q, 0);
	calInitSubstate2Di(life, Qzito, 0);
#pragma region AddActiveCellsToCA
	//set a glider
	calInit2Di(life, Q, 0, 2, 1);
	calInit2Di(life, Q, 1, 0, 1);
	calInit2Di(life, Q, 1, 2, 1);
	calInit2Di(life, Q, 2, 1, 1);
	calInit2Di(life, Q, 2, 2, 1);
	//set another glider
	calInit2Di(life, Q, 49, 50, 1);
	calInit2Di(life, Q, 50, 51, 1);
	calInit2Di(life, Q, 51, 49, 1);
	calInit2Di(life, Q, 51, 50, 1);
	calInit2Di(life, Q, 51, 51, 1);
	//set another glider
	calInit2Di(life, Q, 39, 40, 1);
	calInit2Di(life, Q, 40, 41, 1);
	calInit2Di(life, Q, 41, 39, 1);
	calInit2Di(life, Q, 41, 40, 1);
	calInit2Di(life, Q, 41, 41, 1);
	//set another glider
	calInit2Di(life, Q, 29, 40, 1);
	calInit2Di(life, Q, 30, 41, 1);
	calInit2Di(life, Q, 31, 39, 1);
	calInit2Di(life, Q, 31, 40, 1);
	calInit2Di(life, Q, 31, 41, 1);
	//set another glider
	calInit2Di(life, Q, 29, 60, 1);
	calInit2Di(life, Q, 30, 61, 1);
	calInit2Di(life, Q, 31, 59, 1);
	calInit2Di(life, Q, 31, 60, 1);
	calInit2Di(life, Q, 31, 61, 1);
	//set another glider
	calInit2Di(life, Q, 97, 98, 1);
	calInit2Di(life, Q, 98, 99, 1);
	calInit2Di(life, Q, 99, 97, 1);
	calInit2Di(life, Q, 99, 98, 1);
	calInit2Di(life, Q, 99, 99, 1);
	//set another glider X glider
	calInit2Di(life, Q, 20, 30, 1);
	calInit2Di(life, Q, 21, 29, 1);
	calInit2Di(life, Q, 21, 30, 1);
	calInit2Di(life, Q, 21, 31, 1);
	calInit2Di(life, Q, 22, 30, 1);
	//set another glider Long X glider
	calInit2Di(life, Q, 60, 15, 1);
	calInit2Di(life, Q, 61, 15, 1);
	calInit2Di(life, Q, 62, 15, 1);
	calInit2Di(life, Q, 64, 11, 1);
	calInit2Di(life, Q, 64, 12, 1);
	calInit2Di(life, Q, 64, 13, 1);
	calInit2Di(life, Q, 64, 17, 1);
	calInit2Di(life, Q, 64, 18, 1);
	calInit2Di(life, Q, 64, 19, 1);
	calInit2Di(life, Q, 66, 15, 1);
	calInit2Di(life, Q, 67, 15, 1);
	calInit2Di(life, Q, 68, 15, 1);
	//set another glider
	calInit2Di(life, Q, 10, 80, 1);
	calInit2Di(life, Q, 11, 79, 1);
	calInit2Di(life, Q, 11, 80, 1);
	calInit2Di(life, Q, 11, 81, 1);
	calInit2Di(life, Q, 12, 78, 1);
	calInit2Di(life, Q, 12, 80, 1);
	calInit2Di(life, Q, 12, 82, 1);
	calInit2Di(life, Q, 13, 77, 1);
	calInit2Di(life, Q, 13, 78, 1);
	calInit2Di(life, Q, 13, 79, 1);
	calInit2Di(life, Q, 13, 81, 1);
	calInit2Di(life, Q, 13, 82, 1);
	calInit2Di(life, Q, 13, 83, 1);
	calInit2Di(life, Q, 14, 78, 1);
	calInit2Di(life, Q, 14, 80, 1);
	calInit2Di(life, Q, 14, 82, 1);
	calInit2Di(life, Q, 15, 79, 1);
	calInit2Di(life, Q, 15, 80, 1);
	calInit2Di(life, Q, 15, 81, 1);
	calInit2Di(life, Q, 16, 80, 1);
#pragma endregion

	//simulation run
	calRunAddInitFunc2D(life_simulation, life_init);
	calRunAddSteeringFunc2D(life_simulation, life_steering);
	calRunAddFinalizeFunc2D(life_simulation, life_finalize);
	calRunAddStopConditionFunc2D(life_simulation, lifeStopCondition);

	drawModel = calglDefDrawModel2DNoRun(CALGL_DRAW_MODE_FLAT, "Wave", life);
	calglAddToDrawModel2Di(drawModel, NULL, &Q, CALGL_TYPE_INFO_VERTEX_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
	calglColor2D(drawModel, 0.5f, 0.5f, 0.5f, 1.0f);
	calglAddToDrawModel2Di(drawModel, Q, &Q, CALGL_TYPE_INFO_COLOR_DATA, CALGL_TYPE_INFO_USE_CONST_VALUE, CALGL_DATA_TYPE_DYNAMIC);
	calglAddToDrawModel2Di(drawModel, Q, &Q, CALGL_TYPE_INFO_NORMAL_DATA, CALGL_TYPE_INFO_USE_DEFAULT, CALGL_DATA_TYPE_DYNAMIC);
}

void destroyAll(){
	calRunFinalize2D(life_simulation);
	//finalization
	calFinalize2D(life);
	calglDestroyGlobalSettings();
	calglCleanDrawModelList2D();
}

int* waveGet2Di(int i, int j) {
	return &calGetMatrixElement(Q->current, life->columns, i, j);
}
