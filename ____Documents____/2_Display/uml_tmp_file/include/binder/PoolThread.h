#ifndef ANDROID_POOLTHREAD_H
#define ANDROID_POOLTHREAD_H


#include "typedef11.h"

namespace android {

// ---------------------------------------------------------------------------
class PoolThread : public typedef11 {
  public:
    inline PoolThread(bool isMain) : mIsMain(isMain)
        {
        };


  protected:
    inline virtual bool threadLoop() {
            IPCThreadState::self()->joinThreadPool(mIsMain);
            return false;
        };

    const bool mIsMain;

};

} // namespace android
#endif
