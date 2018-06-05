#ifndef ANDROID_BPGRAPHICBUFFERCONSUMER_H
#define ANDROID_BPGRAPHICBUFFERCONSUMER_H


#include "typedef10.h"

namespace android {

class BpGraphicBufferConsumer : public typedef10 {
  public:
    inline BpGraphicBufferConsumer(const sp<IBinder> & impl) : BpInterface<IGraphicBufferConsumer>(impl)
        {
        };

    inline virtual status_t acquireBuffer(BufferItem * buffer, nsecs_t presentWhen) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt64(presentWhen);
            status_t result = remote()->transact(ACQUIRE_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.read(*buffer);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t detachBuffer(int slot) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(slot);
            status_t result = remote()->transact(DETACH_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            result = reply.readInt32();
            return result;
        };

    inline virtual status_t attachBuffer(int * slot, const sp<GraphicBuffer> & buffer) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.write(*buffer.get());
            status_t result = remote()->transact(ATTACH_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            *slot = reply.readInt32();
            result = reply.readInt32();
            return result;
        };

    virtual status_t releaseBuffer();

    inline virtual status_t consumerConnect(const sp<IConsumerListener> & consumer, bool controlledByApp) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeStrongBinder(consumer->asBinder());
            data.writeInt32(controlledByApp);
            status_t result = remote()->transact(CONSUMER_CONNECT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t consumerDisconnect() {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            status_t result = remote()->transact(CONSUMER_DISCONNECT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t getReleasedBuffers(uint64_t * slotMask) {
            Parcel data, reply;
            if (slotMask == NULL) {
                ALOGE("getReleasedBuffers: slotMask must not be NULL");
                return BAD_VALUE;
            }
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            status_t result = remote()->transact(GET_RELEASED_BUFFERS, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            *slotMask = reply.readInt64();
            return reply.readInt32();
        };

    inline virtual status_t setDefaultBufferSize(uint32_t w, uint32_t h) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(w);
            data.writeInt32(h);
            status_t result = remote()->transact(SET_DEFAULT_BUFFER_SIZE, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t setDefaultMaxBufferCount(int bufferCount) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(bufferCount);
            status_t result = remote()->transact(SET_DEFAULT_MAX_BUFFER_COUNT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t disableAsyncBuffer() {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            status_t result = remote()->transact(DISABLE_ASYNC_BUFFER, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t setMaxAcquiredBufferCount(int maxAcquiredBuffers) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(maxAcquiredBuffers);
            status_t result = remote()->transact(SET_MAX_ACQUIRED_BUFFER_COUNT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual void setConsumerName(const String8 & name) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeString8(name);
            remote()->transact(SET_CONSUMER_NAME, data, &reply);
        };

    inline virtual status_t setDefaultBufferFormat(uint32_t defaultFormat) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(defaultFormat);
            status_t result = remote()->transact(SET_DEFAULT_BUFFER_FORMAT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t setConsumerUsageBits(uint32_t usage) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(usage);
            status_t result = remote()->transact(SET_CONSUMER_USAGE_BITS, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual status_t setTransformHint(uint32_t hint) {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeInt32(hint);
            status_t result = remote()->transact(SET_TRANSFORM_HINT, data, &reply);
            if (result != NO_ERROR) {
                return result;
            }
            return reply.readInt32();
        };

    inline virtual sp<NativeHandle> getSidebandStream() const {
            Parcel data, reply;
            status_t err;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            if ((err = remote()->transact(GET_SIDEBAND_STREAM, data, &reply)) != NO_ERROR) {
                return NULL;
            }
            sp<NativeHandle> stream;
            if (reply.readInt32()) {
                stream = NativeHandle::create(reply.readNativeHandle(), true);
            }
            return stream;
        };

    inline virtual void dump(String8 & result, const char * prefix) const {
            Parcel data, reply;
            data.writeInterfaceToken(IGraphicBufferConsumer::getInterfaceDescriptor());
            data.writeString8(result);
            data.writeString8(String8(prefix ? prefix : ""));
            remote()->transact(DUMP, data, &reply);
            reply.readString8();
        };

};

} // namespace android
#endif
