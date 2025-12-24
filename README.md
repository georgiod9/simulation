# C++ 2D Simulation Starter

## Recommended Libraries

### Rendering: **SFML** (Simple and Fast Multimedia Library)

- Beginner-friendly API
- Cross-platform (Windows, macOS, Linux)
- Handles windowing, input, 2D graphics, audio
- Great documentation and community
- Install: `brew install sfml` (macOS) or download from [sfml-dev.org](https://www.sfml-dev.org/)

### Physics: **Box2D**

- Industry-standard 2D physics engine
- Used in many games and simulations
- Well-documented with tutorials
- Install: `brew install box2d` (macOS) or download from [box2d.org](https://box2d.org/)

### Alternative: **Raylib** + **Physac**

- Raylib: Even simpler than SFML, great for prototyping
- Physac: Lightweight physics engine that integrates with Raylib
- Good if you want something more minimal

## Why Not WebGL?

WebGL is a JavaScript API for web browsers. For native C++:

- Use **SFML** (easiest) or **SDL2** (more control) for 2D
- Use **OpenGL** (more complex) if you need 3D or advanced graphics

## Project Structure

```
simulation/
├── src/
│   └── main.cpp          # Entry point
├── CMakeLists.txt        # Build configuration
└── README.md
```

## Quick Start

1. Install dependencies: `brew install sfml box2d cmake`
2. Build: `mkdir build && cd build && cmake .. && make`
3. Run: `./simulation`
