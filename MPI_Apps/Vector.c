//#include <mpi.h>
//#include <stdio.h>
//
//#define R 4
//#define C 6
//
//int main(int argc, char** argv)	{
//	int numtasks, rank, source=0, tag=1, i;
//	float a[R*C] = {
//		1.0,	2.0,	3.0,	4.0,	5.0,	6.0,
//		7.0,	8.0,	9.0,	10.0,	11.0,	12.0,
//		13.0,	14.0,	15.0,	16.0,	17,		18,
//		19,		20,		21,		22,		23,		24};
//	float b[C];
//
//	MPI_Status stat;
//	MPI_Datatype columntype;
//	MPI_Datatype columntype2;
//
//	MPI_Init(&argc, &argv);
//	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
//
//	MPI_Type_vector(R, 1, C, MPI_FLOAT, &columntype);
//	MPI_Type_commit(&columntype);
//	MPI_Type_vector(R, 1, 1, MPI_FLOAT, &columntype2);
//	MPI_Type_commit(&columntype2);
//
//	if(numtasks == C){
//		if(rank==0){
//			for(i=1; i<numtasks; i++)
//				MPI_Send(&a[i], 1, columntype, i, tag, MPI_COMM_WORLD);
//		} else {
//			MPI_Recv(&b[0], 1, columntype2, source, tag, MPI_COMM_WORLD, &stat);
//			printf("rank = %d b= %3.1f %3.1f %3.1f %3.1f\n", rank, b[0], b[1], b[2], b[3], b[4], b[5]);
//		}
//	} else {
//		printf("Must specify &d processors. Terminating.\n", C);
//	}
//
//	MPI_Type_free(&columntype);
//	MPI_Finalize();
//	return 0;
//}
