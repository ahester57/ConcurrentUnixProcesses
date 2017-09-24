/*
$Id: ipchelper.c,v 1.2 2017/09/23 04:43:04 o1-hester Exp o1-hester $
$Date: 2017/09/23 04:43:04 $
$Revision: 1.2 $
$Log: ipchelper.c,v $
Revision 1.2  2017/09/23 04:43:04  o1-hester
*** empty log message ***

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

// Handler for SIGINT
void catchctrlc(int signo) {
	char* msg = "Ctrl^C pressed, killing children.\n";
	write(STDERR_FILENO, msg, sizeof(msg));
	pid_t pgid = getpgid(getpid());
	while(wait(NULL)) {
		if (errno == ECHILD)
			break;
	}
	kill(pgid, SIGINT);
	char* env[] = { "SOURCE=MYDATA",
			"TARGET=OUTPUT",
			"lines=65",
			(char*)NULL };
	//sleep(2);
	execle("/bin/ipcrm", "ipcrm", "--all", (char*)NULL, env);
}

// Handler for SIGALRM
void handletimer(int signo) {
	char* msg = "Alarm occured. Time to kill children.\n";
	write(STDERR_FILENO, msg, sizeof(msg));
	pid_t pgid = getpgid(getpid());

	kill(pgid, SIGINT);
	while(wait(NULL)) {
		if (errno == ECHILD)
			break;
	}
	char* env[] = { "SOURCE=MYDATA",
			"TARGET=OUTPUT",
			"lines=65",
			(char*)NULL };
	execle("/bin/ipcrm", "ipcrm", "--all", (char*)NULL, env);
}
