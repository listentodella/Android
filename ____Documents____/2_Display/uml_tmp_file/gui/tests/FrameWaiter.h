#ifndef ANDROID_FRAMEWAITER_H
#define ANDROID_FRAMEWAITER_H


namespace android {

typedef GLConsumer::FrameAvailableListener typedef2;
class FrameWaiter : public typedef2 {
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


  private:
    int mPendingFrames;

    Mutex mMutex;

    Condition mCondition;

};

} // namespace android
#endif
