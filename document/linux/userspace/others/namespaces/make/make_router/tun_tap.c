#include <stdio.h>   
#include <string.h>    
#include <sys/socket.h>  

#include <linux/if_tun.h>   
#include <sys/ioctl.h> 
#include <linux/if.h>   
#include <fcntl.h> 

static int tun_handle(const char *devname, int flags, int persist, const uid_t *uid, const gid_t *gid) 
{
    struct ifreq ifr;
    int fd, err;
    char *clonedev = "/dev/net/tun";


    /* open the clone device */
    if ((fd = open(clonedev, O_RDWR)) < 0) 
    {
        perror("open /dev/net/tun");
        return fd;
    }

    /* preparation of the struct ifr, of type "struct ifreq" */
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags | IFF_NO_PI;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

    if (devname) 
    {
        /* if a device name was specified, put it in the structure; otherwise,
        * the kernel will try to allocate the "next" device of the
        * specified type */
        strncpy(ifr.ifr_name, devname, IFNAMSIZ);
    }

    /* try to create the device */
    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) 
    {
        close(fd);
        perror("TUNSETIFF");
        return -1;
    }

    /*set dev to persist*/
    if (ioctl(fd, TUNSETPERSIST, persist) < 0)
    {
        close(fd);
        perror("TUNSETPERSIST");
        return -1;
    }

    if (uid)
    {
        if(ioctl(fd, TUNSETOWNER, *uid) < 0)
        {
            close(fd);
            perror("TUNSETOWNER");
            return -1;
        }
    }

    if (gid)
    {
        if(ioctl(fd, TUNSETGROUP, *gid) < 0)
        {
            close(fd);
            perror("TUNSETOWNER");
            return -1;
        }
    }

    close(fd);
    return 0;
}

int tun_create(const char *devname, const uid_t *uid, const gid_t *gid)
{
    return tun_handle(devname, IFF_TUN, 1, uid, gid);
}

int tun_delete(const char *devname)
{
    return tun_handle(devname, IFF_TUN, 0, NULL, NULL);
}

int tap_create(const char *devname, const uid_t *uid, const gid_t *gid)
{
    return tun_handle(devname, IFF_TAP, 1, uid, gid);
}

int tap_delete(const char *devname)
{
    return tun_handle(devname, IFF_TAP, 0, NULL, NULL);
}






