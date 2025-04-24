import React, { useState, useEffect } from "react";
import { Text, View, StyleSheet, ScrollView, RefreshControl, TouchableOpacity, ActivityIndicator } from "react-native";
import { Picker } from '@react-native-picker/picker';
import { useAppContext } from "./context/AppContext";

// Connection status bar component
function ConnectionStatusBar() {
  const { isConnected } = useAppContext();
  
  return (
    <View style={[styles.statusBar, isConnected ? styles.connected : styles.disconnected]}>
      <Text style={styles.statusText}>
        Server: {isConnected ? 'Connected' : 'Disconnected'}
      </Text>
    </View>
  );
}

// Homato Control System screen
export default
function DevicesScreen() {
  const { devices, refreshDevices, controlDevice, isConnected } = useAppContext();
  const [refreshing, setRefreshing] = useState(false);
  const [selectedDevice, setSelectedDevice] = useState<string>('');
  const [loading, setLoading] = useState(true);
  
  // Set the first device as selected when devices are loaded
  useEffect(() => {
    if (Object.keys(devices).length > 0 && !selectedDevice) {
      setSelectedDevice(Object.keys(devices)[0]);
    }
    setLoading(false);
  }, [devices]);
  
  // Handle pull-to-refresh
  const onRefresh = () => {
    setRefreshing(true);
    refreshDevices();
    setTimeout(() => setRefreshing(false), 1000); // Ensure refresh indicator shows for at least 1 second
  };
  
  // Get the selected device data
  const selectedDeviceData = selectedDevice ? devices[selectedDevice] : null;
  
  // Format the current date and time
  const currentDateTime = new Date().toLocaleString('en-US', {
    day: '2-digit',
    month: '2-digit',
    year: 'numeric',
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit',
    hour12: true
  });
  
  return (
    <ScrollView 
      style={styles.screenContainer}
      refreshControl={<RefreshControl refreshing={refreshing} onRefresh={onRefresh} />}
    >
      <View style={styles.headerContainer}>
        <Text style={styles.screenTitle}>Homato Control System</Text>
        <ConnectionStatusBar />
      </View>
      
      {loading ? (
        <View style={styles.loadingContainer}>
          <ActivityIndicator size="large" color="#0066cc" />
          <Text style={styles.loadingText}>Loading devices...</Text>
        </View>
      ) : !isConnected ? (
        <View style={styles.messageContainer}>
          <Text style={styles.messageText}>Not connected to server</Text>
          <Text style={styles.messageSubtext}>Pull down to try reconnecting</Text>
        </View>
      ) : Object.keys(devices).length === 0 ? (
        <View style={styles.messageContainer}>
          <Text style={styles.messageText}>No devices found</Text>
          <Text style={styles.messageSubtext}>Pull down to refresh</Text>
        </View>
      ) : (
        <View style={styles.container}>
          <View style={styles.selectDeviceContainer}>
            <Text style={styles.selectDeviceLabel}>Select Device</Text>
            <View style={styles.pickerOuterContainer}>
              <View style={styles.pickerContainer}>
                <Picker
                  selectedValue={selectedDevice}
                  style={styles.devicePicker}
                  onValueChange={(itemValue) => setSelectedDevice(itemValue.toString())}
                  dropdownIconColor="#333"
                >
                  {Object.keys(devices).map((deviceId) => (
                    <Picker.Item key={deviceId} label={deviceId} value={deviceId} />
                  ))}
                </Picker>
              </View>
            </View>
          </View>
          
          {selectedDeviceData && (
            <View style={styles.deviceInfoContainer}>
              <View style={styles.deviceInfoTable}>
                <View style={styles.deviceInfoRow}>
                  <Text style={styles.deviceInfoLabel}>Device ID:</Text>
                  <Text style={styles.deviceInfoValue}>
                    <Text style={styles.deviceIdHighlight}>{selectedDevice}</Text>
                  </Text>
                </View>
                
                <View style={styles.deviceInfoRow}>
                  <Text style={styles.deviceInfoLabel}>Status:</Text>
                  <View style={styles.statusContainer}>
                    <Text style={[styles.deviceInfoValue, styles.statusValueText, 
                      selectedDeviceData.status === 'online' ? styles.statusTextOnline : 
                      selectedDeviceData.status === 'offline' ? styles.statusTextOffline : 
                      styles.statusTextUnreachable]}>
                      {selectedDeviceData.status.charAt(0).toUpperCase() + selectedDeviceData.status.slice(1)}
                    </Text>
                  </View>
                </View>
                
                <View style={styles.deviceInfoRow}>
                  <Text style={styles.deviceInfoLabel}>Last Seen:</Text>
                  <Text style={styles.deviceInfoValue}>
                    {new Date(selectedDeviceData.lastSeen).toLocaleString('en-US', {
                      day: '2-digit',
                      month: '2-digit',
                      year: 'numeric',
                      hour: '2-digit',
                      minute: '2-digit',
                      second: '2-digit',
                      hour12: true
                    })}
                  </Text>
                </View>
              </View>
            </View>
          )}
          
          {selectedDeviceData && (
            <View style={styles.relayContainer}>
              <View style={styles.relayGrid}>
                {Object.entries(selectedDeviceData.relays).map(([relay, state]) => (
                  <View key={relay} style={styles.relayCardContainer}>
                    <TouchableOpacity 
                      style={[
                        styles.relayCard, 
                        !selectedDeviceData.online && styles.relayDisabled,
                        state === 'ON' ? styles.relayCardOn : styles.relayCardOff
                      ]}
                      onPress={() => {
                        if (selectedDeviceData.online) {
                          controlDevice(selectedDevice, relay, state === 'ON' ? 'OFF' : 'ON');
                        }
                      }}
                      disabled={!selectedDeviceData.online}
                    >
                      <View style={styles.relayIconContainer}>
                        <View style={[styles.powerIcon, state === 'ON' ? styles.powerIconOn : styles.powerIconOff]} />
                      </View>
                      <View style={styles.relayTextContainer}>
                        <Text style={styles.relayName}>Relay {relay}</Text>
                        <Text style={[styles.relayStateText, state === 'ON' ? styles.stateTextOn : styles.stateTextOff]}>
                          {state === 'ON' ? 'ON' : 'OFF'}
                        </Text>
                        <Text style={styles.relayDescription}>
                          Controls relay{relay}
                        </Text>
                      </View>
                      <View style={styles.relayStatusContainer}>
                        <View style={[styles.relayStatus, state === 'ON' ? styles.relayOn : styles.relayOff]} />
                      </View>
                    </TouchableOpacity>
                  </View>
                ))}
              </View>
            </View>
          )}
          
          <View style={styles.activityLogContainer}>
            <Text style={styles.activityLogTitle}>Activity Log</Text>
            <View style={styles.logEntryContainer}>
              <Text style={styles.logTime}>{currentDateTime}</Text>
              <Text style={styles.logMessage}>Connected to server</Text>
            </View>
            {selectedDeviceData && Object.entries(selectedDeviceData.relays).map(([relay, state], index) => (
              <View key={`log-${index}`} style={styles.logEntryContainer}>
                <Text style={styles.logTime}>{currentDateTime}</Text>
                <Text style={styles.logMessage}>Received state for {selectedDevice} devices</Text>
              </View>
            ))}
          </View>
        </View>
      )}
    </ScrollView>
  );
}

// Styles
const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: '#f5f8fa',
    padding: 16,
  },
  headerContainer: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    marginBottom: 20,
    paddingHorizontal: 16,
    paddingTop: 8,
  },
  statusBar: {
    padding: 8,
    borderRadius: 4,
    minWidth: 120,
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
    fontSize: 12,
  },
  screenContainer: {
    flex: 1,
    backgroundColor: '#f5f8fa',
  },
  screenTitle: {
    fontSize: 24,
    fontWeight: 'bold',
    color: '#0066cc',
  },
  loadingContainer: {
    padding: 24,
    alignItems: 'center',
    justifyContent: 'center',
  },
  loadingText: {
    marginTop: 12,
    fontSize: 16,
    color: '#6c757d',
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
  selectDeviceContainer: {
    marginBottom: 24,
  },
  selectDeviceLabel: {
    fontSize: 20,
    fontWeight: 'bold',
    marginBottom: 12,
    color: '#0099ff',
    paddingLeft: 4,
  },
  pickerOuterContainer: {
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
    borderRadius: 8,
    marginHorizontal: 4,
  },
  pickerContainer: {
    borderWidth: 1,
    borderColor: '#e0e0e0',
    borderRadius: 8,
    backgroundColor: 'white',
    overflow: 'hidden',
  },
  devicePicker: {
    height: 50,
    width: '100%',
  },
  deviceInfoContainer: {
    backgroundColor: 'white',
    borderRadius: 8,
    padding: 0,
    marginBottom: 24,
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.1,
    shadowRadius: 4,
    elevation: 3,
    overflow: 'hidden',
    marginHorizontal: 4,
  },
  deviceInfoTable: {
    width: '100%',
  },
  deviceInfoRow: {
    flexDirection: 'row',
    justifyContent: 'space-between',
    alignItems: 'center',
    paddingVertical: 12,
    paddingHorizontal: 16,
    borderBottomWidth: 1,
    borderBottomColor: '#f0f0f0',
  },
  deviceInfoLabel: {
    fontSize: 14,
    fontWeight: '600',
    color: '#666',
  },
  deviceInfoValue: {
    fontSize: 14,
    color: '#333',
    fontWeight: '500',
  },
  deviceIdHighlight: {
    color: '#0099ff',
    fontWeight: 'bold',
  },
  statusContainer: {
    flexDirection: 'row',
    alignItems: 'center',
  },
  statusValueText: {
    fontWeight: 'bold',
  },
  statusTextOnline: {
    color: '#28a745',
  },
  statusTextOffline: {
    color: '#dc3545',
  },
  statusTextUnreachable: {
    color: '#ffc107',
  },
  deviceStatus: {
    width: 12,
    height: 12,
    borderRadius: 6,
    marginLeft: 8,
  },
  deviceOnline: {
    backgroundColor: '#28a745',
  },
  deviceOffline: {
    backgroundColor: '#dc3545',
  },
  relayContainer: {
    marginBottom: 16,
  },
  relayGrid: {
    flexDirection: 'row',
    flexWrap: 'wrap',
    marginHorizontal: -8,
  },
  relayCardContainer: {
    width: '50%',
    padding: 8,
  },
  relayCard: {
    backgroundColor: 'white',
    borderRadius: 8,
    padding: 16,
    flexDirection: 'row',
    alignItems: 'center',
    shadowColor: '#000',
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.2,
    shadowRadius: 1.41,
    elevation: 2,
    minHeight: 90,
  },
  relayCardOn: {
    borderLeftWidth: 4,
    borderLeftColor: '#28a745',
  },
  relayCardOff: {
    borderLeftWidth: 4,
    borderLeftColor: '#dc3545',
  },
  relayDisabled: {
    opacity: 0.5,
  },
  relayIconContainer: {
    width: 40,
    height: 40,
    borderRadius: 20,
    backgroundColor: '#f0f0f0',
    alignItems: 'center',
    justifyContent: 'center',
    marginRight: 12,
  },
  powerIcon: {
    width: 20,
    height: 20,
    borderRadius: 10,
    borderWidth: 2,
  },
  powerIconOn: {
    borderColor: '#28a745',
    backgroundColor: '#28a745',
  },
  powerIconOff: {
    borderColor: '#dc3545',
    backgroundColor: 'transparent',
  },
  relayTextContainer: {
    flex: 1,
  },
  relayName: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 4,
    color: '#333',
  },
  relayStateText: {
    fontSize: 14,
    fontWeight: 'bold',
    marginBottom: 4,
  },
  stateTextOn: {
    color: '#28a745',
  },
  stateTextOff: {
    color: '#dc3545',
  },
  relayDescription: {
    fontSize: 12,
    color: '#6c757d',
  },
  relayStatusContainer: {
    marginLeft: 8,
  },
  relayStatus: {
    width: 16,
    height: 16,
    borderRadius: 8,
  },
  relayOn: {
    backgroundColor: '#28a745',
  },
  relayOff: {
    backgroundColor: '#dc3545',
  },
  activityLogContainer: {
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
  activityLogTitle: {
    fontSize: 16,
    fontWeight: 'bold',
    marginBottom: 12,
    color: '#333',
  },
  logEntryContainer: {
    borderLeftWidth: 2,
    borderLeftColor: '#0066cc',
    paddingLeft: 12,
    marginBottom: 8,
    paddingVertical: 4,
  },
  logTime: {
    fontSize: 12,
    color: '#6c757d',
    marginBottom: 2,
  },
  logMessage: {
    fontSize: 14,
    color: '#333',
  },
});
