#!/bin/bash
set -e

SERVER="127.0.0.1:7878"

echo "=== Test 1: GET / (expect 200 OK) ==="
response=$(curl -s -w "\nHTTP_CODE:%{http_code}" http://$SERVER/)
http_code=$(echo "$response" | grep -o 'HTTP_CODE:.*' | cut -d: -f2)
body=$(echo "$response" | sed '/HTTP_CODE:/d')

if [ "$http_code" -eq 200 ]; then
    echo "PASS: Got 200 OK"
else
    echo "FAIL: Expected 200, got $http_code"
fi
echo "Response body:"
echo "$body"
echo ""

echo "=== Test 2: GET /nonexistent (expect 404) ==="
response=$(curl -s -w "\nHTTP_CODE:%{http_code}" http://$SERVER/nonexistent)
http_code=$(echo "$response" | grep -o 'HTTP_CODE:.*' | cut -d: -f2)
body=$(echo "$response" | sed '/HTTP_CODE:/d')

if [ "$http_code" -eq 404 ]; then
    echo "PASS: Got 404 Not Found"
else
    echo "FAIL: Expected 404, got $http_code"
fi
echo "Response body:"
echo "$body"
echo ""

echo "=== Tests complete ==="

