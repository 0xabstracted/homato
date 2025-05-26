const mongoose = require('mongoose');

const deviceStateSchema = new mongoose.Schema({
    device_id: {
        type: String,
        required: true,
        trim: true
    },
    state: {
        type: String,
        required: true,
        default: 'active'
    },
    updated_by: {
        type: String,
        required: true,
        enum: ['user', 'system', 'hardware'],
        default: 'system'
    }
}, {
    timestamps: true
});

const DeviceState = mongoose.model('DeviceState', deviceStateSchema);
module.exports = DeviceState; 