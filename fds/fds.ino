#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>

#include "certs.h"

#include "DHT.h"
#include "secrets.h"

#define DHTPIN D4
#define DHTTYPE DHT11

#define ECHOPIN D6
#define TRIGPIN D5
#define SOUND_VELOCITY 0.034
#define CM_TO_INCH 0.393701

#define WATER_LEVEL D7
#define WATER_FLOW D8

DHT dht(DHTPIN, DHTTYPE);

long duration;
float distanceCm;
float distanceInch;
int water_level_status;
float l_hour;

const int httpsPort = 443;
const char *host = "sagarbarai.com";

//const char *fingerprint[] PROGMEM = "   ";

volatile int flow_frequency;
unsigned long currentTime;
unsigned long cloopTime;

//varaibles for sending post request every 10 seconds
unsigned long xTime, eTime; //temporary x and elaspsedTime, couldn't think of better variable names

float h = 0, t = 0, hic = 0;

X509List cert(cert_);

ICACHE_RAM_ATTR void flow() {
  flow_frequency++;
}

void captureFlowRate() {

  currentTime = millis();

  if (currentTime >= (cloopTime + 1000)) {

    if (flow_frequency != 0) {
      // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
      l_hour = (flow_frequency * 60 / 7.5);  // (Pulse frequency x 60 min) / 7.5Q = flowrate in L/hour

    } else {
      l_hour = 0;
    }

    flow_frequency = 0;
  }
}


//_431a8a8b89dedd160856e3d8cdc0f89a.fds.sagarbarai.com.
//_f641a1eb9e43ae930ee59e607c384575.nbnhgqgzdr.acm-validations.aws.

void publishData() {
  /*
  POST /naradamuni/listen HTTP/1.1\r\n
  Host: fds.sagarbarai.com\r\n
  User-Agnet: FDS-ESP8266\r\n
  Content-Type: application/json\r\n
  Content-Length: <LENGTH>\r\n
  Connection: close\r\n
  \r\n
  <PAYLOAD_GOES_HERE>\r\n
  */

  /*
  Payload
  {
    "fds_dev_id": "tce-01",
    "fds_dev_loc": "Pune-India",
    "t" : "t",
    "h" : "h",
    "hic" : "hic",
    "l_hour": "l_hour",
    "d_cm" : "distanceCm",
    "w_level" : "water_level_status"
  }
  */

  String payload = "{\"fds_dev_id\":\"TCE-01\",\"fds_dev_loc\":\"Pune-India\",\"t\":\"" + String(t) + "\",\"h\":\"" + String(h) + "\",\"hic\":\"" + String(hic) + "\",\"l_hour\":\"" + String(l_hour) + "\",\"d_cm\":\"" + String(distanceCm) + "\",\"w_level\":\"" + String(water_level_status) + "\"}";

  int length = payload.length();
  String response;

  WiFiClientSecure client;  //Declare object of class WiFiClient

  client.setTrustAnchors(&cert);

  if(!client.connect(fds_host,fds_port)){
    Serial.println("Connection to FDS server failed !");
    return;
  }

  String requestPayload = String(
    String("POST /naradamuni/listen HTTP/1.1\r\n") +
    "Host: " + fds_host + "\r\n" +
    "user-agent: FDS-ESP8266\r\n" +
    "content-type: application/json\r\n" +
    "x-api-key: " + API_KEY + "\r\n" +
    "content-length: " + String(length) + "\r\n\r\n" +
    payload + "\r\n\r\n" 
  );
  
  Serial.print(requestPayload);

  client.print(requestPayload);

  while(client.available()){
    response = client.readStringUntil('\n');
    Serial.println(response);
  }
  
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("Device Started");

  pinMode(ECHOPIN, INPUT);
  pinMode(TRIGPIN, OUTPUT);

  pinMode(WATER_LEVEL, INPUT);
  pinMode(WATER_FLOW, INPUT);

  digitalWrite(WATER_FLOW, HIGH);
  digitalWrite(WATER_LEVEL, HIGH);

  attachInterrupt(digitalPinToInterrupt(WATER_FLOW), flow, RISING);

  currentTime = millis();
  cloopTime = currentTime;
  xTime = currentTime;

  water_level_status = 1;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWifi Connected");

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  dht.begin();
}


void loop() {
  // put your main code here, to run repeatedly:
  delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!

  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  /*if (isnan(h) || isnan(t) ) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }*/

  // Compute heat index in Celsius (isFahreheit = false)
  hic = dht.computeHeatIndex(t, h, false);

  //calculate distance of objects
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);

  duration = pulseIn(ECHOPIN, HIGH);

  distanceCm = duration * SOUND_VELOCITY / 2;

  captureFlowRate();
  water_level_status = digitalRead(WATER_LEVEL);


  Serial.print("Tempreature : ");
  Serial.println(t);

  Serial.print("Humidity : ");
  Serial.println(h);

  Serial.print("Heat Index : ");
  Serial.println(hic);

  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  Serial.print("Water Level : ");
  Serial.println(water_level_status);

  Serial.print("Flow Rate L/H : ");
  Serial.println(l_hour);

  eTime=millis();

  if((eTime - xTime) > 10000){
    publishData();
    xTime=eTime;
  }
}
