#ifndef _H_TUN_TAP_H
#define _H_TUN_TAP_H

#include <unistd.h>
#include <sys/types.h>

int tun_create(const char *devname, const uid_t *uid, const gid_t *gid);

int tun_delete(const char *devname);

int tap_create(const char *devname, const uid_t *uid, const gid_t *gid);

int tap_delete(const char *devname);

#endif
