#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <time.h>  // Add time library
#include <DHT.h>  

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "st-000002";

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

// Heartbeat and sensor intervals
const int heartbeatInterval = 30000; // 30 seconds
const int sendSensorInterval = 10000; // 10 seconds

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
char mqtt_topic_temperature[50];
char mqtt_topic_humidity[50];
char mqtt_topic_ldr[50];

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

// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

#define DHTPIN              16 //D16  pin connected with DHT
#define LDR_PIN             34 //D34  pin connected with LDR

// define the GPIO connected with Relays and switches
#define RelayPin1 23  //D23
#define RelayPin2 22  //D22
#define RelayPin3 21  //D21
#define RelayPin4 19  //D19
#define RelayPin5 18  //D18
#define RelayPin6 5   //D5
#define RelayPin7 25  //D25
#define RelayPin8 26  //D26

#define SwitchPin1 13  //D13
#define SwitchPin2 12  //D12
#define SwitchPin3 14  //D14
#define SwitchPin4 27  //D27
#define SwitchPin5 33  //D33
#define SwitchPin6 32  //D32
#define SwitchPin7 15  //D15
#define SwitchPin8 4   //D4

#define wifiLed    2   //D2


// Device state variables
bool RelayState1 = false;
bool RelayState2 = false;
bool RelayState3 = false;
bool RelayState4 = false;
bool RelayState5 = false;
bool RelayState6 = false;
bool RelayState7 = false;
bool RelayState8 = false;


// Switch State
bool SwitchState1 = false;
bool SwitchState2 = false;
bool SwitchState3 = false;
bool SwitchState4 = false;
bool SwitchState5 = false;
bool SwitchState6 = false;
bool SwitchState7 = false;
bool SwitchState8 = false;

float temperature1 = 0;
float humidity1   = 0;
int   ldrVal;

// Initialize WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

void readSensor(){
    
  ldrVal = map(analogRead(LDR_PIN), 0, 4095, 10, 0);
  Serial.println("ldrVal: " + String(ldrVal));
  
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  else {
    humidity1 = h;
    temperature1 = t;
   Serial.println("temperature: " + String(temperature1));
   Serial.println("humidity: " + String(humidity1));
  }  
}


void sendSensor()
{
  readSensor();
  publishSensor();
}

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
    // Serial.print("Current time: ");
    // Serial.println(getFormattedTime());
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
    RelayState1 = false;
    RelayState2 = false;
    RelayState3 = false;
    RelayState4 = false;
    RelayState5 = false;
    RelayState6 = false;
    RelayState7 = false;
    RelayState8 = false;
    
    // Save default values to EEPROM
    EEPROM.write(RELAY_PIN_1_STATE_ADDR, RelayState1 ? 1 : 0);
    EEPROM.write(RELAY_PIN_2_STATE_ADDR, RelayState2 ? 1 : 0);
    EEPROM.write(RELAY_PIN_3_STATE_ADDR, RelayState3 ? 1 : 0);
    EEPROM.write(RELAY_PIN_4_STATE_ADDR, RelayState4 ? 1 : 0);
    EEPROM.write(RELAY_PIN_5_STATE_ADDR, RelayState5 ? 1 : 0);
    EEPROM.write(RELAY_PIN_6_STATE_ADDR, RelayState6 ? 1 : 0);
    EEPROM.write(RELAY_PIN_7_STATE_ADDR, RelayState7 ? 1 : 0);
    EEPROM.write(RELAY_PIN_8_STATE_ADDR, RelayState8 ? 1 : 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01); // Mark as initialized
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    // Read values from EEPROM
    RelayState1 = EEPROM.read(RELAY_PIN_1_STATE_ADDR) == 1;
    RelayState2 = EEPROM.read(RELAY_PIN_2_STATE_ADDR) == 1;
    RelayState3 = EEPROM.read(RELAY_PIN_3_STATE_ADDR) == 1;
    RelayState4 = EEPROM.read(RELAY_PIN_4_STATE_ADDR) == 1;
    RelayState5 = EEPROM.read(RELAY_PIN_5_STATE_ADDR) == 1;
    RelayState6 = EEPROM.read(RELAY_PIN_6_STATE_ADDR) == 1;
    RelayState7 = EEPROM.read(RELAY_PIN_7_STATE_ADDR) == 1;
    RelayState8 = EEPROM.read(RELAY_PIN_8_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM");
    Serial.println("Relay1 State: " + String(RelayState1 ? "ON" : "OFF"));
    Serial.println("Relay2 State: " + String(RelayState2 ? "ON" : "OFF"));
    Serial.println("Relay3 State: " + String(RelayState3 ? "ON" : "OFF"));
    Serial.println("Relay4 State: " + String(RelayState4 ? "ON" : "OFF"));
    Serial.println("Relay5 State: " + String(RelayState5 ? "ON" : "OFF"));
    Serial.println("Relay6 State: " + String(RelayState6 ? "ON" : "OFF"));
    Serial.println("Relay7 State: " + String(RelayState7 ? "ON" : "OFF"));
    Serial.println("Relay8 State: " + String(RelayState8 ? "ON" : "OFF"));
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
      state = RelayState1;
      topic = mqtt_topic_relay1;
      break;
    case 2:
      state = RelayState2;
      topic = mqtt_topic_relay2;
      break;
    case 3:
      state = RelayState3;
      topic = mqtt_topic_relay3;
      break;
    case 4:
      state = RelayState4;
      topic = mqtt_topic_relay4;
      break;
    case 5:
      state = RelayState5;
      topic = mqtt_topic_relay5;
      break;
    case 6:
      state = RelayState6;
      topic = mqtt_topic_relay6;
      break;
    case 7:
      state = RelayState7;
      topic = mqtt_topic_relay7;
      break;
    case 8:
      state = RelayState8;
      topic = mqtt_topic_relay8;
      break;
    default:
      return;
  }
  
  client.publish(topic, state ? "ON" : "OFF", false);
  // client.publish(mqtt_topic_availability, "online", false);
  
  Serial.println("Published relay " + String(relayNumber) + " state to MQTT: " + String(state ? "ON" : "OFF"));
  // Serial.println("Published relay " + String(relayNumber) + " state to MQTT: " + String(state ? "ON" : "OFF") + " at " + getFormattedTime());
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
    if (RelayState1 != newState) {
      RelayState1 = newState;
      stateChanged = true;
      changedRelay = 1;
    }
  }
  else if (topicStr == mqtt_topic_relay2) {
    if (RelayState2 != newState) {
      RelayState2 = newState;
      stateChanged = true;
      changedRelay = 2;
    }
  }
  else if (topicStr == mqtt_topic_relay3) {
    if (RelayState3 != newState) {
      RelayState3 = newState;
      stateChanged = true;
      changedRelay = 3;
    }
  }
  else if (topicStr == mqtt_topic_relay4) {
    if (RelayState4 != newState) {
      RelayState4 = newState;
      stateChanged = true;
      changedRelay = 4;
    }
  }
  else if (topicStr == mqtt_topic_relay5) {
    if (RelayState5 != newState) {
      RelayState5 = newState;
      stateChanged = true;
      changedRelay = 5;
    }
  }
  else if (topicStr == mqtt_topic_relay6) {
    if (RelayState6 != newState) {
      RelayState6 = newState;
      stateChanged = true;
      changedRelay = 6;
    }
  }
  else if (topicStr == mqtt_topic_relay7) {
    if (RelayState7 != newState) {
      RelayState7 = newState;
      stateChanged = true;
      changedRelay = 7;
    }
  }
  else if (topicStr == mqtt_topic_relay8) {
    if (RelayState8 != newState) {
      RelayState8 = newState;
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
  updateRelayState(1, RelayState1);
  updateRelayState(2, RelayState2);
  updateRelayState(3, RelayState3);
  updateRelayState(4, RelayState4);
  updateRelayState(5, RelayState5);
  updateRelayState(6, RelayState6);
  updateRelayState(7, RelayState7);
  updateRelayState(8, RelayState8);
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

void publishTemperature() {
  client.publish(mqtt_topic_temperature, String(temperature1).c_str(), false);
  Serial.println("Published temperature: " + String(temperature1) + "°C at " + getFormattedTime());
}

void publishHumidity() {
  client.publish(mqtt_topic_humidity, String(humidity1).c_str(), false);
  Serial.println("Published humidity: " + String(humidity1) + "% at " + getFormattedTime());
}

void publishLDR() {
  client.publish(mqtt_topic_ldr, String(ldrVal).c_str(), false);
  Serial.println("Published LDR value: " + String(ldrVal) + " at " + getFormattedTime());
}

void publishSensor() {
  publishTemperature();
  publishHumidity();
  publishLDR();
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
  sprintf(mqtt_topic_temperature, "%s/temperature", DEVICE_ID);
  sprintf(mqtt_topic_humidity, "%s/humidity", DEVICE_ID);
  sprintf(mqtt_topic_ldr, "%s/ldr", DEVICE_ID);
  
  Serial.println("MQTT topics initialized with Device ID: " + String(DEVICE_ID));
}

void allSwitchesOff() {
  updateRelayState(1, 0);
  updateRelayState(2, 0);
  updateRelayState(3, 0);
  updateRelayState(4, 0);
  updateRelayState(5, 0);
  updateRelayState(6, 0);
  updateRelayState(7, 0);
  updateRelayState(8, 0);
  publishRelayState(1);
  publishRelayState(2);
  publishRelayState(3);
  publishRelayState(4);
  publishRelayState(5);
  publishRelayState(6);
  publishRelayState(7);
  publishRelayState(8);
  Serial.println("All relays turned off");
}


void manual_control()
{
  if (digitalRead(SwitchPin1) == LOW) {
    digitalWrite(RelayPin1, RelayState1);
    RelayState1 = !RelayState1;
    publishRelayState(1);
    delay(300); 
  }
  if (digitalRead(SwitchPin2) == LOW) {
    digitalWrite(RelayPin2, RelayState2);
    RelayState2 = !RelayState2;
    publishRelayState(2);
    delay(300);
  }
  if (digitalRead(SwitchPin3) == LOW) {
    digitalWrite(RelayPin3, RelayState3);
    RelayState3 = !RelayState3;
    publishRelayState(3);
    delay(300);
  }
  if (digitalRead(SwitchPin4) == LOW) {
    digitalWrite(RelayPin4, RelayState4);
    RelayState4 = !RelayState4;
    publishRelayState(4);
    delay(300); 
  }
  if (digitalRead(SwitchPin5) == LOW) {
    digitalWrite(RelayPin5, RelayState5);
    RelayState5 = !RelayState5;
    publishRelayState(5);
    delay(300); 
  }
  if (digitalRead(SwitchPin6) == LOW) {
    digitalWrite(RelayPin6, RelayState6);
    RelayState6 = !RelayState6;
    publishRelayState(6);
    delay(300);
  }
  if (digitalRead(SwitchPin7) == LOW) {
    digitalWrite(RelayPin7, RelayState7);
    RelayState7 = !RelayState7;
    publishRelayState(7);
    delay(300);
  }
  if (digitalRead(SwitchPin8) == LOW) {
    digitalWrite(RelayPin8, RelayState8);
    RelayState8 = !RelayState8;
    publishRelayState(8);
    delay(300); 
  }
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

  pinMode(wifiLed, OUTPUT);

  pinMode(SwitchPin1, INPUT_PULLUP);
  pinMode(SwitchPin2, INPUT_PULLUP);
  pinMode(SwitchPin3, INPUT_PULLUP);
  pinMode(SwitchPin4, INPUT_PULLUP);
  pinMode(SwitchPin5, INPUT_PULLUP);
  pinMode(SwitchPin6, INPUT_PULLUP);
  pinMode(SwitchPin7, INPUT_PULLUP);
  pinMode(SwitchPin8, INPUT_PULLUP);
  
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

  dht.begin();

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
  manual_control();
  // Add a heartbeat message every 30 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > heartbeatInterval) {
    if (client.connected()) {
      client.publish(mqtt_topic_availability, "online", true);
      Serial.println("Heartbeat sent at " + getFormattedTime());
    }
    lastHeartbeat = millis();
  }

  static unsigned long lastSensorPublish = 0;
  if (millis() - lastSensorPublish > sendSensorInterval) {
    sendSensor();
    lastSensorPublish = millis();
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