#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>

//This is just a test function.
void *func(void *arg)
{
    printf(" !!! This is a temporary function. Replace it with actual function call\n");
}

int main()
{
    int n_threads;//Number of threads.
    
    
    //Number of clients = Number of threads (cores).
    printf("Enter the number of Clients: ");
    scanf("%d", &n_threads);
    
    pthread_t thread_array[n_threads];//Array to store the threads.

    //Create threads(cores) for clients.
    for(int i = 0; i<n_threads; i++)
    {
        pthread_create(&thread_array[i], NULL, func, NULL);
    }
    
    //Wait for other threads to complete.
    for(int i = 0; i<n_threads; i++)
    {
        pthread_join(thread_array[i], NULL);
    }
    
    return 0;
}
