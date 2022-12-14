/*
 * Colin Smith
 */
#include "multi-lookup.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
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
	char buf[10][1025];
	FILE* outputfp;
	pthread_cond_t fill, empty;
	int count;
	int done;
};
void Requester(struct thread_args *args, int argc, char **argv){
	//printf("Requester start \n");
	FILE* inputfp = NULL;
	char errorstr[SBUFSIZE];
	char* hostname;
	//printf("count is %d\n", args->count);
	for(int i = 1; i < (argc-1); i++){
		//printf("Requester here  %s\n", argv[i]);
		inputfp = fopen(argv[i], "r");
		if(!inputfp){
			//printf("Requester failed  \n");
			sprintf(errorstr, "Error Opening Input File: %s\n", argv[i]);
			perror(errorstr);
		}
		else{
			//printf("Requester here in else \n");
			hostname = (char*)malloc((SBUFSIZE+1)*sizeof(char));
			while(fscanf(inputfp, INPUTFS, hostname) > 0){
				pthread_mutex_lock(&args->lock);
				while(args->count == 10){
					//printf("waiting count = %d\n", args->count);
					pthread_cond_wait(&args->empty, &args->lock);
				}
				//printf("REQUESTER %s\n", hostname);
				strcpy(args->buf[args->count], hostname);
				args->count++;
				pthread_cond_signal(&args->fill);
				pthread_mutex_unlock(&args->lock);
				hostname = (char*)malloc((SBUFSIZE+1)*sizeof(char));


			}
			fclose(inputfp);
		}

	}
	pthread_mutex_lock(&args->lock);
	args->done = 1;
	pthread_mutex_unlock(&args->lock);
	//printf("done requester\n");
	exit(0);
}
void Resolver(struct thread_args *args){
	//printf("Resolver start \n");
	char hostname[SBUFSIZE];
	char firstipstr[INET6_ADDRSTRLEN];
	while(1){
		pthread_mutex_lock(&args->lock);
		while(args->count == 0){
			if(args->done == 1){
				//printf("returning\n");
				pthread_mutex_unlock(&args->lock);
				//printf("done resolver\n");
				exit(0);
			}
			pthread_cond_wait(&args->fill, &args->lock);
		}
		strcpy(hostname, args->buf[args->count-1]);
		args->count--;
		pthread_cond_signal(&args->empty);
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
	struct thread_args *shared_args;
/*	int shmfd;
	shared_seg_size = (1 * sizeof(struct thread_args));
	shmfd = shm_open("/shmobj", O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG); 
	if(shmfd < 0) {
		printf("failed to shmfd\n");
	}
	ftruncate(shmfd, shared_seg_size);
*/	int shared_seg_size = (1 * sizeof(struct thread_args));
	shared_args = (struct thread_args *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if(shared_args == NULL){
		printf("failed map\n");
	}
	shared_args->outputfp = fopen(argv[(argc-1)], "w");
	if(!shared_args->outputfp){
		perror("Error Opening Output File\n");
		return EXIT_FAILURE;
	}
	//init for lock of buff "lock"
	pthread_mutexattr_t lockAttr;
	pthread_mutexattr_init(&lockAttr);
	pthread_mutexattr_setpshared(&lockAttr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&shared_args->lock, &lockAttr);
	//init for lock of outputfile "outlock"
        pthread_mutexattr_t outAttr;
        pthread_mutexattr_init(&outAttr);
	pthread_mutexattr_setpshared(&outAttr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&shared_args->outlock, &outAttr);
	//init for conditional "empty"
	pthread_condattr_t emptyAttr;
	pthread_condattr_init(&emptyAttr);
	pthread_condattr_setpshared(&emptyAttr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&shared_args->empty, &emptyAttr);
	//init for conditional "fill"
	pthread_condattr_t fillAttr;
	pthread_condattr_init(&fillAttr);
	pthread_condattr_setpshared(&fillAttr, PTHREAD_PROCESS_SHARED);
	pthread_cond_init(&shared_args->fill, &fillAttr);
	shared_args->done = 0;
        shared_args->count = 0;	
	pid_t pid;
	for(int i = 1; i<RESOLVER_THREADS; i++){
		pid = fork();	
		if(pid == 0){
			Resolver(shared_args);
			//printf("did stuff %d\n", pid);
			return 0;
		}
	}
	Requester(shared_args, argc, argv);
	fclose(shared_args->outputfp);
	pthread_mutex_destroy(&shared_args->outlock);
	pthread_mutex_destroy(&shared_args->lock);
	pthread_cond_destroy(&shared_args->fill);
	pthread_cond_destroy(&shared_args->empty);
	//printf("All threads complete!\n");
	return 0;
}
