/*
$Id: palin.c,v 1.4 2017/09/20 01:59:32 o1-hester Exp o1-hester $
$Date: 2017/09/20 01:59:32 $ 
$Revision: 1.4 $
$Log: palin.c,v $
Revision 1.4  2017/09/20 01:59:32  o1-hester
semaphonres set up

Revision 1.3  2017/09/17 22:50:15  o1-hester
shm to msgqueue

Revision 1.2  2017/09/17 21:24:28  o1-hester
shared memory work

Revision 1.1  2017/09/17 00:31:23  o1-hester
Initial revision

$Author: o1-hester $
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#define KEYPATH "./yobanana.boy"
#define PROJ_ID 456234
#define SEM_ID 234456
#define PERM (S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH)

typedef struct {
	long mType;
	char mText[80];
} mymsg_t;

int palindrome(const char* string);
void setsembuf(struct sembuf *s, int n, int op, int flg);

int main (int argc, char** argv) {
	// check for # of args
	if (argc < 3) {
		fprintf(stderr, "Wrong # of args. ");
		exit(1);
	}
	// random r1 r2
	int r1, r2;
	srand(time(NULL));
	r1 = rand() % 3;
	r2 = rand() % 3;

	int id = atoi(argv[1]);
	int index = atoi(argv[2]);
	// get key for shmem
	key_t mkey, skey;
	mkey = ftok(KEYPATH, PROJ_ID);		
	skey = ftok(KEYPATH, SEM_ID);
	
	/***************** Set up semaphore ************/
	int semid;
	if ((semid = semget(skey, 3, PERM)) == -1) {
		perror("Failed to set up semaphore.");
		return 1;
	}
	struct sembuf wait[1];
	struct sembuf signal[1];
	struct sembuf signalDad[1];
	struct sembuf imdone[1];
	setsembuf(wait, 0, -1, 0);
	setsembuf(signal, 0, 1, 0);
	setsembuf(signalDad, 1, -1, 0);
	setsembuf(imdone, 2, 1, 0);
	
	/**************** Set up message queue *********/
	int msgid;
	int size;
	mymsg_t mymsg;	
	if ((msgid = msgget(mkey, PERM)) == -1) {
		perror("Failed to create message queue.");
		return 1;
	}
	if ((size = msgrcv(msgid, &mymsg, 80, index+1, 0)) == -1) {
		perror("Failed to recieve message.");
		return 1;
	}
	/************ Entry section ***************/	
	semop(semid, wait, 1);
	/************ Critical section ***********/
	sleep(r1);
	fprintf(stderr, "%ld\t%d\t%s\n", (long)getpid(), index, mymsg.mText);
	sleep(r2);
	/*********** Exit section **************/
	semop(semid, signal, 1); // unlock file
	semop(semid, signalDad, 1); // decrement line counter
	semop(semid, imdone, 1);	// tell dad im off to college
	if (errno != 0)
		perror("palin:");
	return 0;
}

int palindrome(const char* string) {

}


//set up a semaphore operation
void setsembuf(struct sembuf *s, int n, int op, int flg) {
	s->sem_num = (short)n;
	s->sem_op = (short)op;
	s->sem_flg = (short)flg;
	return;
}
