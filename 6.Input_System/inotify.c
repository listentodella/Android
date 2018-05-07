/*************************************************************************
  > File Name: inotify.c
  > Author: H233
  > Mail: 937138688@qq.com
  > Created Time: 2018年05月07日 星期一 21时50分34秒

  frameworks/native/services/inputflinger/EventHub.cpp


usage: inotify <dir>

 ************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/inotify.h>

int read_process_inotify_fd(int fd)
{
    int res;
    char event_buf[512];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;

    /*read*/
    res = read(fd, event_buf, sizeof(event_buf));
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
            } else {
                printf("delete file : %s\n", event->name);
                //ALOGI("Removing device '%s' due to inotify event\n", devname);
                //closeDeviceByPathLocked(devname);
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
    int mINotifyFd;
    int result;
    if (argc != 2) {
        printf("usage: %s <dir>\n", argv[0]);
        return -1;
    }

    /*inotify_init*/
    mINotifyFd = inotify_init();

    /*add watch*/
    result = inotify_add_watch(mINotifyFd, argv[1], IN_DELETE | IN_CREATE);

    /*read*/
    while(1) {
        read_process_inotify_fd(mINotifyFd);
    }

    return 0;
}
