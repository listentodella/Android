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

void main(void)
{
    /*add Service*/

    /*while(1){read data, parse data, call service_func}*/
    
    /*open driver, mmap*/
    sp<ProcessState> proc(ProcessState::self());

    /*获得BpServiceManager*/
    sp<IServiceManager> sm = defaultServiceManager();
    //ALOGI("ServiceManager: %p", sm.get());

    sm->addService(String16("hello"), new BnHelloService());

    /*循环体*/
    ProcessState::self()->startThreadPool();
    IPCThreadState::self()->joinThreadPool();
}
