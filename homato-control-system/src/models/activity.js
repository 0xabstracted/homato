const mongoose = require('mongoose');

const activitySchema = new mongoose.Schema({
    user: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'User',
        required: true
    },
    device: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'Device',
        required: true
    },
    switch: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'Switch',
        required: true
    },
    action: {
        type: String,
        required: true,
        enum: ['ON', 'OFF']
    }
}, {
    timestamps: true
});

const Activity = mongoose.model('activity', activitySchema);
module.exports = Activity; 