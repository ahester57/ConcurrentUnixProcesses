#$Id: Makefile,v 1.3 2017/09/22 22:55:14 o1-hester Exp o1-hester $
#$Date: 2017/09/22 22:55:14 $
#$Revision: 1.3 $
#$Log: Makefile,v $
#Revision 1.3  2017/09/22 22:55:14  o1-hester
#added headers
#
#Revision 1.2  2017/09/20 01:59:26  o1-hester
#*** empty log message ***
#
#Revision 1.1  2017/09/17 00:30:59  o1-hester
#Initial revision
#
#$Author: o1-hester $

OBJECTS = ipchelper.o sighandler.o filehelper.o
HEADERS = ipchelper.h sighandler.h filehelper.h
default: all

all: master palin

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

master: master.o $(OBJECTS) 
	gcc $< $(OBJECTS) -o $@

palin: palin.o $(OBJECTS) 
	gcc $< $(OBJECTS) -o $@

clean:
	-rm -f *.o *.out
