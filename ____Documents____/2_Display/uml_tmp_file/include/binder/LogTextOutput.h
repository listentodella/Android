#ifndef ANDROID_LOGTEXTOUTPUT_H
#define ANDROID_LOGTEXTOUTPUT_H


#include "typedef12.h"
#include "BufferedTextOutput.h"

namespace android {

class LogTextOutput : public typedef12 {
  public:
    inline LogTextOutput() : BufferedTextOutput(MULTITHREADED) { };

    inline virtual ~LogTextOutput() { };


  protected:
    inline virtual status_t writeLines(const iovec & vec, size_t N) {
            //android_writevLog(&vec, N);       <-- this is now a no-op
            if (N != 1) ALOGI("WARNING: writeLines N=%zu\n", N);
            ALOGI("%.*s", (int)vec.iov_len, (const char*) vec.iov_base);
            return NO_ERROR;
        };

};

} // namespace android
#endif
