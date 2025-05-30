import 'package:shared_preferences/shared_preferences.dart';

class AuthService {
  static const String _isLoggedInKey = 'isLoggedIn';
  static const String _userEmailKey = 'userEmail';
  static const String _userNameKey = 'userName';

  // Check if user is logged in
  static Future<bool> isLoggedIn() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getBool(_isLoggedInKey) ?? false;
  }

  // Get user email from storage
  static Future<String?> getUserEmail() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString(_userEmailKey);
  }

  // Get user name from storage
  static Future<String?> getUserName() async {
    final prefs = await SharedPreferences.getInstance();
    return prefs.getString(_userNameKey);
  }

  // Login user (simplified - in real app you'd validate against server)
  static Future<bool> login(String email, String password) async {
    // Simulate login validation (replace with actual API call)
    if (email.isNotEmpty && password.length >= 6) {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setBool(_isLoggedInKey, true);
      await prefs.setString(_userEmailKey, email);

      // Extract name from email or use provided name
      String name = email.split('@')[0];
      await prefs.setString(_userNameKey, name);

      return true;
    }
    return false;
  }

  // Register user (simplified - in real app you'd create account on server)
  static Future<bool> register(
    String name,
    String email,
    String password,
  ) async {
    // Simulate registration validation (replace with actual API call)
    if (name.isNotEmpty && email.isNotEmpty && password.length >= 6) {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setBool(_isLoggedInKey, true);
      await prefs.setString(_userEmailKey, email);
      await prefs.setString(_userNameKey, name);

      return true;
    }
    return false;
  }

  // Logout user
  static Future<void> logout() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setBool(_isLoggedInKey, false);
    await prefs.remove(_userEmailKey);
    await prefs.remove(_userNameKey);
  }
}
