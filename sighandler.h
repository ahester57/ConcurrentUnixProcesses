/*
$Id: sighandler.h,v 1.1 2017/09/24 23:33:44 o1-hester Exp $
$Date: 2017/09/24 23:33:44 $
$Revision: 1.1 $
$Log: sighandler.h,v $
Revision 1.1  2017/09/24 23:33:44  o1-hester
Initial revision

$Author: o1-hester $
*/
#ifndef SIGHANDLER_H_
#define SIGHANDLER_H_

#include "ipchelper.h"
#include <signal.h>

void catchctrlc(int signo);
void handletimer(int signo);
void catchchildintr(int signo);

#endif
