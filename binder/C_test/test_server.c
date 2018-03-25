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


void sayhello(void)
{
    static int cnt = 0;
    fprintf(stderr, "%s: %d\n", __func__, cnt++);
}


int sayhello_to(char *name)
{
    static int cnt = 0;
    fprintf(stderr, "%s: %s : %d\n", __func__, name, cnt++);
    return cnt;
}

int hello_service_handler(struct binder_state *bs,
                   struct binder_transaction_data *txn,
                   struct binder_io *msg,
                   struct binder_io *reply)
{
    /*
     *根据txn->code知道要调用哪一个函数
     * 如果需要参数，可以从msg取出
     * 如果要返回结果，可以把结果放入reply
     * */
    /* 
     * sayhello
     * sayhello_to */

    uint16_t *s;
    char name[512];
    size_t len;
    uint32_t handle;
    uint32_t strict_policy;
    int i;

    // Equivalent to Parcel::enforceInterface(), reading the RPC
    // header with the strict mode policy mask and the interface name.
    // Note that we ignore the strict_policy and don't propagate it
    // further (since we do no outbound RPCs anyway).
    strict_policy = bio_get_uint32(msg);

    switch(txn->code) {
    case HELLO_SVR_CMD_SAYHELLO:
        sayhello();
        return 0;

    case HELLO_SVR_CMD_SAYHELLO_TO:
        /*从 msg 里取出字符串  */
        s = bio_get_string16(msg, &len);
        if (s == NULL) {
            return -1;
        }

        for(i = 0; i < len; i++)
            name[i] = s[i];
        name[i] = '\0';

        /*处理  */
        i = sayhello_to(name);
        /*把结果放入reply*/
        bio_put_string16(reply, i);
        break;

    default:
        fprintf(stderr, "unknown code %d\n", txn->code);
        return -1;
    }


    return 0;
}



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

//unsigned token;

int main(int argc, char **argv)
{
    int fd, ret;
    struct binder_state *bs;
    uint32_t svcmgr = BINDER_SERVICE_MANAGER;
    uint32_t handle;

    bs = binder_open(128*1024);
    if (!bs) {
        fprintf(stderr, "failed to open binder driver\n");
        return -1;
    }

    /* add service */
    ret = svcmgr_publish(bs, svcmgr, "hello", (void *)123);
    if (ret) {
        fprintf(stderr, "failed to publish hello service\n");
        return -1;
    }

    ret = svcmgr_publish(bs, svcmgr, "goodbye", (void *)123);
    //ret = svcmgr_publish(bs, svcmgr, "goodbye", (void *)124);
    if (ret) {
        fprintf(stderr, "failed to publish hello service\n");
        //return -1;
    }




#if 0
    while(1) {
        /*read data*/

        /*parse data, and process*/

        /*reply*/
    }
#endif

    binder_loop(bs, hello_service_handler);




    return 0;
}
