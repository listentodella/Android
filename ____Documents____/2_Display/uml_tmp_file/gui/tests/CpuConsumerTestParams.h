#ifndef ANDROID_CPUCONSUMERTESTPARAMS_H
#define ANDROID_CPUCONSUMERTESTPARAMS_H


namespace android {

struct CpuConsumerTestParams {
    uint32_t width;

    uint32_t height;

    int maxLockedBuffers;

    PixelFormat format;

};

} // namespace android
#endif
