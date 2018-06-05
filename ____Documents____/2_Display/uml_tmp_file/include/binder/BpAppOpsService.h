#ifndef ANDROID_BPAPPOPSSERVICE_H
#define ANDROID_BPAPPOPSSERVICE_H


#include "typedef3.h"
#include "IInterface.h"

class IBinder;
class IAppOpsService;
class IAppOpsCallback;

namespace android {

// ----------------------------------------------------------------------
class BpAppOpsService : public typedef3 {
  public:
    inline BpAppOpsService(const sp<IBinder> & impl) : BpInterface<IAppOpsService>(impl)
        {
        };

    inline virtual int32_t checkOperation(int32_t code, int32_t uid, const String16 & packageName) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeInt32(code);
            data.writeInt32(uid);
            data.writeString16(packageName);
            remote()->transact(CHECK_OPERATION_TRANSACTION, data, &reply);
            // fail on exception
            if (reply.readExceptionCode() != 0) return MODE_ERRORED;
            return reply.readInt32();
        };

    inline virtual int32_t noteOperation(int32_t code, int32_t uid, const String16 & packageName) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeInt32(code);
            data.writeInt32(uid);
            data.writeString16(packageName);
            remote()->transact(NOTE_OPERATION_TRANSACTION, data, &reply);
            // fail on exception
            if (reply.readExceptionCode() != 0) return MODE_ERRORED;
            return reply.readInt32();
        };

    inline virtual int32_t startOperation(const sp<IBinder> & token, int32_t code, int32_t uid, const String16 & packageName) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeStrongBinder(token);
            data.writeInt32(code);
            data.writeInt32(uid);
            data.writeString16(packageName);
            remote()->transact(START_OPERATION_TRANSACTION, data, &reply);
            // fail on exception
            if (reply.readExceptionCode() != 0) return MODE_ERRORED;
            return reply.readInt32();
        };

    inline virtual void finishOperation(const sp<IBinder> & token, int32_t code, int32_t uid, const String16 & packageName) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeStrongBinder(token);
            data.writeInt32(code);
            data.writeInt32(uid);
            data.writeString16(packageName);
            remote()->transact(FINISH_OPERATION_TRANSACTION, data, &reply);
        };

    inline virtual void startWatchingMode(int32_t op, const String16 & packageName, const sp<IAppOpsCallback> & callback) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeInt32(op);
            data.writeString16(packageName);
            data.writeStrongBinder(callback->asBinder());
            remote()->transact(START_WATCHING_MODE_TRANSACTION, data, &reply);
        };

    inline virtual void stopWatchingMode(const sp<IAppOpsCallback> & callback) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeStrongBinder(callback->asBinder());
            remote()->transact(STOP_WATCHING_MODE_TRANSACTION, data, &reply);
        };

    inline virtual sp<IBinder> getToken(const sp<IBinder> & clientToken) {
            Parcel data, reply;
            data.writeInterfaceToken(IAppOpsService::getInterfaceDescriptor());
            data.writeStrongBinder(clientToken);
            remote()->transact(GET_TOKEN_TRANSACTION, data, &reply);
            // fail on exception
            if (reply.readExceptionCode() != 0) return NULL;
            return reply.readStrongBinder();
        };

};

} // namespace android
#endif
