#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <EEPROM.h>
#include <time.h>  // Add time library
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>  // Replace esp_mqtt_client with PubSubClient
#include <esp_task_wdt.h>

// PubSubClient library defaults
#define MQTT_MAX_PACKET_SIZE 512
#define MQTT_KEEPALIVE 60

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "st-000002";

// Number of relays on this device
const int NUM_RELAYS = 8;

// define the GPIO connected with Relays and switches
// D23, D22, D21, D19, D18, D5, D25, D26
const int RelayPins[8] = {23, 22, 21, 19, 18, 5, 25, 26};
// D13, D12, D14, D27, D33, D32, D15, D4
const int SwitchPins[8] = {13, 12, 14, 27, 33, 32, 15, 4};

#define WifiLed    2   //D2
#define BleLed 2     // Usually the built-in LED

// Button for BLE pairing mode
#define BleTurnOnPin 0  // Usually GPIO0 is the BOOT button on most ESP32 boards

// Device state variables
bool RelayStates[NUM_RELAYS] = {false, false, false, false, false, false, false, false};

// Switch State
bool SwitchStates[NUM_RELAYS] = {false, false, false, false, false, false, false, false};

// EEPROM addresses for storing relay states and WiFi credentials
#define EEPROM_SIZE 512  // Increased for WiFi credentials

// Relay state addresses
#define RELAY_PIN_1_STATE_ADDR 0
#define RELAY_PIN_2_STATE_ADDR 1
#define RELAY_PIN_3_STATE_ADDR 2
#define RELAY_PIN_4_STATE_ADDR 3
#define RELAY_PIN_5_STATE_ADDR 4
#define RELAY_PIN_6_STATE_ADDR 5
#define RELAY_PIN_7_STATE_ADDR 6
#define RELAY_PIN_8_STATE_ADDR 7
#define EEPROM_INITIALIZED_ADDR 8
#define INSTALLATION_STATUS_ADDR 9

// WiFi credentials storage addresses
// Each credential takes 1 byte (valid flag) + 33 bytes (SSID) + 64 bytes (password) = 98 bytes
#define WIFI_CRED_1_ADDR 10   // 10-107
#define WIFI_CRED_2_ADDR 108  // 108-205
#define WIFI_CRED_3_ADDR 206  // 206-303

// Button press timing
#define BLE_BUTTON_LONGPRESS_TIME 3000  // 3 seconds for BLE mode activation
unsigned long bleButtonPressStartTime = 0;
bool buttonPressed = false;

// BLE Service and Characteristic UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define DEVICEINFO_CHAR_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define WIFI_CONFIG_CHAR_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"

// BLE variables
BLEServer* pServer = NULL;
BLECharacteristic* pDeviceInfoCharacteristic = NULL;
BLECharacteristic* pWiFiConfigCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool bleMode = false;

// WiFi credentials
struct WiFiCredential {
  bool valid;
  char ssid[33];      // WiFi SSID can be up to 32 chars + null terminator
  char password[64];  // WiFi password can be up to 63 chars + null terminator
};

WiFiCredential storedCredentials[3]; // Store 3 WiFi credential pairs
int currentCredentialIndex = 0;      // Currently used credential index

// MQTT Broker settings
const char* mqtt_server = "1a87ae45965c44f3abbd2d523241f1f1.s2.eu.hivemq.cloud";
const char* mqtt_username = "homato";
const char* mqtt_password = "HomeAutomation@2025";
const int mqtt_port = 8883;  // TLS port
char mqtt_client_id[50]; // Will be set based on DEVICE_ID
bool mqttInitialized = false;

// Heartbeat and sensor intervals
const int heartbeatInterval = 30000; // 30 seconds

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

// MQTT client objects for PubSubClient
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);


//CA Certificate for www.howsmyssl.com (valid until 04-07-2021)
// NTP Server data
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // IST timezone (UTC+5:30)
const int daylightOffset_sec = 0; // No DST in India

// Forward declarations
void initBLEServer();
void startBLEMode();
void stopBLEMode();
bool setup_wifi();
int findNextCredentialSlot();
void saveWiFiCredentialToEEPROM(int slotIndex, const char* newSSID, const char* newPassword);
void initMQTTTopics();
void reconnectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);

// Function to read WiFi credentials from EEPROM
void readWiFiCredentialsFromEEPROM() {
  // Check if WiFi config is initialized
  byte isWiFiInitialized = EEPROM.read(INSTALLATION_STATUS_ADDR);
  
  if (isWiFiInitialized != 0x01) {
    Serial.println("No stored WiFi credentials found.");
    // Initialize all slots as invalid
    for (int i = 0; i < 3; i++) {
      storedCredentials[i].valid = false;
      memset(storedCredentials[i].ssid, 0, 33);
      memset(storedCredentials[i].password, 0, 64);
    }
    return;
  }
  
  // Read stored WiFi credentials
  Serial.println("Reading WiFi credentials from EEPROM:");
  
  int credentialAddrs[3] = {WIFI_CRED_1_ADDR, WIFI_CRED_2_ADDR, WIFI_CRED_3_ADDR};
  
  for (int i = 0; i < 3; i++) {
    int baseAddr = credentialAddrs[i];
    
    // Read valid flag
    storedCredentials[i].valid = EEPROM.read(baseAddr) == 0x01;
    
    if (storedCredentials[i].valid) {
      // Read SSID (33 bytes including null terminator)
      for (int j = 0; j < 33; j++) {
        storedCredentials[i].ssid[j] = EEPROM.read(baseAddr + 1 + j);
      }
      
      // Read password (64 bytes including null terminator)
      for (int j = 0; j < 64; j++) {
        storedCredentials[i].password[j] = EEPROM.read(baseAddr + 1 + 33 + j);
      }
      
      Serial.print("Slot ");
      Serial.print(i + 1);
      Serial.print(": SSID='");
      Serial.print(storedCredentials[i].ssid);
      // Serial.println("', Password=[stored]");
      Serial.println("', Password='");
      Serial.println(storedCredentials[i].password);
      Serial.println("'");
      
    } else {
      Serial.print("Slot ");
      Serial.print(i + 1);
      Serial.println(": Empty");
    }
  }
}

// Function to read states from EEPROM
void readRelayStatesFromEEPROM() {
  // Check if EEPROM has been initialized
  byte isInitialized = EEPROM.read(EEPROM_INITIALIZED_ADDR);
  
  if (isInitialized == 0xFF) {
    // EEPROM not initialized, set default values
    RelayStates[0] = false;
    RelayStates[1] = false;
    RelayStates[2] = false;
    RelayStates[3] = false;
    RelayStates[4] = false;
    RelayStates[5] = false;
    RelayStates[6] = false;
    RelayStates[7] = false;
    
    // Save default values to EEPROM
    EEPROM.write(RELAY_PIN_1_STATE_ADDR, RelayStates[0] ? 1 : 0);
    EEPROM.write(RELAY_PIN_2_STATE_ADDR, RelayStates[1] ? 1 : 0);
    EEPROM.write(RELAY_PIN_3_STATE_ADDR, RelayStates[2] ? 1 : 0);
    EEPROM.write(RELAY_PIN_4_STATE_ADDR, RelayStates[3] ? 1 : 0);
    EEPROM.write(RELAY_PIN_5_STATE_ADDR, RelayStates[4] ? 1 : 0);
    EEPROM.write(RELAY_PIN_6_STATE_ADDR, RelayStates[5] ? 1 : 0);
    EEPROM.write(RELAY_PIN_7_STATE_ADDR, RelayStates[6] ? 1 : 0);
    EEPROM.write(RELAY_PIN_8_STATE_ADDR, RelayStates[7] ? 1 : 0);
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01); // Mark as initialized
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with default values");
  } else {
    // Read values from EEPROM
    RelayStates[0] = EEPROM.read(RELAY_PIN_1_STATE_ADDR) == 1;
    RelayStates[1] = EEPROM.read(RELAY_PIN_2_STATE_ADDR) == 1;
    RelayStates[2] = EEPROM.read(RELAY_PIN_3_STATE_ADDR) == 1;
    RelayStates[3] = EEPROM.read(RELAY_PIN_4_STATE_ADDR) == 1;
    RelayStates[4] = EEPROM.read(RELAY_PIN_5_STATE_ADDR) == 1;
    RelayStates[5] = EEPROM.read(RELAY_PIN_6_STATE_ADDR) == 1;
    RelayStates[6] = EEPROM.read(RELAY_PIN_7_STATE_ADDR) == 1;
    RelayStates[7] = EEPROM.read(RELAY_PIN_8_STATE_ADDR) == 1;
    
    Serial.println("Read states from EEPROM");
    Serial.println("Relay1 State: " + String(RelayStates[0] ? "ON" : "OFF"));
    Serial.println("Relay2 State: " + String(RelayStates[1] ? "ON" : "OFF"));
    Serial.println("Relay3 State: " + String(RelayStates[2] ? "ON" : "OFF"));
    Serial.println("Relay4 State: " + String(RelayStates[3] ? "ON" : "OFF"));
    Serial.println("Relay5 State: " + String(RelayStates[4] ? "ON" : "OFF"));
    Serial.println("Relay6 State: " + String(RelayStates[5] ? "ON" : "OFF"));
    Serial.println("Relay7 State: " + String(RelayStates[6] ? "ON" : "OFF"));
    Serial.println("Relay8 State: " + String(RelayStates[7] ? "ON" : "OFF"));
  }
}

// BLE Server callbacks
class HomatoBleServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client connected");
    digitalWrite(BleLed, HIGH); // Turn on LED when connected
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client disconnected");
    digitalWrite(BleLed, LOW); // Turn off LED when disconnected
    
    // Restart advertising when disconnected to allow new connections
    if (bleMode) {
      pServer->getAdvertising()->start();
      Serial.println("BLE advertising restarted");
    }
  }
};

// WiFi Config Characteristic callback
class WiFiConfigCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // Simple approach to work with the ESP32 BLE library
    String receivedData = "";
    String value = pCharacteristic->getValue();
    
    // Check if we received data
    if (value.length() > 0) {
      // Convert std::string to Arduino String safely
      for (int i = 0; i < value.length(); i++) {
        receivedData += value[i];
      }
    }
    
    if (receivedData.length() > 0) {
      Serial.println("*********");
      Serial.print("WiFi Config received: ");
      
      // Format should be "SSID:password"
      int separatorPos = receivedData.indexOf('*');
      
      if (separatorPos != -1) {
        String newSSID = receivedData.substring(0, separatorPos);
        String newPassword = receivedData.substring(separatorPos + 1);
        
        // Store the new WiFi credentials in the next slot
        int nextSlot = findNextCredentialSlot();
        // Update storedCredentials array
        storedCredentials[nextSlot].valid = true;
        strncpy(storedCredentials[nextSlot].ssid, newSSID.c_str(), 32);
        storedCredentials[nextSlot].ssid[32] = 0; // Ensure null termination
        strncpy(storedCredentials[nextSlot].password, newPassword.c_str(), 63);
        storedCredentials[nextSlot].password[63] = 0; // Ensure null termination
        
        
        stopBLEMode();
        Serial.println("Attempting to connect with the new WiFi credentials");
        if (!setup_wifi()) {
          Serial.println("Failed to connect with new WiFi credentials. Press BLE button to enter BLE mode.");
          return;
        }

        
        saveWiFiCredentialToEEPROM(nextSlot, newSSID.c_str(), newPassword.c_str());
        Serial.print("Saved SSID: ");
        Serial.print(newSSID);
        Serial.print(" with password, to slot: ");
        Serial.println(nextSlot);

        // Send confirmation
        String response = "WiFi credentials saved to slot " + String(nextSlot + 1);
        pCharacteristic->setValue(response.c_str());
        pCharacteristic->notify();

        if (EEPROM.read(INSTALLATION_STATUS_ADDR) != 0x01) {
          EEPROM.write(INSTALLATION_STATUS_ADDR, 0x01);
          EEPROM.commit();
          Serial.println("Device installation status marked as initialized in EEPROM");
        }
        Serial.println("Restarting ESP32 to apply configuration...");
        delay(1000); // Give time for serial message to be sent
        ESP.restart();
      }
    } else {
      Serial.println("Invalid format. Use 'SSID:password'");
      pCharacteristic->setValue("Error: Invalid format. Use 'SSID:password'");
      pCharacteristic->notify();
    }
    Serial.println("*********");
  }
};

// Initialize BLE Server
void initBLEServer() {
  // Initialize BLE Device
  BLEDevice::init("homato-"+String(DEVICE_ID)); // Use the same client ID for BLE
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new HomatoBleServerCallbacks());
  
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create Device Info Characteristic
  pDeviceInfoCharacteristic = pService->createCharacteristic(
                                DEVICEINFO_CHAR_UUID,
                                BLECharacteristic::PROPERTY_READ |
                                BLECharacteristic::PROPERTY_NOTIFY
                              );
  pDeviceInfoCharacteristic->addDescriptor(new BLE2902());
  
  // Add device info (device ID, number of relays, and initialization status)
  bool installationStatus = (EEPROM.read(INSTALLATION_STATUS_ADDR) == 0x01);
  String deviceInfoStr = String("DeviceID:") + String(DEVICE_ID) + 
                        ",Relays:" + String(NUM_RELAYS) + 
                        ",Initialized:" + String(installationStatus ? "Yes" : "No");
  pDeviceInfoCharacteristic->setValue(deviceInfoStr.c_str());
  
  // Create WiFi Config Characteristic
  pWiFiConfigCharacteristic = pService->createCharacteristic(
                                 WIFI_CONFIG_CHAR_UUID,
                                 BLECharacteristic::PROPERTY_WRITE |
                                 BLECharacteristic::PROPERTY_NOTIFY
                               );
  pWiFiConfigCharacteristic->setCallbacks(new WiFiConfigCallbacks());
  pWiFiConfigCharacteristic->addDescriptor(new BLE2902());
  
  // Start the service
  pService->start();
}

// Start BLE Mode
void startBLEMode() {
  if (!bleMode) {
    Serial.println("Starting BLE Mode");

    // Disconnect MQTT if connected
    if (mqttInitialized && mqttClient.connected()) {
      mqttClient.disconnect();
      Serial.println("MQTT client stopped before entering BLE mode");
    }

    bleMode = true;
    
    // Initialize BLE Server if not already initialized
    if (pServer == NULL) {
      initBLEServer();
    }
    
    // Start advertising
    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // Min connection interval
    pAdvertising->setMaxPreferred(0x12);  // Max connection interval
    pAdvertising->start();
    
    Serial.println("BLE Advertising started");
    Serial.println("Waiting for WiFi configuration via BLE...");
    
    digitalWrite(BleLed, HIGH); // Keep LED on while in BLE mode
  }
  
}

// Stop BLE Mode
void stopBLEMode() {
  if (bleMode) {
    Serial.println("Stopping BLE Mode");
    bleMode = false;
    
    // Stop advertising
    if (pServer != NULL) {
      pServer->getAdvertising()->stop();
    }
    
    digitalWrite(BleLed, LOW); // Turn off BLE LED
  }
}

// Check for BLE button press
void checkBLEButton() {
  // Check if button is pressed
  if (digitalRead(BleTurnOnPin) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      bleButtonPressStartTime = millis();
      Serial.println("BLE button pressed");
    } else {
      // Check if button has been held for required time
      if ((millis() - bleButtonPressStartTime) > BLE_BUTTON_LONGPRESS_TIME) {
        if (!bleMode) {
          Serial.println("Long press detected, activating BLE mode");
          startBLEMode();
        }
        // Reset button state to prevent repeated triggers
        buttonPressed = false;
      }
    }
  } else if (buttonPressed) {
    // Button released before long press time
    buttonPressed = false;
    Serial.println("BLE button released");
  }
}

// Check if we have valid WiFi credentials
bool hasValidWiFiCredentials() {
  for (int i = 0; i < 3; i++) {
    if (storedCredentials[i].valid) {
      return true;
    }
  }
  return false;
}

// Find next available credential slot (or overwrite oldest)
int findNextCredentialSlot() {
  // First check for empty slots
  for (int i = 0; i < 3; i++) {
    if (!storedCredentials[i].valid) {
      return i;
    }
  }
  
  // If all slots are full, overwrite the first one (rotating buffer)
  return 0;
}

// Save WiFi credential to EEPROM
void saveWiFiCredentialToEEPROM(int slotIndex, const char* newSSID, const char* newPassword) {
  if (slotIndex < 0 || slotIndex > 2) {
    Serial.println("Invalid credential slot index");
    return;
  }
  
  int baseAddr;
  switch (slotIndex) {
    case 0: baseAddr = WIFI_CRED_1_ADDR; break;
    case 1: baseAddr = WIFI_CRED_2_ADDR; break;
    case 2: baseAddr = WIFI_CRED_3_ADDR; break;
  }
  
  // Set valid flag
  EEPROM.write(baseAddr, 0x01);
  
  // Write SSID (up to 32 chars + null)
  int len = strlen(newSSID);
  if (len > 32) len = 32;
  
  for (int i = 0; i < 33; i++) {
    if (i < len) {
      EEPROM.write(baseAddr + 1 + i, newSSID[i]);
    } else {
      EEPROM.write(baseAddr + 1 + i, 0);
    }
  }
  
  // Write password (up to 63 chars + null)
  len = strlen(newPassword);
  if (len > 63) len = 63;
  
  for (int i = 0; i < 64; i++) {
    if (i < len) {
      EEPROM.write(baseAddr + 1 + 33 + i, newPassword[i]);
    } else {
      EEPROM.write(baseAddr + 1 + 33 + i, 0);
    }
  }
  
  // Mark WiFi config as initialized
  EEPROM.write(INSTALLATION_STATUS_ADDR, 0x01);
  
  // Commit the changes
  EEPROM.commit();
  
  Serial.println("WiFi credential saved to EEPROM");
}

// Function to connect to WiFi using stored credentials
bool setup_wifi() {
  delay(10);
  
  // If no stored credentials, enter BLE mode to wait for configuration
  if (!hasValidWiFiCredentials()) {
    Serial.println("No valid WiFi credentials found. Enter BLE mode to configure.");
    return false;
  }
  
  // Try connecting with each stored credential
  bool connected = false;
  
  for (int attempt = 0; attempt < 3; attempt++) {
    // Loop through all stored credentials
    for (int i = 0; i < 3; i++) {
      if (storedCredentials[i].valid) {
        Serial.print("Connecting to WiFi network: ");
        Serial.println(storedCredentials[i].ssid);
        Serial.print("Attempt ");
        Serial.print(attempt + 1);
        Serial.print(", Credential slot ");
        Serial.print(i + 1);
        Serial.println("...");
        
        WiFi.begin(storedCredentials[i].ssid, storedCredentials[i].password);
        
        int tries = 0;
        while (WiFi.status() != WL_CONNECTED && tries < 20) {
          delay(500);
          Serial.print(".");
          tries++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("");
          Serial.println("WiFi connected!");
          Serial.print("IP address: ");
          Serial.println(WiFi.localIP());
          
          // Turn on WiFi LED to show connected status
          digitalWrite(WifiLed, HIGH);
          
          // Set the current index to the successful credential
          currentCredentialIndex = i;
          
          connected = true;
          break;
        } else {
          storedCredentials[i].valid = false;
          Serial.println("");
          Serial.print("Failed to connect to ");
          Serial.println(storedCredentials[i].ssid);
        }
      }
    }
    
    if (connected) break;
  }
  
  if (!connected) {
    Serial.println("Could not connect with any stored credentials. Press BLE button to enter BLE mode.");
    // Turn off WiFi LED to show disconnected status
    digitalWrite(WifiLed, LOW);
    return false;
  } 
  return true;
}

// This function is replaced by mqttCallback()

// Function to update a specific relay
void updateRelayState(int relayNumber, bool state) {
  switch (relayNumber) {
    case 1:
      digitalWrite(RelayPins[0], state ? LOW : HIGH);
      Serial.println("Updated relay 1 state: " + String(state ? "ON" : "OFF"));
      break;
    case 2:
      digitalWrite(RelayPins[1], state ? LOW : HIGH);
      Serial.println("Updated relay 2 state: " + String(state ? "ON" : "OFF"));
      break;
    case 3:
      digitalWrite(RelayPins[2], state ? LOW : HIGH);
      Serial.println("Updated relay 3 state: " + String(state ? "ON" : "OFF"));
      break;
    case 4:
      digitalWrite(RelayPins[3], state ? LOW : HIGH);
      Serial.println("Updated relay 4 state: " + String(state ? "ON" : "OFF"));
      break;
    case 5:
      digitalWrite(RelayPins[4], state ? LOW : HIGH);
      Serial.println("Updated relay 5 state: " + String(state ? "ON" : "OFF"));
      break;
    case 6:
      digitalWrite(RelayPins[5], state ? LOW : HIGH);
      Serial.println("Updated relay 6 state: " + String(state ? "ON" : "OFF"));
      break;
    case 7:
      digitalWrite(RelayPins[6], state ? LOW : HIGH);
      Serial.println("Updated relay 7 state: " + String(state ? "ON" : "OFF"));
      break;
    case 8:
      digitalWrite(RelayPins[7], state ? LOW : HIGH);
      Serial.println("Updated relay 8 state: " + String(state ? "ON" : "OFF"));
      break;
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

// Function to publish a specific relay state
void publishRelayState(int relayNumber) {
  char payload[128];
  StaticJsonDocument<128> doc;
  
  // Relay index is 0-based, relay number is 1-based
  int relayIndex = relayNumber - 1;
  if (relayIndex < 0 || relayIndex >= NUM_RELAYS) {
    Serial.println("Invalid relay number");
    return;
  }
  
  doc["state"] = (RelayStates[relayIndex]) ? "ON" : "OFF";
  doc["device_id"] = DEVICE_ID;
  doc["source"] = "device"; // Add source identifier to indicate this came from the physical device
  serializeJson(doc, payload);
  
  bool published = false;
  
  switch(relayNumber) {
    case 1:
      published = mqttClient.publish(mqtt_topic_relay1, payload, true); // retain=true
      break;
    case 2:
      published = mqttClient.publish(mqtt_topic_relay2, payload, true);
      break;
    case 3:
      published = mqttClient.publish(mqtt_topic_relay3, payload, true);
      break;
    case 4:
      published = mqttClient.publish(mqtt_topic_relay4, payload, true);
      break;
    case 5:
      published = mqttClient.publish(mqtt_topic_relay5, payload, true);
      break;
    case 6:
      published = mqttClient.publish(mqtt_topic_relay6, payload, true);
      break;
    case 7:
      published = mqttClient.publish(mqtt_topic_relay7, payload, true);
      break;
    case 8:
      published = mqttClient.publish(mqtt_topic_relay8, payload, true);
      break;
    default:
      Serial.println("Invalid relay number");
      break;
  }
  
  if (published) {
    Serial.print("Published relay ");
    Serial.print(relayNumber);
    Serial.print(" state to MQTT: ");
    Serial.println(RelayStates[relayIndex] ? "ON" : "OFF");
  } else {
    Serial.print("Failed to publish relay ");
    Serial.print(relayNumber);
    Serial.println(" state to MQTT");
  }
}

// Function to apply all relay states at once (for initialization)
void applyAllRelayStates() {
  updateRelayState(1, RelayStates[0]);
  updateRelayState(2, RelayStates[1]);
  updateRelayState(3, RelayStates[2]);
  updateRelayState(4, RelayStates[3]);
  updateRelayState(5, RelayStates[4]);
  updateRelayState(6, RelayStates[5]);
  updateRelayState(7, RelayStates[6]);
  updateRelayState(8, RelayStates[7]);
  Serial.println("Applied all relay states");
}

// Function to read physical switch states and sync with relay states
void checkPhysicalSwitchState() {
  Serial.println("Reading physical switch positions...");
  
  for (int i = 0; i < NUM_RELAYS; i++) {
    // Read the current switch position (INPUT_PULLUP means LOW = pressed/on)
    int reading = digitalRead(SwitchPins[i]);
    bool switchPressed = (reading == LOW);
    
    // Store current switch state for future reference
    SwitchStates[i] = switchPressed;
    
    // Based on the original implementation, if a switch is pressed (LOW),
    // we toggle the corresponding relay state rather than directly setting it
    if (switchPressed) {
      // Toggle relay state (opposite of current state)
      bool newRelayState = !RelayStates[i];
      
      Serial.print("Switch ");
      Serial.print(i+1);
      Serial.print(" is pressed, toggling relay from ");
      Serial.print(RelayStates[i] ? "ON" : "OFF");
      Serial.print(" to ");
      Serial.println(newRelayState ? "ON" : "OFF");
      
      // Update relay state
      RelayStates[i] = newRelayState;
      
      // Physically update the relay
      updateRelayState(i+1, newRelayState);
      
      // Save new state to EEPROM
      saveRelayStateToEEPROM(i+1, newRelayState);
    }
  }
  
  Serial.println("Physical switch reading complete");
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
  if (mqttClient.connected()) {
    bool published = mqttClient.publish(mqtt_topic_availability, "online", true); // retain=true
    if (published) {
      Serial.println("Availability status published");
    } else {
      Serial.println("Failed to publish availability status");
    }
  } else {
    Serial.println("Cannot publish availability - MQTT not connected");
  }
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

// Button state tracking variables
bool buttonPrevStates[NUM_RELAYS] = {HIGH}; // Initialize all to HIGH (not pressed)

// Basic toggle switch implementation without debounce
void manual_control() {
  static unsigned long lastToggleTime = 0;
  
  // Only check buttons every 50ms to avoid excessive polling
  if (millis() - lastToggleTime < 50) {
    return;
  }
  lastToggleTime = millis();
  
  for (int i = 0; i < NUM_RELAYS; i++) {
    // Read current state
    bool currentState = digitalRead(SwitchPins[i]);
    
    // Toggle on FALLING edge only (when button is just pressed)
    if (currentState == LOW && buttonPrevStates[i] == HIGH) {
      // Toggle the relay state
      RelayStates[i] = !RelayStates[i];
      
      // Apply relay state (LOW=ON, HIGH=OFF because relays are active LOW)
      digitalWrite(RelayPins[i], RelayStates[i] ? LOW : HIGH);
      
      // Update MQTT and EEPROM
      publishRelayState(i+1);
      saveRelayStateToEEPROM(i+1, RelayStates[i]);
      
      // Log state change
      Serial.print("Button ");
      Serial.print(i+1);
      Serial.print(" toggled relay to: ");
      Serial.println(RelayStates[i] ? "ON" : "OFF");
    }
    
    // Save current state for next comparison
    buttonPrevStates[i] = currentState;
  }
}


void setup() {
  Serial.begin(115200);
  Serial.println(F("Homato v1 starting"));
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Initialize pins
  pinMode(RelayPins[0], OUTPUT);
  pinMode(RelayPins[1], OUTPUT);
  pinMode(RelayPins[2], OUTPUT);
  pinMode(RelayPins[3], OUTPUT);
  pinMode(RelayPins[4], OUTPUT);
  pinMode(RelayPins[5], OUTPUT);
  pinMode(RelayPins[6], OUTPUT);
  pinMode(RelayPins[7], OUTPUT);
  
  // Set all relays to OFF initially (HIGH for active LOW relays)
  digitalWrite(RelayPins[0], HIGH);
  digitalWrite(RelayPins[1], HIGH);
  digitalWrite(RelayPins[2], HIGH);
  digitalWrite(RelayPins[3], HIGH);
  digitalWrite(RelayPins[4], HIGH);
  digitalWrite(RelayPins[5], HIGH);
  digitalWrite(RelayPins[6], HIGH);
  digitalWrite(RelayPins[7], HIGH);
  
  pinMode(SwitchPins[0], INPUT_PULLUP);
  pinMode(SwitchPins[1], INPUT_PULLUP);
  pinMode(SwitchPins[2], INPUT_PULLUP);
  pinMode(SwitchPins[3], INPUT_PULLUP);
  pinMode(SwitchPins[4], INPUT_PULLUP);
  pinMode(SwitchPins[5], INPUT_PULLUP);
  pinMode(SwitchPins[6], INPUT_PULLUP);
  pinMode(SwitchPins[7], INPUT_PULLUP);
  
  pinMode(BleTurnOnPin, INPUT_PULLUP);

  pinMode(BleLed, OUTPUT);
  pinMode(WifiLed, OUTPUT);
  
  // Initialize watchdog timer with 30-second timeout
  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = 30000,              // 30 seconds timeout
    .idle_core_mask = 0,             // No core is idle
    .trigger_panic = true            // Trigger panic when timeout
  };
  esp_task_wdt_init(&wdt_config);
  esp_task_wdt_add(NULL); 

  // Check if EEPROM has been initialized before
  bool eepromInitialized = EEPROM.read(EEPROM_INITIALIZED_ADDR) == 1;
   
  // Read relay states from EEPROM (if previously initialized)
  if (eepromInitialized) {
    readRelayStatesFromEEPROM();
    Serial.println("Relay states loaded from EEPROM");
  } else {
    Serial.println("EEPROM not initialized, using default states");
    EEPROM.write(EEPROM_INITIALIZED_ADDR, 1);
    EEPROM.commit();
  }
  
  // Explicitly apply all relay states to ensure they match the stored states
  applyAllRelayStates();
  Serial.println("All relay states applied");

  // Check physical switch states and apply relay states
  checkPhysicalSwitchState();

  // Read Device initialized flag from EEPROM
  bool deviceInstallationStatus = EEPROM.read(INSTALLATION_STATUS_ADDR) == 1;
  
  // Read WiFi credentials from EEPROM
  if (!deviceInstallationStatus) {
    Serial.println("Device not initialized. Skipping WiFi setup and MQTT connection.");
    return;
  }
  readWiFiCredentialsFromEEPROM();
  Serial.println("Valid WiFi credentials found. Connecting to WiFi.");
  if (!setup_wifi()) {
    Serial.println("Failed to connect to WiFi. Press BLE button to enter BLE mode.");
    return;
  }

  // Set device-specific MQTT client ID and topics
  snprintf(mqtt_client_id, sizeof(mqtt_client_id), "homato-%s", DEVICE_ID);
  
  // Initialize MQTT topics
  initMQTTTopics();
  
  // Configure MQTT client with PubSubClient
  setupMQTT();
  mqttInitialized = true;

  // Initialize time from NTP server
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  esp_task_wdt_reset();

  // Check for BLE button press
  checkBLEButton();
  
  // BLE connection handling
  if (bleMode) {
    // Handle BLE disconnection/reconnection
    if (!deviceConnected && oldDeviceConnected) {
      delay(500); // Give the bluetooth stack time to get ready
      pServer->startAdvertising(); // Restart advertising
      Serial.println("BLE advertising restarted");
      oldDeviceConnected = deviceConnected;
    }
    // Handle new connection
    if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
    }
    
    // In BLE mode, we're waiting for WiFi credentials, so skip other operations
    return;
  }
  
  // Normal WiFi+MQTT operation mode
  // Check WiFi connection and reconnect if needed
  if (WiFi.status() != WL_CONNECTED) {
    static unsigned long lastWiFiReconnectAttempt = 0;
    unsigned long now = millis();
    if (now - lastWiFiReconnectAttempt > 30000) { // Try every 30 seconds
      lastWiFiReconnectAttempt = now;
      Serial.println("WiFi connection lost. Attempting to reconnect...");
      setup_wifi();
    }
  }

  manual_control();
  // Handle MQTT connection in normal operation mode
  if (!bleMode && WiFi.status() == WL_CONNECTED) {
    if (!mqttClient.connected()) {
      reconnectMQTT();
    }
    mqttClient.loop(); // Process incoming messages
  }
  
  // Add a heartbeat message every 30 seconds
  static unsigned long lastHeartbeat = 0;
  if (millis() - lastHeartbeat > heartbeatInterval && mqttClient.connected()) {
    mqttClient.publish(mqtt_topic_availability, "online", true); // retain=true
    Serial.println("Heartbeat sent at " + String(millis()));
    lastHeartbeat = millis();
  }
}

// MQTT callback function for PubSubClient
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Create strings for topic and payload
  String topicStr = String(topic);
  
  // Convert payload to a null-terminated string
  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';
  String payloadStr = String(message);
  
  Serial.printf("Message arrived on [%s]: %s\n", topicStr.c_str(), payloadStr.c_str());
  
  // Parse the received JSON message
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payloadStr);
  
  // First check if this is a valid JSON message
  if (!error) {
    // Check if this message originated from this device
    const char* source = doc["source"];
    const char* msgDeviceId = doc["device_id"];
    
    // Only ignore messages from this device itself (source="device" AND matching device_id)
    // Messages from the backend (source="backend") should always be processed
    if (source && strcmp(source, "device") == 0 && 
        msgDeviceId && strcmp(msgDeviceId, DEVICE_ID) == 0) {
      Serial.println("Ignoring message that originated from this device");
      return; // Skip processing to avoid feedback loops
    }
    
    // Log source of message for debugging
    if (source && strcmp(source, "backend") == 0) {
      Serial.print("Processing message from source: ");
      Serial.println(source);
    }
  }
  
  if (error) {
    Serial.println("Failed to parse JSON message");
    return;
  } else {
    // Process JSON format message
    bool stateChanged = false;
    int changedRelay = 0;

    // Get the state from the JSON message
    if (!doc.containsKey("state")) {
      Serial.println("Message missing 'state' field");
      return;
    }
      
    String stateStr = doc["state"];
    bool newState = (stateStr == "ON");
      
    // Process the state change based on the topic
    if (topicStr == mqtt_topic_relay1) {
      if (RelayStates[0] != newState) {
        RelayStates[0] = newState;
        stateChanged = true;
        changedRelay = 1;
      }
    }
    else if (topicStr == mqtt_topic_relay2) {
      if (RelayStates[1] != newState) {
        RelayStates[1] = newState;
        stateChanged = true;
        changedRelay = 2;
      }
    }
    else if (topicStr == mqtt_topic_relay3) {
      if (RelayStates[2] != newState) {
        RelayStates[2] = newState;
        stateChanged = true;
        changedRelay = 3;
      }
    }
    else if (topicStr == mqtt_topic_relay4) {
      if (RelayStates[3] != newState) {
        RelayStates[3] = newState;
        stateChanged = true;
        changedRelay = 4;
      }
    }
    else if (topicStr == mqtt_topic_relay5) {
      if (RelayStates[4] != newState) {
        RelayStates[4] = newState;
        stateChanged = true;
        changedRelay = 5;
      }
    }
    else if (topicStr == mqtt_topic_relay6) {
      if (RelayStates[5] != newState) {
        RelayStates[5] = newState;
        stateChanged = true;
        changedRelay = 6;
      }
    }
    else if (topicStr == mqtt_topic_relay7) {
      if (RelayStates[6] != newState) {
        RelayStates[6] = newState;
        stateChanged = true;
        changedRelay = 7;
      }
    }
    else if (topicStr == mqtt_topic_relay8) {
      if (RelayStates[7] != newState) {
        RelayStates[7] = newState;
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
}

// Function to initialize MQTT topics
void initMQTTTopics() {
  // Define the topics based on device ID
  snprintf(mqtt_topic_relay1, sizeof(mqtt_topic_relay1), "homato/%s/relay/1", DEVICE_ID);
  snprintf(mqtt_topic_relay2, sizeof(mqtt_topic_relay2), "homato/%s/relay/2", DEVICE_ID);
  snprintf(mqtt_topic_relay3, sizeof(mqtt_topic_relay3), "homato/%s/relay/3", DEVICE_ID);
  snprintf(mqtt_topic_relay4, sizeof(mqtt_topic_relay4), "homato/%s/relay/4", DEVICE_ID);
  snprintf(mqtt_topic_relay5, sizeof(mqtt_topic_relay5), "homato/%s/relay/5", DEVICE_ID);
  snprintf(mqtt_topic_relay6, sizeof(mqtt_topic_relay6), "homato/%s/relay/6", DEVICE_ID);
  snprintf(mqtt_topic_relay7, sizeof(mqtt_topic_relay7), "homato/%s/relay/7", DEVICE_ID);
  snprintf(mqtt_topic_relay8, sizeof(mqtt_topic_relay8), "homato/%s/relay/8", DEVICE_ID);
  snprintf(mqtt_topic_availability, sizeof(mqtt_topic_availability), "homato/%s/availability", DEVICE_ID);
}


const char root_ca[] PROGMEM =
"-----BEGIN CERTIFICATE-----\n"
"MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\n"
"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
"DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\n"
"PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\n"
"Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\n"
"AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\n"
"rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\n"
"OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\n"
"xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\n"
"7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\n"
"aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\n"
"HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\n"
"SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\n"
"ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\n"
"AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\n"
"R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\n"
"JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\n"
"Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n"
"-----END CERTIFICATE-----\n";

// Function to setup MQTT
void setupMQTT() {
  // Set server and callback
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(MQTT_KEEPALIVE);
  mqttClient.setBufferSize(512);  // Increase buffer size for handling larger messages
  
  // Set secure client
  wifiClient.setCACert(root_ca);
  wifiClient.setInsecure();  // Allow self-signed certificates - remove this line after testing if secure mode works
  
  // Connect to broker
  reconnectMQTT();
}

// Function to handle MQTT connections and reconnections
void reconnectMQTT() {
  // Loop until we're reconnected
  if (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect with LWT
    // Note: PubSubClient only supports QoS 0 and 1, not 2
    if (mqttClient.connect(mqtt_client_id, mqtt_username, mqtt_password, 
                          mqtt_topic_availability, 2, true, "offline")) {
      Serial.println("connected");
      
      // Publish online status
      mqttClient.publish(mqtt_topic_availability, "online", true);
      
      // Subscribe to all relay topics
      mqttClient.subscribe(mqtt_topic_relay1);
      mqttClient.subscribe(mqtt_topic_relay2);
      mqttClient.subscribe(mqtt_topic_relay3);
      mqttClient.subscribe(mqtt_topic_relay4);
      mqttClient.subscribe(mqtt_topic_relay5);
      mqttClient.subscribe(mqtt_topic_relay6);
      mqttClient.subscribe(mqtt_topic_relay7);
      mqttClient.subscribe(mqtt_topic_relay8);
      
      // Publish initial states
      publishAllRelayStates();
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" will try again later");
    }
  }
}
