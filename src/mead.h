#include <stdio.h>

#ifndef _MEAD_H
#define _MEAD_H

#include "dbm.h"

int meadRead(FILE *f, dbmScalNode *node);
int meadSplit(char *line, char *prop, char *value);
int meadSplitValue(char *val, float *v);

#endif
