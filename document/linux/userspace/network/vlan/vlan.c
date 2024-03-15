#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_vlan.h>


int vlan_create(const char *master, unsigned vid)
{
    int result = 0;

    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0) 
        return -1;

    struct vlan_ioctl_args v;
    memset(&v, 0, sizeof v);
    v.cmd = ADD_VLAN_CMD;
    strcpy(v.device1, master);
    v.u.VID = vid;

    result = ioctl(fd, SIOCSIFVLAN, &v);
    if (result < 0) 
    {
        perror("create_vlan:");
        result = -1;
    }

    close(fd);

    return result;
}

int vlan_delete(const char *ifname)
{
    int result = 0;

    int fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0) 
        return -1;

    struct vlan_ioctl_args v;
    memset(&v, 0, sizeof v);
    v.cmd = DEL_VLAN_CMD;
    strcpy(v.device1, ifname);

    result = ioctl(fd, SIOCSIFVLAN, &v);
    if (result < 0) 
    {
        perror("delete_vlan:");
        result = -1;
    }

    close(fd);

    return result;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("%s create master_name vlan_id(0~4096)\n", argv[0]);
        printf("%s delete if_name\n", argv[0]);
        return -1;
    }

    if (strcmp(argv[1], "create") == 0)
    {
        if (argc < 4)
        {   
            printf("%s create master_name vlan_id(0~4096)\n", argv[0]);
            return -1;
        }

        int vlan_id = atoi(argv[3]);
        int ret = vlan_create(argv[2], vlan_id);
        if (ret < 0)
            printf("vlan create fail\n");
        else
            printf("vlan create success\n");
        
    }
    else if (strcmp(argv[1], "delete") == 0)
    {
        if (argc < 3)
        {   
            printf("%s delete if_name\n", argv[0]);
            return -1;
        }

        int ret = vlan_delete(argv[2]);
        if (ret < 0)
            printf("vlan delete fail\n");
        else
            printf("vlan delete success\n");
        
    }
    else
    {
         printf("%s create master_name vlan_id(0~4096)\n", argv[0]);
         printf("%s delete if_name\n", argv[0]);
         return -1;
    }

    return 0;
}
