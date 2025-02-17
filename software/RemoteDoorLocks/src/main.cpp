#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Servo.h>

//巴法云服务器地址默认即可
#define TCP_SERVER_ADDR "bemfa.com"
//服务器端口，tcp创客云端口8344
#define TCP_SERVER_PORT "8344"
 
#define DEFAULT_STASSID  "CMCC-xxx"     //WIFI名称，区分大小写，不要写错
#define DEFAULT_STAPSW   "xxxxxxx"  //WIFI密码
String UID = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  //用户私钥，可在控制台获取,修改为自己的UID
String TOPIC =   "lock002";         //主题名字，可在控制台新建
#define MAX_PACKETSIZE 512
//设置心跳值30s
#define KEEPALIVEATIME 60*1000

const int green_led_pin = 14; // GPIO2
const int red_led_pin = 12;
const int blue_led_pin = 13;
 
//tcp客户端相关初始化，默认即可
WiFiClient TCPclient;
String TcpClient_Buff = "";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long preTCPStartTick = 0;//连接
bool preTCPConnected = false;
Servo my_servo;
 
//相关函数初始化
//连接WIFI
void doWiFiTick();
void startSTA();
 
//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);
void openRemoteLock();
 
 
 
/*
  *发送数据到TCP服务器
 */
void sendtoTCPServer(String p)
{
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    digitalWrite(red_led_pin, HIGH);
    digitalWrite(green_led_pin, HIGH);
    digitalWrite(blue_led_pin, LOW);
    return;
  }
  TCPclient.print(p);
  Serial.println(p);
  preHeartTick = millis();//心跳计时开始，需要每隔60秒发送一次数据
}
 
/*
  *初始化和服务器建立连接
*/
void startTCPClient()
{
  if(TCPclient.connect(TCP_SERVER_ADDR, atoi(TCP_SERVER_PORT)))
  {
    digitalWrite(green_led_pin, HIGH); // 连接到Wi-Fi后点亮板载LED
    digitalWrite(blue_led_pin, LOW);
    digitalWrite(red_led_pin, HIGH);

    Serial.print("\nConnected to server:");
    Serial.printf("%s:%d\r\n",TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT));
    
    String tcpTemp="";  //初始化字符串
    tcpTemp = "cmd=1&uid="+UID+"&topic="+TOPIC+"\r\n"; //构建订阅指令
    sendtoTCPServer(tcpTemp); //发送订阅指令
    tcpTemp="";//清空
     
    preTCPConnected = true;
    TCPclient.setNoDelay(true);
  }
  else
  {
    digitalWrite(green_led_pin, LOW); // 连接到Wi-Fi后点亮板载LED
    digitalWrite(blue_led_pin, LOW);
    digitalWrite(red_led_pin, LOW);

    Serial.print("Failed connected to server:");
    Serial.println(TCP_SERVER_ADDR);
    TCPclient.stop();
    preTCPConnected = false;
  }
  preTCPStartTick = millis();
 }
 
 
/*
  *检查数据，发送心跳
*/
void doTCPClientTick()
{
  //检查是否断开，断开后重连
  if(WiFi.status() != WL_CONNECTED) 
  {
    return;
  }
 
  if (!TCPclient.connected()) 
  {
    //断开重连
    if(preTCPConnected == true)
    {
     preTCPConnected = false;
     preTCPStartTick = millis();
     Serial.println();
     Serial.println("TCP Client disconnected.");
     TCPclient.stop();
   }
   else if(millis() - preTCPStartTick > 1*1000)//重新连接
   {
     startTCPClient();
   }
   }
   else
   {
     if (TCPclient.available()) 
     {
      //收数据
       char c =TCPclient.read();
       TcpClient_Buff +=c;
       TcpClient_BuffIndex++;
       TcpClient_preTick = millis();
       
       if(TcpClient_BuffIndex>=MAX_PACKETSIZE - 1)
       {
         TcpClient_BuffIndex = MAX_PACKETSIZE-2;
         TcpClient_preTick = TcpClient_preTick - 200;
       }
       preHeartTick = millis();
     }
     if(millis() - preHeartTick >= KEEPALIVEATIME)
     {//保持心跳
       preHeartTick = millis();
       Serial.println("--Keep alive:");
       sendtoTCPServer("cmd=0&msg=keep\r\n");
     }
   }
   if((TcpClient_Buff.length() >= 1) && (millis() - TcpClient_preTick>=200))
   {//data ready
     TCPclient.flush();
     Serial.print("Rev string: ");
     TcpClient_Buff.trim(); //去掉首位空格
     Serial.println(TcpClient_Buff); //打印接收到的消息
     String getTopic = "";
     String getMsg = "";
     if(TcpClient_Buff.length() > 15)
     {//注意TcpClient_Buff只是个字符串，在上面开头做了初始化 String TcpClient_Buff = "";
           //此时会收到推送的指令，指令大概为 cmd=2&uid=xxx&topic=light002&msg=off
           int topicIndex = TcpClient_Buff.indexOf("&topic=")+7; //c语言字符串查找，查找&topic=位置，并移动7位，不懂的可百度c语言字符串查找
           int msgIndex = TcpClient_Buff.indexOf("&msg=");//c语言字符串查找，查找&msg=位置
           getTopic = TcpClient_Buff.substring(topicIndex,msgIndex);//c语言字符串截取，截取到topic,不懂的可百度c语言字符串截取
           getMsg = TcpClient_Buff.substring(msgIndex+5);//c语言字符串截取，截取到消息
           Serial.print("topic:------");
           Serial.println(getTopic); //打印截取到的主题值
           Serial.print("msg:--------");
           Serial.println(getMsg);   //打印截取到的消息值
    }
    if(getMsg  == "on")
    {       
      openRemoteLock();
    }
    else if(getMsg == "off")
    { 
      openRemoteLock();
    }
    TcpClient_Buff="";
    TcpClient_BuffIndex = 0;
   }
 }
 
 void startSTA()
 {
   WiFi.disconnect();
   WiFi.mode(WIFI_STA);
   WiFi.begin(DEFAULT_STASSID, DEFAULT_STAPSW);
 }
 
 
 
 /**************************************************************************
                                  WIFI
 ***************************************************************************/
 /*
   WiFiTick
   检查是否需要初始化WiFi
   检查WiFi是否连接上，若连接成功启动TCP Client
   控制指示灯
 */
 void doWiFiTick()
 {
   static bool startSTAFlag = false;
   static bool taskStarted = false;
   static uint32_t lastWiFiCheckTick = 0;
 
   if (!startSTAFlag) 
   {
     startSTAFlag = true;
     startSTA();
     Serial.printf("Heap size:%d\r\n", ESP.getFreeHeap());
   }
 
   //未连接1s重连
   if ( WiFi.status() != WL_CONNECTED ) 
   {
    digitalWrite(green_led_pin, HIGH); // 连接到Wi-Fi后点亮板载LED
    digitalWrite(blue_led_pin, HIGH);
    digitalWrite(red_led_pin, LOW);

     if (millis() - lastWiFiCheckTick > 1000) 
     {
       lastWiFiCheckTick = millis();
     }
   }
   //连接成功建立
   else 
   {
      digitalWrite(green_led_pin, LOW); // 连接到Wi-Fi后点亮板载LED
      digitalWrite(blue_led_pin, HIGH);
      digitalWrite(red_led_pin, HIGH);
    
     if (taskStarted == false) 
     {
       taskStarted = true;
       Serial.print("\r\nGet IP Address: ");
       Serial.println(WiFi.localIP());
       startTCPClient();
     }
   }
 }

void openRemoteLock()
{
  my_servo.write(0);
  delay(2000);
  my_servo.write(90);
  delay(2500);
  my_servo.write(0);
}
 
 
 // 初始化，相当于main 函数
 void setup() 
 {
   Serial.begin(115200);
   Serial.println("Beginning...");
   pinMode(green_led_pin, OUTPUT); // 设置GPIO2为输出模式
   pinMode(red_led_pin, OUTPUT); // 设置GPIO2为输出模式
   pinMode(blue_led_pin, OUTPUT); // 设置GPIO2为输出模式
   my_servo.attach(5);
   my_servo.write(0);
 }
 
 //循环
 void loop()
 {
   doWiFiTick();
   doTCPClientTick();
 }
 