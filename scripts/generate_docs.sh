#!/bin/bash

# Script to generate Doxygen documentation for Alpakka Firmware

set -e

echo "Generating Doxygen documentation..."

# Check if we're in the project root
if [ ! -f "Doxyfile" ]; then
    echo "Error: Please run this script from the project root directory"
    exit 1
fi

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed. Please install it first:"
    echo "  sudo apt-get install doxygen graphviz"
    exit 1
fi

# Create output directory
mkdir -p docs/doxygen

# Generate documentation using standalone config
doxygen Doxyfile

echo "Documentation generated successfully!"
echo "You can find the documentation in: docs/doxygen/html/index.html" 