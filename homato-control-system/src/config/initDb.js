const mongoose = require('mongoose');
const { User, Device, Switch, Activity, UserDevice } = require('../models');
const insertSampleData = require('./sampleData');

const initializeDatabase = async () => {
    try {
        // Get all collections
        const collections = await mongoose.connection.db.collections();
        console.log('Existing collections:', collections.map(c => c.collectionName));

        // Ensure indexes for User collection
        await User.collection.createIndex({ email: 1 }, { unique: true });
        console.log('✓ User indexes created');

        // Ensure indexes for Device collection
        await Device.collection.createIndex({ deviceid: 1, name: 1 });
        console.log('✓ Device indexes created');

        // Ensure indexes for Switch collection
        await Switch.collection.createIndex({ device: 1, pinId: 1 }, { unique: true });
        console.log('✓ Switch indexes created');

        // Ensure indexes for Activity collection
        await Activity.collection.createIndex({ user: 1 });
        await Activity.collection.createIndex({ device: 1 });
        await Activity.collection.createIndex({ createdAt: 1 });
        console.log('✓ Activity indexes created');

        // Ensure indexes for UserDevice collection
        await UserDevice.collection.createIndex({ user: 1, device: 1 }, { unique: true });
        console.log('✓ UserDevice indexes created');

        // Insert sample data if collections are empty
        const userCount = await User.countDocuments();
        const deviceCount = await Device.countDocuments();

        if (userCount === 0 && deviceCount === 0) {
            console.log('No existing data found, inserting sample data...');
            await insertSampleData();
        } else {
            console.log('Existing data found, skipping sample data insertion');
        }

        console.log('✓ Database initialization completed');
    } catch (error) {
        console.error('Database initialization failed:', error);
        throw error;
    }
};

module.exports = initializeDatabase; 