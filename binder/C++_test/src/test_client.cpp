/*consult frameworks/av/media/medaserver/Main_mediaserver.cpp */

#define LOG_TAG "HelloService"
//#define LOG_NDEBUG 0

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <utils/Log.h>

using namespace android;


/* ./test_client sayhello
 *  ./test_client hello <name>
 *
 * */
int main(int argc, char **argv)
{
    int cnt = 0;

    if (argc < 2) {
        ALOGI("usage:\n");
        ALOGI("%s hello or goodbye\n", argv[0]);
        ALOGI("%s hello or goodbye <name>\n", argv[0]);
        return -1;
    }
    
    /*getService*/
    /*open driver, mmap*/
    sp<ProcessState> proc(ProcessState::self());

    /*获得BpServiceManager*/
    sp<IServiceManager> sm = defaultServiceManager();

    sp<IBinder> binder = sm->getService(String16("hello"));
    if (binder == 0) {
        ALOGI("cannot get hello service");
        return -1;
    }
    sp<IHelloService> service = interface_cast<IHelloService>(binder);
    
    /*调用Service的函数*/
    if (argc < 2) {
        service->sayhello();
        ALOGI("client call sayhello");
    } else { 
        cnt = service->sayhello_to(argv[2]);
        ALOGI("client call sayhello_to, cnt = %d", cnt);
    }

    return 0;
}
