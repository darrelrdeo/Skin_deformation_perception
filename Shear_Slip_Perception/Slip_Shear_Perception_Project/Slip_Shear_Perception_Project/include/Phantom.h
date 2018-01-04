
#ifndef PHANTOM_H
#define PHANTOM_H

#include "../../../external/chai3d-3.0.0/src/chai3d.h"
#include "shared_data.h"

void initPhantom(void);
void linkSharedDataToPhantom(shared_data& sharedData);
void updatePhantom(void);
void closePhantom(void);
void updateCursor(void);
cVector3d Rotate_Tool_to_Base_Frame(cVector3d tool_force_desired, cMatrix3d tool_orientation);


#endif
