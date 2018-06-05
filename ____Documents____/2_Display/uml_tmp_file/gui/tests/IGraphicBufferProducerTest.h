#ifndef ANDROID_IGRAPHICBUFFERPRODUCERTEST_H
#define ANDROID_IGRAPHICBUFFERPRODUCERTEST_H


#include "GLTest.h"

namespace android { struct DummyConsumer; } 

namespace android {

class IGraphicBufferProducerTest : public typedef3 {
  protected:
    inline IGraphicBufferProducerTest() {};

    inline virtual void SetUp() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("Begin test: %s.%s", testInfo->test_case_name(),
                    testInfo->name());
    
            mDC = new DummyConsumer;
    
            BufferQueue::createBufferQueue(&mProducer, &mConsumer);
    
            // Test check: Can't connect producer if no consumer yet
            ASSERT_EQ(NO_INIT, TryConnectProducer());
    
            // Must connect consumer before producer connects will succeed.
            ASSERT_OK(mConsumer->consumerConnect(mDC, /*controlledByApp*/false));
        };

    inline virtual void TearDown() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("End test:   %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

    inline status_t TryConnectProducer() {
            IGraphicBufferProducer::QueueBufferOutput output;
            return mProducer->connect(TEST_TOKEN,
                                      TEST_API,
                                      TEST_CONTROLLED_BY_APP,
                                      &output);
            // TODO: use params to vary token, api, producercontrolledbyapp, etc
        };

    // Connect to a producer in a 'correct' fashion.
    //   Precondition: Consumer is connected.
    inline void ConnectProducer() {
            ASSERT_OK(TryConnectProducer());
        };

    // Create a generic "valid" input for queueBuffer
    // -- uses the default buffer format, width, etc.
    static inline IGraphicBufferProducer::QueueBufferInput CreateBufferInput() {
            return QueueBufferInputBuilder().build();
        };


  public:
    struct QueueBufferInputBuilder {
        inline QueueBufferInputBuilder() {
                   timestamp = QUEUE_BUFFER_INPUT_TIMESTAMP;
                   isAutoTimestamp = QUEUE_BUFFER_INPUT_IS_AUTO_TIMESTAMP;
                   crop = QUEUE_BUFFER_INPUT_RECT;
                   scalingMode = QUEUE_BUFFER_INPUT_SCALING_MODE;
                   transform = QUEUE_BUFFER_INPUT_TRANSFORM;
                   async = QUEUE_BUFFER_INPUT_ASYNC;
                   fence = QUEUE_BUFFER_INPUT_FENCE;
                };

        inline IGraphicBufferProducer::QueueBufferInput build() {
                    return IGraphicBufferProducer::QueueBufferInput(
                            timestamp,
                            isAutoTimestamp,
                            crop,
                            scalingMode,
                            transform,
                            async,
                            fence);
                };

        inline QueueBufferInputBuilder & setTimestamp(int64_t timestamp) {
                    this->timestamp = timestamp;
                    return *this;
                };

        inline QueueBufferInputBuilder & setIsAutoTimestamp(bool isAutoTimestamp) {
                    this->isAutoTimestamp = isAutoTimestamp;
                    return *this;
                };

        inline QueueBufferInputBuilder & setCrop(Rect crop) {
                    this->crop = crop;
                    return *this;
                };

        inline QueueBufferInputBuilder & setScalingMode(int scalingMode) {
                    this->scalingMode = scalingMode;
                    return *this;
                };

        inline QueueBufferInputBuilder & setTransform(uint32_t transform) {
                    this->transform = transform;
                    return *this;
                };

        inline QueueBufferInputBuilder & setAsync(bool async) {
                    this->async = async;
                    return *this;
                };

        inline QueueBufferInputBuilder & setFence(sp<Fence> fence) {
                    this->fence = fence;
                    return *this;
                };


      private:
        int64_t timestamp;

        bool isAutoTimestamp;

        Rect crop;

        int scalingMode;

        uint32_t transform;

        int async;

        sp<Fence> fence;

    };
    
    struct DequeueBufferResult {
        int slot;

        sp<Fence> fence;

    };
    

  protected:
    inline status_t dequeueBuffer(bool async, uint32_t w, uint32_t h, uint32_t format, uint32_t usage, DequeueBufferResult * result) {
            return mProducer->dequeueBuffer(&result->slot, &result->fence, async, w, h, format, usage);
        };


  private:
    // hide from test body
    sp<DummyConsumer> mDC;


  protected:
    // accessible from test body
    sp<IGraphicBufferProducer> mProducer;

    sp<IGraphicBufferConsumer> mConsumer;

};

} // namespace android
#endif
