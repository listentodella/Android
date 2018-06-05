#ifndef ANDROID_BPSENSOREVENTCONNECTION_H
#define ANDROID_BPSENSOREVENTCONNECTION_H


#include "typedef13.h"

namespace android {

class BpSensorEventConnection : public typedef13 {
  public:
    inline BpSensorEventConnection(const sp<IBinder> & impl) : BpInterface<ISensorEventConnection>(impl)
        {
        };

    inline virtual sp<BitTube> getSensorChannel() const {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
            remote()->transact(GET_SENSOR_CHANNEL, data, &reply);
            return new BitTube(reply);
        };

    inline virtual status_t enableDisable(int handle, bool enabled, nsecs_t samplingPeriodNs, nsecs_t maxBatchReportLatencyNs, int reservedFlags) {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
            data.writeInt32(handle);
            data.writeInt32(enabled);
            data.writeInt64(samplingPeriodNs);
            data.writeInt64(maxBatchReportLatencyNs);
            data.writeInt32(reservedFlags);
            remote()->transact(ENABLE_DISABLE, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t setEventRate(int handle, nsecs_t ns) {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
            data.writeInt32(handle);
            data.writeInt64(ns);
            remote()->transact(SET_EVENT_RATE, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t flush() {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorEventConnection::getInterfaceDescriptor());
            remote()->transact(FLUSH_SENSOR, data, &reply);
            return reply.readInt32();
        };

};

} // namespace android
#endif
