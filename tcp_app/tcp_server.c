#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h> 

#include <signal.h>
#include <semaphore.h>
#include <pthread.h>

#include <netinet/in.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
volatile bool quitTime = 0;
volatile int server_socket = -1;
sem_t sem, thread_sem; 

typedef struct socket_struct 
{
    pthread_t tid[5];
    volatile int fd; /* No optimization; updated in main() */
} socket_struct;

static void intHandler(int sig)
{
    printf("Triggered signal handler\n");
    quitTime = 1;
}

void *socketThread(void *arg)
{
    socket_struct *socket = NULL;
    char *sendMsg = "Sending to client\n";
    int len = 0;
    int fd = 0;

    char *client_response = NULL;

    printf("Entered socket thread function\n");
    socket = ((socket_struct*)arg);
    
    client_response = (char*)malloc(1024*sizeof(char));    
    if (!client_response)
        goto exit;

    while (!quitTime)
    {
        /* sem_wait */
        sem_wait(&sem);
       
        /* Check 'exit' sem_post */ 
        if(quitTime)
            break;

        /* Local cache */
        fd = socket->fd;

        /* Send stream of msgs */
        len = (int)strlen(sendMsg);
        
        int i = 0;
        while (i < 5)
        {
            printf("Sending a message [TCP Server to client] from thread %ld. strlen: %d\n", pthread_self(), len);
            send(fd, sendMsg, len, 0);
            i++;
        }

        //pthread_mutex_lock(&lock);
        int bytes = 0;
        int j = 0;
        while (j < 5)
        {
            bytes = recv(fd, client_response, 1024*sizeof(char), 0);
            j++;
            
            if (0 >= bytes)
            {
                printf("Byte count error. Return: %d. Socket closed.\n", bytes);
                goto exit;
            }
        
            printf("client_response [thread %ld]  %d is: %s and its len is : %d\n", pthread_self(), j, client_response, bytes);
        }

        //pthread_mutex_unlock(&lock);
        close(fd);

        /* sem_post on thread wait (init = 5)*/
        sem_post(&thread_sem);
    }

exit:
    if(client_response)
        free(client_response);
    pthread_exit(NULL);
 
    return NULL;
}

int main() 
{
	int client_socket = -1;
    socket_struct *client_data = NULL;
    fd_set readfds;
    struct timeval timeout;
    int i = 0;

    /* signal handler */
    if (SIG_ERR == signal(SIGINT, intHandler))
    {
        printf("Signal handler creation failure\n");
        goto exit;
    }

    /* initialize semaphores */
    sem_init(&sem, 1, 0);
    sem_init(&thread_sem, 1, 5);

    /* initialize read fd set */
    FD_ZERO(&readfds);

    client_data = (socket_struct*)malloc(sizeof(socket_struct));
    if (!client_data)
        goto exit;

    while (i < 5)
    {

        if(0 != pthread_create(&(client_data->tid[i++]), NULL, socketThread, client_data))
        {
            printf("Failed to create thread\n");
            goto exit;
        }
    }
    
	server_socket = socket(AF_INET, SOCK_STREAM, 0);

	/* server Address */
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET; /* IPv4 */
	server_address.sin_port = htons(9002); /* TCP Port */
	server_address.sin_addr.s_addr = INADDR_ANY;

	int bind_status = bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));
    if (-1 == bind_status)
        printf("Bind failed\n");

    /* listen for 20 outstanding connections on open port 9002 */
	int listen_status = listen(server_socket, 20);
    if (-1 == listen_status)
        printf("Listen failed\n");
    
    /* 5 threads in parallel */
    while (!quitTime)
    {
        /* clear read fd_set before usage */
        FD_ZERO(&readfds);

        /* set master socket */
        FD_SET(server_socket, &readfds);

        /* Initialize timeout */
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;

        /* select() call can avoid closing the socket in the intHandler b/c
        * you won't need to force the accept to fail. select() can poll and 
        * you can check for quittime */
        int num_fd = select(server_socket + 1,&readfds,NULL,NULL,&timeout);

        /* if SIGINT triggered, select will come here */
        if (quitTime)
            goto exit;

        if (num_fd <= 0)
            continue;

        if (FD_ISSET(server_socket, &readfds))
        {
            client_socket = accept(server_socket, NULL, NULL);
#ifdef DEBUG_ON
            printf("DEBUG: Accept completed, socket_fd: %d\n", client_socket);
#endif
            if (-1 == client_socket)
            {
                printf("call to accept() failed\n");
                goto exit;
            }
        }

        client_data->fd = client_socket;

        /* semaphore signal - start thread */
        sem_post(&sem);

        /* sem_wait(init=5) on thread count */
        sem_wait(&thread_sem);
    }
exit:	
    if ( -1 != server_socket)
        close(server_socket);
    
    for (int j = 0; j < 5; j++)
    {
        /* Mandatory signal after quitTime */
        sem_post(&sem);
    }
    
    for (int j = 0; j < 5; j++)
    {
        pthread_join(client_data->tid[j], NULL);
    }
    
    sem_destroy(&sem);
    free(client_data);

	return 0;
}
