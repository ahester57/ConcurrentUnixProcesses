/*
$Id: sighandler.c,v 1.1 2017/09/24 23:32:22 o1-hester Exp $
$Date: 2017/09/24 23:32:22 $
$Revision: 1.1 $
$Log: sighandler.c,v $
Revision 1.1  2017/09/24 23:32:22  o1-hester
Initial revision

$Author: o1-hester $
*/
#include "ipchelper.h"
#include "sighandler.h"

static alarmhappened = 0;
/************************ Signal Handler ********************/
// Handler for SIGINT
void catchctrlc(int signo) {
	alarm(0); // cancel alarm
	if (alarmhappened == 0) {
		char* msg = "Ctrl^C pressed, killing children.\n";
		write(STDERR_FILENO, msg, 36);
	}
	removeshmem(-1, -1);
	pid_t pgid = getpgid(getpid());
	while(wait(NULL)) {
		if (errno == ECHILD)
			break;
	}

	kill(pgid, SIGKILL);
	//sleep(2);
}

// Handler for SIGALRM
void handletimer(int signo) {
	alarmhappened = 1;
	char* msg = "Alarm occured. Time to kill children.\n";
	write(STDERR_FILENO, msg, 39);
	pid_t pgid = getpgid(getpid());

	kill(pgid, SIGINT);
}

// handler for palin SIGINT
void catchchildintr(int signo) {
	char msg[] = "Child interrupted. Goodbye.\n";
	write(STDERR_FILENO, msg, 32);
	exit(1);
}
