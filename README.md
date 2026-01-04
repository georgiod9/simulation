# C++ 2D Simulation Starter

## Quick Start

1. Install dependencies:

```
brew install sfml box2d cmake entr
```

2. In root, run build script:

```
sh build.sh
```

3. In root, run dev server (with recompile on save):

```
./dev.sh
```

---

## Libraries

### Rendering: **SFML** (Simple and Fast Multimedia Library)

- Beginner-friendly API
- Cross-platform (Windows, macOS, Linux)
- Install: `brew install sfml` (macOS) or download from [sfml-dev.org](https://www.sfml-dev.org/)

### Physics: **Box2D**

- Used in many games and simulations
- Install: `brew install box2d` (macOS) or download from [box2d.org](https://box2d.org/)

## Project Structure

```
simulation/
├── src/
│   └── main.cpp          # Entry point
├── CMakeLists.txt        # Build configuration
├── build.sh              # Build script
├── run.sh                # Run the built application (unix executable)
├── dev.sh                # Start server with entr (hot reload enabled)
└── README.md
```
