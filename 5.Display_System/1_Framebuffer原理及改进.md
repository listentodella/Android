# 1.显示驱动的原理及改进

![Framebuffer原理](Framebuffer%E5%8E%9F%E7%90%86.png)
通过操作Framebuffer就可以控制到lcd的显示
## 只有一个Framebuffer的缺点
* 如果app写fb速度慢，则lcd图像变化也慢
* 如果app写fb速度不快不慢，闪烁
* app写fb速度快，一切ok
## 改进
### 使用多个fb
1. DisplayController使用fb0
2. app写fb1
3. DisplayController使用fb1
4. app写fb0
5. 如此反复。。。
### 硬件合成
> 然而实际使用过程中，绘制图像有很多时候有重复工作，比如状态栏、导航栏、背景等，通常将状态栏、导航栏作为一层，背景作为一层，图标作为一层，通过硬件Hardware Composer合并输出



![分层模型](%E5%88%86%E5%B1%82%E6%A8%A1%E5%9E%8B.png)

驱动支持hwc：
	每一层都对应一个驱动:/dev/fbx
	app操作某一层时，直接写对应的Framebuffer，硬件会自动合并它们
	因此ls /dev/graphics/ 会有很多fb


