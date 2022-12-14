/*
 * Colin Smith
 */
#include "multi-lookup.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "util.h"
#include "queue.h"
#include "unistd.h"
#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"
#define RESOLVER_THREADS 3
#define MAX_THREADS 4
struct thread_args{
	pthread_mutex_t lock;
	pthread_mutex_t outlock;
	queue q;
	FILE* outputfp;
	int argc;
	char** argv;
};
int done = 0;
const int qSize = 50;
void* Requester(void* _args){
	struct thread_args *args = (struct thread_args*) _args;
	FILE* inputfp = NULL;
	char errorstr[SBUFSIZE];
	char* hostname;
	for(int i = 1; i < (args->argc-1); i++){
		inputfp = fopen(args->argv[i], "r");
		if(!inputfp){
			sprintf(errorstr, "Error Opening Input File: %s", args->argv[i]);
			perror(errorstr);
		}
		else{
			hostname = (char*)malloc((SBUFSIZE+1)*sizeof(char));
			while(fscanf(inputfp, INPUTFS, hostname) > 0){
				pthread_mutex_lock(&args->lock);
				while(queue_is_full(&args->q) == 1){
					pthread_mutex_unlock(&args->lock);
					usleep(rand()%100+1);
					pthread_mutex_lock(&args->lock);
				}
				//printf("REQUESTER %s\n", hostname);
				queue_push(&args->q, hostname);
				pthread_mutex_unlock(&args->lock);
				hostname = (char*)malloc((SBUFSIZE+1)*sizeof(char));


			}
			fclose(inputfp);
		}

	}
	done = 1;
	return NULL;
}
void* Resolver(void* _args){
	struct thread_args *args = (struct thread_args*) _args;
	char hostname[SBUFSIZE];
	char firstipstr[INET6_ADDRSTRLEN];
	while(1){
		pthread_mutex_lock(&args->lock);
		while(queue_is_empty(&args->q) == 1){
			if(done == 1){
				//printf("returning\n");
				pthread_mutex_unlock(&args->lock);
				return NULL;
			}
			pthread_mutex_unlock(&args->lock);
			usleep(rand()%100+1);
			//printf("waiting resolver\n");
			pthread_mutex_lock(&args->lock);
		}
		strcpy(hostname, queue_pop(&args->q));
		pthread_mutex_unlock(&args->lock);
		//printf("RESOLVER %s\n", hostname);
		if(dnslookup(hostname, firstipstr, sizeof(firstipstr)) == UTIL_FAILURE){
			fprintf(stderr, "dnslookup error: %s\n", hostname);
			strncpy(firstipstr, "", sizeof(firstipstr));
		}
		pthread_mutex_lock(&args->outlock);
		fprintf(args->outputfp, "%s,%s\n", hostname, firstipstr);
		pthread_mutex_unlock(&args->outlock);
	}
}
int main(int argc, char** argv){
	struct thread_args *args = malloc(sizeof(struct thread_args));
	if(queue_init(&args->q, 0) == QUEUE_FAILURE){
		printf("queue failed\n");
	}
	args->outputfp = fopen(argv[(argc-1)], "w");
	if(!args->outputfp){
		perror("Error Opening Output File");
		return EXIT_FAILURE;
	}
	args->argc = argc;
	args->argv = argv;
	pthread_t tid[MAX_THREADS];
	if(pthread_mutex_init(&args->lock, NULL)!=0){
		printf("failed to create lock\n");
	}
	if(pthread_mutex_init(&args->outlock, NULL)!=0){
		printf("failed to create outlock\n");
	}
	int i;
	int res;
	int req;

	//printf("creating requester thread\n");
	req = pthread_create(&tid[0], NULL, &Requester, args);
	if(req != 0){
		printf("Requester thread creation failed");
	}

	for(i = 1; i<RESOLVER_THREADS+1; i++){
		res = pthread_create(&tid[i], NULL, &Resolver, args);
		if(res != 0){
			printf("Resolver %d thread creation failed", i);
		}
		else{
		//	printf("Creating %d resolver thread\n", i);
		}
	}

	for(i = 0; i<MAX_THREADS; i++){
		pthread_join(tid[i], NULL);
	}
	fclose(args->outputfp);
	free(args);
	pthread_mutex_destroy(&args->outlock);
	pthread_mutex_destroy(&args->lock);
	//printf("All threads complete!\n");
	return 0;
}
