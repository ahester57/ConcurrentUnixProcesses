/*
$Id: palin.c,v 1.7 2017/09/23 04:43:42 o1-hester Exp $
$Date: 2017/09/23 04:43:42 $ 
$Revision: 1.7 $
$Log: palin.c,v $
Revision 1.7  2017/09/23 04:43:42  o1-hester
trying to set up signal handlers

Revision 1.6  2017/09/22 22:56:10  o1-hester
palindrome support

Revision 1.5  2017/09/20 06:50:22  o1-hester
child limit

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

#include <time.h>
#include "ipchelper.h"
#define PALIN "./palin.out"
#define NOPALIN "./nopalin.out"

long masterpid;

int writeToFile(const char* filename, long pid, int index, const char* text); 
int palindrome(const char* string);
char* trimstring(const char* string);
void catch(int signo) {
	kill(masterpid, SIGKILL);
}

int main (int argc, char** argv) {
	// check for # of args
	if (argc < 4) {
		fprintf(stderr, "Wrong # of args. ");
		return 1;
	}

	masterpid = (long)atoi(argv[3]);
	kill(masterpid, SIGCONT);
	// random r1 r2
	int r1, r2;
	srand(time(NULL));
	r1 = rand() % 3;
	r2 = rand() % 3;
	
	// if not number, then id, index = 0, respectively
	int id = atoi(argv[1]);
	int index = atoi(argv[2]);
	// get keys
	key_t mkey, skey;
	mkey = ftok(KEYPATH, PROJ_ID);		
	skey = ftok(KEYPATH, SEM_ID);
	

	/*************** Set up signal handler ********/
	struct sigaction act = {0};
	act.sa_handler = catch;
	act.sa_flags = 0;
	if ((sigemptyset(&act.sa_mask) == -1) ||
	    (sigaction(SIGINT, &act, NULL) == -1)) {
		perror("Failed to set SIGINT handler.");
		return 1;
	}	

	/***************** Set up semaphore ************/
	int semid;
	if ((semid = semget(skey, 3, PERM)) == -1) {
		perror("Failed to set up semaphore.");
		return 1;
	}
	struct sembuf mutex[2];
	struct sembuf signalDad[2];
	setsembuf(mutex, 0, -1, 0);
	setsembuf(mutex+1, 0, 1, 0);
	setsembuf(signalDad, 1, -1, 0);
	setsembuf(signalDad+1, 2, 1, 0);
	
	/**************** Set up message queue *********/
	int msgid;
	size_t size;
	mymsg_t mymsg;	
	if ((msgid = msgget(mkey, PERM)) == -1) {
		perror("Failed to create message queue.");
		return 1;
	}
	// receive message into mymsg.mText
	if ((size = msgrcv(msgid, &mymsg, LINESIZE, index+1, 0)) == -1) {
		perror("Failed to recieve message.");
		return 1;
	}

	/************ Entry section ***************/	
	// wait until your turn
	if (semop(semid, mutex, 1) == -1){
		perror("Failed to lock semid.");
		return 1;	
	}
	/************ Critical section ***********/
	sleep(r1);
		
	long pid = (long)getpid();	
	long ppid = (long)getppid();
	const time_t tm = time(NULL);
	char* tme = ctime(&tm);
	fprintf(stderr, "(ch:=%ld)(par:=%ld in crit sec: %s", pid, ppid, tme); 

	int p = palindrome(mymsg.mText);
	char* filename;

	if (p < 0) { filename = NOPALIN; }
	else { filename = PALIN; }

	if (writeToFile(filename, pid, index, mymsg.mText) == -1) {
		// failed to open file
		// unlock file
		if (semop(semid, mutex+1, 1) == -1)
			perror("Failed to unlock semid.");
		// decrement line counter
		if (semop(semid, signalDad, 2) == -1)
			perror("Failed to signal parent.");
		return 1;
	}	

	//sleep(r2);
	/*********** Exit section **************/
	// unlock file
	if (semop(semid, mutex+1, 1) == -1) { 		
		perror("Failed to unlock semid.");
		return 1;
	}
	// decrement line counter
	if (semop(semid, signalDad, 2) == -1) {
		perror("Failed to signal parent.");
		return 1;
	}
 	if (errno != 0) {
		perror("palin:");
		return 1;
	}
	return 0;
}

// writes to file, returns -1 on error, 0 otherwise
int writeToFile(const char* filename, long pid, int index, const char* text) {
	FILE* fp;
	fp = fopen(filename, "a+");
	if (fp == NULL) {
		perror("Failed to open file.");
		return -1;
	}
	fprintf((FILE*)fp, "%ld\t%d\t%s\n", pid, index, text);
	fclose(fp);
	return 0;
}

// returns 0 if palindrome, -1 if not
int palindrome(const char* string) {
	int i = 0;
	char* trimmed = trimstring(string);
	int len = strlen(trimmed);
	int j = len - 1;
	for (i = 0; i < len/2; i++) {
		if (trimmed[i] != trimmed[j]) 
			return -1;
		j--;
	}
	return 0;
}

// returns the string with only alphanumeric lowercase characters
char* trimstring(const char* string) {
	char* trimmed = (char*)malloc(LINESIZE*sizeof(char));
	int i, j;
	char t;
	j = 0;
	for (i = 0; i < LINESIZE; i++) {
		t = string[i];	
		if (isalpha(t) || isdigit(t) || t == '\0') {
			if (isupper(t)) {
				t = tolower(t);	
			}
			trimmed[j] = t;
			if (trimmed[j] == '\0')
				break;
			j++;
		}
	} 	
	return trimmed;
}

