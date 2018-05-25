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
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
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


int add_to_epoll(char *name, int fd, int epollFd)
{
    int result;
    struct epoll_event eventItem;
    memset(&eventItem, 0, sizeof(eventItem));
    eventItem.events = EPOLLIN;
    eventItem.data.fd = fd;
    result = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &eventItem);
    if (name)
        printf("file %s has been added into epollFd...\n", name);
    return result;
}

int get_filename_fd(char *name)
{
    int i;
    char name_to_find[512];
    sprintf(name_to_find, "%s/%s", base_dir, name);
    printf("filepath & name is %s\n", name_to_find);
    for(i = 0; i < MAX_FILES; i++) {
        printf("epoll_files %d is %s\n", i, epoll_files[i]);
        if (epoll_files[i] == NULL)//it means filename in epoll_files[i] is NULL
            continue;

        if (!strcmp(epoll_files[i], name_to_find)) {
            printf("find this file fd in the epoll_files ^-^\n");
            return i;
        }
    }
    printf("Can not find this file fd in the epoll_files...\n");
    return -1;
}

int rm_from_epoll(int fd, int epollFd)
{
    int result;
    //result = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &eventItem);
    result = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, NULL);
    if (result) {
        perror("rm_from_epoll:someting is wrong->");
        return result;
    }

    printf("Successfully rm this file fd in the epoll_files...\n");
    return result;
}

int read_process_inotify_fd(int inotify_fd, int mEpollFd)
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
                char *name = malloc(512);//remeber to free it once not use any more
                sprintf(name, "%s/%s", base_dir, event->name);
                printf("filepath & name is %s\n", name);
                int tmpFd = open(name, O_RDWR);//rememer to close it once not use any more
                add_to_epoll(name, tmpFd, mEpollFd);
                epoll_files[tmpFd] = name;
            } else {
                printf("delete file : %s\n", event->name);
                //ALOGI("Removing device '%s' due to inotify event\n", devname);
                //closeDeviceByPathLocked(devname);
                /*for each file:
                 * del it from epoll: epoll_ctl(... EPOLL_CTL_DEL ...)
                 * event->name only include file_name, we need file path
                 * close it
                 */
                int tmpFd = get_filename_fd(event->name);
                if (tmpFd < 0)
                    printf("no such file!\n");
                else {
                    rm_from_epoll(tmpFd, mEpollFd);
                    free(epoll_files[tmpFd]);
                    epoll_files[tmpFd] = NULL;
                    close(tmpFd);
                }
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
    if (argc != 2) {
        printf("Usage: %s <base dir>\n", argv[0]);
        return -1;
    }
    if (!strcmp(argv[1], '/')) {
        printf("Please not add '/' after base dir..."\n);
        return -1;
    }

    base_dir = argv[1];

    char buf[DATA_MAX_LEN];
    int i;
    int result;

    /*inotify_init*/
    int mINotifyFd = inotify_init();

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

    int mEpollFd = epoll_create(8);

    //how to add and track file especially new file?
    //actually we can track the base dir fd
    add_to_epoll(NULL, mINotifyFd, mEpollFd);

    /*read*/
    while(1) {
        int pollResult = epoll_wait(mEpollFd, mPendingEventItems,
                EPOLL_MAX_EVENTS, -1);
        for(i = 0; i < pollResult; i++) {
            if (mPendingEventItems[i].data.fd == mINotifyFd)//表明是监测的目录发生变化
                read_process_inotify_fd(mINotifyFd, mEpollFd);
            else {//表明是监测的目录下的文件发生变化, only for FIFO
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
