# Frogger Game

Get the latest release on the [release page!](https://github.com/ChrisG661/frogger/releases/latest/download/frogger.exe)

# Development
```shell
# Clone the repository
git clone https://github.com/ChrisG661/frogger.git
cd frogger
# Build using cmake
cmake -B build -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Debug -S . -G "MinGW Makefiles"
cmake --build build --config Debug
# Run the game
./build/frogger.exe
```

