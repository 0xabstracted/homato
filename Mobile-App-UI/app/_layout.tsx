import { Tabs } from "expo-router";
import { Ionicons } from "@expo/vector-icons";
import { AppProvider } from "./context/AppContext";
import { View, StyleSheet } from "react-native";

const styles = StyleSheet.create({
  tabBar: {
    height: 60,
    paddingBottom: 5,
    paddingTop: 5,
    backgroundColor: 'white',
    borderTopWidth: 1,
    borderTopColor: '#dee2e6',
  },
  tabLabel: {
    fontSize: 12,
    fontWeight: '500',
  }
});

export default function AppLayout() {
  return (
    <AppProvider>
      <Tabs screenOptions={{
        tabBarStyle: styles.tabBar,
        tabBarActiveTintColor: '#007bff',
        tabBarInactiveTintColor: '#6c757d',
        tabBarLabelStyle: styles.tabLabel,
        headerShown: true,
      }}>
      <Tabs.Screen
        name="index"
        options={{
          title: "Devices",
          tabBarIcon: ({ color }) => <Ionicons name="home" size={24} color={color} />,
          headerTitle: "Homato Control",
        }}
      />
      <Tabs.Screen
        name="scenes"
        options={{
          title: "Scenes",
          tabBarIcon: ({ color }) => <Ionicons name="layers" size={24} color={color} />,
        }}
      />
      <Tabs.Screen
        name="settings"
        options={{
          title: "Settings",
          tabBarIcon: ({ color }) => <Ionicons name="settings" size={24} color={color} />,
        }}
      />
    </Tabs>
    </AppProvider>
  );
}
