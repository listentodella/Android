#ifndef ANDROID_STREAMSPLITTERTEST_H
#define ANDROID_STREAMSPLITTERTEST_H


#include "GLTest.h"

namespace android {

class StreamSplitterTest : public typedef3 {
  protected:
    inline StreamSplitterTest() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("Begin test: %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

    inline ~StreamSplitterTest() {
            const ::testing::TestInfo* const testInfo =
                ::testing::UnitTest::GetInstance()->current_test_info();
            ALOGV("End test:   %s.%s", testInfo->test_case_name(),
                    testInfo->name());
        };

};

} // namespace android
#endif
