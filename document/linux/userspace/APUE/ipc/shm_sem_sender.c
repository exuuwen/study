#include <stdio.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>

/*
The sender start and stop all the ids
*/

int main(int argc, char *argv[])
{
    int semid;
    int shmid;
    int running = 1;
    int value;
    void *sharem = NULL;
    struct sembuf sem_b;

    semid = semget((key_t)123456, 1, 0666 | IPC_CREAT);
    if (semid == -1)
    {
        perror("can not get sem signal");
        exit(1);
    }

    if (semctl(semid, 0, SETVAL, 0) == -1)
    {
        perror("can not set sem value");
        semctl(semid, 0, IPC_RMID, 0);		
        exit(1);
    }
		
    if ((shmid = shmget((key_t)654321, (size_t)2048, 0600 | IPC_CREAT)) == -1) 
    {
        perror("can not get share mem");
        semctl(semid, 0, IPC_RMID, 0);		
        exit(1);
    }

    printf("shmid is %d \n", shmid);
    if ((sharem = shmat(shmid, NULL, 0)) == NULL)
    {
        perror("can mount the share mem");
        shmctl(shmid, IPC_RMID, 0);
        semctl(semid, 0, IPC_RMID, 0);
        exit(1);
    }
		
    sem_b.sem_flg = SEM_UNDO;	
    while(running)
    {
        sem_b.sem_op = 0;
        if (semop(semid, &sem_b, 1) == -1)
        {
            perror("pos = 0, semop fail:");
            goto end4;
        }		

	printf("Enter the data to the shared memory\n");
        scanf("%s", (char*)sharem);

        sem_b.sem_op = 1;
        if (semop(semid, &sem_b, 1) == -1)
        {
            perror("semop fail:");
            goto end4;
        }
		
        if (strcmp(sharem, "end") == 0)
            running = 0;		
    }

end4:
    shmdt(sharem);
    shmctl(shmid, IPC_RMID, 0);
    semctl(semid, 0, IPC_RMID, 0);
    
    return 0;
}

