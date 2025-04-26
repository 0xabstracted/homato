const { User, Device, Switch, UserDevice } = require('../models');

const insertSampleData = async () => {
    try {
        // Sample Users
        const users = [
            {
                name: 'John Doe',
                email: 'john@example.com',
                password: 'Password123!',
                phone: '+1234567890'
            },
            {
                name: 'Jane Smith',
                email: 'jane@example.com',
                password: 'Password456!',
                phone: '+9876543210'
            }
        ];

        // Sample Devices
        const devices = [
            {
                deviceid: 'RAG-0000',
                name: 'Living Room Hub',
                description: 'Main control hub for living room'
            },
            {
                deviceid: 'RAG-0001',
                name: 'Bedroom Hub',
                description: 'Control hub for master bedroom'
            }
        ];

        // Insert Users
        console.log('Creating sample users...');
        const createdUsers = await Promise.all(
            users.map(async (user) => {
                try {
                    const newUser = new User(user);
                    await newUser.save();
                    return newUser;
                } catch (error) {
                    if (error.code === 11000) {
                        return await User.findOne({ email: user.email });
                    }
                    throw error;
                }
            })
        );

        // Insert Devices
        console.log('Creating sample devices...');
        const createdDevices = await Promise.all(
            devices.map(async (device) => {
                try {
                    const newDevice = new Device(device);
                    await newDevice.save();
                    return newDevice;
                } catch (error) {
                    if (error.code === 11000) {
                        return await Device.findOne({ name: device.name });
                    }
                    throw error;
                }
            })
        );

        // Create sample switches for each device
        console.log('Creating sample switches...');
        for (const device of createdDevices) {
            for (let i = 1; i <= 4; i++) {
                try {
                    await Switch.create({
                        device: device._id,
                        pinId: `GPIO${i}`,
                        name: `Switch ${i}`,
                        state: false
                    });
                } catch (error) {
                    if (error.code !== 11000) {
                        throw error;
                    }
                }
            }
        }

        // Associate users with devices
        console.log('Creating user-device associations...');
        for (const user of createdUsers) {
            for (const device of createdDevices) {
                try {
                    await UserDevice.create({
                        user: user._id,
                        device: device._id
                    });
                } catch (error) {
                    if (error.code !== 11000) {
                        throw error;
                    }
                }
            }
        }

        console.log('✓ Sample data inserted successfully');

        // Return created data for reference
        return {
            users: createdUsers,
            devices: createdDevices
        };
    } catch (error) {
        console.error('Error inserting sample data:', error);
        throw error;
    }
};

module.exports = insertSampleData; 