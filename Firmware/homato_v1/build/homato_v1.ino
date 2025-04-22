#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <time.h>
#include <DHT.h>
#include <Preferences.h>

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
const long  gmtOffset_sec = 19800;  // GMT+5:30 for IST
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

float temperature1 = 0;
float humidity1 = 0;
int ldrVal;

// Initialize WiFi and MQTT clients
WiFiClientSecure espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

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
#endif

// Forward declaration of writeWiFiCredentials function
void writeWiFiCredentials(String newSsid, String newPassword, int index);

#if !defined(REDUCED_FEATURES)
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

void readSensor() {
  ldrVal = map(analogRead(LDR_PIN), 0, 4095, 10, 0);
#ifdef DEBUG
  Serial.println("ldrVal: " + String(ldrVal));
#endif
  
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  } else {
    humidity1 = h;
    temperature1 = t;
#ifdef DEBUG
    Serial.println("temperature: " + String(temperature1));
    Serial.println("humidity: " + String(humidity1));
#endif
  }
}

void sendSensor() {
  readSensor();
  publishSensor();
}

String getFormattedTime() {
  struct tm timeinfo;
  char timeStringBuff[50];
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
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
  Serial.println("SSID: " + newSsid);
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
      Serial.println("NTP time sync initialized");
      
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
    RelayState1 = false;
    RelayState2 = false;
    RelayState3 = false;
    RelayState4 = false;
    RelayState5 = false;
    RelayState6 = false;
    RelayState7 = false;
    RelayState8 = false;
    
    EEPROM.write(RELAY_PIN_1_STATE_ADDR, RelayState1 ? 1 : 0);
    EEPROM.write(RELAY_PIN_2_STATE_ADDR, RelayState2 ? 1 : 0);
    EEPROM.write(RELAY_PIN_3_STATE_ADDR, RelayState3 ? 1 : 0);
    EEPROM.write(RELAY_PIN_4_STATE_ADDR, RelayState4 ? 1 : 0);
    EEPROM.write(RELAY_PIN_5_STATE_ADDR, RelayState5 ? 1 : 0);
    EEPROM.write(RELAY_PIN_6_STATE_ADDR, RelayState6 ? 1 : 0);
    EEPROM.write(RELAY_PIN_7_STATE_ADDR, RelayState7 ? 1 : 0);
    EEPROM.write(RELAY_PIN_8_STATE_ADDR, RelayState8 ? 1 : 0);
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
  Serial.println("Published relay " + String(relayNumber) + " state to MQTT: " + String(state ? "ON" : "OFF"));
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on [");
  Serial.print(topic);
  Serial.print("] ");
  
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

  if (topicStr == mqtt_topic_relay1) {
    if (RelayState1 != newState) {
      RelayState1 = newState;
      stateChanged = true;
      changedRelay = 1;
    }
  } else if (topicStr == mqtt_topic_relay2) {
    if (RelayState2 != newState) {
      RelayState2 = newState;
      stateChanged = true;
      changedRelay = 2;
    }
  } else if (topicStr == mqtt_topic_relay3) {
    if (RelayState3 != newState) {
      RelayState3 = newState;
      stateChanged = true;
      changedRelay = 3;
    }
  } else if (topicStr == mqtt_topic_relay4) {
    if (RelayState4 != newState) {
      RelayState4 = newState;
      stateChanged = true;
      changedRelay = 4;
    }
  } else if (topicStr == mqtt_topic_relay5) {
    if (RelayState5 != newState) {
      RelayState5 = newState;
      stateChanged = true;
      changedRelay = 5;
    }
  } else if (topicStr == mqtt_topic_relay6) {
    if (RelayState6 != newState) {
      RelayState6 = newState;
      stateChanged = true;
      changedRelay = 6;
    }
  } else if (topicStr == mqtt_topic_relay7) {
    if (RelayState7 != newState) {
      RelayState7 = newState;
      stateChanged = true;
      changedRelay = 7;
    }
  } else if (topicStr == mqtt_topic_relay8) {
    if (RelayState8 != newState) {
      RelayState8 = newState;
      stateChanged = true;
      changedRelay = 8;
    }
  }

  if (stateChanged) {
    updateRelayState(changedRelay, newState);
    saveRelayStateToEEPROM(changedRelay, newState);
  }
}

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

void publishAvailabilityStatus() {
  client.publish(mqtt_topic_availability, "online", false);
  Serial.println("Published availability status: online at " + getFormattedTime());
}

void publishTemperature() {
  client.publish(mqtt_topic_temperature, String(temperature1).c_str(), false);
#ifdef DEBUG
  Serial.println("Published temperature: " + String(temperature1) + "°C at " + getFormattedTime());
#endif
}

void publishHumidity() {
  client.publish(mqtt_topic_humidity, String(humidity1).c_str(), false);
#ifdef DEBUG
  Serial.println("Published humidity: " + String(humidity1) + "% at " + getFormattedTime());
#endif
}

void publishLDR() {
  client.publish(mqtt_topic_ldr, String(ldrVal).c_str(), false);
#ifdef DEBUG
  Serial.println("Published LDR value: " + String(ldrVal) + " at " + getFormattedTime());
#endif
}

void publishSensor() {
  publishTemperature();
  publishHumidity();
  publishLDR();
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

void manual_control() {
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

#if !defined(REDUCED_FEATURES)
// Initialize BLE
void setupBLE() {
  String deviceName = "Homato-" + String(DEVICE_ID);
  BLEDevice::init(deviceName.c_str());
  
  // Security setup
  BLEDevice::setSecurityCallbacks(new MySecurity());
  
  // Set the encryption level (supports ESP_BLE_SEC_ENCRYPT and ESP_BLE_SEC_ENCRYPT_NO_MITM)
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  
  // Generate bonding keys
  uint32_t passkey = 123456; // Fixed passkey for pairing
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  
  // Set I/O capabilities (display only, keyboard only, etc)
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
  String relayCountStr = String(NUM_RELAYS);
  pRelayCountCharacteristic->setValue(relayCountStr.c_str());
  
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
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  
  // Don't start advertising yet - only when button is pressed
  Serial.println("BLE initialized with name: " + deviceName);
  Serial.println("Security features enabled: BOND + ENCRYPT");
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
    Serial.print("Attempting MQTT connection...");
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_relay1);
      client.subscribe(mqtt_topic_relay2);
      client.subscribe(mqtt_topic_relay3);
      client.subscribe(mqtt_topic_relay4);
      client.subscribe(mqtt_topic_relay5);
      client.subscribe(mqtt_topic_relay6);
      client.subscribe(mqtt_topic_relay7);
      client.subscribe(mqtt_topic_relay8);
      publishAvailabilityStatus();
      publishAllRelayStates();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
      attempts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\nHomato Device Starting Up");
  Serial.println("Device ID: " + String(DEVICE_ID));
  Serial.println("Firmware Version: 1.0");
  
  sprintf(mqtt_client_id, "Homato_%s", DEVICE_ID);
  initMQTTTopics();
  
  // Initialize pins
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
  pinMode(BUTTON_PIN, INPUT);
  
  // Set initial relay states (relays are active LOW)
  digitalWrite(RelayPin1, HIGH);
  digitalWrite(RelayPin2, HIGH);
  digitalWrite(RelayPin3, HIGH);
  digitalWrite(RelayPin4, HIGH);
  digitalWrite(RelayPin5, HIGH);
  digitalWrite(RelayPin6, HIGH);
  digitalWrite(RelayPin7, HIGH);
  digitalWrite(RelayPin8, HIGH);
  
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
  
#ifndef REDUCED_FEATURES
  // Initialize sensors
  dht.begin();
  
  // Initialize BLE
  setupBLE();
#endif
  
  Serial.println("Setup complete, entering normal operation mode");
}

void loop() {
#ifndef REDUCED_FEATURES
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
    // In BLE mode, we just wait for connections and handle BLE operations
    // BLE callbacks will handle the pairing and credential setting
    
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
    Serial.println("WiFi connection lost. Attempting to reconnect...");
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
      Serial.println("Heartbeat sent at " + getFormattedTime());
    }
    lastHeartbeat = millis();
  }
  
#ifndef REDUCED_FEATURES
  // Send sensor data
  static unsigned long lastSensorPublish = 0;
  if (millis() - lastSensorPublish > sendSensorInterval) {
    sendSensor();
    lastSensorPublish = millis();
  }
#endif
}