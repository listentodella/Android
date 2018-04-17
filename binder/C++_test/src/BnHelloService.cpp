/*consult: frameworks/av/media/libmedia/IMediaPlayerService.cpp*/

#include "IHelloService.h"

#define LOG_TAG "HelloService"

namespace android{
status_t BnInterface::onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0)
{
    /*解析数据,调用sayhello or sayhello_to*/

}


void BnInterface::sayhello(void)
{
    static int cnt = 0;
    ALOGI("%s: %d\n", __func__, cnt++);
}

int BnInterface::sayhello_to(const char *name)
{
    static int cnt = 0;
    ALOGI("%s: %s : %d\n", __func__, name, cnt++);
    return cnt;
}

}
