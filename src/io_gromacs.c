#include "io_gromacs.h"

#include <math.h>
#include <rpc/rpc.h>
#include <rpc/xdr.h>

#include "com.h"
#include "Cmalloc.h"
#include "struct_db.h"

/*
  first a bunch of static routines from the GROMACS
  src code, extracted from xtcio and libxdrf

  these routines return 1 on success, and 0 on error!
*/

typedef float real;
typedef float rvec[3];
typedef float matrix[3][3];
typedef int bool;

#define MAXID 20
static FILE *xdrfiles[MAXID];
static XDR *xdridptr[MAXID];
static char xdrmodes[MAXID];
static unsigned int cnt;

#define MAXABS INT_MAX-2

#ifndef MIN
#define MIN(x,y) ((x) < (y) ? (x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x) > (y) ? (x):(y))
#endif
#ifndef SQR
#define SQR(x) ((x)*(x))
#endif
static int magicints[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0,
    8, 10, 12, 16, 20, 25, 32, 40, 50, 64,
    80, 101, 128, 161, 203, 256, 322, 406, 512, 645,
    812, 1024, 1290, 1625, 2048, 2580, 3250, 4096, 5060, 6501,
    8192, 10321, 13003, 16384, 20642, 26007, 32768, 41285, 52015, 65536,
    82570, 104031, 131072, 165140, 208063, 262144, 330280, 416127, 524287, 660561,
    832255, 1048576, 1321122, 1664510, 2097152, 2642245, 3329021, 4194304, 5284491, 6658042,
    8388607, 10568983, 13316085, 16777216 };

#define FIRSTIDX 9
/* note that magicints[FIRSTIDX-1] == 0 */
#define LASTIDX (sizeof(magicints) / sizeof(*magicints))

static int sizeofint(const int size) 
{
    unsigned int num = 1;
    int num_of_bits = 0;
    
    while (size >= num && num_of_bits < 32) {
	num_of_bits++;
	num <<= 1;
    }
    return num_of_bits;
}

static int sizeofints( const int num_of_ints, unsigned int sizes[]) 
{
    int i, num;
    unsigned int num_of_bytes, num_of_bits, bytes[32], bytecnt, tmp;
    num_of_bytes = 1;
    bytes[0] = 1;
    num_of_bits = 0;
    for (i=0; i < num_of_ints; i++) {	
	tmp = 0;
	for (bytecnt = 0; bytecnt < num_of_bytes; bytecnt++) {
	    tmp = bytes[bytecnt] * sizes[i] + tmp;
	    bytes[bytecnt] = tmp & 0xff;
	    tmp >>= 8;
	}
	while (tmp != 0) {
	    bytes[bytecnt++] = tmp & 0xff;
	    tmp >>= 8;
	}
	num_of_bytes = bytecnt;
    }
    num = 1;
    num_of_bytes--;
    while (bytes[num_of_bytes] >= num) {
	num_of_bits++;
	num *= 2;
    }
    return num_of_bits + num_of_bytes * 8;
}
    
static int sendbits(int buf[], int num_of_bits, int num) 
{
  unsigned int cnt, lastbyte;
  int lastbits;
  unsigned char * cbuf;
  
  cbuf = ((unsigned char *)buf) + 3 * sizeof(*buf);
  cnt = (unsigned int) buf[0];
  lastbits = buf[1];
  lastbyte =(unsigned int) buf[2];
  while (num_of_bits >= 8) {
    lastbyte = (lastbyte << 8) | ((num >> (num_of_bits -8)) /* & 0xff*/);
    cbuf[cnt++] = lastbyte >> lastbits;
    num_of_bits -= 8;
  }
  if (num_of_bits > 0) {
    lastbyte = (lastbyte << num_of_bits) | num;
    lastbits += num_of_bits;
    if (lastbits >= 8) {
      lastbits -= 8;
      cbuf[cnt++] = lastbyte >> lastbits;
    }
  }
  buf[0] = cnt;
  buf[1] = lastbits;
  buf[2] = lastbyte;
  if (lastbits>0) {
    cbuf[cnt] = lastbyte << (8 - lastbits);
  }
  return 1;
}

static int sendints(int buf[], const int num_of_ints, const int num_of_bits,
	unsigned int sizes[], unsigned int nums[]) 
{
  int i;
  unsigned int bytes[32], num_of_bytes, bytecnt, tmp;
  
  tmp = nums[0];
  num_of_bytes = 0;
  do {
    bytes[num_of_bytes++] = tmp & 0xff;
    tmp >>= 8;
  } while (tmp != 0);
  
  for (i = 1; i < num_of_ints; i++) {
    if (nums[i] >= sizes[i]) {
      fprintf(stderr,"major breakdown in sendints num %d doesn't "
	      "match size %d\n", nums[i], sizes[i]);
      return 0;
    }
    /* use one step multiply */    
    tmp = nums[i];
    for (bytecnt = 0; bytecnt < num_of_bytes; bytecnt++) {
      tmp = bytes[bytecnt] * sizes[i] + tmp;
      bytes[bytecnt] = tmp & 0xff;
      tmp >>= 8;
    }
    while (tmp != 0) {
      bytes[bytecnt++] = tmp & 0xff;
      tmp >>= 8;
    }
    num_of_bytes = bytecnt;
  }
  if (num_of_bits >= num_of_bytes * 8) {
    for (i = 0; i < num_of_bytes; i++) {
      sendbits(buf, 8, bytes[i]);
    }
    sendbits(buf, num_of_bits - num_of_bytes * 8, 0);
  } else {
    for (i = 0; i < num_of_bytes-1; i++) {
      sendbits(buf, 8, bytes[i]);
    }
    sendbits(buf, num_of_bits- (num_of_bytes -1) * 8, bytes[i]);
  }
  return 1;
}

static int receivebits(int buf[], int num_of_bits) 
{
  int cnt, num; 
  unsigned int lastbits, lastbyte;
  unsigned char * cbuf;
  int mask = (1 << num_of_bits) -1;
  
  cbuf = ((unsigned char *)buf) + 3 * sizeof(*buf);
  cnt = buf[0];
  lastbits = (unsigned int) buf[1];
  lastbyte = (unsigned int) buf[2];
  
  num = 0;
  while (num_of_bits >= 8) {
    lastbyte = ( lastbyte << 8 ) | cbuf[cnt++];
    num |=  (lastbyte >> lastbits) << (num_of_bits - 8);
    num_of_bits -=8;
  }
  if (num_of_bits > 0) {
    if (lastbits < num_of_bits) {
      lastbits += 8;
      lastbyte = (lastbyte << 8) | cbuf[cnt++];
    }
    lastbits -= num_of_bits;
    num |= (lastbyte >> lastbits) & ((1 << num_of_bits) -1);
  }
  num &= mask;
  buf[0] = cnt;
  buf[1] = lastbits;
  buf[2] = lastbyte;
  return num; 
}

static int receiveints(int buf[], const int num_of_ints, int num_of_bits,
	unsigned int sizes[], int nums[]) 
{
  int bytes[32];
  int i, j, num_of_bytes, p, num;
  
  bytes[1] = bytes[2] = bytes[3] = 0;
  num_of_bytes = 0;
  while (num_of_bits > 8) {
    bytes[num_of_bytes++] = receivebits(buf, 8);
    num_of_bits -= 8;
  }
  if (num_of_bits > 0) {
    bytes[num_of_bytes++] = receivebits(buf, num_of_bits);
  }
  for (i = num_of_ints-1; i > 0; i--) {
    num = 0;
    for (j = num_of_bytes-1; j >=0; j--) {
      num = (num << 8) | bytes[j];
      p = num / sizes[i];
      bytes[j] = p;
      num = num - p * sizes[i];
    }
    nums[i] = num;
  }
  nums[0] = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
  return 1;
}

static int xdropen(XDR *xdrs, const char *filename, const char *type) 
{
  static int init_done = 0;
  enum xdr_op lmode;
  int xdrid;
  
  if (init_done == 0) {
    for (xdrid = 1; xdrid < MAXID; xdrid++) {
      xdridptr[xdrid] = NULL;
    }
    init_done = 1;
  }
  xdrid = 1;
  while (xdrid < MAXID && xdridptr[xdrid] != NULL) {
    xdrid++;
  }
  if (xdrid == MAXID) {
    return 0;
  }
  if (*type == 'w' || *type == 'W') {
    type = "w+";
    lmode = XDR_ENCODE;
  } else {
    type = "r";
    lmode = XDR_DECODE;
  }
  xdrfiles[xdrid] = fopen(filename, type);
  if (xdrfiles[xdrid] == NULL) {
    xdrs = NULL;
    return 0;
  }
  xdrmodes[xdrid] = *type;
  /* next test isn't usefull in the case of C language
   * but is used for the Fortran interface
   * (C users are expected to pass the address of an already allocated
   * XDR staructure)
   */
  if (xdrs == NULL) {
    xdridptr[xdrid] = (XDR *) malloc(sizeof(XDR));
    xdrstdio_create(xdridptr[xdrid], xdrfiles[xdrid], lmode);
  } else {
    xdridptr[xdrid] = xdrs;
    xdrstdio_create(xdrs, xdrfiles[xdrid], lmode);
  }
  return xdrid;
}

static int xdrclose(XDR *xdrs) 
{
  int xdrid;
  
  if (xdrs == NULL) {
    fprintf(stderr, "xdrclose: passed a NULL pointer\n");
    return 0;
  }
  for (xdrid = 1; xdrid < MAXID; xdrid++) {
    if (xdridptr[xdrid] == xdrs) {
      
      xdr_destroy(xdrs);
      fclose(xdrfiles[xdrid]);
      xdridptr[xdrid] = NULL;
      return 1;
    }
  } 
  fprintf(stderr, "xdrclose: no such open xdr file\n");
  return 0;
}

static int xdr_real(XDR *xdrs,float *r)
{
  return xdr_float(xdrs,(float *)r);
}


int xdr3dfcoord(XDR *xdrs, float *fp, int *size, float *precision) 
{
  static int *ip = NULL;
  static int oldsize;
  static int *buf;
  
  int minint[3], maxint[3], mindiff, *lip, diff;
  int lint1, lint2, lint3, oldlint1, oldlint2, oldlint3, smallidx;
  int minidx, maxidx;
  unsigned sizeint[3], sizesmall[3], bitsizeint[3], size3, *luip;
  int flag, k;
  int small, smaller, larger, i, is_small, is_smaller, run, prevrun;
  float *lfp, lf;
  int tmp, *thiscoord,  prevcoord[3];
  unsigned int tmpcoord[30];
  
  int bufsize, xdrid, lsize;
  unsigned int bitsize;
  float inv_precision;
  int errval = 1;
  
  /* find out if xdrs is opened for reading or for writing */
  xdrid = 0;
  while (xdridptr[xdrid] != xdrs) {
    xdrid++;
    if (xdrid >= MAXID) {
      fprintf(stderr, "xdr error. no open xdr stream\n");
      return -1;
    }
  }
  if (xdrmodes[xdrid] == 'w') {
    
    /* xdrs is open for writing */
    
    if (xdr_int(xdrs, size) == 0)
      return 0;
    size3 = *size * 3;
    /* when the number of coordinates is small, don't try to compress; just
     * write them as floats using xdr_vector
     */
    if (*size <= 9 ) {
      return (xdr_vector(xdrs, (char *) fp, size3, sizeof(*fp),
			 (xdrproc_t)xdr_float));
    }
    
    xdr_float(xdrs, precision);
    if (ip == NULL) {
      ip = (int *)malloc(size3 * sizeof(*ip));
      if (ip == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      bufsize = size3 * 1.2;
      buf = (int *)malloc(bufsize * sizeof(*buf));
      if (buf == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      oldsize = *size;
    } else if (*size > oldsize) {
      ip = (int *)realloc(ip, size3 * sizeof(*ip));
      if (ip == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      bufsize = size3 * 1.2;
      buf = (int *)realloc(buf, bufsize * sizeof(*buf));
      buf = (int *)malloc(bufsize * sizeof(*buf));
      if (buf == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      oldsize = *size;
    }
    /* buf[0-2] are special and do not contain actual data */
    buf[0] = buf[1] = buf[2] = 0;
    minint[0] = minint[1] = minint[2] = INT_MAX;
    maxint[0] = maxint[1] = maxint[2] = INT_MIN;
    prevrun = -1;
    lfp = fp;
    lip = ip;
    mindiff = INT_MAX;
    oldlint1 = oldlint2 = oldlint3 = 0;
    while(lfp < fp + size3 ) {
      /* find nearest integer */
      if (*lfp >= 0.0)
	lf = *lfp * *precision + 0.5;
      else
	lf = *lfp * *precision - 0.5;
      if (fabs(lf) > MAXABS) {
	/* scaling would cause overflow */
	errval = 0;
      }
      lint1 = lf;
      if (lint1 < minint[0]) minint[0] = lint1;
      if (lint1 > maxint[0]) maxint[0] = lint1;
      *lip++ = lint1;
      lfp++;
      if (*lfp >= 0.0)
	lf = *lfp * *precision + 0.5;
      else
	lf = *lfp * *precision - 0.5;
      if (fabs(lf) > MAXABS) {
	/* scaling would cause overflow */
	errval = 0;
      }
      lint2 = lf;
      if (lint2 < minint[1]) minint[1] = lint2;
      if (lint2 > maxint[1]) maxint[1] = lint2;
      *lip++ = lint2;
      lfp++;
      if (*lfp >= 0.0)
	lf = *lfp * *precision + 0.5;
      else
	lf = *lfp * *precision - 0.5;
      if (fabs(lf) > MAXABS) {
	/* scaling would cause overflow */
	errval = 0;
      }
      lint3 = lf;
      if (lint3 < minint[2]) minint[2] = lint3;
      if (lint3 > maxint[2]) maxint[2] = lint3;
      *lip++ = lint3;
      lfp++;
      diff = abs(oldlint1-lint1)+abs(oldlint2-lint2)+abs(oldlint3-lint3);
      if (diff < mindiff && lfp > fp + 3)
	mindiff = diff;
      oldlint1 = lint1;
      oldlint2 = lint2;
      oldlint3 = lint3;
    }
    xdr_int(xdrs, &(minint[0]));
    xdr_int(xdrs, &(minint[1]));
    xdr_int(xdrs, &(minint[2]));
    
    xdr_int(xdrs, &(maxint[0]));
    xdr_int(xdrs, &(maxint[1]));
    xdr_int(xdrs, &(maxint[2]));
    
    if ((float)maxint[0] - (float)minint[0] >= MAXABS ||
	(float)maxint[1] - (float)minint[1] >= MAXABS ||
	(float)maxint[2] - (float)minint[2] >= MAXABS) {
      /* turning value in unsigned by subtracting minint
       * would cause overflow
       */
      errval = 0;
    }
    sizeint[0] = maxint[0] - minint[0]+1;
    sizeint[1] = maxint[1] - minint[1]+1;
    sizeint[2] = maxint[2] - minint[2]+1;
    
    /* check if one of the sizes is to big to be multiplied */
    if ((sizeint[0] | sizeint[1] | sizeint[2] ) > 0xffffff) {
      bitsizeint[0] = sizeofint(sizeint[0]);
      bitsizeint[1] = sizeofint(sizeint[1]);
      bitsizeint[2] = sizeofint(sizeint[2]);
      bitsize = 0; /* flag the use of large sizes */
    } else {
      bitsize = sizeofints(3, sizeint);
    }
    lip = ip;
    luip = (unsigned int *) ip;
    smallidx = FIRSTIDX;
    while (smallidx < LASTIDX && magicints[smallidx] < mindiff) {
      smallidx++;
    }
    xdr_int(xdrs, &smallidx);
    maxidx = MIN(LASTIDX, smallidx + 8) ;
    minidx = maxidx - 8; /* often this equal smallidx */
    smaller = magicints[MAX(FIRSTIDX, smallidx-1)] / 2;
    small = magicints[smallidx] / 2;
    sizesmall[0] = sizesmall[1] = sizesmall[2] = magicints[smallidx];
    larger = magicints[maxidx] / 2;
    i = 0;
    while (i < *size) {
      is_small = 0;
      thiscoord = (int *)(luip) + i * 3;
      if (smallidx < maxidx && i >= 1 &&
	  abs(thiscoord[0] - prevcoord[0]) < larger &&
	  abs(thiscoord[1] - prevcoord[1]) < larger &&
	  abs(thiscoord[2] - prevcoord[2]) < larger) {
	is_smaller = 1;
      } else if (smallidx > minidx) {
	is_smaller = -1;
      } else {
	is_smaller = 0;
      }
      if (i + 1 < *size) {
	if (abs(thiscoord[0] - thiscoord[3]) < small &&
	    abs(thiscoord[1] - thiscoord[4]) < small &&
	    abs(thiscoord[2] - thiscoord[5]) < small) {
	  /* interchange first with second atom for better
	   * compression of water molecules
	   */
	  tmp = thiscoord[0]; thiscoord[0] = thiscoord[3];
	  thiscoord[3] = tmp;
	  tmp = thiscoord[1]; thiscoord[1] = thiscoord[4];
	  thiscoord[4] = tmp;
	  tmp = thiscoord[2]; thiscoord[2] = thiscoord[5];
	  thiscoord[5] = tmp;
	  is_small = 1;
	}
	
      }
      tmpcoord[0] = thiscoord[0] - minint[0];
      tmpcoord[1] = thiscoord[1] - minint[1];
      tmpcoord[2] = thiscoord[2] - minint[2];
      if (bitsize == 0) {
	sendbits(buf, bitsizeint[0], tmpcoord[0]);
	sendbits(buf, bitsizeint[1], tmpcoord[1]);
	sendbits(buf, bitsizeint[2], tmpcoord[2]);
      } else {
	sendints(buf, 3, bitsize, sizeint, tmpcoord);
      }
      prevcoord[0] = thiscoord[0];
      prevcoord[1] = thiscoord[1];
      prevcoord[2] = thiscoord[2];
      thiscoord = thiscoord + 3;
      i++;
      
      run = 0;
      if (is_small == 0 && is_smaller == -1)
	is_smaller = 0;
      while (is_small && run < 8*3) {
	if (is_smaller == -1 && (
				 SQR(thiscoord[0] - prevcoord[0]) +
				 SQR(thiscoord[1] - prevcoord[1]) +
				 SQR(thiscoord[2] - prevcoord[2]) >= smaller * smaller)) {
	  is_smaller = 0;
	}
	
	tmpcoord[run++] = thiscoord[0] - prevcoord[0] + small;
	tmpcoord[run++] = thiscoord[1] - prevcoord[1] + small;
	tmpcoord[run++] = thiscoord[2] - prevcoord[2] + small;
	
	prevcoord[0] = thiscoord[0];
	prevcoord[1] = thiscoord[1];
	prevcoord[2] = thiscoord[2];
	
	i++;
	thiscoord = thiscoord + 3;
	is_small = 0;
	if (i < *size &&
	    abs(thiscoord[0] - prevcoord[0]) < small &&
	    abs(thiscoord[1] - prevcoord[1]) < small &&
	    abs(thiscoord[2] - prevcoord[2]) < small) {
	  is_small = 1;
	}
      }
      if (run != prevrun || is_smaller != 0) {
	prevrun = run;
	sendbits(buf, 1, 1); /* flag the change in run-length */
	sendbits(buf, 5, run+is_smaller+1);
      } else {
	sendbits(buf, 1, 0); /* flag the fact that runlength did not change */
      }
      for (k=0; k < run; k+=3) {
	sendints(buf, 3, smallidx, sizesmall, &tmpcoord[k]);	
      }
      if (is_smaller != 0) {
	smallidx += is_smaller;
	if (is_smaller < 0) {
	  small = smaller;
	  smaller = magicints[smallidx-1] / 2;
	} else {
	  smaller = small;
	  small = magicints[smallidx] / 2;
	}
	sizesmall[0] = sizesmall[1] = sizesmall[2] = magicints[smallidx];
      }
    }
    if (buf[1] != 0) buf[0]++;;
    xdr_int(xdrs, &(buf[0])); /* buf[0] holds the length in bytes */
    return errval * (xdr_opaque(xdrs, (caddr_t)&(buf[3]), (u_int)buf[0]));
  } else {
    
    /* xdrs is open for reading */
    
    if (xdr_int(xdrs, &lsize) == 0) 
      return 0;
    if (*size != 0 && lsize != *size) {
      fprintf(stderr, "wrong number of coordinates in xdr3dfcoor; "
	      "%d arg vs %d in file", *size, lsize);
    }
    *size = lsize;
    size3 = *size * 3;
    if (*size <= 9) {
      return (xdr_vector(xdrs, (char *) fp, size3, sizeof(*fp),
			 (xdrproc_t)xdr_float));
    }
    xdr_float(xdrs, precision);
    if (ip == NULL) {
      ip = (int *)malloc(size3 * sizeof(*ip));
      if (ip == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      bufsize = size3 * 1.2;
      buf = (int *)malloc(bufsize * sizeof(*buf));
      if (buf == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      oldsize = *size;
    } else if (*size > oldsize) {
      ip = (int *)realloc(ip, size3 * sizeof(*ip));
      if (ip == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      bufsize = size3 * 1.2;
      buf = (int *)realloc(buf, bufsize * sizeof(*buf));
      if (buf == NULL) {
	fprintf(stderr,"malloc failed\n");
	return -1;
      }
      oldsize = *size;
    }
    buf[0] = buf[1] = buf[2] = 0;
    
    xdr_int(xdrs, &(minint[0]));
    xdr_int(xdrs, &(minint[1]));
    xdr_int(xdrs, &(minint[2]));
    
    xdr_int(xdrs, &(maxint[0]));
    xdr_int(xdrs, &(maxint[1]));
    xdr_int(xdrs, &(maxint[2]));
    
    sizeint[0] = maxint[0] - minint[0]+1;
    sizeint[1] = maxint[1] - minint[1]+1;
    sizeint[2] = maxint[2] - minint[2]+1;
    
    /* check if one of the sizes is to big to be multiplied */
    if ((sizeint[0] | sizeint[1] | sizeint[2] ) > 0xffffff) {
      bitsizeint[0] = sizeofint(sizeint[0]);
      bitsizeint[1] = sizeofint(sizeint[1]);
      bitsizeint[2] = sizeofint(sizeint[2]);
      bitsize = 0; /* flag the use of large sizes */
    } else {
      bitsize = sizeofints(3, sizeint);
    }
    
    xdr_int(xdrs, &smallidx);
    maxidx = MIN(LASTIDX, smallidx + 8) ;
    minidx = maxidx - 8; /* often this equal smallidx */
    smaller = magicints[MAX(FIRSTIDX, smallidx-1)] / 2;
    small = magicints[smallidx] / 2;
    sizesmall[0] = sizesmall[1] = sizesmall[2] = magicints[smallidx] ;
    larger = magicints[maxidx];
    
    /* buf[0] holds the length in bytes */
    
    if (xdr_int(xdrs, &(buf[0])) == 0)
      return 0;
    if (xdr_opaque(xdrs, (caddr_t)&(buf[3]), (u_int)buf[0]) == 0)
      return 0;
    buf[0] = buf[1] = buf[2] = 0;
    
    lfp = fp;
    inv_precision = 1.0 / * precision;
    run = 0;
    i = 0;
    lip = ip;
    while ( i < lsize ) {
      thiscoord = (int *)(lip) + i * 3;
      
      if (bitsize == 0) {
	thiscoord[0] = receivebits(buf, bitsizeint[0]);
	thiscoord[1] = receivebits(buf, bitsizeint[1]);
	thiscoord[2] = receivebits(buf, bitsizeint[2]);
      } else {
	receiveints(buf, 3, bitsize, sizeint, thiscoord);
      }
      
      i++;
      thiscoord[0] += minint[0];
      thiscoord[1] += minint[1];
      thiscoord[2] += minint[2];
	    
      prevcoord[0] = thiscoord[0];
      prevcoord[1] = thiscoord[1];
      prevcoord[2] = thiscoord[2];
	    
	   
      flag = receivebits(buf, 1);
      is_smaller = 0;
      if (flag == 1) {
	run = receivebits(buf, 5);
	is_smaller = run % 3;
	run -= is_smaller;
	is_smaller--;
      }
      if (run > 0) {
	thiscoord += 3;
	for (k = 0; k < run; k+=3) {
	  receiveints(buf, 3, smallidx, sizesmall, thiscoord);
	  i++;
	  thiscoord[0] += prevcoord[0] - small;
	  thiscoord[1] += prevcoord[1] - small;
	  thiscoord[2] += prevcoord[2] - small;
	  if (k == 0) {
	    /* interchange first with second atom for better
	     * compression of water molecules
	     */
	    tmp = thiscoord[0]; thiscoord[0] = prevcoord[0];
	    prevcoord[0] = tmp;
	    tmp = thiscoord[1]; thiscoord[1] = prevcoord[1];
	    prevcoord[1] = tmp;
	    tmp = thiscoord[2]; thiscoord[2] = prevcoord[2];
	    prevcoord[2] = tmp;
	    *lfp++ = prevcoord[0] * inv_precision;
	    *lfp++ = prevcoord[1] * inv_precision;
	    *lfp++ = prevcoord[2] * inv_precision;
	  } else {
	    prevcoord[0] = thiscoord[0];
	    prevcoord[1] = thiscoord[1];
	    prevcoord[2] = thiscoord[2];
	  }
	  *lfp++ = thiscoord[0] * inv_precision;
	  *lfp++ = thiscoord[1] * inv_precision;
	  *lfp++ = thiscoord[2] * inv_precision;
	}
      } else {
	*lfp++ = thiscoord[0] * inv_precision;
	*lfp++ = thiscoord[1] * inv_precision;
	*lfp++ = thiscoord[2] * inv_precision;		
      }
      smallidx += is_smaller;
      if (is_smaller < 0) {
	small = smaller;
	if (smallidx > FIRSTIDX) {
	  smaller = magicints[smallidx - 1] /2;
	} else {
	  smaller = 0;
	}
      } else if (is_smaller > 0) {
	smaller = small;
	small = magicints[smallidx] / 2;
      }
      sizesmall[0] = sizesmall[1] = sizesmall[2] = magicints[smallidx] ;
    }
  }
  return 1;
}




#define XTC_CHECK(s,b) xtc_check(s,b,__FILE__,__LINE__)
#define XTC_MAGIC 1995
#define snew(ptr,nelem) (ptr)=save_calloc(#ptr,__FILE__,__LINE__,\
                        (nelem),sizeof(*(ptr)))

static void *save_calloc(char *name,char *file,int line,
			unsigned nelem,unsigned elsize)
{
  void *p;
  
  p=NULL;
  if ((nelem==0)||(elsize==0))
    p=NULL;
  else
    {
      if ((p=calloc((size_t)nelem,(size_t)elsize))==NULL) {
        fprintf(stderr,"calloc for %s (nelem=%d, elsize=%d, file %s"
		", line %d)",name,nelem,elsize,file,line);
	return NULL;
      }
    }
  return p;
}


static int xtc_check(char *str,int bResult,char *file,int line)
{
  if (!bResult) {
    fprintf(stderr,"XTC error: read/write of %s failed, "
            "source file %s, line %d\n",str,file,line);
    return 0;
  }
  return 1;
}


static int xtc_header(XDR *xd,int *magic,int *natoms,int *step,float *time,
		      int *bOK)
{
  int result=0;

  if (xdr_int(xd,magic) == 0)  return 0;
  result=XTC_CHECK("natoms", xdr_int(xd,natoms));  /* number of atoms */
  if (result)
    result=XTC_CHECK("step",   xdr_int(xd,step));    /* frame number    */
  if (result)
    result=XTC_CHECK("time",   xdr_real(xd,time));   /* time            */
  *bOK=(result!=0);

  return result;
}

static int check_xtc_magic(int magic)
{
  if (magic != XTC_MAGIC) { 
    fprintf(stderr,"Magic Number Error in XTC file (read %d, should be %d)",
                magic,XTC_MAGIC);
    return 0;
  } else {
    return 1;
  }
}

static int open_xtc(XDR *xd,char *filename,char *mode)
{
  char *bf;
  
  if ( !xdropen(xd,filename,mode) ) {
    return 0;
  }
  return 1;
}

static int close_xtc(XDR *xd)
{
  xdrclose(xd);
  return 1;
}

static int xdr3drcoord(XDR *xdrs, float *fp, int *size, float *precision)
{
  return xdr3dfcoord(xdrs,(float *)fp,size,(float *)precision);
}

static int xtc_coord(XDR *xd,int *natoms,matrix box,rvec *x,float *prec)
{
  int i,j,result;
  
  /* box */
  result=1;
  for(i=0; ((i<3) && result); i++)
    for(j=0; ((j<3) && result); j++)
      result=XTC_CHECK("box",xdr_real(xd,&(box[i][j])));
  
  if (result)
    /* coordinates     */
    result=XTC_CHECK("x",xdr3drcoord(xd,x[0],natoms,prec)); 
  
  return result;
}


static int read_first_xtc(XDR *xd,int *natoms,int *step,float *time,
			  matrix box,rvec **x,float *prec,int *bOK)
{
  int magic,ret=0;
  
  *bOK=1;
  
  /* read header and malloc x */
  ret = xtc_header(xd,&magic,natoms,step,time,bOK);
  if (!ret)
    return 0;
    
  /* Check magic number */
  ret = check_xtc_magic(magic);
  if (!ret)
    return 0;
  
  snew(*x,*natoms);

  *bOK=xtc_coord(xd,natoms,box,*x,prec);
  
  return *bOK;
}

static int read_next_xtc(XDR *xd,
		  int *natoms,int *step,float *time,
		  matrix box,rvec *x,float *prec,int *bOK)
{
  int magic;

  *bOK=1;
  
  /* read header */
  if (!xtc_header(xd,&magic,natoms,step,time,bOK))
    return 0;
    
  /* Check magic number */
  check_xtc_magic(magic);

  *bOK=xtc_coord(xd,natoms,box,x,prec);

  return *bOK;
}

/*
  here follows the actual trj reading code
*/

static void copy_frame(rvec *x, struct STRUCT_TRJ_POSITION *pos, int count, double prec, dbmStructNode *node) 
{
  int i;
  for(i=0;i<count;i++) {
    pos[i].x = x[i][0]*10.0;
    pos[i].y = x[i][1]*10.0;
    pos[i].z = x[i][2]*10.0;
  }
}

int xtcTrjRead(char *filename, dbmStructNode *node, int swap_flag)
{
  XDR xdr;
  int i;
  int natoms, step;
  real time;
  matrix box;
  rvec *x;
  real prec;
  bool bok;
  char message[1024];
  int frame=0;
  int frame_max = 10;
  int frame_size;

  open_xtc(&xdr,filename,"r");

  if(read_first_xtc(&xdr,&natoms,&step,&time,box,&x,&prec,&bok)!=1) {
    comMessage("read_first_xtc failed\n");
    close_xtc(&xdr);
    return -1;
  }

  if(natoms != node->atom_count) {
    sprintf(message,"error: atom missmatch between dataset (%d) and trajectory (%d)\n",
	    node->atom_count,natoms);
    comMessage(message);
    close_xtc(&xdr);
    return -1;
  }

  if(node->trj_flag) {
    /* there is already a trajectory associated */
    Cfree(node->trj.pos);
  }

  node->trj_flag=1;
  node->trj.type=STRUCT_TRJ_XTC;
  node->trj.pos=Crecalloc(0,
			  frame_max*natoms,
			  sizeof(struct STRUCT_TRJ_POSITION));

  frame_size = natoms*sizeof(struct STRUCT_TRJ_POSITION);

  // copy first block of atoms into position
  copy_frame(x,&node->trj.pos[frame*natoms],natoms,prec,node);

  /*
  for(i=0;i<natoms;i++) {
    fprintf(stderr,"%d  %g %g %g\n",i,x[i][0],x[i][1],x[i][2]);
  }
  */

  while(read_next_xtc(&xdr,&natoms,&step,&time,box,x,&prec,&bok)==1) {
    frame++;
    if(frame>=frame_max) {
      frame_max*=2;
      node->trj.pos=Crecalloc(node->trj.pos,
			      frame_max*natoms,
			      sizeof(struct STRUCT_TRJ_POSITION));
    }
    // copy next atoms
    copy_frame(x,&node->trj.pos[frame*natoms],natoms,prec,node);
  }

  close_xtc(&xdr);

  // now remaining trj attributes can be set
  node->trj.size = frame*frame_size;
  node->trj.frame_count = frame;
  node->trj.atom_count = natoms;

  // resize to free unused memory
  node->trj.pos = Crecalloc(node->trj.pos,
			    frame*natoms,
			    sizeof(struct STRUCT_TRJ_POSITION));
  

  return 0;
}
