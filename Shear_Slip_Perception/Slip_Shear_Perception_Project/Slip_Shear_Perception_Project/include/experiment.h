
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

void initializeCursorState(void);


#endif  // EXPERIMENT_H
