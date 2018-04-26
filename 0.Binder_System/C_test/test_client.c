/* Copyright 2008 The Android Open Source Project
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/types.h>
#include <stdbool.h>
#include <string.h>
#include <private/android_filesystem_config.h>
#include "binder.h"
#include "test_server.h"


//get service
uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
{
    uint32_t handle;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);

    /* 
     * msg:含有服务的名字
     * reply:它会含有servicemanager回复的数据
     * target -> get which service
     * SVC_MGR_CHECK_SERVICE:表示要调用servicemanager中的"getservice函数" 
     * */
    if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
        return 0;

    handle = bio_get_ref(&reply);

    if (handle)
        binder_acquire(bs, handle);

    binder_done(bs, &msg, &reply);

    return handle;
}
//register service
int svcmgr_publish(struct binder_state *bs, uint32_t target, const char *name, void *ptr)
{
    int status;
    unsigned iodata[512/4];
    struct binder_io msg, reply;

    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header
    bio_put_string16_x(&msg, SVC_MGR_NAME);
    bio_put_string16_x(&msg, name);
    bio_put_obj(&msg, ptr);
    /* 
     * msg:含有服务的名字
     * reply:它会含有servicemanager回复的数据
     * 0 -> servicemanager
     * SVC_MGR_ADD_SERVICE:表示要调用servicemanager中的"addservice函数" 
     * */
    if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
        return -1;

    status = bio_get_uint32(&reply);

    binder_done(bs, &msg, &reply);

    return status;
}

struct binder_state *g_bs;
uint32_t g_handle_hello;
uint32_t g_handle_goodbye;

void sayhello(void)
{
    /*构造binder_io*/
    unsigned iodata[512/4];
    struct binder_io msg, reply;
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header,必须先放入0
    /*放入参数*/

    /*调用binder_call*/
    if (binder_call(g_bs, &msg, &reply, g_handle_hello, HELLO_SVR_CMD_SAYHELLO))
        return;
    /*从reply中解析出返回值*/


    binder_done(g_bs, &msg, &reply);
}

int sayhello_to(char *name)
{
    int ret;
    /*构造binder_io*/
    unsigned iodata[512/4];
    struct binder_io msg, reply;
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header,必须先放入0
    /*放入参数*/
    bio_put_string16_x(&msg, name);
    /*调用binder_call*/
    if (binder_call(g_bs, &msg, &reply, g_handle_hello, HELLO_SVR_CMD_SAYHELLO_TO))
        return 0;

    /*从reply中解析出返回值*/
    ret = bio_get_uint32(&reply); 

    binder_done(g_bs, &msg, &reply);

    return ret;
}


void saygoodbye(void)
{
    /*构造binder_io*/
    unsigned iodata[512/4];
    struct binder_io msg, reply;
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header,必须先放入0
    /*放入参数*/

    /*调用binder_call*/
    if (binder_call(g_bs, &msg, &reply, g_handle_goodbye, GOODBYE_SVR_CMD_SAYGOODBYE))
        return;
    /*从reply中解析出返回值*/


    binder_done(g_bs, &msg, &reply);
}

int saygoodbye_to(char *name)
{
    int ret;
    /*构造binder_io*/
    unsigned iodata[512/4];
    struct binder_io msg, reply;
    bio_init(&msg, iodata, sizeof(iodata), 4);
    bio_put_uint32(&msg, 0);  // strict mode header,必须先放入0
    /*放入参数*/
    bio_put_string16_x(&msg, name);
    /*调用binder_call*/
    if (binder_call(g_bs, &msg, &reply, g_handle_goodbye, GOODBYE_SVR_CMD_SAYGOODBYE_TO))
        return 0;

    /*从reply中解析出返回值*/
    ret = bio_get_uint32(&reply); 

    binder_done(g_bs, &msg, &reply);

    return ret;
}

/* ./test_client sayhello
 *  ./test_client hello <name>
 *
 * */

int main(int argc, char **argv)
{
    int fd, ret;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;
    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "%s hello or goodbye\n", argv[0]);
        fprintf(stderr, "%s hello or goodbye <name>\n", argv[0]);
        return -1;
    }
    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    g_bs = bs;


    /* get  service 
     * the order of service effects the handle_value of service
     * */
    handle = svcmgr_lookup(bs, svcmgr, "hello");
    if(!handle){
        fprintf(stderr, "failed to get hello service\n");
        return -1;
    }
    g_handle_hello = handle;
    fprintf(stderr, "Handle for hello service = %d\n", g_handle_hello);

    handle = svcmgr_lookup(bs, svcmgr, "goodbye");
    if(!handle){
        fprintf(stderr, "failed to get hello service\n");
        return -1;
    }
    g_handle_goodbye = handle;
    fprintf(stderr, "Handle for goodbye service = %d\n", g_handle_goodbye);

    /* send data to server */
    if (!strcmp(argv[1], "hello")) {
        if (argc == 2) {
            sayhello();
        } else if (argc == 3) {
            ret = sayhello_to(argv[2]);
            fprintf(stderr, "get ret of sayhello_to %d\n", ret);
        }
    } else if (!strcmp(argv[1], "goodbye")) {
        if (argc == 2) {
            saygoodbye();
        } else if (argc == 3) {
            ret = saygoodbye_to(argv[2]);
            fprintf(stderr, "get ret of saygoodbye_to %d\n", ret);
        }
    }


    binder_release(bs, handle);


    return 0;
}
