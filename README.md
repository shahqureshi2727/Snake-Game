
# Terminal-Based Snake Game (C + ncurses)

This is a classic Snake game built in **C**, focused on practicing low-level programming techniques and efficient system resource usage.

## Features

- **Real-Time Mechanics**: Smooth and responsive snake movement and collision detection.
- **Data Structures**: Uses arrays and linked lists to manage the snake's body and game state.
- **Terminal UI**: Cross-platform interface built using the **ncurses** library.
- **Performance Optimization**: Manages dynamic memory allocation and implements efficient event loops to reduce CPU usage.

## Requirements

- GCC or another C compiler
- `ncurses` library installed (`libncurses-dev` on Debian-based systems)

## How to Build and Run

1. **Clone the Repository**
   ```bash
   git clone https://github.com/yourusername/snake-game.git
   cd snake-game
   ```

2. **Compile the Game**
   ```bash
   gcc -o snake main.c -lncurses
   ```

3. **Run the Game**
   ```bash
   ./snake
   ```

## Controls

- Arrow Keys – Move the snake
- `q` – Quit the game

## License

This project is licensed under the MIT License.
