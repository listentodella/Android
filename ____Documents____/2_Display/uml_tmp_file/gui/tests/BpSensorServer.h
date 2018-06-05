#ifndef ANDROID_BPSENSORSERVER_H
#define ANDROID_BPSENSORSERVER_H


#include "typedef14.h"

namespace android {

class BpSensorServer : public typedef14 {
  public:
    inline BpSensorServer(const sp<IBinder> & impl) : BpInterface<ISensorServer>(impl)
        {
        };

    inline virtual Vector<Sensor> getSensorList() {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorServer::getInterfaceDescriptor());
            remote()->transact(GET_SENSOR_LIST, data, &reply);
            Sensor s;
            Vector<Sensor> v;
            int32_t n = reply.readInt32();
            v.setCapacity(n);
            while (n--) {
                reply.read(s);
                v.add(s);
            }
            return v;
        };

    inline virtual sp<ISensorEventConnection> createSensorEventConnection() {
            Parcel data, reply;
            data.writeInterfaceToken(ISensorServer::getInterfaceDescriptor());
            remote()->transact(CREATE_SENSOR_EVENT_CONNECTION, data, &reply);
            return interface_cast<ISensorEventConnection>(reply.readStrongBinder());
        };

};

} // namespace android
#endif
