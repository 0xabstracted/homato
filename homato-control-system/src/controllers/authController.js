const User = require('../models/user');

// Register a new user
const signup = async (req, res) => {
    try {
        if (!req.body.name || !req.body.email || !req.body.password || !req.body.phone) {
            return res.status(400).json({ error: 'Name, email, password and phone are required' });
        }
        const checkUser = await User.findOne({ email: req.body.email });
        if (checkUser) {
            return res.status(400).json({ error: 'Email already exists' });
        }
        let body = {
            name: req.body.name,
            email: req.body.email,
            password: req.body.password,
            phone: req.body.phone,
            is_active: true,
            email_verified: false,
            phone_verified: false,
            role: req.body.role || 'user',
        }
        const user = new User(body);
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