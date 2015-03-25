EXECUTABLES = mpi_solved1 mpi_solved2 mpi_solved3 mpi_solved4 mpi_solved5 mpi_solved6 mpi_solved7  ssort
COMPILER = mpicc
FLAGS = -O3 -Wall

all: $(EXECUTABLES)

mpi_solved: mpi_solved1.c mpi_solved2.c mpi_solved3.c mpi_solved4.c mpi_solved5.c mpi_solved6.c mpi_solved7.c
	$(COMPILER) $(FLAGS) mpi_solved1.c -o mpi_solved1
	$(COMPILER) $(FLAGS) mpi_solved2.c -o mpi_solved2
	$(COMPILER) $(FLAGS) mpi_solved3.c -o mpi_solved3
	$(COMPILER) $(FLAGS) mpi_solved4.c -o mpi_solved4
	$(COMPILER) $(FLAGS) mpi_solved5.c -o mpi_solved5
	$(COMPILER) $(FLAGS) mpi_solved6.c -o mpi_solved6
	$(COMPILER) $(FLAGS) mpi_solved7.c -o mpi_solved7

ssort: ssort.c
	$(COMPILER) $(FLAGS) ssort.c -o ssort

clean:
	rm -rf $(EXECUTABLES) sort_rank*.txt
