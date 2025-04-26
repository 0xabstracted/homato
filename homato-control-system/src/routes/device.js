const express = require('express');
const router = express.Router();
const auth = require('../middleware/auth');
const { addDevice, getUserDevices, getDeviceById, getDeviceByDeviceId } = require('../controllers/deviceController');

// All routes require authentication
router.use(auth);

// Add new device
router.post('/', addDevice);

// Get all devices for current user
router.get('/', getUserDevices);

// Get device by MongoDB ID
router.get('/id/:deviceId', getDeviceById);

// Get device by device ID
router.get('/deviceid/:deviceid', getDeviceByDeviceId);

module.exports = router; 