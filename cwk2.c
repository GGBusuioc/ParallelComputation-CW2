//
// Starting code for the MPI coursework.
//
// Compile with:
//
// mpicc -Wall -o cwk1 cwk1.c
//
// or use the provided makefile.
//


//
// Includes
//
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

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
	// Step 1
	// I belive both versions work

	// int p;
	// if( rank==0 )
	// {
	// 	for( p=1; p<numProcs; p++ )
	// 	{
	// 		MPI_Send( &pixelsPerProc, 1, MPI_INT, p, 0, MPI_COMM_WORLD );
	// 	}
	// }
	// else
	// {
	// 	MPI_Recv( &pixelsPerProc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	// }

	MPI_Bcast(&pixelsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&maxValue, 1, MPI_INT, 0, MPI_COMM_WORLD);



	//
	// Step 2. Distribute the global array to the local arrays on all processes.
	//


	int *recvData = (int*) malloc( sizeof(int)*pixelsPerProc );



	MPI_Scatter(
		image, pixelsPerProc, MPI_INT, // Sent from
		recvData, pixelsPerProc, MPI_INT, // Received to
		0, MPI_COMM_WORLD 						// Source rank 0
	);



	// int *array = (int*) malloc( (maxValue+1)*sizeof(int) );
	//
	// if( !array ) return allocateFail( "local array", rank );
	//
	// for( i=0; i<maxValue+1; i++ ) array[i] = 0;
	//printf("Max value is %d", maxValue);
	//int array[maxValue];
	int *array = (int*) malloc( (maxValue+1)*sizeof(int) );

	for( i=0; i<maxValue+1; i++ ) array[i] = 0;



	for( i=0; i<pixelsPerProc; i++ )
		array[recvData[i]] = array[recvData[i]] + 1;

	// Uncomment to see the computation performed on all 4 processes
	//
	for( i=0; i<maxValue+1; i++ )
		printf("Rank %d - Index %d : %d\n",rank, i, array[i]);


	// int ceva[maxValue];
	// for( i=0; i<maxValue+1; i++ ) ceva[i] = 0;

	//Step 4. Send all of the local counts back to rank 0, which calculates the total.

	MPI_Reduce (&array, combinedHist ,1 , MPI_INT , MPI_SUM ,0 ,	MPI_COMM_WORLD ) ;


	//
	// Constructs the histogram in serial on rank 0. Can be used as part of a check that your parallel version works.
	//
	if( rank==0 )
	{


		// for( i=0; i<maxValue+1; i++ )
		// 	printf("index %d: %d\n", i, ceva[i]);

		// Allocate memory for the check histogram, and then initialise it to zero.
		int *checkHist = (int*) malloc( (maxValue+1)*sizeof(int) );
		if( !checkHist ) return allocateFail( "histogram for checking", rank );
		for( i=0; i<maxValue+1; i++ ) checkHist[i] = 0;

		// Construct the histogram using the global data only.
		for( i=0; i<dataSize; i++ )
			if( image[i]>=0 ) checkHist[image[i]]++;

		// Display the histgram.
		for( i=0; i<maxValue+1; i++ )
			printf( "Greyscale value %i:\tCount %i\t(check: %i)\n", i, combinedHist[i], checkHist[i] );
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
