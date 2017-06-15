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
#include <stdint.h>

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

/* Ringbuffer pointer */
static int* shmptr = (int *)-1;
/* Ringbuffer size */
static long smsize = 0;

static int sem1id = -1;
static int sem2id = -1;

static int shmid = -1;

/*
* ------------------------------------------------------------- functions --
*/

static void check_parms(const int argc, char* argv[]);

static int init_semaphore(const int key, const int mode);
static int block(const int mode);
static int unblock(const int mode);

static int init_sharedmem(const int key);

static void errorhandling(const int mode, const char *error_message);
static void cleanup(const int mode);

/**
*
* \run
*
* \This is the main entry point for this program (SENDER or EMPFAENGER)
* \shmat() attaches the shared memory segment identified by shmid to the address space of the calling process.
* \fgetc() reads the next character from stream and returns it as an unsigned char cast to an int, or EOF on end of file or error.
* \fputc() writes the character c, cast to an unsigned char, to stream.
*
* \param argc given numbers of arguments
* \param argv given arguments
* \param const int mode = SENDER or EMPFAENGER
*
* \retval EXIT_SUCCESS if successful
* \retval EXIT_FAILURE if erroneous
*/
int run(const int argc, char* argv[], const int mode) {

	int temp = 0;
	int counter = 0;
	int userid = getuid() * 1000;

	check_parms(argc, argv);

	if (init_semaphore(userid, EMPFAENGER) == -1) {
		errorhandling(mode, "Cannot init semaphore 1");
	}

	if (init_semaphore(userid + 1, SENDER) == -1) {
		errorhandling(mode, "Cannot init semaphore 2");
	}

	if (init_sharedmem(userid + 2) == -1) {
		errorhandling(mode, "Cannot init shared-memory");
	}

	if (mode == SENDER) {

		if ((shmptr = shmat(shmid, NULL, 0)) == (int *)-1) {
			errorhandling(mode, "Shared-Memory attach error");
		}

		do {
			temp = fgetc(stdin);
			if (ferror(stdin)) {
				errorhandling(mode, "Cannot read fgetc");
			}


			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			/* critical section */
			shmptr[counter] = temp;

			if (unblock(mode) == -1) {
				errorhandling(mode, "Semaphore unblock error");
			}

			counter = (counter + 1) % smsize;

		} while (temp != EOF);
	}
	else if (mode == EMPFAENGER) {

		if ((shmptr = shmat(shmid, NULL, SHM_RDONLY)) == (int *)-1) {
			errorhandling(mode, "Shared-Memory attach error");
		}

		do {

			if (block(mode) == -1) {
				errorhandling(mode, "Semaphore block error");
			}
			/* critical section */
			temp = shmptr[counter];

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
	else {
		errorhandling(mode, "Programm error");
	}

	cleanup(mode);

	return EXIT_SUCCESS;
}

/**
*
* \check_parms
*
* \This function compares the arguments entered with the set parms.
* \If returned unsuccessful, usage is printed.
*
* \param argc given numbers of arguments
* \param argv given arguments
*/
static void check_parms(const int argc, char* argv[]) {

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
			fprintf(stderr, "Usage: %s -m Too many arguments\n", argv[0]);
			if (errno != 0) {
				fprintf(stderr, "Errno: %s \n", strerror(errno));
			}
			exit(EXIT_FAILURE);
		}
		if (option == 'm') {
			smsize = strtol(optarg, &endptr, 10);


			if (*endptr != '\0') {
				fprintf(stderr, "Usage: %s -m Characters found in buffersize\n", argv[0]);
				if (errno != 0) {
					fprintf(stderr, "Errno: %s \n", strerror(errno));
				}
				exit(EXIT_FAILURE);
			}

			if (smsize <= 0 || (unsigned) smsize > SIZE_MAX / sizeof(int)) {
				fprintf(stderr, "Usage: %s -m Buffersize out of range\n", argv[0]);
				if (errno != 0) {
					fprintf(stderr, "Errno: %s \n", strerror(errno));
				}
				exit(EXIT_FAILURE);
			}
		}
		else {
			usage = 1;
		}
	}
	if (optind != argc || usage != 0 || errno != 0) {
		fprintf(stderr, "Usage: %s -m <buffersize>\n", argv[0]);

		if (errno != 0) {
			fprintf(stderr, "Errno: %s \n", strerror(errno));
		}
		exit(EXIT_FAILURE);
	}
}

/**
*
* \init_semaphore
*
* \The seminit() function creates a new semaphore with the given key and permissions as specified in semperm,
* \and initializes it with the value of initval. The function fails if a semaphore associated with key already exists.
* \The semgrab() function obtains the semaphore associated with the given key.
* \The function fails if a semaphore associated with key has not been created yet.
*
* \param const int key = userid
* \param const int mode = SENDER or EMPFAENGER
*
* \retval EXIT_SUCCESS if successful
* \retval EXIT_FAILURE if erroneous
*/
static int init_semaphore(const int key, const int mode) {

	int semid = -1;
	int init_buffer = 0;
	errno = 0;

	if (mode == SENDER) {
		init_buffer = smsize;
	}

	semid = seminit(key, 0660, init_buffer);


	if (errno == EEXIST) {
		semid = semgrab(key);
		errno = 0;
	}

	if (semid == -1) {
		errorhandling(mode, "Cannot grab Semaphore");
	}

	if (mode == SENDER) {
		sem2id = semid;
	}
	else if (mode == EMPFAENGER) {
		sem1id = semid;
	}

	return EXIT_SUCCESS;
}

/**
*
* \block
*
* \The P() function attempts to decrement the value of the semaphore identified by semid atomically by one.
* \If the value of the semaphore is already zero, P() blocks until the value becomes greater than zero.
*
* \param const int mode = SENDER or EMPFAENGER
*
* \retval EXIT_SUCCESS if successful
* \retval EXIT_FAILURE if erroneous
*/

static int block(const int mode) {

	int sem_id = sem2id;

	if (mode == EMPFAENGER) {
		sem_id = sem1id;
	}

	while (P(sem_id) == -1) {
		if (errno == EINTR) {
			/* syscall interrupted by signal, try again */
			continue;
		}
		else {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/**
*
* \unblock
*
* \The V() function increments the value of the semaphore identified by semid atomically by one.
* \If a process is waiting on the semaphore, this process is unblocked
*
* \param const int mode = SENDER or EMPFAENGER
*
* \retval EXIT_SUCCESS if successful
* \retval EXIT_FAILURE if erroneous
*/
static int unblock(const int mode) {

	int sem_id = sem1id;

	if (mode == EMPFAENGER) {
		sem_id = sem2id;
	}

	while (V(sem_id) == -1) {
		if (errno == EINTR) {
			/* syscall interrupted by signal, try again */
			continue;
		}
		else {
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

/**
*
* \init_sharedmem
*
* \shmget - allocates a shared memory segment.
*
* \param const int key = userid
*
* \retval EXIT_SUCCESS if successful
* \retval EXIT_FAILURE if erroneous
*/
static int init_sharedmem(const int key) {

	if ((shmid = shmget(key, (sizeof(int) * smsize), 0660 | IPC_CREAT)) == -1) {
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/**
*
* \errorhandling
*
* \This function prints error messages and call the cleanup function.
*
* \param const int mode = SENDER or EMPFAENGER
* \param const char *error_message = Error String
*/
static void errorhandling(const int mode, const char *error_message) {

	fprintf(stderr, "ERROR: %c \n", *error_message);

	if (errno != 0) {
		fprintf(stderr, "Errno: %s \n", strerror(errno));
	}

	cleanup(mode);

	exit(EXIT_FAILURE);
}

/**
*
* \cleanup
* \The shmdt() function detaches the shared memory segment located at the address specified by the calling process.
* \The semrm() function removes the semaphore identified by the given semid.
* \It fails if a semaphore with the given ID does not exist.
* \The shmctl() function performs the control operationIPC_RMID  Mark the segment to be destroyed.
* \The segment will actually be destroyed only after the last process detaches it.
*
* \param const int mode = SENDER or EMPFAENGER
*/
static void cleanup(const int mode) {

	if (shmdt(shmptr) == -1) {
		errorhandling(mode, "Cannot detache Shared-Memory");
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
			errorhandling(mode, "Cannot remove Shared-Memory");
		}
		shmid = -1;
	}
	if (fflush(stdout) == EOF) {
		fprintf(stderr, "Cannot flushing stdout");
	}
}

/*
* ------------------------------------------------------------------- eof --
*/