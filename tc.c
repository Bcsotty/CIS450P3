#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/time.h>
#include <assert.h>

#define CARS 8
#define TLEFT 5
#define TSTRAIGHT 4
#define TRIGHT 3

double GetTime() {
    struct timeval t;
    int rc = gettimeofday(&t, NULL);
    assert(rc == 0);
    return (double)t.tv_sec + (double)t.tv_usec/1e6;
}

void Spin(int howlong) {
    double t = GetTime();
    while ((GetTime() - t) < (double)howlong)
	;
}

// Mutex semaphores
sem_t mutex1, mutex2;
// Semaphores for arriving/entering the intersection
sem_t head_of_line, north, east, south, west;
// Semaphores for crossing the intersection
sem_t collisions[16];
// Semaphore for exiting intersection
sem_t eNorth, eEast, eSouth, eWest;
// Counter variables to help solve readers problem crossing intersection
int north2east, north2south, north2west, east2north, east2west, east2south, south2east, south2north, south2west, west2north, west2east, west2south;
// Double to keep track of start time, used for outputting time to console
double startTime;

typedef struct _directions {
    char dir_original;
    char dir_target;
} directions;

// Car struct, holds car id, arrival time, thread, and directions
typedef struct _car {
    directions d;
    int cid;
    float arrival_time;
    pthread_t thread;
} car;


// Returns the direction the car is travelling relative to their original direction.
// 0 is left, 1 is straight, 2 is right
int getDirection(directions dir) {
    switch(dir.dir_original) {
        case '^':
            if (dir.dir_target == '^')
                return 1;
            else if (dir.dir_target == '>')
                return 2;
            else
                return 0;
        case '>':
            if (dir.dir_target == '^')
                return 0;
            else if (dir.dir_target == '>')
                return 1;
            else
                return 2;
        case 'v':
            if (dir.dir_target == 'v')
                return 1;
            else if (dir.dir_target == '>')
                return 0;
            else
                return 2;
        case '<':
            if (dir.dir_target == '^')
                return 2;
            else if (dir.dir_target == '<')
                return 1;
            else
                return 0;
    }
}

// Arrive intersection function. Accepts car pointer
void ArriveIntersection(car *car){
    // Print statement for console
    printf("Time  %.1f: Car %d (%c %c) arriving\n", GetTime() - startTime, car->cid, car->d.dir_original, car->d.dir_target);

    // Sleep for 2 seconds
    usleep(2000000);

    // If statements ensures that car reaches the front of its own lane, before checking if it is the first at the stop sign
    if(car->d.dir_original == '^'){
        sem_wait(&south);
    }
    else if(car->d.dir_original == '<'){
        sem_wait(&east);
    }
    else if(car->d.dir_original == 'v'){
        sem_wait(&north);
    }
    else{
        sem_wait(&west);
    }
    usleep(100);
    sem_wait(&head_of_line);
}

// Cross intersection function. Accepts car pointer.
void CrossIntersection(car *car){
    // Acquire mutex, then use the directions of the car to lock specific semaphores for crossing.
    sem_wait(&mutex1);
    if(car->d.dir_original == '^'){
        if(car->d.dir_target == '^'){
            south2north += 1;
            if(south2north == 1){
                sem_wait(&eNorth);
                sem_wait(&collisions[0]);
                sem_wait(&collisions[1]);
                sem_wait(&collisions[2]);
                sem_wait(&collisions[3]);
            }
        }
        else if(car->d.dir_target == '>'){
            south2east += 1;
            if(south2east == 1)
                sem_wait(&eEast);
        }
        else if(car->d.dir_target == '<'){
            south2west += 1;
            if(south2west == 1){
                sem_wait(&eWest);
                sem_wait(&collisions[11]);
                sem_wait(&collisions[12]);
                sem_wait(&collisions[15]);
                sem_wait(&collisions[7]);
            }
        }
    }
    else if(car->d.dir_original == '<'){
        if(car->d.dir_target == '^'){
            east2north += 1;
            if(east2north == 1)
                sem_wait(&eNorth);
        }
        else if(car->d.dir_target == 'v'){
            east2south += 1;
            if(east2south == 1){
                sem_wait(&eSouth);
                sem_wait(&collisions[2]);
                sem_wait(&collisions[13]);
                sem_wait(&collisions[12]);
                sem_wait(&collisions[10]);
            }
        }
        else if(car->d.dir_target == '<'){
            east2west += 1;
            if(east2west == 1){
                sem_wait(&eWest);
                sem_wait(&collisions[3]);
                sem_wait(&collisions[4]);
                sem_wait(&collisions[5]);
                sem_wait(&collisions[6]);
            }
        }
    }
    else if(car->d.dir_original == 'v'){
        if(car->d.dir_target == '>'){
            north2east += 1;
            if(north2east == 1){
                sem_wait(&eEast);
                sem_wait(&collisions[5]);
                sem_wait(&collisions[14]);
                sem_wait(&collisions[13]);
                sem_wait(&collisions[1]);
            }
        }
        else if(car->d.dir_target == 'v'){
            north2south += 1;
            if(north2south == 1){
                sem_wait(&eSouth);
                sem_wait(&collisions[6]);
                sem_wait(&collisions[7]);
                sem_wait(&collisions[8]);
                sem_wait(&collisions[9]);
            }
        }
        else if(car->d.dir_target == '<'){
            north2west += 1;
            if(north2west == 1)
                sem_wait(&eWest);
        }
    }
    else{
        if(car->d.dir_target == '>'){
            west2east += 1;
            if(west2east == 1) {
                sem_wait(&eEast);
                sem_wait(&collisions[9]);
                sem_wait(&collisions[10]);
                sem_wait(&collisions[11]);
                sem_wait(&collisions[0]);
            }
        }
        else if(car->d.dir_target == 'v'){
            west2south += 1;
            if(west2south == 1)
                sem_wait(&eSouth);
        }
        else if(car->d.dir_target == '^'){
            west2north += 1;
            if(west2north == 1){
                sem_wait(&eNorth);
                sem_wait(&collisions[8]);
                sem_wait(&collisions[15]);
                sem_wait(&collisions[14]);
                sem_wait(&collisions[4]);
            }
        }
    }
    sem_post(&mutex1);
    // Car is now "crossing", so need to release semaphore for the lane it came from so next cars can move forward
    if(car->d.dir_original == '^'){
        sem_post(&south);
    }
    else if(car->d.dir_original == '>'){
        sem_post(&west);
    }
    else if(car->d.dir_original == 'v'){
        sem_post(&north);
    }
    else {
        sem_post(&west);
    }
    // Release head of line semaphore so next car at stop sign can start trying to cross.
    sem_post(&head_of_line);
    printf("Time  %.1f: Car %d (%c %c)         crossing\n", GetTime() - startTime, car->cid, car->d.dir_original, car->d.dir_target);
    // Call get direction function to figure out how long we need to spin while car is crossing
    int dir = getDirection(car->d);
    if (dir == 0){
        Spin(TLEFT);
    }
    else if (dir == 1){
        Spin(TSTRAIGHT);
    }
    else {
        Spin(TRIGHT);
    }
}

// Exit intersection function. Accepts Car Pointer
void ExitIntersection(car *car){
    // Essentially the exact same thing as the cross intersection function, just posting the semaphores instead of waiting on them.
    sem_wait(&mutex2);
    if(car->d.dir_original == '^'){
        if(car->d.dir_target == '^'){
            south2north -= 1;
            if(south2north == 0){
                sem_post(&eNorth);
                sem_post(&collisions[0]);
                sem_post(&collisions[1]);
                sem_post(&collisions[2]);
                sem_post(&collisions[3]);
            }
        }
        else if(car->d.dir_target == '>'){
            south2east -= 1;
            if(south2east == 0)
                sem_post(&eEast);
        }
        else if(car->d.dir_target == '<'){
            south2west -= 1;
            if(south2west == 0){
                sem_post(&eWest);
                sem_post(&collisions[11]);
                sem_post(&collisions[12]);
                sem_post(&collisions[15]);
                sem_post(&collisions[7]);
            }
        }
    }
    else if(car->d.dir_original == '<'){
        if(car->d.dir_target == '^'){
            east2north -= 1;
            if(east2north == 0)
                sem_post(&eNorth);
        }
        else if(car->d.dir_target == 'v'){
            east2south -= 1;
            if(east2south == 0){
                sem_post(&eSouth);
                sem_post(&collisions[2]);
                sem_post(&collisions[13]);
                sem_post(&collisions[12]);
                sem_post(&collisions[10]);
            }
        }
        else if(car->d.dir_target == '<'){
            east2west -= 1;
            if(east2west == 0){
                sem_post(&eWest);
                sem_post(&collisions[3]);
                sem_post(&collisions[4]);
                sem_post(&collisions[5]);
                sem_post(&collisions[6]);
            }
        }

    }
    else if(car->d.dir_original == 'v'){
        if(car->d.dir_target == '>'){
            north2east -= 1;
            if(north2east == 0){
                sem_post(&eEast);
                sem_post(&collisions[5]);
                sem_post(&collisions[14]);
                sem_post(&collisions[13]);
                sem_post(&collisions[1]);
            }
        }
        else if(car->d.dir_target == 'v'){
            north2south -= 1;
            if(north2south == 0){
                sem_post(&eSouth);
                sem_post(&collisions[6]);
                sem_post(&collisions[7]);
                sem_post(&collisions[8]);
                sem_post(&collisions[9]);
            }
        }
        else if(car->d.dir_target == '<'){
            north2west -= 1;
            if(north2west == 0)
                sem_post(&eWest);
        }
    }
    else{
        if(car->d.dir_target == '>'){
            west2east -= 1;
            if(west2east == 0) {
                sem_post(&eEast);
                sem_post(&collisions[9]);
                sem_post(&collisions[10]);
                sem_post(&collisions[11]);
                sem_post(&collisions[0]);
            }
        }
        else if(car->d.dir_target == 'v'){
            west2south -= 1;
            if(west2south == 0)
                sem_post(&eSouth);
        }
        else if(car->d.dir_target == '^'){
            west2north -= 1;
            if(west2north == 0){
                sem_post(&eNorth);
                sem_post(&collisions[8]);
                sem_post(&collisions[15]);
                sem_post(&collisions[14]);
                sem_post(&collisions[4]);
            }
        }
    }
    sem_post(&mutex2);
    printf("Time  %.1f: Car %d (%c %c)                  exiting\n", GetTime() - startTime, car->cid, car->d.dir_original, car->d.dir_target);
}

void *Car(car *car){
    usleep(car->arrival_time * 1000000);
    ArriveIntersection(car);
    CrossIntersection(car);
    ExitIntersection(car);
    return NULL;
}

int main(void) {
    car cars[CARS];
    directions d;

    // Initializing various variables and semaphores
    sem_init(&head_of_line, 0, 1);
    sem_init(&north, 0, 1);
    sem_init(&south, 0, 1);
    sem_init(&east, 0, 1);
    sem_init(&west, 0, 1);
    sem_init(&eNorth, 0, 1);
    sem_init(&eSouth, 0, 1);
    sem_init(&eEast, 0, 1);
    sem_init(&eWest, 0, 1);
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    for(int i = 0; i < 16; i++)
        sem_init(&collisions[i], 0, 1);

    north2east = north2south = north2west = east2north = east2west = east2south = south2east = south2north = south2west = west2north = west2east = west2south = 0;
    // Setting up the cars directions.
    d.dir_original = '^';
    d.dir_target = '^';
    cars[0].d = d;
    cars[1].d = d;
    cars[5].d = d;
    d.dir_original = '>';
    cars[6].d = d; 
    d.dir_original = '<';
    cars[7].d = d;
    d.dir_target = '<';
    d.dir_original = '^';
    cars[2].d = d;
    d.dir_original = 'v';
    d.dir_target = 'v';
    cars[3].d = d;
    d.dir_target = '>';
    cars[4].d = d;

    // Assigning car ID and car arrival time
    float x = 1.1;
    for(int i = 0; i < CARS; i++){
        cars[i].cid = i + 1;
        cars[i].arrival_time = x;
        x += 1.1;
    }

    // Get the initial time, then create the car threads and wait for them to return.
    startTime = GetTime();
    for(int i = 0; i < CARS; i++)
        pthread_create(&cars[i].thread, NULL, Car, &cars[i]);
    
    for(int i = 0; i < CARS; i++)
        pthread_join(cars[i].thread, NULL);;

    printf("Brett Csotty\n");
    return 0;
}