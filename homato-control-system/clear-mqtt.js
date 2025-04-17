const mqtt = require('mqtt');

// MQTT Broker settings
const MQTT_HOST = process.env.MQTT_HOST;
const MQTT_PORT = process.env.MQTT_PORT;
const MQTT_USERNAME = process.env.MQTT_USERNAME;
const MQTT_PASSWORD = process.env.MQTT_PASSWORD;

// Topics to clear
const topics = [
    'home/switch',
    'home/light',
    'home/fan',
    'home/tubelight',
    'home/bedlight',
    'home/falseceiling',
    'home/ac',
    'home/switchport',
    'home/status',
    'home/availability'
];

// Connect to MQTT broker
const client = mqtt.connect(`mqtts://${MQTT_HOST}:${MQTT_PORT}`, {
    username: MQTT_USERNAME,
    password: MQTT_PASSWORD,
    rejectUnauthorized: false
});

client.on('connect', () => {
    console.log('Connected to MQTT broker');

    // Clear each topic by publishing an empty retained message
    topics.forEach(topic => {
        client.publish(topic, '', { retain: true }, (err) => {
            if (err) {
                console.error(`Error clearing ${topic}:`, err);
            } else {
                console.log(`Cleared ${topic}`);
            }
        });
    });

    // Wait a bit to ensure all messages are published
    setTimeout(() => {
        client.end();
        console.log('All topics cleared');
    }, 500);
});

client.on('error', (err) => {
    console.error('MQTT error:', err);
    process.exit(1);
}); 