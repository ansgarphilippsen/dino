# Makefile for dino

# possible flags
# CPK_NEW
# RENDER_SOLID
# HSC_NEWCOL
# PICK_NEW
# IMAGICK
# JAVA_GUI
# BUILD
# FORMAT_RGB
# FORMAT_VRML
# FORMAT_TIFF
# SGI_STEREO
# USE_TCL
# USE_DLIST
# WIDTH_NCBONDS
# USE_CMI

OPTIONS= -DRENDER_SOLID -DHSC_NEWCOL -DFORMAT_TIFF -DCPK_NEW
OPTIONS += -DNEW_SHELL -DINTERNAL_COLOR -DUSE_CMI -DUSE_TCL #-DUSE_DLIST

# use tcl as interpreter
#OPTIONS += -DTCL_SHELL

# several GUIs, one must be chosen !
# SDL_GUI
# GLUT_GUI (and USE_FREEGLUT)
# X11_GUI
# WX_GUI

# OBSOLETE, USE VARIABLE GUI NOW
#OPTIONS += -DGLUT_GUI #-DUSE_FREEGLUT
#OPTIONS += -DX11_GUI
#OPTIONS += -DWX_GUI

GUI = X11

MALLOCLIB=

# only for linux versions
#STATIC = 1

SRC= dino.c \
     dbm.c com.c gfx.c help.c conn.c scene.c glf.c glw.c \
     startup.c cl.c set.c bspline.c transform.c prop.c colors.c\
     struct_db.c struct_obj.c struct_read.c struct_write.c struct_gfx.c \
     scal_db.c scal_obj.c scal_read.c scal_write.c scal_gfx.c \
     scal_mc_new.c \
     surf_db.c surf_obj.c surf_read.c surf_write.c surf_gfx.c \
     geom_db.c geom_obj.c geom_gfx.c \
     grid_db.c grid_obj.c grid_read.c grid_gfx.c \
     ccp4.c xplor.c uhbd.c mead.c delphi.c charmm.c\
     write.c render.c mat.c rex.c Cmalloc.c cgfx.c pov.c \
     cubearray.c writePS.c \
     sglib.c symm.c


# add more stuff depending on OPTIONS

# which shell to use
ifneq (,$(findstring NEW_SHELL,$(OPTIONS)))
SRC += gui_terminal.c shell_raw.c shell_command.c
else
ifneq (,$(findstring TCL_SHELL,$(OPTIONS)))
SRC += shell_tcl.c
LOPT += -ltcl
else
SRC += shell.c
endif
endif

# OBSOLETE, USE VARIABLE GUI NOW

# one of the four GUIs: WX, X11, GLUT or SDL
ifneq (,$(findstring WX_GUI,$(OPTIONS)))
#GUI = wx
endif
ifneq (,$(findstring GLUT_GUI,$(OPTIONS)))
#SRC += gui_glut.c om_glut.c
#GUI = glut
endif
ifneq (,$(findstring SDL_GUI,$(OPTIONS)))
#SRC += main.c gui_sdl.c om_sdl.c
#LOPT += -lSDL
endif
ifneq (,$(findstring X11_GUI,$(OPTIONS)))
#SRC += extension.c gui_x11.c om_x11.c AppPlus.c
#GUI    = X11
endif


ifneq (,$(findstring USE_CMI,$(OPTIONS)))
SRC += cmi.c input.c
endif


# test code for imagick image library support
ifneq (,$(findstring IMAGICK,$(OPTIONS)))
SRC += imagick.c
endif

# deprecated code for RGB output
ifneq (,$(findstring FORMAT_RGB,$(OPTIONS)))
SRC += ximage_util.c
endif

# test code for VRML output
ifneq (,$(findstring FORMAT_VRML,$(OPTIONS)))
SRC += vrml.c
endif

# test code for model building
ifneq (,$(findstring BUILD,$(OPTIONS)))
SRC += build.c
endif

# test code for JAVA GUI
ifneq (,$(findstring JAVA_GUI,$(OPTIONS)))
SRC += jci.c
endif

# include architecture specific settings, including additional SRC files
# use HOSTTYPE variable
sinclude makedef.$(HOSTTYPE)

ifneq (,$(findstring USE_TCL,$(OPTIONS)))
LOPT += $(LIBTCL)
endif


#sinclude makedef.java

TMP := $(SRC:.c=.o) 
OBJ := $(TMP:.cpp=.o)

.SUFFIXES: .c .cpp

# for profiling
#COPT += -pg
#LOPT += -pg

COPT += -I./$(GUI)

export CC
export CCP
export COPT
export INCLUDE
export OPTIONS
export .SUFFIXES

DATE = `date +"%Y%m%d"`

sinclude $(GUI)/make.addon

dino: $(OBJ) gui
	$(CC) $(OBJ) $(GUI)/gui.a $(GUILIBS) $(LOPT) -o $@
	cp $@ ../bin/
	$(STRIP) ../bin/$@

gui:
	cd $(GUI) && $(MAKE) gui

.c.o:
	$(CC) $(COPT) -c $<

.cpp.o:
	$(CCP) $(COPT) -c $<

clean:
	rm -f *.o dino *~ core #* .depend
	cd $(GUI) && $(MAKE) clean

depend:
	gcc -MM $(INCLUDE) $(OPTIONS) $(SRC) > .depend
	cd $(GUI) && $(MAKE) depend

snapshot: dino
	gzip < ../bin/dino > ../bin/dino-0.8.5-$(DATE).$(MACH).gz

all: clean depend dino

sinclude .depend
