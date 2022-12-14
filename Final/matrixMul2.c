#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#define N 25
#define n 25

struct v
{
    size_t i;
    size_t j;
};

double A[N][N];
double B[N][N];
double C[N][N];

void setup(){
	srand(time(NULL));
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			A[i][j] = (rand()%9) + 1;
			B[i][j] = (rand()%9) + 1;
		}
	}
}
static void * multiplication(void *arg){
    struct v *data = (struct v *)arg;

    size_t l;
    for(l=0; l < N; l++)
    {
        size_t i=(data[l]).i;
        size_t j=(data[l]).j;
        double sum=0;
        size_t d;

        for (d = 0; d < N; d++)
        {
            sum = sum + A[i][d]*B[d][j];
        }

        C[i][j] = sum;
        sum = 0;
    }
    return 0;
}

int runner(void)
{
    clock_t start, end;
    start = clock();
    setup();
    pthread_t threads[n];
    size_t i, k;

    struct v **data;
    data = (struct v **)malloc(n * sizeof(struct v*));

    for(i = 0; i < n; i++)
    {
        data[i] = (struct v *)malloc(n * sizeof(struct v));

        for(k = 0; k < n; k++)
        {
            data[i][k].i = i;
            data[i][k].j = k;
        }

        pthread_create(&threads[i], NULL, multiplication, data[i]);
    }

    for(i = 0; i < n; i++)
    {
        pthread_join(threads[i], NULL);
    }
    end = clock();
    for (i = 0; i < N; i++)
    {
        for (k = 0; k < N; k++)
        {
            //printf("%f\t", C[i][k]);
        }

        //printf("\n");

        free(data[i]);
    }

    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("%f\n", cpu_time_used);
    free(data);
    
    return 0;
}
int main(void){
	for(int i = 0; i < 100; i++){
		runner();
	}
}
