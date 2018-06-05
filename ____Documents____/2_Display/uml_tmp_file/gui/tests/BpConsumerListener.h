#ifndef ANDROID_BPCONSUMERLISTENER_H
#define ANDROID_BPCONSUMERLISTENER_H


#include "typedef7.h"

namespace android {

class BpConsumerListener : public typedef7 {
  public:
    inline BpConsumerListener(const sp<IBinder> & impl) : BpInterface<IConsumerListener>(impl) {
        };

    inline virtual void onFrameAvailable() {
            Parcel data, reply;
            data.writeInterfaceToken(IConsumerListener::getInterfaceDescriptor());
            remote()->transact(ON_FRAME_AVAILABLE, data, &reply, IBinder::FLAG_ONEWAY);
        };

    inline virtual void onBuffersReleased() {
            Parcel data, reply;
            data.writeInterfaceToken(IConsumerListener::getInterfaceDescriptor());
            remote()->transact(ON_BUFFER_RELEASED, data, &reply, IBinder::FLAG_ONEWAY);
        };

    inline virtual void onSidebandStreamChanged() {
            Parcel data, reply;
            data.writeInterfaceToken(IConsumerListener::getInterfaceDescriptor());
            remote()->transact(ON_SIDEBAND_STREAM_CHANGED, data, &reply, IBinder::FLAG_ONEWAY);
        };

};

} // namespace android
#endif
