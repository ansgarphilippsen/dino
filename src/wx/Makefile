# called from toplevel Makefile

MEMBERS =  gui_wx.o

COPT2 = -I/usr/lib/wx/include/gtk-2.2
LOPT += -lwx_gtk_gl-2.2 -lwx_gtk-2.2


gui: gui.a


gui.a: $(MEMBERS)
	ar cr gui.a $?

.cpp.o:
	$(CCP) $(COPT) $(COPT2) -I../ -c $<


