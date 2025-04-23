import { View, Text, StyleSheet, FlatList, TouchableOpacity } from "react-native";
import { Ionicons } from "@expo/vector-icons";
import { BlurView } from "expo-blur";
import { useState } from "react";

// Define device icon types to match Ionicons
type IconName = React.ComponentProps<typeof Ionicons>['name'];

// Dummy scenes for the UI mockup
const DEMO_SCENES = [
  {
    id: "1",
    name: "Movie Mode",
    icon: "film" as IconName,
    devices: [
      { id: "light", state: "OFF" },
      { id: "fan", state: "ON" },
      { id: "tubelight", state: "OFF" },
      { id: "bedlight", state: "ON" },
      { id: "falseceiling", state: "ON" },
    ],
  },
  {
    id: "2",
    name: "Sleep Mode",
    icon: "moon" as IconName,
    devices: [
      { id: "light", state: "OFF" },
      { id: "fan", state: "ON" },
      { id: "tubelight", state: "OFF" },
      { id: "bedlight", state: "OFF" },
      { id: "falseceiling", state: "OFF" },
    ],
  },
  {
    id: "3",
    name: "Working Mode",
    icon: "laptop" as IconName,
    devices: [
      { id: "light", state: "ON" },
      { id: "fan", state: "ON" },
      { id: "tubelight", state: "ON" },
      { id: "bedlight", state: "OFF" },
      { id: "falseceiling", state: "OFF" },
    ],
  },
];

export default function ScenesScreen() {
  const [scenes] = useState(DEMO_SCENES);

  return (
    <View style={styles.container}>
      <Text style={styles.header}>Scenes</Text>
      <Text style={styles.subheader}>
        Create and manage presets to control multiple devices at once
      </Text>
      
      <FlatList
        data={scenes}
        keyExtractor={(item) => item.id}
        contentContainerStyle={styles.listContent}
        renderItem={({ item }) => (
          <BlurView intensity={80} tint="light" style={styles.sceneCard}>
            <View style={styles.sceneHeader}>
              <View style={styles.sceneIconContainer}>
                <Ionicons name={item.icon} size={24} color="#fff" />
              </View>
              <Text style={styles.sceneName}>{item.name}</Text>
            </View>
            
            <View style={styles.deviceList}>
              {item.devices.map((device) => (
                <View key={device.id} style={styles.deviceItem}>
                  <Text style={styles.deviceName}>
                    {device.id.charAt(0).toUpperCase() + device.id.slice(1)}
                  </Text>
                  <View style={[
                    styles.deviceStatus, 
                    { backgroundColor: device.state === "ON" ? "#4CAF50" : "#F44336" }
                  ]}>
                    <Text style={styles.deviceStatusText}>{device.state}</Text>
                  </View>
                </View>
              ))}
            </View>
            
            <View style={styles.sceneFooter}>
              <TouchableOpacity style={styles.sceneButton}>
                <Ionicons name="play" size={16} color="#fff" />
                <Text style={styles.sceneButtonText}>Activate</Text>
              </TouchableOpacity>
              <TouchableOpacity style={[styles.sceneButton, styles.editButton]}>
                <Ionicons name="create" size={16} color="#fff" />
                <Text style={styles.sceneButtonText}>Edit</Text>
              </TouchableOpacity>
            </View>
          </BlurView>
        )}
        ListFooterComponent={
          <TouchableOpacity style={styles.addSceneButton}>
            <Ionicons name="add-circle" size={24} color="#0366d6" />
            <Text style={styles.addSceneText}>Create New Scene</Text>
          </TouchableOpacity>
        }
      />
      
      <View style={styles.comingSoon}>
        <Text style={styles.comingSoonText}>
          Scene creation and editing coming soon!
        </Text>
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#f6f8fa",
    padding: 16,
  },
  header: {
    fontSize: 24,
    fontWeight: "bold",
    color: "#24292e",
    marginBottom: 8,
  },
  subheader: {
    fontSize: 14,
    color: "#586069",
    marginBottom: 24,
  },
  listContent: {
    paddingBottom: 20,
  },
  sceneCard: {
    borderRadius: 12,
    marginBottom: 16,
    overflow: "hidden",
    borderWidth: 1,
    borderColor: "#e1e4e8",
    backgroundColor: "rgba(255, 255, 255, 0.8)",
  },
  sceneHeader: {
    flexDirection: "row",
    alignItems: "center",
    padding: 16,
    borderBottomWidth: 1,
    borderBottomColor: "#e1e4e8",
  },
  sceneIconContainer: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: "#0366d6",
    alignItems: "center",
    justifyContent: "center",
    marginRight: 12,
  },
  sceneName: {
    fontSize: 18,
    fontWeight: "600",
    color: "#24292e",
  },
  deviceList: {
    padding: 16,
  },
  deviceItem: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    paddingVertical: 8,
    borderBottomWidth: 1,
    borderBottomColor: "#eaecef",
  },
  deviceName: {
    fontSize: 14,
    color: "#24292e",
  },
  deviceStatus: {
    paddingHorizontal: 8,
    paddingVertical: 4,
    borderRadius: 12,
  },
  deviceStatusText: {
    fontSize: 12,
    color: "#fff",
    fontWeight: "bold",
  },
  sceneFooter: {
    flexDirection: "row",
    padding: 16,
    borderTopWidth: 1,
    borderTopColor: "#e1e4e8",
  },
  sceneButton: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    backgroundColor: "#0366d6",
    paddingVertical: 8,
    paddingHorizontal: 16,
    borderRadius: 6,
    marginRight: 12,
  },
  editButton: {
    backgroundColor: "#6f42c1",
  },
  sceneButtonText: {
    color: "#fff",
    fontWeight: "600",
    marginLeft: 6,
  },
  addSceneButton: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "center",
    padding: 16,
    borderWidth: 2,
    borderStyle: "dashed",
    borderColor: "#0366d6",
    borderRadius: 12,
    backgroundColor: "rgba(3, 102, 214, 0.1)",
  },
  addSceneText: {
    marginLeft: 8,
    fontSize: 16,
    fontWeight: "600",
    color: "#0366d6",
  },
  comingSoon: {
    padding: 12,
    backgroundColor: "#fffbdd",
    borderRadius: 6,
    borderLeftWidth: 4,
    borderLeftColor: "#ffd33d",
    marginTop: 16,
  },
  comingSoonText: {
    fontSize: 14,
    color: "#24292e",
  },
}); 