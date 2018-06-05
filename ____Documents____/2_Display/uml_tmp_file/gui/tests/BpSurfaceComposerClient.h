#ifndef ANDROID_BPSURFACECOMPOSERCLIENT_H
#define ANDROID_BPSURFACECOMPOSERCLIENT_H


#include "typedef16.h"

namespace android {

class BpSurfaceComposerClient : public typedef16 {
  public:
    inline BpSurfaceComposerClient(const sp<IBinder> & impl) : BpInterface<ISurfaceComposerClient>(impl) {
        };

    inline virtual status_t createSurface(const String8 & name, uint32_t w, uint32_t h, PixelFormat format, uint32_t flags, sp<IBinder> * handle, sp<IGraphicBufferProducer> * gbp) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
            data.writeString8(name);
            data.writeInt32(w);
            data.writeInt32(h);
            data.writeInt32(format);
            data.writeInt32(flags);
            remote()->transact(CREATE_SURFACE, data, &reply);
            *handle = reply.readStrongBinder();
            *gbp = interface_cast<IGraphicBufferProducer>(reply.readStrongBinder());
            return reply.readInt32();
        };

    inline virtual status_t destroySurface(const sp<IBinder> & handle) {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
            data.writeStrongBinder(handle);
            remote()->transact(DESTROY_SURFACE, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t clearLayerFrameStats(const sp<IBinder> & handle) const {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
            data.writeStrongBinder(handle);
            remote()->transact(CLEAR_LAYER_FRAME_STATS, data, &reply);
            return reply.readInt32();
        };

    inline virtual status_t getLayerFrameStats(const sp<IBinder> & handle, FrameStats * outStats) const {
            Parcel data, reply;
            data.writeInterfaceToken(ISurfaceComposerClient::getInterfaceDescriptor());
            data.writeStrongBinder(handle);
            remote()->transact(GET_LAYER_FRAME_STATS, data, &reply);
            reply.read(*outStats);
            return reply.readInt32();
        };

};

} // namespace android
#endif
