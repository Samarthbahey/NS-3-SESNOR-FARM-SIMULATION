#!/bin/bash
# Automated test suite for LoRa P2P Image Transfer

echo "==================================================="
echo "  LoRa P2P Image Transfer - Test Suite"
echo "==================================================="
echo ""

# Create results directory
RESULTS_DIR="test_results"
mkdir -p $RESULTS_DIR
rm -f $RESULTS_DIR/*.txt

# Test scenarios
declare -a SCENARIOS=(
    "1KB_NoLoss:1:0.0:60"
    "5KB_NoLoss:5:0.0:80"
    "10KB_NoLoss:10:0.0:120"
    "5KB_10Loss:5:0.1:100"
    "5KB_20Loss:5:0.2:120"
    "5KB_30Loss:5:0.3:150"
    "10KB_20Loss:10:0.2:180"
)

echo "Running ${#SCENARIOS[@]} test scenarios..."
echo ""

for scenario in "${SCENARIOS[@]}"; do
    IFS=':' read -r name size loss time <<< "$scenario"
    
    echo "----------------------------------------"
    echo "Test: $name"
    echo "  Image Size: ${size}KB"
    echo "  Packet Loss: ${loss}"
    echo "  Time: ${time}s"
    echo "----------------------------------------"
    
    # Run simulation
    ./ns3 run "lora-p2p-transfer --size=$size --loss=$loss --time=$time" 2>&1 | tee "$RESULTS_DIR/${name}.txt"
    
    # Save received image if exists
    if [ -f "received_image.bin" ]; then
        mv received_image.bin "$RESULTS_DIR/${name}_received.bin"
        echo "✓ Image saved to $RESULTS_DIR/${name}_received.bin"
    fi
    
    echo ""
done

echo "==================================================="
echo "  All tests complete!"
echo "  Results saved in: $RESULTS_DIR/"
echo "==================================================="
echo ""
echo "Run './analyze_results.py' to generate analysis and plots"
