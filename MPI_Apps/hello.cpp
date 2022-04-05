//#include <mpi.h>
//#include <cstdio>
//#include <cstdlib>
//
//int main(int argc, char*argv[]){
//	int rank, nproc;
//	int  nameSize;       /* length of name */
//	char computerName[MPI_MAX_PROCESSOR_NAME];
//	MPI_Init(&argc, &argv);
//	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	MPI_Comm_size(MPI_COMM_WORLD, &nproc);
//	MPI_Get_processor_name(computerName, &nameSize);
//	fprintf(stderr,"Hello from process %d on %s\n", rank, computerName);
//	MPI_Finalize();
//	//system("PAUSE");
//	return 0;
//}