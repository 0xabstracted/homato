import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:permission_handler/permission_handler.dart';

class BLEScreen extends StatefulWidget {
  const BLEScreen({super.key});

  @override
  State<BLEScreen> createState() => _BLEScreenState();
}

class _BLEScreenState extends State<BLEScreen> {
  // Found devices list
  List<ScanResult> scanResults = [];

  // Scanning state
  bool isScanning = false;

  // Selected device
  BluetoothDevice? selectedDevice;

  // Connected device states
  bool isConnecting = false;
  bool isDisconnecting = false;

  // Stream subscription for scanning
  StreamSubscription<List<ScanResult>>? scanSubscription;

  // Status tracking
  bool isBluetoothEnabled = false;
  bool isLocationEnabled = false;
  bool hasPermissions = false;

  @override
  void initState() {
    super.initState();
    // Init permissions
    _requestPermissions();
  }

  @override
  void dispose() {
    scanSubscription?.cancel();
    // Disconnect selected device if any
    selectedDevice?.disconnect();
    super.dispose();
  }

  // Request required permissions
  Future<void> _requestPermissions() async {
    // Check if Bluetooth is enabled
    BluetoothAdapterState bluetoothState =
        await FlutterBluePlus.adapterState.first;
    setState(() {
      isBluetoothEnabled = bluetoothState == BluetoothAdapterState.on;
    });

    if (bluetoothState != BluetoothAdapterState.on) {
      if (mounted) {
        String message;
        switch (bluetoothState) {
          case BluetoothAdapterState.off:
            message = 'Bluetooth is turned off. Please enable Bluetooth.';
            break;
          case BluetoothAdapterState.unavailable:
            message = 'Bluetooth is not available on this device.';
            break;
          case BluetoothAdapterState.unauthorized:
            message = 'Bluetooth access is not authorized.';
            break;
          case BluetoothAdapterState.turningOn:
            message = 'Bluetooth is turning on. Please wait...';
            break;
          case BluetoothAdapterState.turningOff:
            message = 'Bluetooth is turning off.';
            break;
          default:
            message = 'Bluetooth is not ready. Please enable Bluetooth.';
        }

        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(message), backgroundColor: Colors.red),
        );
      }
    }

    // Check and request location permission first
    PermissionStatus locationStatus = await Permission.location.status;
    if (!locationStatus.isGranted) {
      locationStatus = await Permission.location.request();
    }

    // Request Bluetooth permissions
    Map<Permission, PermissionStatus> statuses =
        await [
          Permission.bluetooth,
          Permission.bluetoothScan,
          Permission.bluetoothConnect,
          Permission.location,
        ].request();

    // Check if any critical permissions are missing
    List<String> deniedPermissions = [];

    if (statuses[Permission.location] != PermissionStatus.granted) {
      deniedPermissions.add('Location');
    }
    if (statuses[Permission.bluetoothScan] != PermissionStatus.granted) {
      deniedPermissions.add('Bluetooth Scan');
    }
    if (statuses[Permission.bluetoothConnect] != PermissionStatus.granted) {
      deniedPermissions.add('Bluetooth Connect');
    }

    setState(() {
      hasPermissions = deniedPermissions.isEmpty;
    });

    if (deniedPermissions.isNotEmpty && mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Missing permissions: ${deniedPermissions.join(', ')}'),
          backgroundColor: Colors.red,
          action: SnackBarAction(
            label: 'Settings',
            onPressed: () => openAppSettings(),
          ),
        ),
      );
    }

    // Also check if location services are enabled
    bool serviceEnabled = await Permission.location.serviceStatus.isEnabled;
    setState(() {
      isLocationEnabled = serviceEnabled;
    });

    if (!serviceEnabled && mounted) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text(
            'Location services must be enabled for Bluetooth scanning',
          ),
          backgroundColor: Colors.orange,
        ),
      );
    }
  }

  // Start scanning for BLE devices
  Future<void> startScan() async {
    // Check if Bluetooth is enabled first
    BluetoothAdapterState bluetoothState =
        await FlutterBluePlus.adapterState.first;
    if (bluetoothState != BluetoothAdapterState.on) {
      if (mounted) {
        String message;
        switch (bluetoothState) {
          case BluetoothAdapterState.off:
            message = 'Bluetooth is turned off. Please enable Bluetooth.';
            break;
          case BluetoothAdapterState.unavailable:
            message = 'Bluetooth is not available on this device.';
            break;
          case BluetoothAdapterState.unauthorized:
            message = 'Bluetooth access is not authorized.';
            break;
          case BluetoothAdapterState.turningOn:
            message = 'Bluetooth is turning on. Please wait...';
            break;
          case BluetoothAdapterState.turningOff:
            message = 'Bluetooth is turning off.';
            break;
          default:
            message = 'Bluetooth is not ready. Please enable Bluetooth.';
        }

        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text(message), backgroundColor: Colors.red),
        );
      }
      return;
    }

    // Check location permission first
    PermissionStatus locationStatus = await Permission.location.status;

    if (!locationStatus.isGranted) {
      locationStatus = await Permission.location.request();
      if (!locationStatus.isGranted) {
        if (mounted) {
          ScaffoldMessenger.of(context).showSnackBar(
            const SnackBar(
              content: Text(
                'Location permission is required for Bluetooth scanning',
              ),
              backgroundColor: Colors.red,
            ),
          );
        }
        return;
      }
    }

    // Check if location services are enabled
    bool serviceEnabled = await Permission.location.serviceStatus.isEnabled;
    if (!serviceEnabled) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Please enable location services in device settings'),
            backgroundColor: Colors.orange,
          ),
        );
      }
      return;
    }

    // For Android 12+, also check Bluetooth scan permission
    if (await Permission.bluetoothScan.isDenied) {
      PermissionStatus bluetoothScanStatus =
          await Permission.bluetoothScan.request();
      if (!bluetoothScanStatus.isGranted) {
        if (mounted) {
          ScaffoldMessenger.of(context).showSnackBar(
            const SnackBar(
              content: Text('Bluetooth scan permission is required'),
              backgroundColor: Colors.red,
            ),
          );
        }
        return;
      }
    }

    setState(() {
      scanResults = [];
      isScanning = true;
    });

    // Listen to scan results
    scanSubscription = FlutterBluePlus.scanResults.listen(
      (results) {
        setState(() {
          scanResults = results;
        });
      },
      onDone: () {
        setState(() {
          isScanning = false;
        });
      },
    );

    // Start scanning with error handling
    try {
      await FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to start scan: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
      setState(() {
        isScanning = false;
      });
      return;
    }

    // After 10 seconds, stop scanning
    Future.delayed(const Duration(seconds: 10), () {
      stopScan();
    });
  }

  // Stop scanning for BLE devices
  void stopScan() {
    FlutterBluePlus.stopScan();
    setState(() {
      isScanning = false;
    });
  }

  // Connect to a device
  Future<void> connectToDevice(BluetoothDevice device) async {
    setState(() {
      isConnecting = true;
      selectedDevice = device;
    });

    try {
      await device.connect();
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Connected to ${device.platformName}'),
            backgroundColor: Colors.green,
          ),
        );
        // Navigate to device screen
        Navigator.of(context).push(
          MaterialPageRoute(builder: (context) => DeviceScreen(device: device)),
        );
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to connect: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    } finally {
      setState(() {
        isConnecting = false;
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: const Text('BLE Scanner')),
      body: Column(
        children: <Widget>[
          // Status panel
          Container(
            padding: const EdgeInsets.all(12),
            color: Colors.grey[100],
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                _buildStatusIndicator(
                  'Bluetooth',
                  isBluetoothEnabled,
                  Icons.bluetooth,
                ),
                _buildStatusIndicator(
                  'Location',
                  isLocationEnabled,
                  Icons.location_on,
                ),
                _buildStatusIndicator(
                  'Permissions',
                  hasPermissions,
                  Icons.security,
                ),
              ],
            ),
          ),

          // Scanning status
          Container(
            padding: const EdgeInsets.all(16),
            color: isScanning ? Colors.blue : Colors.grey,
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: <Widget>[
                Text(
                  isScanning ? 'Scanning...' : 'Not scanning',
                  style: const TextStyle(
                    color: Colors.white,
                    fontWeight: FontWeight.bold,
                  ),
                ),
                Icon(
                  isScanning ? Icons.bluetooth_searching : Icons.bluetooth,
                  color: Colors.white,
                ),
              ],
            ),
          ),

          // Device list
          Expanded(
            child: ListView.builder(
              itemCount: scanResults.length,
              itemBuilder: (context, index) {
                ScanResult result = scanResults[index];
                return ListTile(
                  title: Text(
                    result.device.platformName.isEmpty
                        ? 'Unknown Device'
                        : result.device.platformName,
                  ),
                  subtitle: Text(result.device.remoteId.str),
                  trailing: Text('${result.rssi} dBm'),
                  onTap: () => connectToDevice(result.device),
                );
              },
            ),
          ),
        ],
      ),
      floatingActionButton: FloatingActionButton(
        onPressed: isScanning ? stopScan : startScan,
        child: Icon(isScanning ? Icons.stop : Icons.search),
      ),
    );
  }

  Widget _buildStatusIndicator(String label, bool isEnabled, IconData icon) {
    return GestureDetector(
      onTap: _refreshStatus,
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(icon, color: isEnabled ? Colors.green : Colors.red, size: 20),
          const SizedBox(height: 4),
          Text(
            label,
            style: TextStyle(
              fontSize: 12,
              color: isEnabled ? Colors.green : Colors.red,
              fontWeight: FontWeight.w500,
            ),
          ),
          Container(
            width: 8,
            height: 8,
            margin: const EdgeInsets.only(top: 2),
            decoration: BoxDecoration(
              shape: BoxShape.circle,
              color: isEnabled ? Colors.green : Colors.red,
            ),
          ),
        ],
      ),
    );
  }

  // Refresh status method
  Future<void> _refreshStatus() async {
    await _requestPermissions();
  }
}

class DeviceScreen extends StatefulWidget {
  final BluetoothDevice device;

  const DeviceScreen({Key? key, required this.device}) : super(key: key);

  @override
  State<DeviceScreen> createState() => _DeviceScreenState();
}

class _DeviceScreenState extends State<DeviceScreen> {
  // List of discovered services
  List<BluetoothService> services = [];

  // Loading state
  bool isLoading = true;

  @override
  void initState() {
    super.initState();
    // Discover services
    discoverServices();
  }

  // Discover device services
  Future<void> discoverServices() async {
    setState(() {
      isLoading = true;
    });

    try {
      services = await widget.device.discoverServices();
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to discover services: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    } finally {
      setState(() {
        isLoading = false;
      });
    }
  }

  // Read characteristic value
  Future<void> readCharacteristic(
    BluetoothCharacteristic characteristic,
  ) async {
    try {
      List<int> value = await characteristic.read();
      if (mounted) {
        _showReadDialog(value);
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to read: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  // Show read data dialog
  void _showReadDialog(List<int> value) {
    String stringValue = '';

    try {
      stringValue = utf8.decode(value);
    } catch (e) {
      stringValue = 'Unable to decode as UTF-8 string';
    }

    showDialog(
      context: context,
      builder: (BuildContext dialogContext) {
        return AlertDialog(
          title: const Text('Read Value'),
          content: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              const Text(
                'Value:',
                style: TextStyle(fontWeight: FontWeight.bold),
              ),
              Container(
                padding: const EdgeInsets.all(8),
                decoration: BoxDecoration(
                  color: Colors.grey[100],
                  borderRadius: BorderRadius.circular(4),
                ),
                child: SelectableText(stringValue),
              ),
            ],
          ),
          actions: [
            TextButton(
              onPressed: () => Navigator.of(dialogContext).pop(),
              child: const Text('OK'),
            ),
          ],
        );
      },
    );
  }

  // Write to characteristic
  Future<void> writeCharacteristic(
    BluetoothCharacteristic characteristic,
    List<int> value,
  ) async {
    try {
      await characteristic.write(value);
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Write successful'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to write: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  // Subscribe to characteristic notifications
  Future<void> subscribeToCharacteristic(
    BluetoothCharacteristic characteristic,
  ) async {
    try {
      await characteristic.setNotifyValue(true);
      characteristic.lastValueStream.listen((value) {
        if (mounted) {
          String displayValue;
          try {
            displayValue = utf8.decode(value);
          } catch (e) {
            displayValue = 'Unable to decode as UTF-8 string';
          }

          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(
              content: Text('Notification: $displayValue'),
              duration: const Duration(seconds: 3),
            ),
          );
        }
      });
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Subscribed to notifications'),
            backgroundColor: Colors.green,
          ),
        );
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to subscribe: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  // Disconnect from device
  Future<void> disconnect() async {
    try {
      await widget.device.disconnect();
      if (mounted) {
        Navigator.of(context).pop();
      }
    } catch (e) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to disconnect: $e'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text(widget.device.platformName),
        actions: <Widget>[
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: discoverServices,
          ),
        ],
      ),
      body:
          isLoading
              ? const Center(child: CircularProgressIndicator())
              : ListView.builder(
                itemCount: services.length,
                itemBuilder: (context, index) {
                  BluetoothService service = services[index];
                  return ExpansionTile(
                    title: Text('Service: ${service.uuid.toString()}'),
                    children:
                        service.characteristics.map((characteristic) {
                          return ListTile(
                            title: Text(
                              'Characteristic: ${characteristic.uuid.toString()}',
                            ),
                            subtitle: Column(
                              crossAxisAlignment: CrossAxisAlignment.start,
                              children: <Widget>[
                                const Text('Properties:'),
                                Row(
                                  children: <Widget>[
                                    // Properties buttons
                                    if (characteristic.properties.read)
                                      TextButton(
                                        child: const Text('READ'),
                                        onPressed:
                                            () => readCharacteristic(
                                              characteristic,
                                            ),
                                      ),
                                    if (characteristic.properties.write)
                                      TextButton(
                                        child: const Text('WRITE'),
                                        onPressed:
                                            () => showDialog(
                                              context: context,
                                              builder:
                                                  (writeContext) => WriteDialog(
                                                    onWrite:
                                                        (value) =>
                                                            writeCharacteristic(
                                                              characteristic,
                                                              value,
                                                            ),
                                                  ),
                                            ),
                                      ),
                                    if (characteristic.properties.notify)
                                      TextButton(
                                        child: const Text('NOTIFY'),
                                        onPressed:
                                            () => subscribeToCharacteristic(
                                              characteristic,
                                            ),
                                      ),
                                  ],
                                ),
                              ],
                            ),
                          );
                        }).toList(),
                  );
                },
              ),
      floatingActionButton: FloatingActionButton(
        onPressed: disconnect,
        backgroundColor: Colors.red,
        child: const Icon(Icons.bluetooth_disabled),
      ),
    );
  }
}

// Dialog for writing to characteristics
class WriteDialog extends StatefulWidget {
  final Function(List<int>) onWrite;

  const WriteDialog({Key? key, required this.onWrite}) : super(key: key);

  @override
  State<WriteDialog> createState() => _WriteDialogState();
}

class _WriteDialogState extends State<WriteDialog> {
  final TextEditingController _controller = TextEditingController();

  @override
  Widget build(BuildContext context) {
    return AlertDialog(
      title: const Text('Write Value'),
      content: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          TextField(
            controller: _controller,
            decoration: const InputDecoration(
              labelText: 'Enter text to send',
              hintText: 'Hello World',
            ),
            maxLines: 3,
          ),
        ],
      ),
      actions: <Widget>[
        TextButton(
          child: const Text('CANCEL'),
          onPressed: () => Navigator.of(context).pop(),
        ),
        TextButton(
          child: const Text('WRITE'),
          onPressed: () {
            try {
              // Convert string to UTF-8 bytes
              List<int> value = utf8.encode(_controller.text);
              widget.onWrite(value);
              Navigator.of(context).pop();
            } catch (e) {
              // Show error message
              ScaffoldMessenger.of(context).showSnackBar(
                SnackBar(
                  content: Text('Invalid input: $e'),
                  backgroundColor: Colors.red,
                ),
              );
            }
          },
        ),
      ],
    );
  }
}
