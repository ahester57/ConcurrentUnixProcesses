/*
$Id: ipchelper.c,v 1.1 2017/09/22 22:55:29 o1-hester Exp $
$Date: 2017/09/22 22:55:29 $
$Revision: 1.1 $
$Log: ipchelper.c,v $
Revision 1.1  2017/09/22 22:55:29  o1-hester
Initial revision

$Author: o1-hester $
*/

#include "ipchelper.h"

// Initializes semaphore with id, num, and value
// returns -1 on failure
int initelement(int semid, int semnum, int semval) {
	union semun {
		int val;
		struct semid_ds *buf;
		unsigned short *array;
	} arg;
	arg.val = semval;
	return semctl(semid, semnum, SETVAL, arg);
}

//set up a semaphore operation
void setsembuf(struct sembuf *s, int n, int op, int flg) {
	s->sem_num = (short)n;
	s->sem_op = (short)op;
	s->sem_flg = (short)flg;
	return;
}

// destroy message queue segment
int removeMsgQueue(int msgid) {
	return msgctl(msgid, IPC_RMID, NULL);
}
