#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
  generate isocontouring lookup tables

  cube:
 

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


*/


/*
  lookup table that converts a 4bit tetraheder corner code
  into 3 or 4 corner-pairs that form an intersected edge,
  with the correct facet orientation

  the first of the pair is the one with the larger level

     0
    /|\
   / | \
 1 --|-- 3
   \ | /
    \|/
     2

  vertical line (0-2) is in front of horizontal one (1-3)

*/ 
                                /* bits */
static int tet_code[][8] = {    /* 3210 */
  {-1,-1, -1,-1, -1,-1, -1,-1}, /* 0000 0 */
  {0,1, 0,3, 0,2, -1,-1},       /* 0001 1 */
  {1,0, 1,2, 1,3, -1,-1},       /* 0010 2 */
  {0,2, 1,2, 1,3, 0,3},         /* 0011 3 */
  {2,0, 2,3, 2,1, -1,-1},       /* 0100 4 */
  {0,1, 0,3, 2,3, 2,1},         /* 0101 5 */
  {1,0, 2,0, 2,3, 1,3},         /* 0110 6 */
  {0,3, 2,3, 1,3, -1,-1},       /* 0111 7 */
  {3,0, 3,1, 3,2, -1,-1},       /* 1000 8 */
  {0,1, 3,1, 3,2, 0,2},         /* 1001 9 */
  {1,0, 1,2, 3,2, 3,0},         /* 1010 10 */
  {0,2, 1,2, 3,2, -1,-1},       /* 1011 11 */
  {2,0, 3,0, 3,1, 2,1},         /* 1100 12 */
  {0,1, 3,1, 2,1, -1,-1},       /* 1101 13 */
  {1,0, 2,0, 3,0, -1,-1},       /* 1110 14 */
  {-1,-1, -1,-1, -1,-1, -1,-1}  /* 1111 15 */
};


/*
  lookup table that splits a cube into n
  tetraheders, specified by cube corners, 
  with the order according to the tetr diagram
  above

  not working!!!

*/

/*
static int tetrahed[][5] = {
  {0,4,1,3},
  {2,3,1,6},
  {5,6,1,4},
  {7,4,3,6},
  {1,3,4,6}
};
static int tetrahed_num=5;
*/

/*
  alternative tetr subdivision of cube, 
  taken from Paul Bourke
 */

static int tetrahed[][6] = {
  {0,2,3,7}, {0,6,2,7}, {0,4,6,7}, {0,6,1,2}, {0,1,6,4}, {5,6,1,4}
};

static int tetrahed_num=6;



/*
  alternative tetr subdivision taken from
  www.informatik.uni-leipzig.de/ cgip/lehre/ss02/vorl10.pdf
  uses even and odd parity (?)
  parity = ((x^y)^z) &0x1
*/
/*
static int tetrahed_even[][5] = {
  {0,1,3,4}, {6,1,4,3},  {1,6,2,3}, {1,6,4,5}, {3,7,4,6}
};

static int tetrahed_odd[][5] = {
  {4,5,6,0}, {2,5,0,7}, {5,2,6,7}, {5,2,0,1}, {7,3,0,2}
};

static int tetrahed_num=5;
*/



static int max_tri_entry = 12;

static void gen_tab(int code, int tflag);
static void print_tentry(char *buf, int a1,int a2,int b1,int b2,int c1,int c2);
static void print_lentry(char *buf, int a1, int a2, int b1, int b2);

int main(int argc, char **argv)
{
  int c;
  fprintf(stdout,"static struct _MC_EDGE_LOOKUP_TABLE {\n  int count;\n  int tab[%d][6];\n} mc_edge_lookup_table[] = {\n",max_tri_entry);
  for(c=0;c<256;c++) {
    gen_tab(c, 1);
  }
  fprintf(stdout,"};\n");
  
}

/*
  generate the lookup table based on the
  passed in binary code
*/

static void gen_tab(int code, int tflag)
{
  int tetc, corc;
  int hit_code;
  int efill,ecount;
  char buf[1024];


  // go through each tetraheder

  strcpy(buf,"");
  ecount=0;
  for(tetc=0;tetc<tetrahed_num;tetc++) {

    hit_code=0;
    // generate the hitcode
    for(corc=0;corc<4;corc++) {
      if( code & (1 << (tetrahed[tetc][corc]))) {
	hit_code += (1 << corc);
      }
    }

    /*
      the hitcode now contains which corners of the
      tetraheder are affected
    */


    /*
      write entries, converting the tetr corners
      back into orig cube corners

      tet_code[hit_code]* contains the tetr corners
      that form an intersected edge
      by pluging this number into tetrahed[tetc], the
      corresponding corner in the cube is obtained
    */
    if(tet_code[hit_code][0]>=0) {
      // valid entry
      if(tflag) {
	print_tentry(buf,
		     tetrahed[tetc][tet_code[hit_code][0]],
		     tetrahed[tetc][tet_code[hit_code][1]],
		     tetrahed[tetc][tet_code[hit_code][2]],
		     tetrahed[tetc][tet_code[hit_code][3]],
		     tetrahed[tetc][tet_code[hit_code][4]],
		     tetrahed[tetc][tet_code[hit_code][5]]);
	ecount++;
      } else {
	print_lentry(buf,
		     tetrahed[tetc][tet_code[hit_code][0]],
		     tetrahed[tetc][tet_code[hit_code][1]],
		     tetrahed[tetc][tet_code[hit_code][2]],
		     tetrahed[tetc][tet_code[hit_code][3]]);
	print_lentry(buf,
		     tetrahed[tetc][tet_code[hit_code][0]],
		     tetrahed[tetc][tet_code[hit_code][1]],
		     tetrahed[tetc][tet_code[hit_code][4]],
		     tetrahed[tetc][tet_code[hit_code][5]]);
	print_lentry(buf,
		     tetrahed[tetc][tet_code[hit_code][2]],
		     tetrahed[tetc][tet_code[hit_code][3]],
		     tetrahed[tetc][tet_code[hit_code][4]],
		     tetrahed[tetc][tet_code[hit_code][5]]);
      }
      if(tet_code[hit_code][6]>=0) {
	if(tflag) {
	  print_tentry(buf,
		       tetrahed[tetc][tet_code[hit_code][0]],
		       tetrahed[tetc][tet_code[hit_code][1]],
		       tetrahed[tetc][tet_code[hit_code][4]],
		       tetrahed[tetc][tet_code[hit_code][5]],
		       tetrahed[tetc][tet_code[hit_code][6]],
		       tetrahed[tetc][tet_code[hit_code][7]]);
	  ecount++;
	} else {
	  print_lentry(buf,
		       tetrahed[tetc][tet_code[hit_code][0]],
		       tetrahed[tetc][tet_code[hit_code][1]],
		       tetrahed[tetc][tet_code[hit_code][6]],
		       tetrahed[tetc][tet_code[hit_code][7]]);
	  print_lentry(buf,
		       tetrahed[tetc][tet_code[hit_code][4]],
		       tetrahed[tetc][tet_code[hit_code][5]],
		       tetrahed[tetc][tet_code[hit_code][6]],
		       tetrahed[tetc][tet_code[hit_code][7]]);
	}
      }
    }
  }

  for(efill=ecount;efill<max_tri_entry;efill++) {
    strcat(buf,"    {-1,-1,-1,-1,-1,-1},\n");
  }
  
  buf[strlen(buf)-2]='\0';
  
  fprintf(stdout,"  /*0x%02x*/\n  {%d,{\n%s\n  }},\n",code,ecount,buf);
  
}

static void print_tentry(char *buf, int a1,int a2,int b1,int b2,int c1,int c2)
{
  char tmp[256]; 
  sprintf(tmp,"    {%d,%d, %d,%d, %d,%d},\n",a1,a2,b1,b2,c1,c2);
  strcat(buf,tmp);
}



// NOT TESTED

static int cube_face[][4] = {
  {0,1,2,3}, {0,3,7,4}, {1,2,6,5}, {0,1,5,4}, {2,3,7,6}, {4,5,6,7}
};


static void print_lentry(char *buf, int a1, int a2, int b1, int b2)
{
  int i,k,f1,f2;
  char tmp[256];
  // check validity of line first (along outer faces of cube)
  for(i=0;i<6;i++) {
    f1=0,f2=0;
    for(k=0;k<4;k++) {
      if(cube_face[i][k]==a1) f1++;
      if(cube_face[i][k]==a2) f1++;
      if(cube_face[i][k]==b1) f2++;
      if(cube_face[i][k]==b2) f2++;
    }
    if(f1==2 && f2==2) {
      sprintf(tmp,"{%d,%d, %d,%d}",a1,a2,b1,b2);
      strcat(buf,tmp);
      break;
    }

  }
}
