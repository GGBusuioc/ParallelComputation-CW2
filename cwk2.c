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

	int p;
	if( rank==0 )
	{
		for( p=1; p<numProcs; p++ )
		{
			MPI_Send( &pixelsPerProc, 1, MPI_INT, p, 0, MPI_COMM_WORLD );
		}
	}
	else
	{
		MPI_Recv( &pixelsPerProc, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	}

	//MPI_Bcast(&pixelsPerProc, 1, MPI_INT, 0, MPI_COMM_WORLD);


	//
	// Step 2. Distribute the global array to the local arrays on all processes.
	//
	// printf("Here is where it points %d !!!!!!!!!!£££", *image);
	// if( rank==0 )
	// {
	// 	for( p=1; p<numProcs; p++ )
	// 	{
	// 		MPI_Send( &image[p*pixelsPerProc], pixelsPerProc, MPI_INT, p, 0, MPI_COMM_WORLD );
	// 	}
	// }
	// else
	// {
	// 	MPI_Recv( combinedHist, pixelsPerProc, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	// }

	// if( rank==0 )
	// {
	// 	for (p=1; p<numProcs; p++)
	// 		MPI_Send( &image[p*pixelsPerProc], pixelsPerProc, MPI_INT, p, 0, MPI_COMM_WORLD);
	// }
	// MPI_Status status;
	// if( rank > 0)
	// {
	// 	MPI_Recv(combinedHist, pixelsPerProc, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
	// }
	// MPI_Scatter(
	// 	&image[p*pixelsPerProc], pixelsPerProc, MPI_INT,
	// 	combinedHist, pixelsPerProc, MPI_INT,
	// 	0, MPI_COMM_WORLD
	// );

	// Using staggered sends and receives


	int
		*recvData = (int*) malloc( sizeof(int)*pixelsPerProc );

	// Fill the sendData array with some numbers; for the purpose of this exercise it doesn't matter what,
	// although it helps debugging if each rank's array contains different values.

	// MPI_Status status;
	// if( rank % 2 )
	// {
	// 	printf("here is rank %d with size=%d\n" , rank , pixelsPerProc);
	//
	// 	MPI_Send( sendData, pixelsPerProc, MPI_INT, ( rank==numProcs-1 ? 0 : rank+1 ), 0, MPI_COMM_WORLD );
	// 	MPI_Recv( recvData, pixelsPerProc, MPI_INT, ( rank==0 ? numProcs-1 : rank-1 ), 0, MPI_COMM_WORLD, &status );
	//
	// }
	// else
	// {
	// 	printf("here is rank %d with size=%d\n" , rank , pixelsPerProc);
	//
	// 	MPI_Recv( recvData, pixelsPerProc, MPI_INT, ( rank==0 ? numProcs-1 : rank-1 ), 0, MPI_COMM_WORLD, &status );
	// 	MPI_Send( sendData, pixelsPerProc, MPI_INT, ( rank==numProcs-1 ? 0 : rank+1 ), 0, MPI_COMM_WORLD );
	//
	// }
	//
	// if( rank==0 )
	// 	for( i=0; i<(pixelsPerProc<20?pixelsPerProc:20); i++ ) printf( "%i\t%i\t%i\n", i, sendData[i],recvData[i] );
	//
	//

	MPI_Scatter(
		image, pixelsPerProc, MPI_INT, // Sent from
		recvData, pixelsPerProc, MPI_INT, // Received to
		0, MPI_COMM_WORLD 						// Source rank 0
	);

	// Check if data is sent and received corectly
	if( rank==0 )
		for( i=0; i<(pixelsPerProc<100?pixelsPerProc:100); i++ ) printf( "%i\t%i\t%i\n", i, image[i],recvData[i] );


	// Step 3
	int *count_a = NULL;
	count_a = (int*) malloc( (maxValue+1)*sizeof(int) );
	// for( i=0; i<maxValue+1; i++ ) count_a[i] = 0;
	//
	// //
	// for(i=0;i<pixelsPerProc;i++)
	// 	count_a[recvData[i]] += 1;


	// printf(" FOR 231 index %d value %d ", 231, count_a[231]);
	// printf(" FOR 10 index %d value %d ", 10, count_a[10]);
	// printf(" FOR 48 index %d value %d ", 48, count_a[48]);
	// printf(" FOR 123 index %d value %d ", 123, count_a[123]);



	//int a=0;
	//MPI_Reduce(&count_a, a, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	//
	//Step 4. Send all of the local counts back to rank 0, which calculates the total.

	// int total;
	// // Point-to-point: Use a loop of send-and-receives.
	// if( rank==0 )
	// {
	// 	// Start the running total with rank 0's count.
	// 	total = count;
	//
	// 	// Now add on all of the counts from the other processes.
	// 	for( p=1; p<numProcs; p++ )
	// 	{
	// 		int next;
	// 		MPI_Recv( &next, 1, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
	// 		total += next;
	// 	}
	// }
	// else
	// {
	// 	MPI_Send( &count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
	// }


	MPI_Gather(
		count_a, 1, MPI_INT,
		combinedHist, 1, MPI_INT,
		0, MPI_COMM_WORLD
	);





	//
	// Constructs the histogram in serial on rank 0. Can be used as part of a check that your parallel version works.
	//
	if( rank==0 )
	{
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
