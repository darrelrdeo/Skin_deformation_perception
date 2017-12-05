
#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "shared_data.h"


void linkSharedData(shared_data& sharedData);
void setup(void);
void saveOneTimeStep(void);
void recordTrial(void);

#endif  // DATA_H
