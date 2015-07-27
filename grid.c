/* Grid - Game of Live program
uses PI to split the work

Wolfgang Huettinger
StudentID 0685854
whuettin@uoguelph.ca

*/

#include <stdio.h>
#include <stdlib.h>
#include "pilot.h"

//new stuff by myself
#include <string.h> // needed for palindrome testing of the strings


// no. of Workers
#define W 5
PI_PROCESS *Worker[W];		// array of process pointers
PI_CHANNEL *toWorker[W];	// array of channel pointers to Workers
//PI_CHANNEL *result[W];		// array of channel pointers from Workers
PI_CHANNEL *lineNumberToWorker[W];     // line number to the worker
PI_CHANNEL *lineNumberFromWorker[W];   // line number from the worker back
PI_CHANNEL *stringToWorker[W];         // string which should be tested by the workers

PI_BUNDLE *bundleLineNumber, *toAllWorkers;


// size 60 should be sufficient - I don't know of any word with 60 characters
#define sizeString 255

// function to reverse the string for comparison later on
char *reverseString(char *str, int length)
{
   // we need temp pointers to the beginning and
   // end of the sequence we wish to reverse
   char *start = str, *end = str + length - 1;

   char strorg[sizeString];
   strcpy(strorg,str);

   while(start < end)
   {
     // swap characters, and move pointers towards
     // the middle of the sequence
     char temp = *start;
     *start++ = *end;
     *end-- = temp;
   }
   // return the reversed string
   return str;
}


// the palindrome detectur function
// compares the now two strings - original and reversed
int palinDetector(char string[sizeString], int lineNumber)
{
   char strsrc[sizeString];
   char strtmp[sizeString];
   char strrev[sizeString];

   strcpy(strsrc,string);
   strcpy(strtmp,strsrc);
   reverseString(strtmp,strlen(strsrc));

   strcpy(strrev,strtmp);
  // strsrc = "test";

//printf("I am working on String %c \n",strsrc[0]);

   // the palintest isself
   if (strcmp(strsrc,strrev)==0)
   {
     //printf("Entered string %s in line number %d is a palindrome \n",strsrc,lineNumber);
     return lineNumber; // return the linennumber and then test to 0 so we find out that we have a palindrome in this line
   }
   else
   {
     return 0;
     //printf("Entered string %s is not palindrome \n",strsrc);
   };
   //return 0;
}

// this is the node the slaves build of - doing the test - distributed
int slaveNode(int index, void* arg2 )
{
    int i, workers, size, myshare, mystart;
    //int lineNumberlocal;
    //int lineNumber = 0;
    //char string[sizeString];
    //int palinFound = 0;
    //char string[4] = "noon";

    //PI_Read(lineNumberToWorker[index],"%d",&lineNumber);
    //PI_Read(stringToWorker[index],"%256c",&string);

    PI_Read( toWorker[index], "%d %d", &workers, &size );

    // figure out myshare
	myshare = size / workers;
	mystart = index * myshare;
	if ( index == workers-1 ) myshare += size%workers;

    // now we can define the data structure to store all the data in the memory
    //char **storeStrings = (char **)malloc(sizeString * sizeof(char *));
    //for(i = 0; i < sizeString; i++) storeStrings[i] = (char *)malloc(size * sizeof(char));

	//PI_Read( toWorker[index], "%d %m", lineNumber, storeStrings );

	// add up my share and report sum
	for ( i=mystart; i<(myshare+mystart); i++ )
	{
       //sum += buff[mystart + i];
	  // palinFound = palinDetector(storeStrings[i], lineNumber);
       //printf("slave: linenumber is %d \n", lineNumber);
       //printf("slave: palinFound is %d \n", palinFound[i]);
       //printf("slave: string is %s \n", string);
       //PI_Write(lineNumberFromWorker[index],"%d",palinFound);
	};





	//PI_Write( result[index], "%d", sum );


    //string = "noon";
    //lineNumberlocal = int(lineNumber);
//printf("I am working on String %c \n", string);
    // call of the palindrome detector to identify palin and then write it out
/*
    if (palinDetector(string, lineNumber) > 0)
    {
      // this lineNumber has a palindrome so we sent it back to the master
      PI_Write(lineNumberFromWorker[index],"%d",lineNumber);
    }
    else
    {
      PI_Write(lineNumberFromWorker[index],"%d",0);
    };
*/
   //palinFound = palinDetector(string, lineNumber);

  //printf("slave: linenumber is %d \n", lineNumber);
  //printf("slave: palinFound is %d \n", palinFound);
//printf("slave: string is %s \n", string);

   //PI_Write(lineNumberFromWorker[index],"%d",palinFound);
   //printf("Program finished");

   //free(storeStrings);
   return 0;
}

// this is the master node which controls the process
// it reads the file and hands out the workload to the slaves
int masterNode(void)
{

   // file open part
   char string[sizeString] = {0};  /* initialized to zeroes */
   //char *storeStrings;
   int i;
   int j;
   int k;
   int rc;
   //int linecounter = 0;
   int lineNumber = 0;
   int lineNumberInput = 0; /* initialized to zeroes */
   int endOfFile = 0;



//now distibute the data to the nodes and let them do the work

   //broadcast the information so the slaves can generate their data storage
   PI_Broadcast( toAllWorkers, "%d %d", W, lineNumber );

   //broadcast the work to the slave
   PI_Broadcast( toAllWorkers, "%d %m", lineNumber, storeStrings );








/*
// send data
	   PI_Write( lineNumberToWorker[j], "%d", lineNumber);
       PI_Write( stringToWorker[j], "%256c", string );

       // clear out the cache for the next bunch of linenumbers
       if (j == W-1)
       {

         //PI_Gather(bundleLineNumber, "%6d", lineNumberInput);
         for (k=0;k<W;k++)
         {
           //PI_Read( lineNumberFromWorker[k], "%d", &lineNumberInput);
       int w = PI_Select(bundleLineNumber);
       PI_Read(lineNumberFromWorker[w], "%6d", &lineNumberInput);

           if (lineNumberInput > 0)
           {
             printf("Palindrome found in line %d \n",lineNumberInput);
           };
         };
       };

     j++; // increment the index of the workers
     }; //end of the while loop

     //get the last values too - which are not taken into account before
     if (j < W-1)
     {
       //PI_Gather(bundleLineNumber, "%6d", lineNumberInput);
        //int w = PI_Select( allResults );
		//PI_Read( result[w], "%d", &sum );


       for (k=0;k<j;k++)
       {
         //PI_Read( lineNumberFromWorker[k], "%d", &lineNumberInput);
       int w = PI_Select(bundleLineNumber);
       PI_Read(lineNumberFromWorker[w], "%6d", &lineNumberInput);

         if (lineNumberInput > 0)
         {
           printf("Palindrome found in line %d \n",lineNumberInput);
         };
       };
     };
   printf("current linenumber is %d",lineNumber);
   printf("current j is %d", j);
   }
   */

   free(storeStrings);
   return 0;
}


/*
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
*/

int main( int argc, char *argv[] )
{
	int i;



	// Pilot configuration phase; return no. of processes available

	PI_Configure( &argc, &argv );

	// create Worker processes (with arg i) and to/from channels
   for ( i=0; i<W; i++ )
   {
	 Worker[i] = PI_CreateProcess( slaveNode, i, NULL );
     toWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
	 //result[i] = PI_CreateChannel( Worker[i], PI_MAIN );
	 lineNumberToWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
     lineNumberFromWorker[i] = PI_CreateChannel( Worker[i], PI_MAIN );
     stringToWorker[i] = PI_CreateChannel( PI_MAIN, Worker[i] );
   }
	// form "toworker" channels into bundle for broadcasting
	toAllWorkers = PI_CreateBundle( PI_BROADCAST, toWorker, W );

   //bundleLineNumber = PI_CreateBundle( PI_SELECT, lineNumberFromWorker, W);
   //bundleLineNumber = PI_CreateBundle( PI_GATHER, lineNumberFromWorker, W);
	// form "result" channels into bundle for selecting
	bundleLineNumber = PI_CreateBundle( PI_SELECT, lineNumberFromWorker, W );
	// form "toworker" channels into bundle for broadcasting
	//toAllWorkers = PI_CreateBundle( PI_BROADCAST, toWorker, W );

	// form "result" channels into bundle for selecting
	//allResults = PI_CreateBundle( PI_SELECT, result, W );


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

    masterNode();

/*
	int numbers[NUM];	// each element is 0..999

	for ( i=0; i<NUM; i++ ) {
		numbers[i] = (double)rand() * 999.0 / RAND_MAX;
		if ( i < 10 ) printf( " %d", numbers[i] );	// show some
	}
	printf( "\n" );


        //PI_Broadcast( toAllWorkers, "%*d", NUM, numbers );



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
*/


	PI_StopMain(0);

    printf("Program finished \n");

	return 0;
}
