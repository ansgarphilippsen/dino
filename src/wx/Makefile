# called from toplevel Makefile

SRC =  gui_wx.cpp om_wx.cpp
MEMBERS := $(SRC:.cpp=.o)

CADD = $(shell wx-config --cppflags) -I../ -I$(HOME)/sft/include

gui: gui.a


gui.a: $(MEMBERS)
	ar cr gui.a $?

.cpp.o:
	$(CPP) $(COPT) $(CADD) -c $<

clean:
	rm -f *.o *~ gui.a .depend

depend:
	$(CC) -MM $(INCLUDE) $(OPTIONS) $(CADD) $(SRC) > .depend

sinclude .depend
