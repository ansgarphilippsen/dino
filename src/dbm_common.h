#ifndef _DBM_COMMON_H
#define _DBM_COMMON_H

#include "transform.h"

#define DBM_NODE_COMMON_HEADER \
   int type;\
   char name[256];\
   transMat transform;

#endif
