const express = require('express');
const router = express.Router();
const authRoutes = require('./auth');
const deviceRoutes = require('./device');
const factoryRoutes = require('./factory');

router.use('/', authRoutes);
router.use('/device', deviceRoutes);
router.use('/factory/device', factoryRoutes);


module.exports = router;