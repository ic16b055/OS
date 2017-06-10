/**
*
* @file prog.c
* Betriebssysteme myipc
* Beispiel 3
*
* @author Maria Kanikova <ic16b002@technikum-wien.at>
* @author Christian Fuhry <ic16b055@technikum-wien.at>
* @author Sebastian Boehm <ic16b032@technikum-wien.at>
*
* @date 2017/06
*
*/


/*
* -------------------------------------------------------------- includes --
*/

#include "prog.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>
#include <sem182.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>

/*
* --------------------------------------------------------------- defines --
*/

#define SENDER 0
#define EMPFAENGER 1

/*
* -------------------------------------------------------------- typedefs --
*/

/*
* --------------------------------------------------------------- globals --
*/
static long buffer_size = 0;

static int sem_id = -1;
/*
* ------------------------------------------------------------- functions --
*/

static void check_parms(const int argc, char* argv[]);
static int init_semaphore(const int userid, const int mode);



int run(const int argc, char* argv[], const int mode) {
	
	int userid = getuid() * 1000 + 14;	
	
	check_parms(argc,argv);			//OK
	
	sem_id = init_semaphore(userid, SENDER);





	
	
										printf("Buffer size: %li\n", buffer_size);
	/*
	if (mode == SENDER)
	{
		printf("run SENDER:%d\n", mode);
	}
	if (mode == EMPFAENGER)
	{
		printf("run EMPFAENGER:%d\n", mode);
	}
	int i;

		for (i = 0; i < argc; i++) 
			{
			printf("argv[%d] = %s ", i, argv[i]);
			printf("\n");
			}	
		*/
		
		
		
		
	return 0;
}


static void check_parms(const int argc, char* argv[]) {												
	buffer_size = 0;
	int usage = 0;
	int option = 0;
	char* endptr = NULL;
	
	if (argc < 2) {
	usage = 1;
	}
	
	while ((option = getopt(argc, argv, "m:")) != -1) {	
		
		errno = 0;

		if (optind < argc) {
			fprintf(stderr, "Usage: -m Too many arguments\n");
			exit(EXIT_FAILURE);
		}
		if (option == 'm') {
			buffer_size = strtol(optarg, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr, "Usage: -m Characters found in buffersize\n");
				exit(EXIT_FAILURE);
			}		
			if ((errno == ERANGE && (buffer_size == LONG_MAX || buffer_size == LONG_MIN)) || (errno != 0 && buffer_size == 0)) {
				fprintf(stderr, "Usage: -m Buffersize out of range\n");
				exit(EXIT_FAILURE);						
			}
			if (buffer_size <= 0) {
				fprintf(stderr, "Usage: -m Buffersize to small\n");
				exit(EXIT_FAILURE);
			}
		}
		else {
			usage = 1;
		}
	}
	if (optind != argc || usage != 0) {
		fprintf(stderr, "Usage: -m <buffersize>\n");
		exit(EXIT_FAILURE);
	}
}
static int init_semaphore(const int userid, const int mode) {

	int semid = -1;
	int init_buffer = 0;
	errno = 0;

	if (mode == SENDER) {					//buffer für sender oder init buffer 0 für empfänger
		init_buffer = buffer_size;
	}

	semid = seminit(userid, 0660, init_buffer);
	
	if (errno == EEXIST) {
		semid = semgrab(userid);
	}
	
	if (semid == -1) {
	//error handling
	}

	printf("SEMID: %d\n", semid);
	printf("INIT_BUFFER: %d\n", init_buffer);
	
	return semid;
}

/*
* ------------------------------------------------------------------- eof --
*/