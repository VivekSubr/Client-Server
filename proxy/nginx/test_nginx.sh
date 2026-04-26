#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_IP=127.0.0.1
CLIENT_IP=127.0.0.1
SERVER_PORT=1447
CLIENT_PORT=10000

GO_SERVER="$SCRIPT_DIR/go_server/go_server"
GO_CLIENT="$SCRIPT_DIR/go_client/http2_client"

SERVER_LOG="$SCRIPT_DIR/server.log"
CLIENT_LOG="$SCRIPT_DIR/client.log"

SERVER_PID=""
CLIENT_PID=""

free_port() {
	local port="$1"
	local pids
	pids=$(ss -ltnp "sport = :$port" 2>/dev/null | sed -n 's/.*pid=\([0-9]\+\).*/\1/p' | sort -u)
	if [[ -n "$pids" ]]; then
		kill $pids >/dev/null 2>&1 || true
	fi
}

cleanup() {
	set +e
	if [[ -n "$CLIENT_PID" ]]; then
		kill "$CLIENT_PID" >/dev/null 2>&1 || true
	fi
	if [[ -n "$SERVER_PID" ]]; then
		kill "$SERVER_PID" >/dev/null 2>&1 || true
	fi

	nginx -p "$SCRIPT_DIR" -c "$SCRIPT_DIR/server.conf" -s stop >/dev/null 2>&1 || true
}

trap cleanup EXIT INT TERM

# Clear stale listeners from earlier failed test runs.
free_port "$CLIENT_PORT"
free_port "$SERVER_PORT"

# Build local go server/client used by this test.
make -C "$SCRIPT_DIR/go_server" clean build
make -C "$SCRIPT_DIR/go_client" clean build

nginx -t -p "$SCRIPT_DIR" -c "$SCRIPT_DIR/server.conf"

: > "$SERVER_LOG"
: > "$CLIENT_LOG"

"$GO_SERVER" -ip="$SERVER_IP" -port="$SERVER_PORT" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!
sleep 1

if ! kill -0 "$SERVER_PID" >/dev/null 2>&1; then
	echo "test failed"
	echo "----- server.log -----"
	cat "$SERVER_LOG"
	exit 1
fi

nginx -p "$SCRIPT_DIR" -c "$SCRIPT_DIR/server.conf"

"$GO_CLIENT" -ip="$CLIENT_IP" -port="$CLIENT_PORT" > "$CLIENT_LOG" 2>&1 &
CLIENT_PID=$!

if timeout 45s bash -c 'tail -n +1 -F "$1" | grep -m1 "^Replied$"' _ "$SERVER_LOG"; then
	echo "test passed"
else
	echo "test failed"
	echo "----- nginx error.log -----"
	cat "$SCRIPT_DIR/error.log" || true
	echo "----- server.log -----"
	cat "$SERVER_LOG"
	echo "----- client.log -----"
	cat "$CLIENT_LOG"
	exit 1
fi
