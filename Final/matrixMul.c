#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#define size 3
struct input{
	int matA[size][size];
	int matB[size][size];
	int matRes[size][size];
	int counta;
	int countb;
};
pthread_mutex_t lock;
void* mult(void* arg){
	struct input* input = (struct input*) arg;
	pthread_mutex_lock(&lock);
	while(input->counta < size){
		int counta, countb;
		if(input->countb == size){
			countb = 0;
			input->countb = 0;
			input->counta++;
		}
		else {
			countb = input->countb;
			input->countb++;
		}
		counta = input->counta;
		pthread_mutex_unlock(&lock);
		for(int i = 0; i < size; i++){
			input->matRes[counta][countb] += (input->matA[counta][i] * input->matA[i][countb]);
		}
		pthread_mutex_lock(&lock);

	}
	pthread_mutex_unlock(&lock);
	return 0;
}
void setUp(){
	clock_t start, end;
	start = clock();
	struct input* data = malloc(sizeof(struct input));
	int i,j;
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			data->matA[i][j] = rand() % 10;
	for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			data->matB[i][j] = rand() % 10;
        for (i = 0; i < size; i++)
		for (j = 0; j < size; j++)
			data->matRes[i][j] = 0;
	data->counta = 0;
	data->countb = 0;
	if(pthread_mutex_init(&lock, NULL) != 0)
		printf("failed to create lock\n");
	pthread_t *threads;
	threads = (pthread_t*)malloc(2*sizeof(pthread_t));
	for (i = 0; i < 2; i++){
		pthread_create(&threads[i], NULL, mult, (void*)(data));
	}
	for (i = 0; i < 2; i++)
	{
		pthread_join(threads[i], NULL);
	}
	for(i = 0; i < size; i++){
		for(j = 0; j < size; j++){
			//data->matA[i][j] = '.';
			//data->matB[i][j] = '.';
			data->matRes[i][j] = '.';
			//printf("%d", data->matA[i][j]);
			//printf("%d", data->matB[i][j]);
			printf("%d",data->matRes[i][j]);
		}
	}
	free(data);
	end = clock() - start;
	double totalTime = ((double)end)/CLOCKS_PER_SEC;
	printf("%f \n", totalTime);
}
int main(int argc, char* argv[])
{
	int i;
	int small = 175;
	int medium = 50; 
	int large = 175;
	printf("Small Matrix\n");
	for(i = 0; i < 100; i++)
		setUp();
/*	printf("Medium Matrix\n");
	for(i = 0; i < 5; i++)	
		setUp(medium);
	printf("Large Matrix\n");
	for(i = 0; i < 5; i++)	
		setUp(large);
*/	(void)argc;
	(void)argv;
	return 0;
}
