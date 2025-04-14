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
const char* mqtt_topic_status = "home/status";
const char* mqtt_topic_availability = "home/availability"; // For device availability

// EEPROM addresses for storing relay states
#define EEPROM_SIZE 4
#define SWITCH_STATE_ADDR 0
#define LIGHT_STATE_ADDR 1
#define EEPROM_INITIALIZED_ADDR 2

// GPIO pins for relays
const int switchPin = D1;
const int lightPin = D2;

// Device state variables
bool switchState = false;
bool lightState = false;
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
    
    // Save default values to EEPROM
    EEPROM.write(SWITCH_STATE_ADDR, switchState ? 1 : 0);
    EEPROM.write(LIGHT_STATE_ADDR, lightState ? 1 : 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01); // Mark as initialized
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    // Read values from EEPROM
    switchState = EEPROM.read(SWITCH_STATE_ADDR) == 1;
    lightState = EEPROM.read(LIGHT_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM: Switch=" + String(switchState ? "ON" : "OFF") + 
                   ", Light=" + String(lightState ? "ON" : "OFF"));
  }
}

// Function to save states to EEPROM
void saveStatesToEEPROM() {
  EEPROM.write(SWITCH_STATE_ADDR, switchState ? 1 : 0);
  EEPROM.write(LIGHT_STATE_ADDR, lightState ? 1 : 0);
  EEPROM.commit();
  Serial.println("Saved states to EEPROM");
}

// Apply the current state to the relays
void applyStates() {
  // Assuming relays are active LOW, so we invert the logic
  digitalWrite(switchPin, switchState ? LOW : HIGH);
  digitalWrite(lightPin, lightState ? LOW : HIGH);
  
  Serial.println("Applied states: Switch=" + String(switchState ? "ON" : "OFF") + 
                 ", Light=" + String(lightState ? "ON" : "OFF"));
}

// Function to publish current states
void publishStates() {
  // Set flag to ignore the next MQTT message since it's our own publish
  // ignoreNextMqttMessage = true;
  
  // When relays are active LOW, we need to invert the state for the MQTT message
  // "ON" means the relay is energized (LOW), "OFF" means the relay is not energized (HIGH)
  client.publish(mqtt_topic_switch, switchState ? "ON" : "OFF", true);
  client.publish(mqtt_topic_light, lightState ? "ON" : "OFF", true);
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

  // Handle switch control
  if (String(topic) == mqtt_topic_switch) {
    bool newState = (message == "ON");
    if (switchState != newState) {
      switchState = newState;
      stateChanged = true;
    }
  }

  // Handle light control
  if (String(topic) == mqtt_topic_light) {
    bool newState = (message == "ON");
    if (lightState != newState) {
      lightState = newState;
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
  digitalWrite(switchPin, HIGH);
  digitalWrite(lightPin, HIGH);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
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