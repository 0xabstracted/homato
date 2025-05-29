import 'dart:async';
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
    // Request Bluetooth permissions
    Map<Permission, PermissionStatus> statuses =
        await [
          Permission.bluetooth,
          Permission.bluetoothScan,
          Permission.bluetoothConnect,
          Permission.location,
        ].request();

    if (statuses.values.any((status) => status != PermissionStatus.granted)) {
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Permissions not granted'),
            backgroundColor: Colors.red,
          ),
        );
      }
    }
  }

  // Start scanning for BLE devices
  void startScan() {
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

    // Start scanning
    FlutterBluePlus.startScan(timeout: const Duration(seconds: 10));

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
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(content: Text('Read value: ${value.toString()}')),
        );
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
          ScaffoldMessenger.of(context).showSnackBar(
            SnackBar(content: Text('Notification: ${value.toString()}')),
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
      content: TextField(
        controller: _controller,
        decoration: const InputDecoration(
          labelText: 'Enter hex values (e.g., 01 02 FF)',
        ),
      ),
      actions: <Widget>[
        TextButton(
          child: const Text('CANCEL'),
          onPressed: () => Navigator.of(context).pop(),
        ),
        TextButton(
          child: const Text('WRITE'),
          onPressed: () {
            // Convert input hex string to list of integers
            List<int> value =
                _controller.text
                    .split(' ')
                    .map((hex) => int.parse(hex, radix: 16))
                    .toList();
            widget.onWrite(value);
            Navigator.of(context).pop();
          },
        ),
      ],
    );
  }
}
