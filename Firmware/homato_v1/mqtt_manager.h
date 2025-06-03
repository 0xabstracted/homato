#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <functional>

// Prevent name conflicts with PubSubClient connection status macros
#ifdef MQTT_CONNECTED
#undef MQTT_CONNECTED
#endif
#ifdef MQTT_DISCONNECTED
#undef MQTT_DISCONNECTED
#endif
#ifdef MQTT_CONNECTION_LOST
#undef MQTT_CONNECTION_LOST
#endif
#ifdef MQTT_CONNECTION_TIMEOUT
#undef MQTT_CONNECTION_TIMEOUT
#endif

// MQTT Connection States
enum MQTTConnectionState {
  MQTT_DISCONNECTED = 0,
  MQTT_CONNECTING = 1,
  MQTT_CONNECTED = 2,
  MQTT_CONNECTION_LOST = 3,
  MQTT_ERROR = 4,
  MQTT_AUTHENTICATION_FAILED = 5,
  MQTT_NETWORK_ERROR = 6,
  MQTT_TIMEOUT = 7
};

// MQTT Configuration Structure
struct MQTTConfig {
  const char* server;
  int port;
  const char* username;
  const char* password;
  const char* deviceId;
  const char* clientIdPrefix;
  int keepAlive;
  int bufferSize;
  int maxRetries;
  unsigned long reconnectBackoffMs;
  unsigned long maxReconnectBackoffMs;
  bool enableTLS;
  bool verifyCertificate;
};

// MQTT Topic Structure for organized topic management
struct MQTTTopics {
  char availability[64];
  char relay[8][64];  // Topics for 8 relays
  char deviceStatus[64];
  char commands[64];
  char config[64];
};

// MQTT Manager Class
class MQTTManager {
private:
  // Core MQTT components
  WiFiClientSecure* secureClient;
  PubSubClient* mqttClient;
  MQTTConfig config;
  MQTTTopics topics;
  
  // Connection state management
  MQTTConnectionState connectionState;
  bool isInitialized;
  unsigned long lastConnectionAttempt;
  unsigned long currentReconnectBackoff;
  int reconnectAttempts;
  char lastError[256];
  
  // Retry and backoff management
  unsigned long lastHeartbeat;
  unsigned long heartbeatInterval;
  bool lwt_enabled;
  
  // Message callback function
  std::function<void(String, String)> messageCallback;
  
  // Certificate management
  const char* rootCA;
  bool certificateSet;
  
  // Internal helper methods
  void resetConnectionState();
  void updateConnectionState(MQTTConnectionState newState, const char* error = nullptr);
  bool validateConfiguration();
  void setupTLS();
  void generateClientId(char* clientId, size_t size);
  void initializeTopics();
  bool attemptConnection();
  void handleConnectionError(int result);
  void calculateBackoff();
  void subscribeToTopics();
  void publishInitialStates();
  
  // Static callback wrapper for PubSubClient
  static void staticCallback(char* topic, byte* payload, unsigned int length);
  static MQTTManager* instance;

public:
  // Constructor and Destructor
  MQTTManager();
  ~MQTTManager();
  
  // Core lifecycle methods
  bool initialize(const MQTTConfig& config);
  bool connect();
  bool disconnect();
  void update(); // Called in main loop for connection management
  bool isConnected();
  
  // Publishing methods
  bool publishRelayState(int relayNumber, bool state, const char* source = "device");
  bool publishDeviceStatus(const char* status);
  bool publishAvailability(bool online);
  bool publishHeartbeat();
  bool publishMessage(const char* topic, const char* payload, bool retain = false);
  
  // Subscription methods
  bool subscribeToRelay(int relayNumber);
  bool subscribeToCommands();
  bool subscribeToConfig();
  bool subscribeToTopic(const char* topic);
  
  // Status and configuration methods
  MQTTConnectionState getConnectionState() const;
  const char* getConnectionStateString() const;
  const char* getLastError() const;
  unsigned long getLastConnectionAttempt() const;
  int getReconnectAttempts() const;
  float getConnectionUptime() const;
  
  // Certificate and security methods
  bool setCACertificate(const char* certificate);
  bool setInsecureMode(bool insecure);
  
  // Callback management
  void setMessageCallback(std::function<void(String, String)> callback);
  
  // Configuration and topic management
  const MQTTTopics& getTopics() const;
  bool updateConfig(const MQTTConfig& newConfig);
  
  // Error handling and diagnostics
  bool performConnectionTest();
  void printDiagnostics();
  void clearErrors();
  
  // Advanced features
  bool enableLastWillTestament(const char* topic, const char* message, int qos = 1, bool retain = true);
  bool setHeartbeatInterval(unsigned long intervalMs);
  
  // Static instance access for callback
  static MQTTManager* getInstance();
};

// Helper functions for configuration
MQTTConfig createDefaultMQTTConfig(const char* deviceId);
bool validateMQTTConfig(const MQTTConfig& config);

// Error code definitions
#define MQTT_ERROR_NOT_INITIALIZED      -100
#define MQTT_ERROR_INVALID_CONFIG       -101
#define MQTT_ERROR_NETWORK_UNAVAILABLE  -102
#define MQTT_ERROR_CERTIFICATE_INVALID  -103
#define MQTT_ERROR_AUTHENTICATION       -104
#define MQTT_ERROR_CONNECTION_TIMEOUT   -105
#define MQTT_ERROR_MAX_RETRIES_EXCEEDED -106

#endif // MQTT_MANAGER_H