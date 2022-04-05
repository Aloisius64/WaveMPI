#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*	This is a utility code for run
*	in a easyer manner mpi applications.
*/

#define CONFIGURATION_FILE "configuration.txt"
#define NETWORK_FILE "network.txt"

FILE* configurationFile = NULL;
FILE* networkFile = NULL;
int numProccessors = 1;
int numHosts = 1;
char masterHost[100] = "\0";
char executable[100] = "\0";
int k = 0;
char tmp[100] = "";

void createCommand(int argc, char *argv[]);

int main(int argc, char *argv[]){
	createCommand(argc, argv);
	system(tmp);
	return 0;
}

void createCommand(int argc, char *argv[]){
	char buff[100] = "\0";

#ifdef _WIN32
	fopen_s(&configurationFile, CONFIGURATION_FILE, "r");
#else
	configurationFile = fopen(CONFIGURATION_FILE, "r");
#endif
	if(configurationFile != NULL){
		//	Rows
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		//	Columns
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		//	Steps
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		//	ProcessorsNumber
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		numProccessors = atoi(buff);
		//	MasterProcess
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		//	Rows
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		k = atoi(buff);
		//	Columns
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		//	SlaveDisplacement
		fscanf(configurationFile, "%s", buff);
		while(k>=0){
			fscanf(configurationFile, "%s", buff);
			k--;
		}
		//	Executable
		fscanf(configurationFile, "%s", buff);
		fscanf(configurationFile, "%s", buff);
		strcpy(executable, buff);
		fclose(configurationFile);

#ifdef _WIN32
		fopen_s(&networkFile, NETWORK_FILE, "r");
#else
		configurationFile = fopen(NETWORK_FILE, "r");
#endif
		if(networkFile != NULL){
			strset(tmp, ' ');
#ifdef _WIN32
			strcat(tmp, "mpiexec -hosts ");
#else
			strcat(tmp, "mpirun -hosts ");
#endif
			//	No hosts
			fscanf(networkFile, "%s", buff);
			fscanf(networkFile, "%s", buff);
			strcat(tmp, buff);
			strcat(tmp, " ");
			numHosts = atoi(buff);
			//	Hosts
			fscanf(networkFile, "%s", buff);
			for(k=0; k<numHosts; k++){
				fscanf(networkFile, "%s", buff);
				strcat(tmp, buff);
				strcat(tmp, " ");
				if(k==0){	//	Set master host
					strcpy(masterHost, buff);
				}
				fscanf(networkFile, "%s", buff);
				strcat(tmp, buff);
				strcat(tmp, " ");
			}
			//	Directory
			fscanf(networkFile, "%s", buff);
			fscanf(networkFile, "%s", buff);
			strcat(tmp, "-wdir ");
			strcat(tmp, "\\\\");
			strcat(tmp, masterHost);
			strcat(tmp, "\\");
			strcat(tmp, buff);
			strcat(tmp, " ");
			//	Executable
			fscanf(networkFile, "%s", buff);
			fscanf(networkFile, "%s", buff);
			strcat(tmp, buff);
			strcat(tmp, "\0");
			fclose(networkFile);
		} else {
			printf("Localhost running..\n");

			strset(tmp, ' ');
#ifdef _WIN32
			strcat(tmp, "mpiexec -n ");
#else
			strcat(tmp, "mpirun -n ");
#endif
			sprintf(buff, "%d", numProccessors);
			strcat(tmp, buff);
			strcat(tmp, " ");
			strcat(tmp, executable);
			strcat(tmp, "\0");
		}
	} else {
		printf("Error to opening file: %s\n", CONFIGURATION_FILE);
	}

	printf("Run >>> %s\n", tmp);
}
