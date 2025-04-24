import React, { useState, useEffect } from "react";
import { View, Text, StyleSheet, ScrollView, TouchableOpacity, TextInput, Alert, ActivityIndicator } from "react-native";
import { useAppContext } from "./context/AppContext";
import { DEFAULT_SERVER_URL } from "./config/serverConfig";

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

export default function SettingsScreen() {
  const { serverUrl, setServerUrl, isConnected, connect, disconnect } = useAppContext();
  const [inputUrl, setInputUrl] = useState(serverUrl || DEFAULT_SERVER_URL);
  const [isLoading, setIsLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  // Update input when serverUrl changes
  useEffect(() => {
    if (serverUrl) {
      setInputUrl(serverUrl);
    }
  }, [serverUrl]);

  // Handle server URL update
  const handleUpdateServerUrl = async () => {
    // Validate URL format
    if (!inputUrl.startsWith('http://') && !inputUrl.startsWith('https://')) {
      setError('URL must start with http:// or https://');
      return;
    }

    try {
      setIsLoading(true);
      setError(null);
      await setServerUrl(inputUrl);
      Alert.alert('Success', 'Server URL updated successfully');
    } catch (error) {
      console.error('Failed to update server URL:', error);
      setError('Failed to update server URL. Please check the URL and try again.');
    } finally {
      setIsLoading(false);
    }
  };

  // Reset to default URL
  const handleResetToDefault = () => {
    setInputUrl(DEFAULT_SERVER_URL);
  };

  // Handle connection toggle
  const handleConnectionToggle = async () => {
    if (isConnected) {
      disconnect();
    } else {
      try {
        setIsLoading(true);
        await connect();
      } catch (error) {
        console.error('Failed to connect:', error);
        Alert.alert('Connection Error', 'Failed to connect to the server. Please check your settings and try again.');
      } finally {
        setIsLoading(false);
      }
    }
  };

  return (
    <View style={styles.container}>
      <ConnectionStatusBar />
      <ScrollView style={styles.scrollContainer}>
      <Text style={styles.title}>Settings</Text>
      
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Server Connection</Text>
        
        <View style={styles.formGroup}>
          <Text style={styles.label}>Server URL</Text>
          <TextInput
            style={styles.input}
            value={inputUrl}
            onChangeText={setInputUrl}
            placeholder="http://server-address:port"
            autoCapitalize="none"
            autoCorrect={false}
            keyboardType="url"
          />
          {error && <Text style={styles.errorText}>{error}</Text>}
        </View>
        
        <View style={styles.buttonRow}>
          <TouchableOpacity 
            style={[styles.button, styles.secondaryButton]} 
            onPress={handleResetToDefault}
            disabled={isLoading}
          >
            <Text style={styles.secondaryButtonText}>Reset to Default</Text>
          </TouchableOpacity>
          
          <TouchableOpacity 
            style={[styles.button, styles.primaryButton]} 
            onPress={handleUpdateServerUrl}
            disabled={isLoading}
          >
            {isLoading ? (
              <ActivityIndicator color="white" size="small" />
            ) : (
              <Text style={styles.primaryButtonText}>Update URL</Text>
            )}
          </TouchableOpacity>
        </View>
      </View>
      
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>Connection Status</Text>
        
        <View style={styles.statusContainer}>
          <View style={[styles.statusIndicator, isConnected ? styles.connectionStatusConnected : styles.connectionStatusDisconnected]} />
          <Text style={styles.connectionStatusText}>
            {isConnected ? 'Connected to server' : 'Disconnected from server'}
          </Text>
        </View>
        
        <TouchableOpacity 
          style={[styles.button, isConnected ? styles.dangerButton : styles.successButton]} 
          onPress={handleConnectionToggle}
          disabled={isLoading}
        >
          {isLoading ? (
            <ActivityIndicator color="white" size="small" />
          ) : (
            <Text style={styles.buttonText}>
              {isConnected ? 'Disconnect' : 'Connect'}
            </Text>
          )}
        </TouchableOpacity>
      </View>
      
      <View style={styles.section}>
        <Text style={styles.sectionTitle}>About</Text>
        <Text style={styles.aboutText}>Homato - Home Automation Tool</Text>
        <Text style={styles.versionText}>Version 1.0.0</Text>
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
  scrollContainer: {
    flex: 1,
    padding: 16,
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
  title: {
    fontSize: 24,
    fontWeight: 'bold',
    marginBottom: 24,
  },
  section: {
    backgroundColor: 'white',
    borderRadius: 8,
    padding: 16,
    marginBottom: 16,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.2,
    shadowRadius: 1.41,
    elevation: 2,
  },
  sectionTitle: {
    fontSize: 18,
    fontWeight: 'bold',
    marginBottom: 16,
  },
  formGroup: {
    marginBottom: 16,
  },
  label: {
    fontSize: 14,
    fontWeight: 'bold',
    marginBottom: 8,
    color: '#495057',
  },
  input: {
    borderWidth: 1,
    borderColor: '#ced4da',
    borderRadius: 4,
    padding: 12,
    fontSize: 16,
    backgroundColor: '#fff',
  },
  errorText: {
    color: '#dc3545',
    marginTop: 8,
    fontSize: 14,
  },
  buttonRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
  },
  button: {
    borderRadius: 4,
    padding: 12,
    alignItems: 'center',
    justifyContent: 'center',
    marginVertical: 8,
  },
  primaryButton: {
    backgroundColor: '#007bff',
    flex: 1,
    marginLeft: 8,
  },
  secondaryButton: {
    backgroundColor: '#f8f9fa',
    borderWidth: 1,
    borderColor: '#ced4da',
    flex: 1,
    marginRight: 8,
  },
  successButton: {
    backgroundColor: '#28a745',
  },
  dangerButton: {
    backgroundColor: '#dc3545',
  },
  primaryButtonText: {
    color: 'white',
    fontWeight: 'bold',
  },
  secondaryButtonText: {
    color: '#495057',
  },
  buttonText: {
    color: 'white',
    fontWeight: 'bold',
  },
  statusContainer: {
    flexDirection: 'row',
    alignItems: 'center',
    marginBottom: 16,
  },
  statusIndicator: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginRight: 8,
  },
  connectionStatusConnected: {
    backgroundColor: '#28a745',
  },
  connectionStatusDisconnected: {
    backgroundColor: '#dc3545',
  },
  connectionStatusText: {
    fontSize: 16,
  },
  aboutText: {
    fontSize: 16,
    marginBottom: 8,
  },
  versionText: {
    fontSize: 14,
    color: '#6c757d',
  },
});
