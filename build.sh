#!/bin/bash
# Build and Flash Script for M5StickC Plus SubGHz Tool

# Set the PlatformIO binary path
PIO=~/.platformio/penv/bin/platformio

# Change to project directory
cd "$(dirname "$0")"

echo "=================================="
echo "M5StickC Plus SubGHz Tool Builder"
echo "=================================="
echo ""

# Parse arguments
case "$1" in
    "build")
        echo "Building project..."
        $PIO run
        ;;
    "upload")
        echo "Building and uploading to device..."
        $PIO run --target upload
        ;;
    "monitor")
        echo "Opening serial monitor..."
        $PIO device monitor
        ;;
    "clean")
        echo "Cleaning build files..."
        $PIO run --target clean
        ;;
    "all")
        echo "Building, uploading, and monitoring..."
        $PIO run --target upload && $PIO device monitor
        ;;
    *)
        echo "Usage: $0 {build|upload|monitor|clean|all}"
        echo ""
        echo "Commands:"
        echo "  build    - Compile the project"
        echo "  upload   - Compile and upload to M5StickC Plus"
        echo "  monitor  - Open serial monitor"
        echo "  clean    - Clean build files"
        echo "  all      - Build, upload, and monitor"
        exit 1
        ;;
esac

exit 0
