# 14_输入系统_InputStage_理论

标签（空格分隔）： Input InputStage

---

## 理论处理流程
* `activity` 先发给 `window`，如果 `window` 不能处理，由 `activity` 处理
* `window` 先发给 `DecorView`，如果 `DecorView` 不能处理，由 `window` 处理
* `DecorView` 发给输入焦点

但是实际情况是，并没有完全遵循此流程。

### 在输入法之前的处理：
`ViewPreImeInputStage`最终会调用到输入焦点的`onKeyPreIme`.因此如果想在输入法之前处理输入事件，可以重写`onKeyPreIme`。

### 在输入法之后的处理：
`ViewPostImeInputStage`最终会调用到输入焦点的三个方法：
* 使用`setOnKeyListener`注册的监听器的`onKey`
* `onKeyDown`
* `onKeyUp`
或者调用`activity`的`onKeyDown`或`onKeyUp`完成兜底工作，即处理最终无人处理的事件。




