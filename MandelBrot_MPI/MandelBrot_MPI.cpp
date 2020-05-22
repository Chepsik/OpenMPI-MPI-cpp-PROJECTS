#include <iostream>
#include <mpi.h>
#include <string>
#include <cstdlib>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>


using namespace std;

struct point {
    double x;
    double y;
};

point dodaj(point punkt_1, point punkt_2){
    point wynik;
    wynik.x = punkt_1.x + punkt_2.x;
    wynik.y = punkt_1.y + punkt_2.y;
    return wynik;
}

point mnozenie(point punkt_1, point punkt_2){
    point wynik;
    wynik.x = (punkt_1.x*punkt_2.x-punkt_1.y*punkt_2.y);
    wynik.y = (punkt_1.x*punkt_2.y+punkt_1.y*punkt_2.x);
    return wynik;
}

int mandelbrot(point p){
    point pp;
    int N = 10000;
    pp.x=0.0;
    pp.y=0.0;
    if (N!=0){
        for (int i =0; i<N;i++)
        {
        pp = dodaj(mnozenie(pp,pp),p);
        if ((pp.x*pp.x)+(pp.y*pp.y)>=2.0){
            return 0;
        }
        }
    }
    return 1;
}

int main(int argc, char *argv[]){
  MPI_Init(&argc, &argv);

  int world_rank,world_size;
  double x_min,x_max,y_min,y_max;
  point p;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  if(world_rank == 0)
  {
    fstream file_in;
    file_in.open("mandelbrot.in");
    file_in >> x_min;
    file_in >> x_max;
    file_in >> y_min;
    file_in >> y_max;
    file_in.close();
  }
  MPI_Bcast(&x_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&x_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&y_min, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&y_max, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  int array[180][120] = {};
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  for (int i = world_rank; i < 120; i += world_size)
  {
      for (int j = 0; j < 180; ++j)
      {
          p.x = x_min + (j*((x_max-x_min)/180));
          p.y = y_max - (i*((y_max-y_min)/120));
          array[j][i] = mandelbrot(p);
      }
  }
  int wynik[180][120];
  MPI_Reduce(array, wynik, 180*120, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  if(world_rank == 0){
    fstream zapis;
    zapis.open("mandelbrot.out", ios::out | ios::trunc);
    for(int i = 0; i < 120; i++){
      for(int j = 0; j < 180; j++){
        if(wynik[j][i] == 1){
          zapis << "X";
        }
        else{
          zapis << " ";
        }
      zapis << "\n";
    }
    zapis.close();
    }
  }
  MPI_Finalize();
  return 0;
}
