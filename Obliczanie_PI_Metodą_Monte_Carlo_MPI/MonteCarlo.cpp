#include <stdlib.h> // standard C library
#include <mpi.h>    // MPI library
#include <time.h>   // standard time library, used to calculate different times for different amount of processors

// This code can be compile by mpicxx compiler.
// ie. 'mpicxx -o runfile MonteCarlo.cpp'
// After that you'll get runfile file which you can execute with this command:
// mpiexec -N 2 runfile
// where -N determines how much processors you want to include when running runfile (you can't use more than your computers have)
// To include other computers in your calculations you need to specify them by using -host flag.
// mpiexec -N 16 -host vh2,vh4,vh6,vh8 runfile
// Right now I will use 16 processors on machines with names vh2, vh4, vh6, vh8

// Number of tosses is given by using scanf()

int MonteCarlo(unsigned long long int number_of_tosses){
  unsigned long long int number_in_circle = 0;
  int world_rank;
  // Seed generation, since processes can start in the same moment, it is very likely to have the same seed. Since world_rank value is different for every process, every seed'll be different.
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  unsigned int seed = time(NULL) + world_rank;
  // Here we're going to take random x and y and calcucate the distance to estimate if the point is in the circle
  for(unsigned long long int toss = 0; toss < number_of_tosses; toss++){
      double x = static_cast <float> (rand_r(&seed)) / static_cast <float> (RAND_MAX); // random double
      double y = static_cast <float> (rand_r(&seed)) / static_cast <float> (RAND_MAX); // random double
      double distance_squared = x*x + y*y;
      if(distance_squared <= 1){
          number_in_circle++; // if the point is in the circle we're going to increment number_in_circle value;
      }
      // else we're not doing anything
  }
  // After number_of_tosses calculation MonteCarlo returns number of points in circle
  return number_in_circle;
}

int main(int argc, char *argv[]){
  MPI_Init(&argc, &argv);         // Initialize MPI

  unsigned long long int initial_number_of_tosses;  // declare number_of_tosses
  int world_rank;                 // integer that'll holds information about processor number (0,1,2,..etc).
  int world_size;                 // integer that'll holds information about size number of using processors (ie. 2,4,8)
  unsigned long long int numbers_in_circle; // unsigned integer that holds information about how many points there're in the circle.

  clock_t start, end;
  double cpu_time_used;

  unsigned long long int global_sum_of_points_in_circle = 0; // this integer will be used by process with world_rank = 0, we'll estimate pi with a help of this integer

  // Get the number of processes
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Get the rank of the process
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if(world_rank == 0){
    printf("You're using %d processors\n", world_size);

    printf("How many tosses do you want to calculate?\n");
    scanf("%ld",&initial_number_of_tosses);  // Specify number of tosses

    printf("Since you're using %d processors, each will calculate %ld points.\n",world_size, initial_number_of_tosses/world_size);
    start = clock();  // We're starting to count seconds.
  }

  unsigned long long int number_of_tosses = initial_number_of_tosses/world_size;  // Here We're calculating how many tosses we should execute on one processor. ie. if we want 1000 tosses for 4 processes, one's going to calculate 1000/4 = 250.
  // We're sending information about number_of_tosses to execute to every processor that we're using.
  MPI_Bcast(&number_of_tosses, 1, MPI_INT, 0, MPI_COMM_WORLD);

  //printf("%d\n", number_of_tosses); // you can check here how many tosses one processor's going to execute

  numbers_in_circle = MonteCarlo(number_of_tosses);  // now every processor's going to execute "MonteCarlo" function and assign the return value (numbers of point in the circle) to number_in_circle integer

  MPI_Reduce(&numbers_in_circle, &global_sum_of_points_in_circle, 1, MPI_LONG_LONG_INT, MPI_SUM, 0, MPI_COMM_WORLD); // MPI_Reduce takes numbers_in_circle value from every processor and take's sum of it, then it returns value of this sum to global_sum_of_points_in_circle.

  // Now again we're using world_rank = 0 processor to estimate pi value.
  if(world_rank == 0){
      end = clock();  // stop counting
      printf("There are %ld points in the circle\n", global_sum_of_points_in_circle);
      double pi_estimate = 4 * global_sum_of_points_in_circle /(double) initial_number_of_tosses; // estimating pi value.
      cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;                                  // this is how many seconds we were calculating pi.
      printf("Estimation of pi: %f, calculated in %f seconds.\n", , cpu_time_used);
  }

  MPI_Finalize(); // Because there're no other calculations we can tell our processors to stop this program by using MPI_Finalize
  return 0;
}
