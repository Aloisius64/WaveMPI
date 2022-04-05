#include "LifeCA.h"
#include <mpi.h>

#define CONFIGURATION_FILE "configuration.txt"
//#define VERBOSE
#define CATYPE MPI_FLOAT
#define WIDTH_WINDOW 640
#define HEIGHT_WINDOW 480

pthread_t threadForMPI;
int idProcessor, noProcessors;
int idMasterProcess = 0;
MPI_Status send_status, receive_status, masterStatus;
char tmpBuff[100] = "\0";
//	Each process has its neighboor idProcess
//	-1 means that there is no process in that direction
#pragma region BorderExchangeRegion
int neighboor[8] = {-1,	//	UP
	-1,	//	UP-RIGHT
	-1,	//	RIGHT
	-1,	//	DOWN-RIGHT
	-1,	//	DOWN
	-1,	//	DOWN-LEFT
	-1,	//	LEFT
	-1};//	UP-LEFT
MPI_Request borderSendRequest[8];
MPI_Request borderReceiveRequest[8];
void* buffSendInit[8] = {NULL,	//	UP
	NULL,	//	UP-RIGHT
	NULL,	//	RIGHT
	NULL,	//	DOWN-RIGHT
	NULL,	//	DOWN
	NULL,	//	DOWN-LEFT
	NULL,	//	LEFT
	NULL};//	UP-LEFT
void* buffRecvInit[8] = {NULL,	//	UP
	NULL,	//	UP-RIGHT
	NULL,	//	RIGHT
	NULL,	//	DOWN-RIGHT
	NULL,	//	DOWN
	NULL,	//	DOWN-LEFT
	NULL,	//	LEFT
	NULL};	//	UP-LEFT
int countInit[8] = {0,	//	UP
	0,	//	UP-RIGHT
	0,	//	RIGHT
	0,	//	DOWN-RIGHT
	0,	//	DOWN
	0,	//	DOWN-LEFT
	0,	//	LEFT
	0};//	UP-LEFT
float* tmpExchangeBuffer[8] = {NULL,	//	UP
	NULL,	//	UP-RIGHT
	NULL,	//	RIGHT
	NULL,	//	DOWN-RIGHT
	NULL,	//	DOWN
	NULL,	//	DOWN-LEFT
	NULL,	//	LEFT
	NULL};	//	UP-LEFT
#pragma endregion

void finalizeAll();
void setupBordersConnection();
void readConfigurationFile();
int getCellDataAt(int i, int j, int rows, int columns, int* data);
void executeBorderExchange();
void* updaterForMPICalls(void* arg);
void transferCAtoTmpExchangeBuffer();
void transferTmpExchangeBufferToCA();

int main(int argc, char** argv)	{
	int idOtherProcess = 0, stopValue = 0;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &noProcessors);
	MPI_Comm_rank(MPI_COMM_WORLD, &idProcessor);

	readConfigurationFile();
	if(idProcessor != idMasterProcess){
		initializeCAandGraphic();
		setupBordersConnection();
		pthread_create(&threadForMPI, NULL, updaterForMPICalls, NULL);
		calglStartProcessWindow2D(argc, argv);
		//	Finalization
		destroyAll();
	} else {
		//	Receive message from slaves for close all
		for(idOtherProcess = 0; idOtherProcess<noProcessors; idOtherProcess++){
			if(idOtherProcess != idMasterProcess){
				MPI_Recv(&stopValue, 1, MPI_INT, idOtherProcess, 0, MPI_COMM_WORLD, &masterStatus);
			}
		}
	}

	MPI_Finalize();
	return 0;
}

void setupBordersConnection(){
	int k = 0, errCode = 0;

#pragma region PersistentSendReceiveInitialization
	//	Buffers Initialization
	buffSendInit[0] = waveGet2Di(1, 1);	//	UP
	buffRecvInit[0] = waveGet2Di(0, 1);
	buffSendInit[1] = waveGet2Di(1, calglGetGlobalSettings()->columns-2);	//	UP-RIGHT
	buffRecvInit[1] = waveGet2Di(0, calglGetGlobalSettings()->columns-1);
	buffSendInit[2] = waveGet2Di(1, calglGetGlobalSettings()->columns-2);	//	RIGHT
	buffRecvInit[2] = waveGet2Di(1, calglGetGlobalSettings()->columns-1);
	buffSendInit[3] = waveGet2Di(calglGetGlobalSettings()->rows-2, calglGetGlobalSettings()->columns-2);	//	DOWN-RIGHT
	buffRecvInit[3] = waveGet2Di(calglGetGlobalSettings()->rows-1, calglGetGlobalSettings()->columns-1);
	buffSendInit[4] = waveGet2Di(calglGetGlobalSettings()->rows-2, 1);	//	DOWN
	buffRecvInit[4] = waveGet2Di(calglGetGlobalSettings()->rows-1, 1);
	buffSendInit[5] = waveGet2Di(calglGetGlobalSettings()->rows-2, 1);	//	DOWN-LEFT
	buffRecvInit[5] = waveGet2Di(calglGetGlobalSettings()->rows-1, 0);
	buffSendInit[6] = waveGet2Di(1, 1);	//	LEFT
	buffRecvInit[6] = waveGet2Di(1, 0);
	buffSendInit[7] = waveGet2Di(1, 1);	//	UP-LEFT
	buffRecvInit[7] = waveGet2Di(0, 0);
	//	Count Initialization
	countInit[0] = calglGetGlobalSettings()->columns-2;
	countInit[1] = 1;
	countInit[2] = calglGetGlobalSettings()->rows-2;
	countInit[3] = 1;
	countInit[4] = calglGetGlobalSettings()->columns-2;
	countInit[5] = 1;
	countInit[6] = calglGetGlobalSettings()->rows-2;
	countInit[7] = 1;
#pragma endregion

	for(k=0; k<8; k++){
		if(neighboor[k]!=-1){
			tmpExchangeBuffer[k] = (float*) malloc(sizeof(float)*countInit[k]);
		}
	}

	for(k=0; k<8; k++){
		if(neighboor[k]!=-1){
			errCode = MPI_Send_init(tmpExchangeBuffer[k], countInit[k], CATYPE, neighboor[k], idProcessor, MPI_COMM_WORLD, &borderReceiveRequest[k]);
			//printf("Proc %d, err %d for k %d\n", idProcessor, errCode, k);
			errCode = MPI_Recv_init(tmpExchangeBuffer[k], countInit[k], CATYPE, neighboor[k], neighboor[k], MPI_COMM_WORLD, &borderSendRequest[k]);
			//printf("Proc %d, err %d for k %d\n", idProcessor, errCode, k);
		}
	}
}

void readConfigurationFile(){
	FILE* configurationFile = NULL;
	char buff[100] = "\0";
	int *data = NULL;
	int rows = 0, columns = 0;
	int steps = 0, numProccessors = 1;
	int tmp = 0, k = 0;
	int myRow = 0, myColumn = 0;

#ifdef _WIN32
	fopen_s(&configurationFile, CONFIGURATION_FILE, "r");
#else
	configurationFile = fopen(CONFIGURATION_FILE, "r");
#endif
	if(configurationFile != NULL){
		//	Cellular Automata Rows
		fscanf(configurationFile, "%s", buff);	//	Jump header line
		fscanf(configurationFile, "%s", buff);
		rows = atoi(buff);
		//	Cellular Automata Columns
		fscanf(configurationFile, "%s", buff);	//	Jump header line
		fscanf(configurationFile, "%s", buff);
		columns = atoi(buff);
		calglSetRowsAndColumnsGlobalSettings(rows+2, columns+2);	//	+2 is for ghosts borders
		//	Steps
		fscanf(configurationFile, "%s", buff);	//	Jump header line
		fscanf(configurationFile, "%s", buff);
		steps = atoi(buff);
		calglSetStepGlobalSettings(steps);
		//	ProcessorsNumber
		fscanf(configurationFile, "%s", buff);	//	Jump header line
		fscanf(configurationFile, "%s", buff);
		numProccessors = atoi(buff);
		//	MasterProcess
		fscanf(configurationFile, "%s", buff);	//	Jump header line
		fscanf(configurationFile, "%s", buff);
		idMasterProcess = atoi(buff);
		if(idProcessor != idMasterProcess){
			//	Rows
			fscanf(configurationFile, "%s", buff);	//	Jump header line
			fscanf(configurationFile, "%s", buff);
			rows = atoi(buff);
			//	Columns
			fscanf(configurationFile, "%s", buff);	//	Jump header line
			fscanf(configurationFile, "%s", buff);
			columns = atoi(buff);
			//	SlaveDisplacement
			fscanf(configurationFile, "%s", buff);	//	Jump header line
			data = (int*) malloc(sizeof(int)*(numProccessors-1));	//	The master is excluded
			k = 0;
			while(1){
				tmp = fscanf(configurationFile, "%s", buff);
				if(tmp == EOF || strcmp(buff, "Executable:")==0){
					break;
				}
				data[k] = atoi(buff);
				if(data[k] == idProcessor){
					if(rows==1){
						myRow = 0;
					} else {
						myRow = k/rows;
					}
					myColumn = k%columns;

					strset(tmpBuff, ' ');
					strcat(tmpBuff, "WaveCAProc");
					strcat(tmpBuff, buff);
					strcat(tmpBuff, "\0");	//printf("%s\n", tmpBuff);
					calglSetApplicationNameGlobalSettings(tmpBuff);
					calglSetWindowDimensionGlobalSettings(WIDTH_WINDOW, HEIGHT_WINDOW);
					calglSetWindowPositionGlobalSettings(WIDTH_WINDOW*myColumn, HEIGHT_WINDOW*myRow);
					calglSetRefreshTime(10);
#ifdef VERBOSE
					printf("process %d, r: %d, c: %d\n", idProcessor, myRow, myColumn);
#endif
				}
				k++;
			}
			//	Neighboor assignment
			neighboor[0] = getCellDataAt(myRow-1, myColumn, rows, columns, data);	//	UP
			neighboor[1] = getCellDataAt(myRow-1, myColumn+1, rows, columns, data);	//	UP-RIGHT
			neighboor[2] = getCellDataAt(myRow, myColumn+1, rows, columns, data);	//	RIGHT
			neighboor[3] = getCellDataAt(myRow+1, myColumn+1, rows, columns, data);	//	DOWN-RIGHT
			neighboor[4] = getCellDataAt(myRow+1, myColumn, rows, columns, data);	//	DOWN
			neighboor[5] = getCellDataAt(myRow+1, myColumn-1, rows, columns, data);	//	DOWN-LEFT
			neighboor[6] = getCellDataAt(myRow, myColumn-1, rows, columns, data);	//	LEFT
			neighboor[7] = getCellDataAt(myRow-1, myColumn-1, rows, columns, data);//	UP-LEFT

#ifdef VERBOSE
			printf("process %d, my neighboor\t", idProcessor);
			for(k=0; k<8; k++){
				if(neighboor[k] != -1){
					printf("%d ", neighboor[k]);
				}
			}
			printf("\n");
#endif

			if(data)
				free(data);
		}

		fclose(configurationFile);
	} else {
		printf("Error to opening file: %s\n", CONFIGURATION_FILE);
		MPI_Finalize();
		exit(1);
	}
}

int getCellDataAt(int i, int j, int rows, int columns, int* data){
	if(i>=0 && i<rows && j>=0 && j<columns){
		return 1+(i*columns)+j;
	}
	return -1;
}

void executeBorderExchange(){
	int k = 0;

	transferCAtoTmpExchangeBuffer();

	for(k=0; k<8; k++){
		if(neighboor[k] != -1){
			MPI_Start(&borderSendRequest[k]);
		}
	}
	for(k=0; k<8; k++){
		if(neighboor[k] != -1){
			MPI_Start(&borderReceiveRequest[k]);
		}
	}
	for(k=0; k<8; k++){
		if(neighboor[k] != -1){
			MPI_Wait(&borderSendRequest[k], &send_status);
		}
	}
	for(k=0; k<8; k++){
		if(neighboor[k] != -1){
			MPI_Wait(&borderReceiveRequest[k], &receive_status);
		}
	}

	transferTmpExchangeBufferToCA();
}

//	In this function we have to put all MPI calls
void* updaterForMPICalls(void* arg){
	int flag = 0;
	while(!lifeStopCondition(getWaveModel())){
		executeBorderExchange();		//	Exchange borders
		getWaveRun()->step++;			//	Update steps
		calRunCAStep2D(getWaveRun());	//	Update CA
		Sleep(10);
	}

	//	Send message to master for comunicate to close all
	MPI_Send(&flag, 1, MPI_INT, idMasterProcess, 0, MPI_COMM_WORLD);
	//	Finalization
	finalizeAll();
	return (void *) 0;
}

void transferCAtoTmpExchangeBuffer(){
	int k=0, i=0;

	if(neighboor[k]!=-1){ // UP
		for(i=1; i<countInit[k]; i++){
			tmpExchangeBuffer[k][i-1] = *waveGet2Di(1, i);
		}
	}
	k++;
	if(neighboor[k]!=-1){ // UP-R
		tmpExchangeBuffer[k][0] = *waveGet2Di(1, calglGetGlobalSettings()->columns-2);
	}
	k++;
	if(neighboor[k]!=-1){ // RIGHT
		for(i=1; i<countInit[k]; i++){
			tmpExchangeBuffer[k][i-1] = *waveGet2Di(i, calglGetGlobalSettings()->columns-2);
		}
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN-R
		tmpExchangeBuffer[k][0] = *waveGet2Di(calglGetGlobalSettings()->rows-2, calglGetGlobalSettings()->columns-2);
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN
		for(i=1; i<countInit[k]; i++){
			tmpExchangeBuffer[k][i-1] = *waveGet2Di(calglGetGlobalSettings()->rows-2, i);
		}
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN-L
		tmpExchangeBuffer[k][0] = *waveGet2Di(calglGetGlobalSettings()->rows-2, 1);
	}
	k++;
	if(neighboor[k]!=-1){ // LEFT
		for(i=1; i<countInit[k]; i++){
			tmpExchangeBuffer[k][i-1] = *waveGet2Di(i, 1);
		}
	}
	k++;
	if(neighboor[k]!=-1){ // UP-L
		tmpExchangeBuffer[k][0] = *waveGet2Di(1, 1);
	}
	k++;
}

void transferTmpExchangeBufferToCA(){
	int k=0, i=0;

	if(neighboor[k]!=-1){ // UP
		for(i=1; i<countInit[k]; i++){
			*waveGet2Di(0, i) = tmpExchangeBuffer[k][i-1];
		}
	}
	k++;
	if(neighboor[k]!=-1){ // UP-R
		*waveGet2Di(0, calglGetGlobalSettings()->columns-1) = tmpExchangeBuffer[k][0];
	}
	k++;
	if(neighboor[k]!=-1){ // RIGHT
		for(i=1; i<countInit[k]; i++){
			*waveGet2Di(i, calglGetGlobalSettings()->columns-1) = tmpExchangeBuffer[k][i-1];
		}
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN-R
		*waveGet2Di(calglGetGlobalSettings()->rows-1, calglGetGlobalSettings()->columns-1) = tmpExchangeBuffer[k][0];
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN
		for(i=1; i<countInit[k]; i++){
			*waveGet2Di(calglGetGlobalSettings()->rows-1, i) = tmpExchangeBuffer[k][i-1];
		}
	}
	k++;
	if(neighboor[k]!=-1){ // DOWN-L
		*waveGet2Di(calglGetGlobalSettings()->rows-1, 0) = tmpExchangeBuffer[k][0];
	}
	k++;
	if(neighboor[k]!=-1){ // LEFT
		for(i=1; i<countInit[k]; i++){
			*waveGet2Di(i, 0) = tmpExchangeBuffer[k][i-1];
		}
	}
	k++;
	if(neighboor[k]!=-1){ // UP-L
		*waveGet2Di(0, 0) = tmpExchangeBuffer[k][0];
	}
	k++;
}

void finalizeAll(){
	int k=0;
	if(idProcessor != idMasterProcess){
		for(k=0; k<8; k++){
			if(borderSendRequest[k]){
				MPI_Request_free(&borderSendRequest[k]);
			}
			if(borderReceiveRequest[k]){
				MPI_Request_free(&borderReceiveRequest[k]);
			}
			if(neighboor[k]){
				free(tmpExchangeBuffer[k]);
			}
		}
	}
	MPI_Finalize();
}
