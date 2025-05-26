const mongoose = require('mongoose');

const deviceSchema = new mongoose.Schema({
    device_id: {
        type: String,
        required: true,
        trim: true
    },
    name: {
        type: String,
        required: true,
        trim: true
    },
    description: {
        type: String,
        trim: true,
        default: null
    },
    lot_number: {
        type: String,
        trim: true,
        default: null
    },
    barcode: {
        type: String,
        trim: true,
        default: null
    },
    image: {
        type: String,
        trim: true,
        default: null
    },
    version: {
        type: String,
        trim: true,
        default: 'v1.0.0'
    },
    firmware_version: { 
        type: String,
        trim: true,
        default: 'v1.0.0'
    },
    firmware_update: {
        type: String,
        trim: true,
        default: 'pending'
    },
    hardware_info: {
        type: JSON,
        default: null
    },
    type: {
        type: String,
        default: 'device'
    },
    status: {
        type: String,
        enum: ['active', 'inactive'],
        default: 'active'
    }
}, {
    timestamps: true
});

const Device = mongoose.model('device', deviceSchema);
module.exports = Device; 