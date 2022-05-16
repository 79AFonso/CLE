#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "fifo.h"
#include <time.h>

int order = 0;
struct timespec start, finish;

static fifo_t fifo;
static double **shared_mem;
static pthread_t *workers;


void *det_calc(void *argp)
{
    while(1){
        matrix_t *matrix = retrieve_fifo(&fifo);

        if (matrix->m_id == -1)
        {
            free(matrix);
            break;
        }

        double det=1;
        double aux;

        for(int i=0; i<matrix->order; i++){
            for(int j=i+1; j<matrix->order; j++)
            {
                aux = matrix->values[matrix->order * j + i]/matrix->values[matrix->order * i + i];

                for(int k=0; k<matrix->order; k++)
                {
                    matrix->values[matrix->order * j + k] = matrix->values[matrix->order * j + k]-aux*matrix->values[matrix->order * i + k];
                }
            }
        }

        for(int i=0; i<matrix->order; i++){
            det = det*matrix->values[matrix->order * i + i];
        }
        shared_mem[matrix->f_id][matrix->m_id] = det;
        free(matrix->values);
        free(matrix);
    }
}

void create_thread(int num_workers){
    for (int i = 0; i < num_workers; i++){
        pthread_create(&workers[i], NULL, det_calc, NULL);
    }
}

void join_thread(int num_workers){
    for (int i = 0; i < num_workers; i++){
        pthread_join(workers[i], NULL);
    }
}

int main(int argc, char **argv)
{

    clock_gettime(CLOCK_MONOTONIC, &start);
    double elapsed;

    int c = 0;
    int num_files = 0;
    int num_workers = 0;
    char *files[5];
    int matrices_num[num_files];

    

    while ((c = getopt(argc, argv, "w:f:")) != -1) {
        switch (c)
        {
        case 'w':
            num_workers = atoi(optarg);
            break;
        case 'f':
            files[num_files++] = optarg;
            break;
        }
        
    }

    init_fifo(&fifo);
    workers = malloc(sizeof(pthread_t) * num_workers);
    shared_mem = malloc(sizeof(double *) * num_files);

    create_thread(num_workers);
    
    for (int i = 0; i < num_files; i++){

        FILE * f;
        f = fopen(files[i], "rb");

        int n_matrices, order;
        fread(&n_matrices, 4, 1, f);
        fread(&order, 4, 1, f);

        shared_mem[i] = malloc(8 * n_matrices);
        matrices_num[i] = n_matrices;

        for (int j = 0; j < n_matrices; j++) {

            matrix_t *matrix = malloc(sizeof(matrix_t));

            matrix->m_id = j;
            matrix->f_id = i;
            matrix->order = order;

            int aux = order * order;
            matrix->values = malloc(8 * aux);
            fread(matrix->values, 8, aux, f);

            insert_fifo(&fifo, matrix);
        }

        fclose(f);
    }

    for (int i = 0; i < num_workers; i++){
        matrix_t *end_var = malloc(sizeof(matrix_t));
        end_var->m_id = -1;
        insert_fifo(&fifo, end_var);
    }

    join_thread(num_workers);

    for (int i = 0; i < num_files; i++)
    {
        printf("Processing matrix %s\n", argv[i+1]);

        for (int j = 0; j < matrices_num[i]; j++)
        {
            printf("The determinant is %.3e\n", shared_mem[i][j]);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish);
    printf ("\nElapsed time = %.6f s\n",  (finish.tv_sec - start.tv_sec) / 1.0 + (finish.tv_nsec - start.tv_nsec) / 1000000000.0);
    
    for (int i = 0; i < num_files; i++){
        free(shared_mem[i]);
    }

    free(workers);
    free(shared_mem);

    return 0;
}