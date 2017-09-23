/*
$Id: ipchelper.h,v 1.2 2017/09/23 04:43:08 o1-hester Exp $
$Date: 2017/09/23 04:43:08 $
$Revision: 1.2 $
$Log: ipchelper.h,v $
Revision 1.2  2017/09/23 04:43:08  o1-hester
*** empty log message ***

Revision 1.1  2017/09/22 22:55:33  o1-hester
Initial revision

$Author: o1-hester $
*/

#ifndef IPCHELPER_H_
#define IPCHELPER_H_

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/stat.h>
#define PERM (S_IRUSR | S_IWUSR)
#define KEYPATH "./yobanana.boy"
#define PROJ_ID 456234
#define SEM_ID 234456
#define LINESIZE 256

// for message queues
typedef struct {
	long mType;
	char mText[LINESIZE];
} mymsg_t;

int initelement(int semid, int semnum, int semval);
void setsembuf(struct sembuf *s, int n, int op, int flg);
int removeMsgQueue(int msgid);
void catchctrlc(int signo);
void handletimer(int signo);

#endif
