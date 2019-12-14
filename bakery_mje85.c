#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

struct arguments{
  int id;

  pthread_mutex_t leftUpdate;
  pthread_mutex_t rightUpdate;

  int leftNum;
  int rightNum;

  pthread_mutex_t checkForMitt;
  pthread_mutex_t checkWhileLock;

  pthread_cond_t leftFree;
  pthread_cond_t rightFree;

} arguments;

void random_sleep(int a, int b);
void *doLeftHandWork(void* arg);
void *doRightHandWork(void* arg);
void *doCautiousWork(void* arg);
int checkWhile(struct arguments *args, int choice);
//int checkWhile(pthread_mutex_t lock, struct arguments);

#define NUM_ITERATIONS 10
#define NUM_LEFT_OVEN_MITTS 3
#define NUM_RIGHT_OVEN_MITTS 3



/* Helper function to sleep a random number of microseconds
 * picked between two bounds (provided in seconds)
 */
void random_sleep(int lbound_sec, int ubound_sec) {
   int num_usec;
   num_usec = lbound_sec*100000 +
	      (int)((ubound_sec - lbound_sec)*100000 * ((double)(rand()) / RAND_MAX));
   usleep(num_usec);
   return;
}

/* Function doLeftHandWork(void* arg)
 *
 * Desc: This function is incharge of the "Left hand only baker"
 * threads and how they run.
 */
void *doLeftHandWork(void* arg){
  struct arguments *argument;
  int id;
  pthread_mutex_t checkForMitt;
  pthread_mutex_t leftUpdate;

  argument = (struct arguments*)arg;

  id = argument->id;
  checkForMitt = argument->checkForMitt;
  leftUpdate = argument->leftUpdate;

  for(int i = 0; i < 10; i++){
    //work  -> print prep, then sleep(2,5)
    printf("[Left-handed baker %d] is working. . .\n", id);
    random_sleep(2, 5);


    //get oven mitt -> acquire lock and print
    pthread_mutex_lock(&checkForMitt);
    printf("[Left-handed baker %d] wants a left-handed mitt. . .\n", id);
    while(checkWhile(argument, 0)== 0){
      pthread_cond_wait(&argument->leftFree, &checkForMitt);
    }



    /*bakers receive their mitts through the function "checkWhile" called
    in the while loop*/
    printf("[Left-handed baker %d] has got a left-handed mitt. . .\n", id);
    pthread_mutex_unlock(&checkForMitt);


    //put cookies in oven -> print, and sleep(2,5)
    printf("[Left-handed baker %d] has put cookies in the oven and is waiting. . .\n", id);
    random_sleep(2, 5);

    //remove cookies -> print statement
    printf("[Left-handed baker %d] has taken cookies out of the oven. . .\n", id);

    //put oven mitts back -> unlock any lock and print
    pthread_mutex_lock(&leftUpdate);
    printf("[Left-handed baker %d] has put back a left-handed mitt. . .\n", id);
    argument->leftNum++;
    pthread_mutex_unlock(&leftUpdate);

    pthread_cond_signal(&argument->leftFree);

  }
}

/* Function: doRightHandWork(void *arg)
 *
 * Desc: This function mimics the workings of the function "doLeftHandWork"
 * This function is incharge of the "Right handed only baker" threads.
 */
void* doRightHandWork(void *arg){
  struct arguments *argument;
  int id;
  pthread_mutex_t checkForMitt;
  pthread_mutex_t rightUpdate;

  argument = (struct arguments*)arg;

  id = argument->id;
  checkForMitt = argument->checkForMitt;
  rightUpdate = argument->rightUpdate;

  for(int i = 0; i < 10; i++){
    //work  -> print prep, then sleep(2,5)
    printf("[Right-handed baker %d] is working. . .\n", id);
    random_sleep(2, 5);

    //get oven mitt -> acquire lock and print
    pthread_mutex_lock(&checkForMitt);
    printf("[Right-handed baker %d] wants a right-handed mitt. . .\n", id);
    while(checkWhile(argument, 1)== 0){
      pthread_cond_wait(&argument->rightFree, &checkForMitt);
    }



    /*bakers receive their mitts through the function "checkWhile" called
    in the while loop*/
    printf("[Right-handed baker %d] has got a right-handed mitt. . .\n", id);
    pthread_mutex_unlock(&checkForMitt);


    //put cookies in oven -> print
    printf("[Right-handed baker %d] has put cookies in the oven and is waiting. . .\n", id);

    //wait for cookies to bake -> sleep(2,5)
    random_sleep(2, 5);
    //remove cookies -> print statement
    printf("[Right-handed baker %d] has taken cookies out of the oven. . .\n", id);

    //put oven mitts back -> unlock any lock and print
    pthread_mutex_lock(&rightUpdate);
    printf("[Right-handed baker %d] has put back a right-handed mitt. . .\n", id);
    argument->rightNum++;
    pthread_mutex_unlock(&rightUpdate);
    pthread_cond_signal(&argument->rightFree);

  }
}



/* Function doCautiousWork(void* arg);
 *
 * Desc: This function is the functionality for the
 * "cautious baker" threads. The function first acquires the left mitt, unlocks
 * then relocks to find the right mitt. Once both mitts are found. It bakes,
 * and releases in the order it received the mitts.
 */
void *doCautiousWork(void* arg){
  struct arguments *argument;
  int id;
  pthread_mutex_t checkForMitt;
  pthread_mutex_t leftUpdate;
  pthread_mutex_t rightUpdate;


  argument = (struct arguments*)arg;

  id = argument->id;
  checkForMitt = argument->checkForMitt;
  leftUpdate = argument->leftUpdate;
  rightUpdate = argument->rightUpdate;

  for(int i = 0; i < 10; i++){
    //work  -> print prep, then sleep(2,5)
    printf("[Cautious baker %d] is working. . .\n", id);

    random_sleep(2, 5);


    //get oven mitt -> acquire lock and print
    pthread_mutex_lock(&checkForMitt);
    //look for left hand mitt first
    printf("[Cautious baker %d] wants a left-handed mitt. . .\n", id);
    while(checkWhile(argument, 0)== 0){
      pthread_cond_wait(&argument->leftFree, &checkForMitt);
    }

    /*bakers receive their mitts through the function "checkWhile" called
    in the while loop*/
    printf("[Cautious baker %d] has got a left-handed mitt. . .\n", id);
    pthread_mutex_unlock(&checkForMitt);
    //Unlocks mitt to not hog maybe...?


    //look for right mitt
    pthread_mutex_lock(&checkForMitt);
    printf("[Cautious baker %d] wants a right-handed mitt. . .\n", id);

    while(checkWhile(argument, 1)== 0){
      pthread_cond_wait(&argument->rightFree, &checkForMitt);
    }

    /*bakers receive their mitts through the function "checkWhile" called
    in the while loop*/
    printf("[Cautious baker %d] has got a right-handed mitt. . .\n", id);
    pthread_mutex_unlock(&checkForMitt);



    //put cookies in oven -> print
    printf("[Cautious baker %d] has put cookies in the oven and is waiting. . .\n", id);

    //wait for cookies to bake -> sleep(2,5)
    random_sleep(2, 5);
    //remove cookies -> print statement
    printf("[Cautious baker %d] has taken cookies out of the oven. . .\n", id);

    //put oven mitts back -> unlock any lock and print
    pthread_mutex_lock(&leftUpdate);
    printf("[Cautious baker %d] has put back a left-handed mitt. . .\n", id);
    argument->leftNum++;
    pthread_mutex_unlock(&leftUpdate);
    pthread_cond_signal(&argument->leftFree);


    pthread_mutex_lock(&rightUpdate);
    printf("[Cautious baker %d] has put back a right-handed mitt. . .\n", id);
    argument->rightNum++;
    pthread_mutex_unlock(&rightUpdate);
    pthread_cond_signal(&argument->rightFree);
  }
}

/* Funciton checkWhile(struct arguments *args, int choice);
 *
 * Desc: This funciton takes in a struct and an integer
 * "choice" which is either 0 or 1. (0 = left, 1 = right).
 * The funciton originally was used to safely check the while
 * loops which host the cond_wait funcitons.
 * After finding a race condition existed after checking this value,
 * and then updating args->left/rightNum (number of mitts) in the
 * calling function, I've decided to update this function so that
 * it will also be in charge of updating the mitt count as well.
 */

int checkWhile(struct arguments *args, int choice){
  pthread_mutex_t whileLock = args->checkWhileLock;
  int retVal = 0;
  pthread_mutex_lock(&whileLock);

  if(choice == 0){
      retVal = args->leftNum;
      if(retVal > 0){
        args->leftNum--;
      }
    }
  else if(choice == 1){
      retVal = args->rightNum;
      if(retVal > 0){
        args->rightNum--;
      }
    }
  pthread_mutex_unlock(&whileLock);

  return retVal;
}

/*
 * Main function
 */
int main(int argc, char **argv) {

  int num_left_handed_bakers;
  int num_right_handed_bakers;
  int num_cautious_bakers;
  int seed;


  struct arguments *args;


  pthread_mutex_t checkForMitt;
  pthread_mutex_t checkWhileLock;
  pthread_mutex_t leftUpdate;
  pthread_mutex_t rightUpdate;

  pthread_cond_t leftFree;
  pthread_cond_t rightFree;

  pthread_cond_init(&leftFree, NULL);
  pthread_cond_init(&rightFree, NULL);

  pthread_mutex_init(&checkForMitt, NULL);
  pthread_mutex_init(&leftUpdate, NULL);
  pthread_mutex_init(&rightUpdate, NULL);
  pthread_mutex_init(&checkWhileLock, NULL);



  //Builds arguments
  args = (struct arguments *)calloc(1, sizeof(struct arguments));


  args->checkForMitt = checkForMitt;
  args->checkWhileLock = checkWhileLock;

  args->leftUpdate = leftUpdate;
  args->rightUpdate = rightUpdate;

  args->leftFree = leftFree;
  args->rightFree = rightFree;

  args->leftNum = 3;
  args->rightNum = 3;
  /* Process command-line arguments */
  if (argc != 5) {
    fprintf(stderr,"Usage: %s <# left-handed bakers> <# right-handed bakers> <# cautious bakers> <seed>\n",argv[0]);
    exit(1);
  }

  if ((sscanf(argv[1],"%d",&num_left_handed_bakers) != 1) ||
      (sscanf(argv[2],"%d",&num_right_handed_bakers) != 1) ||
      (sscanf(argv[3],"%d",&num_cautious_bakers) != 1) ||
      (sscanf(argv[4],"%d",&seed) != 1) ||
      (num_left_handed_bakers < 1) ||
      (num_right_handed_bakers < 1) ||
      (num_cautious_bakers < 1) ||
      (seed < 0)) {
    fprintf(stderr,"Invalid command-line arguments... Aborting\n");
    exit(1);
  }

  /* Seed the RNG */
  srand(seed);


  pthread_t leftHandWorkers[num_left_handed_bakers];
  pthread_t rightHandWorkers[num_right_handed_bakers];
  pthread_t cautiousWorkers[num_cautious_bakers];

  int idNums[num_left_handed_bakers+num_right_handed_bakers+num_cautious_bakers];

  for(int i = 0; i < num_left_handed_bakers; i++){
    args->id = i;
    if(pthread_create(&leftHandWorkers[i], NULL, doLeftHandWork, (void*)args)){
      fprintf(stderr, "Thread Creation Error\n");
    }
    usleep(5000);

  }
  for(int i = 0; i < num_right_handed_bakers; i++){
    args->id = i;
    if(pthread_create(&rightHandWorkers[i], NULL, doRightHandWork, (void*)args)){
      fprintf(stderr, "Thread Creation Error\n");
    }
    usleep(5000);

  }
  for(int i = 0; i < num_cautious_bakers; i++){
    args->id = i;
    if(pthread_create(&cautiousWorkers[i], NULL, doCautiousWork, (void*)args)){
      fprintf(stderr, "Thread Creation Error\n");
    }
    usleep(5000);

  }

  for(int i = 0; i < num_cautious_bakers; i++){
     pthread_join(cautiousWorkers[i], NULL);
   }
  for(int i = 0; i < num_left_handed_bakers; i++){
    pthread_join(leftHandWorkers[i], NULL);

  }
  for(int i = 0; i < num_right_handed_bakers; i++){
    pthread_join(rightHandWorkers[i], NULL);
  }





  exit(0);
}
