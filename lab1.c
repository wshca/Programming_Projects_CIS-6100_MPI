/* Pilot tutorial - lab1.c

AS GIVEN
	- demonstrates typical skeleton program
	- run with 2 processes: "mpirun -np 2 lab1"
	- try running with excess processes and with 1 process, and observe
	  the printout
MODIFY
1)	- create another process Midway "between" PI_MAIN and Hello
	- redirect channel printThis to Midway, and create a second channel
	  for Midway to communicate with Hello
	- send the no. of processes to Midway, which adds 100, and
	  forwards the sum to Hello (which prints it)
	  
2)	- let Midway generate a floating point number and send that to
	  Hello along with processes+100, using a single PI_Write (modify the
	  formats).  Hello prints both numbers.

3)	- inject an error:  have PI_MAIN write on the wrong channel
	- try running and see what happens
	
*/

#include <stdio.h>
#include "pilot.h"


PI_PROCESS *Hello;		// process that prints Hello
PI_CHANNEL *printThis;		// what Hello should print


// process function: arg2 is used for char*

int helloFunc( int arg1, void* arg2 )
{
	int numproc;
	
	// obtain no. of processes
	
	PI_Read( printThis, "%d", &numproc );
	
	printf( "Hello, %s!  %d processes\n", (char*)arg2, numproc );
	return 0;		// exit process function
}

int main( int argc, char *argv[] )
{
	// Pilot configuration phase; return no. of processes available
	
	int N = PI_Configure( &argc, &argv );
	
	// pass an argument to Hello just to show we can do it
	Hello = PI_CreateProcess( helloFunc, 0, (void*)"World" );
	
	printThis = PI_CreateChannel( PI_MAIN, Hello ) ;
	
	
	// start execution (helloFunc gets control on its processor)
	
	PI_StartAll();
	
	// PI_MAIN continues: send no. of processes to be printed
	
	PI_Write( printThis, "%d", N );
	
	// end program
	
	PI_StopMain(0);
	return 0;
}
