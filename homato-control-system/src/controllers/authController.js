const User = require('../models/user');

// Register a new user
const signup = async (req, res) => {
    try {
        const user = new User(req.body);
        const token = await user.generateAuthToken();
        res.status(201).json({ user, token });
    } catch (error) {
        res.status(400).json({ error: error.message });
    }
};

// Login user
const login = async (req, res) => {
    try {
        const { email, password } = req.body;
        const user = await User.findByCredentials(email, password);
        const token = await user.generateAuthToken();
        res.json({ user, token });
    } catch (error) {
        res.status(401).json({ error: 'Invalid login credentials' });
    }
};

// Logout user
const logout = async (req, res) => {
    try {
        req.user.tokens = req.user.tokens.filter(token => token.token !== req.token);
        await req.user.save();
        res.json({ message: 'Logged out successfully' });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
};

// Get user profile
const getProfile = async (req, res) => {
    res.json(req.user);
};

module.exports = {
    signup,
    login,
    logout,
    getProfile
}; 