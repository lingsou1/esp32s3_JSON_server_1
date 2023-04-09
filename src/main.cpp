/*
接线说明:无

程序说明:烧录进合宙的板子
        这是服务器端的程序,实现了服务器对客户端的请求信息中的JSON数据的解析
        本实例用于演示esp32S3的json数据通讯。
        操作测试本程序需要使用两台esp32s3开发板。其中一台为服务器端，一台为客户端。
        本程序为服务器程序，功能如下：
        1. 获取客户端请求信息中的json
        2. 解析json信息内容
        3. 将解析后的数据信息显示于串口监视器
        4. 利用json中的相关信息来控制服务器端开发板上LED的点亮和熄灭


注意事项:当该程序只运行一点时,有可能不是他自己的问题,可能是客户端出现问题了,导致服务器无数据可处理,不运行
        json数据官网:https://arduinojson.org/
        在这可以进行数据解析,将JSON格式的数据转化为代码,
        不需要手动书写代码,但是需要自己写出JSON数据

        本例中服务器收到的的JSON数据(不一定是这个,可能是一部分):
        {
          "info": {
            "name": "lingsou",
            "url": "www.bilibili.com",
            "email": "haoze20212021@outlook.com"
          },
          "digital_pin": {
            "digitPin": "digitPinValue",
            "BOOT" : "bootValue"
          },
          "analog_pin": {
            "analogPin": "analogPinValue",
            "capPin": "capPinValue"
          
          }
        }


函数示例:无

作者:灵首

时间:2023_4_5

*/

#include <Arduino.h>
#include <WiFiMulti.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

WiFiServer esp32s3_wifiServe(80);//实例化一个网页服务的对象
WiFiMulti wifi_multi;  //建立WiFiMulti 的对象,对象名称是 wifi_multi

//通过 ping ipconfig 方法实现找到以下设置参数
IPAddress local_IP(192, 168, 0, 123); // 设置ESP32s3-NodeMCU联网后的IP
IPAddress gateway(192, 168, 0, 1);    // 设置网关IP（通常网关IP是WiFI路由IP）
IPAddress subnet(255, 255, 255, 0);   // 设置子网掩码
IPAddress dns(192,168,0,1);           // 设置局域网DNS的IP（通常局域网DNS的IP是WiFI路由IP）

#define LED_A 10
#define LED_B 11

//定义函数
void wifi_multi_con(void);
void wifi_multi_init(void);
void runServer();
void parseData(WiFiClient client);



/*
# brief 连接WiFi的函数
# param 无
# retval 无
*/
void wifi_multi_con(void){
  int i=0;
  while(wifi_multi.run() != WL_CONNECTED){
    delay(1000);
    i++;
    Serial.print(i);
  }

}



/*
# brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
# param 无
# retval  无
*/
void wifi_multi_init(void){
  wifi_multi.addAP("LINGSOU1029","12345678");
  wifi_multi.addAP("haoze1029","12345678");   //通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}


/*
# brief  运行相关程序,实现对客户端的HTTP请求的解析,同时会对请求做出相关的响应
# param   无
# retval    无
*/
void runServer(){
  // 重点1：建立WiFiClient对象用于处理客户端请求信息
  WiFiClient incomingClient = esp32s3_wifiServe.available();
 
  // 如果没有客户端连接服务器，则“跳过”本函数中后续程序内容
  if (!incomingClient) {
    return;
  }
  
  Serial.println("====Client  Connected===\n");
  
  // 重点2：如果有客户端连接服务器，则尝试使用find跳过HTTP请求头
  if (incomingClient.find("\r\n\r\n")) {
    Serial.println("Found Header End. Start Parsing.\n");
  }
  
  // 解析请求体中的json信息,这是很重要的一步 
  parseData(incomingClient);
 
  // 建立服务器响应信息
  String httpResponse =
        "HTTP/1.0 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Type: text/plain;\r\n"
        "\r\n"
        "client_message_received";
 
  // 向客户端发送以上服务器响应信息
  incomingClient.print(httpResponse); 
 
  incomingClient.stop();  
  Serial.println("incomingClient stop\n");     
}

/*
# brief   解析出HTTP请求信息中的JSON数据
# param   WiFiClient client :建立一个待解析的对象
# retval    无
*/
void parseData(WiFiClient client){
  //建立动态内存实现解析数据
  const size_t capacity = JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(3) + 140;
  DynamicJsonDocument doc(capacity);

  //对接受的JSON数据进行对应的反序列化,并将输出存放在doc中待使用
  deserializeJson(doc, client);
  
  //解析info
  JsonObject info = doc["info"];
  if(info){
    //解析
    Serial.println("Server Json has info: true");
    const char* info_name = info["name"]; // "lingsou"
    const char* info_url = info["url"]; // "www.bilibili.com"
    const char* info_email = info["email"]; // "haoze20212021@outlook.com"

    //将指针字符转化为字符串
    String info_name_str =  info["name"].as<String>();
    String info_url_str = info["url"].as<String>();
    String info_emial_str = info["email"].as<String>();

    //串口输出
    Serial.print("info_name_str is :");
    Serial.print(info_name_str);
    Serial.print("\n");
    Serial.print("info_url_str is :");
    Serial.print(info_url_str);
    Serial.print("\n");
    Serial.print("info_emial_str is :");
    Serial.print(info_emial_str);
    Serial.print("\n");
    Serial.print("\n");
  } else {
    Serial.println("Server Json has info: false\n");
  }
  
  //解析digital_pin
  JsonObject digital_pin = doc["digital_pin"];
  if (digital_pin){
    //解析
    Serial.println("Server Json has digital_pin: true");
    const char* digitaPinValue = digital_pin["digitPin"];
    const char* BOOTValue = digital_pin["BOOT"]; 

    //将指针字符转化为整型变量
    int digitaPinValueInt =  digital_pin["digitPin"].as<int>();
    int BOOT_int = digital_pin["BOOT"].as<int>();

    //根据服务器端的BOOT按键状态控制客户端的LED灯的状态
    //实现通过客户端来控制服务器端的操作
    if(BOOT_int == 1){
      digitalWrite(LED_A,1);
      digitalWrite(LED_B,1);
    }
    else{
      digitalWrite(LED_A,0);
      digitalWrite(LED_B,0);
    }

    //串口输出
    Serial.print("digitaPinValueInt is :");
    Serial.print(digitaPinValueInt);
    Serial.print("\n");
    Serial.print("BOOT_int is :");
    Serial.print(BOOT_int);
    Serial.print("\n");
    Serial.print("\n");
  } else {
    Serial.println("Server Json has digital_pin: false\n");
  }
  
  //解析并通过串口输出analog_pin数据
  JsonObject analog_pin = doc["analog_pin"];
  if (analog_pin){
    //解析
    Serial.println("Server Json has analog_pin: true");
    const char* analogPinValue = digital_pin["analogPin"]; 
    const char* capPinValue = digital_pin["capPin"];

    //将指针字符转化为整型变量
    int analogPinValueInt =  analog_pin["analogPin"].as<int>();
    int capPinValueInt = analog_pin["capPin"].as<int>();

    //串口输出
    Serial.print("analogPinValueInt is :");
    Serial.print(analogPinValueInt);
    Serial.print("\n");
    Serial.print("capPinValueInt is :");
    Serial.print(capPinValueInt);
    Serial.print("\n");
    Serial.print("\n");
  } else {
    Serial.println("Server Json has analog_pin: false\n");
  }
}



void setup() {
  //连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //LED灯的设置
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_A,0);
  digitalWrite(LED_B,0);

  // 设置开发板网络环境
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Failed to Config ESP32s3 IP\n"); 
  } 

  //wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  //输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //开启网络服务器功能
  esp32s3_wifiServe.begin();
}



void loop(){
  // 运行服务器
  runServer();  
}