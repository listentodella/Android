#ifndef ANDROID_BPGRAPHICBUFFERPRODUCER_H
#define ANDROID_BPGRAPHICBUFFERPRODUCER_H


#include "typedef11.h"

namespace android {

class BpGraphicBufferProducer : public typedef11 {
  public:
    inline BpGraphicBufferProducer(const sp<IBinder> & impl) : BpInterface<IGraphicBufferProducer>(impl)
        {
        };

    inline virtual status_t requestBuffer(int bufferIdx, sp<GraphicBuffer> * buf) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(bufferIdx);
            status_t result =remote()->transact(REQUEST_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            bool nonNull = reply.readInt32();
            if (nonNull) {
                *buf = new GraphicBuffer();
                result = reply.read(**buf);
                if(result != NO_ERROR) {
                    (*buf).clear();
                    return result;
                }
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t setBufferCount(int bufferCount) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(bufferCount);
            status_t result =remote()->transact(SET_BUFFER_COUNT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t dequeueBuffer(int * buf, sp<Fence> * fence, bool async, uint32_t w, uint32_t h, uint32_t format, uint32_t usage) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(async);
            data.writeInt32(w);
            data.writeInt32(h);
            data.writeInt32(format);
            data.writeInt32(usage);
            status_t result = remote()->transact(DEQUEUE_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            *buf = reply.readInt32();
            bool nonNull = reply.readInt32();
            if (nonNull) {
                *fence = new Fence();
                reply.read(**fence);
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t detachBuffer(int slot) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(slot);
            status_t result = remote()->transact(DETACH_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t detachNextBuffer(sp<GraphicBuffer> * outBuffer, sp<Fence> * outFence) {
            if (outBuffer == NULL) {
                ALOGE("detachNextBuffer: outBuffer must not be NULL");
                return BAD_VALUE;
            } else if (outFence == NULL) {
                ALOGE("detachNextBuffer: outFence must not be NULL");
                return BAD_VALUE;
            }
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            status_t result = remote()->transact(DETACH_NEXT_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.readInt32();
            if (result == NO_ERROR) {
                bool nonNull = reply.readInt32();
                if (nonNull) {
                    *outBuffer = new GraphicBuffer;
                    reply.read(**outBuffer);
                }
                nonNull = reply.readInt32();
                if (nonNull) {
                    *outFence = new Fence;
                    reply.read(**outFence);
                }
            }
            return result;
        };

    inline virtual status_t attachBuffer(int * slot, const sp<GraphicBuffer> & buffer) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.write(*buffer.get());
            status_t result = remote()->transact(ATTACH_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            *slot = reply.readInt32();
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t queueBuffer(int buf, const QueueBufferInput & input, QueueBufferOutput * output) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(buf);
            data.write(input);
            status_t result = remote()->transact(QUEUE_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
            result = reply.readInt32();
            return result;
        };

    inline virtual void cancelBuffer(int buf, const sp<Fence> & fence) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(buf);
            data.write(*fence.get());
            remote()->transact(CANCEL_BUFFER, data, &reply);
        };

    inline virtual int query(int what, int * value) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(what);
            status_t result = remote()->transact(QUERY, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            value[0] = reply.readInt32();
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t connect(const sp<IProducerListener> & listener, int api, bool producerControlledByApp, QueueBufferOutput * output) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            if (listener != NULL) {
                data.writeInt32(1);
                data.writeStrongBinder(listener->asBinder());
            } else {
                data.writeInt32(0);
            }
            data.writeInt32(api);
            data.writeInt32(producerControlledByApp);
            status_t result = remote()->transact(CONNECT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            memcpy(output, reply.readInplace(sizeof(*output)), sizeof(*output));
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t disconnect(int api) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(api);
            status_t result =remote()->transact(DISCONNECT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t setSidebandStream(const sp<NativeHandle> & stream) {
            Parcel data, reply;
            status_t result;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            if (stream.get()) {
                data.writeInt32(true);
                data.writeNativeHandle(stream->handle());
            } else {
                data.writeInt32(false);
            }
            if ((result = remote()->transact(SET_SIDEBAND_STREAM, data, &reply)) == NO_ERROR) {
                result = reply.readInt32();
            }
            return result;
        };

    inline virtual void allocateBuffers(bool async, uint32_t width, uint32_t height, uint32_t format, uint32_t usage) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferProducer::getInterfaceDescriptor());
            data.writeInt32(static_cast<int32_t>(async));
            data.writeInt32(static_cast<int32_t>(width));
            data.writeInt32(static_cast<int32_t>(height));
            data.writeInt32(static_cast<int32_t>(format));
            data.writeInt32(static_cast<int32_t>(usage));
            status_t result = remote()->transact(ALLOCATE_BUFFERS, data, &reply);
            if (result != NO_ERROR) {
                ALOGE("allocateBuffers failed to transact: %d", result);
            }
        };

};

} // namespace android
#endif
