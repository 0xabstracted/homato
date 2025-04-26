const mongoose = require('mongoose');

const userDeviceSchema = new mongoose.Schema({
    user: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'User',
        required: true
    },
    device: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'Device',
        required: true
    }
}, {
    timestamps: true
});

// Ensure unique user-device pairs
userDeviceSchema.index({ user: 1, device: 1 }, { unique: true });

const UserDevice = mongoose.model('user_device', userDeviceSchema);
module.exports = UserDevice; 