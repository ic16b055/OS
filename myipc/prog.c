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

static int* shmptr = (int *)-1;	//Ringbuffer Pointer
static long smsize = 0;				//Ringbuffer Size

static int sem1id = -1;
static int sem2id = -1;

static int shmid = -1;

/*
* ------------------------------------------------------------- functions --
*/

int check_parms(const int argc, char* argv[]);

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

	int userid = getuid() * 1000;

	check_parms(argc, argv);

	if (init_semaphore(userid, EMPFAENGER) == -1) {
		errorhandling(mode, "Cannot init semaphore");
	}

	if (init_semaphore(userid + 1, SENDER) == -1) {
		errorhandling(mode, "Cannot init semaphore");
	}

	if (init_sharedmem(userid + 2) == -1) {				//vielleicht nur bei Sender?
		errorhandling(mode, "Cannot init shared-memory");
	}


	if (mode == SENDER) {

		if ((shmptr = shmat(shmid, NULL, 0)) == (int *)-1) {		//das erzeugte Shared-Memory-Segment an einen Prozess anbinden
			errorhandling(mode, "Shared-Memory einblenden fehler");
		}

		do {
			temp = fgetc(stdin);
			if (ferror(stdin)) {
				errorhandling(mode, "Cannot read fgetc");
			}


			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			shmptr[counter] = temp;					// critical section 

			if (unblock(mode) == -1) {
				errorhandling(mode, "Semaphore unblock error");
			}

			counter = (counter + 1) % smsize;

		} while (temp != EOF);
	}
	else {

		if ((shmptr = shmat(shmid, NULL, SHM_RDONLY)) == (int *)-1) {   //das erzeugte Shared-Memory-Segment an einen Prozess anbinden
			errorhandling(mode, "Shared-Memory einblenden fehler");
		}

		do {

			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			temp = shmptr[counter];					// critical section  

			if (unblock(mode) == -1) {
				errorhandling(mode, "Semaphore unblock error");
			}

			if (temp != EOF)

				if (fputc(temp, stdout) < 0) {
					errorhandling(mode, "Cannot write fputc");
				}

			counter = (counter + 1) % smsize;

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
	smsize = 0;
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
			smsize = strtol(optarg, &endptr, 10);

			if (*endptr != '\0') {
				fprintf(stderr, "Usage: -m Characters found in buffersize\n");
				exit(EXIT_FAILURE);
			}
			if (errno == ERANGE && smsize == LONG_MAX) {
				fprintf(stderr, "Usage: -m Buffersize to large\n");
				exit(EXIT_FAILURE);
			}
			if (smsize <= 0) {
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
		init_buffer = smsize;
	}

	semid = seminit(key, 0660, init_buffer);
		
	
	if (errno == EEXIST) {
		semid = semgrab(key);
		errno = 0;
	}

	if (semid == -1) {
		fprintf(stderr, "Cannot grab Semaphore\n");
		exit(EXIT_FAILURE);
	}

	if (mode == SENDER) {
		sem2id = semid;
	}
	if (mode == EMPFAENGER) {
		sem1id = semid;
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

int block(const int mode) {

	int sem_id = sem2id;

	if (mode == EMPFAENGER) {
		sem_id = sem1id;
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

	int sem_id = sem1id;

	if (mode == EMPFAENGER) {
		sem_id = sem2id;
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

	if ((shmid = shmget(key, (sizeof(int) * smsize), 0660 | IPC_CREAT)) == -1) {
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

	if (shmdt(shmptr) == -1) { 		//Anbindung Shared-Memory-Segment vom Prozess aufheben
		errorhandling(mode, "Cannot Shared-Memory ausblenden");
	}
	if (mode == EMPFAENGER) {

		if (semrm(sem1id) == -1) {
			errorhandling(mode, "Cannot remove Semaphore");
		}
		sem1id = -1;

		if (semrm(sem2id) == -1) {
			errorhandling(mode, "Cannot remove Semaphore");
		}
		sem2id = -1;

		if (shmctl(shmid, IPC_RMID, NULL) == -1) {
			errorhandling(mode, "Cannot Shared-Memory entfernen");
		}
		shmid = -1;

	}
	if (fflush(stdout) == EOF) {
		fprintf(stderr, "Error flushing stdout");
	}
	return 0;
}

/*
* ------------------------------------------------------------------- eof --
*/