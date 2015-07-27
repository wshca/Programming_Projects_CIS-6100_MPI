/* Pilot tutorial - lab2.c

AS GIVEN
	- demonstrates master/worker pattern
	- observe the PI_Write/Read formats for arrays
	- run with 6 processes: "mpirun -np 6 lab2" and observe the division of
	  labour
MODIFY
1)	- cause a deadlock: comment out the 2nd PI_Read in Worker, and run
	- why does program hang? (use ^C to interrupt)
	- run again with "-np 7 lab2 -pisvc=d" deadlock detection, and observe printout

2)	- put back PI_Read, then comment out PI_Read in PI_MAIN; run without
	  -pisvc=d; program exits normally after printing bogus result; why?
	- run again with "-pisvc=d" deadlock detection, and observe printout

3)	- change no. of workers to take up all available (N-1) processes; so
	  the "#define W" will change to an "int W" variable. Why N-1 and not N?
	- this needs dynamic allocation of pointer arrays:
	  PI_PROCESS **Worker;
	  PI_CHANNEL **toWorker;
	  W = N-1;
	  Worker = malloc( W*sizeof(PI_PROCESS*) ); and so on
	- now try running with increasing -np # and observe the work division
	
4)	- another error case: comment out the 1st PI_Read in Worker, and run
	  with or without -pisvc=d
	- this error is reported from MPI via Pilot; can you tell what it is
	  complaining about? Hint: it gives you a line no. in lab2.c
*/

#include <stdio.h>
#include <stdlib.h>
#include "pilot.h"

// no. of Workers
#define W 5
PI_PROCESS *Worker[W];		// array of process pointers
PI_CHANNEL *toWorker[W];	// array of channel pointers to Workers
PI_CHANNEL *result[W];		// array of channel pointers from Workers


// no. of numbers to add up
#define NUM 10000


// process function: arg2 is really char*

int workerFunc( int index, void* arg2 )
{
	int i, myshare, sum=0, *buff;
	
	PI_Read( toWorker[index], "%d", &myshare );
	printf( "Worker #%d signing on to do share of %d!\n", index, myshare );
	
	buff = malloc( myshare * sizeof(int) );
	PI_Read( toWorker[index], "%*d", myshare, buff );
	
	// add up my share and report sum
	for ( i=0; i<myshare; i++ )
		sum += buff[i];
	
	free( buff );
	PI_Write( result[index], "%d", sum );
	return 0;		// exit process function
}

int main( int argc, char *argv[] )
{
	int i;
	
	// Pilot configuration phase; return no. of processes available
	
	int N = PI_Configure( &argc, &argv );
	
	// create Worker processes (with arg i) and to/from channels
	for ( i=0; i<W; i++ ) {
		Worker[i] = PI_CreateProcess( workerFunc, i, NULL );
		toWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
		result[i] = PI_CreateChannel( Worker[i], PI_MAIN );
	}
	
	
	// start execution (helloFunc gets control on its processor)
	
	PI_StartAll();
	
	// PI_MAIN continues
	
	/* Here's the work to be done:  We start with an array of 10,000
	   random numbers (they're pseudo-random, so it will be the same
	   array everytime you run the program).  Each worker will be sent 1/W
	   of the numbers to add up, and report the sum.  PI_MAIN adds the
	   sums to give the grand total.  NOTE: This is not intended as an
	   "efficient model" of parallel programming, just as a simple demo.
	*/
	
	int numbers[NUM];	// each element is 0..999
	
	for ( i=0; i<NUM; i++ ) {
		numbers[i] = (double)rand() * 999.0 / RAND_MAX;
		if ( i < 10 ) printf( " %d", numbers[i] );	// show some
	}
	printf( "\n" );
	
	// distribute the work
	
	for ( i=0; i<W; i++ ) {
		
		// last portion may be larger
		int portion = NUM/W;
		if ( i == W-1 ) portion += NUM%W;
		
		// send portion size first, so worker can allocate buffer
		PI_Write( toWorker[i], "%d", portion );
		PI_Write( toWorker[i], "%*d", portion, &numbers[i*(NUM/W)] );
	}
	
	// collect the results
	
	int sum, total = 0;
	for ( i=0; i<W; i++ ) {
		
		PI_Read( result[i], "%d", &sum );
		printf( "Worker #%d reports sum = %d\n", i, sum );
		total += sum;
	}
	
	printf( "Grand total = %d\n", total );
	// end program
	
	PI_StopMain(0);
	return 0;
}
