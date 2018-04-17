# server的多线程实现
在binder系统里，但是有一个server进程提供服务，但是可能有一个或多个client同时向它发出请求，那server进程 "忙不过来" 时需要创建多线程。
* binder_proc.todo .wait里有线程在等待，说明忙的过来，反之为忙不过来

## 流程
* 驱动判断是否"忙不过来"
* 驱动向app发请求: 创建新线程
* app创建新线程
```
if (proc->requested_threads + proc->ready_threads == 0 &&
	    proc->requested_threads_started < proc->max_threads &&
	    (thread->looper & (BINDER_LOOPER_STATE_REGISTERED |
	     BINDER_LOOPER_STATE_ENTERED)) /* the user-space code fails to */
     /*spawn a new thread if we leave this out */) {
	proc->requested_threads++;
	binder_debug(BINDER_DEBUG_THREADS,
		     "binder: %d:%d BR_SPAWN_LOOPER\n",
		     proc->pid, thread->pid);
    
    /*print info: proc's name, proc's id, thread, cmd's name*/
    printk("%s (%d, %d), %s, %s\n", proc->tsk->comm, proc->pid,
            thread->pid, __func__, binder_cmd_name(BR_SPAWN_LOOPER));
	if (put_user(BR_SPAWN_LOOPER, (uint32_t __user *)buffer))
		return -EFAULT;
}
```
### 驱动向app发出"创建新线程"请求的条件
* proc->requested_threads = 0，“未处理的新线程请求”为0，
* proc->ready_threads = 0，空闲线程数为0
* 已启动的线程数＜max_threads（此值可以设置）

### 怎么写app
* 设置max_threads
```
case BINDER_SET_MAX_THREADS:
if (copy_from_user(&proc->max_threads, ubuf, sizeof(proc->max_threads))) {
	ret = -EINVAL;
	goto err;
}
```
* 收到BR_SPAWN_LOOPER后，创建新线程
```
void binder_thread_loop(struct binder_state *bs, binder_handler func)
{
    int res;
    struct binder_write_read bwr;
    uint32_t readbuf[32];

    bwr.write_size = 0;
    bwr.write_consumed = 0;
    bwr.write_buffer = 0;

    readbuf[0] = BC_REGISTER_LOOPER;
    binder_write(bs, readbuf, sizeof(uint32_t));

    for (;;) {
        bwr.read_size = sizeof(readbuf);
        bwr.read_consumed = 0;
        bwr.read_buffer = (uintptr_t) readbuf;

        res = ioctl(bs->fd, BINDER_WRITE_READ, &bwr);

        if (res < 0) {
            ALOGE("binder_loop: ioctl failed (%s)\n", strerror(errno));
            break;
        }
        //func是传过来的函数指针,readbuf里面存放着cmd，此时的cmd可能
        //已经经过ioctl后被改动了
        res = binder_parse(bs, 0, (uintptr_t) readbuf, bwr.read_consumed, func);
        if (res == 0) {
            ALOGE("binder_loop: unexpected reply?!\n");
            break;
        }
        if (res < 0) {
            ALOGE("binder_loop: io error %d %s\n", res, strerror(errno));
            break;
        }
    }
}

//struct binder_state *bs, binder_handler func
struct binder_thread_desc {
    struct binder_state *bs;
    binder_handler func;
};

static void *binder_thread_routine(struct binder_thread_desc *btd)
{
    binder_thread_loop(btd->bs, btd->func);
    return NULL;
}


------
binder_parse():
case BR_SPAWN_LOOPER: {
    /*create new thread*/
    //if(fork() == 0)//WRONG!!!
    pthread_t tid;
    struct binder_thread_desc btd;
    btd.bs = bs;
    btd.func = func;
    pthread_create(&tid, NULL, binder_thread_routine, &btd);
    /*in new thread: ioctl(BC_ENTER_LOOPER), enter binder_looper*/
    break;
}
```
* 新线程发出ioctl：BC_REGISTER_LOOPER
* 像主线程一样，进入一个循环 {read driver,处理}