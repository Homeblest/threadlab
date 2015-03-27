#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "help.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in below.
 *
 * === User information ===
 * User 1: Hjaltil13
 * SSN: 1209912809
 * User 2: arnia13
 * SSN: 0804922789
 * === End User Information ===
 ********************************************************/

struct chairs
{
  struct customer **customer; /* Array of customers */
  int max;    // Number of chairs in waiting room
  int front;  // First available chair
  int rear;   // Back of buffer
  sem_t mutex; // Protects access to chairs
  sem_t slots; // Number of available chairs
  sem_t items; // Number of occupied chairs
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


/**
 * Called in a new thread each time a customer has arrived.
 */
static void customer_arrived(struct customer *customer, void *arg)
{
  struct simulator *simulator = arg;
  struct chairs *chairs = &simulator->chairs;

  sem_init(&customer->mutex, 0, 0);

  if (sem_trywait(&chairs->slots) == -1) {
    // No chair available, customer leaves.
    thrlab_reject_customer(customer);
  } else {
    sem_wait(&chairs->mutex); // Lock the chairs

    thrlab_accept_customer(customer);
    // Choose a seat for the customer
    chairs->customer[(++chairs->rear) % (chairs->max)] = customer;

    sem_post(&chairs->mutex); // Unlock the chairs
    sem_post(&chairs->items); // Announce available customer
    sem_wait(&customer->mutex);
  }
}

static void *barber_work(void *arg)
{
  struct barber *barber = arg;
  struct chairs *chairs = &barber->simulator->chairs;
  struct customer *customer;

  /* Main barber loop */
  while (true) {
    sem_wait(&chairs->items); /* Wait for available item */
    sem_wait(&chairs->mutex); /* Lock the buffer */

    customer = chairs->customer[(++chairs->front) % (chairs->max)];  /* Remove the item */

    sem_post(&chairs->mutex); /* Unlock the buffer */
    sem_post(&chairs->slots); /* Announce available slot */

    thrlab_prepare_customer(customer, barber->room);
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
  struct chairs *chairs = &simulator->chairs;

  /* Setup semaphores*/
  chairs->max = thrlab_get_num_chairs();
  chairs->front = 0;
  chairs->rear = 0;

  sem_init(&chairs->mutex, 0, 1);
  sem_init(&chairs->slots, 0, chairs->max);
  sem_init(&chairs->items, 0, 0);

  /* Create chairs*/
  chairs->customer = malloc(sizeof(struct customer *) * thrlab_get_num_chairs());

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
    if (pthread_create(&simulator->barberThread[i], 0, barber_work, barber) != 0) {
      fprintf(stderr, "Could not create barber thread\n");
    };
    if (pthread_detach(simulator->barberThread[i]) != 0) {
      fprintf(stderr, "Could not detach barber thread\n");
    };
  }
}
/**
 * Free all used resources and end the barber threads.
 */
static void cleanup(struct simulator *simulator)
{
  /* Free chairs */
  free(simulator->chairs.customer);
  // Free the semaphores
  sem_destroy(&simulator->chairs.mutex);
  sem_destroy(&simulator->chairs.slots);
  sem_destroy(&simulator->chairs.items);
  /* Free barber thread data */
  free(simulator->barber);
  free(simulator->barberThread);
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
