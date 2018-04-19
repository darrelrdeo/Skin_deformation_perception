
#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <cstdlib>
#include <conio.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "data.h"
#include "shared_data.h"

void linkSharedDataToExperiment(shared_data& sharedData);
void initExperiment(void);
void initDemo(void);

void updateExperiment(void);
void closeExperiment(void);

void setOutputForceToZero(void);

void setup_Perception_Force_Profiles(void);
void setZHomingProfile(void);
void randomizeTargets(void);
void randomize(int arr[], int arrInd[], int n);
void swap(int *a, int *b);

#endif  // EXPERIMENT_H
