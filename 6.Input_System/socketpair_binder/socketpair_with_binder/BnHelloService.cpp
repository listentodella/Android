/*consult: frameworks/av/media/libmedia/IMediaPlayerService.cpp*/

#define LOG_TAG "HelloService"

#include "IHelloService.h"


namespace android {

BnHelloService::BnHelloService()
{

}


BnHelloService::BnHelloService(int fd)
{
    this->fd = fd;
}


status_t BnHelloService::onTransact( uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags)
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
            //first string not be needed
            String16 name16_tmp = data.readString16();
            String16 name16 = data.readString16();
            String8 name8(name16);

            int cnt = sayhello_to(name8.string());

            /*把返回值写入reply传出去*/
            reply->writeInt32(cnt);
            return NO_ERROR;
        } break;
        
        case HELLO_SVR_CMD_GET_FD: {
            int fd = this->get_fd();
            reply->writeInt32(0);
            //framworks/base/core/jni/android_view_InputChannel.cpp
            //android_view_InputChannel_nativeWriteToParcel()
            
            //reply->writeFileDescriptor(fd, 1);
            reply->writeDupFileDescriptor(fd);
            return NO_ERROR;
        } break;


        default:
            return BBinder::onTransact(code, data, reply, flags);
    }
}

void BnHelloService::sayhello(void)
{
    static int cnt = 0;
    ALOGI("%s: %d\n", __func__, cnt++);
}

int BnHelloService::sayhello_to(const char *name)
{
    static int cnt = 0;
    ALOGI("%s: %s : %d\n", __func__, name, cnt++);
    return cnt;
}

int BnHelloService::get_fd(void)
{
    return fd;
}


}
