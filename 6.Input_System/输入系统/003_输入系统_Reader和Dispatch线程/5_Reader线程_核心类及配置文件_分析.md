# 5_Reader线程_核心类及配置文件_分析

标签（空格分隔）： Input Reader

---

## 输入设备配置文件（idc文件）
输入设备配置文件由 USB 供应商、产品（及可选版本）ID 或输入设备名称定位。

按顺序查阅以下路径。
```
/odm/usr/idc/Vendor_XXXX_Product_XXXX_Version_XXXX.idc
/vendor/usr/idc/Vendor_XXXX_Product_XXXX_Version_XXXX.idc
/system/usr/idc/Vendor_XXXX_Product_XXXX_Version_XXXX.idc
/data/system/devices/idc/Vendor_XXXX_Product_XXXX_Version_XXXX.idc
/odm/usr/idc/Vendor_XXXX_Product_XXXX.idc
/vendor/usr/idc/Vendor_XXXX_Product_XXXX.idc
/system/usr/idc/Vendor_XXXX_Product_XXXX.idc
/data/system/devices/idc/Vendor_XXXX_Product_XXXX.idc
/odm/usr/idc/DEVICE_NAME.idc
/vendor/usr/idc/DEVICE_NAME.idc
/system/usr/idc/DEVICE_NAME.idc
/data/system/devices/idc/DEVICE_NAME.idc
```
当构建包含设备名称的文件路径时，设备名称中除“0-9”、“a-z”、“A-Z”、“-”或“_”之外的所有字符将替换为“_”。

代码通过`loadConfigurationLocked`来加载`.idc`文件
```
void EventHub::loadConfigurationLocked(Device* device) {
//对应 Deivce的成员String8 configurationFile
    device->configurationFile = getInputDeviceConfigurationFilePathByDeviceIdentifier(
            device->identifier, INPUT_DEVICE_CONFIGURATION_FILE_TYPE_CONFIGURATION);
    if (device->configurationFile.isEmpty()) {
        ALOGD("No input device configuration file found for device '%s'.",
                device->identifier.name.string());
    } else {
//对应Device的成员PropertyMap* configuration
        status_t status = PropertyMap::load(device->configurationFile,
                &device->configuration);
        if (status) {
            ALOGE("Error loading input device configuration file for device '%s'.  "
                    "Using default configuration.",
                    device->identifier.name.string());
        }
    }
}

```
## 配置文件解析流程
```seq
openDeviceLocked->open: 
open->ioctl:获得信息
ioctl->new Device():
new Device()->loadConfigurationLocked:加载idc
loadConfigurationLocked->加载kl/kcm:
```
![配置文件加载流程](%E9%85%8D%E7%BD%AE%E6%96%87%E4%BB%B6%E5%8A%A0%E8%BD%BD%E6%B5%81%E7%A8%8B.jpg)

### 解析kl文件
```
class KeyLayoutMap : public RefBase {
...
    KeyedVector<int32_t, Key> mKeysByScanCode;
    KeyedVector<int32_t, Key> mKeysByUsageCode;
...
};
Key {
...
    uint32_t keyCode;
    uint32_t flags;//这里主要用于区分电容屏等特殊输入设备
...
};

==>>>有以下对应关系

<linux_scancode, AKEYCODE_xxx>
例如 input_emulator.kl文件里，
<1, KEY> 对应为 1 -> (AKEYCODE_ESCAPE, 0)
<2, KEY> 对应为 2 -> (AKEYCODE_1, 0)
...
<227, KEY> 对应为 227 -> (AKEYCODE_STAR, 0)
<228, KEY> 对应为 227 -> (AKEYCODE_POUND, 0)
    
```
对于kl文件,还有一种`usage`的语法，对应着`mKeysByUsageCode`成员
```
key 1     ESCAPE
key 2     1
...
key 12    MINUS
key 13    EQUALS
...
★ key usage 0x0c006F          BRIGHTNESS_UP
```

### 解析kcm文件
```
class KeyCharacterMap : public RefBase {
...
★    KeyedVector<int32_t, Key*> mKeys;
    KeyedVector<int32_t, int32_t> mKeysByScanCode;
    KeyedVector<int32_t, int32_t> mKeysByUsageCode;
...
}
<scancode, keycode>
由此对应关系可知，kcm没有flags，因此对于电容屏等设备支持得不是很好
```
对于`kcm`文件,还有另外的写法：
```
map key 1 ESCAPE 对应于 <1, AKEYCODE_ESCAPE>
map key usage ... 对应于usage的mKeysByUsageCode
```
* 一般不在`kcm`文件中放`keylayout`

```
key A {
    label:                              'A'
    base:                               'a'
    shift, capslock:                    'A'
}
===>
<AKEYCODE_A, KEY>
.label = 'A'
对于.base, 会生成一个Behavior:metaState = 0, character = 'a';
对于.shift, capslock,会各生成个Behavior：
metaState = shift, character = 'A';
metaState = capslock, character = 'A';

.firstBehavior将三者链接在一起，会指向.base生成的Behavior,.base生成的Behavior又会指向shift、shift的又指向capslock的
```

> 当输入键a按下的时候，就可以通过Device找到KeyMap，再找到keyCharacterMap，再找到KeyVector<int32_t, Key*>,根据keycode值找到KEY值，根据KEY值找到firstBehavior，再根据里面的metaState，得到对应的值

```
key SPACE {
    label:                              ' '
    base:                               ' '
    alt, meta:                          fallback SEARCH
    ctrl:                               fallback LANGUAGE_SWITCH
}
<AKEYCODE_SPACE, Key>
.label = ' '
.firstBehavior:
    .base -> Behavior:metaState = 0, character = ' ';
    .alt -> Behavior:metaState = alt, character = fallback_keycode = AKEYCODE_SEARCH;
    .meta -> Behavior:metaState = meta, character = fallback_keycode = AKEYCODE_SEARCH;
    .ctrl -> Behavior:metaState = ctrl, character = fallback = LANGUAGE_SWITCH;
```
> fallback:对于Android应用程序，如果能够处理上报来的keycode，就处理，如果不能处理，输入系统会再上报一个值，就是fallback，给它处理。
