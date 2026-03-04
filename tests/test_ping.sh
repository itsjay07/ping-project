#!/bin/bash

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "========================================"
echo "🧪 PING COMMAND TEST SUITE"
echo "========================================"

# Test 1: Basic ping
echo -n "Test 1: Basic ping (4 packets)... "
sudo ../ping -c 4 1.1.1.1 > /tmp/ping_test1.log 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
fi

# Test 2: Count option
echo -n "Test 2: Count option (-c 3)... "
COUNT=$(sudo ../ping -c 3 1.1.1.1 2>&1 | grep "packets transmitted" | awk '{print $1}')
if [ "$COUNT" -eq 3 ]; then
    echo -e "${GREEN}PASSED${NC}"
else
    echo -e "${RED}FAILED${NC}"
fi

echo "========================================"
echo "✅ Test complete!"
