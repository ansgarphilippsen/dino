struct SPIDER_MAP_HEADER {
  /* 1 */  float fNslice;  // NUMBER OF SLICES (PLANES) IN VOLUME        
  // (=1 FOR AN IMAGE)  FOR NEW LONG LABEL    
  // FORMAT THE VALUE OF NSLICE STORED IN     
  // THE FILE IS NEGATIVE.                       
  /* 2 */  float fNrow;    // NUMBER OF ROWS PER SLICE (Y)                   
  /* 3 */  float fNrec;    // TOTAL NUMBER OF RECORDS (SEE NOTE #3).   
  /* 4 */  float fNlabel;  // AUXILIARY NUMBER TO COMPUTE TOTAL NUMBER OF RECS
  /* 5 */  float fIform;   // FILE TYPE SPECIFIER.                    
  // +3 FOR A 3-D FILE  (FLOAT)                   
  // +1 FOR A 2-D IMAGE (FLOAT)                      
  // -1 FOR A 2-D FOURIER TRANSFORM 
  // -3 FOR A 3-D FOURIER TRANSFORM 
  // -5 FOR A NEW 2-D FOURIER TRANSFORM   
  // -7 FOR A NEW 3-D FOURIER TRANSFORM
  // +8 FOR A 2-D EIGHT BIT IMAGE FILE
  // +9 FOR A 2-D INT IMAGE FILE
  // 10 FOR A 3-D INT IMAGE FILE
  // 11 FOR A 2-D EIGHT BIT COLOR IMAGE FILE                                  
  /* 6 */  float fImami;   // MAXIMUM/MINIMUM FLAG. IS SET AT 0 WHEN THE
  // FILE IS CREATED, AND AT 1 WHEN THE MAXIMUM AND 
  // MINIMUM HAVE BEEN COMPUTED, AND HAVE BEEN STORED 
  // INTO THIS LABEL RECORD (SEE FOLLOWING WORDS)
  /* 7 */  float fFmax;    // MAXIMUM VALUE
  /* 8 */  float fFmin;    // MINIMUM VALUE
  /* 9 */  float fAv;      // AVERAGE VALUE
  /* 10*/  float fSig;     // STANDARD DEVIATION. A VALUE OF -1. INDICATES 
  // THAT SIG HAS NOT BEEN COMPUTED PREVIOUSLY.
  /* 11*/  float fIhist;   // FLAG INDICATING IF THE HISTOGRAM HAS BE 
  // COMPUTED. NOT USED IN 3D FILES!
  /* 12*/  float fNcol;    // NUMBER OF PIXELS PER LINE (Columns X)
  /* 13*/  float fLabrec;  // NUMBER OF LABEL RECORDS IN FILE HEADER
  /* 14*/  float fIangle;  // FLAG THAT TILT ANGLES HAVE BEEN FILLED
  /* 15*/  float fPhi;     // EULER: ROTATIONAL ANGLE
  /* 16*/  float fTheta;   // EULER: TILT ANGLE
  /* 17*/  float fPsi;     // EULER: PSI  = TILT ANGLE
  /* 18*/  float fXoff;    // X TRANSLATION
  /* 19*/  float fYoff;    // Y TRANSLATION
  /* 20*/  float fZoff;    // Z TRANSLATION
  /* 21*/  float fScale;   // SCALE
  /* 22*/  float fLabbyt;  // TOTAL NUMBER OF BYTES IN LABEL
  /* 23*/  float fLenbyt;  // RECORD LENGTH IN BYTES
  char  fNada[24];// this is a spider incongruence
  /* 30*/  float fFlag;    // THAT ANGLES ARE SET. 1 = ONE ADDITIONAL 
  //	ROTATION IS PRESENT, 2 = ADDITIONAL ROTATION 
  // THAT PRECEEDS THE ROTATION THAT WAS STORED IN
  // 15 FOR DETAILS SEE MANUAL CHAPTER VOCEUL.MAN
  /* 31*/  float fPhi1;
  /* 32*/  float fTheta1;
  /* 33*/  float fPsi1;
  /* 34*/  float fPhi2;
  /* 35*/  float fTheta2;
  /* 36*/  float fPsi2;

  double fGeo_matrix[3][3];  // x9 = 72 bytes: Geometric info
  float fAngle1;             // angle info

  float fr1;
  float fr2;                 // lift up cosine mask parameters

  /** Fraga 23/05/97  For Radon transforms **/

  float RTflag;              // 1=RT, 2=FFT(RT)
  float Astart;
  float Aend;
  float Ainc;
  float Rsigma;  		// 4*7 = 28 bytes
  float Tstart;
  float Tend;
  float Tinc;   		// 4*3 = 12, 12+28 = 40B
  char  fNada2[584];         // empty     700-76-40=624-40= 584 bytes

  /*212-214*/ char szIDat[12];   // LOGICAL * 1 ARRAY DIMENSIONED 10, CONTAINING 
  // THE DATE OF CREATION (10 CHARS)
  /*215-216*/ char szITim[8];    // LOGICAL * 1 ARRAY DIMENSIONED 8, CONTAINING 
  // THE TIME OF CREATION (8 CHARS)
  /*217-256*/ char szITit[160];  // LOGICAL * 1 ARRAY DIMENSIONED 160
     
};
