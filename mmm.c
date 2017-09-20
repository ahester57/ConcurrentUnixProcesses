/*
$Id: master.c,v 1.2 2017/09/17 21:24:14 o1-hester Exp o1-hester $
$Date: 2017/09/17 21:24:14 $
$Revision: 1.2 $
$Log: master.c,v $
Revision 1.2  2017/09/17 21:24:14  o1-hester
shared memory work

Revision 1.1  2017/09/17 00:31:12  o1-hester
Initial revision

$Author: o1-hester $ 
*/

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/stat.h>
#define PROJ_ID 456234
#define PERM (S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH)

void setArrayFromFile(char** list, FILE* fp);
int countLines(FILE* fp);
int detachAndRemove(int shmid, void* shmaddr);

int main (int argc, char** argv) {
	const char* filename = "./strings.in";
	const char* keypath = "./yobanana.boy";
	char** mylist;

	FILE* fp = fopen(filename, "r");
	if (fp == NULL) {
		fprintf(stderr, "File not found.\n");
		exit(1);
	}
	// initialize mylist from file
	int lines = countLines(fp);
	mylist = malloc(lines*sizeof(char*));
	setArrayFromFile(mylist, fp);

	// get key from file
	key_t key;
	key = ftok(keypath, PROJ_ID);
		
	int shmid;
	char* shmlist;
	if ((shmid = shmget(key, sizeof(char*), PERM | IPC_CREAT)) == -1) {
		perror("Failed to create shared memory segment.");
		exit(1);
	}
	if ((shmlist = (char*)shmat(shmid, NULL, 0)) == (void*)-1) {
		perror("Failed to attached shared memory segment.");
		if (shmctl(shmid, IPC_RMID, NULL) == -1)
			perror("Failed to remove memory segment.");
		exit(1);
	}
	printf("%d:%d\n", shmid, key);
	int j;
	for (j = 0; j < lines; j++) {
	//	shmlist[j] = &mylist[j];
	}
	for (j = 0; j < lines; j++) {
	//	printf("%s\n", *(shmlist)[j]);
	}
	fprintf(stderr,"segf");
	strcpy(shmlist, mylist[0]);
	//free(mylist);
	
	// make child
	int childid;
	if ((childid = fork()) == -1) {
		perror("Failed to create child.");
		return 1;
	}
	if (childid == 0) {
		execl("./palin", "palin", "1", "0", NULL);
		fprintf(stderr, "wut");
		perror("Exec failure.");
		exit(1); // if error
	}
	if (childid > 0) {
		wait(NULL);
		// destroy shmem segment
		if (detachAndRemove(shmid, shmlist) == -1){
			perror("Failed to destroy memory segment.");
			exit(1);
		}
	}	
	free(mylist);
	return 0;	

}

// Set a char** to a list of strings (by line) from a file
void setArrayFromFile(char** list, FILE* fp) {
	const size_t LINESIZE = 80;
	int n = 0;
	char* line = malloc(LINESIZE*sizeof(char));
	rewind(fp);
	while (fgets(line, LINESIZE, (FILE*)fp)) {
		list[n] = malloc(LINESIZE*sizeof(char));
		// copy line into array, remove newline
		memcpy(list[n], line, LINESIZE);
		list[n][strcspn(list[n], "\n")] = '\0';
		n++;
	}
	free(line);
}

// Count how many lines a file has
int countLines(FILE* fp) {
	const size_t LINESIZE = 80;
	int n = 0;
	char* line = malloc(LINESIZE*sizeof(char));
	rewind(fp);
	while (fgets(line, LINESIZE, (FILE*)fp)) {
		n++;
	}
	free(line);
	return n;
}

// destroy shared memory segment
int detachAndRemove(int shmid, void* shmaddr) {
	int err = 0;
	if (shmdt(shmaddr) == -1)
		err = errno;
	if ((shmctl(shmid, IPC_RMID, NULL) == -1) && !err)
		err = errno;
	if (!err)
		return 0;
	errno = err;
	return -1;	

}

