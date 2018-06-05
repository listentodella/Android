#ifndef ANDROID_BPPERMISSIONCONTROLLER_H
#define ANDROID_BPPERMISSIONCONTROLLER_H


#include "typedef8.h"
#include "IInterface.h"

class IBinder;
class IPermissionController;

namespace android {

// ----------------------------------------------------------------------
class BpPermissionController : public typedef8 {
  public:
    inline BpPermissionController(const sp<IBinder> & impl) : BpInterface<IPermissionController>(impl)
        {
        };

    inline virtual bool checkPermission(const String16 & permission, int32_t pid, int32_t uid) {
            Parcel data, reply;
            data.writeInterfaceToken(IPermissionController::getInterfaceDescriptor());
            data.writeString16(permission);
            data.writeInt32(pid);
            data.writeInt32(uid);
            remote()->transact(CHECK_PERMISSION_TRANSACTION, data, &reply);
            // fail on exception
            if (reply.readExceptionCode() != 0) return 0;
            return reply.readInt32() != 0;
        };

};

} // namespace android
#endif
