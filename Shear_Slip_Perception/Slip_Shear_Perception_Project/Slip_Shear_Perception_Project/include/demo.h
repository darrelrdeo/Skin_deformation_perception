
#ifndef DEMO_H
#define DEMO_H

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

void linkSharedDataToDemo(shared_data& sharedData);
void initDemo(void);

void updateDemo(void);
void closeDemo(void);




#endif  // DEMO_H
