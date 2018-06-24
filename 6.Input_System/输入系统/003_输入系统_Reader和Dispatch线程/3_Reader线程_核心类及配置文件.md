# 3_Reader线程_核心类及配置文件

标签（空格分隔）： Input Reader

---

## Reader线程核心类
`mEventHub`,含有成员`mDevices`,它是一个`vector`,表示了多个输入设备。`mDevice <编号 Devices*>`,编号代表第几个输入设备，`Devices`表示输入设备本身。
```
class EventHub : public EventHubInterface
{
public:
...
    int32_t mNextDeviceId;
    //分别表示<编号，输入设备>
    KeyedVector<int32_t, Device*> mDevices;//用来记录多个输入设备
    Device *mOpeningDevices;
    Device *mClosingDevices;
...
private:
    struct Device {
        ...
    };
}
```
`Device`，含有`fd`，一些信息`identify`，含有设备的`name`,`bus`,`vid`,`pid`。另外含有一些映射信息。
```
struct Device {
    Device* next;
    // = open("/dev/input/event*")
    int fd; // may be -1 if device is virtual
...
    const InputDeviceIdentifier identifier;
    uint8_t keyBitmask[(KEY_MAX + 1) / 8];
...
    String8 configurationFile;
    PropertyMap* configuration;
    VirtualKeyMap* virtualKeyMap;
    KeyMap keyMap;
...
};

struct InputDeviceIdentifier {
    inline InputDeviceIdentifier() :
            bus(0), vendor(0), product(0), version(0) {
    }

    // Information provided by the kernel.可以通过ioctl获得
    String8 name;
    String8 location;
    String8 uniqueId;
    uint16_t bus;
    uint16_t vendor;
    uint16_t product;
    uint16_t version;
...
};


class KeyMap {
public:
    String8 keyLayoutFile;
    sp<KeyLayoutMap> keyLayoutMap;//来自kl
    String8 keyCharacterMapFile;
    sp<KeyCharacterMap> keyCharacterMap;//来自kcm
...
};

```
## 三种配置文件：
* `idc`:input device conf
* `kl`:keylayout
* `kcm`:key character map
例如，对于同一个按键，linux内核输入驱动上报`EV_KEY`事件，对于`KEY_1`，它的`code`为`KEY_1 (2)`(通过宏定义)，对于Android输入系统则为`AKEYCODE_1 (8)`，如果想要对应上，则需要通过`.kl`文件。为什么Android不直接使用内核的键值呢？因为如果直接使用，万一内核的键值被修改，则Android也得重新编译。如果是通过`.kl`文件转换，就可以通过修改`.kl`文件即可。

### 按键布局文件(`.kl`文件)
按键布局文件（`.kl`文件）将Linux按键代码和坐标轴代码映射到Android按键代码和坐标轴代码，并指定相关的策略标记。

* 对具有按键（包括音量、电源和耳机媒体按键等特殊按键）的内部（内置）输入设备而言是必要文件
* 对其他输入设备而言是可选文件，而对特殊用途的键盘和操纵杆而言则是推荐文件
如果没有可用的设备专属按键布局文件，则系统将改选默认文件。

按键布局文件由 USB供应商、产品（可能还包括版本）ID或输入设备名称来确定位置。系统会按顺序查阅以下路径：
```
/odm/usr/keylayout/Vendor_XXXX_Product_XXXX_Version_XXXX.kl
/vendor/usr/keylayout/Vendor_XXXX_Product_XXXX_Version_XXXX.kl
/system/usr/keylayout/Vendor_XXXX_Product_XXXX_Version_XXXX.kl
/data/system/devices/keylayout/Vendor_XXXX_Product_XXXX_Version_XXXX.kl
/odm/usr/keylayout/Vendor_XXXX_Product_XXXX.kl
/vendor/usr/keylayout/Vendor_XXXX_Product_XXXX.kl
/system/usr/keylayout/Vendor_XXXX_Product_XXXX.kl
/data/system/devices/keylayout/Vendor_XXXX_Product_XXXX.kl
/odm/usr/keylayout/DEVICE_NAME.kl
/vendor/usr/keylayout/DEVICE_NAME.kl
/system/usr/keylayout/DEVICE_NAME.kl
/data/system/devices/keylayout/DEVICE_NAME.kl
/odm/usr/keylayout/Generic.kl
/vendor/usr/keylayout/Generic.kl
/system/usr/keylayout/Generic.kl
/data/system/devices/keylayout/Generic.kl
```
当构建包含设备名称的文件路径时，设备名称中除“0-9”、“a-z”、“A-Z”、“-”或“_”之外的所有字符将替换为“_”。
系统提供了一个特殊的内置常规按键布局文件，名为`Generic.kl`。此按键布局旨在支持各种标准外部键盘和操纵杆。请勿修改常规按键布局！一般可以修改`/data/system/devices/keylayout`下的。

#### keylayout 文件格式
按键声明包含关键字 `key`（后跟一个`Linux按键代码编号`和`Android按键代码名称`），或该关键字的用途（后跟 `HID 用途`和 `Android按键代码名`称）。HID用途由32位整数表示，其中高16位表示HID用途页面，低16 位表示 HID 用途 ID。任何一项声明都可以后跟一组由空格分隔的可选策略标记。
> kl文件里的映射关系，只是描述了linux内核scancode 如何转换成 Android的keycode，只是表示按下了那个键，至于该键对应哪个字符，由kcm文件决定
```
key 内核code值  Android code值（AKEYCODE_W）
key     17              W
```
### 实验
* 创建
`/data/system/devices/keylayout/input_emulator.kl`
* 添加内容
```
key 227 STAR //这里227是一个未经使用的值，STAR代表 * 号
key 228 POUND//同上，代表 # 号
```
* 修改权限
```
(busybox) chmod 777 /data/system/devices -R
```

* 重启后挂载nfs
```
//如果路径不存在可以尝试手动创建先
busybox mount -t nfs -o nolock,vers=2 192.168.xxx.xxx:/work/nfs_root /data/nfs

# insmod input_emulator.ko
# 发送 * 键 
sendevent /dev/input/event5 1 227 1 //按下
sendevent /dev/input/event5 1 227 0 //松开
sendevent /dev/input/event5 0 0 0 //同步事件

# 发送 # 键
sendevent /dev/input/event5 1 228 1 
sendevent /dev/input/event5 1 228 0 
sendevent /dev/input/event5 0 0 0
```

### 按键字符映射文件（kcm文件）
按键字符映射文件（`.kcm`文件）负责将 Android 按键代码与修饰符的组合映射到 Unicode 字符。

按键字符映射文件可以通过 USB 供应商、产品（和可选版本）ID 或输入设备名称进行定位。
按上述顺序查阅以下路径。
```
/odm/usr/keychars/Vendor_XXXX_Product_XXXX_Version_XXXX.kcm
/vendor/usr/keychars/Vendor_XXXX_Product_XXXX_Version_XXXX.kcm
/system/usr/keychars/Vendor_XXXX_Product_XXXX_Version_XXXX.kcm
/data/system/devices/keychars/Vendor_XXXX_Product_XXXX_Version_XXXX.kcm
/odm/usr/keychars/Vendor_XXXX_Product_XXXX.kcm
/vendor/usr/keychars/Vendor_XXXX_Product_XXXX.kcm
/system/usr/keychars/Vendor_XXXX_Product_XXXX.kcm
/data/system/devices/keychars/Vendor_XXXX_Product_XXXX.kcm
/odm/usr/keychars/DEVICE_NAME.kcm
/vendor/usr/keychars/DEVICE_NAME.kcm
/system/usr/keychars/DEVICE_NAME.kcm
/data/system/devices/keychars/DEVICE_NAME.kcm
/odm/usr/keychars/Generic.kcm
/vendor/usr/keychars/Generic.kcm
/system/usr/keychars/Generic.kcm
/data/system/devices/keychars/Generic.kcm
/odm/usr/keychars/Virtual.kcm
/vendor/usr/keychars/Virtual.kcm
/system/usr/keychars/Virtual.kcm
/data/system/devices/keychars/Virtual.kcm
```
当构建包含设备名称的文件路径时，设备名称中除“0-9”、“a-z”、“A-Z”、“-”或“_”之外的所有字符都会被替换为“_”。

通用按键字符映射文件,系统会提供一个名为`Generic.kcm`的特殊内置按键字符映射文件。此按键字符映射旨在支持各种标准外部键盘。

#### kcm文件格式
按键声明包括关键字 key，后跟 Android 按键代码名称、左大括号、一组属性和行为以及一个右大括号。
```
key A {                     # A 表示 AKEYCODE_A
    label:              'A' # 印在按键上的文字
    base:               'a' # 如果没有其他按键（shift，ctrl等）同时按下，此时对应的字符为 'a'
    shift, capslock:    'A' # 如果有shift等，则为 'A'
    ctrl, alt, meta:    none
}
```

#### 实验
* 创建
```
mkdir -p /data/system/devices/keychars
cp /system/usr/keychars/Generic.kcm /data/system/devices/keychars/input_emulator.kcm
```
* 修改文件内容
```
key STAR {
    label:   '*'
    #base:    '*'
    base:    '1'
}

key POUND {
    label:   '#'
    #base:    '#'
    base:    '2'
}
```
* 修改权限
```
busybox chmod 777 /data/system/devices -R
```
* 重启后挂载nfs
```
//如果路径不存在可以尝试手动创建先
busybox mount -t nfs -o nolock,vers=2 192.168.xxx.xxx:/work/nfs_root /data/nfs

# insmod input_emulator.ko
# 发送 * 键 ,但得到的是 1
sendevent /dev/input/event5 1 227 1 //按下
sendevent /dev/input/event5 1 227 0 //松开
sendevent /dev/input/event5 0 0 0 //同步事件

# 发送 # 键，但得到的是 2
sendevent /dev/input/event5 1 228 1 
sendevent /dev/input/event5 1 228 0 
sendevent /dev/input/event5 0 0 0
```

## Note!!!
> keylayout:只是用来表示驱动上报的scancode对应哪一个android按键(AKEYCODE_x),只是表示按键被按下,它对应哪一个字符，由kcm文件决定
> kcm: 用来表示android按键(AKEYCODE_x)对应哪一个字符,表示同时按下其他按键后，对应哪个字符

* 如果不想修改kcm、kl文件，也可以使用组合键（基于常用的pc键盘的对应关系）
```
sendevent /dev/input/event5 1 42 1  // shift 按下
sendevent /dev/input/event5 1 9 1   // 数字8按下
sendevent /dev/input/event5 1 9 0   //数字8松开
sendevent /dev/input/event5 1 42 0  // shift松开
sendevent /dev/input/event5 0 0 0   //同步事件， 可以得到组合值 *

sendevent /dev/input/event5 1 42 1 
sendevent /dev/input/event5 1 4 1 
sendevent /dev/input/event5 1 4 0
sendevent /dev/input/event5 1 42 0 
sendevent /dev/input/event5 0 0 0   //同步事件， 可以得到组合值 #  
```