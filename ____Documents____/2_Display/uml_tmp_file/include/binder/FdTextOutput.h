#ifndef ANDROID_FDTEXTOUTPUT_H
#define ANDROID_FDTEXTOUTPUT_H


#include "typedef12.h"
#include "BufferedTextOutput.h"

namespace android {

class FdTextOutput : public typedef12 {
  public:
    inline FdTextOutput(int fd) : BufferedTextOutput(MULTITHREADED), mFD(fd) { };

    inline virtual ~FdTextOutput() { };


  protected:
    inline virtual status_t writeLines(const iovec & vec, size_t N) {
            writev(mFD, &vec, N);
            return NO_ERROR;
        };


  private:
    int mFD;

};

} // namespace android
#endif
