# LED Stick Game

## Introduction

Welcome to LED Stick Game! This is an Arduino-based game that runs on an AZ-Nano V3-Board with ATMEGA328 CH340. In this game, players control a character using buttons and must defeat oncoming enemies. The game features an exciting LED display and offers opportunities for customization and contributions.

## Requirements

To run and contribute to this project, you will need the following:

1. [Arduino IDE](https://www.arduino.cc/en/software): The project is developed in the Arduino IDE, so you'll need to have it installed.

2. Hardware: This game was originally designed for the "AZ-Nano V3-Board with ATMEGA328 CH340," which uses the ATmega328P microcontroller. Ensure you have the necessary hardware or adapt the code to your specific hardware if needed.

   - Choose the `Arduino Nano` board from the `Tools --> Select board` menu.
   - Select `Atmega328P (old bootloader)` from the `Tools --> Select processor` menu.

3. [FastLED Library](https://github.com/FastLED/FastLED): This project utilizes the FastLED library by Daniel Garcia for controlling LED strips. Make sure to install this library in your Arduino IDE.

## Getting Started

Follow these steps to get started with LED Stick Game:

1. **Clone the Repository**: Start by cloning this repository to your local machine using Git:

   ```bash
   git clone <repo-url>
   ```

2. **Open the Arduino IDE**: Launch the Arduino IDE on your computer.

3. **Install FastLED Library**: If you haven't already, install the FastLED library. You can do this by navigating to `Sketch -> Include Library -> Manage Libraries`, searching for "FastLED," and clicking the "Install" button.

4. **Select the Board**: Choose the correct board and processor settings in the Arduino IDE, as mentioned in the "Requirements" section.

5. **Open the Sketch**: In the Arduino IDE, go to `File -> Open` and select the `sketch_WS2812.ino` file from the cloned repository.

6. **Upload the Sketch**: Connect your hardware to your computer, select the appropriate port from the `Tools -> Port` menu, and click the "Upload" button (right arrow) in the Arduino IDE to upload the sketch to your board.

7. **Play the Game**: Once the sketch is uploaded successfully, power on your hardware and play the LED Stick Game!

## How to Play

- Use the buttons connected to your hardware to control the character.
- Press the "UP" and "DOWN" buttons to move the character vertically.
- Press the "ATTACK" button to attack enemies.
- Defeat enemies to progress through the game.
- Enjoy the LED visual effects and aim for victory!

## Contributing

We welcome contributions from the community to enhance this LED Stick Game. Here's how you can contribute:

1. **Fork the Repository**: Click the "Fork" button on the top right of this repository's page to create your own copy.

2. **Clone Your Fork**: Clone your forked repository to your local machine using Git.

   ```bash
   git clone <repo-url>
   ```

3. **Create a Branch**: Create a new branch for your feature or bug fix.

   ```bash
   git checkout -b feature-name
   ```

4. **Make Changes**: Make your desired changes or improvements to the game code.

5. **Test Your Changes**: Test your changes thoroughly to ensure they work as expected.

6. **Commit Changes**: Commit your changes with meaningful commit messages.

   ```bash
   git commit -m "feat: Adds feature xyz"
   ```
    or:
   ```bash
   git commit -m "fix: bug foo-bar"
   ```

7. **Push to Your Fork**: Push your changes to your forked repository on GitHub.

   ```bash
   git push origin feature-name
   ```

8. **Create a Pull Request**: Go to the original repository on GitHub and click the "New Pull Request" button. Compare your changes and submit the pull request.

9. **Review and Collaborate**: Collaborate with the project maintainers and address any feedback or comments on your pull request.

10. **Contribute More**: Continue to contribute to the project by working on new features or addressing open issues.

Enjoy playing and contributing to LED Stick Game! If you have any questions or need assistance, feel free to reach out to us. Happy coding!