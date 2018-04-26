/*consult frameworks/av/media/medaserver/Main_mediaserver.cpp */

#define LOG_TAG "TestService"
//#define LOG_NDEBUG 0

#include <fcntl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <cutils/properties.h>
#include <utils/Log.h>

#include "IHelloService.h"
#include "IGoodbyeService.h"

using namespace android;

int main(void)
{
    /*add Service*/

    /*while(1){read data, parse data, call service_func}*/
    
    /*open driver, mmap*/
    sp<ProcessState> proc(ProcessState::self());

    /*获得BpServiceManager*/
    sp<IServiceManager> sm = defaultServiceManager();
    //ALOGI("ServiceManager: %p", sm.get());

	sm->addService(String16("hello"), new BnHelloService());
	sm->addService(String16("goodbye"), new BnGoodbyeService());

    /*循环体*/
    ProcessState::self()->startThreadPool();//创建子线程
    IPCThreadState::self()->joinThreadPool();//主线程

    return 0;
}
