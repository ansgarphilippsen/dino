#ifndef CMALLOC_H
#define CMALLOC_H

#include <stdlib.h>

#ifndef OSX
#include <malloc.h>
#endif

void *Cmalloc(size_t size);
void *Ccalloc(size_t nelem, size_t elsize);
void *Crealloc(void *p, size_t elsize);
void *Crecalloc(void *p, size_t nelem, size_t elsize);
void Cfree(void *p);

#endif

