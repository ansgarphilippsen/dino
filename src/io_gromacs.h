#ifndef _IO_GROMACS_H
#define _IO_GROMACS_H

#include <gromacs/xtcio.h>

#include "struct_db.h"

int xtcTrjRead(char *filename, dbmStructNode *n, int swap_flag);

#endif
