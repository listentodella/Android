#ifndef ANDROID_BPPRODUCERLISTENER_H
#define ANDROID_BPPRODUCERLISTENER_H


#include "typedef12.h"

namespace android {

class BpProducerListener : public typedef12 {
  public:
    inline BpProducerListener(const sp<IBinder> & impl) : BpInterface<IProducerListener>(impl) {};

    inline virtual void onBufferReleased() {
            Parcel data, reply;
            data.writeInterfaceToken(IProducerListener::getInterfaceDescriptor());
            remote()->transact(ON_BUFFER_RELEASED, data, &reply, IBinder::FLAG_ONEWAY);
        };

};

} // namespace android
#endif
