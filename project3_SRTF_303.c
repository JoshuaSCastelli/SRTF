/* ******************************************************************** *
* project3_SRTF.c:                                                     *
*                                                                      *
* This is a solution for the 3rd programming project (user-level SRTF  *
* scheduler for p-thread) for CS314 in Spring 2017.                    *
*                                                                      *
* Name: Joshua Castelli                                                *
*   ID: 303							       *
*                                                                      *
* Compile: gcc project3_SRTF_303.c -lpthread                           *
*                                                                      *
* Host: os.cs.siue.edu                                                 *
*                                                                      *
* Date: April 26 , 2017                                                *
*                                                                      *
* Place: Edwardsville, IL                                              *
*                                                                      *
* ********************************************************************  */
#include <string.h>                    /* for strerror()                */
#include <stdio.h>                     /* for printf                    */
#include <pthread.h>                   /* P-thread implementation       */
#include <signal.h>                    /* for interrupt handling        */

#define MAX_THREADS       10           /* Number of child threads       */
#define SCHEDULE_INTERVAL  1           /* interval for clock interrupt  */
/* in second(s)                  */

#define TRUE               1           /* TRUE label                    */
#define FALSE              0           /* FALSE label                   */

/* Prototype ---------------------------------------------------------- */
void * child_thread(void * arg);           /* the child thread body     */
int  find_next_thread(void);           /* find the next thread to run   */
void test_thread_list(void);           /* for test use only             */
void initialize_thread_list(void);     /* initialize the thread schedule*/

									   // YOU (CAN) ADD YOUR OWN PROTOTYPES HERE ////////////////////////////////







									   /* Global variables --------------------------------------------------- */
pthread_mutex_t mutex;                 /* mutex semaphore               */
pthread_cond_t  cond[MAX_THREADS];     /* condition variables           */

int time_to_check;                     /* "time to check" flag          */
int loop_counter;                      /* loop counter                  */

pthread_t parent_thread;           /* Thread ID of the parent thread    */
pthread_t pt[MAX_THREADS];         /* thread descriptor (for system use)*/

int thread_list[MAX_THREADS][4];       /* the thread list               */

unsigned int current_time;             /* current time                  */
unsigned int exec_time;                /* next execution time           */

									   // YOU (CAN) ADD YOUR OWN GLPBAL VARIABLES HERE //////////////////////////
int ready[MAX_THREADS];
int ready_ctr=0;
int exit = FALSE;
int time_to_stop[MAX_THREADS];
int next_child_to_run;// The thread # for the next child thread
int threads_complete = 0;
									   /* The interrupt handler for SIGALM interrupt ------------------------- */
void clock_interrupt_handler(void)
{
	time_to_stop[next_child_to_run]=TRUE;
	current_time++;
							   // to be scheduled ...                 

							   // YOU (CAN) ADD YOUR OWN LOCAL VARIABLES HERE ////////////////////////






							   // PLEASE DO NOT REMOVE THE FOLLOWING THREE LINES /////////////////////
	printf("scheduler started ++++++++++++++++++++++++++++++++++ \n");
	printf("current time: %d\n", current_time);
	//sleep(2);   // for "slow motion version"

	// Find the next child to run ----------------------------------
	if(threads_complete < MAX_THREADS){
		next_child_to_run = find_next_thread();
	// THE FOLLOWING PLACE IS YOUR PLAYGROUND ///////////////////////
		pthread_mutex_lock(&mutex);
		time_to_stop[next_child_to_run]=FALSE;
		pthread_cond_signal(&cond[next_child_to_run]);
		pthread_mutex_unlock(&mutex);
		alarm(SCHEDULE_INTERVAL);
	}
}

/* the MAIN ----------------------------------------------------------- */
void main(void)
{
	int  i;                 // general-purpose loop counter

	time_to_check = TRUE;   // initialize the "time_to_check" flag
	loop_counter = 0;      // initialize the loop counter
	int ack_to_sch = FALSE;  // the ACK to the scheduler flag
	int continue_more = TRUE;   // set "continue flag"

							// YOU (CAN) ADD YOUR OWN LOCAL VARIABLES HERE ////////////////////////

							// initialzie the thread list ------------------------------------
	initialize_thread_list();
	test_thread_list();

	// initialzie the scheduler time ---------------------------------
	current_time = 0;
	exec_time = 0;

	// create ten threads --------------------------------------------
	pthread_mutex_init(&mutex,NULL);

	for(i = 0;i < MAX_THREADS;i++){
		time_to_stop[i]=TRUE;
		pthread_cond_init(&cond[i],NULL);
		pthread_create(&pt[i],0,child_thread,(void*)i);
		sleep(1);
		loop_counter++;
	} 


	// wait for 1 second ---------------------------------------------
	sleep(1);
	signal(SIGALRM,clock_interrupt_handler);
	alarm(SCHEDULE_INTERVAL); // signal is generated
	// Inifinite loop for the parent thread --------------------------
	while (continue_more == TRUE)
	{
		// Put the parent scheduler into sleep until the next SIGALRM
		int finished[MAX_THREADS];
		int finished_count;
		for(int i = 0; i < MAX_THREADS; i++){
			finished[i]=1;
		}

		for(int i = 0; i < MAX_THREADS; i++){
			finished[i] = pthread_join(pt[i],NULL);
			if(finished[i]==0){
				finished_count++;
			}
		}
		
		if(finished_count==MAX_THREADS){
			goto all_done;
		}

	}
	all_done:;
	// terminating myself --------------------------------------------
	printf("the parent thread is terminating ....\n");
	pthread_mutex_destroy(&mutex);
	for(int i = 0; i < MAX_THREADS; i++){
		pthread_cond_destroy(&cond[i]);
	}
	pthread_exit(NULL);
}

/* child-thread definition --------------------------------------------- */
void * child_thread(void*arg)
{
	int index = (int)arg;
	int local_counter = 0;

	printf("Thread %d has started ...\n", index);

	while(loop_counter < 10);	//do nothing until all 10 threads are created.

	// the main infinite loop for a child thread -------------------

	while (loop_counter == 10)
	{
		local_counter++;

		// THE FOLLOWING PLACE IS YOUR PLAYGROUND ///////////////////////
		pthread_mutex_lock(&mutex);

		if(time_to_stop[index]==TRUE){
			if(thread_list[index][3]<=0){
				pthread_mutex_unlock(&mutex);
				goto finish;
			}
			pthread_cond_wait(&cond[index],&mutex);
		}
		// show my presence -----------------------------------------
		if ((local_counter % 10000000) == 0)
		{
			printf("     Thread %d is running ...\n", index);
		}
		pthread_mutex_unlock(&mutex);
	}
	finish:;
	printf("child %d exiting\n",index);
}

/* find the next thread to run ----------------------------------------- */
int find_next_thread(void)
{
	// THE FOLLOWING PLACE IS YOUR PLAYGROUND ///////////////////////
	//SRTF SCHEDULER

	//CHECK START TIMES. IF EQUAL TO CURRENT TIME, ADD THREAD ID(S) TO LIST OF READY THREADS
	for(int i  = 0; i < MAX_THREADS; i++){
		if(thread_list[i][1] == current_time){
			ready[ready_ctr] = thread_list[i][0];
			ready_ctr++;
		}
	}
	/*
	* OF THE LIST OF READY THREADS, CHECK WHICH HAS THE LOWEST REMAINING EXECUTION TIME.
	* AS LONG AS THE REMAINING TIME DOES NOT EQUAL ZERO AND IS LESS THAN THE REST OF THE 
	* READY THREADS, RETURN THIS THREAD ID TO BE EXECUTED. (ID TIE BREAKER)
	*/
	int lowest_remain_time_ID=0;
	if(ready_ctr != 0){
		for(int i = 0; i < ready_ctr; i++){
			if(thread_list[ready[i]][3] != 0){
				lowest_remain_time_ID = thread_list[i][0];
				goto done;
			}
		}done:;

		for(int i = 0; i < ready_ctr; i++){
			if(thread_list[ready[i]][3] <= thread_list[lowest_remain_time_ID][3] && thread_list[ready[i]][3] != 0){
				if(thread_list[ready[i]][3] < thread_list[lowest_remain_time_ID][3]){
					lowest_remain_time_ID = thread_list[ready[i]][0];
				}
				else if(thread_list[ready[i]][3] == thread_list[lowest_remain_time_ID][3]){ //ELSE THEY'RE EQUAL, ID TIE BREAKER
					if(thread_list[ready[i]][0] < thread_list[lowest_remain_time_ID][0]){
						lowest_remain_time_ID = thread_list[ready[i]][0];
					}
				}
			}
		}
	}
	int next_thread = lowest_remain_time_ID;
	//Decrement remaining time.
	thread_list[lowest_remain_time_ID][3]--;
	threads_complete = 0;
	for(int i = 0; i < MAX_THREADS; i++){
		if(thread_list[i][3] <= 0){
			threads_complete++;
		}
	}
	// return the next thread ID -----------------------
	return (next_thread);
}

/* initialize the thread list ------------------------------------------ */
void initialize_thread_list(void)
{
	// Thread #0 ----------------------------------------
	thread_list[0][0] = 0;     // thread ID
	thread_list[0][1] = 1;     // start time
	thread_list[0][2] = 15;    // total execution time
	thread_list[0][3] = 15;    // remaining time

							   // Thread #1 ----------------------------------------
	thread_list[1][0] = 1;     // thread ID
	thread_list[1][1] = 5;     // start time
	thread_list[1][2] = 9;     // total execution time
	thread_list[1][3] = 9;     // remaining time

							   // Thread #2 ----------------------------------------
	thread_list[2][0] = 2;     // thread ID
	thread_list[2][1] = 3;     // start time
	thread_list[2][2] = 10;    // total execution time
	thread_list[2][3] = 10;    // remaining time

							   // Thread #3 ----------------------------------------
	thread_list[3][0] = 3;     // thread ID
	thread_list[3][1] = 5;     // start time
	thread_list[3][2] = 12;    // total execution time
	thread_list[3][3] = 12;    // remaining time

							   // Thread #4 ----------------------------------------
	thread_list[4][0] = 4;     // thread ID
	thread_list[4][1] = 8;     // start time
	thread_list[4][2] = 3;     // total execution time
	thread_list[4][3] = 3;     // remaining time

							   // Thread #5 ----------------------------------------
	thread_list[5][0] = 5;     // thread ID
	thread_list[5][1] = 20;    // start time
	thread_list[5][2] = 6;     // total execution time
	thread_list[5][3] = 6;     // remaining time

							   // Thread #6 ----------------------------------------
	thread_list[6][0] = 6;     // thread ID
	thread_list[6][1] = 17;    // start time
	thread_list[6][2] = 9;     // total execution time
	thread_list[6][3] = 9;     // remaining time

							   // Thread #7 ----------------------------------------
	thread_list[7][0] = 7;     // thread ID
	thread_list[7][1] = 11;    // start time
	thread_list[7][2] = 5;     // total execution time
	thread_list[7][3] = 5;     // remaining time

							   // Thread #8 ----------------------------------------
	thread_list[8][0] = 8;     // thread ID
	thread_list[8][1] = 4;     // start time
	thread_list[8][2] = 10;    // total execution time
	thread_list[8][3] = 10;    // remaining time

							   // Thread #9 ----------------------------------------
	thread_list[9][0] = 9;     // thread ID
	thread_list[9][1] = 6;     // start time
	thread_list[9][2] = 9;     // total execution time
	thread_list[9][3] = 9;     // remaining time
}


/* test thread list ------------------------------------------------- */
void test_thread_list(void)
{
	int  i;   // loop counter

	printf("thread ID, start time, execution time, remaining time\n");
	for (i = 0; i < MAX_THREADS; i++)
	{
		printf("%d,      %d,      %d,      %d\n", thread_list[i][0],
			thread_list[i][1], thread_list[i][2], thread_list[i][3]);
	}
	printf("\n\n");
}

