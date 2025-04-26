const mongoose = require('mongoose');

const switchSchema = new mongoose.Schema({
    device: {
        type: mongoose.Schema.Types.ObjectId,
        ref: 'Device',
        required: true
    },
    pinId: {
        type: String,
        required: true
    },
    name: {
        type: String,
        required: true,
        trim: true
    },
    state: {
        type: Boolean,
        default: false
    }
}, {
    timestamps: true
});

// Compound index to ensure unique pin per device
switchSchema.index({ device: 1, pinId: 1 }, { unique: true });

const Switch = mongoose.model('Switch', switchSchema);
module.exports = Switch; 