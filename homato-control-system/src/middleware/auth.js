const jwt = require('jsonwebtoken');
const User = require('../models/user');

const auth = async (req, res, next) => {
    try {
        const token = req.header('Authorization')?.replace('Bearer ', '');

        if (!token) {
            throw new Error('No token provided');
        }

        const decoded = jwt.verify(token, process.env.JWT_SECRET);

        // check if token is expired for 24 hrs
        if (parseInt(decoded.iat) + (60 * 60 * 24) < Math.floor(Date.now() / 1000)) {
            throw new Error('Token expired');
        }

        const user = await User.findOne({ _id: decoded._id });

        if (!user) {
            throw new Error('User not found');
        }

        req.token = token;
        req.user = user;
        next();
    } catch (error) {
        console.log(error);
        res.status(401).json({ error: error.message });
    }
};

module.exports = auth; 