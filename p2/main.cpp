#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>


// setting the number of threads:
#ifndef NUMT
#define NUMT 1
#endif

// setting the number of nodes for the base of the superquadric:
#ifndef NUMNODES
#define NUMNODES 4
#endif

// setting the power for which the superqadric is raised:
#ifndef N
#define N 4
#endif

// setting the number of tries to discover the maximum performance:
#ifndef NUMTRIES
#define NUMTRIES 10
#endif


#define XMIN -1.
#define XMAX 1.
#define YMIN -1.
#define YMAX 1.


float height(int, int);


// main program:
int main( int argc, char *argv[ ] ) {
   #ifndef _OPENMP
   fprintf( stderr, "No OpenMP support!\n" );
   return 1;
   #endif

   float fullTileArea = ( ((XMAX - XMIN) / (float)(NUMNODES - 1)) *
                          ((YMAX - YMIN) / (float)(NUMNODES - 1)) );
   float halfTileArea = fullTileArea / 2;
   float quarterTileArea = fullTileArea / 4;

   omp_set_num_threads(NUMT); 
   float maxHeights = 0;
   float finalSum = 0;

   for(int j = 0; j < NUMTRIES; j++) {
      float sum = 0;
  
      double time0 = omp_get_wtime( );
   
      #pragma omp parallel for default(none) \
      shared(fullTileArea, halfTileArea, quarterTileArea) reduction(+:sum)
      for(int i = 0; i < NUMNODES*NUMNODES; i++) {
         int iu = i % NUMNODES;
         int iv = i / NUMNODES;
         float z = height(iu, iv) * 2;
         float xy;
   
         if( (iu == 0 || iu == NUMNODES - 1) && (iv == 0 || iv == NUMNODES - 1) ) {
            xy = quarterTileArea;
         }
         else if(iu == 0 || iu == NUMNODES - 1 || iv == 0 || iv == NUMNODES - 1) {
            xy = halfTileArea;
         }
         else {
            xy = fullTileArea;
         }
   
         sum += (xy * z);
      }
   
      double time1 = omp_get_wtime( );
      double megaHeightsPerSec = (double)NUMNODES*NUMNODES / ( time1 - time0 ) / 1000000.;
      if(megaHeightsPerSec > maxHeights) {
         maxHeights = megaHeightsPerSec;
         finalSum = sum;
      }
   } 

   //printf("Threads: %d Nodes: %d Area Sum: %0.2lf MegaHeightsPerSec: %0.2lf\n" \
   //       , NUMT, NUMNODES, finalSum, maxHeights);
   printf("%0.2lf, %0.2lf ", maxHeights, finalSum);

   return 0;

}

float height(int iu, int iv) {
   float x = -1. + 2. * (float)iu / (float)(NUMNODES - 1);
   float y = -1. + 2. * (float)iv / (float)(NUMNODES - 1);

   float xn = pow(fabs(x), (double)N);
   float yn = pow(fabs(y), (double)N);
   float r = 1. - xn - yn;

   if(r < 0.) {
      return 0;
   }

   float height = pow(1. - xn - yn, 1. / (float)N);
   return height;
}

float Ranf( float low, float high ) {
   float r = (float) rand(); // 0 - RAND_MAX
   float t = r / (float) RAND_MAX; // 0. - 1.

   return low + t * ( high - low );
}

int Ranf( int ilow, int ihigh ) {
   float low = (float)ilow;
   float high = ceil( (float)ihigh );

   return (int) Ranf(low,high);
}

void TimeOfDaySeed( ) {
   struct tm y2k = { 0 };
   y2k.tm_hour = 0; y2k.tm_min = 0; y2k.tm_sec = 0;
   y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

   time_t timer;
   time( &timer );
   double seconds = difftime( timer, mktime(&y2k) );
   unsigned int seed = (unsigned int)( 1000.*seconds ); // milliseconds
   srand( seed );
}
