/**
 * 提供 “hello” 服务
 * 函数：
 *  void sayhello(void)
 *  int sayhello_to(char *name)
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <errno.h>

 #include "binder.h"

 ##include "test_server.h"

 uint32_t svcmgr_lookup(struct binder_state *bs, uint32_t target, const char *name)
 {
     uint32_t handle;
     unsigned iodata[512/4];
     struct binder_io msg, reply;

     bio_init(&msg, iodata, sizeof(iodata), 4);
     bio_put_uint32(&msg, 0);  // strict mode header
     bio_put_string16_x(&msg, SVC_MGR_NAME);
     bio_put_string16_x(&msg, name);

     if (binder_call(bs, &msg, &reply, target, SVC_MGR_CHECK_SERVICE))
         return 0;

     handle = bio_get_ref(&reply);

     if (handle)
         binder_acquire(bs, handle);

     binder_done(bs, &msg, &reply);

     return handle;
 }

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

     if (binder_call(bs, &msg, &reply, target, SVC_MGR_ADD_SERVICE))
         return -1;

     status = bio_get_uint32(&reply);

     binder_done(bs, &msg, &reply);

     return status;
 }

 unsigned token;

void sayhello(void)
{
  static int cnt = 0;
  fprintf(stderr, "say hello : %d\n", count++);
}

int sayhello_to(char *name)
{
  static int cnt = 0;
  fprintf(stderr, "say hello to %s : %d\n", name, count++);
  return cnt;
}

 int hello_service_handler(struct binder_state *bs,
                    struct binder_transaction_data *txn,
                    struct binder_io *msg,
                    struct binder_io *reply)
 {
   /**
    * 根据txn->code知道要调用哪一个函数
    * 如果需要参数，可以从msg取出
    * 如果要返回结果，可以把结果放入reply
    */
     /**
      * say hello
      * sayhello_to
      * @param svcmgr_handle [description]
      */
     uint16_t *s;
     char name[512];
     size_t len;
     uint32_t handle;
     uint32_t strict_policy;
     int i;


     //ALOGI("target=%x code=%d pid=%d uid=%d\n",
     //  txn->target.handle, txn->code, txn->sender_pid, txn->sender_euid);

     if (txn->target.handle != svcmgr_handle)
         return -1;

     if (txn->code == PING_TRANSACTION)
         return 0;

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
       /*从 msg 里取出字符串*/
       s = bio_get_string16(msg, &len);
       if (s == NULL) {
           return -1;
       }
       for(i = 0; i < len; i++)
          name[i] = s[i];
        name[i] = '\0';
        /*处理*/
        i = sayhello_to(name);
        /*把结果放入reply*/
        bio_put_uint32(reply, i);
        break;

     default:
         ALOGE("unknown code %d\n", txn->code);
         return -1;
     }

     bio_put_uint32(reply, 0);
     return 0;
 }


 int main(int argc, char **argv)
 {
     int fd;
     struct binder_state *bs;
     uint32_t svcmgr = BINDER_SERVICE_MANAGER;
     uint32_t handle;
     int ret;
     bs = binder_open(128*1024);
     if (!bs) {
         fprintf(stderr, "failed to open binder driver\n");
         return -1;
     }

     /*add service*/
     ret = svcmgr_publish(bs, svcmgr, "hello", 123);
     if(!ret) {
       fprintf(stderr, "failed to publish hello service\n");
       return -1;
     }
     svcmgr_publish(bs, svcmgr, "goodbye", 124);//如果也是123便会失败
     if(!ret) {
       fprintf(stderr, "failed to publish hello service\n");
       return -1;
     }

#if 0
     /*add service*/
     while(1) {
       /*read data*/
       /*parse data, and process*/
       /*reply*/
     }
#endif
      binder_loop(bs, hello_service_handler);
     return 0;
 }
