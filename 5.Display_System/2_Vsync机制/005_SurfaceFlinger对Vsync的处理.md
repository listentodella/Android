# 005_SurfaceFlinger对Vsync的处理

标签（空格分隔）： Display Vsync

---

 1. 多显示器Display
 add 、remove
 2. 多个`surface`,每个`surface`有多个`Layer`
 内容更新、add/remove、属性变化（比如大小变化）、界面角度变化等等

## 几个重要的成员
通过比较各自的`State`的对象`mDrawingState`、`mCurrentState`里的关键成员，来判断上面的变化
* `SurfaceFlinger`中的`State`
```
SurfaceFlinger.h
★ struct State {
    LayerVector layersSortedByZ;//比较它，就可以知道Layer的增加、减少
    DefaultKeyedVector< wp<IBinder>, DisplayDeviceState> displays;//比较它，就可以知道Display的增加、减少
};
↓
可以理解为
struct SurfaceFlinger::State {
    ...
}

★ State mDrawingState;//正在/上次使用的状态
★ State mCurrentState;//当前的/被修改的状态
```
* `Layer`中的`State`
```
/frameworks/native/services/surfaceflinger/Layer.h
★ struct State {
    Geometry active;
    Geometry requested;
    uint32_t z;
    uint32_t layerStack;
    uint8_t alpha;
    uint8_t flags;
    uint8_t reserved[2];
//比较它，就可以知道属性有无变化
    int32_t sequence; // changes when visible regions can change
    Transform transform;
    // the transparentRegion hint is a bit special, it's latched only
    // when we receive a buffer -- this is because it's "content"
    // dependent.
    Region activeTransparentRegion;
    Region requestedTransparentRegion;
};
可以理解为 struct Layer::State{...}
★ State mDrawingState;//正在/上次使用的状态
★ State mCurrentState;//当前的/被修改的状态
```
另外，`SurfaceFlinger::State->displays->layerStack`与`Layer::State->layerStack`如果相等，该`Layer`就可以在这个`Display`显示
```
/frameworks/native/services/surfaceflinger/SurfaceFlinger.h
struct DisplayDeviceState {
    DisplayDeviceState();
    DisplayDeviceState(DisplayDevice::DisplayType type);
    bool isValid() const { return type >= 0; }
    bool isMainDisplay() const { return type == DisplayDevice::DISPLAY_PRIMARY; }
    bool isVirtualDisplay() const { return type >= DisplayDevice::DISPLAY_VIRTUAL; }
    DisplayDevice::DisplayType type;
    sp<IGraphicBufferProducer> surface;
★    uint32_t layerStack;
    Rect viewport;
    Rect frame;
    uint8_t orientation;
    uint32_t width, height;
    String8 displayName;
    bool isSecure;
};
```
## 代码分析
由上一篇可知，`SurfaceFlinger`处理`Vsync`时最终会调用到`SurfaceFlinger::onMessageReceived()`.
先来看一下`SurfaceFlinger`对`Vsync`的处理的几个主要的点:
* 处理各种事务
* 处理各`Layer`的Buffer更换
* 发出`Refresh`信号，最终导致`handleMessageRefresh()`被调用
```
void SurfaceFlinger::onMessageReceived(int32_t what) {
    ATRACE_CALL();
    switch (what) {
...
    case MessageQueue::INVALIDATE:
①        handleMessageTransaction();
②        handleMessageInvalidate();
③        signalRefresh();
        break;
    case MessageQueue::REFRESH:
        handleMessageRefresh();
        break;
    }
}

```
### 处理各种事务 `handleMessageTransaction()`
只是设置一些flag，并未实际操作各个Buffer
```seq
handleMessageTransaction-> handleTransaction:transactionFlags
handleTransaction->handleTransactionLocked:transactionFlags

```
![handleMessageTransaction部分流程](handleMessageTransaction%E9%83%A8%E5%88%86%E6%B5%81%E7%A8%8B.jpg)

* 遍历每个`Layer`,执行`Layer->doTransaction`，根据返回值设置`mVisibleRegionDirty`
* 处理Display（显示器）事务，add/remove/change
* `Layer`的角度发生了变化（例如屏幕旋转）
* 处理`SurfaceFlinger`本身的事务：layer的增加、减少

```
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
void SurfaceFlinger::handleMessageTransaction() {
    uint32_t transactionFlags = peekTransactionFlags(eTransactionMask);
    if (transactionFlags) {
☆        handleTransaction(transactionFlags);
    }
}
↓
↓
↓
void SurfaceFlinger::handleTransaction(uint32_t transactionFlags)
{
    ATRACE_CALL();

    // here we keep a copy of the drawing state (that is the state that's
    // going to be overwritten by handleTransactionLocked()) outside of
    // mStateLock so that the side-effects of the State assignment
    // don't happen with mStateLock held (which can cause deadlocks).
    State drawingState(mDrawingState);

    Mutex::Autolock _l(mStateLock);
    const nsecs_t now = systemTime();
    mDebugInTransaction = now;

    // Here we're guaranteed that some transaction flags are set
    // so we can call handleTransactionLocked() unconditionally.
    // We call getTransactionFlags(), which will also clear the flags,
    // with mStateLock held to guarantee that mCurrentState won't change
    // until the transaction is committed.

    transactionFlags = getTransactionFlags(eTransactionMask);
☆    handleTransactionLocked(transactionFlags);

    mLastTransactionTime = systemTime() - now;
    mDebugInTransaction = 0;
    invalidateHwcGeometry();
    // here the transaction has been committed
}
↓
↓
↓
void SurfaceFlinger::handleTransactionLocked(uint32_t transactionFlags)
{
    const LayerVector& currentLayers(mCurrentState.layersSortedByZ);
    const size_t count = currentLayers.size();

//遍历每一个Layer
    if (transactionFlags & eTraversalNeeded) {
        for (size_t i=0 ; i<count ; i++) {
            const sp<Layer>& layer(currentLayers[i]);
            uint32_t trFlags = layer->getTransactionFlags(eTransactionNeeded);
            if (!trFlags) continue;
//执行Layer->doTransaction,根据返回值设置mVisibleRegionDirty
            const uint32_t flags = layer->doTransaction(0);
            if (flags & Layer::eVisibleRegion)
                mVisibleRegionsDirty = true;
        }
    }

    /*
     * Perform display own transactions if needed
     */

    if (transactionFlags & eDisplayTransactionNeeded) {
        // here we take advantage of Vector's copy-on-write semantics to
        // improve performance by skipping the transaction entirely when
        // know that the lists are identical
        const KeyedVector<  wp<IBinder>, DisplayDeviceState>& curr(mCurrentState.displays);
        const KeyedVector<  wp<IBinder>, DisplayDeviceState>& draw(mDrawingState.displays);
        if (!curr.isIdenticalTo(draw)) {
            mVisibleRegionsDirty = true;
            const size_t cc = curr.size();
                  size_t dc = draw.size();

            // find the displays that were removed
            // (ie: in drawing state but not in current state)
            // also handle displays that changed
            // (ie: displays that are in both lists)
            for (size_t i=0 ; i<dc ; i++) {
                const ssize_t j = curr.indexOfKey(draw.keyAt(i));
                if (j < 0) {
                    // in drawing state but not in current state
                    if (!draw[i].isMainDisplay()) {
                        // Call makeCurrent() on the primary display so we can
                        // be sure that nothing associated with this display
                        // is current.
                        const sp<const DisplayDevice> defaultDisplay(getDefaultDisplayDevice());
                        defaultDisplay->makeCurrent(mEGLDisplay, mEGLContext);
                        sp<DisplayDevice> hw(getDisplayDevice(draw.keyAt(i)));
                        if (hw != NULL)
                            hw->disconnect(getHwComposer());
                        if (draw[i].type < DisplayDevice::NUM_BUILTIN_DISPLAY_TYPES)
                            mEventThread->onHotplugReceived(draw[i].type, false);
                        mDisplays.removeItem(draw.keyAt(i));
                    } else {
                        ALOGW("trying to remove the main display");
                    }
                } else {
                    // this display is in both lists. see if something changed.
                    const DisplayDeviceState& state(curr[j]);
                    const wp<IBinder>& display(curr.keyAt(j));
                    if (state.surface->asBinder() != draw[i].surface->asBinder()) {
                        // changing the surface is like destroying and
                        // recreating the DisplayDevice, so we just remove it
                        // from the drawing state, so that it get re-added
                        // below.
                        sp<DisplayDevice> hw(getDisplayDevice(display));
                        if (hw != NULL)
                            hw->disconnect(getHwComposer());
                        mDisplays.removeItem(display);
                        mDrawingState.displays.removeItemsAt(i);
                        dc--; i--;
                        // at this point we must loop to the next item
                        continue;
                    }

                    const sp<DisplayDevice> disp(getDisplayDevice(display));
                    if (disp != NULL) {
                        if (state.layerStack != draw[i].layerStack) {
                            disp->setLayerStack(state.layerStack);
                        }
                        if ((state.orientation != draw[i].orientation)
                                || (state.viewport != draw[i].viewport)
                                || (state.frame != draw[i].frame))
                        {
                            disp->setProjection(state.orientation,
                                    state.viewport, state.frame);
                        }
                        if (state.width != draw[i].width || state.height != draw[i].height) {
                            disp->setDisplaySize(state.width, state.height);
                        }
                    }
                }
            }

            // find displays that were added
            // (ie: in current state but not in drawing state)
            for (size_t i=0 ; i<cc ; i++) {
                if (draw.indexOfKey(curr.keyAt(i)) < 0) {
                    const DisplayDeviceState& state(curr[i]);

                    sp<DisplaySurface> dispSurface;
                    sp<IGraphicBufferProducer> producer;
                    sp<IGraphicBufferProducer> bqProducer;
                    sp<IGraphicBufferConsumer> bqConsumer;
                    BufferQueue::createBufferQueue(&bqProducer, &bqConsumer,
                            new GraphicBufferAlloc());

                    int32_t hwcDisplayId = -1;
                    if (state.isVirtualDisplay()) {
                        // Virtual displays without a surface are dormant:
                        // they have external state (layer stack, projection,
                        // etc.) but no internal state (i.e. a DisplayDevice).
                        if (state.surface != NULL) {

                            hwcDisplayId = allocateHwcDisplayId(state.type);
                            sp<VirtualDisplaySurface> vds = new VirtualDisplaySurface(
                                    *mHwc, hwcDisplayId, state.surface,
                                    bqProducer, bqConsumer, state.displayName);

                            dispSurface = vds;
                            producer = vds;
                        }
                    } else {
                        ALOGE_IF(state.surface!=NULL,
                                "adding a supported display, but rendering "
                                "surface is provided (%p), ignoring it",
                                state.surface.get());
                        hwcDisplayId = allocateHwcDisplayId(state.type);
                        // for supported (by hwc) displays we provide our
                        // own rendering surface
                        dispSurface = new FramebufferSurface(*mHwc, state.type,
                                bqConsumer);
                        producer = bqProducer;
                    }

                    const wp<IBinder>& display(curr.keyAt(i));
                    if (dispSurface != NULL) {
                        sp<DisplayDevice> hw = new DisplayDevice(this,
                                state.type, hwcDisplayId,
                                mHwc->getFormat(hwcDisplayId), state.isSecure,
                                display, dispSurface, producer,
                                mRenderEngine->getEGLConfig());
                        hw->setLayerStack(state.layerStack);
                        hw->setProjection(state.orientation,
                                state.viewport, state.frame);
                        hw->setDisplayName(state.displayName);
                        mDisplays.add(display, hw);
                        if (state.isVirtualDisplay()) {
                            if (hwcDisplayId >= 0) {
                                mHwc->setVirtualDisplayProperties(hwcDisplayId,
                                        hw->getWidth(), hw->getHeight(),
                                        hw->getFormat());
                            }
                        } else {
                            mEventThread->onHotplugReceived(state.type, true);
                        }
                    }
                }
            }
        }
    }

    if (transactionFlags & (eTraversalNeeded|eDisplayTransactionNeeded)) {
        // The transform hint might have changed for some layers
        // (either because a display has changed, or because a layer
        // as changed).
        //
        // Walk through all the layers in currentLayers,
        // and update their transform hint.
        //
        // If a layer is visible only on a single display, then that
        // display is used to calculate the hint, otherwise we use the
        // default display.
        //
        // NOTE: we do this here, rather than in rebuildLayerStacks() so that
        // the hint is set before we acquire a buffer from the surface texture.
        //
        // NOTE: layer transactions have taken place already, so we use their
        // drawing state. However, SurfaceFlinger's own transaction has not
        // happened yet, so we must use the current state layer list
        // (soon to become the drawing state list).
        //
        sp<const DisplayDevice> disp;
        uint32_t currentlayerStack = 0;
        for (size_t i=0; i<count; i++) {
            // NOTE: we rely on the fact that layers are sorted by
            // layerStack first (so we don't have to traverse the list
            // of displays for every layer).
            const sp<Layer>& layer(currentLayers[i]);
            uint32_t layerStack = layer->getDrawingState().layerStack;
            if (i==0 || currentlayerStack != layerStack) {
                currentlayerStack = layerStack;
                // figure out if this layerstack is mirrored
                // (more than one display) if so, pick the default display,
                // if not, pick the only display it's on.
                disp.clear();
                for (size_t dpy=0 ; dpy<mDisplays.size() ; dpy++) {
                    sp<const DisplayDevice> hw(mDisplays[dpy]);
                    if (hw->getLayerStack() == currentlayerStack) {
                        if (disp == NULL) {
                            disp = hw;
                        } else {
                            disp = NULL;
                            break;
                        }
                    }
                }
            }
            if (disp == NULL) {
                // NOTE: TEMPORARY FIX ONLY. Real fix should cause layers to
                // redraw after transform hint changes. See bug 8508397.

                // could be null when this layer is using a layerStack
                // that is not visible on any display. Also can occur at
                // screen off/on times.
                disp = getDefaultDisplayDevice();
            }
            layer->updateTransformHint(disp);
        }
    }


    /*
     * Perform our own transaction if needed
     */

    const LayerVector& layers(mDrawingState.layersSortedByZ);
    if (currentLayers.size() > layers.size()) {
        // layers have been added
        mVisibleRegionsDirty = true;
    }

    // some layers might have been removed, so
    // we need to update the regions they're exposing.
    if (mLayersRemoved) {
        mLayersRemoved = false;
        mVisibleRegionsDirty = true;
        const size_t count = layers.size();
        for (size_t i=0 ; i<count ; i++) {
            const sp<Layer>& layer(layers[i]);
            if (currentLayers.indexOf(layer) < 0) {
                // this layer is not visible anymore
                // TODO: we could traverse the tree from front to back and
                //       compute the actual visible region
                // TODO: we could cache the transformed region
                const Layer::State& s(layer->getDrawingState());
                Region visibleReg = s.transform.transform(
                        Region(Rect(s.active.w, s.active.h)));
                invalidateLayerStack(s.layerStack, visibleReg);
            }
        }
    }

    commitTransaction();

    updateCursorAsync();
}

```

### 处理各Layer的Buffer更换`handleMessageInvalidate();`
使原来的界面数据无效：准备好新数据用来更换
```seq
handleMessageInvalidate->handlePageFlip:
handlePageFlip->latchBuffer:
latchBuffer->updateTexImage:
updateTexImage->bindTextureImageLocked:
bindTextureImageLocked->bindToTextureTarget:
bindToTextureTarget->glEGLImageTargetTexture2DOES:
```
![handleMessageInvalidate部分流程](handleMessageInvalidate%E9%83%A8%E5%88%86%E6%B5%81%E7%A8%8B.jpg)

* `accquire` next Buffer
* `release` previous Buffer
* `bindTextureImageLocked`-->`glEGLImageTargetTexture2DOES`

![Buffer过程](Buffer%E8%BF%87%E7%A8%8B.jpg)

```
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
void SurfaceFlinger::handleMessageInvalidate() {
    ATRACE_CALL();
☆    handlePageFlip();
}
↓
↓
↓
void SurfaceFlinger::handlePageFlip()
{
    Region dirtyRegion;

    bool visibleRegions = false;
    const LayerVector& layers(mDrawingState.layersSortedByZ);

    // Store the set of layers that need updates. This set must not change as
    // buffers are being latched, as this could result in a deadlock.
    // Example: Two producers share the same command stream and:
    // 1.) Layer 0 is latched
    // 2.) Layer 0 gets a new frame
    // 2.) Layer 1 gets a new frame
    // 3.) Layer 1 is latched.
    // Display is now waiting on Layer 1's frame, which is behind layer 0's
    // second frame. But layer 0's second frame could be waiting on display.
    Vector<Layer*> layersWithQueuedFrames;
    for (size_t i = 0, count = layers.size(); i<count ; i++) {
        const sp<Layer>& layer(layers[i]);
        if (layer->hasQueuedFrame())
            layersWithQueuedFrames.push_back(layer.get());
    }
    for (size_t i = 0, count = layersWithQueuedFrames.size() ; i<count ; i++) {
        Layer* layer = layersWithQueuedFrames[i];
☆        const Region dirty(layer->latchBuffer(visibleRegions));
        const Layer::State& s(layer->getDrawingState());
        invalidateLayerStack(s.layerStack, dirty);
    }

    mVisibleRegionsDirty |= visibleRegions;
}
↓
↓
↓
/frameworks/native/services/surfaceflinger/Layer.cpp
Region Layer::latchBuffer(bool& recomputeVisibleRegions)
{
    ATRACE_CALL();
...
☆    status_t updateResult = mSurfaceFlingerConsumer->updateTexImage(&r,
         mFlinger->mPrimaryDispSync);
    if (updateResult == BufferQueue::PRESENT_LATER) {
     // Producer doesn't want buffer to be displayed yet.  Signal a
     // layer update so we check again at the next opportunity.
     mFlinger->signalLayerUpdate();
     return outDirtyRegion;
    }
...
}
↓
↓
↓
/frameworks/native/services/surfaceflinger/SurfaceFlingerConsumer.cpp
status_t SurfaceFlingerConsumer::updateTexImage(BufferRejecter* rejecter,
        const DispSync& dispSync)
{
    ATRACE_CALL();
    ALOGV("updateTexImage");
    Mutex::Autolock lock(mMutex);

    if (mAbandoned) {
        ALOGE("updateTexImage: GLConsumer is abandoned!");
        return NO_INIT;
    }
...
    if (!SyncFeatures::getInstance().useNativeFenceSync()) {
        // Bind the new buffer to the GL texture.
        //
        // Older devices require the "implicit" synchronization provided
        // by glEGLImageTargetTexture2DOES, which this method calls.  Newer
        // devices will either call this in Layer::onDraw, or (if it's not
        // a GL-composited layer) not at all.
☆        err = bindTextureImageLocked();
    }

    return err;
}
↓
↓
↓
/frameworks/native/libs/gui/GLConsumer.cpp
status_t GLConsumer::bindTextureImageLocked() {
    if (mEglDisplay == EGL_NO_DISPLAY) {
        ALOGE("bindTextureImage: invalid display");
        return INVALID_OPERATION;
    }

    GLint error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        ST_LOGW("bindTextureImage: clearing GL error: %#04x", error);
    }

    glBindTexture(mTexTarget, mTexName);
    if (mCurrentTexture == BufferQueue::INVALID_BUFFER_SLOT &&
            mCurrentTextureImage == NULL) {
        ST_LOGE("bindTextureImage: no currently-bound texture");
        return NO_INIT;
    }

    status_t err = mCurrentTextureImage->createIfNeeded(mEglDisplay,
                                                        mCurrentCrop);
    if (err != NO_ERROR) {
        ST_LOGW("bindTextureImage: can't create image on display=%p slot=%d",
                mEglDisplay, mCurrentTexture);
        return UNKNOWN_ERROR;
    }
☆    mCurrentTextureImage->bindToTextureTarget(mTexTarget);

    // In the rare case that the display is terminated and then initialized
    // again, we can't detect that the display changed (it didn't), but the
    // image is invalid. In this case, repeat the exact same steps while
    // forcing the creation of a new image.
    if ((error = glGetError()) != GL_NO_ERROR) {
        glBindTexture(mTexTarget, mTexName);
        status_t err = mCurrentTextureImage->createIfNeeded(mEglDisplay,
                                                            mCurrentCrop,
                                                            true);
        if (err != NO_ERROR) {
            ST_LOGW("bindTextureImage: can't create image on display=%p slot=%d",
                    mEglDisplay, mCurrentTexture);
            return UNKNOWN_ERROR;
        }
☆        mCurrentTextureImage->bindToTextureTarget(mTexTarget);
        if ((error = glGetError()) != GL_NO_ERROR) {
            ST_LOGE("bindTextureImage: error binding external image: %#04x", error);
            return UNKNOWN_ERROR;
        }
    }

    // Wait for the new buffer to be ready.
    return doGLFenceWaitLocked();
}
↓
↓ 调用 OpenGL 的方法
↓
void GLConsumer::EglImage::bindToTextureTarget(uint32_t texTarget) {
    glEGLImageTargetTexture2DOES(texTarget, (GLeglImageOES)mEglImage);
}
```

### 发出`Refresh`信号`signalRefresh()`
将新的数据刷新到当前界面。
```
/frameworks/native/services/surfaceflinger/SurfaceFlinger.cpp
void SurfaceFlinger::signalRefresh() {
    mEventQueue.refresh();
}
↓
↓
↓
void MessageQueue::refresh() {
#if INVALIDATE_ON_VSYNC
☆    mHandler->dispatchRefresh();
#else
    mEvents->requestNextVsync();
#endif
}
↓
↓
↓
void MessageQueue::Handler::dispatchRefresh() {
    if ((android_atomic_or(eventMaskRefresh, &mEventMask) & eventMaskRefresh) == 0) {
        mQueue.mLooper->sendMessage(this, Message(MessageQueue::REFRESH));
    }
}
```
