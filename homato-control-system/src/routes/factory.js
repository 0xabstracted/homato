const express = require('express');
const router = express.Router();
const auth = require('../middleware/auth');
const { addDevice, isFactory } = require('../controllers/factoryDeviceController');

router.use(auth);

router.post('/', auth, isFactory, addDevice);

module.exports = router; 