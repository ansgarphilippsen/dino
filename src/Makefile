# Makefile for dino

# possible flags
# CPK_NEW
# PICK_NEW
# BUILD
# FORMAT_VRML
# FORMAT_TIFF
# SGI_STEREO
# USE_TCL
# USE_DLIST
# WITH_NCBONDS
# CONTOUR_COLOR
# INTERNAL_COLOR
# IO_GROMACS

OPTIONS= -DFORMAT_TIFF -DCPK_NEW
OPTIONS += -DINTERNAL_COLOR
OPTIONS += -DCONTOUR_COLOR
#OPTIONS += -DVRML

GUI = X11

# only for linux versions
#STATIC = 1

SRC= dino.c \
     dbm.c com.c gfx.c help.c conn.c scene.c glf.c glw.c \
     startup.c cl.c set.c bspline.c transform.c prop.c colors.c\
     struct_db.c struct_obj.c struct_read.c struct_write.c struct_gfx.c \
     scal_db.c scal_obj.c scal_read.c scal_write.c scal_gfx.c \
     scal_mc_new.c scal_iso.c \
     surf_db.c surf_obj.c surf_read.c surf_write.c surf_gfx.c \
     geom_db.c geom_obj.c geom_gfx.c \
     grid_db.c grid_obj.c grid_read.c grid_gfx.c \
     ccp4.c xplor.c uhbd.c mead.c delphi.c charmm.c io_spider.c \
     write.c render.c mat.c rex.c Cmalloc.c cgfx.c pov.c \
     cubearray.c writePS.c \
     sglib.c symm.c \
     gui_terminal.c shell_raw.c shell_command.c \
     cmi.c input.c \
     io_gromacs.c


# add more stuff depending on OPTIONS

# test code for VRML output
ifneq (,$(findstring VRML,$(OPTIONS)))
SRC += vrml.c
endif

# test code for model building
ifneq (,$(findstring BUILD,$(OPTIONS)))
SRC += build.c
endif

# include architecture specific settings, including additional SRC files
# use HOSTTYPE variable
sinclude makedef.$(HOSTTYPE)

ifneq (,$(findstring USE_TCL,$(OPTIONS)))
LOPT += $(LIBTCL)
endif

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
	gzip < ../bin/dino > ../bin/dino-0.8.6-$(DATE).$(MACH).gz

iso_tab: gen_iso_table.c
	$(CC) gen_iso_table.c -o gen_iso_table
	-./gen_iso_table > scal_iso_table.h


all: clean depend dino

sinclude .depend
