#include <Firebase_ESP_Client.h>
#include <ESP8266WiFi.h>
#include <AccelStepper.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <addons/TokenHelper.h>


#define HALFSTEP 8  //Half-step mode (8 step control signal sequence)
#define mtrPin1  D1     // IN1 on the ULN2003 driver 1
#define mtrPin2  D2    // IN2 on the ULN2003 driver 1
#define mtrPin3  D3     // IN3 on the ULN2003 driver 1
#define mtrPin4  D4     // IN4 on the ULN2003 driver 1
AccelStepper stepper1(HALFSTEP, mtrPin1, mtrPin3, mtrPin2, mtrPin4);


#define API_KEY "<YOUR API KEY>"
#define FIREBASE_PROJECT_ID "monkey-repellent-device"
#define USER_EMAIL "user@gmail.com"
#define USER_PASSWORD "123456"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WiFiServer server(80);
WiFiManager wifiManager;

// INTERRUPTS
const int buttonPin = D6;
void IRAM_ATTR IntCallback(){
  Serial.print("After Clicked Pin: ");
  Serial.print(digitalRead(buttonPin));
  wifiManager.resetSettings();
  delay(2500);
  ESP.reset(); 
}



void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(D7, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), IntCallback, FALLING);

  // WIFI MANAGER
  wifiManager.autoConnect("Monkey Repellent - 0001");
  wifiManager.setCustomHeadElement("<center>MRD - 0001</center>");
  wifiManager.setCustomMenuHTML("<style>body {background-color: powderblue;}</style>");
  Serial.println("Connected.");
  server.begin();

  // FIREBASE
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // STEPPER
  stepper1.setMaxSpeed(1000.0);
  stepper1.setAcceleration(100.0);  //Make the acc quick
  stepper1.setSpeed(300);
}




void loop() {
  delay(1000);
  String path = "users/7wDpVqY1i41GwpCkfp2O";
  Serial.print("Pin: ");
  Serial.print(digitalRead(buttonPin));

  if (Firebase.Firestore.getDocument(&fbdo, FIREBASE_PROJECT_ID, "", path.c_str(), "")) {
    FirebaseJson payload;
    payload.setJsonData(fbdo.payload().c_str());
    FirebaseJsonData jsonData;
    FirebaseJsonData jsonData2;
    
    payload.get(jsonData, "fields/LED_STATUS/booleanValue", true);
    payload.get(jsonData2, "fields/STEPPER_ANGLE/stringValue", true);
    Serial.println(jsonData2.stringValue);

    bool DEVICE_STATUS = jsonData.boolValue;
    int DEVICE_ANGLE = jsonData2.stringValue.toInt();
    
    Serial.println(DEVICE_STATUS);
    Serial.println(DEVICE_ANGLE);

    if (DEVICE_STATUS == 1){
      digitalWrite(D7, HIGH);
      stepper1.runToNewPosition(DEVICE_ANGLE*(4096/360));
    }
    else{
      digitalWrite(D7, LOW);
      delay(5000);
    }
  }
}
