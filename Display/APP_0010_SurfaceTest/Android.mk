# 编译出的结果在/out/target/product/tiny4412/system/bin

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	resize.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
    libui \
    libgui \
	libbinder

LOCAL_MODULE:= SurfaceTest

LOCAL_MODULE_TAGS := tests

include $(BUILD_EXECUTABLE)
