# called from toplevel Makefile

SRC =  gui_wx.cpp Shell.cpp
MEMBERS := $(SRC:.cpp=.o)

CADD = -I/usr/lib/wx/include/gtk-2.2 -I../
LOPT += -lwx_gtk_gl-2.2 -lwx_gtk-2.2


gui: gui.a


gui.a: $(MEMBERS)
	ar cr gui.a $?

.cpp.o:
	$(CCP) $(COPT) $(CADD) -c $<

clean:
	rm -f *.o *~ gui.a .depend

depend:
	$(CC) -MM $(INCLUDE) $(OPTIONS) $(CADD) $(SRC) > .depend

sinclude .depend



