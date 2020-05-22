#include <iostream> // standardowa biblioteka c++
#include <fstream>  // biblioteka do zarządzania plikami
#include <string>   // biblioteka stringów c++
#include <sstream>  // bibloiteka posiadająca narzędzie double -> string
#include <iomanip>  // biblioteka posiadająca narzędzia formatowania tekstu
#include <mpi.h>    // biblioteka MPI


//  KOMPILACJA
//  mpicxx -o runfile gauss_mnoz.cpp
//  mpiexec -N 4 runfile
using namespace std;

int main(int argc, char *argv[]){
  MPI_Init(&argc, &argv);

  int world_size;
  int world_rank;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

  // rozmiary macierzy A i B, total jest elementem wspólnym rozmiar (tzw. n)
  int array_size_A_m = 0;
  int array_size_total = 0;
  int array_size_B_p = 0;

  // zmienna informująca o tym czy macierz jest kwadratowa (jest używana przy czytaniu danych z plików)
  bool square = false;


  if(world_rank == 0){
    ifstream file;      // deklaracja pliku, który będziemy otwierać
    char temp[2000];    // zmienna pomocnicza do zapisywania numerów z pliku (nie musi być to tablica z aż 2000 pozycjami, moze ich byc mniej)

        // CZYTANIE Z PLIKU MACIERZ A
    file.open("matrix_A.csv", ifstream::in);                  // TUTAJ NALEŻY ZMIENIĆ NAZWĘ z jakiego pliku chcemy pobierać dane, nie było go w wymaganiach więc jest data.csv
        if (file.is_open()) {
          string line;
          file.getline(temp, 256, ';');
          // sprawdzamy czy pierwsza liczba jest floatem czy int'em, jeżeli to float to wtedy wiemy, że jest to macierz kwadratowa, ale nie wiemy jakiego rozmiaru
          // sprawdzanie działa następująco: zamieniamy temp na float i odejmujemy ten float od samego siebie ale w postaci int. Jeśli odejmowanie od siebie jest różne od 0 to temp na pewno jest floatem.
          if(atof(temp)-int(atof(temp)) != 0){
            while(getline(file, line))
            {
              // tutaj podliczamy wszystkie linie w pliku, czyli rozmiar macierzy kwadratowej
              array_size_A_m++;
            }
            // przepisujemy ten rozmiar do drugiej zmiennej odpowiadającej za wymiary
            array_size_total = array_size_A_m;

          }
          // kod poniżej wykonujemy jesli macierz nie jest kwadratowa
          else{
            // zamykamy plik aby wrócić do pierwszej linii
            file.close();
            file.open("matrix_A.csv", ifstream::in);
            // pierwszy wymiar
            file.getline(temp, 256);
            array_size_total = atoi(temp);
            // drugi wymiar
            file.getline(temp, 256);
            array_size_A_m = atoi(temp);
            square = true;
          }
          file.close();
        }

        // CZYTANIE Z PLIKU MACIERZ B KOD JEST IDENTYCZNY JAK W PRZYPADKU MACIERZY A
    ifstream file_b;
    file_b.open("matrix_B.csv", ifstream::in);                  // TUTAJ NALEŻY ZMIENIĆ NAZWĘ z jakiego pliku chcemy pobierać dane, nie było go w wymaganiach więc jest data.csv
        if (file_b.is_open()) {
          string line;
          file_b.getline(temp, 256, ';');
          if(atof(temp)-int(atof(temp)) != 0){
            while(getline(file_b, line))
            {
              array_size_B_p++;
            }
          }
          else{
            file_b.close();
            file_b.open("matrix_B.csv", ifstream::in);
            file_b.getline(temp, 256);
            array_size_B_p = atoi(temp);
            square = true;
          }
          file.close();
        }
        //cout<<array_size_total<<" "<<array_size_A_m<<" "<<array_size_B_p<<endl;
    }
    // ROZSŁYAMY INFORMACJĘ O ROZMIARACH MACIERZY DO RESZTY PROCESÓW
    MPI_Bcast(&array_size_total, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&array_size_A_m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&array_size_B_p, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // TWORZYMY INSTANCJE MACIERZY W KAŻDYM PROCESIE
    float **matrix_A = (float **)malloc(array_size_total*sizeof(float *));
    for(int i=0;i<array_size_total;i++){
      matrix_A[i] = (float *)malloc(array_size_A_m*sizeof(float));
    }
    float **matrix_B = (float **)malloc(array_size_B_p*sizeof(float *));
    for(int i=0;i<array_size_B_p;i++){
      matrix_B[i] = (float *)malloc(array_size_total*sizeof(float));
    }
    // KONIEC DYNAMICZNYCH DEKLARACJI


    // CZYTANIE Z PLIKU CIĄG DALSZY
    // Tutaj przypisujemy odpowiednie wartości na odpowiednie pozycje tablic zadeklarowanych wyżej robimy to tylko w procesie o numerze 0
  if(world_rank == 0){
      ifstream file;      // deklaracja pliku, który będziemy otwierać
      ifstream file_b;    // plik z macierzą B
      char temp[2000];    // zmienna pomocnicza do zapisywania numerów z pliku (nie musi być to tablica z aż 2000 pozycjami, moze ich byc mniej)
      file.open("matrix_A.csv", ifstream::in);
      if(square){
        // jeśli macierz jest kwadratowa to musimy przeczytać dwie pierwsze linie żeby rozmiar nie był przeczytany jako element macierzy
        file.getline(temp, 256);
        file.getline(temp, 256);
      }
      if(file.is_open()){
        for(int i = 0; i<array_size_total; ++i){
          for(int j = 0; j<array_size_A_m; ++j){
            if(j != array_size_A_m-1){
              // czytanie nieostatniego elementu
            file.getline(temp, sizeof(temp), ';');
            }
            else{
              // czytanie ostatniego elementu
            file.getline(temp, sizeof(temp));
            }
            // wpisanie elementu do macierzy A
            matrix_A[i][j] = atof(temp);
          }
        }
        // koniec czytania - zamykamy plik
        file.close();
      }
      // KOD IDENTYCZNY JAK W PRZYPADKU CZYTANIA MACIERZY A, jednak DLA PLIKU Z MACIERZĄ B
      file_b.open("matrix_B.csv", ifstream::in);
      if(square){
        file_b.getline(temp, 256);
        file_b.getline(temp, 256);
      }
      if(file_b.is_open()){
        for(int i = 0; i<array_size_B_p; ++i){
          for(int j = 0; j<array_size_total; ++j){
            if(j != array_size_total-1){
            file_b.getline(temp, sizeof(temp), ';');
            }
            else{
            file_b.getline(temp, sizeof(temp));
            }
            matrix_B[i][j] = atof(temp);
          }
        }
        file_b.close();
      }
    // KONIEC CZYTANIA Z PLIKU
  }


  // ROZSYŁAMY MACIERZE A I B DO INNYCH PROCESÓW
  for(int i = 0; i < array_size_total; i ++){
    // przesyłanie rzędów macierzy
    MPI_Bcast(&matrix_A[i][0], array_size_A_m, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }
  for(int i = 0; i < array_size_B_p; i ++){
    // przesyłanie rzędów macierzy
    MPI_Bcast(&matrix_B[i][0], array_size_total, MPI_FLOAT, 0, MPI_COMM_WORLD);
  }



  // // ZAKOMENTOWANY KOD SPRAWDZAJĄCY CZY PLIK ZOSTAŁ DOBRZE WCZYTANY (WYŚWIETLA TABLICE NA WYJŚCIE)
  // if(world_rank == 2){       // najlepiej sprawdzić go dla procesu z world_rank != 0 aby sprawdzić czy wszystko dobrze działa
  //   for(int i = 0; i < array_size_total; i++){
  //     for(int j = 0; j < array_size_A_m; j++){
  //       cout<<fixed;
  //       cout<<matrix_A[i][j]<<" "<< setprecision(6);
  //     }
  //     cout<<endl;
  //   }
  //   cout<<endl<<endl;
  //   for(int i = 0; i < array_size_B_p; i++){
  //     for(int j = 0; j < array_size_total; j++){
  //       cout<<fixed;
  //       cout<<matrix_B[i][j]<<" "<< setprecision(6);
  //     }
  //     cout<<endl;
  //   }
  // }

  // TWORZYMY DYNAMICZNĄ MACIERZ C KTÓRA BĘDZIE TRZYMAŁA WYNIKI KOLEJNYCH OPERACJI NA POSZCZEGÓLNYCH PROCESACH
  float **result_C = (float **)malloc(array_size_B_p*sizeof(float *));
  for(int i=0;i<array_size_B_p;i++){
    result_C[i] = (float *)malloc(array_size_A_m*sizeof(float));
  }

  // ZERUJEMY TE MACIERZ
  for(int i = 0; i < array_size_B_p; i++){
    for(int j = 0; j < array_size_A_m; j++){
      result_C[i][j] = 0;
    }
  }

  // Zmienne pomocnicze wykorzystywane przy zliczaniu czasu
  double starttime, endtime;

  // MNOŻENIE MACIERZY
  starttime = MPI_Wtime();
  // PROCESY PRZESKAKUJĄ O TYLE WIERSZY JAKI JEST ROZMIAR WORLD_SIZE - czyli ilości działających procesów.
  // Dzięki temu każdy proces będzie odpowiadał za jeden rząd.
  for(int i = world_rank; i < array_size_B_p; i+=world_size){
    float tmp = 0;
    // te operacje są identyczne dla każdego procesu. Tj. Jeden proces odpowiada za wynik dla całego rzędu nowej macierzy.
    for(int j = 0; j < array_size_A_m; j++){
      for(int k = 0; k < array_size_total; k++){
        tmp += matrix_B[i][k] * matrix_A[k][j];
      }
      result_C[i][j] = tmp;
    }
  }
  // kończymy zliczanie czasu, gdyż skończyliśmy mnożyć
  endtime   = MPI_Wtime();
  // KONIEC MNOŻENIA MACIERZY

  // Tworzymy kolejną dynamiczną tablice, która zostanie wykorzystana do zsumowania tablic wynikowych poszczególnych procesów (Macierzy C)
  float **final = (float **)malloc(array_size_B_p*sizeof(float *));
  for(int i=0;i<array_size_B_p;i++){
    final[i] = (float *)malloc(array_size_A_m*sizeof(float));
  }
  // Zerujemy te tablice.
  for(int i = 0; i < array_size_B_p; i++){
    for(int j = 0; j < array_size_A_m; j++){
      final[i][j] = 0;
    }
  }

  // PRZESYŁANIE TABLICY DO PROCESU 0 I SUMOWANIE JEJ ELEMENTÓW
  for(int i = 0; i < array_size_B_p; i++){
    for(int j = 0; j < array_size_A_m; j++){
      // Ponieważ każdy proces ma u siebie nie tylko rezultaty swoich obliczeń, ale również zapełnione zerami puste miejsca, których nie obliczył
      // to możemy dodać te miejsca wraz z innymi procesorami, gdyż te 0 nie wpłyną na nasz wynik.
      MPI_Reduce(&result_C[i][j], &final[i][j], 1, MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
    }
  }

  double mpi_time;  // zmienna zawierająca informacje o czasie działania z mpi
  double seq_time;  // zmienna zawierająca informacje o czasie działania sekwencyjnego
  mpi_time = endtime - starttime;

  if(world_rank == 0){
    // TEST CZASOWY DLA JEDNEGO PROCESU
    starttime = MPI_Wtime();
    for(int i = 0; i < array_size_B_p; i++){
      float tmp = 0;
      for(int j = 0; j < array_size_A_m; j++){
        for(int k = 0; k < array_size_total; k++){
          tmp += matrix_B[i][k] * matrix_A[k][j];
        }
        result_C[i][j] = tmp;
      }
    }
    endtime = MPI_Wtime();
    // TEST CZASOWY DLA JEDNEGO PROCESU
    seq_time = endtime - starttime;

    // Tutaj wykonuje operacje konwersji, identyczne jak w poprzednim zadaniu.
    // Tak naprawdę to każda niżej umieszczona operacja jest identyczna jak w poprzednim zadaniu, zarówno konwersja jak i zapis do pliku.
    ostringstream strs;
    strs << setprecision(4) << mpi_time;
    string cpu_1 = strs.str();    //Tp
    ostringstream strs_2;
    strs_2 << setprecision(4) << seq_time;
    string cpu_2 = strs_2.str();  //T1
    // // koniec konwersji
    //
    string file_name = "C_" + cpu_2 + "_" + cpu_1 + ".csv";                       // utworzenie nazwy pliku (identyczna jak w zadaniu)

    ofstream result_file(file_name);                                              // utworzenie pliku zapisowego o nazwie file_name

    for(int i = 0; i<array_size_B_p; i++){
      for(int j = 0; j<array_size_A_m; j++){
        if(i!=array_size_A_m-1){
          result_file << fixed << setprecision(4) << result_C[i][j] << ';';               // jeżeli zapisywany element nie jest ostatni to dopisujemy ;
        }
        else{
          result_file << fixed << setprecision(4) << result_C[i][j];                      // jeśli zapisywany element jest ostatni to nie dopisujemy ;
        }
      }
    }
  }
  // KONIEC ZAPISU DO PLIKU
  MPI_Finalize();
  return 0;
}
