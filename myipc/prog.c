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
#include <errno.h>



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

/*
* ------------------------------------------------------------- functions --
*/
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


	int i;

		for (i = 0; i < argc; i++) 
			{
			printf("argv[%d] = %s ", i, argv[i]);
			printf("\n");
			}



	return 0;
}


/*
* ------------------------------------------------------------------- eof --
*/