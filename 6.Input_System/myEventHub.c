/*************************************************************************
	> File Name: myEventHub.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: Thu 10 May 2018 11:44:24 AM CST
 ************************************************************************/

#if 0
typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;

struct epoll_event {
    __uint32_t events; //epoll event
    epoll_data_t data; //user data variable
};

epoll events:
EPOLLIN
EPOLLOUT

man 2 epoll_ctl, you can get:
EPOLL_CTL_ADD
EPOLL_CTL_MOD
EPOLL_CTL_DEL

#endif


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DATA_MAX_LEN 512
#define MAX_FILES 1000

static char *base_dir;
static char *epoll_files[MAX_FILES];



int main(int argc, char **argv)
{
    if (argv != 2) {
        printf("Usage: %s <base dir>\n", argv[0]);
        return -1;
    }

    base_dir = argv[1];

    int mINotifyFd, mEpollFd;
    char buf[DATA_MAX_LEN];
    int i;
    int result;

    /*inotify_init*/
    mINotifyFd = inotify_init();

    /*add watch, inotify a base dir*/
    //result = inotify_add_watch(mINotifyFd, argv[1], IN_DELETE | IN_CREATE | IN_MODIFY | IN_OPEN);
    result = inotify_add_watch(mINotifyFd, argv[1], IN_DELETE | IN_CREATE);
    if (result < 0) {
        printf("could not register inotify for base dir <%s>\n", argv[1]);
        return -1;
    }

    //maxinum number of signalled FDs to handle at a time
    static const int EPOLL_MAX_EVENTS = 16;

    //the array of pending epoll events and the index of the next event to be handled
    struct epoll_event mPendingEventItems[EPOLL_MAX_EVENTS];

    mEpollFd = epoll_create(8);

    //how to add and track file especially new file?
    //actually we can track the base dir fd
    add_to_epoll(mINotifyFd, mEpollFd);

    /*read*/
    while(1) {
        int pollResult = epoll_wait(mEpollFd, mPendingEventItems,
                EPOLL_MAX_EVENTS, -1);
        for(i = 0; i < pollResult; i++) {
            if (mPendingEventItems[i].data.fd == mINotifyFd)//
                read_process_inotify_fd(mINotifyFd);
            else {//
                printf("Reason : 0x%x\n", mPendingEventItems[i].events);
                int len = read(mPendingEventItems[i].data.fd, buf, DATA_MAX_LEN);
                buf[len] = '\0';
                printf("get data: %s\n", buf);
                //sleep(3);
            }
        }
    }

    //epoll wait







    return 0;
}
