# Makefile for dino

# additional options
#
# CMALLOC_DEBUG
# SCAL_DEBUG
# MENU
# STDMALLOC
# USE_VERTEX_ARRAY
# GEOM
# CGFX_DEBUG
# EXPO

OPTIONS= -DMENU -DGEOM -DUSE_VERTEX_ARRAY -D CPK_NEW -D NEWCONN2 -D NEWCONN3

MALLOCLIB=


SRC= dino.c \
     dbm.c shell.c com.c gfx.c gui.c help.c conn.c scene.c glf.c\
     cl.c set.c bspline.c transform.c prop.c\
     struct_db.c struct_obj.c struct_read.c struct_write.c struct_gfx.c\
     scal_db.c scal_obj.c scal_read.c scal_write.c scal_gfx.c \
     scal_mc_new.c \
     surf_db.c surf_obj.c surf_read.c surf_write.c surf_gfx.c \
     geom_db.c geom_obj.c geom_gfx.c \
     grid_db.c grid_obj.c grid_read.c grid_gfx.c \
     ccp4.c xplor.c uhbd.c mead.c delphi.c charmm.c\
     write.c render.c mat.c rex.c extension.c Cmalloc.c cgfx.c raster.c pov.c \
     ximage_util.c vrml.c cubearray.c om.c menu.c input.c writePS.c\
     sglib.c symm.c GLwStereo.c


# include architecture specific settings, including additional SRC files
# use HOSTTYPE variable
sinclude makedef.$(HOSTTYPE)

OBJ := $(SRC:.c=.o)


#OBJ= dino.o \
#     dbm.o shell.o com.o gfx.o gui.o help.o conn.o scene.o glf.o\
#     cl.o set.o bspline.o transform.o prop.o\
#     struct_db.o struct_obj.o struct_read.o struct_write.o struct_gfx.o\
#     scal_db.o scal_obj.o scal_read.o scal_write.o scal_gfx.o \
#     scal_mc_new.o \
#     surf_db.o surf_obj.o surf_read.o surf_write.o surf_gfx.o \
#     geom_db.o geom_obj.o geom_gfx.o \
#     grid_db.o grid_obj.o grid_read.o grid_gfx.o \
#     ccp4.o xplor.o uhbd.o mead.o delphi.o charmm.o\
#     write.o render.o mat.o rex.o extension.o Cmalloc.o cgfx.o raster.o pov.o\
#     ximage_util.o vrml.o cubearray.o om.o menu.o input.o writePS.o\
#     sglib.o symm.o GLwStereo.o \
#     autoplay.o joy.o

.SUFFIXES: .c


dino: $(OBJ)
	$(CC) $(OBJ) $(LOPT) -o $@
	cp $@ ../bin
	strip ../bin/$@

.c.o:
	$(CC) $(COPT) -c $<

clean:
	rm -f *.o dino *~ core #* .depend

depend:
	gcc -MM $(COPT) $(SRC) > .depend

all: clean depend dino

sinclude .depend
