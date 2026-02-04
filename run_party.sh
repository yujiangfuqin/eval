#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <process_name>"
    exit 1
fi

PROCESS_NAME=$1
BUILD_DIR="build"

if [ ! -f "$BUILD_DIR/$PROCESS_NAME" ]; then
    echo "Error: Executable '$PROCESS_NAME' not found in '$BUILD_DIR'"
    exit 1
fi

# Clear old pids file
> pids.log

echo "Starting $PROCESS_NAME for player0..."
$BUILD_DIR/$PROCESS_NAME player0 > player0.log 2>&1 &
PID0=$!
echo $PID0 >> pids.log

echo "Starting $PROCESS_NAME for player1..."
$BUILD_DIR/$PROCESS_NAME player1 > player1.log 2>&1 &
PID1=$!
echo $PID1 >> pids.log

echo "Starting $PROCESS_NAME for player2..."
$BUILD_DIR/$PROCESS_NAME player2 > player2.log 2>&1 &
PID2=$!
echo $PID2 >> pids.log

echo "Processes started. PIDs: $PID0, $PID1, $PID2 saved to pids.log"
