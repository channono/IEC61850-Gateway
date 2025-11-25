#!/bin/bash

# IED Simulator Quick Start Script
# Usage: ./start_ied_simulator.sh [simulator_type] [port]

set -e

SIMULATOR_TYPE="${1:-basic_io}"
PORT="${2:-10102}"
BUILD_DIR="/Users/danielliu/IEC6850/deps/libiec61850/build/examples"

echo "🚀 Starting IEC61850 IED Simulator..."
echo "📦 Type: ${SIMULATOR_TYPE}"
echo "🔌 Port: ${PORT}"
echo ""

case "$SIMULATOR_TYPE" in
    basic_io)
        EXEC="${BUILD_DIR}/server_example_basic_io/server_example_basic_io"
        echo "✅ Starting Basic I/O Server (4 analog inputs + 4 digital outputs)"
        ;;
    goose)
        EXEC="${BUILD_DIR}/server_example_goose/server_example_goose"
        echo "✅ Starting GOOSE Server"
        ;;
    control)
        EXEC="${BUILD_DIR}/server_example_control/server_example_control"
        echo "✅ Starting Control Server"
        ;;
    config)
        EXEC="${BUILD_DIR}/server_example_config_file/server_example_config_file"
        echo "✅ Starting Config File Server"
        ;;
    *)
        echo "❌ Unknown simulator type: ${SIMULATOR_TYPE}"
        echo "Available types: basic_io, goose, control, config"
        exit 1
        ;;
esac

if [ ! -f "$EXEC" ]; then
    echo "❌ Simulator not found: $EXEC"
    echo "Please build libiec61850 first."
    exit 1
fi

echo ""
echo "ℹ️  Press Ctrl+C to stop the simulator"
echo "----------------------------------------"

# Run the simulator
"$EXEC" "$PORT"
