#ifndef _JOY_H
#define _JOY_H

#ifdef LINUX
#include <linux/joystick.h>
#endif

int jInit(void);

int jCheck(void);

#endif

