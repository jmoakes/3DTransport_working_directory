#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define LONGSTRING 200
#define TINY 1e-10

char FilePrefix[LONGSTRING]; 
int ND, TS, NumNodes, NumElements, **Connectivity, *BDFlag, BDCount = 0;
double **Coordinates, **Vel;

void ProcessCommandLineArguements(int argc, char *argv[]);
void PrintUsage(char *argv[]);
void FreeMemory(void);
void SetBoundaryElementFlags(void);
void OutputElementFlags(void);
void LoadVelData(void);
void LoadMeshData(void);
void FatalError(char *text, ...);

int main(int argc, char *argv[]) {
	
  ProcessCommandLineArguements(argc, argv);
  LoadMeshData();
  LoadVelData();
  SetBoundaryElementFlags();
  OutputElementFlags();
  FreeMemory();
	
  return 0;
	
}


void ProcessCommandLineArguements(int argc, char *argv[]) {  
	
  if(argc < 4) 
    PrintUsage(argv);
  else if(argc > 4) 
    FatalError("Unsupported number of command line arguements");
	
  ND = atoi(argv[1]);
  sprintf(FilePrefix, "%s", argv[2]);
  TS = atoi(argv[3]);
  if(ND != 2 && ND != 3)
    FatalError("Invalid ND");
  if(ND == 2)
    FatalError("ND = 2 not currently supported");
	
}

void PrintUsage(char *argv[]) {
	
  fprintf(stderr, "Usage: %s ND fileprefix TS\n", argv[0]);
  fprintf(stderr, "Description:\n");
  fprintf(stderr, "  Determines elements containing nodes with zero velocity.\n");
  fprintf(stderr, "  Uses data from $fileprefix_vel.$TS.bin, $fileprefix_coordinates.bin and $fileprefix_connectivity.bin.\n");
  fprintf(stderr, "  Contents of output file: number of elements (int), element flags (int: 0 if interior element, 1 if element on boundary).\n");
  fprintf(stderr, "  Output file has name fileprefix_bflags.bin naming convention for use with flowVC.\n");
	
  exit(1);
	
}

void FreeMemory(void) {
	
  int i;
	
  for(i = 0; i < NumNodes; i++) {
    free(Coordinates[i]);
    free(Vel[i]);
  }
  free(Coordinates);
  free(Vel);
  for(i = 0; i < NumElements; i++)   
    free(Connectivity[i]);
  free(Connectivity);
  free(BDFlag);
	
}

void OutputElementFlags(void) {
	
  char File[LONGSTRING];
  FILE *FileID;

  /* Open file */
  sprintf(File, "%s_bflags.bin", FilePrefix);
  if((FileID = fopen(File, "wb")) == NULL) 
    FatalError("Could not open %s", File);
	
  /* Write number of elements */
  fwrite(&NumElements, sizeof(int), 1, FileID);	
	
  /* Write flags */
  fwrite(BDFlag, sizeof(int), NumElements, FileID);
	
  fclose(FileID);
	
}


void SetBoundaryElementFlags(void) {
	
  int i, j;
  double u, v, w;
	
  /* Allocated memory */
  if((BDFlag = (int *)calloc(NumElements, sizeof(int))) == NULL)
    FatalError("Malloc failed for BDFlag");
	
  /* Loop over elements */
  for(i = 0; i < NumElements; i++)
    /* Loop over nodes */ 
    for(j = 0; j < ND + 1; j++) {
      u = Vel[Connectivity[i][j]][0]; 
      v = Vel[Connectivity[i][j]][1]; 		
      w = Vel[Connectivity[i][j]][2]; 
      if(u*u + v*v + w*w < TINY) 
	BDFlag[i]++;
    }
}

void LoadVelData(void) {
	
  int i;
  char File[LONGSTRING];
  double time;
  FILE *FileID;
	
  /* Allocate memory */
  if((Vel = (double **)malloc(NumNodes * sizeof(double *))) == NULL)
    FatalError("Malloc failed for Vel");
  for(i = 0; i < NumNodes; i++)
    if((Vel[i] = (double *)malloc(3 * sizeof(double))) == NULL)
      FatalError("Malloc failed for Vel[%d]", i);
	
  /* Open file */
  sprintf(File, "%s_vel.%d.bin", FilePrefix, TS);
  if((FileID = fopen(File, "rb")) == NULL) 
    FatalError("Could not open %s", File);
	
  /* Read time stamp */
  if(fread(&time, sizeof(double), 1, FileID) < 1)
    FatalError("Could not read time stamp from %s", File); 
	
  /* Read data */
  for(i = 0; i < NumNodes; i++)
    if(fread(Vel[i], sizeof(double), 3, FileID) < 3)
      FatalError("Could not read Vel[%d]", i);
	
  fclose(FileID);
	
}

void LoadMeshData(void) {
  int i;
  char File[LONGSTRING];
  FILE *FileID;
	
  /* Open coordinate data file */
  sprintf(File, "%s_coordinates.bin", FilePrefix);
  if((FileID = fopen(File, "rb")) == NULL) 
    FatalError("Could not open %s", File);
	
  /* Read number of nodes */
  if(fread(&NumNodes, sizeof(int), 1, FileID) < 1)
    FatalError("Could not read number of nodes from %s", File);
	
  /* Allocate memory for Coordinates array */
  if((Coordinates = (double **)malloc(NumNodes * sizeof(double *))) == NULL)
    FatalError("Malloc failed for Coordinates");
  for(i = 0; i < NumNodes; i++)
    if((Coordinates[i] = (double *)malloc(3 * sizeof(double))) == NULL)
      FatalError("Malloc failed for Coordinates[%d]", i);
	
  /* Read coordinates */
  for(i = 0; i < NumNodes; i++) 
    if(fread(Coordinates[i], sizeof(double), 3, FileID) < 3)
      FatalError("Could read Coordinates[%d] from %s", i, File);
	
  /* Close coordinates file */
  fclose(FileID);
	
  /* Open connectivity data file */
  sprintf(File, "%s_connectivity.bin", FilePrefix);
  if((FileID = fopen(File, "rb")) == NULL) 
    FatalError("Could not open %s", File);
	
  /* Read number of elements */
  if(fread(&NumElements, sizeof(int), 1, FileID) < 1)
    FatalError("Could not read number of elements from %s", File);
	
  /* Allocate memory for Connectivity array */
  if((Connectivity = (int **)malloc(NumElements * sizeof(int *))) == NULL)
    FatalError("Malloc failed for Connectivity");
  for(i = 0; i < NumElements; i++)
    if((Connectivity[i] = (int *)malloc(4 * sizeof(int))) == NULL)
      FatalError("Malloc failed for Connectivity[%d]", i);
	
  /* Read connectivity */
  for(i = 0; i < NumElements; i++) /* { */
    if(fread(Connectivity[i], sizeof(int), 4, FileID) < 4)
      FatalError("Could not read Connectivity[%d] from %s", i, File);
	
  /* Close connectivity file */
  fclose(FileID);
	
}

void FatalError(char *text, ...) {
  va_list ap;
  char *p, *sval;
  int ival;
  double dval;
	
  fprintf(stderr, "\nERROR:\n");
  va_start(ap, text);
  for(p = text; *p; p++) {
    if(*p != '%') {
      putc(*p, stderr);
      continue;
    }
    switch (*++p) {
    case 'd':
      ival = va_arg(ap, int);
      fprintf(stderr, "%d", ival);
      break;
    case 'f':
      dval = va_arg(ap, double);
      fprintf(stderr, "%f", dval);
      break;
    case 's':
      for(sval = va_arg(ap, char *); *sval; sval++)
	putc(*sval, stderr);
      break;
    default:
      putc(*p, stderr);
      break;
    }
  }
  va_end(ap);
	
  fprintf(stderr, "\n");
  fflush(stderr);
	
  exit(1);
}
