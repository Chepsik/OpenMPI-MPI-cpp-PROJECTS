#include <stdlib.h> // standard C library
#include <mpi.h>    // MPI library
#include <time.h>   // Time library

//  KOMPILACJA
//  mpicxx -o runfile fiverr_mpi.cpp
//  po użyciu powyższej komendy utworzony zostanie plik runfile, który możemy uruchomić za pomocą:
//  mpiexec -N 4 runfile
//  gdzie -N odpowiada za liczbę użytych procesorów (w powyższym przypadku są to 4 procesory), oczywiście liczba ta nie może
//  przewyższać możliwości komputera.

//  KOD

// Tutaj umieszczona jest funkcja, w tym przypadku jest to Ax^2+Bx+C, ale może to być również inna funkcja
double f1(double x, double a, double b, double c) {
  return (a*x*x + b*x + c);
}

// Główny kod programu
int main(int argc, char *argv[]){
  // Deklarujemy utworzenie MPI
  MPI_Init(&argc, &argv);

  // inicjujemy zmienne dla każdego procesu
  int a,b,c;       // A, B i C z zadania
  int lim_a,lim_b; // a i b z zadania
  long long int n; // n z zadania (liczba operacji)
  double h;        // h - wykorzystana przy obliczaniu całki

  double result; // wynik obliczeń
  clock_t start, end; // zmienne pomocnicze do obliczania czasu pracy programu


  int world_rank;
  int world_size;
  // przypisz liczbę używanych procesorów do world_size
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // przypisz numer procesora do world_rank
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // poniższy kod wykona tylko proces z numerem 0 (węzeł 0) wpisujemy tam dane wejściowe
  if(world_rank == 0){
    printf("Korzystasz z %d procesorów\n", world_size);

    printf("Wpisz A\n");
    scanf("%d",&a);

    printf("Wpisz B\n");
    scanf("%d",&b);

    printf("Wpisz C\n");
    scanf("%d",&c);

    printf("Wpisz granicę całkowania a\n");
    scanf("%d",&lim_a);

    printf("Wpisz granicę całkowania b\n");
    scanf("%d",&lim_b);

    printf("Wpisz n - liczbę podziałów\n");
    scanf("%lld",&n);

    h = (lim_b-lim_a)/double(n); // obliczamy h - identyczne dla każdego procesu
  }

  // rozsyłamy dane wpisane w procesorze 0 do innych procesów
  MPI_Bcast(&a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&b, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&c, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lim_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&lim_b, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&n, 1, MPI_LONG_LONG_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&h, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  // koniec rozsyłania

  // dane są rozesłane, rozpoczynamy odliczanie czasu pracy
  start = clock();

  // deklarujemy zmienne różne dla każdego procesu (każdy procesor będzie obliczać tylko część całki)
  int local_n = n/world_size;                         // liczba operacji na jeden procesor
  double local_a = lim_a + world_rank * local_n * h;  // początek przedziału obliczeń dla jednego procesora
  double local_b = local_a + local_n * h;             // koniec przedziału obliczeń dla jednego procesora


  // lokalna zmienna pomocnicza
  double x = local_a;

  // lokalny wynik
  double integral = (f1(local_a, a, b, c)+f1(local_b, a, b, c))/2.0;

  // funkcja obliczająca całkę oznaczoną
  for(int i = 1; i <= local_n-1; i++) {
    x += h;                                           // obliczamy nowe x, a nastepnie
    integral += f1(x, a, b, c);                       // wykorzystujemy je do pozyskania kolejnej części całki, a, b i c są tutaj stałe zmienia się jedynie x
  }
  integral = integral*h;

  //printf("%f\n", integral); // wyniki poszczególnych obliczeń procesów

  MPI_Reduce(&integral, &result, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);  // odsyłamy obliczone całki lokalne do procesu o numerze 0 i sumujemy je do zmiennej result

  end = clock();  // skończyliśmy obliczanie, więc zatrzymujemy zegar

  // Wyświetlamy wyniki korzystając z procesu o numerze 0
  if(world_rank == 0){
      double cpu_time_used;                                                   // zmienna pomocnicza
      cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;              // tutaj obliczamy liczbę sekund potrzebną na realizację programu.
      printf("Zadanie obliczono w %f sekund.\n", cpu_time_used);              // wyświetlanie czasu realizacji zadania
      printf("Calka: %f\n", result);                                          // wyświetlanie wyniku.
  }

  MPI_Finalize(); // KONIEC OBLICZEŃ
  return 0;
}

//  KOD
