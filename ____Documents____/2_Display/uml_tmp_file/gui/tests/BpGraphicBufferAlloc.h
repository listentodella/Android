#ifndef ANDROID_BPGRAPHICBUFFERALLOC_H
#define ANDROID_BPGRAPHICBUFFERALLOC_H


#include "typedef9.h"

namespace android {

class BpGraphicBufferAlloc : public typedef9 {
  public:
    inline BpGraphicBufferAlloc(const sp<IBinder> & impl) : BpInterface<IGraphicBufferAlloc>(impl)
        {
        };

    inline virtual sp<GraphicBuffer> createGraphicBuffer(uint32_t w, uint32_t h, PixelFormat format, uint32_t usage, status_t * error) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferAlloc::getInterfaceDescriptor());
            data.writeInt32(w);
            data.writeInt32(h);
            data.writeInt32(format);
            data.writeInt32(usage);
            remote()->transact(CREATE_GRAPHIC_BUFFER, data, &reply);
            sp<GraphicBuffer> graphicBuffer;
            status_t result = reply.readInt32();
            if (result == NO_ERROR) {
                graphicBuffer = new GraphicBuffer();
                result = reply.read(*graphicBuffer);
                // reply.readStrongBinder();
                // here we don't even have to read the BufferReference from
                // the parcel, it'll die with the parcel.
            }
            *error = result;
            return graphicBuffer;
        };

};

} // namespace android
#endif
