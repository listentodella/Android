# 005_SurfaceFlinger对Vsync的处理

标签（空格分隔）： Display Vsync

---

 1. 多显示器Display
 add 、remove
 2. 多个surface,每个surface有多个Layer
 内容更新、add/remove、属性变化（比如大小变化）、界面角度变化等等

## 几个重要的成员
通过比较各自的State的对象`mDrawingState`、`mCurrentState`里的关键成员，来判断上面的变化
* SurfaceFlinger
```
SurfaceFlinger.h
★ struct State {
    LayerVector layersSortedByZ;//比较它，就可以知道Layer的增加、减少
    DefaultKeyedVector< wp<IBinder>,        DisplayDeviceState> displays;//比较它，就可以知道Display的增加、减少
};
↓
可以理解为
struct SurfaceFlinger::State {
    ...
}

★ State mDrawingState;//正在/上次使用的状态
★ State mCurrentState;//当前的/被修改的状态
```
* Layer
```
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
* 处理各种事务
* 处理各`Layer`的Buffer更换
* 发出`Refresh`信号，最终导致`handleMessageRefresh()`被调用
```
void SurfaceFlinger::onMessageReceived(int32_t what) {
    ATRACE_CALL();
    switch (what) {
...
    case MessageQueue::INVALIDATE:
        handleMessageTransaction();
        handleMessageInvalidate();
        signalRefresh();
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

### 发出`Refresh`信号`signalRefresh()`