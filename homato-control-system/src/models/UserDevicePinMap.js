const mongoose = require('mongoose');

const userDevicePinMapSchema = new mongoose.Schema({
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
    pin_name: {
        type: String,
        required: true,
        trim: true
    },
    is_active: {
        type: Boolean,
        default: true
    }
}, {
    timestamps: true
});

// Compound index to ensure unique pin per device
userDevicePinMapSchema.index({ device_id: 1, pin_id: 1 }, { unique: true });

const UserDevicePinMap = mongoose.model('UserDevicePinMap', userDevicePinMapSchema);
module.exports = UserDevicePinMap; 