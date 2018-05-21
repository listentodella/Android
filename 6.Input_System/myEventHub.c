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

struct inotify_event {
   int      wd;       /* Watch descriptor */
   uint32_t mask;     /* Mask of events */
   uint32_t cookie;   /* Unique cookie associating related
                         events (for rename(2)) */
   uint32_t len;      /* Size of name field */
   char     name[];   /* Optional null-terminated name */
};
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


int add_to_epoll(int fd, int epollFd)
{
    int result;
    struct epoll_event eventItem;
    memset(&eventItem, 0, sizeof(eventItem));
    eventItem.events = EPOLLIN;
    eventItem.data.fd = fd;
    result = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &eventItem);

    return result;
}

int rm_from_epoll(int fd, int epollFd)
{
    // int result;
    // struct epoll_event eventItem;
    // memset(&eventItem, 0, sizeof(eventItem));
    // eventItem.events = EPOLLIN;
    // eventItem.data.fd = fd;
    result = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &eventItem);

    return result;
}

int read_process_inotify_fd(int inotify_fd)
{
    int res;
    char event_buf[DATA_MAX_LEN];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;

    /*read*/
    res = read(inotify_fd, event_buf, sizeof(event_buf));
    if(res < (int)sizeof(*event)) {
        if(errno == EINTR)
            return 0;
        printf("could not get event, %s\n", strerror(errno));
        return -1;
    }

    /*process
     * read data: 1 or more inotify_event
     * they have different lengths
     * process them one by one
     */
    while(res >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        //printf("%d: %08x \"%s\"\n", event->wd, event->mask, event->len ? event->name : "");
        if(event->len) {
            //strcpy(filename, event->name);
            if(event->mask & IN_CREATE) {
                //openDeviceLocked(devname);
                printf("create file : %s\n", event->name);
                /*for each file:
                 * open it
                 * add it to epoll: epoll_ctl(... EPOLL_CTL_ADD ...)
                 *
                 * event->name only include file_name, we need file path
                 */
                char *name = malloc(512);
                sprintf(name, "%s/%s", base_dir, event->name);
                int tmpFd = open(name, O_RDWR);
                add_to_epoll(tmpFd, mEpollFd);
            } else {
                printf("delete file : %s\n", event->name);
                //ALOGI("Removing device '%s' due to inotify event\n", devname);
                //closeDeviceByPathLocked(devname);
                /*for each file:
                 * del it from epoll: epoll_ctl(... EPOLL_CTL_DEL ...)
                 * close it
                 */
                rm_from_epoll(tmpFd, mEpollFd);
            }
        }
        event_size = sizeof(*event) + event->len;
        res -= event_size;
        event_pos += event_size;
    }

    return 0;
}


int main(int argc, char **argv)
{
    if (argv != 2) {
        printf("Usage: %s <base dir>\n", argv[0]);
        return -1;
    }

    base_dir = argv[1];

    char buf[DATA_MAX_LEN];
    int i;
    int result;

    /*inotify_init*/
    mINotifyFd = inotify_init();

    /*add watch, inotify a base dir*/
    //result = inotify_add_watch(mINotifyFd, base_dir, IN_DELETE | IN_CREATE | IN_MODIFY | IN_OPEN);
    result = inotify_add_watch(mINotifyFd, base_dir, IN_DELETE | IN_CREATE);
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
            if (mPendingEventItems[i].data.fd == mINotifyFd)//表明是监测的目录发生变化
                read_process_inotify_fd(mINotifyFd);
            else {//表明是监测的目录下的文件发生变化
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
