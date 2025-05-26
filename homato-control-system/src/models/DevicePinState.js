const mongoose = require('mongoose');

const devicePinStateSchema = new mongoose.Schema({
    device_id: {
        type: String,
        required: true,
        trim: true
    },
    pin_id: {
        type: String,
        required: true,
        trim: true
    },
    state: {
        type: Boolean,
        required: true
    }
}, {
    timestamps: true
});

// Compound index for efficient querying
devicePinStateSchema.index({ device_id: 1, pin_id: 1 }, { unique: true });

const DevicePinState = mongoose.model('DevicePinState', devicePinStateSchema);
module.exports = DevicePinState; 