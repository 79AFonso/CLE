/*
  Codigo do fifo foi fornecido pelo grupo

*/

#include <stdint.h>
#include <pthread.h>

#define END 9997
#define FIFO_SIZE 32

typedef struct matrix_t
{
  int f_id;
  int m_id;
  double *values;
  int order;
} matrix_t;

typedef struct fifo_t
{
  matrix_t *array[FIFO_SIZE];
  unsigned int inp;
  unsigned int out;
  unsigned int cnt;
  pthread_mutex_t mutex;
  pthread_cond_t isNotEmpty;
  pthread_cond_t isNotFull;
} fifo_t;

void init_fifo(fifo_t *fifo);
int empty_fifo(fifo_t *fifo);
int full_fifo(fifo_t *fifo);
void insert_fifo(fifo_t *fifo, matrix_t *mat);
matrix_t *retrieve_fifo(fifo_t *fifo);
