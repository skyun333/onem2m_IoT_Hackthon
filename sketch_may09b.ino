#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define IR D7 //아두이노 적외선 센서 데이터 받아올 연결 핀번호
WiFiServer server(80);

int ir;
int is_Obstacle=0;
String response;
const char *ssid = "skyun"; // 연결할 와이파이 이름
const char *password = "13131313"; // 연결할 와이파이 비밀번호
const char *host = "219.240.212.226";
int Speaker = 12; // GPIO13
int c = 262;
int d = 294;
int e = 330;
int f = 349;
int g = 392;
int a = 440;
int b = 494;
int bf = 466;
int C = 523;
int frequency[56] = {
  c, d, f, d, a, a, g, c, d, f, d, g, g, f, e, d, c, d, f, d, f, g, e, d, c, c, g, f,
  c, d, f, d, a, a, g, c, d, f, d, C, e, f, e, d, c, d, f, d, f, g, e, d, c, c, g, f};
int duration[56] = {
  120, 120, 120, 120, 360, 360, 720, 120, 120, 120, 120, 360, 360, 480, 120, 240, 120, 120, 120, 120,
  480, 240, 360, 120, 480, 240, 480, 960,
  120, 120, 120, 120, 360, 360, 720, 120, 120, 120, 120, 360, 360, 480, 120, 240, 120, 120, 120, 120,
  480, 240, 360, 120, 480, 240, 480, 960};

void rickroll(){ 
  for(int i=0; i<56; i++){ 
    tone(Speaker, frequency[i]); 
    delay(duration[i]);
    noTone(Speaker);
    delay(10);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(IR,INPUT);
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);
  delay(1000);
  WiFi.mode(WIFI_STA);
  
  WiFi.begin(ssid, password);
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}



void loop() {
  
  ir=digitalRead(IR);
  HTTPClient http; 
  if(ir==1){
    if(is_Obstacle!=0){ // 장애물이 있다가 치워졌을 때 is_Obstacle 0으로 세팅 & con값 다시 off로 세팅
      is_Obstacle=0;
      http.begin("http://219.240.212.226:7579/Mobius/ae_detectSensor/sensorData"); // path
      http.addHeader("X-M2M-RI", "12345");
      http.addHeader("X-M2M-Origin", "S");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+json;ty=4");
      // header 정의
    
      String postdata="{\"m2m:cin\":{\"con\":\"no_obstacles\"}}";
      //body 정의
    
      int httpCode = http.POST(postdata);   //Send the request
      //Mobius 서버에 POST 요청
    
      //String payload = http.getString();    //Get the response payload
      Serial.println(httpCode);
      //Serial.println(payload);
      
      http.end();
      
      http.begin("http://219.240.212.226:7579/Mobius/ae_warningSpeaker/warning"); // path
      http.addHeader("X-M2M-RI", "12345");
      http.addHeader("X-M2M-Origin", "S");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+json;ty=4");

      postdata="{\"m2m:cin\":{\"con\":\"off\"}}";
      httpCode = http.POST(postdata); 
      Serial.println(httpCode);
      http.end();
    }
    
  }
  if(ir==0){ // 장애물이 존재하는 상태
    if(is_Obstacle<20){ // 장애물이 없다가 등장시
      is_Obstacle++;
    }
    
    else if(is_Obstacle==20){ // 장애물 일정 시간 감지 시 -> Mobius 서버로 장애물 여부 전송
      is_Obstacle=21;
      http.begin("http://219.240.212.226:7579/Mobius/ae_detectSensor/sensorData"); // path
      http.addHeader("X-M2M-RI", "12345");
      http.addHeader("X-M2M-Origin", "S");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+json;ty=4");
      // header 정의
    
      String postdata="{\"m2m:cin\":{\"con\":\"obstacle_detected!\"}}";
      //body 정의
    
      int httpCode = http.POST(postdata);   //Send the request
      //Mobius 서버에 POST 요청
    
      //String payload = http.getString();    //Get the response payload
      Serial.println(httpCode);
      http.end();
      http.begin("http://219.240.212.226:7579/Mobius/ae_warningSpeaker/warning"); // path
      http.addHeader("X-M2M-RI", "12345");
      http.addHeader("X-M2M-Origin", "S");
      http.addHeader("Content-Type", "application/vnd.onem2m-res+json;ty=4");
      postdata="{\"m2m:cin\":{\"con\":\"on\"}}";
      httpCode = http.POST(postdata); 
      Serial.println(httpCode);
      http.end();
    }    
  }
  WiFiClient client = server.available();
  if(client){    
    StaticJsonBuffer<400> JSONBuffer;
    for(int i=0;i<9;i++){
      String im=client.readStringUntil('\r');
      //Serial.println(response);      
    }
    for(int i=0;i<46;i++){
      String im=client.readStringUntil('"');
      //Serial.println(response);
      response=im;     
    }
    Serial.println(response);      
  }
  if(response=="on"){
      rickroll();
  }  
  delay(500);  //0.5초마다 감지 반복
}
