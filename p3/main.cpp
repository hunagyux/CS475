/**********************************************************
 * Author: 	Andrew Derringer
 * Program:	P3 - Eco-Simulator
 * Course:	OSU CS475 Spring 2020
 * Summary:	Simulation generates temperature and precipitation
 * 		randomly within range for each month followed by
 * 		changes in grain height, deer, and cougar population
 * 		in turn.
**********************************************************/

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
  
    // Calculate starting variables for simulation.
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

void watcher(int* year, int* month, float* nowHeight, int* nowNumDeer, int* nowNumCougars, float* nowTemp, float* nowPrecip) {
    /**********************************
    * Summary: 	Prints values for each month and calculates
    * 		next months temp and precip in advance of
    * 		other threads calculating respective values.
    * Params:	[1] Year 2020 to 2025
    * 		[2] month 0 to 11
    * 		[3] nowHeight of grain (in) shared by all threads.
    * 		[4] nowNumDeer shared by all threads.
    * 		[5] nowNumCougars shared by all threads.
    * 		[6] nowTemp (f) init for first loop in main.
    * 		[7] nowPrecip (in) init for first loop in main.
    **********************************/

    // Print column headers for CSV formatted text file.
    printf("Year,Month,Grain Ht(in),Deer Pop.,Cougar Pop.,Temp(f),Precip(in)\n");

    while(*year < 2026) {
        // Threads calculate next values.
        #pragma omp barrier

        // Threads replace now values with next values.
        #pragma omp barrier

        // Print results for CSV formatted text file.
        printf("%d,%d,%0.2f,%d,%d,%0.2f,%0.2f\n", *year, *month, *nowHeight, *nowNumDeer, *nowNumCougars, *nowTemp, *nowPrecip);

        *month += 1;
        if(*month > 11) {
            *month = 0;
            *year += 1;
        }

        // Calculate next month temp and percip.
        float ang = (30. * (float)*month + 15.) * (M_PI / 180.);
        float temp = AVG_TEMP - AMP_TEMP * cos(ang);
        *nowTemp = temp + Ranf(&seed, -RANDOM_TEMP, RANDOM_TEMP);
        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH + sin(ang);
        *nowPrecip = precip + Ranf(&seed, -RANDOM_PRECIP, RANDOM_PRECIP);
        if(*nowPrecip < 0) {
            *nowPrecip = 0;
        }
   
        #pragma omp barrier
        // All threads meet up before next loop.
    }
}

void grainDeer(int* year, int* nowNumDeer, int* nowNumCougars, float* nowHeight) {
    /**********************************
    * Summary: 	Calculates next months deer population using this
    * 		months grain height and cougar population then
    * 		changes data at pointer to share with other threads.
    * Params:	[1] Year 2020 to 2025
    * 		[2] nowNumDeer shared by all threads.
    * 		[3] nowNumCougars shared by all threads.
    * 		[4] nowHeight of grain (in) shared by all threads.
    **********************************/

    while(*year < 2026) {
        int nextNumDeer;
 
        // If height of grain exceeds door increase pop else decrease.
        if(*nowHeight > *nowNumDeer) {
            nextNumDeer = *nowNumDeer + 1;
        }
        else if (*nowHeight < *nowNumDeer) {
            nextNumDeer = *nowNumDeer - 1;
        }

        // Decrease deer pop according to cougar pop.
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
    /**********************************
    * Summary: 	Calculates next months grain height using this
    * 		months temp, precip, and deer population then
    * 		changes data at pointer to share with other threads.
    * Params:	[1] Year 2020 to 2025
    * 		[2] nowTemp (f) init for first loop in main.
    * 		[3] nowPrecip (in) init for first loop in main.
    * 		[4] nowHeight of grain (in) shared by all threads.
    * 		[5] nowNumDeer shared by all threads.
    **********************************/

    while(*year < 2026) {
        // Calculate temp and recip factor for next month grain growth.
        float tempFactor = exp( -SQR( (*nowTemp - MIDTEMP) / 10. ) );
        float precipFactor = exp( -SQR( (*nowPrecip - MIDPRECIP) / 10. ) );
        float nextHeight = *nowHeight + (tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH);
        // Decrease grain ht according to deer pop.
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
    /**********************************
    * Summary: 	Calculates next months cougar population using this
    * 		months deer population then changes data at pointer
    * 		to share with other threads.
    * Params:	[1] Year 2020 to 2025
    * 		[2] nowNumCougars shared by all threads.
    * 		[3] nowNumDeer shared by all threads.
    **********************************/

   while(*year < 2026) {
        // Calculate number of cougars according to deer pop.
        int nextNumCougars = *nowNumDeer / 3;

        #pragma omp barrier
        *nowNumCougars = nextNumCougars;

        #pragma omp barrier
        // Watcher prints values, increments time, and calcs new parameters.

        #pragma omp barrier
        // All threads meet up before next loop.
    }
}
