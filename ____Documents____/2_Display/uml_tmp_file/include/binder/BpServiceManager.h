#ifndef ANDROID_BPSERVICEMANAGER_H
#define ANDROID_BPSERVICEMANAGER_H


#include "typedef9.h"
#include "IInterface.h"

class IBinder;
class IServiceManager;

namespace android {

// ----------------------------------------------------------------------
class BpServiceManager : public typedef9 {
  public:
    inline BpServiceManager(const sp<IBinder> & impl) : BpInterface<IServiceManager>(impl)
        {
        };

    inline virtual sp<IBinder> getService(const String16 & name) const {
            unsigned n;
            for (n = 0; n < 5; n++){
                sp<IBinder> svc = checkService(name);
                if (svc != NULL) return svc;
                ALOGI("Waiting for service %s...\n", String8(name).string());
                sleep(1);
            }
            return NULL;
        };

    inline virtual sp<IBinder> checkService(const String16 & name) const {
            Parcel data, reply;
            data.writeInterfaceToken(IServiceManager::getInterfaceDescriptor());
            data.writeString16(name);
            remote()->transact(CHECK_SERVICE_TRANSACTION, data, &reply);
            return reply.readStrongBinder();
        };

    inline virtual status_t addService(const String16 & name, const sp<IBinder> & service, bool allowIsolated) {
            Parcel data, reply;
            data.writeInterfaceToken(IServiceManager::getInterfaceDescriptor());
            data.writeString16(name);
            data.writeStrongBinder(service);
            data.writeInt32(allowIsolated ? 1 : 0);
            status_t err = remote()->transact(ADD_SERVICE_TRANSACTION, data, &reply);
            return err == NO_ERROR ? reply.readExceptionCode() : err;
        };

    inline virtual Vector<String16> listServices() {
            Vector<String16> res;
            int n = 0;
    
            for (;;) {
                Parcel data, reply;
                data.writeInterfaceToken(IServiceManager::getInterfaceDescriptor());
                data.writeInt32(n++);
                status_t err = remote()->transact(LIST_SERVICES_TRANSACTION, data, &reply);
                if (err != NO_ERROR)
                    break;
                res.add(reply.readString16());
            }
            return res;
        };

};

} // namespace android
#endif
