#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// WiFi credentials
const char* ssid = "Airtel_704B";
const char* password = "unseen@1";

// MQTT Broker settings
const char* mqtt_server = "a9de91952e404a93bc1b4be0175fd299.s1.eu.hivemq.cloud";
const char* mqtt_username = "rrdevices_RO_Plants";
const char* mqtt_password = "RRdevices@123";
const int mqtt_port = 8883;
const char* mqtt_client_id = "Homato";

// Topics for communication
const char* mqtt_topic_switch = "home/switch";
const char* mqtt_topic_light = "home/light";
const char* mqtt_topic_fan = "home/fan";
const char* mqtt_topic_tubelight = "home/tubelight";
const char* mqtt_topic_bedlight = "home/bedlight";
const char* mqtt_topic_falseceiling = "home/falseceiling";
const char* mqtt_topic_ac = "home/ac";
const char* mqtt_topic_switchport = "home/switchport";
const char* mqtt_topic_status = "home/status";
const char* mqtt_topic_availability = "home/availability"; // For device availability

// EEPROM addresses for storing relay states
#define EEPROM_SIZE 8
#define SWITCH_STATE_ADDR 0
#define LIGHT_STATE_ADDR 1
#define FAN_STATE_ADDR 2
#define TUBELIGHT_STATE_ADDR 3
#define BEDLIGHT_STATE_ADDR 4
#define FALSECEILING_STATE_ADDR 5
#define AC_STATE_ADDR 6
#define SWITCHPORT_STATE_ADDR 7
#define EEPROM_INITIALIZED_ADDR 8

// GPIO pins for relays
const int switchPin = D1;
const int lightPin = D2;
const int fanPin = D3;
const int tubelightPin = D4;
const int bedlightPin = D5;
const int falseceilingPin = D6;
const int acPin = D7;
const int switchportPin = D8;

// Device state variables
bool switchState = false;
bool lightState = false;
bool fanState = false;
bool tubelightState = false;
bool bedlightState = false;
bool falseceilingState = false;
bool acState = false;
bool switchportState = false;
// bool ignoreNextMqttMessage = false; // Flag to ignore self-published messages

// Initialize WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Function to connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi: " + String(ssid));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.println("IP address: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

// Function to read states from EEPROM
void readStatesFromEEPROM() {
  // Check if EEPROM has been initialized
  byte isInitialized = EEPROM.read(EEPROM_INITIALIZED_ADDR);
  
  if (isInitialized == 0xFF) {
    // EEPROM not initialized, set default values
    switchState = false;
    lightState = false;
    fanState = false;
    tubelightState = false;
    bedlightState = false;
    falseceilingState = false;
    acState = false;
    switchportState = false;
    
    // Save default values to EEPROM
    EEPROM.write(SWITCH_STATE_ADDR, switchState ? 1 : 0);
    EEPROM.write(LIGHT_STATE_ADDR, lightState ? 1 : 0);
    EEPROM.write(FAN_STATE_ADDR, fanState ? 1 : 0);
    EEPROM.write(TUBELIGHT_STATE_ADDR, tubelightState ? 1 : 0);
    EEPROM.write(BEDLIGHT_STATE_ADDR, bedlightState ? 1 : 0);
    EEPROM.write(FALSECEILING_STATE_ADDR, falseceilingState ? 1 : 0);
    EEPROM.write(AC_STATE_ADDR, acState ? 1 : 0);
    EEPROM.write(SWITCHPORT_STATE_ADDR, switchportState ? 1 : 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01); // Mark as initialized
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    // Read values from EEPROM
    switchState = EEPROM.read(SWITCH_STATE_ADDR) == 1;
    lightState = EEPROM.read(LIGHT_STATE_ADDR) == 1;
    fanState = EEPROM.read(FAN_STATE_ADDR) == 1;
    tubelightState = EEPROM.read(TUBELIGHT_STATE_ADDR) == 1;
    bedlightState = EEPROM.read(BEDLIGHT_STATE_ADDR) == 1;
    falseceilingState = EEPROM.read(FALSECEILING_STATE_ADDR) == 1;
    acState = EEPROM.read(AC_STATE_ADDR) == 1;
    switchportState = EEPROM.read(SWITCHPORT_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM");
    Serial.println("Switch State: " + String(switchState ? "ON" : "OFF"));
    Serial.println("Light State: " + String(lightState ? "ON" : "OFF"));
    Serial.println("Fan State: " + String(fanState ? "ON" : "OFF"));
    Serial.println("Tubelight State: " + String(tubelightState ? "ON" : "OFF"));
    Serial.println("Bedlight State: " + String(bedlightState ? "ON" : "OFF"));
    Serial.println("Falseceiling State: " + String(falseceilingState ? "ON" : "OFF"));
    Serial.println("AC State: " + String(acState ? "ON" : "OFF"));
    Serial.println("Switchport State: " + String(switchportState ? "ON" : "OFF"));
  }
}

// Function to save states to EEPROM
void saveStatesToEEPROM() {
  EEPROM.write(SWITCH_STATE_ADDR, switchState ? 1 : 0);
  EEPROM.write(LIGHT_STATE_ADDR, lightState ? 1 : 0);
  EEPROM.write(FAN_STATE_ADDR, fanState ? 1 : 0);
  EEPROM.write(TUBELIGHT_STATE_ADDR, tubelightState ? 1 : 0);
  EEPROM.write(BEDLIGHT_STATE_ADDR, bedlightState ? 1 : 0);
  EEPROM.write(FALSECEILING_STATE_ADDR, falseceilingState ? 1 : 0);
  EEPROM.write(AC_STATE_ADDR, acState ? 1 : 0);
  EEPROM.write(SWITCHPORT_STATE_ADDR, switchportState ? 1 : 0);
  EEPROM.commit();
  Serial.println("Saved states to EEPROM");
}

// Apply the current state to the relays
void applyStates() {
  // Assuming relays are active LOW, so we invert the logic
  digitalWrite(switchPin, switchState ? LOW : HIGH);
  digitalWrite(lightPin, lightState ? LOW : HIGH);
  digitalWrite(fanPin, fanState ? LOW : HIGH);
  digitalWrite(tubelightPin, tubelightState ? LOW : HIGH);
  digitalWrite(bedlightPin, bedlightState ? LOW : HIGH);
  digitalWrite(falseceilingPin, falseceilingState ? LOW : HIGH);
  digitalWrite(acPin, acState ? LOW : HIGH);
  digitalWrite(switchportPin, switchportState ? LOW : HIGH);
  Serial.println("Applied states to relays");
  Serial.println("Switch State: " + String(switchState ? "ON" : "OFF"));
  Serial.println("Light State: " + String(lightState ? "ON" : "OFF"));
  Serial.println("Fan State: " + String(fanState ? "ON" : "OFF"));
  Serial.println("Tubelight State: " + String(tubelightState ? "ON" : "OFF"));
  Serial.println("Bedlight State: " + String(bedlightState ? "ON" : "OFF"));
  Serial.println("Falseceiling State: " + String(falseceilingState ? "ON" : "OFF"));
  Serial.println("AC State: " + String(acState ? "ON" : "OFF"));
  Serial.println("Switchport State: " + String(switchportState ? "ON" : "OFF"));
}

// Function to publish current states
void publishStates() {
  // Set flag to ignore the next MQTT message since it's our own publish
  // ignoreNextMqttMessage = true;
  
  // When relays are active LOW, we need to invert the state for the MQTT message
  // "ON" means the relay is energized (LOW), "OFF" means the relay is not energized (HIGH)
  client.publish(mqtt_topic_switch, switchState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_light, lightState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_fan, fanState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_tubelight, tubelightState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_bedlight, bedlightState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_falseceiling, falseceilingState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_ac, acState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_switchport, switchportState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_status, "States updated", false);
  client.publish(mqtt_topic_availability, "online", true);
  
  Serial.println("Published states to MQTT");
}

// Callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // // Ignore message if flag is set (our own message)
  // if (ignoreNextMqttMessage) {
  //   ignoreNextMqttMessage = false;
  //   Serial.println("Ignoring self-published message");
  //   return;
  // }

  bool stateChanged = false;
  bool newState = (message == "ON");
  String topicStr = String(topic);

  // Use if-else chain for better performance than multiple independent if statements
  if (topicStr == mqtt_topic_switch) {
    if (switchState != newState) {
      switchState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_light) {
    if (lightState != newState) {
      lightState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_fan) {
    if (fanState != newState) {
      fanState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_tubelight) {
    if (tubelightState != newState) {
      tubelightState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_bedlight) {
    if (bedlightState != newState) {
      bedlightState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_falseceiling) {
    if (falseceilingState != newState) {
      falseceilingState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_ac) {
    if (acState != newState) {
      acState = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_switchport) {
    if (switchportState != newState) {
      switchportState = newState;
      stateChanged = true;
    }
  }

  // If state changed, update EEPROM and relays
  if (stateChanged) {
    applyStates();
    saveStatesToEEPROM();
    publishStates();
  }
}

// Function to reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Configure LWT (Last Will and Testament)
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password, 
                      mqtt_topic_availability, 1, true, "offline")) {
      Serial.println("connected");
      
      // Subscribe to control topics
      client.subscribe(mqtt_topic_switch);
      client.subscribe(mqtt_topic_light);
      client.subscribe(mqtt_topic_fan);
      client.subscribe(mqtt_topic_tubelight);
      client.subscribe(mqtt_topic_bedlight);
      client.subscribe(mqtt_topic_falseceiling);
      client.subscribe(mqtt_topic_ac);
      client.subscribe(mqtt_topic_switchport);
      
      // Announce that we're online and publish current states
      publishStates();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Set pins to HIGH (relay off) immediately
  pinMode(switchPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(tubelightPin, OUTPUT);
  pinMode(bedlightPin, OUTPUT);
  pinMode(falseceilingPin, OUTPUT);
  pinMode(acPin, OUTPUT);
  pinMode(switchportPin, OUTPUT);
  
  digitalWrite(switchPin, HIGH);
  digitalWrite(lightPin, HIGH);
  digitalWrite(fanPin, HIGH);
  digitalWrite(tubelightPin, HIGH);
  digitalWrite(bedlightPin, HIGH);
  digitalWrite(falseceilingPin, HIGH);
  digitalWrite(acPin, HIGH);
  digitalWrite(switchportPin, HIGH);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE + 1); // +1 for initialized flag
  
  // Read saved states from EEPROM
  readStatesFromEEPROM();
  
  // Apply saved states to relays
  applyStates();
  
  // Connect to WiFi
  setup_wifi();
  
  // Configure secure client
  espClient.setInsecure(); // For testing only
  // For production, use proper certificate validation:
  // espClient.setCACert(root_ca);
  
  // Set up MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // Set maximum packet size to support larger payloads if needed
  client.setBufferSize(512);
}

void loop() {
  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Attempting to reconnect...");
    setup_wifi();
  }

  // Check MQTT connection and reconnect if needed
  if (!client.connected()) {
    reconnect();
  }
  
  // Process MQTT messages
  client.loop();
  
  // Add a heartbeat message every 30 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > 30000) {
    if (client.connected()) {
      client.publish(mqtt_topic_availability, "online", true);
      Serial.println("Heartbeat sent");
    }
    lastHeartbeat = millis();
  }
}