#ifndef ANDROID_BPSURFACECOMPOSER_H
#define ANDROID_BPSURFACECOMPOSER_H


#include "typedef15.h"

namespace android {

class BpSurfaceComposer : public typedef15 {
  public:
    inline BpSurfaceComposer(const sp<IBinder> & impl) : BpInterface<ISurfaceComposer>(impl)
        {
        };

    inline virtual sp<ISurfaceComposerClient> createConnection() {
            uint32_t n;
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            remote()->transact(BnSurfaceComposer::CREATE_CONNECTION, data, &reply);
            return interface_cast<ISurfaceComposerClient>(reply.readStrongBinder());
        };

    inline virtual sp<IGraphicBufferAlloc> createGraphicBufferAlloc() {
            uint32_t n;
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            remote()->transact(BnSurfaceComposer::CREATE_GRAPHIC_BUFFER_ALLOC, data, &reply);
            return interface_cast<IGraphicBufferAlloc>(reply.readStrongBinder());
        };

    inline virtual void setTransactionState(const Vector<ComposerState> & state, const Vector<DisplayState> & displays, uint32_t flags) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            {
                Vector<ComposerState>::const_iterator b(state.begin());
                Vector<ComposerState>::const_iterator e(state.end());
                data.writeInt32(state.size());
                for ( ; b != e ; ++b ) {
                    b->write(data);
                }
            }
            {
                Vector<DisplayState>::const_iterator b(displays.begin());
                Vector<DisplayState>::const_iterator e(displays.end());
                data.writeInt32(displays.size());
                for ( ; b != e ; ++b ) {
                    b->write(data);
                }
            }
            data.writeInt32(flags);
            remote()->transact(BnSurfaceComposer::SET_TRANSACTION_STATE, data, &reply);
        };

    inline virtual void bootFinished() {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            remote()->transact(BnSurfaceComposer::BOOT_FINISHED, data, &reply);
        };

    inline virtual status_t captureScreen(const sp<IBinder> & display, const sp<IGraphicBufferProducer> & producer, Rect sourceCrop, uint32_t reqWidth, uint32_t reqHeight, uint32_t minLayerZ, uint32_t maxLayerZ, bool useIdentityTransform, ISurfaceComposer::Rotation rotation) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            data.writeStrongBinder(producer->asBinder());
            data.write(sourceCrop);
            data.writeInt32(reqWidth);
            data.writeInt32(reqHeight);
            data.writeInt32(minLayerZ);
            data.writeInt32(maxLayerZ);
            data.writeInt32(static_cast<int32_t>(useIdentityTransform));
            data.writeInt32(static_cast<int32_t>(rotation));
            remote()->transact(BnSurfaceComposer::CAPTURE_SCREEN, data, &reply);
            return reply.readInt32();
        };

    inline virtual bool authenticateSurfaceTexture(const sp<IGraphicBufferProducer> & bufferProducer) const {
            Parcel data, reply;
            int err = NO_ERROR;
            err = data.writeInterfaceToken(
                    ISurfaceComposer::getInterfaceDescriptor());
            if (err != NO_ERROR) {
                ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error writing "
                        "interface descriptor: %s (%d)", strerror(-err), -err);
                return false;
            }
            err = data.writeStrongBinder(bufferProducer->asBinder());
            if (err != NO_ERROR) {
                ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error writing "
                        "strong binder to parcel: %s (%d)", strerror(-err), -err);
                return false;
            }
            err = remote()->transact(BnSurfaceComposer::AUTHENTICATE_SURFACE, data,
                    &reply);
            if (err != NO_ERROR) {
                ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error "
                        "performing transaction: %s (%d)", strerror(-err), -err);
                return false;
            }
            int32_t result = 0;
            err = reply.readInt32(&result);
            if (err != NO_ERROR) {
                ALOGE("ISurfaceComposer::authenticateSurfaceTexture: error "
                        "retrieving result: %s (%d)", strerror(-err), -err);
                return false;
            }
            return result != 0;
        };

    inline virtual sp<IDisplayEventConnection> createDisplayEventConnection() {
            Parcel data, reply;
            sp<IDisplayEventConnection> result;
            int err = data.writeInterfaceToken(
                    ISurfaceComposer::getInterfaceDescriptor());
            if (err != NO_ERROR) {
                return result;
            }
            err = remote()->transact(
                    BnSurfaceComposer::CREATE_DISPLAY_EVENT_CONNECTION,
                    data, &reply);
            if (err != NO_ERROR) {
                ALOGE("ISurfaceComposer::createDisplayEventConnection: error performing "
                        "transaction: %s (%d)", strerror(-err), -err);
                return result;
            }
            result = interface_cast<IDisplayEventConnection>(reply.readStrongBinder());
            return result;
        };

    inline virtual sp<IBinder> createDisplay(const String8 & displayName, bool secure) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeString8(displayName);
            data.writeInt32(secure ? 1 : 0);
            remote()->transact(BnSurfaceComposer::CREATE_DISPLAY, data, &reply);
            return reply.readStrongBinder();
        };

    inline virtual void destroyDisplay(const sp<IBinder> & display) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            remote()->transact(BnSurfaceComposer::DESTROY_DISPLAY, data, &reply);
        };

    inline virtual sp<IBinder> getBuiltInDisplay(int32_t id) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeInt32(id);
            remote()->transact(BnSurfaceComposer::GET_BUILT_IN_DISPLAY, data, &reply);
            return reply.readStrongBinder();
        };

    inline virtual void setPowerMode(const sp<IBinder> & display, int mode) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            data.writeInt32(mode);
            remote()->transact(BnSurfaceComposer::SET_POWER_MODE, data, &reply);
        };

    inline virtual status_t getDisplayConfigs(const sp<IBinder> & display, Vector<DisplayInfo> * configs) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            remote()->transact(BnSurfaceComposer::GET_DISPLAY_CONFIGS, data, &reply);
            status_t result = reply.readInt32();
            if (result == NO_ERROR) {
                size_t numConfigs = static_cast<size_t>(reply.readInt32());
                configs->clear();
                configs->resize(numConfigs);
                for (size_t c = 0; c < numConfigs; ++c) {
                    memcpy(&(configs->editItemAt(c)),
                            reply.readInplace(sizeof(DisplayInfo)),
                            sizeof(DisplayInfo));
                }
            }
            return result;
        };

    inline virtual status_t getDisplayStats(const sp<IBinder> & display, DisplayStatInfo * stats) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            remote()->transact(BnSurfaceComposer::GET_DISPLAY_STATS, data, &reply);
            status_t result = reply.readInt32();
            if (result == NO_ERROR) {
                memcpy(stats,
                        reply.readInplace(sizeof(DisplayStatInfo)),
                        sizeof(DisplayStatInfo));
            }
            return result;
        };

    inline virtual int getActiveConfig(const sp<IBinder> & display) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            remote()->transact(BnSurfaceComposer::GET_ACTIVE_CONFIG, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t setActiveConfig(const sp<IBinder> & display, int id) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            data.writeStrongBinder(display);
            data.writeInt32(id);
            remote()->transact(BnSurfaceComposer::SET_ACTIVE_CONFIG, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t clearAnimationFrameStats() {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            remote()->transact(BnSurfaceComposer::CLEAR_ANIMATION_FRAME_STATS, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t getAnimationFrameStats(FrameStats * outStats) const {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposer::getInterfaceDescriptor());
            remote()->transact(BnSurfaceComposer::GET_ANIMATION_FRAME_STATS, data, &reply);
            reply.read(*outStats);
            return reply.readInt32();
        };

};

} // namespace android
#endif
