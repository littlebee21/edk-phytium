版本V1.0  2022.3
第一个可用版本，只支持双路同时现实。

版本V1.1  2022.8.9
使用方法：
1.加载驱动，同时注册对应的non discovery设备，参考edk2-platform/Platform/Cherry/Drivers/PlatformDxe
2.注册时参数说明：
　参数0:DCDP控制器基址
　参数1:Phy控制器基址
　参数2:传递给驱动的share memory，用来传递参数
　　UINT32    Version
    UINT32    Dp Used bit mask : 0 - unused, 1 - used
    UINT32    Edp bit mask : 0 - not, 1 - yes
    UINT32    Init row resolution
    UINT32    Init column resolution
    UINT32    mode : 0 - main ram, 1 - vedio ram
    UINT32    Edp light value
    UINT32    Downspread bit mask : 0 - not use, 1 - use.
    edp相关参数为与预留参数
