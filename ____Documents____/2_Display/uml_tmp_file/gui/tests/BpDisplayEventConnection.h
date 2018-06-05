#ifndef ANDROID_BPDISPLAYEVENTCONNECTION_H
#define ANDROID_BPDISPLAYEVENTCONNECTION_H


#include "typedef8.h"

namespace android {

class BpDisplayEventConnection : public typedef8 {
  public:
    inline BpDisplayEventConnection(const sp<IBinder> & impl) : BpInterface<IDisplayEventConnection>(impl)
        {
        };

    inline virtual sp<BitTube> getDataChannel() const {
            Parcel data, reply;
            data.writeInterfaceToken(IDisplayEventConnection::getInterfaceDescriptor());
            remote()->transact(GET_DATA_CHANNEL, data, &reply);
            return new BitTube(reply);
        };

    inline virtual void setVsyncRate(uint32_t count) {
            Parcel data, reply;
            data.writeInterfaceToken(IDisplayEventConnection::getInterfaceDescriptor());
            data.writeInt32(count);
            remote()->transact(SET_VSYNC_RATE, data, &reply);
        };

    inline virtual void requestNextVsync() {
            Parcel data, reply;
            data.writeInterfaceToken(IDisplayEventConnection::getInterfaceDescriptor());
            remote()->transact(REQUEST_NEXT_VSYNC, data, &reply, IBinder::FLAG_ONEWAY);
        };

};

} // namespace android
#endif
