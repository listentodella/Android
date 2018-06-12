# 006_基础知识_Region定义及操作

标签（空格分隔）： Display

---

## 几个概念
### 透光度
* opaque 完全不透明
* transparent 全透明
* translucent 半透明

用一个结构体`Region`描述这些区域，`Region`由一系列的矩形`Rect`构成
* 从上到下、从左到右排序
* ~~互不遮挡~~ ：各矩形的y轴坐标相同或无交叉
对于y轴坐标相同的矩形，x轴坐标无交叉

* `Span`:一组`Rect`，y轴一样，x坐标无交叉


### Region：由一组y轴无交叉的Span组成

`Vector<Rect> mStorage`;其最后一项表示边界
如果`Region`只有一个`Rect`，那么`mStorage`只含有1项，即只有一个`Region`
如果`Region`有2个`Rect`，那么`mStorage`含有3个`Rect`
因此`mStorage`永远不会有2个`Rect`

## 几个操作
### andSelf
r1.andSelf(r2)
得到共同部分
结果r1的值会受影响

### orSelf
r1.orSelf(r2)
得到合并部分（所有的）
结果r1的值会受影响

### xorSelf
异或：相同为0，相异为1
r1.xorSelf(r2)
得到共同部分以外的部分
结果r1的值会受影响

### intersect
交集，共同部分，结果r1的值不会受影响
r3 = r1.intersect(r2) = r1 and r2

### subtractSelf
与非操作
r1.subtractSelf(r2)
r1 = r1 - r2
从自身减去公共部分