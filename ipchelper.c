/*
$Id: ipchelper.c,v 1.5 2017/09/25 06:35:46 o1-hester Exp $
$Date: 2017/09/25 06:35:46 $
$Revision: 1.5 $
$Log: ipchelper.c,v $
Revision 1.5  2017/09/25 06:35:46  o1-hester
*** empty log message ***

Revision 1.4  2017/09/24 23:32:22  o1-hester
cleanup, modularization

Revision 1.3  2017/09/24 06:37:17  o1-hester
better

Revision 1.2  2017/09/23 04:43:04  o1-hester
*** empty log message ***

Revision 1.1  2017/09/22 22:55:29  o1-hester
Initial revision

$Author: o1-hester $
*/

#include "ipchelper.h"

static int sem_id;
static int msg_id;

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

// creates message queue, returns -1 on error and msgid on success
int getsemid(key_t skey, int nsems) {
	int semid;
	if ((semid = semget(skey, nsems, PERM | IPC_CREAT)) == -1) {
		return -1;
	}
	sem_id = semid;
	return sem_id;
}

//set up a semaphore operation
void setsembuf(struct sembuf *s, int n, int op, int flg) {
	s->sem_num = (short)n;
	s->sem_op = (short)op;
	s->sem_flg = (short)flg;
	return;
}

// creates message queue, returns -1 on error and msgid on success
int getmsgid(key_t mkey) {
	int msgid;
	if ((msgid = msgget(mkey, PERM | IPC_CREAT)) == -1) {
		return -1;
	}
	msg_id = msgid;
	return msg_id;
}

// send messages to msgqueue, return -1 on error
int sendmessages(int msgid, char** mylist, int lines) {
	int j;
	mymsg_t* mymsg;
	for (j = 0; j < lines; j++) {
		if ((mymsg = (mymsg_t*) malloc(sizeof(mymsg_t))) == NULL) {
			return -1;
		}
		// mType is index of string
		// mText is string
		// child finds string using mType
		mymsg->mType = j+1;	// because mType cannot be 0
		memcpy(mymsg->mText, mylist[j], LINESIZE);
		if (msgsnd(msgid, mymsg, LINESIZE, 0) == -1) {
			return -1;
		}	
	}
	return 0;
}

// destroy message queue segment
int removeMsgQueue(int msgid) {
	return msgctl(msgid, IPC_RMID, NULL);
}

// Remove shared memory segments
int removeshmem(int msgid, int semid) {
	if (msgid == -1)
		msgid = msg_id;
	if (semid == -1)
		semid = sem_id;
	// Kill message queue
	char* msg = "Killing msgqueue.\n";
	write(STDERR_FILENO, msg, 18);
	if (removeMsgQueue(msgid) == -1) {
		return -1;
	}
	// kill semaphore set
	msg = "Killing semaphore set.\n";
	write(STDERR_FILENO, msg, 23);
	if (semctl(semid, 0, IPC_RMID) == -1) {
		return -1;
	}
	if (errno != 0)
		return -1;
	return 0;
}

