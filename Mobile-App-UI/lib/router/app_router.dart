import 'package:go_router/go_router.dart';
import '../screens/index.dart';
import '../widgets/scaffold_with_nav_bar.dart';

// GoRouter configuration
final GoRouter appRouter = GoRouter(
  initialLocation: '/',
  routes: [
    // Splash screen route
    GoRoute(
      path: '/',
      pageBuilder:
          (context, state) => const NoTransitionPage(child: SplashScreen()),
    ),

    // Login route
    GoRoute(
      path: '/login',
      pageBuilder:
          (context, state) => const NoTransitionPage(child: LoginScreen()),
    ),

    // Signup route
    GoRoute(
      path: '/signup',
      pageBuilder:
          (context, state) => const NoTransitionPage(child: SignupScreen()),
    ),

    // Bottom navigation shell route (authenticated routes)
    ShellRoute(
      builder: (context, state, child) {
        return ScaffoldWithNavBar(child: child);
      },
      routes: [
        // Rooms tab routes
        GoRoute(
          path: '/rooms',
          pageBuilder:
              (context, state) => const NoTransitionPage(child: RoomsPage()),
          routes: [
            GoRoute(
              path: '/devices',
              pageBuilder:
                  (context, state) =>
                      const NoTransitionPage(child: DevicesPage()),
              routes: [
                GoRoute(
                  path: '/:id',
                  pageBuilder:
                      (context, state) => NoTransitionPage(
                        child: DevicePage(id: state.pathParameters['id'] ?? ''),
                      ),
                ),
              ],
            ),
          ],
        ),

        // Home tab routes
        GoRoute(
          path: '/home',
          pageBuilder:
              (context, state) => const NoTransitionPage(child: HomePage()),
          routes: [
            GoRoute(
              path: '/ble',
              pageBuilder:
                  (context, state) =>
                      const NoTransitionPage(child: BLEScreen()),
            ),
            GoRoute(
              path: '/settings',
              pageBuilder:
                  (context, state) =>
                      const NoTransitionPage(child: SettingsPage()),
            ),
          ],
        ),

        // Profile tab routes
        GoRoute(
          path: '/profile',
          pageBuilder:
              (context, state) => const NoTransitionPage(child: ProfilePage()),
        ),
      ],
    ),
  ],
);
