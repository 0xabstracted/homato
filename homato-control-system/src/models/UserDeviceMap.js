const mongoose = require('mongoose');

const userDeviceMapSchema = new mongoose.Schema({
    user_given_name: {
        type: String,
        required: true,
        trim: true
    },
    device_id: {
        type: String,
        required: true,
        unique: true,
        trim: true
    },
    user_id: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'User',
        required: true
    }
}, {
    timestamps: true
});

const UserDeviceMap = mongoose.model('UserDeviceMap', userDeviceMapSchema);
module.exports = UserDeviceMap; 