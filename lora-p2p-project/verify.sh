#!/bin/bash
# Quick verification that everything works

echo "=================================================="
echo "  LoRa P2P Transfer - Quick Verification"
echo "=================================================="
echo ""

cd /home/samarth/ns-allinone-3.45/ns-3.45

# Check if project is built
if [ ! -f "build/scratch/ns3.45-lora-p2p-transfer-default" ]; then
    echo "Building project..."
    ./ns3 build lora-p2p-transfer
    echo ""
fi

# Run a quick 1KB test
echo "Running quick 1KB test (no packet loss)..."
echo ""
./ns3 run "lora-p2p-transfer --size=1 --loss=0.0 --time=40" 2>&1 | grep -E "(LoRa|chunks|Complete|Results|Retrans)"

echo ""
echo "=================================================="
echo "  Verification Complete!"
echo "=================================================="
echo ""

# Check if file was created
if [ -f "received_image.bin" ]; then
    SIZE=$(ls -lh received_image.bin | awk '{print $5}')
    echo "✓ Image successfully received: $SIZE"
    echo ""
fi

echo "Next steps:"
echo "  1. Read documentation: cat scratch/QUICKSTART.md"
echo "  2. Run full tests: cd scratch && ./run-tests.sh"
echo "  3. Analyze results: ./analyze_results.py"
echo ""
