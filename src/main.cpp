#define BLYNK_TEMPLATE_ID "TMPL66cE_TBdD"
#define BLYNK_TEMPLATE_NAME "Air Quality Monitoring"
#define BLYNK_AUTH_TOKEN "ZXi5DWCvZqnDNsRlHerDuZJYJbTIkUbm"

#include <Arduino.h>
//thư viện sensor dòng DHT
#include <DHT.h>
#include <Adafruit_Sensor.h>
//
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
//thư viện màn hình 
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <string>
#include <Fonts/FreeSerif12pt7b.h>//font chữ

#define WIFI_SSID "Zo Zo 2.4G"//tên wifi
#define WIFI_PASSWORD "20101978"//mật khẩu wifi7
#define DHTTYPE DHT11//khai báo loại cảm biến nhiệt độ độ ẩm
//khai báo các chân kết nối
#define DHTPIN 2
#define MQPIN 35
#define TFT_CS 15
#define TFT_RESET 4
#define TFT_A0 16
#define TFT_SDA 23
#define TFT_SCK 18
#define FLAME_SENSOR 22
#define LEDPIN 17
#define BUZZER 27
#define BUTTON 34

char auth[] = BLYNK_AUTH_TOKEN;
unsigned long dataMillis = 0;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_A0, TFT_RESET);
WidgetLED LED_on_App(V4);
void connectWiFi()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}
//hàm in chuỗi ký tự ra màn hình
void printText(String text, uint16_t color, int x, int y, const GFXfont *f, int textsize)
{
  tft.setCursor(x, y);//x là khoảng cách tới lề, y là khoảng cách đến hàng trên cùng, đơn vị pixel 
  tft.setFont(f);//cài đăt font chữ
  tft.setTextColor(color);//màu chữ
  tft.setTextSize(textsize);//cỡ chữ
  tft.print(text);//in chuỗi ký tự
}

int flag = 0;
void UpdateDataBlynk(float t, float h, int x)
{
  Blynk.virtualWrite(V0, t);//V0 là chân ảo khai báo trên app Blynk, dữ liệu hiển thị được lấy từ V0=t
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, x);
  
}
void Notification(float t, int x){
int flame = digitalRead(FLAME_SENSOR);
  if ((t > 60.00 || x > 1000 || flame == 0||digitalRead(BUTTON)) && flag == 0)//nếu ít nhất một giá trị cảm biến vượt ngưỡng thì đèn báo sáng, còi kêu và Blynk gửi cảnh báo đến điện thoại
  {
    Serial.println("Fire in the House");
    Blynk.logEvent("fire_alert", "Fire Detected");//gửi cảnh báo trên Blynk
    digitalWrite(LEDPIN, HIGH);
    digitalWrite(BUZZER, HIGH);
    LED_on_App.on();
    flag = 1;
    delay(500);
  }
  else if (digitalRead(BUTTON))
  {
    digitalWrite(LEDPIN, LOW);
    digitalWrite(BUZZER, LOW);
    LED_on_App.off();
    flag = 0;
    delay(500);
  }
  BLYNK_WRITE(V3);
}
void print_LCD(float t, float h, int x)
{
  tft.fillScreen(ST7735_BLACK);//tô toàn màn hình màu đen
  printText("Temp: ", ST7735_RED, 5, 25, &FreeSerif12pt7b, 1);
  tft.print(t);
  tft.print(char(128));
  tft.print("C");
  printText("Humi: ", ST7735_BLUE, 5, 65, &FreeSerif12pt7b, 1);
  tft.print(h);
  tft.print("%");
  printText("Gas : ", ST7735_GREEN, 5, 105, &FreeSerif12pt7b, 1);
  tft.print(x);
  tft.print("ppm");
}
BLYNK_WRITE(V3){
  int p=param.asInt();
  if(p&&flag==0){
    Serial.println("Fire in the House");
    Blynk.logEvent("fire_alert", "Fire Detected");//gửi cảnh báo trên Blynk
    digitalWrite(LEDPIN, HIGH);
    digitalWrite(BUZZER, HIGH);
    LED_on_App.on();
    flag = 1;
  }
  else if(p==0){
     digitalWrite(LEDPIN, LOW);
    digitalWrite(BUZZER, LOW);
    LED_on_App.off();
    flag = 0;
  }
}
void setup()
{
  Serial.begin(115200);
  dht.begin();//khởi tạo cảm biến
   connectWiFi();
  //cài mode cho các chân
  pinMode(MQPIN, INPUT);
  pinMode(FLAME_SENSOR, INPUT);
  pinMode(BUTTON,INPUT);
  pinMode(LEDPIN, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  Blynk.begin(auth, WIFI_SSID, WIFI_PASSWORD);//khởi tạo Blynk
  tft.initR(INITR_BLACKTAB);//khởi tạo màn hình
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1);//quay màn hình 90 độ
}
void loop()
{
  Blynk.run();
  float h = dht.readHumidity();//đọc giá trị độ ẩm
  float t = dht.readTemperature();//đọc giá trị nhiệt độ
  int x = analogRead(MQPIN);//đọc giá trị cảm biến gas
  if (isnan(h) || isnan(t))
  {
    Serial.println(F("False to read from DHT sensor!"));
    return;
  }

  Notification(t,x);
  unsigned long currentMillis=millis();
  if(currentMillis-dataMillis>3000){
    print_LCD(t, h, x);
  UpdateDataBlynk(t,h,x);
  dataMillis=currentMillis;
  } 
}
