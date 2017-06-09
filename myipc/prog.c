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
/*
* ------------------------------------------------------------- functions --
*/

static int check_parms(const int argc, char* argv[]);




int run(const int argc, char* argv[], const int mode)
{

	if (mode == SENDER)
	{
		printf("run SENDER:%d\n", mode);
	}
	if (mode == EMPFAENGER)
	{
		printf("run EMPFAENGER:%d\n", mode);
	}
	
	/*
	int i;

		for (i = 0; i < argc; i++) 
			{
			printf("argv[%d] = %s ", i, argv[i]);
			printf("\n");
			}

		
		*/
		check_parms(argc,argv);
		
		
		printf("Buffer size: %li\n", buffer_size);
	return 0;
}


static int check_parms(const int argc, char* argv[])
{												
	int option = 0;
	char* endptr = NULL;

	if (argc < 2) {
	fprintf(stderr, "Usage: -m <buffersize>\n");
	exit(EXIT_FAILURE);
	}
	
	while ((option = getopt(argc, argv, "m:")) != -1) {
		
		errno = 0;

		if (optind < argc) {
			fprintf(stderr, "Usage: -m Too many arguments\n");
			exit(EXIT_FAILURE);
		}
		if (option == 'm') {

			buffer_size = strtol(optarg, &endptr, 10);

			if ((errno == ERANGE && (buffer_size == LONG_MAX || buffer_size == LONG_MIN)) || (errno != 0 && buffer_size == 0)) {
				fprintf(stderr, "Usage: -m Buffersize out of range\n");
				exit(EXIT_FAILURE);			
			}
			if (*endptr != '\0') {
				fprintf(stderr, "Usage: -m Characters found in buffersize\n");
				exit(EXIT_FAILURE);
			}
			if (buffer_size <= 0) {
				fprintf(stderr, "Usage: -m Buffersize to small\n");
				exit(EXIT_FAILURE);
			}
		}
		else {
			fprintf(stderr, "Usage: -m <buffersize>\n");
			exit(EXIT_FAILURE);	
		}
	}
	return 0;
}


/*
* ------------------------------------------------------------------- eof --
*/