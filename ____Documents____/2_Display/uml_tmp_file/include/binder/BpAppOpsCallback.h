#ifndef ANDROID_BPAPPOPSCALLBACK_H
#define ANDROID_BPAPPOPSCALLBACK_H


#include "typedef2.h"
#include "IInterface.h"

class IBinder;
class IAppOpsCallback;

namespace android {

// ----------------------------------------------------------------------
class BpAppOpsCallback : public typedef2 {
  public:
    inline BpAppOpsCallback(const sp<IBinder> & impl) : BpInterface<IAppOpsCallback>(impl)
        {
        };

    inline virtual void opChanged(int32_t op, const String16 & packageName) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsCallback::getInterfaceDescriptor());
            data.writeInt32(op);
            data.writeString16(packageName);
            remote()->transact(OP_CHANGED_TRANSACTION, data, &reply);
        };

};

} // namespace android
#endif
