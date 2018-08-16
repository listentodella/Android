# 13_输入系统_补充知识_activity_window_decor_view关系_理论

标签（空格分隔）： Input

---
一个教室有多个活动房，
每个活动房有一个窗口，
通过窗口观察里面的活动，

> 一个教室有多个活动房，每个活动房有一个窗口，装修窗口时先选定样式，然后在指定区域贴窗花

----------
一个`application`有一个或多个`activity`，`activity`是一组相对独立的功能；
每个`activity`有一个界面，界面显示在窗口上，每个`activity`含有一个`window`；
窗口是毛坯的，如何装修？
* 选择样式
* 在指定区域，自由创作：比如贴窗花

怎么给`activity`编写`window`?
* 选择样式 ：`decor`（装饰）：全屏、有标题栏的、无标题栏的...
* 在`decor`指定区域，自由创作：放各种窗花`view`：Button、Textview、choicebox...

android里:
1个`application`, 有1个或多个`activity`(一组相对独立的功能)
1个`activity`, 有1个`window`(毛坯的)
1个`window`, 有1个`decor`(先选定样式)
1个`decor`, 有多个`viewgroup/layout`,而`viewgroup/layout`中, 有多个`view`(贴窗花)


----------
* 状态栏/导航栏是由`SystemUI`进程描绘的
* app的`activity`只是预留出状态栏/导航栏的位置
* `activity`中的`view`是树状结构

![view_树状结构](view_%E6%A0%91%E7%8A%B6%E7%BB%93%E6%9E%84.jpg)

> 在 decor的 指定区域 放置各种 view
> decor 是 DecorView 类 ；指定区域是 FrameLayout类；view是View的派生类（Button、Textview等等）

所以一个界面(`window`)是由2个应用程序描绘的


