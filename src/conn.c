#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "conn.h"
#include "Cmalloc.h"
#include "cl.h"

struct CONNECTIVITY conn;

static struct CONN_DEF_ENTRY def_entry[]={
  {"Alanine","ALA",'A',
   {"N","CA","C","O","CB","OXT"},6,
   {{-2,1}, {1,2}, {2,3}, {3,4}, {2,5}, {3,-3}, {6, 3}},7,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
   },3,
   {0, 0, 0, 0, 0, 0},6
  },
  {"Cystein","CYS",'C',
   {"N","CA","C","O","CB","SG","OXT"},7,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{3,-3},{8, 3}},8,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"}
   },4,
   {0, 0, 0, 0, 0, 0, 0},7
  },
  {"Aspartate","ASP",'D',
   {"N","CA","C","O","CB","CG","OD1","OD2","OXT"},9,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{3,-3},{9, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {0, 0, 0, 0, 0, 0, 0, 0, 0},9
  },
  {"Glutamate","GLU",'E',
   {"N","CA","C","O","CB","CG","CD","OE1","OE2","OXT"},10,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,8},{7,9},{3,-3},{10, 3}},11,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"},{5,6,7,8,"chi3"}
   },6,
   {},0
  },
  {"Phenylalanine","PHE",'F',
   {"N","CA","C","O","CB","CG","CD1","CD2","CE1","CE2","CZ","OXT"},12,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{7,9},{8,10},{9,11},{10,11},{3,-3},{12, 3}},14,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Glycin","GLY",'G',
   {"N","CA","C","O","OXT"},5,
   {{-2,1},{1,2},{2,3},{3,4},{3,-3},{5, 3}},6,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
   },3,
   {},0
  },
  {"Histidine","HIS",'H',
   {"N","CA","C","O","CB","CG","ND1","CD2","CE1","NE2","OXT"},11,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{7,9},{8,10},{9,10},{3,-3},{11, 3}},13,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Isoleucine","ILE",'I',
   {"N","CA","C","O","CB","CG1","CG2","CD1","OXT"},9,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{5,7},{6,8},{3,-3},{9, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,8,"chi2"}
   },5,
   {},0
  },
  {"Lysin","LYS",'K',
   {"N","CA","C","O","CB","CG","CD","CE","NZ","OXT"},10,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,8},{8,9},{3,-3},{10, 3}},11,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"},{5,6,7,8,"chi3"},{6,7,8,9,"chi4"}
   },7,
   {},0
  },
  {"Leucin","LEU",'L',
   {"N","CA","C","O","CB","CG","CD1","CD2","OXT"},9,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{3,-3},{9, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Methionine","MET",'M',
   {"N","CA","C","O","CB","CG","SD","CE","OXT"},9,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,8},{3,-3},{10, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"},{5,6,7,8,"chi3"}
   },6,
   {},0
  },
  {"Asparagine","ASN",'N',
   {"N","CA","C","O","CB","CG","OD1","ND2","OXT"},9,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{3,-3},{9, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Proline","PRO",'P',
   {"N","CA","C","O","CB","CG","CD","OXT"},8,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,1},{3,-3},{8, 3}},10,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Glutamine","GLN",'Q',
   {"N","CA","C","O","CB","CG","CD","OE1","NE2","OXT"},10,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,8},{7,9},{3,-3},{10, 3}},11,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"},{5,6,7,8,"chi3"}
   },6,
   {},0
  },
  {"Arginine","ARG",'R',
   {"N","CA","C","O","CB","CG","CD","NE","CZ","NH1","NH2","OXT"},12,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{7,8},{8,9},{9,10},{9,11},{3,-3},{12, 3}},13,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"},{5,6,7,8,"chi3"},{6,7,8,9,"chi4"},{7,8,9,10,"chi5"}
   },8,
   {},0
  },
  {"Serine","SER",'S',
   {"N","CA","C","O","CB","OG","OXT"},7,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{3,-3},{7, 3}},8,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"}
   },4,
   {},0
  },
  {"Threonine","THR",'T',
   {"N","CA","C","O","CB","OG1","CG2","OXT"},8,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{5,7},{3,-3},{8, 3}},9,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"}
   },4,
   {},0
  },
  {"Valine","VAL",'V',
   {"N","CA","C","O","CB","CG1","CG2","OXT"},8,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{5,7},{3,-3},{8, 3}},9,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"}
   },4,
   {},0
  },
  {"Tryptophan","TRP",'W',
   {"N","CA","C","O","CB","CG","CD1","CD2","NE1","CE2","CE3","CZ2","CZ3","CH2","OXT"},15,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{7,9},{8,10},{9,10},{8,11},{10,12},{11,13},{12,14},{13,14},{3,-3},{15, 3}},18,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  {"Tyrosin","TYR",'Y',
   {"N","CA","C","O","CB","CG","CD1","CD2","CE1","CE2","CZ","OH","OXT"},13,
   {{-2,1},{1,2},{2,3},{3,4},{2,5},{5,6},{6,7},{6,8},{7,9},{8,10},{9,11},{10,11},{11,12},{3,-3},{13, 3}},15,
   {{-2,1,2,3,"phi"},{1,2,3,-3,"psi"},{-3,3,2,1,"isp"},
    {1,2,5,6,"chi1"},{2,5,6,7,"chi2"}
   },5,
   {},0
  },
  /* NUCLEIC ACIDS */
  {"Adenosin","A",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","N9","C8","N7","C5","C6","N6","N1","C2","N3","C4","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,22},{7,-3}},14,
   {{}},0,
   {},0
  },
  {"Cytosin","C",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,12},{7,-3}},14,
   {{}},0,
   {},0
  },
  {"Guanidin","G",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  },
  {"Thymidin","T",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{8,12},{1,11}},15,
   {{}},0,
   {},0
  },
  {"Uracil","U",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  },
  {"Adenosin","ADE",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","N9","C8","N7","C5","C6","N6","N1","C2","N3","C4","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,22},{9,12},{7,-3}},15,
   {{}},0,
   {},0
  },
  {"Cytosin","CYT",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,12},{7,-3}},14,
   {{}},0,
   {},0
  },
  {"Guanidin","GUA",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  },
  {"Thymidin","THY",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{8,12},{1,11}},15,
   {{}},0,
   {},0
  },
  {"Uracil","URI",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  },
  {"Adenosin","A",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","N9","C8","N7","C5","C6","N6","N1","C2","N3","C4","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,22},{9,12},{7,-3}},15,
   {{}},0,
   {},0
  },
  {"Cytosin","C",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{1,10},{1,11},{8,12},{7,-3}},14,
   {{}},0,
   {},0
  },
  {"Guanidin","G",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  },
  {"Thymidin","T",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{8,12},{1,11}},15,
   {{}},0,
   {},0
  },
  {"Uracil","U",' ',
   {"P","O5'","C5'","C4'","O4'","C3'","O3'","C2'","C1'","O1P","O2P","O2'"},12,
   {{-2,1},{1,2},{2,3},{3,4},{4,5},{4,6},{6,7},{6,8},{8,9},{5,9},{7,-3},{1,10},{1,11},{8,12}},14,
   {{}},0,
   {},0
  }
};

static int def_entry_count=sizeof(def_entry)/sizeof(struct CONN_DEF_ENTRY);

int conInit()
{
  int i,j;
  
  /*
  if(conLoad(&conn,"connectivity.def")!=0)
    return -1;
  */

  conn.entry_count=def_entry_count;
  conn.entry=Ccalloc(conn.entry_count,sizeof(struct CONN_ENTRY));

  /*
    this memroy management could be done better by
    allocating a big chunk and just distributing pointer
  */

  for(i=0;i<def_entry_count;i++) {
    strcpy(conn.entry[i].name,def_entry[i].name);
    strcpy(conn.entry[i].abbrev,def_entry[i].abbrev);
    conn.entry[i].name_count=def_entry[i].name_count;
    conn.entry[i].name_list=Ccalloc(conn.entry[i].name_count,sizeof(conName));
    for(j=0;j<conn.entry[i].name_count;j++)
      strcpy(conn.entry[i].name_list[j],def_entry[i].name_list[j]);
    conn.entry[i].conn_count=def_entry[i].conn_count;
    conn.entry[i].conn_list=Ccalloc(conn.entry[i].conn_count,sizeof(conConn));
    for(j=0;j<conn.entry[i].conn_count;j++) {
      conn.entry[i].conn_list[j][0]=def_entry[i].conn_list[j][0];
      conn.entry[i].conn_list[j][1]=def_entry[i].conn_list[j][1];
    }
    conn.entry[i].tor_count=def_entry[i].tor_count;
    conn.entry[i].tor_list=Ccalloc(conn.entry[i].tor_count,sizeof(conTorsion));
    for(j=0;j<conn.entry[i].tor_count;j++) {
      conn.entry[i].tor_list[j].n[0]=def_entry[i].tor_list[j].n1;
      conn.entry[i].tor_list[j].n[1]=def_entry[i].tor_list[j].n2;
      conn.entry[i].tor_list[j].n[2]=def_entry[i].tor_list[j].n3;
      conn.entry[i].tor_list[j].n[3]=def_entry[i].tor_list[j].n4;
      clStrncpy(conn.entry[i].tor_list[j].name,def_entry[i].tor_list[j].name,5);
    }
  }

  return 0;
}
enum CONN_CONTEXT {CON_TOP,
		   CON_DEF,
		   CON_ABBREV,
		   CON_ATOMS,
		   CON_CONNECT};

int conLoad(struct CONNECTIVITY *conn, char *filename)
{
  FILE *f;
  char *buf,fline[256];
  int bc;
  struct CONN_ENTRY *entry;
  int entry_max=100;
  int entry_count;
  int wc,i;
  char **wl,p;
  struct stat st;
  int comment;

  if((f=fopen(filename,"r"))==NULL) {
    fprintf(stderr,"error opening %s\n",filename);
    return -1;
  }

  fstat(fileno(f),&st);

  buf=Cmalloc(st.st_size);
  bc=0;

  entry=Ccalloc(entry_max,sizeof(struct CONN_ENTRY));
  entry_count=0;

  p=' ';
  comment=0;
  while(!feof(f)) {
    fgets(fline,sizeof(fline),f);
    
    for(i=0;i<strlen(fline);i++) {
      if(fline[i]=='[') {
	if(!comment)
	  comment=1;
      } else if(fline[i]==']') {
	if(!comment) {
	  fprintf(stderr,"connectivity file parse error\n");
	  return -1;
	}
	comment=0;
      } else if(iscntrl(fline[i])) {
	if(isspace(fline[i]) && isspace(p)) {
	  p=' ';
	} else {
	  p=' ';
	  if(!comment)
	    buf[bc++]=' ';
	}
      } else {
	if(isspace(fline[i]) && isspace(p)) {
	  p=' ';
	} else {
	  p=fline[i];
	  if(!comment)
	    buf[bc++]=fline[i];
	}
      }      
    }
  }
  buf[bc]='\0';

  if(bc>0) {
    conSplitLine(buf,' ',&wc,&wl);
    
    i=0;
    while(i<wc) {

      if(!strcmp(wl[i],"define")) {
	i++;
	conAssembleEntry(&entry[entry_count++],&i,wl,CON_TOP);
      } else {
	fprintf(stderr,"\nunknown file entry: %s",wl[i]);
	break;
      }
    }
    
    Cfree(wl);    
  }
  
  conn->entry=entry;
  conn->entry_count=entry_count;
  
  return 0;
}



int conAssembleEntry(struct CONN_ENTRY *entry, int *wc, char **wl, int context){
  int i;
  int nwc;
  char **nwl;

  if(context==CON_TOP) {
    conSplitLine(wl[(*wc)++],' ',&nwc,&nwl);
    strcpy(entry->name,nwl[0]);
    Cfree(nwl);
    conSplitLine(wl[(*wc)++],' ',&nwc,&nwl);
    i=0;
    while(i<nwc) {
      if(!strcmp(nwl[0],"abbrev")) {
	conAssembleEntry(entry,&i,nwl,CON_ABBREV);
      } else if(!strcmp(nwl[0],"atoms")) {
      } else if(!strcmp(nwl[0],"connect")) {
      } else {
	fprintf(stderr,"unknown entry in define: %s\n",nwl[0]);
	return -1;
      }
      i++;
    }
    Cfree(nwl);
    
  }
  return 0;
}

int conSplitLine(char *expr, char split_char, int *nwc, char ***wl)
{
  int c,s,wc;
  char **owl,**nwl;
  int bc;

  int wc_max=1024;
  nwl=Ccalloc(wc_max,sizeof(char *));

  c=0;
  wc=0;
  s=strlen(expr);
  nwl[wc]=&expr[c];
  while(c<s) {
    if(expr[c]==split_char) {
      expr[c]='\0';
      c++;
      if(c>s)
	break;
      if(split_char==' ' && strlen(nwl[wc])==0) {
	nwl[wc]=&expr[c];
      } else {
	if(wc>=wc_max) {
	  owl=nwl;
	  nwl=Ccalloc(wc_max+1024,sizeof(char *));
	  memcpy(nwl,owl,sizeof(char *)*wc_max);
	  wc_max+=1024;
	}
	nwl[++wc]=&expr[c];
      }
    } else if(expr[c]=='{') {
      expr[c]='\0';
      c++;
      if(c>s)
	break;
      if(strlen(nwl[wc])>0) {
	if(wc>=wc_max) {
	  owl=nwl;
	  nwl=Ccalloc(wc_max+1024,sizeof(char *));
	  memcpy(nwl,owl,sizeof(char *)*wc_max);
	  wc_max+=1024;
	}
	nwl[++wc]=&expr[c];
      } else {
	nwl[wc]=&expr[c];
      }
      bc=1;
      while(bc>=1) {
	if(expr[c]=='{')
	  bc++;
	if(expr[c]=='}')
	  bc--;
	c++;
	if(c>=s)
	  break;
      }
      expr[c-1]='\0';
      if(c>s)
	break;
      if(wc>=wc_max) {
	owl=nwl;
	nwl=Ccalloc(wc_max+1024,sizeof(char *));
	memcpy(nwl,owl,sizeof(char *)*wc_max);
	wc_max+=1024;
      }
      nwl[++wc]=&expr[c];
    } else {
      c++;
    }
  }

  if(strlen(nwl[wc])>0)
    wc++;

  (*nwc)=wc;  
  (*wl)=nwl;
  return 0;

}


struct CONN_ENTRY * conGetEntry(char *abbrev)
{
  int i;
  struct CONN_ENTRY *entry=NULL;

  for(i=0;i<conn.entry_count;i++)
    if(!strcmp(abbrev,conn.entry[i].abbrev))
      entry=&conn.entry[i];

  return entry;
}

int conGetAtomID(struct CONN_ENTRY *entry, char *name)
{
  int i;

  for(i=0;i<entry->name_count;i++)
    if(!strcmp(name,entry->name_list[i]))
      return i+1;

  return 0;
}

int conGetNextID(struct CONN_ENTRY *entry)
{
  int i;
  
  for(i=0;i<entry->conn_count;i++)
    if(entry->conn_list[i][0]==-3)
      return(entry->conn_list[i][1]);
    else if(entry->conn_list[i][1]==-3)
      return(entry->conn_list[i][0]);
	
  return 0;
}

int conGetPrevID(struct CONN_ENTRY *entry)
{
  int i;

  for(i=0;i<entry->conn_count;i++)
    if(entry->conn_list[i][0]==-2)
      return(entry->conn_list[i][1]);
    else if(entry->conn_list[i][1]==-2)
      return(entry->conn_list[i][0]);
	
  return 0;
}

int conIsConnected(struct CONN_ENTRY *entry, int id1, int id2)
{
  int i;

  for(i=0;i<entry->conn_count;i++) {
    if(id1==entry->conn_list[i][0] && id2==entry->conn_list[i][1])
      return 1;
    else if(id2==entry->conn_list[i][0] && id1==entry->conn_list[i][1])
      return 1;
  }

  return 0;
}

