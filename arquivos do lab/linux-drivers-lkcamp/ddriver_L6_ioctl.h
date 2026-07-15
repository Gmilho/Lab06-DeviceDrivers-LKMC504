#ifndef DDRIVER_L6_IOCTL_H
#define DDRIVER_L6_IOCTL_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#endif

#define MNUM 'k'
#define PATH_SZ 512

struct conf_st {
    char path[PATH_SZ];
};

#define CONF_PATH   _IOW(MNUM, 1, struct conf_st)

#endif