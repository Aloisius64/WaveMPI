//#include <mpi.h>
//#include <stdio.h>
//#include <stdlib.h>
//
//#define MAXSIZE 10
//
//int main(int argc, char*argv[]){
//	int myid, numprocs;
//	int x, low, high, result_temp, i;
//	int dest, source;
//	int *data, *local_data;
//	double myresult, result;
//	double start, end;
//	MPI_Status status;
//
//	MPI_Init(&argc, &argv);
//	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
//	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
//
//	result = 0;
//	myresult = 0;
//
//	if(myid == 0) {
//		data = (int*) malloc(sizeof(int)*MAXSIZE);
//		for(i=0; i<MAXSIZE; i++){
//			data[i] = i;
//		}
//	}
//	start = MPI_Wtime();
//
//	x = MAXSIZE/numprocs;
//	local_data = (int*) malloc(sizeof(int)*x);
//	MPI_Scatter(data, x, MPI_INT, local_data, x, MPI_INT, 0, MPI_COMM_WORLD);
//
//	for(i=0; i<x; i++){
//		myresult += local_data[i];
//	}
//
//	//if(myid == 0){
//	//	result = myresult;
//	//	for(source=0)
//	//}
//
//	if(myid == 0){
//		printf("The sum is: %d.\n", result);
//	}
//
//	MPI_Finalize();
//	//system("PAUSE");
//	return 0;
//}