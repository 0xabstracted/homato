const { Device, UserDevice } = require('../models');

// Add new device
const addDevice = async (req, res) => {
    try {
        const { deviceid, name, description } = req.body;

        // Validate required fields
        if (!deviceid || !name) {
            return res.status(400).json({ error: 'Device ID and name are required' });
        }

        // Check if device already exists
        const existingDevice = await Device.findOne({ deviceid });
        if (existingDevice) {
            return res.status(400).json({ error: 'Device with this ID already exists' });
        }

        // Create device
        const device = new Device({
            deviceid,
            name,
            description
        });
        await device.save();

        // Associate device with the current user
        await UserDevice.create({
            user: req.user._id,
            device: device._id
        });

        res.status(201).json(device);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
};

// Get all devices for current user
const getUserDevices = async (req, res) => {
    try {
        const userDevices = await UserDevice.find({ user: req.user._id })
            .populate({
                path: 'device',
                select: 'deviceid name description createdAt updatedAt'
            });

        const devices = userDevices.map(ud => ud.device);
        res.json(devices);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
};

// Get device by ID
const getDeviceById = async (req, res) => {
    try {
        const { deviceId } = req.params;

        // Check if user has access to this device
        const userDevice = await UserDevice.findOne({
            user: req.user._id,
            device: deviceId
        }).populate('device');

        if (!userDevice) {
            return res.status(404).json({ error: 'Device not found' });
        }

        res.json(userDevice.device);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
};

// Get device by deviceid
const getDeviceByDeviceId = async (req, res) => {
    try {
        const { deviceid } = req.params;

        const device = await Device.findOne({ deviceid });
        if (!device) {
            return res.status(404).json({ error: 'Device not found' });
        }

        // Check if user has access to this device
        const userDevice = await UserDevice.findOne({
            user: req.user._id,
            device: device._id
        });

        if (!userDevice) {
            return res.status(403).json({ error: 'Access denied' });
        }

        res.json(device);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
};

module.exports = {
    addDevice,
    getUserDevices,
    getDeviceById,
    getDeviceByDeviceId
}; 