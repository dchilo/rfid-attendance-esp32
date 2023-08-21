#include <WiFi.h>
#include <WebServer.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <MFRC522.h>

// Wi-Fi credentials
const char* ssdi = "";
const char* password = "";

// Create an instance of the web server
WebServer server(80);

// Database API
String apiURL = "http://192.168.1.2:4000/api/";
String apiURLAttendance = "http://192.168.1.2:4000/api/attendances";

// Define the SPI and MFRC522 pins
#define RST_PIN 0
#define SS_PIN 5

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Define LED pin
#define LED_PIN 26

// Variables
bool isTakingAttendance = false;
String course = "";
String token = "";

void setup() {
  // Start serial communication
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssdi, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  IPAddress localIP = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(localIP);

  // Initialize the SPI bus
  SPI.begin();

  // Initialize MFRC522
  mfrc522.PCD_Init();

  // Handle root URL '/'
  server.on("/", handleRoot);

  //Handle API request '/addcode'
  server.on("/addcode", HTTP_POST, handleAddCode);

  // Handle API Taking Assitance
  server.on("/attendance", HTTP_POST, handleTakeAttendance);

  // Start the server
  server.begin();
  Serial.println("Server started");
  
}

void loop() {
  // Listen for incoming client requests
  server.handleClient();

  // Idle
  digitalWrite(LED_PIN, HIGH);

  // See if is taking assistance
  if (isTakingAttendance == true){
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Mostrar UID de la tarjeta

      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      Serial.print("UID de la tarjeta: ");
      Serial.println(uid);
      Serial.println(course);

      String codeRfid = uid;
      sendAttendanceToDatabase(course, codeRfid);

      mfrc522.PICC_HaltA();     // Detener la comunicación con la tarjeta
    }
  } 
}

void handleRoot() {
  // Return a simple HTML
  String html = "<h1>Server on</h1>";

  server.send(200, "text/html", html);
}

void handleTakeAttendance() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  // Check if there's data in the request
  if (server.hasArg("courseId") && server.hasArg("isTaking")){
    String courseId = server.arg("courseId");
    String takingValue = server.arg("isTaking");
    if (takingValue == "true"){
      isTakingAttendance = true;
    } else if(takingValue == "false"){
      isTakingAttendance = false;
    }
    Serial.println(isTakingAttendance);
    course = courseId;

    // Create a JSON response
    StaticJsonDocument<200> jsonResponse;
    jsonResponse["status"] = "success";

    // Serialize the JSON response to a string
    String responseStr;
    serializeJson(jsonResponse, responseStr);

    // Set the content type to JSON
    server.sendHeader("Content-Type", "application/json");
    // Set the HTTP status code to 200 (OK)
    server.send(200, "application/json", responseStr);
  } else {
    // Create a JSON response
    StaticJsonDocument<200> jsonResponse;
    jsonResponse["status"] = "error";
    jsonResponse["message"] = "Not enougth params";

    // Serialize the JSON response to a string
    String responseStr;
    serializeJson(jsonResponse, responseStr);

    // Set the content type to JSON
    server.sendHeader("Content-Type", "application/json");
    // Set the HTTP status code to 400 (Bad Request)
    server.send(400, "application/json", responseStr);
  }
}

void handleAddCode() {
  // CORS
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,PUT,DELETE,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
  // Check if there's data in the request
  if (server.hasArg("id") && server.hasArg("token")){
    String id = server.arg("id");
    String token = server.arg("token");
    Serial.println(id);
    
    String uid = "";
    // Read the RFID code
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      // Mostrar UID de la tarjeta
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      
      Serial.print("UID de la tarjeta: ");
      Serial.println(uid);
      mfrc522.PICC_HaltA();     // Detener la comunicación con la tarjeta
    }

    if(uid != ""){
      digitalWrite(LED_PIN, LOW);
    }

    String rfidCode = uid;
    Serial.println(uid);

    // Send API reques to database with student data and RFID code
    sendStudentToDatabase(id, token, rfidCode);

    // Create a JSON response
    StaticJsonDocument<200> jsonResponse;
    jsonResponse["status"] = "success";
    jsonResponse["message"] = "Student Code Added";

    // Serialize the JSON response to a string
    String responseStr;
    serializeJson(jsonResponse, responseStr);

    // Set the content type to JSON
    server.sendHeader("Content-Type", "application/json");
    // Set the HTTP status code to 200 (OK)
    server.send(200, "application/json", responseStr);
  } else {
    // Create a JSON response
    StaticJsonDocument<200> jsonResponse;
    jsonResponse["status"] = "error";
    jsonResponse["message"] = "Not enougth params";

    // Serialize the JSON response to a string
    String responseStr;
    serializeJson(jsonResponse, responseStr);

    // Set the content type to JSON
    server.sendHeader("Content-Type", "application/json");
    // Set the HTTP status code to 400 (Bad Request)
    server.send(400, "application/json", responseStr);
  }
}

void sendStudentToDatabase(const String& id, const String& token, const String& rfidCode) {
  Serial.println(id);
  Serial.println(token);
  // Create a JSON object for the API reques
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["codeRfid"] = rfidCode;

  // Serialize the JSON object to a string
  String jsonStr;
  serializeJson(jsonDoc, jsonStr);

  String apiURLAddCode = apiURL + "students/" + id;
  // Send the API request to the database
  HTTPClient http;
  http.begin(apiURLAddCode);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-access-token", token);
  int httpResponseCode = http.PUT(jsonStr);
  if (httpResponseCode > 0) {
    Serial.printf("API response code %d\n", httpResponseCode);
    String response = http.getString();
    digitalWrite(LED_PIN, LOW);
    Serial.println(response);
  } else {
    Serial.printf("Error sending API request. Error code: %d\n", httpResponseCode);
  }
  http.end();
}

void sendAttendanceToDatabase(const String& course, const String& codeRfid) {
  Serial.println(course);
  Serial.println(codeRfid);

  // Create a JSON object for the API reques
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["course"] = course;
  jsonDoc["codeRfid"] = codeRfid;

  // Serialize the JSON object to a string
  String jsonStr;
  serializeJson(jsonDoc, jsonStr);

  HTTPClient http;
  http.begin(apiURLAttendance);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(jsonStr);
  if (httpResponseCode > 0) {
    Serial.printf("API response code %d\n", httpResponseCode);
    String response = http.getString();
    digitalWrite(LED_PIN, LOW);
    Serial.println(response);
  } else {
    Serial.printf("Error sending API request. Error code: %d\n", httpResponseCode);
  }
  http.end();

}
