# 009_使用opengl进行界面合成流程速览

标签（空格分隔）： Display

---

## 如何合成
### 方案一
在Framebuffer上依次画出（使用opengl）每个app，然后把Framebuffer显示出来
### 方案二（HWComposer）
各个app写入不同的window，硬件合成后在LCD上直接显示


## 使用OpenGL

```
void SurfaceFlinger::handleMessageRefresh() {
    ATRACE_CALL();
    preComposition();
    rebuildLayerStacks();//计算dirty区域
    setUpHWComposer();//如果有硬件合成单元的话。。。
    doDebugFlashRegions();
    doComposition();//对于opengl，★
    postComposition();
}

```
* 绘画出每一个Layer
> 对于每个Layer，执行`layer->draw()`===>`drawWithOpenGL()`
* swapBuffer
```
void SurfaceFlinger::doComposition() {
    ATRACE_CALL();
    const bool repaintEverything = android_atomic_and(0, &mRepaintEverything);
    for (size_t dpy=0 ; dpy<mDisplays.size() ; dpy++) {
        const sp<DisplayDevice>& hw(mDisplays[dpy]);
        if (hw->isDisplayOn()) {
            // transform the dirty region into this screen's coordinate space
            const Region dirtyRegion(hw->getDirtyRegion(repaintEverything));

            // repaint the framebuffer (if needed)
★            doDisplayComposition(hw, dirtyRegion);//重绘buffer

            hw->dirtyRegion.clear();
            hw->flip(hw->swapRegion);
            hw->swapRegion.clear();
        }
        // inform the h/w that we're done compositing
        hw->compositionComplete();
    }
    postFramebuffer();
}


void SurfaceFlinger::doDisplayComposition(const sp<const DisplayDevice>& hw,
        const Region& inDirtyRegion)
{
    // We only need to actually compose the display if:
    // 1) It is being handled by hardware composer, which may need this to
    //    keep its virtual display state machine in sync, or
    // 2) There is work to be done (the dirty region isn't empty)
    bool isHwcDisplay = hw->getHwcDisplayId() >= 0;
    if (!isHwcDisplay && inDirtyRegion.isEmpty()) {
        return;
    }

    Region dirtyRegion(inDirtyRegion);

    // compute the invalid region
    hw->swapRegion.orSelf(dirtyRegion);

    uint32_t flags = hw->getFlags();
    if (flags & DisplayDevice::SWAP_RECTANGLE) {
        // we can redraw only what's dirty, but since SWAP_RECTANGLE only
        // takes a rectangle, we must make sure to update that whole
        // rectangle in that case
        dirtyRegion.set(hw->swapRegion.bounds());
    } else {
        if (flags & DisplayDevice::PARTIAL_UPDATES) {
            // We need to redraw the rectangle that will be updated
            // (pushed to the framebuffer).
            // This is needed because PARTIAL_UPDATES only takes one
            // rectangle instead of a region (see DisplayDevice::flip())
            dirtyRegion.set(hw->swapRegion.bounds());
        } else {
            // we need to redraw everything (the whole screen)
            dirtyRegion.set(hw->bounds());
            hw->swapRegion = dirtyRegion;
        }
    }

    if (CC_LIKELY(!mDaltonize && !mHasColorMatrix)) {
        if (!doComposeSurfaces(hw, dirtyRegion)) return;
    } else {
        RenderEngine& engine(getRenderEngine());
        mat4 colorMatrix = mColorMatrix;
        if (mDaltonize) {
            colorMatrix = colorMatrix * mDaltonizer();
        }
        engine.beginGroup(colorMatrix);
★        doComposeSurfaces(hw, dirtyRegion);
        engine.endGroup();
    }

    // update the swap region and clear the dirty region
    hw->swapRegion.orSelf(dirtyRegion);

    // swap buffers (presentation)
★    hw->swapBuffers(getHwComposer());//重绘完图层后，应该把绘制完的图层替换过来以显示
}


bool SurfaceFlinger::doComposeSurfaces(const sp<const DisplayDevice>& hw, const Region& dirty)
{
    RenderEngine& engine(getRenderEngine());
    const int32_t id = hw->getHwcDisplayId();
    HWComposer& hwc(getHwComposer());
    HWComposer::LayerListIterator cur = hwc.begin(id);
    const HWComposer::LayerListIterator end = hwc.end(id);

    bool hasGlesComposition = hwc.hasGlesComposition(id);
    if (hasGlesComposition) {
        if (!hw->makeCurrent(mEGLDisplay, mEGLContext)) {
            ALOGW("DisplayDevice::makeCurrent failed. Aborting surface composition for display %s",
                  hw->getDisplayName().string());
            eglMakeCurrent(mEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if(!getDefaultDisplayDevice()->makeCurrent(mEGLDisplay, mEGLContext)) {
              ALOGE("DisplayDevice::makeCurrent on default display failed. Aborting.");
            }
            return false;
        }

        // Never touch the framebuffer if we don't have any framebuffer layers
        const bool hasHwcComposition = hwc.hasHwcComposition(id);
        if (hasHwcComposition) {
            // when using overlays, we assume a fully transparent framebuffer
            // NOTE: we could reduce how much we need to clear, for instance
            // remove where there are opaque FB layers. however, on some
            // GPUs doing a "clean slate" clear might be more efficient.
            // We'll revisit later if needed.
            engine.clearWithColor(0, 0, 0, 0);
        } else {
            // we start with the whole screen area
            const Region bounds(hw->getBounds());

            // we remove the scissor part
            // we're left with the letterbox region
            // (common case is that letterbox ends-up being empty)
            const Region letterbox(bounds.subtract(hw->getScissor()));

            // compute the area to clear
            Region region(hw->undefinedRegion.merge(letterbox));

            // but limit it to the dirty region
            region.andSelf(dirty);

            // screen is already cleared here
★            if (!region.isEmpty()) {
                // can happen with SurfaceView
                drawWormhole(hw, region);//全黑，对于OpenGL，先清掉成全黑，效率会更高
            }
        }

        if (hw->getDisplayType() != DisplayDevice::DISPLAY_PRIMARY) {
            // just to be on the safe side, we don't set the
            // scissor on the main display. It should never be needed
            // anyways (though in theory it could since the API allows it).
            const Rect& bounds(hw->getBounds());
            const Rect& scissor(hw->getScissor());
            if (scissor != bounds) {
                // scissor doesn't match the screen's dimensions, so we
                // need to clear everything outside of it and enable
                // the GL scissor so we don't draw anything where we shouldn't

                // enable scissor for this frame
                const uint32_t height = hw->getHeight();
                engine.setScissor(scissor.left, height - scissor.bottom,
                        scissor.getWidth(), scissor.getHeight());
            }
        }
    }

    /*
     * and then, render the layers targeted at the framebuffer
     */

    const Vector< sp<Layer> >& layers(hw->getVisibleLayersSortedByZ());
    const size_t count = layers.size();
    const Transform& tr = hw->getTransform();
    if (cur != end) {
        // we're using h/w composer,如果是硬件合成
        for (size_t i=0 ; i<count && cur!=end ; ++i, ++cur) {
            const sp<Layer>& layer(layers[i]);
            const Region clip(dirty.intersect(tr.transform(layer->visibleRegion)));
            if (!clip.isEmpty()) {
                switch (cur->getCompositionType()) {
                    case HWC_CURSOR_OVERLAY:
                    case HWC_OVERLAY: {
                        const Layer::State& state(layer->getDrawingState());
                        if ((cur->getHints() & HWC_HINT_CLEAR_FB)
                                && i
                                && layer->isOpaque(state) && (state.alpha == 0xFF)
                                && hasGlesComposition) {
                            // never clear the very first layer since we're
                            // guaranteed the FB is already cleared
                            layer->clearWithOpenGL(hw, clip);
                        }
                        break;
                    }
                    case HWC_FRAMEBUFFER: {
                        layer->draw(hw, clip);
                        break;
                    }
                    case HWC_FRAMEBUFFER_TARGET: {
                        // this should not happen as the iterator shouldn't
                        // let us get there.
                        ALOGW("HWC_FRAMEBUFFER_TARGET found in hwc list (index=%zu)", i);
                        break;
                    }
                }
            }
            layer->setAcquireFence(hw, *cur);
        }
    } else {
        // we're not using h/w composer，软件合成
        for (size_t i=0 ; i<count ; ++i) {
            const sp<Layer>& layer(layers[i]);
            const Region clip(dirty.intersect(
                    tr.transform(layer->visibleRegion)));
            if (!clip.isEmpty()) {
★                layer->draw(hw, clip);//间接调用到Layer.cpp里的onDraw()
            }
        }
    }

    // disable scissor at the end of the frame
    engine.disableScissor();
    return true;
}

//Layer.cpp
void Layer::onDraw(const sp<const DisplayDevice>& hw, const Region& clip,
        bool useIdentityTransform) const
{
   。。。
★    drawWithOpenGL(hw, clip, useIdentityTransform);
    engine.disableTexturing();
}

void Layer::drawWithOpenGL(const sp<const DisplayDevice>& hw,
        const Region& /* clip */, bool useIdentityTransform) const {
    const uint32_t fbHeight = hw->getHeight();
    const State& s(getDrawingState());

    computeGeometry(hw, mMesh, useIdentityTransform);

    /*
     * NOTE: the way we compute the texture coordinates here produces
     * different results than when we take the HWC path -- in the later case
     * the "source crop" is rounded to texel boundaries.
     * This can produce significantly different results when the texture
     * is scaled by a large amount.
     *
     * The GL code below is more logical (imho), and the difference with
     * HWC is due to a limitation of the HWC API to integers -- a question
     * is suspend is whether we should ignore this problem or revert to
     * GL composition when a buffer scaling is applied (maybe with some
     * minimal value)? Or, we could make GL behave like HWC -- but this feel
     * like more of a hack.
     */
    const Rect win(computeBounds());

    float left   = float(win.left)   / float(s.active.w);
    float top    = float(win.top)    / float(s.active.h);
    float right  = float(win.right)  / float(s.active.w);
    float bottom = float(win.bottom) / float(s.active.h);

    // TODO: we probably want to generate the texture coords with the mesh
    // here we assume that we only have 4 vertices
    Mesh::VertexArray<vec2> texCoords(mMesh.getTexCoordArray<vec2>());
    texCoords[0] = vec2(left, 1.0f - top);
    texCoords[1] = vec2(left, 1.0f - bottom);
    texCoords[2] = vec2(right, 1.0f - bottom);
    texCoords[3] = vec2(right, 1.0f - top);

    RenderEngine& engine(mFlinger->getRenderEngine());
    engine.setupLayerBlending(mPremultipliedAlpha, isOpaque(s), s.alpha);
    engine.drawMesh(mMesh);
    engine.disableBlending();
}
```

```seq
doComposition->doDisplayComposition:
doDisplayComposition->doComposeSurfaces:
doComposeSurfaces->layer:draw(hw,clip)
Note right of layer:drawWithOpenGL()
doDisplayComposition-->swapBuffers:doComposeSurfaces合成完毕后
swapBuffers->eglSwapBuffers:DisplayDevice.cpp
eglSwapBuffers->swapBuffer:egl.cpp
Note right of swapBuffer:egl_window_surface_v2_t
```
![使用OpenGL合成的大致流程](%E4%BD%BF%E7%94%A8OpenGL%E5%90%88%E6%88%90%E7%9A%84%E5%A4%A7%E8%87%B4%E6%B5%81%E7%A8%8B.jpg)

最后的`egl_window_surface_v2_t::swapBuffers()`主要包含`queueBuffer`和`dequeueBuffer`
* `queueBuffer`：导致Gralloc HAL:post被调用，会提交给lcd驱动进行显示
* `dequeueBuffer`：得到新的Framebuffer，供下次使用