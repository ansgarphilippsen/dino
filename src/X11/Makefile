# called from toplevel Makefile

ifdef READLINE
TERMHANDLER = gui_x11_readline.c
else
TERMHANDLER = gui_x11_terminal.c
endif


SRC = main_x11.c gui_x11.c om_x11.c AppPlus.c extension.c $(TERMHANDLER) \
      GLwDrawA.c GLwMDrawA.c

sinclude makedef.$(HOSTTYPE)

MEMBERS := $(SRC:.c=.o)


gui: gui.a

gui.a: $(MEMBERS)
	ar cr $@ $?

.c.o:
	$(CC) $(COPT) -I../ -c $<

clean:
	rm -f *.o *~ gui.a .depend


depend:
	gcc -MM $(INCLUDE) $(OPTIONS) -I../ $(SRC) > .depend

sinclude .depend

