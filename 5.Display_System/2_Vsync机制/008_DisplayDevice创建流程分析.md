# 008_DisplayDevice创建流程分析

标签（空格分隔）： Display

---

> 之前提到过，Android为了优化效果，使用了3个buffer，那么在哪个buffer上合并数据？这些buffer又如何获得？

app使用`Surface`来管理这些buffer，`DisplayDevice`也是使用`Surface`来管理
DisplayDevice:
`dequeueBuffer`:向sf申请framebuffer
`queueBuffer`:提交buffer给sf，sf发送给显示驱动程序

```
SurfaceFlinger.cpp
void SurfaceFlinger::init() {
...
    sp<FramebufferSurface> fbs = new FramebufferSurface(*mHwc, i, consumer);
    int32_t hwcId = allocateHwcDisplayId(type);
//创建DisplayDevice
    sp<DisplayDevice> hw = new DisplayDevice(this,
            type, hwcId, mHwc->getFormat(hwcId), isSecure, token,
            fbs, producer,
            mRenderEngine->getEGLConfig());

...
}


FramebufferSurface.cpp
FramebufferSurface::FramebufferSurface(HWComposer& hwc, int disp,
        const sp<IGraphicBufferConsumer>& consumer) :
    ConsumerBase(consumer),
    mDisplayType(disp),
    mCurrentBufferSlot(-1),
    mCurrentBuffer(0),
    mHwc(hwc)
{
    mName = "FramebufferSurface";
    mConsumer->setConsumerName(mName);
    
//会根据GRALLOC_USAGE_HW_FB的值来分辨，向Ashmem申请->for app or 向 Framebuffer申请->for DisplayDevice
    mConsumer->setConsumerUsageBits(GRALLOC_USAGE_HW_FB |
                                       GRALLOC_USAGE_HW_RENDER |
                                       GRALLOC_USAGE_HW_COMPOSER);
    mConsumer->setDefaultBufferFormat(mHwc.getFormat(disp));
    mConsumer->setDefaultBufferSize(mHwc.getWidth(disp),  mHwc.getHeight(disp));
    mConsumer->setDefaultMaxBufferCount(NUM_FRAMEBUFFER_SURFACE_BUFFERS);
}



BufferQueueConsumer.cpp
status_t BufferQueueConsumer::setConsumerUsageBits(uint32_t usage) {
    ATRACE_CALL();
    BQ_LOGV("setConsumerUsageBits: %#x", usage);
    Mutex::Autolock lock(mCore->mMutex);
    mCore->mConsumerUsageBits = usage;
    return NO_ERROR;
}


```

```
DisplayDevice.cpp
DisplayDevice::DisplayDevice(
        const sp<SurfaceFlinger>& flinger,
        DisplayType type,
        int32_t hwcId,
        int format,
        bool isSecure,
        const wp<IBinder>& displayToken,
        const sp<DisplaySurface>& displaySurface,
        const sp<IGraphicBufferProducer>& producer,
        EGLConfig config)
    : lastCompositionHadVisibleLayers(false),
      mFlinger(flinger),
      mType(type), mHwcDisplayId(hwcId),
      mDisplayToken(displayToken),
      mDisplaySurface(displaySurface),
      mDisplay(EGL_NO_DISPLAY),
      mSurface(EGL_NO_SURFACE),
      mDisplayWidth(), mDisplayHeight(), mFormat(),
      mFlags(),
      mPageFlipCount(),
      mIsSecure(isSecure),
      mSecureLayerVisible(false),
      mLayerStack(NO_LAYER_STACK),
      mOrientation(),
      mPowerMode(HWC_POWER_MODE_OFF),
      mActiveConfig(0)
{
    mNativeWindow = new Surface(producer, false);
    ANativeWindow* const window = mNativeWindow.get();

    /*
     * Create our display's surface
     */

    EGLSurface surface;
    EGLint w, h;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (config == EGL_NO_CONFIG) {
        config = RenderEngine::chooseEglConfig(display, format);
    }
    surface = eglCreateWindowSurface(display, config, window, NULL);
    eglQuerySurface(display, surface, EGL_WIDTH,  &mDisplayWidth);
    eglQuerySurface(display, surface, EGL_HEIGHT, &mDisplayHeight);

    // Make sure that composition can never be stalled by a virtual display
    // consumer that isn't processing buffers fast enough. We have to do this
    // in two places:
    // * Here, in case the display is composed entirely by HWC.
    // * In makeCurrent(), using eglSwapInterval. Some EGL drivers set the
    //   window's swap interval in eglMakeCurrent, so they'll override the
    //   interval we set here.
    if (mType >= DisplayDevice::DISPLAY_VIRTUAL)
        window->setSwapInterval(window, 0);

    mConfig = config;
    mDisplay = display;
    mSurface = surface;
    mFormat  = format;
    mPageFlipCount = 0;
    mViewport.makeInvalid();
    mFrame.makeInvalid();

    // virtual displays are always considered enabled
    mPowerMode = (mType >= DisplayDevice::DISPLAY_VIRTUAL) ?
                  HWC_POWER_MODE_NORMAL : HWC_POWER_MODE_OFF;

    // Name the display.  The name will be replaced shortly if the display
    // was created with createDisplay().
    switch (mType) {
        case DISPLAY_PRIMARY:
            mDisplayName = "Built-in Screen";
            break;
        case DISPLAY_EXTERNAL:
            mDisplayName = "HDMI Screen";
            break;
        default:
            mDisplayName = "Virtual Screen";    // e.g. Overlay #n
            break;
    }

    // initialize the display orientation transform.
    setProjection(DisplayState::eOrientationDefault, mViewport, mFrame);
}

```

`DisplayDevice`的`mSurface`成员，指向
```
egl.cpp
surface = new egl_window_surface_v2_t(dpy, config, depthFormat, static_cast<ANativeWindow*>(window))
```
其`winodw`参数则指向一个`surface`对象,而`egl_window_surface_v2_t`对象含有一个`nativeWindow`成员（实际是`ANativeWindow`对象指针）,指向`window`

* `ANativeWindow`的重要性如下（`Surface`继承自`ANativeWindow`）
```
Surface::Surface(
        const sp<IGraphicBufferProducer>& bufferProducer,
        bool controlledByApp)
    : mGraphicBufferProducer(bufferProducer)
{
    // Initialize the ANativeWindow function pointers.
    ANativeWindow::setSwapInterval  = hook_setSwapInterval;
    ANativeWindow::dequeueBuffer    = hook_dequeueBuffer;
    ANativeWindow::cancelBuffer     = hook_cancelBuffer;
    ANativeWindow::queueBuffer      = hook_queueBuffer;
    ANativeWindow::query            = hook_query;
    ANativeWindow::perform          = hook_perform;

    ANativeWindow::dequeueBuffer_DEPRECATED = hook_dequeueBuffer_DEPRECATED;
    ANativeWindow::cancelBuffer_DEPRECATED  = hook_cancelBuffer_DEPRECATED;
    ANativeWindow::lockBuffer_DEPRECATED    = hook_lockBuffer_DEPRECATED;
    ANativeWindow::queueBuffer_DEPRECATED   = hook_queueBuffer_DEPRECATED;

    const_cast<int&>(ANativeWindow::minSwapInterval) = 0;
    const_cast<int&>(ANativeWindow::maxSwapInterval) = 1;

    ...
}

```

###例子`DisplayDevice::SwapBuffer`
####`queueBuffer`
* 把构造好的buffer放进`mCore`的队列中
* 调用`FramebufferSurface::onFrameAvailable`==>post给硬件
####`dequeueBuffer`