#ifndef ANDROID_DUMMYLISTENER_H
#define ANDROID_DUMMYLISTENER_H


#include "DisconnectWaiter.h"

namespace android {

struct DummyListener : public typedef1 {
    inline virtual void onFrameAvailable() {};

    inline virtual void onBuffersReleased() {};

    inline virtual void onSidebandStreamChanged() {};

};

} // namespace android
#endif
