#ifndef LifeCA_h
#define LifeCA_h

#include <cal2D.h>
#include <cal2DIO.h>
#include <cal2DRun.h>
#include <calgl2DWindow.h>
#include <calgl2D.h>

struct CALModel2D* getWaveModel();
struct CALSubstate2Di* getWaveMainSubstate();
struct CALRun2D* getWaveRun();
void life_transition_function(struct CALModel2D* life, int i, int j);
void life_elementary_process_1(struct CALModel2D* life, int i, int j);
void life_elementary_process_2(struct CALModel2D* life, int i, int j);
void life_init(struct CALModel2D* life);
void life_steering(struct CALModel2D* life);
void life_finalize(struct CALModel2D* life);
CALbyte lifeStopCondition(struct CALModel2D* model);
void initializeCAandGraphic();
void destroyAll();

int* waveGet2Di(int i, int j);

#endif
