# called from toplevel Makefile

MEMBERS:  main.c gui_glut.c om_glut.c

gui: gui.a


gui.a: $(MEMBERS)
	ar cr gui.a $?

.c.o:
	$(CC) $(COPT) -I../ -c $<


