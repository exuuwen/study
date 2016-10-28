#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX 512

struct my_msg_buf
{
    long my_msg_types;
    char msg_text[MAX];
};

/* The operation of reader is blcok in default, set IPC_NOWAIT for non-block.
if two porcesses want to get the same type message through the mq, only one can get it*/

int main(int argc, char *argv[])
{
    int msgid, time;
    struct msqid_ds msg_info;
    struct my_msg_buf my_msg;
    int receive_types;

    if (argc < 2)
    {
        printf("%s message_types(1|2)\n", argv[0]);
        return 0;
    }

    receive_types = atoi(argv[1]);

    printf("receive type is %d\n", receive_types);

    /*if the megqueu is not exist with the key in the kernel and set the flag IPC_CREAT, a new msgqueue will be created in kernel or fail to -ENOENT*/
    /* if the key is IPC_PRIVATE, the kernel will create a mew msgqueue and it should be public to the parner*/
    /* if flag set both IPC_CREAT and IPC_EXCL, the msgqueue is exit in kernel it will be fail to -EEXIST*/
    if ((msgid = msgget(/*IPC_PRIVATE*/(key_t)ftok("a", 1), 0666 | IPC_CREAT)) == -1)
    {
        perror("can not msgget:");
        exit(1);
    }
	
    printf("print the info *************\n");
    printf("get the mesg and the id is %d\n", msgid);

    msgctl(msgid, IPC_STAT, &msg_info);
    printf("the max byes is %ld\n", msg_info.msg_qbytes);

    for(time=0; time<5; time++)
    {
	if (msgrcv(msgid, (void*)&my_msg, MAX, receive_types, 0/*IPC_NOWAIT*/) == -1)
        {
            perror("can not get the msg");
            exit(1);
        }
	
        printf("msg has been recive and the types is %ld and the msg is:%s\n", my_msg.my_msg_types, my_msg.msg_text);
    }

    return 0;
}
