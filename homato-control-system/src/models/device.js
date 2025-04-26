const mongoose = require('mongoose');

const deviceSchema = new mongoose.Schema({
    deviceid: {
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
        trim: true
    }
}, {
    timestamps: true
});

const Device = mongoose.model('device', deviceSchema);
module.exports = Device; 