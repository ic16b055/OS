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
* @date 2017/06				 ipcrm -s/-m key
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

static int* ringbuffer_id = (int*)-1;
static long ringbuffer_size = 0;

static int semread_id = -1;
static int semwrite_id = -1;

static int sharedmem_id = -1;

/*
* ------------------------------------------------------------- functions --
*/

int check_parms(const int argc, char* argv[]);				//OK

int init_semaphore(const int key, const int mode);
int block(const int mode);
int unblock(const int mode);

int init_sharedmem(const int key);

void errorhandling(const int mode, const char *error_message);
int cleanup(const int mode);

/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
int run(const int argc, char* argv[], const int mode) {
	
	int temp = 0;
	int counter = 0;
	
	int userid = getuid() * 1000 + 14;	
	
	check_parms(argc,argv);			
	
	semread_id = init_semaphore(userid, EMPFAENGER);
	semwrite_id = init_semaphore(userid + 1, SENDER);
	init_sharedmem(userid + 2);							//vielleicht nur bei Sender?

	
	if (mode == SENDER) {
		
		ringbuffer_id = shmat(sharedmem_id, NULL, 0);		//das erzeugte Shared-Memory-Segment an einen Prozess anbinden
				
		do {
			temp = fgetc(stdin);
			if (ferror(stdin)) {
				errorhandling(mode, "Cannot read fgetc");		
			}


			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			ringbuffer_id[counter] = temp;					// critical section 
			
			if (unblock(mode) == -1) {
				errorhandling(mode, "Semaphore unblock error");
			}
			
		counter = (counter + 1) % ringbuffer_size;

		} while (temp != EOF);
	}
	else {
		
		ringbuffer_id = shmat(sharedmem_id, NULL, SHM_RDONLY);   //das erzeugte Shared-Memory-Segment an einen Prozess anbinden
		
		
		do {
			
			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			temp = ringbuffer_id[counter];					// critical section  
			
			if (unblock(mode) == -1) {
				errorhandling(mode, "Semaphore unblock error");
			}

			if (temp != EOF) 

			if	(fputc(temp, stdout) < 0) {
				errorhandling(mode, "Cannot write fputc");
			}
				
					counter = (counter + 1) % ringbuffer_size;

				} while (temp != EOF);

			}

	cleanup(mode);	
		
	return 0;
}

/**
*
* \name 
*
* \brief 
* 
* \param 	
*
* \return 	
*
*/
int check_parms(const int argc, char* argv[]) {												
	ringbuffer_size = 0;
	int usage = 0;
	int option = 0;
	char* endptr = NULL;
	errno = 0;
	
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
			ringbuffer_size = strtol(optarg, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr, "Usage: -m Characters found in buffersize\n");
				exit(EXIT_FAILURE);
			}		
			if (errno == ERANGE && ringbuffer_size == LONG_MAX) {
				fprintf(stderr, "Usage: -m Buffersize to large\n");
				exit(EXIT_FAILURE);						
			}
			if (ringbuffer_size <= 0) {
				fprintf(stderr, "Usage: -m Buffersize to small\n");
				exit(EXIT_FAILURE);
			}
		}
		else {
			usage = 1;
		}
	}
	if (optind != argc || usage != 0 || errno != 0) {
		fprintf(stderr, "Usage: -m <buffersize>\n");
		exit(EXIT_FAILURE);
	}
	return 0;
}
/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
int init_semaphore(const int key, const int mode) {

	int semid = -1;
	int init_buffer = 0;
	errno = 0;

	if (mode == SENDER) {					//buffer für sender oder init buffer 0 für empfänger
		init_buffer = ringbuffer_size;
	}

	semid = seminit(key, 0660, init_buffer);
	
	if (errno == EEXIST) {
		semid = semgrab(key);
	}
	
	if (semid == -1) {
		fprintf(stderr, "Cannot grab semaphore\n");
		exit(EXIT_FAILURE);
	}
																
	return semid;
}

/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/

int block(const int mode) {

	int sem_id = semwrite_id;

	if (mode == EMPFAENGER) {
		sem_id = semread_id;
	}

	if (P(sem_id) == -1) {
		return -1;
	}
	return 0;
}

/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
int unblock(const int mode) {

	int sem_id = semread_id;

	if (mode == EMPFAENGER) {
		sem_id = semwrite_id;
	}

	if (V(sem_id) == -1) {
		return -1;
	}
	return 0;
}

/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
int init_sharedmem(const int key) {

	sharedmem_id = shmget(key, sizeof(int) * ringbuffer_size, 0666 | IPC_CREAT);

	return 0;
}


/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
void errorhandling(const int mode, const char *error_message) {
	printf("ERROR %c \n", *error_message);


	
	cleanup(mode);

exit(EXIT_FAILURE);
}


/**
*
* \name
*
* \brief
*
* \param
*
* \return
*
*/
int cleanup(const int mode) {
	
	shmdt(ringbuffer_id);		//Anbindung Shared-Memory-Segment vom Prozess aufheben
	
	if (mode == EMPFAENGER) {

	semrm(semread_id);
	semread_id = -1;

	semrm(semwrite_id);
	semwrite_id = -1;

	shmctl(sharedmem_id, IPC_RMID, NULL);
	sharedmem_id = -1;
	
	}
	if (fflush(stdout) == EOF) {
		fprintf(stderr, "Error flushing stdout");
	}
	return 0;
}

/*
* ------------------------------------------------------------------- eof --
*/