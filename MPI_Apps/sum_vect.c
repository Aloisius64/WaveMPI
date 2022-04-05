//#include <mpi.h>
//#include <stdio.h>
//
//#define MAXSIZE 10
//
//int main(int argc, char*argv[]){
//	int myid, numprocs;
//	int data[MAXSIZE], i, x, low, high, myresult, result;
//	MPI_Init(&argc, &argv);
//	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
//	MPI_Comm_rank(MPI_COMM_WORLD, &myid);
//
//	result = 0;
//	myresult = 0;
//
//	if(myid == 0) {
//		for(i=0; i<MAXSIZE; i++){
//			data[i] = i;
//		}
//	}
//
//	MPI_Bcast(data, MAXSIZE, MPI_INT, 0, MPI_COMM_WORLD);
//
//	x = MAXSIZE/numprocs;
//	low = myid * x;
//	if(myid == (numprocs-1)){
//		high = MAXSIZE;
//	} else {		
//		high = low + x;
//	}
//
//	for(i=low; i<high; i++){
//		myresult += data[i];
//	}
//
//	printf("Process %d calculated %d\n", myid, myresult);
//
//	MPI_Reduce(&myresult, &result, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
//
//	if(myid == 0){
//		printf("The sum is: %d.\n", result);
//	}
//
//	MPI_Finalize();
//	//system("PAUSE");
//	return 0;
//}