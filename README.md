# 项目介绍

IOT_RemoteDoorLock_Saturn是一个使用ESP8266 + 巴法云 + 小米米家平台搭建的一个可以用小爱同学远程操控的门锁。

# 硬件部分

IOT_RemoteDoorLock_Saturn的主控是一枚AiThink ESP8266 NodeMCU，它是一个ESP8266的最小系统，提供了完整的外围电路和GPIO。NodeMCU连接两枚PWM舵机，通过驱动舵机模拟人手拉门栓实现开门效果。NodeMCU还连接一枚RGB灯模组，通过它来显示当前系统的状态。

IOT_RemoteDoorLock_Saturn的PCB图如下：

![PCB图](/home/nyx/IOT_RemoteDoorLock_Saturn/hardware/PCB.jpg)

IOT_RemoteDoorLock_Saturn的实物运行图如下：

![实物运行图](/home/nyx/IOT_RemoteDoorLock_Saturn/hardware/Schematic.jpg)

# 软件部分

因为米家现在不允许个人开发者接入，所以这里通过商业物联网平台巴法云来接入米家。NodeMCU与巴法云之间通过MQTT通信，当用户通过米家或者小爱同学控制远程门锁时，巴法云将控制消息发送给NodeMCU，NodeMCU则控制舵机模拟人手开关门。

整体数据流程如下：

![数据流程](/home/nyx/Desktop/whiteboard_exported_image.png)