#!/bin/bash

# Exit immediately if a command fails
set -e

# Clean build directory command
if ["$1" == "clean"]; then
	echo "Cleaning build directory..."
	rm -rf build
	echo "Build directory removed."
	exit 0
fi

# Create directory and build the project
mkdir -p build
cd build
cmake ..
cmake --build .

# Running the app
./ccraft

