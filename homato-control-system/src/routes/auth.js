const express = require('express');
const router = express.Router();
const auth = require('../middleware/auth');
const { signup, login, logout, getProfile } = require('../controllers/authController');

// Public routes
router.post('/signup', signup);
router.post('/login', login);

// Protected routes
router.post('/logout', auth, logout);
router.get('/profile', auth, getProfile);

module.exports = router; 