#ifndef GEOM_DB_H
#define GEOM_DB_H

#include "geom_obj.h"
#include "transform.h"

typedef struct DBM_GEOM_NODE {
  int type;
  char name[256];
  char path[256];
  struct GEOM_OBJ **obj;
  int obj_count;
  int obj_max;
  transMat transform;
}dbmGeomNode;

typedef struct GEOM_NAME_LIST {
  int t,n1,n2;
}geomNameList;

enum            {GEOM_TYPE_POINT,
		 GEOM_TYPE_LINE,
		 GEOM_TYPE_TRI,
		 GEOM_TYPE_RECT,
		 GEOM_TYPE_LABEL,
		 GEOM_TYPE_ALL};

enum {GEOM_PRIM,GEOM_SLAB};

int geomCommand(struct DBM_GEOM_NODE *,int wc, char **wl);
int geomNew(struct DBM_GEOM_NODE *, int wc, char **wl);
int geomGenNameList(geomNameList *list, char *expr);

int geomDraw(dbmGeomNode *node, int f);
int geomDrawObj(geomObj *obj);
int geomDrawCyl(geomObj *obj, geomLine *cyl);

#endif
