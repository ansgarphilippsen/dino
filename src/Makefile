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

# several GUIs, one must be chose !
# SDL_GLUT
# GLUT_GUI
# X11_GUI


OPTIONS= -DCPK_NEW -DRENDER_SOLID -DHSC_NEWCOL -DSDL_GUI

MALLOCLIB=

# only for linux versions
#STATIC = 1

SRC= dino.c \
     dbm.c shell.c com.c gfx.c help.c conn.c scene.c glf.c\
     cl.c set.c bspline.c transform.c prop.c\
     struct_db.c struct_obj.c struct_read.c struct_write.c struct_gfx.c \
     scal_db.c scal_obj.c scal_read.c scal_write.c scal_gfx.c \
     scal_mc_new.c \
     surf_db.c surf_obj.c surf_read.c surf_write.c surf_gfx.c \
     geom_db.c geom_obj.c geom_gfx.c \
     grid_db.c grid_obj.c grid_read.c grid_gfx.c \
     ccp4.c xplor.c uhbd.c mead.c delphi.c charmm.c\
     write.c render.c mat.c rex.c Cmalloc.c cgfx.c raster.c pov.c \
     cubearray.c input.c writePS.c \
     sglib.c symm.c GLwStereo.c

ifneq (,$(findstring BUILD,$(OPTIONS)))
SRC += build.c
endif

ifneq (,$(findstring JAVA_GUI,$(OPTIONS)))
SRC += jci.c
endif

ifneq (,$(findstring GLUT_GUI,$(OPTIONS)))
SRC += gui_glut.c om_glut.c
endif
ifneq (,$(findstring SDL_GUI,$(OPTIONS)))
SRC += gui_sdl.c om_sdl.c
endif
ifneq (,$(findstring X11_GUI,$(OPTIONS)))
SRC += extension.c gui_x11.c om_x11.c AppPlus.c
endif

ifneq (,$(findstring EXPO,$(OPTIONS)))
SRC += autoplay.c
endif

ifneq (,$(findstring IMAGICK,$(OPTIONS)))
SRC += imagick.c
endif

ifneq (,$(findstring FORMAT_RGB,$(OPTIONS)))
SRC += ximage_util.c
endif

ifneq (,$(findstring FORMAT_RGB,$(OPTIONS)))
SRC += vrml.c
endif

# include architecture specific settings, including additional SRC files
# use HOSTTYPE variable
sinclude makedef.$(HOSTTYPE)

sinclude makedef.java

OBJ := $(SRC:.c=.o)

.SUFFIXES: .c


dino: $(OBJ)
	$(CC) $(OBJ) $(LOPT) -o $@
	cp $@ ../bin
	strip ../bin/$@

jci: jci.c
	$(CC) jci.c $(COPT) $(LOPT) -o jci_test

.c.o:
	$(CC) $(COPT) -c $<

clean:
	rm -f *.o dino *~ core #* .depend

depend:
	gcc -MM $(IOPT) $(SRC) > .depend

all: clean depend dino

sinclude .depend
