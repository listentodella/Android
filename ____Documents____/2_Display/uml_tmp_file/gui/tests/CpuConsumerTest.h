#ifndef ANDROID_CPUCONSUMERTEST_H
#define ANDROID_CPUCONSUMERTEST_H


#include "typedef18.h"

namespace android {

class CpuConsumerTest : public typedef18 {
  protected:
    inline virtual void SetUp() {
            const ::testing::TestInfo* const test_info =
                    ::testing::UnitTest::GetInstance()->current_test_info();
            CpuConsumerTestParams params = GetParam();
            ALOGV("** Starting test %s (%d x %d, %d, 0x%x)",
                    test_info->name(),
                    params.width, params.height,
                    params.maxLockedBuffers, params.format);
            sp<IGraphicBufferProducer> producer;
            sp<IGraphicBufferConsumer> consumer;
            BufferQueue::createBufferQueue(&producer, &consumer);
            mCC = new CpuConsumer(consumer, params.maxLockedBuffers);
            String8 name("CpuConsumer_Under_Test");
            mCC->setName(name);
            mSTC = new Surface(producer);
            mANW = mSTC;
        };

    inline virtual void TearDown() {
            mANW.clear();
            mSTC.clear();
            mCC.clear();
        };


  public:
    class FrameWaiter : public typedef19 {
      public:
        inline FrameWaiter() :
                        mPendingFrames(0) {
                };

        inline void waitForFrame() {
                    Mutex::Autolock lock(mMutex);
                    while (mPendingFrames == 0) {
                        mCondition.wait(mMutex);
                    }
                    mPendingFrames--;
                };

        inline virtual void onFrameAvailable() {
                    Mutex::Autolock lock(mMutex);
                    mPendingFrames++;
                    mCondition.signal();
                };

        int mPendingFrames;

        Mutex mMutex;

        Condition mCondition;

    };
    
    typedef CpuConsumer::FrameAvailableListener typedef19;

    class DisconnectWaiter : public typedef20 {
      public:
        inline DisconnectWaiter() :
                    mWaitForDisconnect(false),
                    mPendingFrames(0) {
                };

        inline void waitForFrame() {
                    Mutex::Autolock lock(mMutex);
                    while (mPendingFrames == 0) {
                        mFrameCondition.wait(mMutex);
                    }
                    mPendingFrames--;
                };

        inline virtual void onFrameAvailable() {
                    Mutex::Autolock lock(mMutex);
                    mPendingFrames++;
                    mFrameCondition.signal();
                };

        inline virtual void onBuffersReleased() {
                    Mutex::Autolock lock(mMutex);
                    while (!mWaitForDisconnect) {
                        mDisconnectCondition.wait(mMutex);
                    }
                };

        inline void finishDisconnect() {
                    Mutex::Autolock lock(mMutex);
                    mWaitForDisconnect = true;
                    mDisconnectCondition.signal();
                };


      private:
        Mutex mMutex;

        bool mWaitForDisconnect;

        Condition mDisconnectCondition;

        int mPendingFrames;

        Condition mFrameCondition;

    };
    
    typedef BufferQueue::ConsumerListener typedef20;


  protected:
    sp<CpuConsumer> mCC;

    sp<Surface> mSTC;

    sp<ANativeWindow> mANW;

};

} // namespace android
#endif
