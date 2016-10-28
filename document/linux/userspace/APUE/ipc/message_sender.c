#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX  512

struct my_msg_buf
{
    long my_msg_types;
    char msg_text[MAX];
};

/*
the opeertion of writer will be block if the message queue is full. if there is not enough availble space and send with flag IPC_NOWAIT, it will return immediately
The space can be setten by msgctl in msg_info.msg_qbytes 
*/

int main(int argc, char *argv[])
{
    int msgid, time;
    struct msqid_ds msg_info;
    struct my_msg_buf msg_data;	

    /*reader start to create msg queue*/
    if ((msgid = msgget((key_t)ftok("a", 1), 0666 /*| IPC_CREAT*/)) == -1)
    {
        perror("can not msgget:");
        exit(1);
    }

    printf("print the info *************\n");
    printf("get  the  mesg  and  the  id  is %d\n", msgid);

    msgctl(msgid, IPC_STAT, &msg_info);
	
    printf("the max byes is %ld\n", msg_info.msg_qbytes);

    for (time=0; time<12; time++)
    {
	if (time%2)
            msg_data.my_msg_types = 1;
        else
            msg_data.my_msg_types = 2;
        sprintf(msg_data.msg_text, "%s %d", "hello world", time);
	
        if ((msgsnd(msgid, (void*)&msg_data, MAX, 0)) == -1)
        {
            perror("can not send the msg:");
            exit(1);
        }

        printf("msg has been send and the types is %ld and the msg is %s\n", msg_data.my_msg_types, msg_data.msg_text);
    }

    sleep(3);
    /*it will delete the msgqueue*/
    msgctl(msgid, IPC_RMID, 0);

    return 0;

}
