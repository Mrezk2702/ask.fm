# Ask.fm Clone

A C++ implementation of a question-and-answer platform inspired by Ask.fm. This project provides a desktop application where users can sign up, log in, ask questions to other users (optionally anonymously), answer received questions, and view feeds of answered questions. The application features a graphical user interface built with Qt and uses file-based storage for data persistence.

## Features

- **User Authentication**: Secure signup and login system with password hashing and session management.
- **Question Management**: Users can ask questions to other users, with support for anonymous questions.
- **Answering System**: Recipients can answer questions, and answers are displayed in the feed.
- **Feed and Inbox**: Users have an inbox for unanswered questions and a feed showing answered questions (both asked and received).
- **Threaded Conversations**: Support for follow-up questions and replies in threads.
- **Data Persistence**: File-based storage system with locking mechanisms for concurrent access.
- **Graphical User Interface**: Intuitive Qt-based interface for desktop use.
- **Testing Suite**: Comprehensive unit tests using Google Test framework.

## Architecture

The project is structured as follows:

- **src/**: Main source code
  - **ask_fm/**: Core application logic
  - **auth/**: Authentication and session management
  - **gui/**: Qt-based graphical user interface components
  - **models/**: Data structures for users, questions, and sessions
  - **questionManager/**: Business logic for question operations
  - **storage/**: File-based data storage and retrieval

- **tests/**: Unit tests for verifying functionality
- **scripts/**: Build and maintenance scripts
- **CMakeLists.txt**: Build configuration file

### Technologies Used

- **Programming Language**: C++17
- **GUI Framework**: Qt 5/6
- **Build System**: CMake
- **Testing Framework**: Google Test
- **Data Storage**: Custom file-based system with JSON-like serialization

## Dependencies

- CMake (version 3.16 or higher)
- Qt5 or Qt6 development libraries
- Google Test (automatically downloaded via CMake)
- C++ compiler with C++17 support (e.g., GCC, Clang)

## Build Instructions

1. Clone the repository:
   ```
   git clone <repository-url>
   cd ask_fm
   ```

2. Create a build directory and configure with CMake:
   ```
   mkdir build
   cd build
   cmake ..
   ```

3. Build the project:
   ```
   make
   ```

   Or using CMake:
   ```
   cmake --build .
   ```

This will build the main application (`askFM_gui`) and the test suite (`askFM_tests`).

## Usage

### Running the Application

After building, run the GUI application:
```
./askFM_gui
```

The application will launch a desktop interface where users can:
- Sign up for new accounts
- Log in to existing accounts
- Ask questions to other users
- Answer received questions
- View their feed and inbox
- Participate in threaded conversations

### Running Tests

To run the test suite:
```
./askFM_tests
```

For specific test suites:
```
./askFM_tests --gtest_filter=TestSuiteName.*
```

## Testing

The project includes comprehensive unit tests covering:
- User authentication
- Question management
- Data storage operations
- GUI components

Tests are automatically built with the project and can be run using the Google Test framework.

## Screenshots

![Application Screenshot](screenshot.png)

*Main application interface showing the feed and inbox tabs.*

## Contributing

Contributions are welcome. Please ensure that:
- All new code includes appropriate unit tests
- Code follows the existing style and structure
- Changes are tested on multiple platforms if possible

## License

This project is provided as-is without any specific license. Please refer to the source code for any usage restrictions.
