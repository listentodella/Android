#ifndef ANDROID_DISCONNECTWAITER_H
#define ANDROID_DISCONNECTWAITER_H


namespace android {

typedef BnConsumerListener typedef1;
// Note that GLConsumer will lose the notifications
// onBuffersReleased and onFrameAvailable as there is currently
// no way to forward the events.  This DisconnectWaiter will not let the
// disconnect finish until finishDisconnect() is called.  It will
// also block until a disconnect is called
class DisconnectWaiter : public typedef1 {
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

    inline virtual void onSidebandStreamChanged() {};

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

} // namespace android
#endif
