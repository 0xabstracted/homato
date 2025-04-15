# Contributing to Homato

Thank you for your interest in contributing to Homato! This document provides guidelines and instructions for contributing to this project.

## Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md).

## How to Contribute

### Reporting Bugs

1. Ensure the bug hasn't already been reported by searching GitHub Issues
2. If you can't find an existing issue, open a new one with:
   - A clear, descriptive title
   - Detailed steps to reproduce the bug
   - Expected vs actual behavior
   - Screenshots or code snippets if applicable
   - Environment information (OS, browser, device, etc.)

### Suggesting Features

1. Check if the feature has already been suggested in GitHub Issues
2. If not, open a new issue with:
   - A clear title prefixed with "Feature:"
   - Detailed description of the proposed feature
   - Explanation of why this feature would be useful
   - Any implementation ideas you might have

### Pull Requests

1. Fork the repository
2. Create a new branch from `main` for your changes
3. Make your changes and commit with clear messages
4. Test your changes thoroughly
5. Push your branch and create a pull request
6. Include in your PR:
   - Reference to any related issues
   - Description of the changes
   - Screenshots for UI changes

## Development Environment Setup

1. Clone the repository
   ```
   git clone https://github.com/yourusername/homato.git
   cd homato
   ```

2. Install dependencies
   ```
   cd homato-control-system
   npm install
   ```

3. Set up environment variables
   ```
   cp .env.example .env
   # Edit .env with your configuration
   ```

4. Run the development server
   ```
   npm run dev
   ```

## Coding Standards

- Follow existing code style and conventions
- Keep your code clean and well-documented
- Write clear, descriptive commit messages
- Add or update tests as necessary

### JavaScript Style Guidelines

- Use ES6+ features when appropriate
- Follow standard JS naming conventions
- Properly document functions and classes
- Maintain responsive design principles for frontend

### Firmware Style Guidelines

- Follow Arduino coding style
- Document hardware connections
- Add comments for complex logic
- Test on actual hardware before submission

## License

By contributing to Homato, you agree that your contributions will be licensed under the project's [MIT License](LICENSE). 