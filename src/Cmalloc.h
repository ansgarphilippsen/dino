#ifdef STDMALLOC

#include <malloc.h>

#else

#include <stdlib.h>

#endif

#ifndef CMALLOC_H
#define CMALLOC_H

void *Cmalloc(size_t size);
void *Ccalloc(size_t nelem, size_t elsize);
void *Crecalloc(void *p, size_t nelem, size_t elsize);
void Cfree(void *p);

#endif

