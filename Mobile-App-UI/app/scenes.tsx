import React from "react";
import { View, Text, StyleSheet, ScrollView } from "react-native";
import { useAppContext } from "./context/AppContext";

// Connection status bar component
function ConnectionStatusBar() {
  const { isConnected } = useAppContext();
  
  return (
    <View style={[styles.statusBar, isConnected ? styles.connected : styles.disconnected]}>
      <Text style={styles.statusText}>
        {isConnected ? 'Connected to server' : 'Disconnected from server'}
      </Text>
    </View>
  );
}

export default function ScenesScreen() {
  return (
    <View style={styles.container}>
      <ConnectionStatusBar />
      <ScrollView style={styles.screenContainer}>
        <Text style={styles.screenTitle}>Scenes</Text>
        <View style={styles.messageContainer}>
          <Text style={styles.messageText}>No scenes configured</Text>
          <Text style={styles.messageSubtext}>Scenes will be available in a future update</Text>
        </View>
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f8f9fa',
  },
  statusBar: {
    padding: 8,
    alignItems: 'center',
    justifyContent: 'center',
  },
  connected: {
    backgroundColor: '#28a745',
  },
  disconnected: {
    backgroundColor: '#dc3545',
  },
  statusText: {
    color: 'white',
    fontWeight: 'bold',
  },
  screenContainer: {
    flex: 1,
    padding: 16,
  },
  screenTitle: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 16,
  },
  messageContainer: {
    padding: 24,
    alignItems: 'center',
    justifyContent: 'center',
  },
  messageText: {
    fontSize: 18,
    fontWeight: 'bold',
    color: '#6c757d',
    marginBottom: 8,
  },
  messageSubtext: {
    fontSize: 14,
    color: '#6c757d',
  },
});
