/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>


static int compare(const void *a, const void *b)
{
  int *da = (int *)a;
  int *db = (int *)b;

  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}

int main( int argc, char *argv[])
{
  int rank, mpisize;
  int i, ind, N, s, count;
  int smax;
  int *vec, *sample, *temp, *splitter;

  double tbegin = 0, tend = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpisize);

  /* Number of random numbers per processor and the sample size (this should be increased
   * for actual tests or could be made a passed in through the command line */
  N = 1000;
  s = 10;

  vec = calloc(N, sizeof(int));
  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 393919));

  /* fill vector with random integers */
  for (i = 0; i < N; ++i) {
    vec[i] = rand();
  }


  /* time count begins */
  MPI_Barrier(MPI_COMM_WORLD);
  tbegin = MPI_Wtime();

  /* sort locally */
  qsort(vec, N, sizeof(int), compare);

  /* randomly sample s entries from vector or select local splitters,
   * i.e., every N/P-th entry of the sorted vector */
  sample = calloc(s, sizeof(int));
  for (i = 0; i < s; ++i) {
      ind = (N/s) * i + (N/s/2);   //local splitter
      //ind = rand() % N;          //random splitter
      sample[i] = vec[ind];
  }

  /* every processor communicates the selected entries
   * to the root processor */
  if (rank == 0)
    temp = calloc(s*mpisize, sizeof(int));
  MPI_Gather(sample, s, MPI_INT, temp, s, MPI_INT, 0, MPI_COMM_WORLD);
  free(sample);

  /* root processor does a sort, determinates splitters and broadcasts them */
  splitter = calloc(mpisize-1, sizeof(int));
  if (rank == 0) {
      qsort(temp, s*mpisize, sizeof(int), compare);
      count = s*mpisize;
      smax = temp[count-1];
      while (temp[count-2] == smax) {
          count--;
      }
      for (i = 0; i < mpisize-1; ++i)
          splitter[i] = temp[(i+1)*count/mpisize-1];
      free(temp);
/*
      printf("splitters:");
      for (i = 0; i < mpisize-1; ++i)
          printf("\t%d", splitter[i]);
      printf("\n");
*/
  }
  MPI_Bcast(splitter, mpisize-1, MPI_INT, 0, MPI_COMM_WORLD);

  /* every processor uses the obtained splitters to decide to send
   * which integers to whom */
  int csent[mpisize];
  for (i = 0; i < mpisize; i++)
      csent[i] = 0;
  count = 0;
  for (i = 0; i < N; ++i) {
      if (vec[i] <= splitter[count]) {
          csent[count] = csent[count] + 1;
      } else if (count < mpisize-2) {
          ++count;
          csent[count] = csent[count] + 1;
      } else {
          csent[mpisize-1] = csent[mpisize-1] + 1;
      }
  }

/*
  int summ = 0;
  for (i = 0; i < mpisize; ++i)
      summ += csent[i];
  printf("rank %d of N=%d sums = %d\n", rank, N, summ);
*/

  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */
  int crecv[mpisize];
  for (i = 0; i < mpisize; i++)
      crecv[i] = 0;
  MPI_Alltoall(csent, 1, MPI_INT, crecv, 1, MPI_INT, MPI_COMM_WORLD);
  int vecsize = 0;
  for (i = 0; i < mpisize; i++)
      vecsize += crecv[i];
  int *vecord;
  vecord = calloc(vecsize, sizeof(int));
  int sdispls[mpisize], rdispls[mpisize];
  int spos = 0, rpos = 0;
  for (i = 0; i < mpisize; i++) {
      sdispls[i] = spos;
      spos += csent[i];
      rdispls[i] = rpos;
      rpos += crecv[i]; 
  }
  MPI_Alltoallv(vec, csent, sdispls, MPI_INT, vecord, crecv, rdispls, MPI_INT, MPI_COMM_WORLD);

  /* local sort */
  qsort(vecord, vecsize, sizeof(int), compare);

  /* time count ends */
  MPI_Barrier(MPI_COMM_WORLD);
  tend = MPI_Wtime();

  /* every processor writes its result to a file */
  FILE* fd = NULL;
  char filename[256];
  snprintf(filename, 256, "sort_rank%02d.txt", rank);
  fd = fopen(filename, "w+");

  if (NULL == fd) {
      printf("Error opening file!\n");
      return 1;
  }
  fprintf(fd, "rank %d sorted data of size %d:\n", rank, vecsize);
  for (i = 0; i < vecsize; i++)
      fprintf(fd, "%d\n", vecord[i]);
  fclose(fd);


  // print timing
  for (i = 0; i < mpisize; ++i) {
      MPI_Barrier(MPI_COMM_WORLD);
      if (i == rank) {
         printf("sorting %04d elements on rank %d takes %f s.\tmin = %d, max = %d.\n", vecsize, rank, tend-tbegin, vecord[0], vecord[vecsize-1]);
      }
  }
  free(vec);
  free(splitter);
  free(vecord);
  MPI_Finalize();
  return 0;
}
