#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

char use[] = "Usage: ./partial_sum3 n m\n";

pthread_mutex_t mutex;
long total_sum = 0;

void *partial_sum(void *arg)
{
	int j = 0;
	int ni = ((int *)arg)[0];
	int nf = ((int *)arg)[1];

	for (j = ni; j <= nf; j++) {
        /* Adquire the mutex lock */
        pthread_mutex_lock(&mutex);
        total_sum = total_sum + j;
        /* Release the mutex lock */
        pthread_mutex_unlock(&mutex);
    }
		
	pthread_exit(0);
}

int main(int argc, char *argv[])
{
	pthread_t* th_pool;
    int nThreads;
    int i, **num, fn, ln;
    long n, quickCalc;

    /* Minimum args requiered = 3: partial_sum3 5 50000 */
    if(argc < 2) {
        fprintf(stderr, "%s", use);
        exit(EXIT_FAILURE);
    }

    /* Parse command line arguments */
    nThreads = atoi(argv[1]);
    n = atol(argv[2]);

    if(nThreads > n) {
        fprintf(stderr, "nThreads must be below n!\n");
        exit(EXIT_FAILURE);
    }

    /* A nThread array is created and filled up with the corresponding numbers */
    if((num = (int **) malloc(nThreads * sizeof(num[0]))) == NULL) {
        fprintf(stderr, "Error allocating memory...\n");
        exit(EXIT_FAILURE);
    }

    fn = 1; ln = (n/nThreads)-1;

    for(i = 0; i < nThreads; ++i) {
        int j = 0;
        if((num[i] = (int *) malloc(2 * sizeof(int))) == NULL) {
            for(j = 0; j < i; ++j)
                free(num[i]);
            fprintf(stderr, "Error allocating memory...\n");
            exit(EXIT_FAILURE);
        }
        
        num[i][0] = fn;

        if(i == nThreads-1)
            num[i][1] = ln + 1;
        else
            num[i][1] = ln;
        
        fn = ln + 1;
        ln += (n/nThreads);

        printf("num[%d][2] = { %d, %d } \n", i, num[i][0], num[i][1]);
    }

    /* Create and initialize the mutex lock */
    pthread_mutex_init(&mutex, NULL);

    /* Thread pool is allocated */
    if((th_pool = (pthread_t *) malloc(nThreads * sizeof(th_pool[0]))) == NULL) {
        fprintf(stderr, "Error allocating the pool of threads!\n");
        exit(EXIT_FAILURE);
    }

	/* nThreads are created */
    for(i = 0; i < nThreads; ++i) {
        pthread_create(&th_pool[i], NULL, partial_sum, (void *)num[i]);
    }

	/* The main thread waits until the thread pool is completed */
	for(i = 0; i < nThreads; ++i)
        pthread_join(th_pool[i], NULL);

    quickCalc = (n*(n+1))/2;
	printf("total_sum=%ld and it should be %ld\n", total_sum, quickCalc);

	exit(EXIT_SUCCESS);
}
