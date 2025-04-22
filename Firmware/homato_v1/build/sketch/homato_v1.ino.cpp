#include <Arduino.h>
#line 1 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <time.h>
#include <DHT.h>
#include <Preferences.h>

// Enable/disable features
#define ENABLE_SENSORS 1  // Set to 0 to disable temperature, humidity and LDR sensors

// Check if we are building with reduced features
#if !defined(REDUCED_FEATURES)
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <esp_gap_ble_api.h>
#endif

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "st-000002";

// WiFi credentials
String ssid;
String password;

// Multi-WiFi support
#define MAX_WIFI_NETWORKS 3
struct WiFiNetwork {
  char ssid[32];
  char password[64];
  bool isConfigured;
};
WiFiNetwork wifiNetworks[MAX_WIFI_NETWORKS];

// MQTT Broker settings
const char* mqtt_server = "1a87ae45965c44f3abbd2d523241f1f1.s2.eu.hivemq.cloud";
const char* mqtt_username = "homato";
const char* mqtt_password = "HomeAutomation@2025";
const int mqtt_port = 8883;
char mqtt_client_id[50];

// NTP Server settings
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // GMT+5:30 for IST
const int daylightOffset_sec = 0;  // No DST in India

// Heartbeat and sensor intervals
const int heartbeatInterval = 30000; // 30 seconds
const int sendSensorInterval = 10000; // 10 seconds

// Topics for communication
char mqtt_topic_relay1[40];
char mqtt_topic_relay2[40];
char mqtt_topic_relay3[40];
char mqtt_topic_relay4[40];
char mqtt_topic_relay5[40];
char mqtt_topic_relay6[40];
char mqtt_topic_relay7[40];
char mqtt_topic_relay8[40];
char mqtt_topic_availability[40];
char mqtt_topic_temperature[40];
char mqtt_topic_humidity[40];
char mqtt_topic_ldr[40];

// EEPROM addresses for storing relay states and WiFi credentials
#define EEPROM_SIZE 512
#define RELAY_PIN_1_STATE_ADDR 0
#define RELAY_PIN_2_STATE_ADDR 1
#define RELAY_PIN_3_STATE_ADDR 2
#define RELAY_PIN_4_STATE_ADDR 3
#define RELAY_PIN_5_STATE_ADDR 4
#define RELAY_PIN_6_STATE_ADDR 5
#define RELAY_PIN_7_STATE_ADDR 6
#define RELAY_PIN_8_STATE_ADDR 7
#define EEPROM_INITIALIZED_ADDR 8
#define WIFI_INITIALIZED_ADDR 9
#define WIFI_NETWORKS_START_ADDR 10

// Sensor and relay pins
#define DHTTYPE DHT11
#define DHTPIN 16
#define LDR_PIN 34
#define RelayPin1 23
#define RelayPin2 22
#define RelayPin3 21
#define RelayPin4 19
#define RelayPin5 18
#define RelayPin6 5
#define RelayPin7 25
#define RelayPin8 26
#define SwitchPin1 13
#define SwitchPin2 12
#define SwitchPin3 14
#define SwitchPin4 27
#define SwitchPin5 33
#define SwitchPin6 32
#define SwitchPin7 15
#define SwitchPin8 4
#define wifiLed 2
#define BUTTON_PIN 35

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

#if ENABLE_SENSORS
float temperature1 = 0;
float humidity1 = 0;
int ldrVal;
#endif

// Initialize WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient client(espClient);

#if ENABLE_SENSORS
DHT dht(DHTPIN, DHTTYPE);
#endif

#define NUM_RELAYS 8

// Create a preferences object for persistent storage
Preferences preferences;

#if !defined(REDUCED_FEATURES)
// BLE related variables
bool inBLEMode = false;
unsigned long buttonPressStart = 0;
String new_ssid;
String new_password;

// UUIDs for BLE service and characteristics
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define RELAY_COUNT_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DEVICE_ID_CHAR_UUID "1c95d7e3-0d9c-4477-8b75-9f2a5d2f5a88"
#define SSID_CHAR_UUID "a3b74f0f-33a4-4e4b-98f6-8a9d9b1c2d3c"
#define PASSWORD_CHAR_UUID "c4d5e6f7-1234-5678-9abc-def012345678"
#define WIFI_INDEX_CHAR_UUID "b5c6d7e8-2345-6789-bcde-012345678901"

// BLE Server pointer
BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic *pRelayCountCharacteristic = NULL;
BLECharacteristic *pDeviceIdCharacteristic = NULL;
BLECharacteristic *pSsidCharacteristic = NULL;
BLECharacteristic *pPasswordCharacteristic = NULL;
BLECharacteristic *pWifiIndexCharacteristic = NULL;
BLEAdvertising *pAdvertising = NULL;
#else
// Dummy variables for reduced build
bool inBLEMode = false;
unsigned long buttonPressStart = 0;
#endif

// Wifi network index for storage
int currentWifiIndex = 0;

// Forward declaration of functions
void readWiFiCredentials();
bool tryConnectWiFi();
void writeWiFiCredentials(String newSsid, String newPassword, int index);
void updateRelayState(int relayNumber, bool state);

#if !defined(REDUCED_FEATURES)
// Callback classes for BLE
class MySecurity : public BLESecurityCallbacks {
  uint32_t onPassKeyRequest() {
    return 123456; // Fixed passkey
  }
  void onPassKeyNotify(uint32_t pass_key) {
    Serial.println("Passkey notification: " + String(pass_key));
  }
  bool onConfirmPIN(uint32_t pin) {
    Serial.println("Confirming PIN: " + String(pin));
    return true;
  }
  bool onSecurityRequest() {
    Serial.println("Security request received");
    return true; // Always accept pairing requests
  }
  void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl) {
    if (auth_cmpl.success) {
      Serial.println("Authentication successful");
    } else {
      Serial.println("Authentication failed");
    }
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Client connected to BLE");
    digitalWrite(wifiLed, HIGH); // Turn on LED to indicate connection
  }

  void onDisconnect(BLEServer* pServer) {
    Serial.println("Client disconnected from BLE");
    digitalWrite(wifiLed, LOW); // Turn off LED when disconnected
    
    // Restart advertising to allow new connections
    pAdvertising->start();
  }
};

class MyCharacteristicCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // Get value as String first
    String rawValue = pCharacteristic->getValue().c_str();
    
    // Convert to Arduino String
    String str_value = rawValue;
    
    // Process based on UUID
    String uuidStr = pCharacteristic->getUUID().toString().c_str();
    
    if (uuidStr == SSID_CHAR_UUID) {
      new_ssid = str_value;
      Serial.println("Received SSID: " + new_ssid);
    } 
    else if (uuidStr == PASSWORD_CHAR_UUID) {
      new_password = str_value;
      Serial.println("Received Password: (hidden for security)");
    }
    else if (uuidStr == WIFI_INDEX_CHAR_UUID) {
      currentWifiIndex = str_value.toInt();
      if (currentWifiIndex < 0 || currentWifiIndex >= MAX_WIFI_NETWORKS) {
        currentWifiIndex = 0; // Default to first slot if invalid
      }
      Serial.println("Received WiFi Index: " + String(currentWifiIndex));
    }
    
    // If we have both SSID and password, save them to the preferences
    if (!new_ssid.isEmpty() && !new_password.isEmpty()) {
      writeWiFiCredentials(new_ssid, new_password, currentWifiIndex);
      
      // Provide feedback that credentials were saved
      String response = "OK";
      pCharacteristic->setValue(response.c_str());
      pCharacteristic->notify();
      
      // Clear for next set
      new_ssid = "";
      new_password = "";
    }
  }
};
#endif

#if ENABLE_SENSORS
#line 268 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void readSensor();
#line 283 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void sendSensor();
#line 292 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
String getFormattedTime();
#line 306 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void enterBLEMode();
#line 327 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void exitBLEMode();
#line 468 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void setup_wifi();
#line 478 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void readStatesFromEEPROM();
#line 511 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void saveRelayStateToEEPROM(int relayNumber, bool state);
#line 526 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void publishRelayState(int relayNumber);
#line 545 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void callback(char* topic, byte* payload, unsigned int length);
#line 576 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void applyAllRelayStates();
#line 584 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void publishAllRelayStates();
#line 590 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void publishAvailabilityStatus();
#line 594 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void manual_control();
#line 612 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void setupBLE();
#line 692 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void reconnect();
#line 715 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void initMQTTTopics();
#line 731 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void printDiagnosticInfo();
#line 772 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void setup();
#line 835 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void loop();
#line 268 "/Users/bhargavveepuri/Homato/v1/homato/Firmware/homato_v1/homato_v1.ino"
void readSensor() {
  ldrVal = map(analogRead(LDR_PIN), 0, 4095, 10, 0);
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
  }
}

void sendSensor() {
  readSensor();
  
  client.publish(mqtt_topic_temperature, String(temperature1).c_str(), false);
  client.publish(mqtt_topic_humidity, String(humidity1).c_str(), false);
  client.publish(mqtt_topic_ldr, String(ldrVal).c_str(), false);
}
#endif

String getFormattedTime() {
  struct tm timeinfo;
  char timeStringBuff[30];
  
  if (!getLocalTime(&timeinfo)) {
    return String(millis());
  }
  
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

#if !defined(REDUCED_FEATURES)
// Function to enter BLE mode
void enterBLEMode() {
  if (!inBLEMode) {
    inBLEMode = true;
    Serial.println("Entering BLE pairing mode");
    
    // Disconnect from WiFi and MQTT
    WiFi.disconnect();
    client.disconnect();
    
    // Turn on LED to indicate BLE mode
    digitalWrite(wifiLed, HIGH);
    
    // Start BLE advertising
    if (pAdvertising != NULL) {
      pAdvertising->start();
      Serial.println("BLE advertising started with name: Homato-" + String(DEVICE_ID));
    }
  }
}

// Function to exit BLE mode and reconnect to WiFi
void exitBLEMode() {
  if (inBLEMode) {
    inBLEMode = false;
    Serial.println("Exiting BLE mode");
    
    // Stop BLE advertising
    if (pAdvertising != NULL) {
      pAdvertising->stop();
    }
    
    // Turn off LED
    digitalWrite(wifiLed, LOW);
    
    // Reconnect to WiFi
    readWiFiCredentials();
    tryConnectWiFi();
  }
}
#else
// Stub implementations for reduced build
void enterBLEMode() {}
void exitBLEMode() {}
#endif

// Read all WiFi credentials from preferences
void readWiFiCredentials() {
  preferences.begin("wifi-config", false);
  
  // Check how many networks are configured
  int configuredNetworks = preferences.getInt("numNetworks", 0);
  Serial.println("Found " + String(configuredNetworks) + " configured WiFi networks");
  
  // Read each network
  for (int i = 0; i < MAX_WIFI_NETWORKS; i++) {
    String networkKey = "network" + String(i);
    String ssidKey = networkKey + "ssid";
    String passKey = networkKey + "pass";
    
    if (preferences.isKey(ssidKey.c_str())) {
      String storedSsid = preferences.getString(ssidKey.c_str(), "");
      String storedPass = preferences.getString(passKey.c_str(), "");
      
      if (storedSsid.length() > 0) {
        storedSsid.toCharArray(wifiNetworks[i].ssid, 32);
        storedPass.toCharArray(wifiNetworks[i].password, 64);
        wifiNetworks[i].isConfigured = true;
        
        Serial.println("Network " + String(i) + " SSID: " + storedSsid);
      } else {
        wifiNetworks[i].isConfigured = false;
      }
    } else {
      wifiNetworks[i].isConfigured = false;
    }
  }
  
  preferences.end();
}

// Write WiFi credentials to preferences
void writeWiFiCredentials(String newSsid, String newPassword, int index) {
  if (index < 0 || index >= MAX_WIFI_NETWORKS) {
    Serial.println("Invalid WiFi network index");
    return;
  }
  
  if (newSsid.length() > 31 || newPassword.length() > 63) {
    Serial.println("WiFi credentials too long");
    return;
  }
  
  preferences.begin("wifi-config", false);
  
  // Update credentials at specified index
  String networkKey = "network" + String(index);
  String ssidKey = networkKey + "ssid";
  String passKey = networkKey + "pass";
  
  preferences.putString(ssidKey.c_str(), newSsid);
  preferences.putString(passKey.c_str(), newPassword);
  
  // Update network count if needed
  int configuredNetworks = preferences.getInt("numNetworks", 0);
  if (index >= configuredNetworks) {
    preferences.putInt("numNetworks", index + 1);
  }
  
  preferences.end();
  
  // Update the local array as well
  newSsid.toCharArray(wifiNetworks[index].ssid, 32);
  newPassword.toCharArray(wifiNetworks[index].password, 64);
  wifiNetworks[index].isConfigured = true;
  
  Serial.println("Wrote WiFi credentials to storage at index " + String(index));
}

// Try to connect to WiFi using stored credentials
bool tryConnectWiFi() {
  int attempts = 0;
  bool connected = false;
  
  // Try each configured network
  for (int i = 0; i < MAX_WIFI_NETWORKS && !connected; i++) {
    if (!wifiNetworks[i].isConfigured) {
      continue;
    }
    
    Serial.println("Trying to connect to WiFi network " + String(i) + ": " + String(wifiNetworks[i].ssid));
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiNetworks[i].ssid, wifiNetworks[i].password);
    
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      ssid = String(wifiNetworks[i].ssid);
      password = String(wifiNetworks[i].password);
      connected = true;
      
      Serial.println("\nConnected to WiFi network: " + ssid);
      Serial.println("IP address: " + WiFi.localIP().toString());
      
      // Set up NTP time sync
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      
      // Turn on WiFi LED
      digitalWrite(wifiLed, HIGH);
    } else {
      Serial.println("\nFailed to connect to network: " + String(wifiNetworks[i].ssid));
    }
  }
  
  return connected;
}

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi");
  
  if (!tryConnectWiFi()) {
    Serial.println("Failed to connect to any WiFi networks");
    digitalWrite(wifiLed, LOW);
  }
}

void readStatesFromEEPROM() {
  byte isInitialized = EEPROM.read(EEPROM_INITIALIZED_ADDR);
  
  if (isInitialized == 0xFF) {
    RelayState1 = RelayState2 = RelayState3 = RelayState4 = false;
    RelayState5 = RelayState6 = RelayState7 = RelayState8 = false;
    
    EEPROM.write(RELAY_PIN_1_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_2_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_3_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_4_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_5_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_6_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_7_STATE_ADDR, 0);
    EEPROM.write(RELAY_PIN_8_STATE_ADDR, 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01);
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    RelayState1 = EEPROM.read(RELAY_PIN_1_STATE_ADDR) == 1;
    RelayState2 = EEPROM.read(RELAY_PIN_2_STATE_ADDR) == 1;
    RelayState3 = EEPROM.read(RELAY_PIN_3_STATE_ADDR) == 1;
    RelayState4 = EEPROM.read(RELAY_PIN_4_STATE_ADDR) == 1;
    RelayState5 = EEPROM.read(RELAY_PIN_5_STATE_ADDR) == 1;
    RelayState6 = EEPROM.read(RELAY_PIN_6_STATE_ADDR) == 1;
    RelayState7 = EEPROM.read(RELAY_PIN_7_STATE_ADDR) == 1;
    RelayState8 = EEPROM.read(RELAY_PIN_8_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM");
  }
}

void saveRelayStateToEEPROM(int relayNumber, bool state) {
  if (relayNumber < 1 || relayNumber > 8) return;
  
  EEPROM.write(RELAY_PIN_1_STATE_ADDR + (relayNumber - 1), state ? 1 : 0);
  EEPROM.commit();
}

void updateRelayState(int relayNumber, bool state) {
  uint8_t pins[8] = {RelayPin1, RelayPin2, RelayPin3, RelayPin4, RelayPin5, RelayPin6, RelayPin7, RelayPin8};
  
  if (relayNumber < 1 || relayNumber > 8) return;
  
  digitalWrite(pins[relayNumber-1], state ? LOW : HIGH);
}

void publishRelayState(int relayNumber) {
  bool state;
  char* topic;
  
  switch (relayNumber) {
    case 1: state = RelayState1; topic = mqtt_topic_relay1; break;
    case 2: state = RelayState2; topic = mqtt_topic_relay2; break;
    case 3: state = RelayState3; topic = mqtt_topic_relay3; break;
    case 4: state = RelayState4; topic = mqtt_topic_relay4; break;
    case 5: state = RelayState5; topic = mqtt_topic_relay5; break;
    case 6: state = RelayState6; topic = mqtt_topic_relay6; break;
    case 7: state = RelayState7; topic = mqtt_topic_relay7; break;
    case 8: state = RelayState8; topic = mqtt_topic_relay8; break;
    default: return;
  }
  
  client.publish(topic, state ? "ON" : "OFF", false);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  bool newState = (message == "ON");
  String topicStr = String(topic);
  int changedRelay = 0;
  bool *states[8] = {&RelayState1, &RelayState2, &RelayState3, &RelayState4, 
                     &RelayState5, &RelayState6, &RelayState7, &RelayState8};
  
  for (int i = 0; i < 8; i++) {
    char* topics[8] = {mqtt_topic_relay1, mqtt_topic_relay2, mqtt_topic_relay3, mqtt_topic_relay4,
                       mqtt_topic_relay5, mqtt_topic_relay6, mqtt_topic_relay7, mqtt_topic_relay8};
    
    if (topicStr == topics[i]) {
      if (*states[i] != newState) {
        *states[i] = newState;
        changedRelay = i + 1;
        break;
      }
    }
  }

  if (changedRelay > 0) {
    updateRelayState(changedRelay, newState);
    saveRelayStateToEEPROM(changedRelay, newState);
  }
}

void applyAllRelayStates() {
  for (int i = 1; i <= 8; i++) {
    bool *states[8] = {&RelayState1, &RelayState2, &RelayState3, &RelayState4, 
                       &RelayState5, &RelayState6, &RelayState7, &RelayState8};
    updateRelayState(i, *states[i-1]);
  }
}

void publishAllRelayStates() {
  for (int i = 1; i <= 8; i++) {
    publishRelayState(i);
  }
}

void publishAvailabilityStatus() {
  client.publish(mqtt_topic_availability, "online", false);
}

void manual_control() {
  uint8_t pins[8] = {SwitchPin1, SwitchPin2, SwitchPin3, SwitchPin4, 
                     SwitchPin5, SwitchPin6, SwitchPin7, SwitchPin8};
  bool *states[8] = {&RelayState1, &RelayState2, &RelayState3, &RelayState4, 
                     &RelayState5, &RelayState6, &RelayState7, &RelayState8};
  
  for (int i = 0; i < 8; i++) {
    if (digitalRead(pins[i]) == LOW) {
      updateRelayState(i+1, *states[i]);
      *states[i] = !(*states[i]);
      publishRelayState(i+1);
      delay(300);
    }
  }
}

#if !defined(REDUCED_FEATURES)
// Initialize BLE
void setupBLE() {
  String deviceName = "Homato-" + String(DEVICE_ID);
  BLEDevice::init(deviceName.c_str());
  
  // Security setup
  BLEDevice::setSecurityCallbacks(new MySecurity());
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  
  // Generate bonding keys
  uint32_t passkey = 123456; // Fixed passkey for pairing
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  
  // Set I/O capabilities
  esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE; // No input no output capabilities
  esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
  
  // Enable security features
  uint8_t auth_req = ESP_LE_AUTH_REQ_SC_BOND; // Secure connection, bonding
  esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
  
  // Create server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create service
  pService = pServer->createService(SERVICE_UUID);
  
  // Relay count characteristic (read-only)
  pRelayCountCharacteristic = pService->createCharacteristic(
    RELAY_COUNT_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  pRelayCountCharacteristic->setValue(String(NUM_RELAYS).c_str());
  
  // Device ID characteristic (read-only)
  pDeviceIdCharacteristic = pService->createCharacteristic(
    DEVICE_ID_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ
  );
  pDeviceIdCharacteristic->setValue(String(DEVICE_ID).c_str());
  
  // WiFi index characteristic (write with security)
  pWifiIndexCharacteristic = pService->createCharacteristic(
    WIFI_INDEX_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pWifiIndexCharacteristic->setCallbacks(new MyCharacteristicCallback());
  
  // SSID characteristic (write with security)
  pSsidCharacteristic = pService->createCharacteristic(
    SSID_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pSsidCharacteristic->setCallbacks(new MyCharacteristicCallback());
  
  // Password characteristic (write with security)
  pPasswordCharacteristic = pService->createCharacteristic(
    PASSWORD_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pPasswordCharacteristic->setCallbacks(new MyCharacteristicCallback());
  
  // Start the service
  pService->start();
  
  // Setup advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  
  Serial.println("BLE initialized with name: " + deviceName);
}
#else
// Empty BLE setup function for reduced build
void setupBLE() {
  Serial.println("BLE functionality disabled in this build");
}
#endif

void reconnect() {
  int attempts = 0;
  while (!client.connected() && attempts < 3) {
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      // Subscribe to topics
      const char* topics[8] = {
        mqtt_topic_relay1, mqtt_topic_relay2, mqtt_topic_relay3, mqtt_topic_relay4,
        mqtt_topic_relay5, mqtt_topic_relay6, mqtt_topic_relay7, mqtt_topic_relay8
      };
      
      for (int i = 0; i < 8; i++) {
        client.subscribe(topics[i]);
      }
      
      publishAvailabilityStatus();
      publishAllRelayStates();
    } else {
      delay(5000);
      attempts++;
    }
  }
}

void initMQTTTopics() {
  sprintf(mqtt_topic_relay1, "%s/relay1", DEVICE_ID);
  sprintf(mqtt_topic_relay2, "%s/relay2", DEVICE_ID);
  sprintf(mqtt_topic_relay3, "%s/relay3", DEVICE_ID);
  sprintf(mqtt_topic_relay4, "%s/relay4", DEVICE_ID);
  sprintf(mqtt_topic_relay5, "%s/relay5", DEVICE_ID);
  sprintf(mqtt_topic_relay6, "%s/relay6", DEVICE_ID);
  sprintf(mqtt_topic_relay7, "%s/relay7", DEVICE_ID);
  sprintf(mqtt_topic_relay8, "%s/relay8", DEVICE_ID);
  sprintf(mqtt_topic_availability, "%s/available", DEVICE_ID);
  sprintf(mqtt_topic_temperature, "%s/temp", DEVICE_ID);
  sprintf(mqtt_topic_humidity, "%s/humidity", DEVICE_ID);
  sprintf(mqtt_topic_ldr, "%s/ldr", DEVICE_ID);
}

// Function to print diagnostic information about the ESP32
void printDiagnosticInfo() {
  Serial.println("\n=== ESP32 DIAGNOSTIC INFO ===");
  
  // Chip information
  Serial.print("Chip Model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip Revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("CPU Frequency: ");
  Serial.print(ESP.getCpuFreqMHz());
  Serial.println(" MHz");
  
  // Memory information
  Serial.print("Total Heap: ");
  Serial.print(ESP.getHeapSize() / 1024);
  Serial.println(" KB");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");
  
  // Flash memory information
  Serial.print("Flash Size: ");
  Serial.print(ESP.getFlashChipSize() / (1024 * 1024));
  Serial.println(" MB");
  Serial.print("Flash Speed: ");
  Serial.print(ESP.getFlashChipSpeed() / 1000000);
  Serial.println(" MHz");
  
  // Sketch information
  Serial.print("Sketch Size: ");
  Serial.print(ESP.getSketchSize() / 1024);
  Serial.println(" KB");
  Serial.print("Free Space: ");
  Serial.print(ESP.getFreeSketchSpace() / 1024);
  Serial.println(" KB");
  Serial.print("SDK Version: ");
  Serial.println(ESP.getSdkVersion());
  
  Serial.println("===========================\n");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\nHomato Device Starting Up");
  printDiagnosticInfo();
  
  sprintf(mqtt_client_id, "Homato_%s", DEVICE_ID);
  initMQTTTopics();
  
  // Initialize pins
  const uint8_t relayPins[8] = {
    RelayPin1, RelayPin2, RelayPin3, RelayPin4, 
    RelayPin5, RelayPin6, RelayPin7, RelayPin8
  };
  
  const uint8_t switchPins[8] = {
    SwitchPin1, SwitchPin2, SwitchPin3, SwitchPin4, 
    SwitchPin5, SwitchPin6, SwitchPin7, SwitchPin8
  };
  
  // Set relay pins as outputs
  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH); // Relays are active LOW
  }
  
  // Set switch pins as inputs with pullup
  for (int i = 0; i < 8; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);
  }
  
  pinMode(wifiLed, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  // Initialize storage
  EEPROM.begin(EEPROM_SIZE);
  readStatesFromEEPROM();
  applyAllRelayStates();
  
  // Read WiFi credentials and try to connect
  readWiFiCredentials();
  setup_wifi();
  
  // Setup MQTT client
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.setBufferSize(512);
  
  #if !defined(REDUCED_FEATURES)
  #if ENABLE_SENSORS
  // Initialize sensors
  dht.begin();
  #endif
  
  // Initialize BLE
  setupBLE();
  #endif
  
  Serial.println("Setup complete");
}

void loop() {
#if !defined(REDUCED_FEATURES)
  // Check button press for entering BLE mode
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (buttonPressStart == 0) {
      buttonPressStart = millis();
    } else if (millis() - buttonPressStart >= 5000 && !inBLEMode) {
      enterBLEMode();
    }
  } else {
    buttonPressStart = 0;
  }
  
  // BLE mode operation
  if (inBLEMode) {
    // Blink the LED to indicate BLE mode
    static unsigned long lastBlink = 0;
    if (millis() - lastBlink > 500) {
      digitalWrite(wifiLed, !digitalRead(wifiLed));
      lastBlink = millis();
    }
    
    // Check if we want to exit BLE mode (button press for 5 seconds again)
    if (digitalRead(BUTTON_PIN) == LOW) {
      if (millis() - buttonPressStart >= 5000) {
        exitBLEMode();
      }
    }
    return; // Skip the rest of the loop when in BLE mode
  } 
#endif
  
  // Normal operation mode
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    setup_wifi();
  }
  
  // Check MQTT connection
  if (!client.connected()) {
    reconnect();
  }
  
  // Handle MQTT communications
  client.loop();
  
  // Check manual switch controls
  manual_control();
  
  // Send heartbeat
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > heartbeatInterval) {
    if (client.connected()) {
      client.publish(mqtt_topic_availability, "online", true);
    }
    lastHeartbeat = millis();
  }
  
#if !defined(REDUCED_FEATURES) && ENABLE_SENSORS
  // Send sensor data
  static unsigned long lastSensorPublish = 0;
  if (millis() - lastSensorPublish > sendSensorInterval) {
    sendSensor();
    lastSensorPublish = millis();
  }
#endif
}
