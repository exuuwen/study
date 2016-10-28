#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

#define BUFSIZE 50

int main(int argc, char *argv[])
{
    int semid;
    int shmid;
    int running = 1;
    int value;
    char  *sharem = NULL;
    struct sembuf sem_b;
	
    semid = semget((key_t)123456, 1, 0666 /*| IPC_CREAT*/);
    if (semid == -1)
    {
        perror("can not get sem signal");
        exit(1);
    }
		
    if ((shmid = shmget((key_t)654321,(size_t)2048, 0600 /*| IPC_CREAT*/)) == -1) 
    {
        perror("can not get share mem");
        exit(1);
    }

    if ((sharem = shmat(shmid, NULL, 0)) == NULL)
    {
        perror("can mount the shared mem");
        exit(1);
    }

    printf("shmid is %d \n", shmid);			
    //sem_b.sem_flg = SEM_UNDO;
	
    while (running)
    {
        sem_b.sem_op = -1;
        if(semop(semid, &sem_b, 1) == -1)
        {
            perror("can not chage sem value");
            exit(1);
        }
        printf("print the data \n");
        printf("%s\n", (char*)sharem);

        if (strcmp(sharem, "end") == 0)
            running = 0;
    }	

end:
    shmdt(sharem);
    return 0;
}

