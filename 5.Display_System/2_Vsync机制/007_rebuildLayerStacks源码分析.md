# 007_rebuildLayerStacks源码分析

标签（空格分隔）： Display Vsync

---

## `handleMessageRefresh()`
 1. 计算各个Layer的显示区域
 2. 合成显示
使用openGL，把各个Layer的可视区域在一个内存上描绘出来
或者 使用HWComposer合成
```
void SurfaceFlinger::handleMessageRefresh() {
    ATRACE_CALL();
    preComposition();
 ★   rebuildLayerStacks();
    setUpHWComposer();
    doDebugFlashRegions();
    doComposition();
    postComposition();
}


void SurfaceFlinger::rebuildLayerStacks() {
    // rebuild the visible layer list per screen
    if (CC_UNLIKELY(mVisibleRegionsDirty)) {
        ATRACE_CALL();
        mVisibleRegionsDirty = false;
        invalidateHwcGeometry();

        const LayerVector& layers(mDrawingState.layersSortedByZ);
        for (size_t dpy=0 ; dpy<mDisplays.size() ; dpy++) {
            Region opaqueRegion;
            Region dirtyRegion;
            Vector< sp<Layer> > layersSortedByZ;
            const sp<DisplayDevice>& hw(mDisplays[dpy]);
            const Transform& tr(hw->getTransform());
            const Rect bounds(hw->getBounds());
            if (hw->isDisplayOn()) {
★                SurfaceFlinger::computeVisibleRegions(layers,
                        hw->getLayerStack(), dirtyRegion, opaqueRegion);

                const size_t count = layers.size();
                for (size_t i=0 ; i<count ; i++) {
                    const sp<Layer>& layer(layers[i]);
                    const Layer::State& s(layer->getDrawingState());
                    if (s.layerStack == hw->getLayerStack()) {
                        Region drawRegion(tr.transform(
                                layer->visibleNonTransparentRegion));
                        drawRegion.andSelf(bounds);
                        if (!drawRegion.isEmpty()) {
                            layersSortedByZ.add(layer);
                        }
                    }
                }
            }
            hw->setVisibleLayersSortedByZ(layersSortedByZ);
            hw->undefinedRegion.set(bounds);
            hw->undefinedRegion.subtractSelf(tr.transform(opaqueRegion));
            hw->dirtyRegion.orSelf(dirtyRegion);
        }
    }
}
```
## `rebuildLayerStacks()`

 ### 对每个屏幕，构建可视Layer List
`computeVisibleRegions()`
* 从z轴最大（即最上面）的Layer开始计算
* z轴小的Layer会被它上面的Layer遮盖


> Layer可视区域是矩形，且可视区域要么是opaque(不透明),要么是translucent(半透明)

```
void SurfaceFlinger::computeVisibleRegions(
        const LayerVector& currentLayers, uint32_t layerStack,
        Region& outDirtyRegion, Region& outOpaqueRegion)
{
    ATRACE_CALL();

//当前层之上的不透明区域：opaque
    Region aboveOpaqueLayers;
//当前层之上的被遮盖的区域：opaque（实际就是上面的变量） + translucent 
//得到的就是当前层之上各层的可视区域
    Region aboveCoveredLayers;

    Region dirty;

    outDirtyRegion.clear();
//几层，从最上层开始处理
    size_t i = currentLayers.size();
    while (i--) {
        const sp<Layer>& layer = currentLayers[i];

        // start with the whole surface at its current location
        const Layer::State& s(layer->getDrawingState());

        // only consider the layers on the given layer stack
        if (s.layerStack != layerStack)//判断当前层是否需要显示
            continue;

        /*
         * opaqueRegion: area of a surface that is fully opaque.
         */
        Region opaqueRegion;//每循环一次，初始值为0

        /*
         * visibleRegion: area of a surface that is visible on screen
         * and not fully transparent. This is essentially the layer's
         * footprint minus the opaque regions above it.
         * Areas covered by a translucent surface are considered visible.
         */
        Region visibleRegion;//每循环一次，初始值为0

        /*
         * coveredRegion: area of a surface that is covered by all
         * visible regions above it (which includes the translucent areas).
         */
        Region coveredRegion;//每循环一次，初始值为0

        /*
         * transparentRegion: area of a surface that is hinted to be completely
         * transparent. This is only used to tell when the layer has no visible
         * non-transparent regions and can be removed from the layer list. It
         * does not affect the visibleRegion of this layer or any layers
         * beneath it. The hint may not be correct if apps don't respect the
         * SurfaceView restrictions (which, sadly, some don't).
         */
        Region transparentRegion;

★★★  具体处理各层_start
//得到当前Layer的原始可视区域、原始opaque区域
        // handle hidden surfaces by setting the visible region to empty
        if (CC_LIKELY(layer->isVisible())) {
            const bool translucent = !layer->isOpaque(s);
            Rect bounds(s.transform.transform(layer->computeBounds()));
            //设置可视区域边界（原始的，因此是还未经过处理的）
            visibleRegion.set(bounds);
            if (!visibleRegion.isEmpty()) {
                // Remove the transparent area from the visible region
                if (translucent) {
                    const Transform tr(s.transform);
                    if (tr.transformed()) {
                        if (tr.preserveRects()) {
                            // transform the transparent region
                            transparentRegion = tr.transform(s.activeTransparentRegion);
                        } else {
                            // transformation too complex, can't do the
                            // transparent region optimization.
                            transparentRegion.clear();
                        }
                    } else {
                        transparentRegion = s.activeTransparentRegion;
                    }
                }

                // compute the opaque region
                //计算不透明区域，值得注意的是每次循环只计算当前层
                const int32_t layerOrientation = s.transform.getOrientation();
                if (s.alpha==255 && !translucent &&
                        ((layerOrientation & Transform::ROT_INVALID) == false)) {
                    // the opaque region is the layer's footprint
                    opaqueRegion = visibleRegion;
                }
            }
        }
★★★  具体处理各层_end
        // Clip the covered region to the visible region
        //当前层被遮盖的区域（取交集）
        coveredRegion = aboveCoveredLayers.intersect(visibleRegion);

        // Update aboveCoveredLayers for next (lower) layer
        //更新可视区域（或操作），为下一个layer做准备
        aboveCoveredLayers.orSelf(visibleRegion);

        // subtract the opaque region covered by the layers above us
        //（与非操作）
        visibleRegion.subtractSelf(aboveOpaqueLayers);

        // compute this layer's dirty region
        //假设dirty就是可视区域
        if (layer->contentDirty) {
            // we need to invalidate the whole region
            dirty = visibleRegion;
            // as well, as the old visible region
            dirty.orSelf(layer->visibleRegion);
            layer->contentDirty = false;
        } else {
            /* compute the exposed region:
             *   the exposed region consists of two components:
             *   1) what's VISIBLE now and was COVERED before
             *   2) what's EXPOSED now less what was EXPOSED before
             *
             * note that (1) is conservative, we start with the whole
             * visible region but only keep what used to be covered by
             * something -- which mean it may have been exposed.
             *
             * (2) handles areas that were not covered by anything but got
             * exposed because of a resize.
             */
            const Region newExposed = visibleRegion - coveredRegion;
            const Region oldVisibleRegion = layer->visibleRegion;
            const Region oldCoveredRegion = layer->coveredRegion;
            const Region oldExposed = oldVisibleRegion - oldCoveredRegion;
            dirty = (visibleRegion&oldCoveredRegion) | (newExposed-oldExposed);
        }
        dirty.subtractSelf(aboveOpaqueLayers);

        // accumulate to the screen dirty region
        outDirtyRegion.orSelf(dirty);

        // Update aboveOpaqueLayers for next (lower) layer
        aboveOpaqueLayers.orSelf(opaqueRegion);//会给下一次循环使用

        // Store the visible region in screen space
        layer->setVisibleRegion(visibleRegion);
        layer->setCoveredRegion(coveredRegion);
        layer->setVisibleNonTransparentRegion(
                visibleRegion.subtract(transparentRegion));
    }

    outOpaqueRegion = aboveOpaqueLayers;
}

```

## 总结
* 计算各个Layer的可视区域、被遮盖区域，并记录在Layer中
* 汇总 dirty区域、不透明区域