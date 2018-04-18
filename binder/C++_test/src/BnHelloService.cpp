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
    switch (code) {
        case HELLO_SVR_CMD_SAYHELLO: {
            sayhello();
            return NO_ERROR;
        } break;

        case HELLO_SVR_CMD_SAYHELLO_TO: {
            /*从data中取出参数*/
            int32_t policy = data.readInt32();
            String16 name16 = data.readString16();
            String8 name8(name16);

            int cnt = sayhello_to(name8.string());

            /*把返回值写入reply传出去*/
            reply->writeInt32(cnt);
            return NO_ERROR;
        } break;
        
        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
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
