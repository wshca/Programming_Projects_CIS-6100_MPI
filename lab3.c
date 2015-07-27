/* Pilot tutorial - lab3.c

AS GIVEN
	- demonstrates collective operations, including bundles
	- NOTE: this is back to the first lab2 version with fixed number of
	  workers (5); don't bother changing it to use dynamic allocation
	- run with 6 processes: "mpirun -np 6 lab3" and observe the same results
	  as lab2 (same grand total)
MODIFY
1)	- comment out PI_MAIN's for{PI_Read} loop, and replace it with a single
	  call to PI_Gather using the bundle "allResults"
	  - you need an int array for the results with W elements
	  - the same PI_Read format works for PI_Gather
	  - you still have to add the results in a loop
	  - up in the configuration phase, change the bundle usage to PI_GATHER
	  - nothing needs to change in the worker function
*/

#include <stdio.h>
#include <stdlib.h>
#include "pilot.h"

// no. of Workers
#define W 5
PI_PROCESS *Worker[W];		// array of process pointers
PI_CHANNEL *toWorker[W];	// array of channel pointers to Workers
PI_CHANNEL *result[W];		// array of channel pointers from Workers

PI_BUNDLE *toAllWorkers, *allResults;


// no. of numbers to add up
#define NUM 10000


// process function: arg2 is really char*

int workerFunc( int index, void* arg2 )
{
	int i, workers, size, myshare, mystart, sum=0, *buff;
	
	// get no. of workers and size of data set
	PI_Read( toWorker[index], "%d %d", &workers, &size );
	
	// figure out myshare
	myshare = size / workers;
	mystart = index * myshare;
	if ( index == workers-1 ) myshare += size%workers;
	
	printf( "Worker #%d signing on to do share of %d!\n", index, myshare );
	
	buff = malloc( size * sizeof(int) );
	PI_Read( toWorker[index], "%*d", size, buff );
	
	// add up my share and report sum
	for ( i=0; i<myshare; i++ )
		sum += buff[mystart + i];
	
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
	
	// form "toworker" channels into bundle for broadcasting
	toAllWorkers = PI_CreateBundle( PI_BROADCAST, toWorker, W );
	
	// form "result" channels into bundle for selecting
	allResults = PI_CreateBundle( PI_SELECT, result, W );
	
	
	// start execution (helloFunc gets control on its processor)
	
	PI_StartAll();
	
	// PI_MAIN continues
	
	/* We will solve the computation as in lab2, but here take a different
	   approach using collective operations.
	   
	   Instead of dividing the data set on PI_MAIN, we simply broadcast
	   the entire array to all the workers.  Based on their index numbers
	   and the size of the array, they can easily figure out which portion
	   to sum up.
	   
	   First, we'll collect the results using PI_Select, which allows
	   PI_MAIN to read from the workers as they finish, rather than in
	   strict numerical order.  (For this computation, they will each take
	   the same time, so it doesn't give a real advantage.)
	   
	   Second, we'll use PI_Gather to collect the results in a single
	   operation.
	*/
	
	int numbers[NUM];	// each element is 0..999
	
	for ( i=0; i<NUM; i++ ) {
		numbers[i] = (double)rand() * 999.0 / RAND_MAX;
		if ( i < 10 ) printf( " %d", numbers[i] );	// show some
	}
	printf( "\n" );
	
	// broadcast the work; send W and NUM first so workers can allocate buffer
	PI_Broadcast( toAllWorkers, "%d %d", W, NUM );
	PI_Broadcast( toAllWorkers, "%*d", NUM, numbers );
	
	// collect the results using selection
	int sum, total = 0;
	for ( i=0; i<W; i++ ) {
		
		// find out which worker is done next
		int w = PI_Select( allResults );
		PI_Read( result[w], "%d", &sum );
		printf( "Worker #%d reports sum = %d\n", w, sum );
		total += sum;
	}
	
	printf( "Grand total = %d\n", total );
	// end program
	
	PI_StopMain(0);
	return 0;
}
