#ifndef WaveCA_h
#define WaveCA_h

#include <cal2D.h>
#include <cal2DIO.h>
#include <cal2DRun.h>
#include <cal2DBuffer.h>
#include <calgl2DWindow.h>
#include <calgl2D.h>

struct CALModel2D* getWaveModel();
struct CALSubstate2Dr* getWaveMainSubstate();
struct CALRun2D* getWaveRun();
void wave_transition_function(struct CALModel2D* wave, int i, int j);
void wave_smooth_function(struct CALModel2D* life, int i, int j);
void wave_init(struct CALModel2D* wave);
void wave_steering(struct CALModel2D* wave);
void wave_finalize(struct CALModel2D* wave);
CALbyte waveStopCondition(struct CALModel2D* model);
void initializeCAandGraphic();
void destroyAll();

CALreal* waveGet2Dr(int i, int j);
#endif
