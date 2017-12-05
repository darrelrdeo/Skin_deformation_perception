
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include <string>
#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "../../../external/chai3d-3.0.0/extras/freeglut/include/GL/freeglut.h"

#include "Phantom.h"
#include "shared_Data.h"

void initGraphics(int argc, char* argv[]);
void linkSharedDataToGraphics(shared_data& sharedData);
void updateGraphics(void);
void graphicsTimer(int data);
void resizeWindow(int w, int h);
void respToKey(unsigned char key, int x, int y);
void close(void);

#endif  // GRAPHICS_H
