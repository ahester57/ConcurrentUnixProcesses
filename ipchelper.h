/*
$Id: ipchelper.h,v 1.4 2017/09/24 23:33:44 o1-hester Exp o1-hester $
$Date: 2017/09/24 23:33:44 $
$Revision: 1.4 $
$Log: ipchelper.h,v $
Revision 1.4  2017/09/24 23:33:44  o1-hester
cleanup, modularization

Revision 1.3  2017/09/24 06:37:14  o1-hester
*** empty log message ***

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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
int getsemid(key_t skey, int nsems);
void setsembuf(struct sembuf *s, int n, int op, int flg);
int getmsgid(key_t mkey);
int sendmessages(int msgid, char** mylist, int lines);
int removeMsgQueue(int msgid);
void setmsgid(int msgid);
int removeshmem(int msgid, int semid);

#endif
