#ifndef ANDROID_LIBBINDERIPCTSTATICS_H
#define ANDROID_LIBBINDERIPCTSTATICS_H


namespace android {

class LibBinderIPCtStatics {
  public:
    inline LibBinderIPCtStatics() {
        };

    inline ~LibBinderIPCtStatics() {
            IPCThreadState::shutdown();
        };

};

} // namespace android
#endif
