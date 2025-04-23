import { useEffect, useState } from "react";
import { Text, View, StyleSheet, ScrollView, RefreshControl, Alert, Platform } from "react-native";
import { Ionicons } from "@expo/vector-icons";
import { BlurView } from "expo-blur";
import { Switch } from "react-native";
import * as Haptics from "expo-haptics";
import { useHomeAutomation, DeviceType, HomeAutomationProvider } from "./contexts/HomeAutomationContext";

// Define device icon types to match Ionicons
type IconName = React.ComponentProps<typeof Ionicons>['name'];

// Device configuration with icons and labels
const DEVICES = [
  { id: "switch", name: "Main Switch", icon: "power" as IconName },
  { id: "light", name: "Light", icon: "bulb" as IconName },
  { id: "fan", name: "Fan", icon: "fan" as IconName },
  { id: "tubelight", name: "Tube Light", icon: "flashlight" as IconName },
  { id: "bedlight", name: "Bed Light", icon: "bed" as IconName },
  { id: "falseceiling", name: "False Ceiling Light", icon: "cloud" as IconName },
  { id: "ac", name: "AC", icon: "snow" as IconName },
  { id: "switchport", name: "Switch Port", icon: "flash" as IconName },
];

// Main app component
export default function App() {
  return (
    <HomeAutomationProvider>
      <DevicesScreen />
    </HomeAutomationProvider>
  );
}

function DevicesScreen() {
  // Get state and functions from context
  const { 
    deviceState, 
    isConnected, 
    controlDevice, 
    isLoading, 
    refreshConnection, 
    activityLog 
  } = useHomeAutomation();
  
  const [refreshing, setRefreshing] = useState(false);

  const onRefresh = async () => {
    setRefreshing(true);
    refreshConnection();
    setTimeout(() => {
      setRefreshing(false);
    }, 1500);
  };
  
  const toggleDevice = (device: DeviceType) => {
    if (!deviceState.deviceConnected) {
      Alert.alert("Error", "Device is offline, cannot process command");
      return;
    }
    
    const currentState = deviceState[device];
    const newState = currentState === "ON" ? "OFF" : "ON";
    
    // Provide haptic feedback
    Haptics.impactAsync(Haptics.ImpactFeedbackStyle.Medium);
    
    // Call context method to toggle device
    controlDevice(device, newState as "ON" | "OFF");
  };
  
  const getDeviceColor = (device: string) => {
    const deviceKey = device as DeviceType;
    if (!deviceState.deviceConnected) return "#888";
    return deviceState[deviceKey] === "ON" ? "#4CAF50" : "#ccc";
  };

  return (
    <View style={styles.container}>
      <View style={styles.header}>
        <Text style={styles.title}>Homato Control System</Text>
        <View style={[styles.connectionStatus, { backgroundColor: deviceState.deviceConnected ? "#4CAF50" : "#F44336" }]}>
          <Text style={styles.connectionText}>
            {deviceState.deviceConnected ? "Connected" : "Disconnected"}
          </Text>
        </View>
      </View>

      <ScrollView 
        style={styles.devicesContainer}
        contentContainerStyle={styles.devicesContent}
        refreshControl={
          <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
        }
      >
        {DEVICES.map((device) => (
          <BlurView 
            key={device.id} 
            intensity={90} 
            tint="light" 
            style={[
              styles.deviceCard,
              { borderColor: getDeviceColor(device.id) }
            ]}
          >
            <View style={styles.deviceInfo}>
              <View style={[styles.deviceIcon, { backgroundColor: getDeviceColor(device.id) }]}>
                <Ionicons name={device.icon} size={24} color="#fff" />
              </View>
              <View>
                <Text style={styles.deviceName}>{device.name}</Text>
                <Text style={styles.deviceStatus}>
                  {deviceState.deviceConnected ? deviceState[device.id as DeviceType] : "Unknown"}
                </Text>
              </View>
            </View>
            <Switch
              value={deviceState[device.id as DeviceType] === "ON"}
              onValueChange={() => toggleDevice(device.id as DeviceType)}
              disabled={!deviceState.deviceConnected}
              trackColor={{ false: "#767577", true: "#81b0ff" }}
              thumbColor={deviceState[device.id as DeviceType] === "ON" ? "#4CAF50" : "#f4f3f4"}
              ios_backgroundColor="#3e3e3e"
            />
          </BlurView>
        ))}
        
        <View style={styles.logContainer}>
          <Text style={styles.logTitle}>Activity Log</Text>
          <View style={styles.logItems}>
            {activityLog.map((log, index) => (
              <Text key={index} style={styles.logEntry}>
                <Text style={styles.logTime}>{log.time}</Text> - {log.message}
              </Text>
            ))}
            {activityLog.length === 0 && (
              <Text style={styles.logEmpty}>No activity yet</Text>
            )}
          </View>
        </View>
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#f6f8fa",
  },
  header: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    paddingHorizontal: 16,
    paddingTop: 12,
    paddingBottom: 12,
    backgroundColor: "#fff",
    borderBottomWidth: 1,
    borderBottomColor: "#e1e4e8",
  },
  title: {
    fontSize: 18,
    fontWeight: "600",
    color: "#24292e",
  },
  connectionStatus: {
    paddingVertical: 4,
    paddingHorizontal: 8,
    borderRadius: 16,
  },
  connectionText: {
    color: "#fff",
    fontSize: 12,
    fontWeight: "bold",
  },
  devicesContainer: {
    flex: 1,
  },
  devicesContent: {
    padding: 16,
  },
  deviceCard: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    padding: 16,
    borderRadius: 12,
    marginBottom: 16,
    borderWidth: 1,
    backgroundColor: "rgba(255, 255, 255, 0.7)",
    ...Platform.select({
      ios: {
        shadowColor: "#000",
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
      },
      android: {
        elevation: 3,
      },
    }),
  },
  deviceInfo: {
    flexDirection: "row",
    alignItems: "center",
  },
  deviceIcon: {
    width: 40,
    height: 40,
    borderRadius: 20,
    alignItems: "center",
    justifyContent: "center",
    marginRight: 12,
  },
  deviceName: {
    fontSize: 16,
    fontWeight: "600",
    color: "#24292e",
    marginBottom: 4,
  },
  deviceStatus: {
    fontSize: 14,
    color: "#586069",
  },
  logContainer: {
    backgroundColor: "#fff",
    borderRadius: 12,
    padding: 16,
    marginTop: 8,
    ...Platform.select({
      ios: {
        shadowColor: "#000",
        shadowOffset: { width: 0, height: 2 },
        shadowOpacity: 0.1,
        shadowRadius: 4,
      },
      android: {
        elevation: 2,
      },
    }),
  },
  logTitle: {
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 12,
    color: "#24292e",
  },
  logItems: {
    maxHeight: 200,
  },
  logEntry: {
    fontSize: 12,
    color: "#586069",
    marginBottom: 6,
  },
  logTime: {
    color: "#0366d6",
    fontWeight: "500",
  },
  logEmpty: {
    fontSize: 12,
    color: "#586069",
    fontStyle: "italic",
  },
});
