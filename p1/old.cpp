#include <omp.h>
#include <stdio.h>
#include <math.h>


#ifndef NUMT
#define NUMT 1  
#endif

#ifndef SIZE
#define SIZE 16000
#endif

#ifndef NUMTRIES
#define NUMTRIES 10
#endif


float A[SIZE];
float B[SIZE];
float C[SIZE];


int main() {

     for(int i = 0; i < SIZE; i++) {
          A[i] = 2.;
          B[i] = 3.;
     }

     omp_set_num_threads(NUMT);
     fprintf(stderr, "Using %d threads.\n", NUMT);

     double maxMegaMults = 0.;
     double sumMegaMults = 0.;
     
     for(int t = 0; t < NUMTRIES; t++) {
          double time0 = omp_get_wtime();

          #pragma omp parallel for
          for(int i = 0; i < SIZE; i++) {
               C[i] = A[i] * B[i];
          }

          double time1 = omp_get_wtime();
          double megaMults = (double)SIZE / (time1 - time0) / 1000000.;
    
          sumMegaMults += megaMults;
          if(megaMults > maxMegaMults) {
               maxMegaMults = megaMults;
          }
     }

     double avgMegaMults = sumMegaMults / NUMTRIES;

     //printf("Threads = %d | Array Size = %d | Tries = %d\n", NUMT, SIZE, NUMTRIES); 
     //printf("Peak Performance = %8.2lf MegaMults/sec.\n", maxMegaMults);
     //printf("Avg Performace = %8.2lf MegaMults/sec.\n", avgMegaMults);
     printf("%8.2lf", avgMegaMults);

     return 0;

}
