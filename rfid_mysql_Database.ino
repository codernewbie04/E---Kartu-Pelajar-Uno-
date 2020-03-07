#include <ArduinoJson.h>
#include<MFRC522.h>
#include <Ethernet.h>
#include <SPI.h>
#define SS_PIN 4 //FOR RFID SS PIN BECASUSE WE ARE USING BOTH ETHERNET SHIELD AND RS-522
#define RST_PIN 9

EthernetClient client;
MFRC522 rfid(SS_PIN,RST_PIN);

const char* server = "ipaddress";
String rfid_key = "";
String resource = "";


const unsigned long HTTP_TIMEOUT = 10000;  // max respone time from server
const size_t MAX_CONTENT_SIZE = 512;       // max size of the HTTP response

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// The type of data that we want to extract from the page
struct clientData {
  bool status;
  String nama;
  String kelas;
  char msg[8];
};

// ARDUINO entry point #1: runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  while (!Serial) {
    ;  // wait for serial port to initialize
  }
  Serial.println("Serial ready");
  if(!Ethernet.begin(mac)) {
    Serial.println("Failed to configure Ethernet");
    return;
  }
  Serial.println("Ethernet ready");
  delay(1000);
}

// ARDUINO entry point #2: runs over and over again forever
void loop() {
  resource = "";
  rfid_key = "";
  
  if(!rfid.PICC_IsNewCardPresent())
    return;
    if(!rfid.PICC_ReadCardSerial())
    return;
    Serial.print("UID:");
    for (byte i = 0; i < rfid.uid.size; i++) {
      rfid_key += rfid.uid.uidByte[i] < 0x10 ? "0" : "";
      rfid_key += rfid.uid.uidByte[i], HEX;
    }
    Serial.println(rfid_key);
    resource = "/share/rfid/getrfid?accessToken=Tftywau1yrkPGrq58ZMoDqROoBCX8SwW54zjWxbc&rfid_key="+rfid_key;
    Serial.println(); 
  if(connect(server)) {
    if(sendRequest(server, resource) && skipResponseHeaders()) {
        Serial.println("Data terkirim");
    }
  }
  disconnect();
    rfid.PICC_HaltA();
  
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
  wait();
  
}

// Open connection to the HTTP server
bool connect(const char* hostName) {
  Serial.print("Connect to ");
  Serial.println(hostName);
  bool ok = client.connect(hostName, 80);

  Serial.println(ok ? "Connected" : "Connection Failed!");
  return ok;
}

// Send the HTTP GET request to the server
bool sendRequest(const char* host, String resource) {
  Serial.print("GET ");
  Serial.println(resource);

  client.print("GET ");
  client.print(resource);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(host);
  client.println("Connection: close");
  client.println();

  return true;
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders() {
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout(HTTP_TIMEOUT);
  bool ok = client.find(endOfHeaders);

  if (!ok) {
    Serial.println("No response or invalid response!");
  }
  return ok;
}

bool readReponseContent(struct clientData* clientData) {
  const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(8) + 260;
  DynamicJsonDocument doc(capacity);

  deserializeJson(doc, client);


  // Here were copy the strings we're interested in using to your struct data
  strcpy(clientData->msg, doc["msg"]);
  bool st = doc["status"]; // true
  clientData->status = st;
  JsonObject data = doc["data"];
  String nm = data["nama"];
  String kls = data["kelas"]; 
  clientData->nama = nm;
  clientData->kelas = kls;
  if(clientData->status){
      Serial.println('Nama : ');
      Serial.println(clientData->nama );
      Serial.println('Kelas : ');
      Serial.println(clientData->kelas );
  }
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string

  return true;
}

// Print the data extracted from the JSON
void printclientData(const struct clientData* clientData) {
  Serial.print("Status = ");
  Serial.println(clientData->status);
  Serial.print("msg = ");
  Serial.println(clientData->msg);
}

// Close the connection with the HTTP server
void disconnect() {
  Serial.println("Disconnect");
  client.stop();
}

// Pause for a 1 minute
void wait() {
  Serial.println("Wait 1 seconds");
  delay(1000);
}
