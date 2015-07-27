/* Game of Life Simulation program
uses PI to split the work

Wolfgang Huettinger
StudentID 0685854
whuettin@uoguelph.ca

*/

#include <stdio.h>
#include <stdlib.h>
#include "pilot.h"

//new stuff by myself
#include <string.h> // testing of the strings


// no. of Workers
#define W 10

PI_PROCESS *Master;                    // master node
PI_PROCESS *Worker[W];		            // array of process pointers
PI_CHANNEL *MessageOut[W];             // send the needed data out to the neighbor
PI_CHANNEL *MessageIn[W];              // receive the needed data from the neighbor
PI_CHANNEL *stopSlaveLoop[W];
PI_CHANNEL *startCalculation[W];
PI_CHANNEL *firstRunSlave[W];
PI_CHANNEL *readDataIn[W];
PI_CHANNEL *readDataOnceSlave[W];
PI_CHANNEL *writeDataOut[W];
PI_CHANNEL *nextCycle[W];             // used to tell the slaves that the next cycle is to be calculated
PI_CHANNEL *dataExchangeOut[W];
PI_CHANNEL *dataExchangeIn[W];
PI_CHANNEL *finishCalc[W];

// number of interation the program should run
#define numberInteration 600000

// grid size
#define gridX 100 // if you change this value you have to change the %200d values, too
#define gridY 100



// life or death decision module
int modificationRule(int upperNbr, int lowerNbr, int leftNbr, int rightNbr)
{
  // all neighbors are present so the cell is dead
  if (upperNbr == 1 & lowerNbr == 1 & leftNbr == 1 & rightNbr == 1) return 0;

  // one neighbor is empty but all other three are present so the cell is dead
  if (upperNbr == 0 & lowerNbr == 1 & leftNbr == 1 & rightNbr == 1) return 0;
  if (upperNbr == 1 & lowerNbr == 0 & leftNbr == 1 & rightNbr == 1) return 0;
  if (upperNbr == 1 & lowerNbr == 1 & leftNbr == 0 & rightNbr == 1) return 0;
  if (upperNbr == 1 & lowerNbr == 1 & leftNbr == 1 & rightNbr == 0) return 0;

  // two neighbors are empty and two are present so the cell lifes
  if (upperNbr == 1 & lowerNbr == 1 & leftNbr == 0 & rightNbr == 0) return 1;
  if (upperNbr == 0 & lowerNbr == 0 & leftNbr == 1 & rightNbr == 1) return 1;
  if (upperNbr == 1 & lowerNbr == 0 & leftNbr == 1 & rightNbr == 0) return 1;
  if (upperNbr == 0 & lowerNbr == 1 & leftNbr == 0 & rightNbr == 1) return 1;

  // three neighbors are empty and one is present so the cell lifes
  if (upperNbr == 1 & lowerNbr == 0 & leftNbr == 0 & rightNbr == 0) return 1;
  if (upperNbr == 0 & lowerNbr == 1 & leftNbr == 0 & rightNbr == 0) return 1;
  if (upperNbr == 0 & lowerNbr == 0 & leftNbr == 1 & rightNbr == 0) return 1;
  if (upperNbr == 0 & lowerNbr == 0 & leftNbr == 0 & rightNbr == 1) return 1;

  // all neightbors are empty so there is no mother to generate the cell
  if (upperNbr == 0 & lowerNbr == 0 & leftNbr == 0 & rightNbr == 0) return 0;

  return 0;
}


// random number generator for the initialization of the field
int randomgenerator(int reseed)
{
  double rnd;
  double i;
  double j;
  int sendback;
  //Initialize the random number generator with the seed
  srand((unsigned)time(NULL)*reseed);

  // Call the C routine rand() for a random number
  // hand out the random number to the calling routine

  i = rand();
  j = RAND_MAX;
  rnd = i/j;

  if (rnd < 0.5)
  {
      sendback = 0;
  }
  else
  {
      sendback = 1;
  };

  //printf("rand fnct is %f \n",i);
  //printf("RAND_Max is %f \n",j);
  //printf("rnd is %d \n",sendback);
  return sendback;
}


// this is the node the slaves build of - doing the test - distributed
int slaveNode(int index, void* arg2 )
{
    //local variables
    int gridSize = gridX*gridY/W+2;
    // working grid were the edge are predefined or filled by the neighboring slave
    int partedGrid[gridSize];

    //char string[sizeString];
    //int palinFound = 0;
    int i;
    int j;
    int k;
    int l;
    int stopSlave = 0;       //preinitialization is important and used
    //int stopCalculation = 0; //preinitialization is important and used
    int startCalc = 0;
    int readDataInOnce = 0;
    int nextCycleCalc = 0;
    int initGrid = 0;
    int dataExchangeOutInput = 0;
    int dataExchangeInInput = 0;
    int partedGridIn[2*gridX] = {0};
    int partedGridOut[2*gridX] = {0};


    // start the forever loop
    while (stopSlave == 0)
    {
      if( PI_ChannelHasData(nextCycle[index]) == 1)
      {
        // get signal to do the next calculation step
        PI_Read(nextCycle[index],"%d",&nextCycleCalc);
      };

      // initialize the local grid with random data once
      if(initGrid == 0)
      {
        for(i=0;i<gridSize;i++)
        {
          partedGrid[i] = randomgenerator(i);
        };
        // reset the value so we only do it once
        initGrid = 1;
      };


      if(dataExchangeOutInput == 1)
      {
        // generate partedGridOut
        for (i=0;i<gridX;i++)
        {
          partedGridOut[i] = partedGrid[i]; //upper values
          partedGridOut[i+gridX] = partedGrid[i+gridSize-gridX]; //lower values
        };

        // write out the data to the neighboring slaves
        PI_Write(MessageOut[index], "%200d", partedGridOut);

        // reset the variable so we only do it once per cycle time
        dataExchangeOutInput = 0;
      };

      if(dataExchangeInInput == 1)
      {
      if( PI_ChannelHasData(MessageIn[index]) == 1)
        {
        // read in the data from the neighboring slaves
        PI_Read(MessageIn[index], "%200d", partedGridIn);
        };
        // reset the variable so we only do it once per cycle time
        dataExchangeInInput = 0;

        // overwrite the grid with input data
        for (i=0;i<gridX;i++)
        {
          partedGrid[i] = partedGridIn[i];
          partedGrid[i+gridSize-gridX] = partedGridIn[i+gridX];
        };
      };

      // do actuall calculations in this loop once for every time step
      if(nextCycleCalc == 1)
      {

      // so we calcultate the cells
      for(i=gridX;i<gridSize-gridX;i++)
      {
        partedGrid[i] = modificationRule(partedGrid[i-gridX],partedGrid[i+gridX],partedGrid[i-1],partedGrid[i+1]);
      };

      PI_Write(finishCalc[index], "%d", 1);

      nextCycleCalc = 2; // make sure we only do it once to avoid problems
      };

      // for the data exchange
      if( PI_ChannelHasData(dataExchangeOut[index]) == 1)
      {
        PI_Read(dataExchangeOut[index],"%d",&dataExchangeOutInput);
      };
      if( PI_ChannelHasData(dataExchangeIn[index]) == 1)
      {
        PI_Read(dataExchangeIn[index],"%d",&dataExchangeInInput);
      };

      // final end of the slave sub program is determined by this channel
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
   //char string[sizeString] = {0};  /* initialized to zeroes */
   int i;
   int j;
   int k;
   int rc;
   int t;
   int iterationIndex;
   int lineNumber = 0;
   int lineNumberInput = 0; /* initialized to zeroes */
   int endOfFile = 0;
   int finishCalcIn[W] = {0};
   int finishWriteData[W] = {0};
   //FILE *fileInput = fopen("n1000000_p75.inp", "rb");       // open file to read the palindrome for the test
   FILE *fileOutput = fopen("output.txt", "w"); // open file to write out the results

   //int wholeGrid[gridX*gridY]; (unneeded can be avoided to save memory
   int partedGrid[gridX*gridY/W+2];
   int partedGridOut[W][2*gridX];


   // datastrcutre is a matrix which is modelled as a list of integers 0 and 1 (can be modified
   // to a bigger grid and more dimensions as more states
   // it works as follows:
   //
   // index:     0 ... 1*(Worker_i+1)*gridY/W*gridX-1
   //   for the indeces for the Worker 0
   // index:     (Worker_i+1)*gridX ... 2*(Worker_i+1)*gridY/W*gridX-1
   //   for the indeces for the Worker 1
   // and so on up to
   // index:     (Worker_i+1)*gridX ... NumberOfWorker*(Worker_i+1)*gridY/W*gridX-1
   //
   // to my knowledge this is the most efficient way to store random data - there can be modification done
   // by change int to bool
   iterationIndex=0;
   i = 0;
   while (iterationIndex<numberInteration)
   {
     for (k=0;k<W;k++)
     {

       PI_Write( dataExchangeOut[k], "%d", 1);
       if( PI_ChannelHasData(MessageOut[k]) == 1)
       {
         PI_Read(MessageOut[k], "%200d", &partedGridOut[k]); //%200d -> the flexible size seems not to be working in pilot
         finishWriteData[k] = 1;
         i = i++;

         if (i == W-1)
         {
           iterationIndex++;
           i = 0;
         };

       };
     };
     t = 0;
     k = 0;

     while (k<W)
     {

       if (finishWriteData[k] == 1)
       {
           t = t++;
       };


       PI_Write( MessageIn[k], "%200d", partedGridOut[W-k]);
       PI_Write( dataExchangeIn[k], "%d", 1);
       PI_Write( nextCycle[k], "%d", 1);

       if( PI_ChannelHasData(finishCalc[k]) == 1)
       {
         PI_Read(finishCalc[k],"%d",&finishCalcIn);

         if (finishCalcIn[k] == 1)
         {
           i = i + 1;
           //finishCalcIn = 0;
         };

         if (i == W-1)
         {

             i = 0;
         };

//printf("i is %d \n", i);
       };
       k++;
     };




     //printf("now we are in year %d \n",iterationIndex);





   };




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
    long int timestart;
    long int timeend;
    timestart = time(NULL);
	// Pilot configuration phase; return no. of processes available
	PI_Configure( &argc, &argv );

    // create Master process to be more clean
    Master = PI_CreateProcess( masterNode, 0, NULL);

	// create Worker processes (with arg i) and to/from channels
    for ( i=0; i<W; i++ )
    {
	  Worker[i] = PI_CreateProcess( slaveNode, i, NULL );
      stopSlaveLoop[i] = PI_CreateChannel( Master, Worker[i]);
      nextCycle[i] = PI_CreateChannel( Master, Worker[i]);
      startCalculation[i] = PI_CreateChannel( Master, Worker[i]);
      firstRunSlave[i] = PI_CreateChannel( Master, Worker[i]);
      readDataIn[i] = PI_CreateChannel( Master, Worker[i]);
      readDataOnceSlave[i] = PI_CreateChannel( Master, Worker[i]);
      writeDataOut[i] = PI_CreateChannel( Worker[i], Master);
      finishCalc[i] = PI_CreateChannel( Worker[i], Master);
      dataExchangeOut[i] = PI_CreateChannel( Master, Worker[i]);
      dataExchangeIn[i] = PI_CreateChannel( Master, Worker[i]);
    };

    // create the connection channels - note the number is W-1 instead of W!
    for ( i=0; i<W; i++ )
    {
      MessageOut[i] = PI_CreateChannel( Worker[i], Master );
      MessageIn[i] = PI_CreateChannel( Master, Worker[i] );
    };

    // start execution (slaveNode gets control on its processor)
    PI_StartAll();

    // stop the execution
    PI_StopMain(0);

    // calculate the time used in this program run
    timeend = time(NULL);
    timestart = timeend - timestart;

    // send a small message that the program is finished to be nice
    printf("Elapsed time is %ld .\n", timestart);
    printf("Program executed and finished without errors.");

    return 0;
}
