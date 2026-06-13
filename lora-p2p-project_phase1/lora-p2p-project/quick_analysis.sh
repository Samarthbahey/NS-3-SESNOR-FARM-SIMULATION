#!/bin/bash
echo "=== LoRa P2P Image Transfer Analysis ==="
echo ""
echo "Running tests with different configurations..."
echo ""

cd /home/samarth/ns-allinone-3.45/ns-3.45

echo "Test 1: 100 KB, 0% loss"
timeout 60 ./ns3 run "lora-p2p-transfer --size=100 --loss=0.0 --time=60" 2>&1 | grep -E "Chunks sent|Chunks received|Retransmissions|Duration"

echo ""
echo "Test 2: 100 KB, 10% loss"
timeout 90 ./ns3 run "lora-p2p-transfer --size=100 --loss=0.1 --time=80" 2>&1 | grep -E "Chunks sent|Chunks received|Retransmissions|Duration"

echo ""
echo "Test 3: 100 KB, 20% loss"
timeout 120 ./ns3 run "lora-p2p-transfer --size=100 --loss=0.2 --time=100" 2>&1 | grep -E "Chunks sent|Chunks received|Retransmissions|Duration"

echo ""
echo "=== Analysis Complete ==="
