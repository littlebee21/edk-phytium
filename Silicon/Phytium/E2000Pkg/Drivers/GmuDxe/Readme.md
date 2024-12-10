版本V1.0  2022.8.5
使用方法：
1.加载驱动，同时注册对应的non discovery设备，参考edk2-platform/Platform/Phytium/Cherry/Drivers/PlatformDxe驱动；
2.设备注册参数说明：
　参数0:MAC控制器基址;
  参数1:传递给驱动的share memory,用来传递参数
    UINT32    Version
    UINT32    Interface : 0-Usxgmii, 1-Xgmii, 2-Sgmii, 3-rgmii, 4-rmii
    UINT32    Autoeng : 0-not auto, 1-auto
    UINT32    Speed : 10-10M, 100-100M, 1000-1G, 2500-2.5G, 10000-10G
    UINT32    Duplex : 0-Half, 1-Full
    UINT8*6   Mac Address.
  该版本只支持sgmii，rgmii和rmii，其他为预留项，请根据实际硬件情况进行参数配置。
