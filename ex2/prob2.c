#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int order = 0;

double det_calc(double matrix[order][order], int n)
{
    double det=1;
    double aux;

    for(int i=0; i<n; i++){
        for(int j=i+1; j<n; j++)
        {
            aux = matrix[j][i]/matrix[i][i];

            for(int k=0; k<n; k++)
            {
                matrix[j][k] = matrix[j][k]-aux*matrix[i][k];
            }
        }
    }

    for(int i=0; i<n; i++){
        det = det*matrix[i][i];
    }
    return det;
}


void matrixRead(FILE* f, char* fname)
{
    
    int n_matrixes;
    fread(&n_matrixes, 4, 1, f);
    fread(&order, 4, 1, f);

    double matrix[order][order];

    for(int j=1; j<=n_matrixes; j++){

        printf("Processing matrix %d\n", j);

        for (int i=0; i<order; i++){
            fread(&matrix[i], 8, order, f);
        }


        double det = det_calc(matrix, order);

        printf("The determinant is %.3e\n", det);
    }
}


int main(int argc, char **argv) { 
    clock_t t;
    t = clock();

    for (int i=1; i<argc; i++){
        FILE * f;
        f = fopen(argv[i], "rb");

        matrixRead(f, argv[i]);
        fclose (f);
    }
    
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("Elapsed time %f s\n", time_taken);
    
    return 0;


}