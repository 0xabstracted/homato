const { Device } = require('../models');


// check if user is organization
const isFactory = async (req, res, next) => {
    try {
        if (req.user.role !== 'organization') {
            return res.status(403).json({ error: 'You are not authorized to access this resource' });
        }
        next();
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
}

// add device
const addDevice = async (req, res) => {
    try {
        const { device_id, name, description, lot_number, barcode, image, version, firmware_version, firmware_update, hardware_info, type, status } = req.body;

        if (!device_id || !name) {
            return res.status(400).json({ error: 'Device ID and name are required' });
        }

        const existingDevice = await Device.findOne({ device_id });
        if (existingDevice) {
            return res.status(400).json({ error: 'Device with this ID already exists' });
        }
        // check barcode must be unique
        const checkBarcode = await Device.findOne({ barcode });
        if (checkBarcode) {
            return res.status(400).json({ error: 'Barcode already exists for another device. Please use a different barcode.' });
        }

        const device = {
            device_id,
            name,
            description,
            lot_number,
            barcode,
            image: image || null,
            version: version || null,
            firmware_version: firmware_version || null,
            firmware_update: firmware_update || null,
            hardware_info: hardware_info || null,
            type: type || null,
            status: true
        }

        const newDevice = await Device.create(device);
        res.status(201).json(newDevice);
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
}

module.exports = {
    addDevice,
    isFactory
}