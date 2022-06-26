#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common.h"
#include <cuda_runtime.h>

__global__ void det_calc_onDevice(double *matrix,double * results, int n);
void det_calc_onHost(int id, double *matrix, int n);

int main(int argc, char **argv) {

    printf("%s Starting...\n", argv[0]);

    // set up device
    int dev = 0;
    cudaDeviceProp deviceProp;
    CHECK(cudaGetDeviceProperties(&deviceProp, dev));
    printf("Using device %d: %s\n", dev, deviceProp.name);
    CHECK(cudaSetDevice(dev));

    char *files[5];
    int input = 0;
    int num_files = 0;
    double *h_matrix;
    double *d_matrix;
    double *h_results;
    double *d_results;

    while (input != -1) {
        input = getopt(argc, argv, "t:i:");
        if (input == 'i')
            files[num_files++] = optarg;
    }

    for (int i=0; i<num_files; i++){
        FILE * f;
        f = fopen(files[i], "rb");

        int a;
        int n_matrices;
        int order;
        a=fread(&n_matrices, 4, 1, f);
        a=fread(&order, 4, 1, f);

        //read and store on host each matrix
        h_matrix = (double*) malloc(n_matrices*order*order*sizeof(double));
        a=fread(h_matrix, 8, n_matrices*order*order, f);

        // allocate space for Host results
        h_results = (double *)malloc(sizeof(double) * n_matrices * order);

        input = a;
        
        for (int i=0; i<n_matrices;i++){
                h_results[i] = 1;
        }

        printf("\n\nProcessing file: %s\n", argv[num_files + 1]);

        // prepare launching grid
        dim3 grid, block;
        grid.x = n_matrices;
        grid.y = 1;
        grid.z = 1;
        block.x = order;
        block.y = 1;
        block.z = 1;


        //malloc device memory
        CHECK(cudaMalloc(&d_results, 8*n_matrices*order));
        CHECK(cudaMalloc(&d_matrix, n_matrices*order*order*8));

        // transfer data from host to device
        CHECK(cudaMemcpy(d_matrix, h_matrix, n_matrices*order*order*8, cudaMemcpyHostToDevice));
        CHECK(cudaMemcpy(d_results, h_results, n_matrices*order*8, cudaMemcpyHostToDevice));

        double iStart = seconds();
        // det_calc_onHost(n_matrices, h_matrix, order);
        // double iElapsHost = seconds() - iStart;

        // free matrices on Host
        free(h_matrix);

        iStart = seconds();
        det_calc_onDevice<<< grid, block >>>(d_matrix, d_results, order);
        CHECK(cudaDeviceSynchronize());
        double iElaps = seconds() - iStart;

        // free matrices on Device
        cudaFree(d_matrix);

        // copy kernel result back to host side
        cudaMemcpy(h_results, d_results, sizeof(double) * n_matrices, cudaMemcpyDeviceToHost);

        // free device global memory
        cudaFree(d_results);

        printf("\n\n\n\n");

        for (int j = 0; j < n_matrices; j++) {
            printf("Processing matrix %d\n", j + 1);
            printf("The determinant is %.3e\n", h_results[j]);
        }
        // printf("Time elapsed on det_calc_onHost >>> %f\n", iElapsHost);
        printf("Time elapsed on det_calc_onDevice >>> %f\n", iElaps);
        
        fclose (f);
    }

    return 0;


}


__global__ void det_calc_onDevice(double *matrix,double* results, int n)
{

    int m_idx = blockIdx.x+gridDim.x*blockIdx.y+gridDim.x*gridDim.y*blockIdx.z;

    int c_idx = threadIdx.x+blockDim.x*threadIdx.y+blockDim.x*blockDim.y*threadIdx.z;

    int mat = m_idx*n*n;
    int col = mat + c_idx;

    for(int i=0; i<n; i++){

            if(c_idx < i) continue;

            if(threadIdx.x == i){
                    results[m_idx] *= matrix[mat+i + i*n];
                    continue;
            }
            for(int j = i+1; j< n; j++)
                        matrix[col + j*n] -= matrix[mat+i + j*n] * matrix[col + i*n] / matrix[mat+i + i*n];

        __syncthreads();
    }

}


void det_calc_onHost(int n_mat, double *matrix, int n)
{

    for(int m_idx=0; m_idx < n_mat; m_idx++){

	int mat = m_idx * n * n;

        double det = 1;


        for (int c_idx = 0; c_idx < n; c_idx++){

            int col = mat + c_idx;

            for(int i=0; i<n; i++){

                 if(c_idx < i) continue;

                 if(c_idx == i){
                    det *= matrix[mat+i + i*n];
                    continue;
                }

                 for(int j = i+1; j< n; j++)
                        matrix[col + j*n] -= matrix[mat+i + j*n] * matrix[col + i*n] / matrix[mat+i + i*n];

            }

        }

        //printf("Processing matrix %d\n", m_idx+1);
        //printf("The determinant is %.3e\n", det);


    }
}