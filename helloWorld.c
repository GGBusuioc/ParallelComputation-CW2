//
// A simple 'Hello World' program for MPI-C.
//
// Compile with:
//
// mpicc -Wall -o helloWorld helloWorld.c
//
// and execute with (for p processes on the local machine):
//
// mpirun -n p helloWorld
//


//
// Includes
//
#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"


//
// Main
//
int main( int argc, char **argv )
{
	int numprocs, rank;

	// Initialises the MPI execution environment; returns MPI_SUCCESS or MPI_ERR_OTHER (if called twice).
	MPI_Init( &argc, &argv );

	// Get the size of the group associated with the communicator (here 'world', i.e. everything).
	MPI_Comm_size( MPI_COMM_WORLD, &numprocs );

	// Get the rank of the calling process in the communicator.
	MPI_Comm_rank( MPI_COMM_WORLD, &rank     );

	// Output message to stdout.
	printf( "Process %d of %d.\n", rank, numprocs );

	// Terminates the MPI execution environment; all processes must call this before terminating.
	MPI_Finalize();

	return EXIT_SUCCESS;
}
