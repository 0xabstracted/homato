#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "in-000001";

// WiFi credentials
const char* ssid = "Airtel_704B";
const char* password = "unseen@1";

// MQTT Broker settings
const char* mqtt_server = "a9de91952e404a93bc1b4be0175fd299.s1.eu.hivemq.cloud";
const char* mqtt_username = "rrdevices_RO_Plants";
const char* mqtt_password = "RRdevices@123";
const int mqtt_port = 8883;
char mqtt_client_id[50]; // Will be set based on DEVICE_ID

// Topics for communication
char mqtt_topic_relay1[50];
char mqtt_topic_relay2[50];
char mqtt_topic_relay3[50];
char mqtt_topic_relay4[50];
char mqtt_topic_relay5[50];
char mqtt_topic_relay6[50];
char mqtt_topic_relay7[50];
char mqtt_topic_relay8[50];
char mqtt_topic_status[50];
char mqtt_topic_availability[50];

// EEPROM addresses for storing relay states
#define EEPROM_SIZE 8
#define RELAY_PIN_1_STATE_ADDR 0
#define RELAY_PIN_2_STATE_ADDR 1
#define RELAY_PIN_3_STATE_ADDR 2
#define RELAY_PIN_4_STATE_ADDR 3
#define RELAY_PIN_5_STATE_ADDR 4
#define RELAY_PIN_6_STATE_ADDR 5
#define RELAY_PIN_7_STATE_ADDR 6
#define RELAY_PIN_8_STATE_ADDR 7
#define EEPROM_INITIALIZED_ADDR 8

// define the GPIO connected with Relays and switches
#define RelayPin1 23  //D23
#define RelayPin2 22  //D22
#define RelayPin3 21  //D21
#define RelayPin4 19  //D19
#define RelayPin5 18  //D18
#define RelayPin6 5   //D5
#define RelayPin7 25  //D25
#define RelayPin8 26  //D26

// Device state variables
bool Relay1State = false;
bool Relay2State = false;
bool Relay3State = false;
bool Relay4State = false;
bool Relay5State = false;
bool Relay6State = false;
bool Relay7State = false;
bool Relay8State = false;

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
    Relay1State = false;
    Relay2State = false;
    Relay3State = false;
    Relay4State = false;
    Relay5State = false;
    Relay6State = false;
    Relay7State = false;
    Relay8State = false;
    
    // Save default values to EEPROM
    EEPROM.write(RELAY_PIN_1_STATE_ADDR, Relay1State ? 1 : 0);
    EEPROM.write(RELAY_PIN_2_STATE_ADDR, Relay2State ? 1 : 0);
    EEPROM.write(RELAY_PIN_3_STATE_ADDR, Relay3State ? 1 : 0);
    EEPROM.write(RELAY_PIN_4_STATE_ADDR, Relay4State ? 1 : 0);
    EEPROM.write(RELAY_PIN_5_STATE_ADDR, Relay5State ? 1 : 0);
    EEPROM.write(RELAY_PIN_6_STATE_ADDR, Relay6State ? 1 : 0);
    EEPROM.write(RELAY_PIN_7_STATE_ADDR, Relay7State ? 1 : 0);
    EEPROM.write(RELAY_PIN_8_STATE_ADDR, Relay8State ? 1 : 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01); // Mark as initialized
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    // Read values from EEPROM
    Relay1State = EEPROM.read(RELAY_PIN_1_STATE_ADDR) == 1;
    Relay2State = EEPROM.read(RELAY_PIN_2_STATE_ADDR) == 1;
    Relay3State = EEPROM.read(RELAY_PIN_3_STATE_ADDR) == 1;
    Relay4State = EEPROM.read(RELAY_PIN_4_STATE_ADDR) == 1;
    Relay5State = EEPROM.read(RELAY_PIN_5_STATE_ADDR) == 1;
    Relay6State = EEPROM.read(RELAY_PIN_6_STATE_ADDR) == 1;
    Relay7State = EEPROM.read(RELAY_PIN_7_STATE_ADDR) == 1;
    Relay8State = EEPROM.read(RELAY_PIN_8_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM");
    Serial.println("Relay1 State: " + String(Relay1State ? "ON" : "OFF"));
    Serial.println("Relay2 State: " + String(Relay2State ? "ON" : "OFF"));
    Serial.println("Relay3 State: " + String(Relay3State ? "ON" : "OFF"));
    Serial.println("Relay4 State: " + String(Relay4State ? "ON" : "OFF"));
    Serial.println("Relay5 State: " + String(Relay5State ? "ON" : "OFF"));
    Serial.println("Relay6 State: " + String(Relay6State ? "ON" : "OFF"));
    Serial.println("Relay7 State: " + String(Relay7State ? "ON" : "OFF"));
    Serial.println("Relay8 State: " + String(Relay8State ? "ON" : "OFF"));
  }
}

// Function to save states to EEPROM
void saveStatesToEEPROM() {
  EEPROM.write(RELAY_PIN_1_STATE_ADDR, Relay1State ? 1 : 0);
  EEPROM.write(RELAY_PIN_2_STATE_ADDR, Relay2State ? 1 : 0);
  EEPROM.write(RELAY_PIN_3_STATE_ADDR, Relay3State ? 1 : 0);
  EEPROM.write(RELAY_PIN_4_STATE_ADDR, Relay4State ? 1 : 0);
  EEPROM.write(RELAY_PIN_5_STATE_ADDR, Relay5State ? 1 : 0);
  EEPROM.write(RELAY_PIN_6_STATE_ADDR, Relay6State ? 1 : 0);
  EEPROM.write(RELAY_PIN_7_STATE_ADDR, Relay7State ? 1 : 0);
  EEPROM.write(RELAY_PIN_8_STATE_ADDR, Relay8State ? 1 : 0);
  EEPROM.commit();
  Serial.println("Saved states to EEPROM");
}

// Apply the current state to the relays
void applyStates() {
  // Assuming relays are active LOW, so we invert the logic
  digitalWrite(RelayPin1, Relay1State ? LOW : HIGH);
  digitalWrite(RelayPin2, Relay2State ? LOW : HIGH);
  digitalWrite(RelayPin3, Relay3State ? LOW : HIGH);
  digitalWrite(RelayPin4, Relay4State ? LOW : HIGH);
  digitalWrite(RelayPin5, Relay5State ? LOW : HIGH);
  digitalWrite(RelayPin6, Relay6State ? LOW : HIGH);
  digitalWrite(RelayPin7, Relay7State ? LOW : HIGH);
  digitalWrite(RelayPin8, Relay8State ? LOW : HIGH);
  Serial.println("Applied states to relays");
  Serial.println("Relay1 State: " + String(Relay1State ? "ON" : "OFF"));
  Serial.println("Relay2 State: " + String(Relay2State ? "ON" : "OFF"));
  Serial.println("Relay3 State: " + String(Relay3State ? "ON" : "OFF"));
  Serial.println("Relay4 State: " + String(Relay4State ? "ON" : "OFF"));
  Serial.println("Relay5 State: " + String(Relay5State ? "ON" : "OFF"));
  Serial.println("Relay6 State: " + String(Relay6State ? "ON" : "OFF"));
  Serial.println("Relay7 State: " + String(Relay7State ? "ON" : "OFF"));
  Serial.println("Relay8 State: " + String(Relay8State ? "ON" : "OFF"));
}

// Function to publish current states
void publishStates() {
  // When relays are active LOW, we need to invert the state for the MQTT message
  // "ON" means the relay is energized (LOW), "OFF" means the relay is not energized (HIGH)
  client.publish(mqtt_topic_relay1, Relay1State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay2, Relay2State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay3, Relay3State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay4, Relay4State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay5, Relay5State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay6, Relay6State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay7, Relay7State ? "ON" : "OFF", true);
  client.publish(mqtt_topic_relay8, Relay8State ? "ON" : "OFF", true);
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

  bool stateChanged = false;
  bool newState = (message == "ON");
  String topicStr = String(topic);

  // Use if-else chain for better performance than multiple independent if statements
  if (topicStr == mqtt_topic_relay1) {
    if (Relay1State != newState) {
      Relay1State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay2) {
    if (Relay2State != newState) {
      Relay2State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay3) {
    if (Relay3State != newState) {
      Relay3State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay4) {
    if (Relay4State != newState) {
      Relay4State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay5) {
    if (Relay5State != newState) {
      Relay5State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay6) {
    if (Relay6State != newState) {
      Relay6State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay7) {
    if (Relay7State != newState) {
      Relay7State = newState;
      stateChanged = true;
    }
  }
  else if (topicStr == mqtt_topic_relay8) {
    if (Relay8State != newState) {
      Relay8State = newState;
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
      client.subscribe(mqtt_topic_relay1);
      client.subscribe(mqtt_topic_relay2);
      client.subscribe(mqtt_topic_relay3);
      client.subscribe(mqtt_topic_relay4);
      client.subscribe(mqtt_topic_relay5);
      client.subscribe(mqtt_topic_relay6);
      client.subscribe(mqtt_topic_relay7);
      client.subscribe(mqtt_topic_relay8);
      
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

// Function to initialize MQTT topics with device ID
void initMQTTTopics() {
  // Format: DEVICE_ID/endpoint
  sprintf(mqtt_topic_relay1, "%s/relay1", DEVICE_ID);
  sprintf(mqtt_topic_relay2, "%s/relay2", DEVICE_ID);
  sprintf(mqtt_topic_relay3, "%s/relay3", DEVICE_ID);
  sprintf(mqtt_topic_relay4, "%s/relay4", DEVICE_ID);
  sprintf(mqtt_topic_relay5, "%s/relay5", DEVICE_ID);
  sprintf(mqtt_topic_relay6, "%s/relay6", DEVICE_ID);
  sprintf(mqtt_topic_relay7, "%s/relay7", DEVICE_ID);
  sprintf(mqtt_topic_relay8, "%s/relay8", DEVICE_ID);
  sprintf(mqtt_topic_status, "%s/status", DEVICE_ID);
  sprintf(mqtt_topic_availability, "%s/availability", DEVICE_ID);
  
  Serial.println("MQTT topics initialized with Device ID: " + String(DEVICE_ID));
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Initialize MQTT client ID with DEVICE_ID
  sprintf(mqtt_client_id, "Homato_%s", DEVICE_ID);
  
  // Initialize MQTT topics
  initMQTTTopics();
  
  // Set pins to HIGH (relay off) immediately
  pinMode(RelayPin1, OUTPUT);
  pinMode(RelayPin2, OUTPUT);
  pinMode(RelayPin3, OUTPUT);
  pinMode(RelayPin4, OUTPUT);
  pinMode(RelayPin5, OUTPUT);
  pinMode(RelayPin6, OUTPUT);
  pinMode(RelayPin7, OUTPUT);
  pinMode(RelayPin8, OUTPUT);
  
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
  digitalWrite(RelayPin4, HIGH);
  digitalWrite(RelayPin5, HIGH);
  digitalWrite(RelayPin6, HIGH);
  digitalWrite(RelayPin7, HIGH);
  digitalWrite(RelayPin8, HIGH);
  
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