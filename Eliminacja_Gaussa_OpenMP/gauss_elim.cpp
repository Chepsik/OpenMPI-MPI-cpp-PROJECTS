#include <iostream> // standardowa biblioteka c++
#include <fstream>  // biblioteka do zarządzania plikami
#include <string>   // biblioteka stringów c++
#include <omp.h>    // biblioteka openmp
#include <sstream>  // bibloiteka posiadająca narzędzie double -> string
#include <time.h>   // biblioteka do liczenia czasu
#include <iomanip>  // biblioteka posiadająca narzędzia formatowania tekstu
//  KOMPILACJA
//  g++ -fopenmp gauss_elim.cpp
//  OMP_NUM_THREADS=4 ./a.out
//  BARDZO WAŻNE JEST OKREŚLENIE ILOŚCI WĄTKÓW. Czasem bez określenia tej zmiennej openmp odpala się na jednym wątku, przez co oczywiście czas pracy jest gorszy.
using namespace std;

int main(){
  // deklaracja zmiennych
  ifstream file;      // deklaracja pliku, który będziemy otwierać
  char temp[2000];    // zmienna pomocnicza do zapisywania numerów z pliku (nie musi być to tablica z aż 2000 pozycjami, moze ich byc mniej)
  int array_size;     // wielkość tablicy (pierwsza linia w pliku)
  int i = 0;          // zmienna pomocnicza przy iteracjach

  // zmienne do obliczania czasu poszczególnych operacji
  clock_t start, end;
  double cpu_time_used_openmp, cpu_time_used_seq;

  // CZYTANIE Z PLIKU
  file.open("data.csv", ifstream::in);                  // TUTAJ NALEŻY ZMIENIĆ NAZWĘ z jakiego pliku chcemy pobierać dane, nie było go w wymaganiach więc jest data.csv
  if (file.is_open()) {                                 // CZYTAMY PIERWSZĄ LINIĘ i przypisujemy rozmiar tablicy do array_size
    file.getline(temp, 256);
    array_size = atoi(temp);
  }
  // KONIEC CZYTANIA Z PLIKU


  // DYNAMICZNA DEKLARACJA TABLIC DWUWYMIAROWYCH(dane) I JEDNOWYMIAROWEJ(wyniki)
  // Program tworzy 2 identyczne tablice, robi to ze względu na to, że podczas obliczania (zarówno OpenMP jak i sekwencyjnego)
  // algorytm modyfikuje je. Gdybyśmy operowali na jednej tablicy to otrzymalibyśmy błędne obliczenia dla drugiego algorytmu.
  double** gauss_array = new double*[array_size]; // Wykorzystana w algorytmie openMP
  double** copy_gauss = new double*[array_size];  // Wykorzystana w algorytmie sekwencyjnym

  for(int i = 0; i < array_size; ++i){
    gauss_array[i] = new double[array_size];      // open_mp
    copy_gauss[i] = new double[array_size];       // seq
  }

  double* result_array = new double[array_size];  // open_mp
  double* copy_result = new double[array_size];   // seq
  // KONIEC DYNAMICZNYCH DEKLARACJI


  // CZYTANIE Z PLIKU CIĄG DALSZY
  // Tutaj przypisujemy odpowiednie wartości na odpowiednie pozycje tablic zadeklarowanych wyżej
  if(file.is_open()){
    for(int i = 0; i<array_size; ++i){
      for(int j = 0; j<=array_size; ++j){
        if(j == array_size){                            // ten if to specjalny wyjątek na ostatnią liczbę w pliku (ona jako jedyna nie zawiera ; po sobie więc)
          file.getline(temp, sizeof(temp));             // nie możemy używać getline z ustawionym 'delimeterem' tak jak jest to pokazane w przypadku else
          result_array[i] = atof(temp);
          copy_gauss[i][j] = atof(temp);
          copy_result[i] = atof(temp);
        }
        else{
          file.getline(temp, sizeof(temp), ';');
          gauss_array[i][j] = atof(temp);
          copy_gauss[i][j] = atof(temp);
        }
      }
    }
    file.close();
  }
  // KONIEC CZYTANIA Z PLIKU


  // ZAKOMENTOWANY KOD SPRAWDZAJĄCY CZY PLIK ZOSTAŁ DOBRZE WCZYTANY (WYŚWIETLA TABLICE NA WYJŚCIE)
  // for(int i = 0; i < array_size; i++){
  //   for(int j = 0; j < array_size; j++){
  //     cout<<fixed;
  //     cout<<gauss_array[i][j]<<" "<< setprecision(6);
  //   }
  //   cout<<endl;
  // }


  // deklaracja zmiennych wykorzystanych przy obliczaniu openmp
  int norm, row, col, multiplier;
  double *sub = new double[array_size];

  //  CZĘŚĆ OPENMP
  //  Tutaj jest elimacja Gaussa z wykorzystaniem OPENMP
  //  Jest ona bardzo podobna do klasycznej -> sekwencyjnej eliminacji umieszczonej niżej, jednakże
  //  wykorzystuje ona obliczanie równoległe.
  //
  //  komentarz do linii pragma omp parallel...
  //  Linia ta odpowiada za rozpoczęcie działania równoległego - tj. kod poniżej pragmy będzie wykonany przez wszystkie procesory jednocześnie
  //  ważne jest tutaj aby upewnić się czy zmienne z których korzystają procesory nie będą wykorzystane w tym samym czasie,
  //  odpowiada za to linia shared(gauss_array, result_array), która mówi, że te zasoby będą wykorzystywane wspólnie (tj. mogą być wykorzystywane w tym samym czasie)
  //  oraz priate(multiplier, row, col) która mówi, że te zmienne są prywatne (tj. każdy procesor ma własną instancje danej zmiennej). Dzięki temu te zmienne nie nachodzą na siebie podczas obliczeń.


  start = clock();  // odpalamy zegar dla openMP
  for (norm = 0; norm < array_size - 1; norm++) {
    #pragma omp parallel for shared(gauss_array, result_array) private(multiplier,row,col)
      for (row = norm + 1; row < array_size; row++) {                                                 // tutaj przeprowadzane są tzw. operacje elementarne
        double multiplier = gauss_array[row][norm] / gauss_array[norm][norm];                         // aby sprowadzić macierz do postaci schodkowej
        for (col = norm; col < array_size; col++) {                                                   // którą następnie można wykorzystać do poszukiwania
          gauss_array[row][col] -= gauss_array[norm][col] * multiplier;
        }
        result_array[row] -= result_array[norm] * multiplier;
      }
  }
  end = clock();  // zapisujemy miejsce zakończenia obliczeń openMP
  cpu_time_used_openmp = ((double) (end - start)) / CLOCKS_PER_SEC;                                 // obliczamy czas pomiędzy end a start
  // CZĘŚĆ OPENMP - KONIEC

  // Tutaj jest już tylko podstawienie wsteczne do obliczenia wyniku, podobna operacja jest po sprowadzeniu macierzy do postaci schodkowej przy algorytmie sekwencyjnym.
  // dzięki niemu wracamy do "pierwotnej" formy macierzy, tych obliczeń nie wykonujemy już równolegle
  for (int row = array_size - 1; row >= 0; row--) {
    sub[row] = result_array[row];
    for (int col = array_size-1; col > row; col--) {
      sub[row] -= gauss_array[row][col] * sub[col];
    }
    sub[row] /= gauss_array[row][row];
  }


  // CZĘŚĆ SEKWENCYJNA
  // Zasada działania jest identyczne jak w przypadku openmp tylko bez równoległości
  // kod poniżej również wykonuje operacje elementarne
  // jedną z różnic jest również to że tutaj operujemy już na drugiej kopii naszej macierzy tj. copy_gauss i copy_result
  start = clock();      // rozpoczynamy odliczanie
  for(int j=0; j<array_size; j++)
  {
    for(int i=0; i<array_size; i++)
    {
      if(i!=j)
      {
        double c = copy_gauss[i][j]/copy_gauss[j][j];
        for(int k=0; k<=array_size; k++)
        {
          copy_gauss[i][k]=copy_gauss[i][k]-c*copy_gauss[j][k];
        }
      }
    }
  }

  // podstawienie wsteczne
  for(int i=array_size-1; i>=0; i--)
  {
    double sum = 0;
    for(int j = i+1; j < array_size; j++)
    {
      sum = sum + copy_gauss[i][j] * copy_result[j];
    }
    copy_result[i]=(copy_gauss[i][array_size]-sum)/copy_gauss[i][i];
  }
  end = clock();    // kończymy odliczanie
  cpu_time_used_seq = ((double) (end - start)) / CLOCKS_PER_SEC;                // obliczamy czas pomiędzy end a start
  // CZĘŚĆ SEKWENCYJNA - KONIEC

  // WYŚWIETL WYNIK OPENMP
  // for(int i = 0; i < array_size; i++){
  //   cout<<sub[i]<<" ";
  // }
  // cout<<"czas omp: "<<fixed<<cpu_time_used_openmp<<endl;

  // WYŚWIETL WYNIK SEKWENCYJNY
  // for(int i=0; i<array_size; i++)
  // {
  //   cout<<copy_result[i]<<" ";
  // }
  // cout<<"czas seq: "<<fixed<<cpu_time_used_seq<<endl;


  // ZAPIS DO PLIKU
  // poniższe linie to konwersja czasu seq i omp na string.
  ostringstream strs;
  strs << cpu_time_used_seq;
  string cpu_1 = strs.str();
  ostringstream strs_2;
  strs_2 << cpu_time_used_openmp;
  string cpu_2 = strs_2.str();
  // koniec konwersji

  string file_name = "C_" + cpu_1 + "_" + cpu_2 + ".csv";                       // utworzenie nazwy pliku (identyczna jak w zadaniu)

  ofstream result_file(file_name);                                              // utworzenie pliku zapisowego o nazwie file_name

  for(int i = 0; i<array_size; i++){                                            // kolejno zapisujemy elementy w formie csv to pliku
      if(i!=array_size-1){
        result_file << fixed << setprecision(6) << sub[i] << ';';               // jeżeli zapisywany element nie jest ostatni to dopisujemy ;
      }
      else{
        result_file << fixed << setprecision(6) << sub[i];                      // jeśli zapisywany element jest ostatni to nie dopisujemy ;
      }
  }
  // KONIEC ZAPISU DO PLIKU
  return 0;
}
