#$Id: Makefile,v 1.2 2017/09/20 01:59:26 o1-hester Exp $
#$Date: 2017/09/20 01:59:26 $
#$Revision: 1.2 $
#$Log: Makefile,v $
#Revision 1.2  2017/09/20 01:59:26  o1-hester
#*** empty log message ***
#
#Revision 1.1  2017/09/17 00:30:59  o1-hester
#Initial revision
#
#$Author: o1-hester $

default: all

all: master palin

%.o: %.c
	gcc -c $< -o $@

master: master.o 
	gcc $< -o $@

palin: palin.o
	gcc $< -o $@

clean:
	-rm -f *.o
