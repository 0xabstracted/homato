const mongoose = require('mongoose');

const activitySchema = new mongoose.Schema({
    device_id: {
        type: String,
        required: true,
        trim: true
    },
    text: {
        type: String,
        required: true
    },
    updated_by: {
        type: String,
        required: true,
        enum: ['user', 'system', 'hardware']
    }
}, {
    timestamps: true
});

const Activity = mongoose.model('Activity', activitySchema);
module.exports = Activity; 