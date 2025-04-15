const { verifyToken } = require('../config/jwt');

exports.authenticate = async (req, res, next) => {
    try {
        const token = req.headers.authorization?.split(' ')[1];

        if (!token) {
            return res.status(401).json({ message: 'No token provided' });
        }

        const decoded = verifyToken(token);
        req.user = decoded;
        next();
    } catch (error) {
        return res.status(401).json({ message: 'Invalid token' });
    }
}; 