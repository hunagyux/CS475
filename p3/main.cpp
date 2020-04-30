#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>


unsigned int seed = 0;

// Simulation Parameters
const float GRAIN_GROWS_PER_MONTH =   9.0; // Inches
const float ONE_DEER_EATS_PER_MONTH = 1.0;

const float AVG_PRECIP_PER_MONTH =    7.0;
const float AMP_PRECIP_PER_MONTH =    6.0;
const float RANDOM_PRECIP =           2.0;

const float AVG_TEMP =                60.0; // Fahrenheit
const float AMP_TEMP =                20.0;
const float RANDOM_TEMP =             10.0;

const float MIDTEMP =                 40.0;
const float MIDPRECIP =               10.0;


float Ranf(unsigned int*, float, float);
int Ranf(unsigned int*, int, int);
float SQR(float);

void watcher(int*, int*, unsigned int*, float*, int*, int*, float*, float*);
void grainDeer(int*, int*, int*, float*);
void grain(int*, float*, float*, float*, int*);
void cougars(int*, int*, int*);
 

int main() {
    // Starting Values
    int month =       0;
    int year =        2020;

    int nowNumDeer =    1;
    int nowNumCougars = 0;
    float nowHeight =   1.;
  
    float ang = (30. * (float)month + 15.) * (M_PI / 180.);

    float temp = AVG_TEMP - AMP_TEMP * cos(ang);
    float nowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH + sin(ang);
    float nowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
    if(nowPrecip < 0) {
        nowPrecip = 0;
    }

    omp_set_num_threads(4);
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            grain(&year, &nowTemp, &nowPrecip, &nowHeight, &nowNumDeer);
        }
        #pragma omp section
        {
            grainDeer(&year, &nowNumDeer, &nowNumCougars, &nowHeight);
        }
        #pragma omp section
        {
            cougars(&year, &nowNumCougars, &nowNumDeer);
        }
        #pragma omp section
        {
            watcher(&year, &month, &seed, &nowHeight, &nowNumDeer, &nowNumCougars, &nowTemp, &nowPrecip);
        }
    } 

    return 0;
}

float Ranf(unsigned int *seedp, float low, float high) {
    float r = (float) rand_r(seedp);
    return (low + r * (high - low) / (float)RAND_MAX);
}

int Ranf(unsigned int *seedp, int ilow, int ihigh) {
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;
    return (int)(Ranf(seedp, low, high));
}

float SQR(float x) {
    return x * x;
}

void watcher(int* year, int* month, unsigned int* seed, float* nowHeight, int* nowNumDeer, int* nowNumCougars, float* nowTemp, float* nowPrecip) {
    //printf("%15s %15s %15s %15s %15s %15s %15s\n", "year", "month", "Grain Ht(in)", "Deer Pop.", "Cougar Pop.", "Temp(f)", "Precip(in)");
    printf("Year,Month,Grain Ht(in),Deer Pop.,Cougar Pop.,Temp(f),Precip(in)\n");

    while(*year < 2026) {
        // Threads calculate next values.
        #pragma omp barrier

        // Threads replace now values with next values.
        #pragma omp barrier

        printf("%d,%d,%0.2f,%d,%d,%0.2f,%0.2f\n", *year, *month, *nowHeight, *nowNumDeer, *nowNumCougars, *nowTemp, *nowPrecip);
;
        //printf("%15d %15d %15.2f %15d %15d %15.2f %15.2f\n", *year, *month, *nowHeight, *nowNumDeer, *nowNumCougars, *nowTemp, *nowPrecip);
        //printf("Year:%d Month:%d\n", *year, *month);
        //printf("Grain Ht(in):%0.2f Deer Pop:%d Cougar Pop:%d\n", *nowHeight, *nowNumDeer, *nowNumCougars);
        //printf("Temp(f):%0.2f Precip(in):%0.2f\n", *nowTemp, *nowPrecip);

        *month += 1;
        if(*month > 11) {
            *month = 0;
            *year += 1;
        }

        float ang = (30. * (float)*month + 15.) * (M_PI / 180.);

        float temp = AVG_TEMP - AMP_TEMP * cos(ang);
        *nowTemp = temp + Ranf(seed, -RANDOM_TEMP, RANDOM_TEMP);

        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH + sin(ang);
        *nowPrecip = precip + Ranf(seed, -RANDOM_PRECIP, RANDOM_PRECIP);
        if(*nowPrecip < 0) {
            *nowPrecip = 0;
        }
   
        #pragma omp barrier
        // All threads meet up before next loop.
    }
}

void grainDeer(int* year, int* nowNumDeer, int* nowNumCougars, float* nowHeight) {
    while(*year < 2026) {
        int nextNumDeer;
        if(*nowHeight > *nowNumDeer) {
            nextNumDeer = *nowNumDeer + 1;
        }
        else if (*nowHeight < *nowNumDeer) {
            nextNumDeer = *nowNumDeer - 1;
        }

        nextNumDeer -= (*nowNumCougars / 2);

        #pragma omp barrier
        *nowNumDeer = nextNumDeer;

        #pragma omp barrier
        // Watcher prints values, increments time, and calcs new parameters.

        #pragma omp barrier
        // All threads meet up before next loop.
    }
}

void grain(int* year, float* nowTemp, float* nowPrecip, float* nowHeight, int* nowNumDeer) {
    while(*year < 2026) {
        float tempFactor = exp( -SQR( (*nowTemp - MIDTEMP) / 10. ) );
        float precipFactor = exp( -SQR( (*nowPrecip - MIDPRECIP) / 10. ) );
        float nextHeight = *nowHeight + (tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH);
        nextHeight -= (float)*nowNumDeer * ONE_DEER_EATS_PER_MONTH * 0.5;
        if(nextHeight < 0) {
            nextHeight = 0;
        }
        #pragma omp barrier
        *nowHeight = nextHeight;
 
        #pragma omp barrier
        // Watcher prints values, increments time, and calcs new parameters.

        #pragma omp barrier
        // All threads meet up before next loop.
    }
}

void cougars(int* year, int* nowNumCougars, int* nowNumDeer) {
    while(*year < 2026) {
        int nextNumCougars = *nowNumDeer / 3;

        #pragma omp barrier
        *nowNumCougars = nextNumCougars;

        #pragma omp barrier
        // Watcher prints values, increments time, and calcs new parameters.

        #pragma omp barrier
        // All threads meet up before next loop.
    }
}
