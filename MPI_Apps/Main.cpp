#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#define MAXSIZE 100

const float FISH_PERCENTAGE = 0.60;
const float SHARK_PERCENTAGE = 0.1;

const int SIZE = MAXSIZE;

const int WATER = 0;
const int FISH = 1;
const int SHARK = 2;

const int NORTH = 0;
const int SOUTH = 1;
const int EAST = 2;
const int WEST = 3;

const int DIRECTIONS[4] = {NORTH,SOUTH,EAST,WEST};

const int FISH_STARTING_AGE = 2;
const int SHARK_STARTING_AGE = 1;

const int SHARK_STARVING_AGE = 0;

const int SHARK_BREEDING_AGE = 2;
const int FISH_BREEDING_AGE = 2;

const int FISH_BREEDING_STEP = 1;
const int SHARK_BREEDING_STEP = 1;

const int ITERATIONS = 1;

void init(int*,int*,int*,int*);
void init2(int* state,int* newState, int* agingState, int* newAgingState);
void testingInit(int* state,int* newState, int* agingState, int* newAgingState);

void wator(int*,int*,int*,int*,int);

void printState(int*);

int getDirection(int&,int&,int&);

int main(int argc, char** argv) {
	srand(time(0));

	bool clientMode;

	int *state;
	int *newState;
	int *agingState;
	int *newAgingState;

	int* tmp1;
	int* tmp2;

	int *localState;
	int *localNewState;
	int *localAgingState;
	int *localNewAgingState;

	int elementsToCompute;

	int rest;
	int chunks;
	int *chunkSizes;
	int *splittingPoints;
	int left;
	int right;
	int index;
	int k;
	int i;
	int j;
	int nextLine;
	int previousLine;
	int currentProc;

	int rowsToCompute;

	int iterationsCounter = 0;

	MPI_Status status;
	MPI_Request request;
	int numprocs = 0;
	int myid = 0;
	const int ROOT = 0;

	MPI_Init(&argc,&argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myid);

	assert( numprocs > 1 );

	//localElements = new int[chunkSizes[myid]];

	if ( myid == ROOT ) {
		state = new int[SIZE*SIZE];
		newState = new int[SIZE*SIZE];
		agingState = new int[SIZE*SIZE];
		newAgingState = new int[SIZE*SIZE];

		init(state,newState,agingState,newAgingState);
		//testingInit(state,newState,agingState,newAgingState);
		//printState(state);
		//printf("\n\n\n");
		//printState(agingState);
		//printf("\n");

		rest = SIZE%(numprocs-1);
		//printf("rest %d\n",rest );

		chunks = SIZE / ( numprocs-1 );
		chunkSizes = new int[numprocs-1];

		//calcolo i punti in cui spezzare i chunks
		splittingPoints = new int[numprocs];
		splittingPoints[ROOT] = 0;
		for ( i = 1; i < numprocs-1; ++i)
		{
			splittingPoints[i] = (i-1)*chunks;
			chunkSizes[i] = chunks;
		}
		splittingPoints[numprocs-1] = chunks * (numprocs-2);
		chunkSizes[numprocs-1] = SIZE - splittingPoints[numprocs-1];

		//load balancing
		currentProc = 2;
		while ( rest > 0 ) {
			for ( i = currentProc; i < numprocs-1; ++i) {
				chunkSizes[i]+=1;
				splittingPoints[i+1] += 1;
				chunkSizes[i+1] -= 1;
			}
			rest--;
			currentProc++;
		}

		//while ( iterationsCounter < ITERATIONS ) {
		double start = MPI_Wtime();

		for (  i = 1 ; i < numprocs ; i++ ) {
			//printf("proc %d, you have to compute from %d to %d chunkSize %d\n",i,splittingPoints[i]*SIZE,splittingPoints[i]*SIZE+chunkSizes[i]*SIZE,chunkSizes[i]);

			elementsToCompute = chunkSizes[i] * SIZE + 2 * SIZE;

			localState = new int[elementsToCompute];
			//localNewState = new int[elementsToCompute];
			localAgingState = new int[elementsToCompute];
			//localNewAgingState = new int[elementsToCompute];

			MPI_Send(&chunkSizes[i],1,MPI_INT,i,0,MPI_COMM_WORLD);

			//send critical parts
			left = i-1;
			if ( i == 1 )
				left = numprocs - 1;

			//ultima riga del precedente
			previousLine = splittingPoints[left]*SIZE + chunkSizes[left]*SIZE - SIZE;

			index = 0;
			//printf("Sending to %d critical part %d\n",i,previousLine );
			for ( j = 0 ; j < SIZE; j++ ) {
				localState[index] = state[previousLine];
				//localNewState[index] = newState[previousLine];
				localAgingState[index] = agingState[previousLine];
				//localNewAgingState[index] = newAgingState[previousLine];
				++index;
				++previousLine;
			}

			//chunk effettivo
			//printf("Sending to %d effective part %d to %d\n",i,splittingPoints[i]*SIZE,splittingPoints[i]*SIZE+chunkSizes[i]*SIZE );

			k = splittingPoints[i]*SIZE;

			for ( j = 0; j < chunkSizes[i]*SIZE ; j++ ) {
				localState[index] = state[k];
				//localNewState[index] = newState[k];
				localAgingState[index] = agingState[k];
				//localNewAgingState[index] = newAgingState[k];
				++index;
				++k;
			}

			right = i + 1;
			if ( i == numprocs-1 ) {
				right = 1;
			}

			//prima riga del successivo
			nextLine = splittingPoints[right]*SIZE;
			//printf("Sending to %d critical part %d\n",i,nextLine);

			for ( j = nextLine ; j < nextLine+SIZE; j++ ) {
				localState[index] = state[j];
				//localNewState[index] = newState[j];
				localAgingState[index] = agingState[j];
				//localNewAgingState[index] = localNewAgingState[j];
				++index;
			}

			// int count = 0;
			// printf("elementsToCompute %d\n",elementsToCompute);
			// for ( int j = 0 ; j < elementsToCompute; j++ ) {
			// 	printf("[%d]",localState[j] );
			// 	if ( count == SIZE-1 ) {
			// 		printf("\n");
			// 		count = -1;
			// 	}
			// 	count++;
			// }
			// printf("\n\n");

			MPI_Send(localState,elementsToCompute,MPI_INT,i,1,MPI_COMM_WORLD);
			//MPI_Send(localNewState,elementsToCompute,MPI_INT,i,2,MPI_COMM_WORLD);
			MPI_Send(localAgingState,elementsToCompute,MPI_INT,i,3,MPI_COMM_WORLD);
			//MPI_Send(localNewAgingState,elementsToCompute,MPI_INT,i,4,MPI_COMM_WORLD);

			delete[] localState;
			//delete[] localNewState;
			delete[] localAgingState;
			//delete[] localNewAgingState;
		}

		for (  i = 1 ; i < numprocs ; i++ ) {
			elementsToCompute = chunkSizes[i] * SIZE + 2 * SIZE;
			localNewState = new int[elementsToCompute];
			localNewAgingState = new int[elementsToCompute];

			MPI_Recv(localNewState,elementsToCompute,MPI_INT,i,5,MPI_COMM_WORLD,&status);
			MPI_Recv(localNewAgingState,elementsToCompute,MPI_INT,i,6,MPI_COMM_WORLD,&status);

			left = i-1;
			if ( i == 1 )
				left = numprocs - 1;

			previousLine = splittingPoints[left]*SIZE + chunkSizes[left]*SIZE - SIZE;

			//printf("gathering from %d setting up CRITICAL line PREVIOUS %d to %d in %d to %d\n",i,0,SIZE,previousLine,previousLine+SIZE);

			index = previousLine;

			for ( j = 0 ; j < SIZE ; j++ ) {
				newState[index] = localNewState[j];
				newAgingState[index] = localNewAgingState[j];
				index++;
			}

			index = (i-1)*chunkSizes[i]*SIZE;
			//printf("for proc %d need to fill from %d\n",i,index );

			for ( j = SIZE ; j < chunkSizes[i]*SIZE + SIZE; j++ ) {
				newState[index] = localNewState[j];
				newAgingState[index] = localNewAgingState[j];
				index++;
			}

			right = i + 1;
			if ( i == numprocs-1 ) {
				right = 1;
			}

			//prima riga del successivo
			nextLine = splittingPoints[right]*SIZE;

			//printf("gathering from %d setting up CRITICAL line NEXT %d to %d in %d to %d\n",i,chunkSizes[i]*SIZE+SIZE,chunkSizes[i]*SIZE+SIZE+SIZE,nextLine,nextLine+SIZE);
			index = nextLine;
			for ( j = chunkSizes[i]*SIZE+SIZE ; j < chunkSizes[i]*SIZE+SIZE*2 ; j++ ) {
				newState[index] = localNewState[index];
				newAgingState[index] = localNewAgingState[index];
				//newState[index] = -i;
				//newAgingState[index] = -i;
				index++;
			}

			delete [] localNewState;
			delete [] localNewAgingState;
		}

		// tmp1 = state;
		// state = newState;
		// newState = tmp1;

		// tmp2 = agingState;
		// agingState = newAgingState;
		// newAgingState = tmp2;

		//printState(state);
		//printf("\n\n\n");
		//printState(agingState);
		//printf("\n");

		//	iterationsCounter++;
		//}

		double end = MPI_Wtime();

		printf("%f\n",end-start);
	} else {
		MPI_Recv(&rowsToCompute,1,MPI_INT,ROOT,0,MPI_COMM_WORLD,&status);
		//printf("I am Proc %d and I compute %d\n",myid,rowsToCompute );
		elementsToCompute = rowsToCompute*SIZE + 2 * SIZE;
		//printf("elementsToCompute %d\n",elementsToCompute);

		localState = new int[elementsToCompute];
		localNewState = new int[elementsToCompute];
		localAgingState = new int[elementsToCompute];
		localNewAgingState = new int[elementsToCompute];

		for ( int i = 0 ; i < elementsToCompute; i++ ) {
			localNewAgingState[i] = 0;
			localNewState[i] = 0;
		}

		MPI_Recv(localState,elementsToCompute,MPI_INT,ROOT,1,MPI_COMM_WORLD,&status);
		//MPI_Recv(localNewState,elementsToCompute,MPI_INT,ROOT,2,MPI_COMM_WORLD,&status);
		MPI_Recv(localAgingState,elementsToCompute,MPI_INT,ROOT,3,MPI_COMM_WORLD,&status);
		//MPI_Recv(localNewAgingState,elementsToCompute,MPI_INT,ROOT,4,MPI_COMM_WORLD,&status);

		while ( iterationsCounter < ITERATIONS ) {
			wator(localState,localNewState,localAgingState,localNewAgingState,rowsToCompute);

			tmp1 = localState;
			localState = localNewState;
			localNewState = tmp1;

			tmp2 = localAgingState;
			localAgingState = localNewAgingState;
			localNewAgingState = tmp2;

			iterationsCounter++;
		}

		MPI_Send(localNewState,elementsToCompute,MPI_INT,ROOT,5,MPI_COMM_WORLD);
		MPI_Send(localNewAgingState,elementsToCompute,MPI_INT,ROOT,6,MPI_COMM_WORLD);

		// delete[] localState;
		// delete[] localNewState;
		// delete[] localAgingState;
		// delete[] localNewAgingState;
	}

	MPI_Finalize();
	return 0;
}

void init(int* state,int* newState, int* agingState, int* newAgingState) {
	for (int i = 0; i < SIZE*SIZE; ++i)
	{
		state[i] = 0;
		newState[i] = 0;
		agingState[i] = 0;
		newAgingState[i] = 0;
	}

	int fishToLocate = static_cast<int>(floor(SIZE * SIZE * FISH_PERCENTAGE));
	int sharksToLocate = static_cast<int>(floor(SIZE * SIZE * SHARK_PERCENTAGE));

	//fishToLocate = 1;
	//sharksToLocate = 0;

	//printf("fish %d sharks %d \n",fishToLocate, sharksToLocate);

	//fishToLocate = 5;
	//shark = 0;
	bool placed = false;

	for (int i = 0; i < fishToLocate; i++) {
		placed = false;

		do {
			int randomx = rand()%(SIZE*SIZE);

			if ( state[ randomx ] == WATER ) {
				state[ randomx ] = FISH;
				newState[ randomx ] = FISH;
				agingState[ randomx ] = FISH_STARTING_AGE;
				newAgingState[ randomx ] = FISH_STARTING_AGE;
				placed = true;
			}
		} while ( !placed );
	}

	for (int i = 0; i < sharksToLocate; i++) {
		placed = false;

		do {
			int randomx = rand()%(SIZE*SIZE);

			if ( state[ randomx ] == WATER ) {
				state[ randomx ] = SHARK;
				newState[ randomx ] = SHARK;
				agingState[ randomx ] = SHARK_STARTING_AGE;
				newAgingState[ randomx ] = SHARK_STARTING_AGE;
				placed = true;
			}
		} while ( !placed );
	}
}

void init2(int* state,int* newState, int* agingState, int* newAgingState) {
	int k = 0;
	for ( int i = 0 ; i < SIZE ; i++ ) {
		for ( int j = 0 ; j < SIZE ; j++ ) {
			state[(i*SIZE)+j] = (i*SIZE)+j;
		}
	}
}

void printState(int* state) {
	for ( int i = 0 ; i < SIZE ; i++ ) {
		for (int j = 0; j < SIZE; j++ ) {
			printf("[%d]",state[i*SIZE+j] );
		}
		printf("\n");
	}
}

void wator(int* state,int* newState,int* agingState, int* newAgingState,int rows) {
	for ( int i = 0; i < rows; i++ ) {
		for ( int j = 0 ; j < SIZE; j++ ) {
			int index = i*SIZE + j + SIZE;

			if ( state[index] == WATER ) {
				newState[index] = WATER;
				newAgingState[index] = 0;
			}

			if ( state[index] == FISH ) {
				int newIndex;

				int direction = DIRECTIONS[rand()%4];

				newIndex = getDirection(direction,i,index);

				if ( state[newIndex] == WATER ) {
					newState[newIndex] = FISH;
					newState[index] = WATER;
					newAgingState[index] = 0;
					state[newIndex] = -1;
					state[index] = -1;

					if ( agingState[index] > FISH_BREEDING_AGE ) {
						newAgingState[index] = FISH_STARTING_AGE;
						newAgingState[newIndex] = FISH_STARTING_AGE;
						newState[index] = FISH;
						state[index] = -1;
					} else {
						newAgingState[newIndex] = agingState[index] + FISH_BREEDING_STEP;
						agingState[index] = 0;
					}
				} else {
					newState[index] = FISH;
					newAgingState[index] = agingState[index];
				}
			}

			if ( state[index] == SHARK ) {
				int newIndex = -1;
				bool found = false;

				int k = 0;
				int direction = -1;

				while ( k < 4 && newIndex == -1  ) {
					direction = DIRECTIONS[k];
					newIndex = getDirection(direction,i,index);
					if ( state[newIndex] != FISH ) {
						newIndex = -1;
					}
					k++;
				}

				if ( newIndex == -1 ) {
					direction = DIRECTIONS[rand()%4];
					newIndex = getDirection(direction,i,index);
				}

				if ( state[newIndex] == FISH ) {
					newState[newIndex] = SHARK;
					newState[index] = WATER;
					newAgingState[newIndex] = agingState[index] + SHARK_BREEDING_STEP;
					state[newIndex] = -1;
					state[index] = -1;

					if ( agingState[index] > SHARK_BREEDING_AGE ) {
						newState[index] = SHARK;
						newAgingState[index] = SHARK_STARTING_AGE;
						newAgingState[newIndex] = SHARK_STARTING_AGE;
					}
				}

				if ( state[newIndex] == WATER ) {
					newState[newIndex] = SHARK;
					newState[index] = WATER;
					newAgingState[index] = 0;
					state[newIndex] = -1;
					state[index] = -1;
					if ( agingState[index] > SHARK_BREEDING_AGE ) {
						newState[index] = SHARK;
						newAgingState[index] = SHARK_STARTING_AGE;
						newAgingState[newIndex] = SHARK_STARTING_AGE;
						state[index] = -1;
					} else if ( agingState[index] < SHARK_STARTING_AGE ) {
						newState[newIndex] = WATER;
						newAgingState[newIndex] = 0;
						state[index] = -1;
					} else {
						newAgingState[newIndex] = agingState[index] - SHARK_BREEDING_STEP;
						agingState[index] = 0;
						state[index] = -1;
					}
				} else if ( state[newIndex] == SHARK ){
					newState[index] = SHARK;
					newAgingState[index] = agingState[index];
				}
			}
		}
	}

	// for( int i = 0 ; i < rows; i++ ) {
	// 	for ( int j = 0 ; j < SIZE ; j++ ) {
	// 		printf("[%d]",newState[i*SIZE+j+SIZE] );
	// 	}
	// 	printf("\n");
	// }
	// printf("\n\n\n");

	// for( int i = 0 ; i < rows; i++ ) {
	// 	for ( int j = 0 ; j < SIZE ; j++ ) {
	// 		printf("[%d]",newAgingState[i*SIZE+j+SIZE] );
	// 	}
	// 	printf("\n");
	// }
	// printf("------------\n");
}

void testingInit(int* state,int* newState, int* agingState, int* newAgingState) {
	for ( int i = 0 ; i < SIZE; i++ ) {
		for ( int j = 0; j < SIZE ; j++ ) {
			state[i*SIZE+j] = 0;
			newState[i*SIZE+j] = 0;
			agingState[i*SIZE+j] = 0;
			newAgingState[i*SIZE+j] = 0;
		}
	}

	state[3*SIZE+3] = FISH;
	newState[3*SIZE+3] = FISH;
	agingState[3*SIZE+3] = FISH_STARTING_AGE;
	newAgingState[3*SIZE+3] = FISH_STARTING_AGE;

	// state[3*SIZE+1] = SHARK;
	// newState[3*SIZE+1] = SHARK;
	// agingState[3*SIZE+1] = SHARK_STARTING_AGE;
	// newAgingState[3*SIZE+1] = SHARK_STARTING_AGE;
}

int getDirection(int &direction,int &i,int &index) {
	int newIndex = 0;

	switch (direction) {
	case (NORTH):
		newIndex = index-SIZE;
		break;

	case (SOUTH):
		newIndex = index+SIZE;
		break;

	case (EAST):
		if ( ( index + 1 ) == i * SIZE + SIZE + SIZE ) {
			newIndex = index-SIZE+1;
			//printf("EST CRITICAL index %d newIndex %d , ( %d - %d )\n",index,newIndex,index+1,i*SIZE+SIZE+SIZE );
			//return;
		} else {
			newIndex = index+1;
			//printf("EST index %d newIndex %d , ( %d - %d )\n",index,newIndex,index+1,i*SIZE+SIZE+SIZE );
		}
		break;

	case (WEST):
		if ( ( index - 1 ) < i * SIZE + SIZE ) {
			newIndex = index+SIZE-1;
			//printf("WEST CRITICAL index %d newIndex %d \n",index,newIndex );
			//return;
		} else {
			newIndex = index-1;
			//printf("WEST index %d newIndex %d \n",index,newIndex );
		}
		break;
	}

	return newIndex;
}