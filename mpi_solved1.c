/******************************************************************************
* FILE: mpi_solved1.c
* DESCRIPTION: 
*   This program has a bug that causes it to hang.
*   This is because the tag is inconsistent between the send and receive processors.
* AUTHOR: Blaise Barney, corrected by Di Qi 
* LAST REVISED: 03/23/15
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
int numtasks, rank, dest, tag1, tag2, source, rc, count;
char inmsg, outmsg='x';
MPI_Status Stat;

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
printf("Task %d starting...\n",rank);

if (rank == 0) {
  if (numtasks > 2) 
    printf("Numtasks=%d. Only 2 needed. Ignoring extra...\n",numtasks);
  dest = rank + 1;
  source = dest;
  // send and received tag should be the same
  tag1 = 99;
  tag2 = 999;
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag1, MPI_COMM_WORLD);
  printf("Sent to task %d...\n",dest);
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag2, MPI_COMM_WORLD, &Stat);
  printf("Received from task %d...\n",source);
  }

else if (rank == 1) {
  dest = rank - 1;
  source = dest;
  // send and received tag should be the same
  tag1 = 99;
  tag2 = 999;
  rc = MPI_Recv(&inmsg, 1, MPI_CHAR, source, tag1, MPI_COMM_WORLD, &Stat);
  printf("Received from task %d...\n",source);
  rc = MPI_Send(&outmsg, 1, MPI_CHAR, dest, tag2, MPI_COMM_WORLD);
  printf("Sent to task %d...\n",dest);
  }

if (rank < 2) {
  rc = MPI_Get_count(&Stat, MPI_CHAR, &count);
  printf("Task %d: Received %d char(s) from task %d with tag %d \n",
         rank, count, Stat.MPI_SOURCE, Stat.MPI_TAG);
  }

MPI_Finalize();
return 0;
}
