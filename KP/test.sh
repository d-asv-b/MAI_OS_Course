#!/bin/bash

echo "Starting server..."
cd build
./server &
SERVER_PID=$!

sleep 1

echo "Starting client..."
./client &
CLIENT_PID=$!

sleep 2

echo "Sending HELP command..."
# Send HELP command to client
echo "HELP" | kill -USR1 $CLIENT_PID 2>/dev/null || true

sleep 1

echo "Stopping client..."
kill $CLIENT_PID 2>/dev/null || true

echo "Stopping server..."
kill $SERVER_PID 2>/dev/null || true

wait
echo "Test completed."

