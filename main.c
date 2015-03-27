#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "help.h"
//#include "csapp.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in below.
 *
 * === User information ===
 * User 1: Hjaltil13
 * SSN: 1209912809
 * User 2:
 * SSN:
 * === End User Information ===
 ********************************************************/
/*
typedef struct {
    int *customers; // **customers;
    int n;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
} sbuf_t;


int sbuf_remove(sbuf_t *sp);
void sbuf_deinit(sbuf_t *sp);
void sbuf_init(sbuf_t *sp, int n);
void sbuf_insert(sbuf_t *sp, int item);


sbuf_t chairBuffer; // Available chairs in waiting room
*/

struct chairs
{
  struct customer **customers; /* Array of customers */
  int max;
  int front;
  int rear;
  sem_t slots;
  sem_t items;
  sem_t mutex;
  sem_t barber;
  //sem_t chair;
  /* TODO: Add more variables related to threads */
  /* Hint: Think of the consumer producer thread problem */
};

struct barber
{
  int room;
  struct simulator *simulator;
};

struct simulator
{
  struct chairs chairs;

  pthread_t *barberThread;
  struct barber **barber;
};

static void *barber_work(void *arg)
{
  struct barber *barber = arg;
  struct chairs *chairs = &barber->simulator->chairs;
  struct customer *customer = 0; /* TODO: Fetch a customer from a chair */

  /* Main barber loop */
  while (true) {

    /* TODO: Here you must add you semaphores and locking logic */
    sem_wait(&chairs->barber);
    customer =  chairs->customers[(++chairs->front)%(chairs->max)];  /* Remove the item */ 
//chairs->customer[sbuf_remove(&chairBuffer)]; // chairs->customer[0]; /* TODO: You must choose the customer */
    thrlab_prepare_customer(customer, barber->room);
    sem_post(&chairs->slots);
    thrlab_sleep(5 * (customer->hair_length - customer->hair_goal));
    thrlab_dismiss_customer(customer, barber->room);
    sem_post(&customer->mutex);
        
  }
  return NULL;
}

/**
 * Initialize data structures and create waiting barber threads.
 */
static void setup(struct simulator *simulator)
{
  //Veit ekki alveg með þetta..
  //sbuf_init(&chairBuffer, thrlab_get_num_chairs());

  struct chairs *chairs = &simulator->chairs;

  /* Setup semaphores*/
  chairs->max = thrlab_get_num_chairs();
//  int num_barbers = thrlab_get_num_barbers();

  sem_init(&chairs->mutex, 0, 1);
  sem_init(&chairs->barber, 0, 0);
  //sem_init(&chairs->chair, 0, chairs->max);
  sem_init(&chairs->slots, 0, chairs->max);
  sem_init(&chairs->items, 0, 0);

  /* Create chairs*/
  chairs->customers = malloc(sizeof(struct customer *) * thrlab_get_num_chairs());

  /* Create barber thread data */
  simulator->barberThread = malloc(sizeof(pthread_t) * thrlab_get_num_barbers());
  simulator->barber = malloc(sizeof(struct barber*) * thrlab_get_num_barbers());

  /* Start barber threads */
  struct barber *barber;

  for (unsigned int i = 0; i < thrlab_get_num_barbers(); i++) 
  {
    barber = calloc(sizeof(struct barber), 1);
    barber->room = i;
    barber->simulator = simulator;
    simulator->barber[i] = barber;
    pthread_create(&simulator->barberThread[i], 0, barber_work, barber);
    pthread_detach(simulator->barberThread[i]);
  }
}

/**
 * Free all used resources and end the barber threads.
 */
static void cleanup(struct simulator *simulator)
{
  /* Free chairs */
  free(simulator->chairs.customers);

  /* Free barber thread data */
  free(simulator->barber);
  free(simulator->barberThread);
}

/**
 * Called in a new thread each time a customer has arrived.
 */
static void customer_arrived(struct customer *customer, void *arg)
{
  struct simulator *simulator = arg;
  struct chairs *chairs = &simulator->chairs;

  sem_init(&customer->mutex, 0, 0);

  /* TODO: Accept if there is an available chair */
  // Check if any chairs are available. If not, reject customer
  if (sem_trywait(&chairs->slots) == -1)
  {
	thrlab_reject_customer(customer);
  }
  else 
  {
    sem_wait(&chairs->slots);  
    sem_wait(&chairs->mutex);

    thrlab_accept_customer(customer);
    //sbuf_insert(&chairs->customers, customer);
    //chairs->customer[0] = customer;
	chairs->customers[(++chairs->rear)%(chairs->max)] = customer;   /* Insert the item */
    
	sem_post(&chairs->mutex);
    sem_post(&chairs->barber);
	sem_post(&chairs->slots);
    sem_wait(&customer->mutex);

  }
}
int main (int argc, char **argv)
{
  struct simulator simulator;

  thrlab_setup(&argc, &argv);
  setup(&simulator);

  thrlab_wait_for_customers(customer_arrived, &simulator);

  thrlab_cleanup();
  cleanup(&simulator);

  return EXIT_SUCCESS;
}


//===============SBUF===============
//void sbuf_init(sbuf_t *sp, int n)
//{
//   sp->customers = Calloc(n, sizeof(int)); 
//    sp->n = n;                       /* Buffer holds max of n items */
//    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear */
//    sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking */
//    sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots */
//    sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
//}
/* $end sbuf_init */

/* Clean up buffer sp */
/* $begin sbuf_deinit */
//void sbuf_deinit(sbuf_t *sp)
//{
//    free(sp->customers);
//}
/* $end sbuf_deinit */

/* Insert item onto the rear of shared buffer sp */
/* $begin sbuf_insert */
//void sbuf_insert(sbuf_t *sp, int item)
//{
//    sem_wait(&sp->slots);                          /* Wait for available slot */
//    sem_wait(&sp->mutex);                          /* Lock the buffer */
//    sp->customers[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
//    sem_post(&sp->mutex);                          /* Unlock the buffer */
//    sem_post(&sp->items);                          /* Announce available item */
//}
/* $end sbuf_insert */

/* Remove and return the first item from buffer sp */
/* $begin sbuf_remove */
//int sbuf_remove(sbuf_t *sp)
//{
//    int item;
//    sem_wait(&sp->items);                          /* Wait for available item */
//    sem_wait(&sp->mutex);                          /* Lock the buffer */
//    item = sp->customers[(++sp->front)%(sp->n)];  /* Remove the item */
//    sem_post(&sp->mutex);                          /* Unlock the buffer */
//    sem_post(&sp->slots);                          /* Announce available slot */
//    return item;
//}
/* $end sbuf_remove */
/* $end sbufc */
