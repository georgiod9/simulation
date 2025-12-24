#! /bin/sh
clang++ src/main.cpp -o simulation \
   -I/opt/homebrew/include \
   -L/opt/homebrew/lib \
   -lsfml-graphics -lsfml-window -lsfml-system \
   -lbox2d \
   -std=c++17