import { View, Text, StyleSheet, ScrollView, TouchableOpacity, Switch, Linking } from "react-native";
import { useState } from "react";
import { Ionicons } from "@expo/vector-icons";
import { BlurView } from "expo-blur";

// Define icon type to match Ionicons
type IconName = React.ComponentProps<typeof Ionicons>['name'];

export default function SettingsScreen() {
  const [settings, setSettings] = useState({
    notifications: true,
    darkMode: false,
    hapticFeedback: true,
    autoConnect: true,
    debugMode: false,
  });

  const toggleSetting = (key: keyof typeof settings) => {
    setSettings((prev) => ({
      ...prev,
      [key]: !prev[key],
    }));
  };

  const SettingItem = ({ 
    icon, 
    title, 
    description, 
    toggleKey, 
    value 
  }: { 
    icon: IconName; 
    title: string; 
    description: string; 
    toggleKey: keyof typeof settings; 
    value: boolean; 
  }) => (
    <View style={styles.settingItem}>
      <View style={styles.settingInfo}>
        <View style={styles.iconContainer}>
          <Ionicons name={icon} size={22} color="#fff" />
        </View>
        <View>
          <Text style={styles.settingTitle}>{title}</Text>
          <Text style={styles.settingDescription}>{description}</Text>
        </View>
      </View>
      <Switch
        value={value}
        onValueChange={() => toggleSetting(toggleKey)}
        trackColor={{ false: "#767577", true: "#81b0ff" }}
        thumbColor={value ? "#0366d6" : "#f4f3f4"}
        ios_backgroundColor="#3e3e3e"
      />
    </View>
  );

  return (
    <ScrollView style={styles.container} contentContainerStyle={styles.content}>
      <Text style={styles.header}>Settings</Text>
      
      <BlurView intensity={80} tint="light" style={styles.section}>
        <Text style={styles.sectionTitle}>App Preferences</Text>
        
        <SettingItem
          icon={"notifications" as IconName}
          title="Notifications"
          description="Receive alerts for device status changes"
          toggleKey="notifications"
          value={settings.notifications}
        />
        
        <SettingItem
          icon={"moon" as IconName}
          title="Dark Mode"
          description="Switch to dark color theme"
          toggleKey="darkMode"
          value={settings.darkMode}
        />
        
        <SettingItem
          icon={"hand-right" as IconName} 
          title="Haptic Feedback"
          description="Vibration when toggling controls"
          toggleKey="hapticFeedback"
          value={settings.hapticFeedback}
        />
      </BlurView>
      
      <BlurView intensity={80} tint="light" style={styles.section}>
        <Text style={styles.sectionTitle}>Connection</Text>
        
        <View style={styles.serverInfo}>
          <Text style={styles.serverLabel}>Server URL</Text>
          <Text style={styles.serverValue}>http://54.226.69.130:3000</Text>
        </View>
        
        <SettingItem
          icon={"flash" as IconName}
          title="Auto Connect"
          description="Connect to server automatically on startup"
          toggleKey="autoConnect"
          value={settings.autoConnect}
        />
        
        <SettingItem
          icon={"bug" as IconName}
          title="Debug Mode"
          description="Show detailed logs and connection info"
          toggleKey="debugMode"
          value={settings.debugMode}
        />
      </BlurView>
      
      <BlurView intensity={80} tint="light" style={styles.section}>
        <Text style={styles.sectionTitle}>About</Text>
        
        <View style={styles.aboutItem}>
          <Text style={styles.aboutLabel}>App Version</Text>
          <Text style={styles.aboutValue}>1.0.0</Text>
        </View>
        
        <View style={styles.aboutItem}>
          <Text style={styles.aboutLabel}>Device Name</Text>
          <Text style={styles.aboutValue}>Homato Control</Text>
        </View>
        
        <TouchableOpacity style={styles.aboutButton}>
          <Ionicons name={"help-circle" as IconName} size={18} color="#0366d6" />
          <Text style={styles.aboutButtonText}>Help & Support</Text>
        </TouchableOpacity>
        
        <TouchableOpacity style={styles.aboutButton}>
          <Ionicons name={"information-circle" as IconName} size={18} color="#0366d6" />
          <Text style={[styles.aboutButtonText, {textDecorationLine: 'underline'}]} onPress={() => Linking.openURL('http://54.226.69.130:3000')}>Server URL</Text>
        </TouchableOpacity>
      </BlurView>
      
      <TouchableOpacity style={styles.resetButton}>
        <Text style={styles.resetButtonText}>Reset All Settings</Text>
      </TouchableOpacity>
      
      <Text style={styles.footer}>
        Homato Control System v1.0.0
      </Text>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#f6f8fa",
  },
  content: {
    padding: 16,
    paddingBottom: 32,
  },
  header: {
    fontSize: 24,
    fontWeight: "bold",
    color: "#24292e",
    marginBottom: 24,
  },
  section: {
    borderRadius: 12,
    marginBottom: 24,
    overflow: "hidden",
    borderWidth: 1,
    borderColor: "#e1e4e8",
    padding: 16,
    backgroundColor: "rgba(255, 255, 255, 0.8)",
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: "600",
    color: "#24292e",
    marginBottom: 16,
  },
  settingItem: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: "#eaecef",
  },
  settingInfo: {
    flexDirection: "row",
    alignItems: "center",
    flex: 1,
    marginRight: 16,
  },
  iconContainer: {
    width: 36,
    height: 36,
    borderRadius: 18,
    backgroundColor: "#0366d6",
    alignItems: "center",
    justifyContent: "center",
    marginRight: 12,
  },
  settingTitle: {
    fontSize: 16,
    fontWeight: "500",
    color: "#24292e",
    marginBottom: 2,
  },
  settingDescription: {
    fontSize: 14,
    color: "#586069",
  },
  serverInfo: {
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: "#eaecef",
  },
  serverLabel: {
    fontSize: 14,
    color: "#586069",
    marginBottom: 4,
  },
  serverValue: {
    fontSize: 16,
    fontWeight: "500",
    color: "#24292e",
  },
  aboutItem: {
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: "#eaecef",
  },
  aboutLabel: {
    fontSize: 14,
    color: "#586069",
    marginBottom: 4,
  },
  aboutValue: {
    fontSize: 16,
    fontWeight: "500",
    color: "#24292e",
  },
  aboutButton: {
    flexDirection: "row",
    alignItems: "center",
    paddingVertical: 12,
    borderBottomWidth: 1,
    borderBottomColor: "#eaecef",
  },
  aboutButtonText: {
    marginLeft: 8,
    fontSize: 16,
    color: "#0366d6",
  },
  resetButton: {
    padding: 16,
    backgroundColor: "#ffdce0",
    borderRadius: 8,
    alignItems: "center",
    marginTop: 8,
  },
  resetButtonText: {
    color: "#d73a49",
    fontWeight: "600",
    fontSize: 16,
  },
  footer: {
    marginTop: 24,
    textAlign: "center",
    fontSize: 14,
    color: "#6a737d",
  },
}); 