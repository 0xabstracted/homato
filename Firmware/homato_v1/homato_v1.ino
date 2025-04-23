#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include <time.h>  // Add time library
#include <DHT.h>  
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Device Identifier - Unique ID for each device
const char* DEVICE_ID = "st-000002";

// Number of relays on this device
const int NUM_RELAYS = 8;

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
#define WIFI_CONFIG_INITIALIZED_ADDR 9

// WiFi credentials storage addresses
// Each credential takes 1 byte (valid flag) + 33 bytes (SSID) + 64 bytes (password) = 98 bytes
#define WIFI_CRED_1_ADDR 10   // 10-107
#define WIFI_CRED_2_ADDR 108  // 108-205
#define WIFI_CRED_3_ADDR 206  // 206-303

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


// Button for BLE pairing mode
#define BLE_BUTTON_PIN 0  // Usually GPIO0 is the BOOT button on most ESP32 boards
#define BLE_LED_PIN 2     // Usually the built-in LED

// Button press timing
#define BUTTON_LONGPRESS_TIME 5000  // 5 seconds for BLE mode activation
unsigned long buttonPressStartTime = 0;
bool buttonPressed = false;


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

// Forward declarations
void startBLEMode();
void setup_wifi();
void stopBLEMode();
void initBLEServer();
int findNextCredentialSlot();
void saveWiFiCredential(int slotIndex, const char* newSSID, const char* newPassword);

// BLE Server callbacks
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("BLE Client connected");
    digitalWrite(BLE_LED_PIN, HIGH); // Turn on LED when connected
  };

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("BLE Client disconnected");
    digitalWrite(BLE_LED_PIN, LOW); // Turn off LED when disconnected
    
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
        saveWiFiCredential(nextSlot, newSSID.c_str(), newPassword.c_str());
        
        Serial.print("Saved SSID: ");
        Serial.print(newSSID);
        Serial.print(" with password, to slot: ");
        Serial.println(nextSlot);

        // Send confirmation
        String response = "WiFi credentials saved to slot " + String(nextSlot + 1);
        pCharacteristic->setValue(response.c_str());
        pCharacteristic->notify();
        
        // Add a delay to make sure BLE notification has time to be sent
        delay(500);
        
        // Try connecting to WiFi with the new credentials
        Serial.println("Attempting to connect with the new WiFi credentials");
        stopBLEMode();
        // setup_wifi();
        Serial.println("Restarting ESP32 to apply new WiFi configuration...");
        delay(1000);  // Wait 1 second
        ESP.restart();  
        
      } else {
        Serial.println("Invalid format. Use 'SSID:password'");
        pCharacteristic->setValue("Error: Invalid format. Use 'SSID:password'");
        pCharacteristic->notify();
      }
      Serial.println("*********");
    }
  }
};

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


void sendSensor(){
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

// Function to read WiFi credentials from EEPROM
void readWiFiCredentials() {
  // Check if WiFi config is initialized
  byte isWiFiInitialized = EEPROM.read(WIFI_CONFIG_INITIALIZED_ADDR);
  
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
      Serial.println("', Password=[stored]");
    } else {
      Serial.print("Slot ");
      Serial.print(i + 1);
      Serial.println(": Empty");
    }
  }
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

// Check if we have valid WiFi credentials
bool hasValidWiFiCredentials() {
  for (int i = 0; i < 3; i++) {
    if (storedCredentials[i].valid) {
      return true;
    }
  }
  return false;
}

// Save WiFi credential to EEPROM
void saveWiFiCredential(int slotIndex, const char* newSSID, const char* newPassword) {
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
  
  // Update storedCredentials array
  storedCredentials[slotIndex].valid = true;
  strncpy(storedCredentials[slotIndex].ssid, newSSID, 32);
  storedCredentials[slotIndex].ssid[32] = 0; // Ensure null termination
  strncpy(storedCredentials[slotIndex].password, newPassword, 63);
  storedCredentials[slotIndex].password[63] = 0; // Ensure null termination
  
  // Mark WiFi config as initialized
  EEPROM.write(WIFI_CONFIG_INITIALIZED_ADDR, 0x01);
  
  // Commit the changes
  EEPROM.commit();
  
  Serial.println("WiFi credential saved to EEPROM");
}
// Initialize BLE Server
void initBLEServer() {
  // Initialize BLE Device
  BLEDevice::init(mqtt_client_id); // Use the same client ID for BLE
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
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
  bool isInitialized = (EEPROM.read(EEPROM_INITIALIZED_ADDR) == 0x01);
  String deviceInfoStr = String("DeviceID:") + String(DEVICE_ID) + 
                        ",Relays:" + String(NUM_RELAYS) + 
                        ",Initialized:" + String(isInitialized ? "Yes" : "No");
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

// Function to connect to WiFi using stored credentials
void setup_wifi() {
  delay(10);
  
  // If no stored credentials, enter BLE mode to wait for configuration
  if (!hasValidWiFiCredentials()) {
    Serial.println("No valid WiFi credentials found. Enter BLE mode to configure.");
    startBLEMode();
    return;
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
          digitalWrite(wifiLed, HIGH);
          
          // Set the current index to the successful credential
          currentCredentialIndex = i;
          
          connected = true;
          break;
        } else {
          Serial.println("");
          Serial.print("Failed to connect to ");
          Serial.println(storedCredentials[i].ssid);
        }
      }
    }
    
    if (connected) break;
  }
  
  if (!connected) {
    Serial.println("Could not connect with any stored credentials. Enter BLE mode.");
    // Turn off WiFi LED to show disconnected status
    digitalWrite(wifiLed, LOW);
    startBLEMode();
  } else {
    // Successfully connected, exit BLE mode if it was active
    if (bleMode) {
      stopBLEMode();
    }
    
    // Mark device as initialized after successful WiFi connection
    if (EEPROM.read(EEPROM_INITIALIZED_ADDR) != 0x01) {
      EEPROM.write(EEPROM_INITIALIZED_ADDR, 0x01);
      EEPROM.commit();
      Serial.println("Device marked as initialized in EEPROM");
      
      // Restart ESP32 after successful initial connection and configuration
      Serial.println("Restarting ESP32 to apply configuration...");
      delay(1000); // Give time for serial message to be sent
      ESP.restart();
    }
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
  
  Serial.println("Published relay " + String(relayNumber) + " state to MQTT: " + String(state ? "ON" : "OFF"));
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
  Serial.println(millis());
  // Serial.println(getFormattedTime());

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
  Serial.println("Published availability status: online at " + millis());
  // Serial.println("Published availability status: online at " + getFormattedTime());
}

void publishTemperature() {
  client.publish(mqtt_topic_temperature, String(temperature1).c_str(), false);
  Serial.println("Published temperature: " + String(temperature1) + "°C at " + millis());
  // Serial.println("Published temperature: " + String(temperature1) + "°C at " + getFormattedTime());
}

void publishHumidity() {
  client.publish(mqtt_topic_humidity, String(humidity1).c_str(), false);
  Serial.println("Published humidity: " + String(humidity1) + "% at " + millis());
  // Serial.println("Published humidity: " + String(humidity1) + "% at " + getFormattedTime());
}

void publishLDR() {
  client.publish(mqtt_topic_ldr, String(ldrVal).c_str(), false);
  Serial.println("Published LDR value: " + String(ldrVal) + " at " + millis());
  // Serial.println("Published LDR value: " + String(ldrVal) + " at " + getFormattedTime());
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

void manual_control(){
  if (digitalRead(SwitchPin1) == LOW) {
    digitalWrite(RelayPin1, RelayState1);
    RelayState1 = !RelayState1;
    publishRelayState(1);
    saveRelayStateToEEPROM(1, RelayState1); // Save state to EEPROM
    delay(300); 
  }
  if (digitalRead(SwitchPin2) == LOW) {
    digitalWrite(RelayPin2, RelayState2);
    RelayState2 = !RelayState2;
    publishRelayState(2);
    saveRelayStateToEEPROM(2, RelayState2); // Save state to EEPROM
    delay(300);
  }
  if (digitalRead(SwitchPin3) == LOW) {
    digitalWrite(RelayPin3, RelayState3);
    RelayState3 = !RelayState3;
    publishRelayState(3);
    saveRelayStateToEEPROM(3, RelayState3); // Save state to EEPROM
    delay(300);
  }
  if (digitalRead(SwitchPin4) == LOW) {
    digitalWrite(RelayPin4, RelayState4);
    RelayState4 = !RelayState4;
    publishRelayState(4);
    saveRelayStateToEEPROM(4, RelayState4); // Save state to EEPROM
    delay(300); 
  }
  if (digitalRead(SwitchPin5) == LOW) {
    digitalWrite(RelayPin5, RelayState5);
    RelayState5 = !RelayState5;
    publishRelayState(5);
    saveRelayStateToEEPROM(5, RelayState5); // Save state to EEPROM
    delay(300); 
  }
  if (digitalRead(SwitchPin6) == LOW) {
    digitalWrite(RelayPin6, RelayState6);
    RelayState6 = !RelayState6;
    publishRelayState(6);
    saveRelayStateToEEPROM(6, RelayState6); // Save state to EEPROM
    delay(300);
  }
  if (digitalRead(SwitchPin7) == LOW) {
    digitalWrite(RelayPin7, RelayState7);
    RelayState7 = !RelayState7;
    publishRelayState(7);
    saveRelayStateToEEPROM(7, RelayState7); // Save state to EEPROM
    delay(300);
  }
  if (digitalRead(SwitchPin8) == LOW) {
    digitalWrite(RelayPin8, RelayState8);
    RelayState8 = !RelayState8;
    publishRelayState(8);
    saveRelayStateToEEPROM(8, RelayState8); // Save state to EEPROM
    delay(300); 
  }
}

// Start BLE Mode
void startBLEMode() {
  if (!bleMode) {
    Serial.println("Starting BLE Mode");
    bleMode = true;
    
    // Initialize BLE Server if not already initialized
    if (pServer == NULL) {
      initBLEServer();
    }
    
    // Start advertising
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Advertising started");
    Serial.println("Waiting for WiFi configuration via BLE...");
    
    // Blink LED to indicate BLE mode
    for (int i = 0; i < 5; i++) {
      digitalWrite(BLE_LED_PIN, HIGH);
      delay(100);
      digitalWrite(BLE_LED_PIN, LOW);
      delay(100);
    }
    digitalWrite(BLE_LED_PIN, HIGH); // Keep LED on while in BLE mode
  }
}

// Stop BLE Mode
void stopBLEMode() {
  if (bleMode) {
    Serial.println("Stopping BLE Mode");
    bleMode = false;
    
    // Stop advertising
    if (pServer != NULL) {
      BLEDevice::getAdvertising()->stop();
    }
    
    digitalWrite(BLE_LED_PIN, LOW); // Turn off BLE LED
  }
}

// Check for BLE button press
void checkBLEButton() {
  // Check if button is pressed
  if (digitalRead(BLE_BUTTON_PIN) == LOW) {
    if (!buttonPressed) {
      buttonPressed = true;
      buttonPressStartTime = millis();
      Serial.println("BLE button pressed");
    } else {
      // Check if button has been held for required time
      if ((millis() - buttonPressStartTime) > BUTTON_LONGPRESS_TIME) {
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
  pinMode(BLE_LED_PIN, OUTPUT);
  pinMode(BLE_BUTTON_PIN, INPUT_PULLUP);

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
  digitalWrite(BLE_LED_PIN, LOW);
  digitalWrite(wifiLed, LOW);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Read saved relay states from EEPROM
  readStatesFromEEPROM();
  
  // Read saved WiFi credentials from EEPROM
  readWiFiCredentials();
  
  // Apply saved states to relays
  applyAllRelayStates();
  
  // Check if initialization is done (device has been set up previously)
  bool deviceInitialized = (EEPROM.read(EEPROM_INITIALIZED_ADDR) == 0x01);
  
  // Check if BLE button is pressed during boot (manual override)
  if (digitalRead(BLE_BUTTON_PIN) == LOW) {
    Serial.println("BLE button pressed during boot, entering BLE mode");
    startBLEMode();
    delay(1000); // Wait for button release
  } 
  // If device is not initialized or no WiFi credentials, start in BLE mode
  else if (!deviceInitialized || !hasValidWiFiCredentials()) {
    Serial.println("First boot or no valid WiFi credentials. Starting in BLE mode.");
    startBLEMode();
  } 
  // Otherwise connect to WiFi with stored credentials
  else {
    Serial.println("Device previously initialized. Connecting to WiFi.");
    setup_wifi();
  }
  
  // Configure secure client with extended timeout
  espClient.setInsecure(); // For testing only
  // For production, use proper certificate validation:
  // espClient.setCACert(root_ca);
  espClient.setTimeout(5000); // Increase timeout to 15 seconds for slow connections
  
  // Set up MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  
  // Set maximum packet size to support larger payloads if needed
  client.setBufferSize(512);

  dht.begin();
}

void loop() {
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
      Serial.println("Heartbeat sent at " + millis());
      // Serial.println("Heartbeat sent at " + getFormattedTime());
    }
    lastHeartbeat = millis();
  }

  // static unsigned long lastSensorPublish = 0;
  // if (millis() - lastSensorPublish > sendSensorInterval) {
  //   sendSensor();
  //   lastSensorPublish = millis();
  // }
}

// Function to reconnect to MQTT broker
void reconnect() {
  // Limit reconnection attempts to prevent blocking the main loop for too long
  int attempts = 0;
  const int MAX_ATTEMPTS = 3;
  
  while (!client.connected() && attempts < MAX_ATTEMPTS) {
    attempts++;
    Serial.print("\nClient ID: " + String(mqtt_client_id));
    
    // Try to resolve the host name first to verify DNS is working
    IPAddress ip;
    Serial.print("\nResolving MQTT broker hostname...");
    if (WiFi.hostByName(mqtt_server, ip)) {
      Serial.print(" Success! IP: ");
      Serial.println(ip.toString());
    } else {
      Serial.println(" Failed to resolve hostname. Check DNS or broker address.");
      delay(5000);
      continue;
    }
    
    // Configure LWT (Last Will and Testament)
    if (client.connect(mqtt_client_id, mqtt_username, mqtt_password, 
                      mqtt_topic_availability, 2, true, "offline")) {
      Serial.println("connected!");
      
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
      return; // Success - exit the function
    } else {
      int errorCode = client.state();
      Serial.print("failed, rc=" + String(errorCode) + " (");
      
      // Print detailed error message
      switch(errorCode) {
        case -4: Serial.print("MQTT_CONNECTION_TIMEOUT"); break;
        case -3: Serial.print("MQTT_CONNECTION_LOST"); break;
        case -2: Serial.print("MQTT_CONNECT_FAILED"); break;
        case -1: Serial.print("MQTT_DISCONNECTED"); break;
        case 1: Serial.print("MQTT_CONNECT_BAD_PROTOCOL"); break;
        case 2: Serial.print("MQTT_CONNECT_BAD_CLIENT_ID"); break;
        case 3: Serial.print("MQTT_CONNECT_UNAVAILABLE"); break;
        case 4: Serial.print("MQTT_CONNECT_BAD_CREDENTIALS"); break;
        case 5: Serial.print("MQTT_CONNECT_UNAUTHORIZED"); break;
        default: Serial.print("MQTT_UNKNOWN_ERROR"); break;
      }
      Serial.println(") trying again in 5 seconds");
      delay(5000);
    }
  }
  
  if (!client.connected()) {
    Serial.println("Failed to connect to MQTT after multiple attempts. Will retry later.");
  }
}