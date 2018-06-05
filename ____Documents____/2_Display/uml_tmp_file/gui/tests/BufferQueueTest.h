#ifndef ANDROID_BUFFERQUEUETEST_H
#define ANDROID_BUFFERQUEUETEST_H


#include "GLTest.h"

namespace android {

class BufferQueueTest : public typedef3 {
  protected:
    inline BufferQueueTest() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("Begin test: %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

    inline ~BufferQueueTest() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("End test:   %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

    inline void GetMinUndequeuedBufferCount(int * bufferCount) {
            ASSERT_TRUE(bufferCount != NULL);
            ASSERT_EQ(OK, mProducer->query(NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS,
                        bufferCount));
            ASSERT_GE(*bufferCount, 0);
        };

    inline void createBufferQueue() {
            BufferQueue::createBufferQueue(&mProducer, &mConsumer);
        };

    sp<IGraphicBufferProducer> mProducer;

    sp<IGraphicBufferConsumer> mConsumer;

};

} // namespace android
#endif
