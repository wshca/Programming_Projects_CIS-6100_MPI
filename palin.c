/* Palandrome test program
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

PI_PROCESS *Master;                    // master node
PI_PROCESS *Worker[W];		            // array of process pointers
PI_CHANNEL *lineNumberToWorker[W];     // line number to the worker
PI_CHANNEL *lineNumberFromWorker[W];   // line number from the worker back
PI_CHANNEL *stringToWorker[W];         // string which should be tested by the workers
PI_CHANNEL *stopSlaveLoop[W];          // to end the loop the slaves are running all the the time
PI_CHANNEL *stopCalculationLoop[W];    // used to write out the data from the slave to master


// size 256 should be sufficient
#define sizeString 256

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


// the palindrome detector function
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

   // the palintest isself
   if (strcmp(strsrc,strrev)==0)
   {
     return lineNumber; // return the linennumber and then test to 0 so we find out that we have a palindrome in this line
   }
   else
   {
     return 0;
   };
}

// this is the node the slaves build of - doing the test - distributed
int slaveNode(int index, void* arg2 )
{
    //local variables preinitialized
    int lineNumber = 0;
    char string[sizeString];
    int palinFound = 0;
    int stopSlave = 0;       //preinitialization is important and used
    int stopCalculation = 0; //preinitialization is important and used

    // start the forever loop
    while (stopSlave == 0)
    {
      // read the linenumber into the slave for the palindetector
      if( PI_ChannelHasData(lineNumberToWorker[index]) == 1)
      {
        PI_Read(lineNumberToWorker[index],"%d",&lineNumber);
      };

      // read the string into the slave for the palindetector
      // and execute the palindrome detector
      if( PI_ChannelHasData(stringToWorker[index]) == 1)
      {
        PI_Read(stringToWorker[index],"%256c",&string);
        palinFound = palinDetector(string, lineNumber);
      };

      // trigger the signal so we write only once per calculation
      if(PI_ChannelHasData(stopCalculationLoop[index]) == 1)
      {
        PI_Read(stopCalculationLoop[index],"%d",&stopCalculation);
      };

      // now we write once out to the write function
      // this is IMPORTANT! because otherwise we end up in a deadlock
      if(stopCalculation == 1)
      {
        // send the result back to main
        PI_Write(lineNumberFromWorker[index],"%d",palinFound);

        // make sure we only write out once by changing the variable
        stopCalculation = 2;
      };

      // signal which trigers all slaves to stop so the program can finish
      if( PI_ChannelHasData(stopSlaveLoop[index]) == 1)
      {
        PI_Read(stopSlaveLoop[index],"%d",&stopSlave);
      };

    };//end while exterior

   return 0;
}


// this is the master node which controls the process
// it reads the file and hands out the workload to the slaves
int masterNode(int index, void* arg2)
{
   char string[sizeString] = {0};  /* initialized to zeroes */
   int i;
   int j;
   int k;
   int rc;
   int t;
   int lineNumber = 0;
   int lineNumberInput = 0; /* initialized to zeroes */
   int endOfFile = 0;
   FILE *fileInput = fopen("n1000000_p75.inp", "rb");       // open file to read the palindrome for the test
   FILE *fileOutput = fopen("n1000000_p75.out.palin", "w"); // open file to write out the results

   // check that we can open the output file
   if (fileOutput == NULL)
   {
     printf("Can not open output file!\n");
   };

   // file open part
   if (fileInput != NULL) /* If no error occurred while opening file */
   { /* input the data from the file. */

     j = 0; // set the index value of the workers to 0

     while(fgets(string, sizeof string, fileInput) != NULL)
     {
        /* strip trailing newline */
        for (i = 0; i < strlen(string); i++)
        {
           if ( string[i] == '\n' || string[i] == '\r' )
           {
             string[i] = '\0';
           }
        }
       lineNumber++; // incremement because we start with linecounting at one and not with zero

       if (j == W)
	   {
	      j = 0; // reset the loop index to 0 when we reach the last worker
	   };

       // send data out to the nodes
	   PI_Write(lineNumberToWorker[j],"%d",lineNumber);
       PI_Write(stringToWorker[j],"%256c",string);

       // read the data from the nodes all at once when they all were filled
       if(j == W-1)
       {
         for (k=0;k<W;k++)
         {
           // send signal so the nodes write out their results once in the channel
           PI_Write( stopCalculationLoop[k], "%d", 1);

           // here we do not have a non-locking read so the program waits until
           // the computation is finished in the nodes
           PI_Read( lineNumberFromWorker[k],"%d",&lineNumberInput);

           // write out the received data to the outputfile
           if(lineNumberInput>0)
           {
             //printf("%d\n", lineNumberInput);
             fprintf(fileOutput, "%d\n", lineNumberInput);
           };
         };
       };

       j++; // increment the index of the workers

     }; //end of the while loop

     // read the date from the nodes from the nodes that still have data to hand outs
     if(j < W-1)
     {
       for (k=0;k<j;k++)
       {
         // send signal so the nodes write out their results once in the channel
         PI_Write( stopCalculationLoop[k], "%d", 1);

         // here we do not have a non-locking read so the program waits until
         // the computation is finished in the nodes
         PI_Read( lineNumberFromWorker[k],"%d",&lineNumberInput);

         // write out the received data to the outputfile
         if(lineNumberInput>0)
         {
           //printf("%d\n", lineNumberInput);
           fprintf(fileOutput, "%d\n", lineNumberInput);
         };
       };
     };


   }
   else /* If error occurred, display message. */
   {
     printf("An error occurred while opening the file.\n");
   };

   // close the file to be tidy
   fclose(fileInput);
   fclose(fileOutput);

   // stop the slaves
   for (k=0;k<W;k++)
   {
     // send signal to slave to finish calculations so the program can finish
     PI_Write( stopSlaveLoop[k], "%d", 1);
   };

   return 0;
}


int main( int argc, char *argv[] )
{
	int i;

	// Pilot configuration phase; return no. of processes available
	PI_Configure( &argc, &argv );

    // create Master process to be more clean
    Master = PI_CreateProcess( masterNode, 0, NULL);

	// create Worker processes (with arg i) and to/from channels
    for ( i=0; i<W; i++ )
    {
	  Worker[i] = PI_CreateProcess( slaveNode, i, NULL );
      lineNumberToWorker[i] = PI_CreateChannel( Master, Worker[i] );
      stringToWorker[i] = PI_CreateChannel( Master, Worker[i] );
      stopSlaveLoop[i] = PI_CreateChannel( Master, Worker[i]);
      stopCalculationLoop[i] = PI_CreateChannel( Master, Worker[i]);
      lineNumberFromWorker[i] = PI_CreateChannel( Worker[i], Master);
    }

    // start execution (slaveNode gets control on its processor)
    PI_StartAll();

    // stop the execution
    PI_StopMain(0);

    // send a small message that the program is finished to be nice
    printf("Program executed and finished without errors.");

    return 0;
}
