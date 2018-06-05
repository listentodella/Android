#ifndef ANDROID_DUMMYCONSUMER_H
#define ANDROID_DUMMYCONSUMER_H


#include "DisconnectWaiter.h"

namespace android {

struct DummyConsumer : public typedef1 {
    inline virtual void onFrameAvailable() {};

    inline virtual void onBuffersReleased() {};

    inline virtual void onSidebandStreamChanged() {};

};

} // namespace android
#endif
