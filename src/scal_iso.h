#ifndef _SCAL_ISO_H
#define _SCAL_ISO_H

#include "scal_obj.h"
#include "set.h"

struct SCAL_ISO_CUBE {
  int pcode;
  int coord_index[28];
};


int scalIso(scalObj *obj, Select *sel);

#endif
