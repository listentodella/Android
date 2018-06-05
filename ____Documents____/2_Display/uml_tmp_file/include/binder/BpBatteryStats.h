#ifndef ANDROID_BPBATTERYSTATS_H
#define ANDROID_BPBATTERYSTATS_H


#include "typedef4.h"
#include "IInterface.h"

class IBinder;
class IBatteryStats;

namespace android {

// ----------------------------------------------------------------------
class BpBatteryStats : public typedef4 {
  public:
    inline BpBatteryStats(const sp<IBinder> & impl) : BpInterface<IBatteryStats>(impl)
        {
        };

    inline virtual void noteStartSensor(int uid, int sensor) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            data.writeInt32(sensor);
            remote()->transact(NOTE_START_SENSOR_TRANSACTION, data, &reply);
        };

    inline virtual void noteStopSensor(int uid, int sensor) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            data.writeInt32(sensor);
            remote()->transact(NOTE_STOP_SENSOR_TRANSACTION, data, &reply);
        };

    inline virtual void noteStartVideo(int uid) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            remote()->transact(NOTE_START_VIDEO_TRANSACTION, data, &reply);
        };

    inline virtual void noteStopVideo(int uid) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            remote()->transact(NOTE_STOP_VIDEO_TRANSACTION, data, &reply);
        };

    inline virtual void noteStartAudio(int uid) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            remote()->transact(NOTE_START_AUDIO_TRANSACTION, data, &reply);
        };

    inline virtual void noteStopAudio(int uid) {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            data.writeInt32(uid);
            remote()->transact(NOTE_STOP_AUDIO_TRANSACTION, data, &reply);
        };

    inline virtual void noteResetVideo() {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            remote()->transact(NOTE_RESET_VIDEO_TRANSACTION, data, &reply);
        };

    inline virtual void noteResetAudio() {
            Parcel data, reply;
            data.writeInterfaceToken(IBatteryStats::getInterfaceDescriptor());
            remote()->transact(NOTE_RESET_AUDIO_TRANSACTION, data, &reply);
        };

};

} // namespace android
#endif
