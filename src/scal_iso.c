/*
  isocontouring algorithm

  approx the sixth attempt :)
*/

#include <stdio.h>
#include <math.h>

#include "mat.h"
#include "Cmalloc.h"

#include "scal_db.h"
#include "scal_obj.h"
#include "scal_iso.h"



/*
  unit cube definition:

  v  w
  | /
  |/
  +---u

  corners:

   7 __________ 6
    /.        /|
 3 / .     2 / |
  +---------+  |
  |  .      |  |
  |  .      |  |
  |  .......|..|
  | .4      | / 5
  |.        |/
  +---------+
 0           1

 edges:

 01: 0
 12: 1
 23: 2
 30: 3
 04: 4
 15: 5
 26: 6
 37: 7
 45: 8
 56: 9
 67: 10
 47: 11

*/

#if 0
static struct tri_vertex {
  float p[3],n[3]; // position and normal
  int fi[3],fc; // face-index and count
};

static struct tri_face {
  int i[3]; // vertex-index
};

int scalIso(scalObj *obj, Select *sel)
{
  tri_vertex *vert;
  tri_face *face;

  int *i_layer; // index layer
  /*
    generate first layer of cubes
    actually only need 3*us*vs-us-vs
  */
  i_layer = Cmalloc(3*obj->u_size*obj->v_size*sizeof(int));
}
#else
int scalIso(scalObj *obj, Select *sel)
{
  return 0;
}
#endif
