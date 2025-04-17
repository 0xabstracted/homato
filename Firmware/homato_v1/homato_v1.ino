#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <time.h>  // Add time library

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "st-000001";

// WiFi credentials
const char* ssid = "Airtel_704B";
const char* password = "unseen@1";

// MQTT Broker settings
const char* mqtt_server = "1a87ae45965c44f3abbd2d523241f1f1.s2.eu.hivemq.cloud";
const char* mqtt_username = "homato";
const char* mqtt_password = "HomeAutomation@2025";
const int mqtt_port = 8883;
char mqtt_client_id[50]; // Will be set based on DEVICE_ID

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;  // GMT+5:30 for IST (5*3600 + 30*60)
const int   daylightOffset_sec = 0; // No DST in India

// Topics for communication
char mqtt_topic_relay1[50];
char mqtt_topic_relay2[50];
char mqtt_topic_relay3[50];
char mqtt_topic_relay4[50];
char mqtt_topic_relay5[50];
char mqtt_topic_relay6[50];
char mqtt_topic_relay7[50];
char mqtt_topic_relay8[50];
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

// Function to get formatted date and time as string
String getFormattedTime() {
  struct tm timeinfo;
  char timeStringBuff[50];
  
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return String(millis()); // Fallback to millis if time sync fails
  }
  
  // Format: YYYY-MM-DD HH:MM:SS
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

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
    
    // Configure and initialize NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    Serial.println("NTP time sync initialized");
    
    // Display current time
    Serial.print("Current time: ");
    Serial.println(getFormattedTime());
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

// Function to save a specific relay state to EEPROM
void saveRelayStateToEEPROM(int relayNumber, bool state) {
  switch (relayNumber) {
    case 1:
      EEPROM.write(RELAY_PIN_1_STATE_ADDR, state ? 1 : 0);
      break;
    case 2:
      EEPROM.write(RELAY_PIN_2_STATE_ADDR, state ? 1 : 0);
      break;
    case 3:
      EEPROM.write(RELAY_PIN_3_STATE_ADDR, state ? 1 : 0);
      break;
    case 4:
      EEPROM.write(RELAY_PIN_4_STATE_ADDR, state ? 1 : 0);
      break;
    case 5:
      EEPROM.write(RELAY_PIN_5_STATE_ADDR, state ? 1 : 0);
      break;
    case 6:
      EEPROM.write(RELAY_PIN_6_STATE_ADDR, state ? 1 : 0);
      break;
    case 7:
      EEPROM.write(RELAY_PIN_7_STATE_ADDR, state ? 1 : 0);
      break;
    case 8:
      EEPROM.write(RELAY_PIN_8_STATE_ADDR, state ? 1 : 0);
      break;
  }
  EEPROM.commit();
  Serial.println("Saved relay " + String(relayNumber) + " state to EEPROM: " + String(state ? "ON" : "OFF"));
}

// Function to update a specific relay
void updateRelayState(int relayNumber, bool state) {
  switch (relayNumber) {
    case 1:
      digitalWrite(RelayPin1, state ? LOW : HIGH);
      Serial.println("Updated relay 1 state: " + String(state ? "ON" : "OFF"));
      break;
    case 2:
      digitalWrite(RelayPin2, state ? LOW : HIGH);
      Serial.println("Updated relay 2 state: " + String(state ? "ON" : "OFF"));
      break;
    case 3:
      digitalWrite(RelayPin3, state ? LOW : HIGH);
      Serial.println("Updated relay 3 state: " + String(state ? "ON" : "OFF"));
      break;
    case 4:
      digitalWrite(RelayPin4, state ? LOW : HIGH);
      Serial.println("Updated relay 4 state: " + String(state ? "ON" : "OFF"));
      break;
    case 5:
      digitalWrite(RelayPin5, state ? LOW : HIGH);
      Serial.println("Updated relay 5 state: " + String(state ? "ON" : "OFF"));
      break;
    case 6:
      digitalWrite(RelayPin6, state ? LOW : HIGH);
      Serial.println("Updated relay 6 state: " + String(state ? "ON" : "OFF"));
      break;
    case 7:
      digitalWrite(RelayPin7, state ? LOW : HIGH);
      Serial.println("Updated relay 7 state: " + String(state ? "ON" : "OFF"));
      break;
    case 8:
      digitalWrite(RelayPin8, state ? LOW : HIGH);
      Serial.println("Updated relay 8 state: " + String(state ? "ON" : "OFF"));
      break;
  }
}

// Function to publish a specific relay state
void publishRelayState(int relayNumber) {
  bool state;
  char* topic;
  
  switch (relayNumber) {
    case 1:
      state = Relay1State;
      topic = mqtt_topic_relay1;
      break;
    case 2:
      state = Relay2State;
      topic = mqtt_topic_relay2;
      break;
    case 3:
      state = Relay3State;
      topic = mqtt_topic_relay3;
      break;
    case 4:
      state = Relay4State;
      topic = mqtt_topic_relay4;
      break;
    case 5:
      state = Relay5State;
      topic = mqtt_topic_relay5;
      break;
    case 6:
      state = Relay6State;
      topic = mqtt_topic_relay6;
      break;
    case 7:
      state = Relay7State;
      topic = mqtt_topic_relay7;
      break;
    case 8:
      state = Relay8State;
      topic = mqtt_topic_relay8;
      break;
    default:
      return;
  }
  
  client.publish(topic, state ? "ON" : "OFF", false);
  // client.publish(mqtt_topic_availability, "online", false);
  
  Serial.println("Published relay " + String(relayNumber) + " state to MQTT: " + String(state ? "ON" : "OFF") + " at " + getFormattedTime());
}

// Callback function for MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on [");
  Serial.print(topic);
  Serial.print("] ");
  
  // Convert payload to string
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print(message);
  Serial.print(" at ");
  Serial.println(getFormattedTime());

  bool stateChanged = false;
  int changedRelay = 0;
  bool newState = (message == "ON");
  String topicStr = String(topic);

  // Use if-else chain for better performance than multiple independent if statements
  if (topicStr == mqtt_topic_relay1) {
    if (Relay1State != newState) {
      Relay1State = newState;
      stateChanged = true;
      changedRelay = 1;
    }
  }
  else if (topicStr == mqtt_topic_relay2) {
    if (Relay2State != newState) {
      Relay2State = newState;
      stateChanged = true;
      changedRelay = 2;
    }
  }
  else if (topicStr == mqtt_topic_relay3) {
    if (Relay3State != newState) {
      Relay3State = newState;
      stateChanged = true;
      changedRelay = 3;
    }
  }
  else if (topicStr == mqtt_topic_relay4) {
    if (Relay4State != newState) {
      Relay4State = newState;
      stateChanged = true;
      changedRelay = 4;
    }
  }
  else if (topicStr == mqtt_topic_relay5) {
    if (Relay5State != newState) {
      Relay5State = newState;
      stateChanged = true;
      changedRelay = 5;
    }
  }
  else if (topicStr == mqtt_topic_relay6) {
    if (Relay6State != newState) {
      Relay6State = newState;
      stateChanged = true;
      changedRelay = 6;
    }
  }
  else if (topicStr == mqtt_topic_relay7) {
    if (Relay7State != newState) {
      Relay7State = newState;
      stateChanged = true;
      changedRelay = 7;
    }
  }
  else if (topicStr == mqtt_topic_relay8) {
    if (Relay8State != newState) {
      Relay8State = newState;
      stateChanged = true;
      changedRelay = 8;
    }
  }

  // If state changed, update only the specific relay
  if (stateChanged) {
    updateRelayState(changedRelay, newState);
    saveRelayStateToEEPROM(changedRelay, newState);
  }
}

// Function to apply all relay states at once (for initialization)
void applyAllRelayStates() {
  updateRelayState(1, Relay1State);
  updateRelayState(2, Relay2State);
  updateRelayState(3, Relay3State);
  updateRelayState(4, Relay4State);
  updateRelayState(5, Relay5State);
  updateRelayState(6, Relay6State);
  updateRelayState(7, Relay7State);
  updateRelayState(8, Relay8State);
  Serial.println("Applied all relay states");
}

// Function to publish all relay states (for initial connection)
void publishAllRelayStates() {
  publishRelayState(1);
  publishRelayState(2);
  publishRelayState(3);
  publishRelayState(4);
  publishRelayState(5);
  publishRelayState(6);
  publishRelayState(7);
  publishRelayState(8);
  Serial.println("Published all relay states");
}

// Function to publish availability status
void publishAvailabilityStatus() {
  client.publish(mqtt_topic_availability, "online", false);
  Serial.println("Published availability status: online at " + getFormattedTime());
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
  applyAllRelayStates();
  
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
      Serial.println("Heartbeat sent at " + getFormattedTime());
    }
    lastHeartbeat = millis();
  }
}

// Function to reconnect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Configure LWT (Last Will and Testament)
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password, 
                      mqtt_topic_availability, 2, true, "offline")) {
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
      publishAvailabilityStatus();
      publishAllRelayStates();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}