/*
$Id: filehelper.h,v 1.1 2017/09/24 23:33:44 o1-hester Exp $
$Date: 2017/09/24 23:33:44 $
$Revision: 1.1 $
$Log: filehelper.h,v $
Revision 1.1  2017/09/24 23:33:44  o1-hester
Initial revision

$Author: o1-hester $
*/
#ifndef FILEHELPER_H_
#define FILEHELPER_H_

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define LINESIZE 256

int setArrayFromFile(const char* filename, char** list);
int countLines(const char* filename);
int writeToFile(const char* filename, long pid, int index, const char* text); 

#endif
