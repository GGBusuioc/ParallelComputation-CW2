//
// Starting code for the MPI coursework.
//
// When using collective communications routines version
// Compile with:
//
// mpicc -Wall -o cwk1 cwk1.c
//
// or use the provided makefile.
//
// When using point-to-point communication in a binary tree
// Compile with:
//
// mpicc -Wall -lm -o cwk1 cwk1.c
//
// And uncomment the specified code situated at the end of the program


//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>


// Some extra routines for this coursework. DO NOT MODIFY OR REPLACE THESE ROUTINES,
// as this file will be replaced with a different version for assessment.
#include "cwk2_extras.h"


//
// Main
//
int main( int argc, char **argv )
{
	int i;

	// Initialise MPI and get the rank and no. of processes.
	int rank, numProcs;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &numProcs );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Read in the image file to rank 0.
	int *image = NULL, maxValue, pixelsPerProc, dataSize;
	if( rank==0 )
	{
		// Read in the file and extract the maximum grey scale value and the data size (including padding bytes).
		// Defined in cwk2_extras.h; do not change, although feel free to inspect.
		image = readImage( "image.pgm", &maxValue, &dataSize, numProcs );

		if( image==NULL )
		{
			MPI_Finalize();
			return EXIT_FAILURE;
		}

		// The image size has already been rounded up to a multiple of numProcs by "readImage()".
		pixelsPerProc = dataSize / numProcs;


		printf( "Rank 0: Read in PGM image array of size %d (%d per process), with max value %d.\n", dataSize, pixelsPerProc, maxValue );
	}

	// Allocate memory for the final histogram on rank 0.
	int *combinedHist = NULL;
 	if( rank==0 )
 	{
 		combinedHist = (int*) malloc( (maxValue+1)*sizeof(int) );
 		if( !combinedHist ) return allocateFail( "global histogram", rank );

 		for( i=0; i<maxValue+1; i++ ) combinedHist[i] = 0;
 	}

	//
	// Your parallel code goes here. Feel free to change anything else in this file,
	// EXCEPT calls to the routines in cwk2_extras.h which MUST be called as provided.
	//

	//
	// Let all the process know about the value of pixelsPerProc and maxValue
	//
	MPI_Bcast(&pixelsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxValue, 1, MPI_INT, 0, MPI_COMM_WORLD);


	//
	// Distribute the global array to the local arrays on all processes.
	//

	int *recvData = (int*) malloc( sizeof(int)*pixelsPerProc );
	MPI_Scatter(
		image, pixelsPerProc, MPI_INT, // Sent from
		recvData, pixelsPerProc, MPI_INT, // Received to
		0, MPI_COMM_WORLD 						// Source rank 0
	);

	// Create a local array for each process
	int *array = (int*) malloc( (maxValue+1)*sizeof(int) );
	// Initialise the local array with zeroes
	for( i=0; i<maxValue+1; i++ ) array[i] = 0;

	// Perform the pixel counting on each array
	for( i=0; i<pixelsPerProc; i++ )
		array[recvData[i]] = array[recvData[i]] + 1;


	//
	// Perform Reduction
	//

	MPI_Reduce (array, combinedHist ,maxValue+1 , MPI_INT , MPI_SUM ,0 ,	MPI_COMM_WORLD ) ;

	//
	// Reducing the local histograms to the global one using point-to-point communication in a binary tree (First example from the lecture)
	//

	// MPI_Status status;
	//
	// // Create a remote array to hold the received array when receiving
	// int *remoteArray = (int*) malloc( (maxValue+1)*sizeof(int) );
	//
	// // Initialise remoteArray with zeroes
	// for( i=0; i<maxValue+1; i++ ) remoteArray[i] = 0;
	//
  // // Initial level
	// int lev = 1;
	//
	// int power_level, power_level_minus;
	//
	// // Compute  2 * (first level - 1)
	// power_level_minus = pow(2,lev-1);
	//
	// // Compute  2 * (first level )
	// power_level = pow(2,lev);
	//
	// // While level <= log(numProcs)
	// while(1<<lev<=numProcs)
	// 	{
	// 		// If on the first level
	// 		if(lev==1)
	// 		{
	// 			if(rank%2==0)
	// 			{
	// 			MPI_Recv(remoteArray, maxValue+1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status);
	// 			for(i=0;i<maxValue+1;i++)
	// 				array[i] += remoteArray[i];
	// 			}
	// 			else
	// 			{
	// 			MPI_Send(array, maxValue+1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
	// 			}
	// 		}
	// 		// Not on the first level and the logic for receiving and sending changes
	// 		else
	// 		{
	// 			// If the current rank is a receiver
	// 			if(rank%power_level==0)
	// 			{
	// 				MPI_Recv(remoteArray, maxValue+1, MPI_INT, rank+power_level_minus, 0, MPI_COMM_WORLD, &status);
	// 				for(i=0;i<maxValue+1;i++)
	// 					array[i] += remoteArray[i];
	// 			}
	// 			// It is not a receiver
	// 			else
	// 			{
	// 				// Check if it a sender
	// 				if(rank%power_level_minus==0)
	// 				{
	// 					MPI_Send(array,maxValue+1, MPI_INT, rank-power_level_minus, 0, MPI_COMM_WORLD);
	// 				}
	// 			}
	// 		}
	// 		lev++;
	// 		power_level_minus = pow(2,lev-1);
	// 		power_level = pow(2,lev);
	// 	}
	//
	// free(remoteArray);

	free( recvData );

	//
	// Constructs the histogram in serial on rank 0. Can be used as part of a check that your parallel version works.
	//
	if( rank==0)
	{
		// The final local array to reach rank 0 will have the final results

		//
		// Uncomment this when using point-to-point communication in a binary tree
		//
		// for( i=0; i<maxValue+1; i++ )
		// 	combinedHist[i] = array[i];
		// free(array);

		// Allocate memory for the check histogram, and then initialise it to zero.
		int *checkHist = (int*) malloc( (maxValue+1)*sizeof(int) );
		if( !checkHist ) return allocateFail( "histogram for checking", rank );
		for( i=0; i<maxValue+1; i++ ) checkHist[i] = 0;

		// Construct the histogram using the global data only.
		for( i=0; i<dataSize; i++ )
			if( image[i]>=0 ) checkHist[image[i]]++;

		//Display the histgram.
		for( i=0; i<maxValue+1; i++ )
			printf( "Greyscale value %i:\tCount %i\t(check: %i)\n",i, combinedHist[i], checkHist[i] );
		free( checkHist );
	}

	//
	// Clear up and quit.
	//

	if( rank==0 )
	{
		saveHist( combinedHist, maxValue );		// Defined in cwk2_extras.h; do not change or replace the call.
		free( image );
		free( combinedHist );
	}

	MPI_Finalize();
	return EXIT_SUCCESS;
}
