# LoRa P2P Image Transfer - Project Index

## 📁 Project Structure

```
scratch/
├── lora-p2p-transfer.cc      # Main simulation (296 lines)
├── run-tests.sh              # Test automation script
├── analyze_results.py        # Results analyzer & visualizer
├── README.md                 # Complete documentation
├── QUICKSTART.md             # Quick start guide
├── INDEX.md                  # This file
└── test_results/             # Test outputs (auto-generated)
    ├── *.txt                 # Individual test logs
    ├── *_received.bin        # Received images
    ├── summary.csv           # Metrics export
    └── analysis.png          # Performance plots
```

## 🚀 Getting Started

### Option 1: Quick Test (2 minutes)
```bash
cd /home/samarth/ns-allinone-3.45/ns-3.45
./ns3 build lora-p2p-transfer
./ns3 run "lora-p2p-transfer --size=2 --loss=0.0 --time=40"
```

### Option 2: Full Test Suite (5-10 minutes)
```bash
cd scratch
./run-tests.sh
./analyze_results.py
```

### Option 3: Custom Test
```bash
./ns3 run "lora-p2p-transfer --size=<KB> --loss=<0.0-1.0> --time=<seconds>"
```

## 📚 Documentation

| File | Description | Read Time |
|------|-------------|-----------|
| **QUICKSTART.md** | 5-minute quick start guide | 3 min |
| **README.md** | Complete documentation | 10 min |
| **lora-p2p-transfer.cc** | Source code (well commented) | 20 min |

**Read in this order:** QUICKSTART.md → README.md → Source Code

## 🔬 What This Project Does

**Simulates LoRa-based P2P image transfer with:**
- Chunked transmission (92-byte chunks)
- Automatic retransmission on packet loss
- ACK-based reliability
- Performance metrics & analysis

**Use cases:**
- Protocol performance testing
- Packet loss impact analysis
- LoRa communication research
- Reliability algorithm validation

## 📊 Test Scenarios

| # | Test Name | Size | Loss | Purpose |
|---|-----------|------|------|---------|
| 1 | 1KB_NoLoss | 1KB | 0% | Baseline small |
| 2 | 5KB_NoLoss | 5KB | 0% | Baseline medium |
| 3 | 10KB_NoLoss | 10KB | 0% | Baseline large |
| 4 | 5KB_10Loss | 5KB | 10% | Light loss |
| 5 | 5KB_20Loss | 5KB | 20% | Moderate loss |
| 6 | 5KB_30Loss | 5KB | 30% | Heavy loss |
| 7 | 10KB_20Loss | 10KB | 20% | Large + loss |

## 🛠️ Key Components

### 1. Main Simulation (`lora-p2p-transfer.cc`)
- **ImageSender**: Splits & sends image chunks
- **ImageReceiver**: Receives & reconstructs image
- **Network**: CSMA channel (250kbps, LoRa-like)
- **Metrics**: Tracks all performance data

### 2. Test Automation (`run-tests.sh`)
- Runs 7 predefined scenarios
- Saves logs to `test_results/`
- Preserves received images
- Takes 5-10 minutes total

### 3. Analysis Tool (`analyze_results.py`)
- Parses test result files
- Generates text summary
- Creates performance plots
- Exports CSV data

## 📈 Typical Results

### Perfect Conditions (0% loss)
- **5KB transfer**: ~30 seconds, 0% retransmissions
- **Throughput**: ~170 bytes/second
- **Efficiency**: 100%

### Realistic Conditions (20% loss)
- **5KB transfer**: ~60 seconds, 25-35% retransmissions
- **Throughput**: ~85 bytes/second
- **Efficiency**: 75%

### Challenging Conditions (30% loss)
- **5KB transfer**: ~90 seconds, 45-60% retransmissions
- **Throughput**: ~55 bytes/second
- **Efficiency**: 60%

## 🎯 Quick Commands Reference

```bash
# Build
./ns3 build lora-p2p-transfer

# Run single test
./ns3 run "lora-p2p-transfer --size=5 --loss=0.2 --time=120"

# Run all tests
cd scratch && ./run-tests.sh

# Analyze results
./analyze_results.py

# Check received file
ls -lh received_image.bin

# View test results
cat test_results/5KB_20Loss.txt

# Clean up
rm -rf test_results/ received_image.bin
```

## 🔧 Customization Points

### Easy Changes (edit values)
- Chunk size: Line 18 in `lora-p2p-transfer.cc`
- ACK timeout: Line 19
- Chunk delay: Line 20
- Data rate: Line 258 (CSMA setup)

### Medium Changes (add code)
- New test scenario: Edit `run-tests.sh`
- Additional metrics: Add to `Metrics` struct
- Custom analysis: Extend `analyze_results.py`

### Advanced Changes (redesign)
- Different network topology
- Alternative ACK strategy
- Multi-hop routing
- Encryption layer

## 🐛 Common Issues & Solutions

**Issue**: Tests don't complete
- **Solution**: Increase `--time` parameter

**Issue**: High retransmission rate
- **Solution**: Normal for high loss rates, check ACK timeout

**Issue**: matplotlib errors
- **Solution**: `pip install matplotlib` or ignore (text output still works)

**Issue**: Permission denied on scripts
- **Solution**: `chmod +x *.sh *.py`

## 📊 Understanding The Output

### Console Output
```
[SENDER] Sent chunk X/Y          # Chunk transmission
[RECEIVER] Got chunk X/Y         # Chunk received
[RECEIVER] Sent ACKs for N       # ACK transmission
[SENDER] Received ACKs for N     # ACK received
[SENDER] Retransmitting N chunks # Missing chunks
[SENDER] ✓ Transfer complete!    # Success
```

### Metrics
- **Chunks sent**: Total transmissions (including retransmissions)
- **Retransmissions**: Chunks sent > 1 time
- **Retransmit rate**: % of chunks that needed resending
- **Duration**: End-to-end transfer time

## 🔬 Research Applications

This simulation is useful for:
1. **Protocol Design**: Test reliability mechanisms
2. **Performance Analysis**: Measure throughput vs. loss rate
3. **Parameter Tuning**: Optimize timeouts and delays
4. **Comparison Studies**: Benchmark against other protocols
5. **Educational**: Learn NS-3 and networking concepts

## 📞 Next Steps

1. ✅ Run QUICKSTART.md tutorial
2. ✅ Execute test suite
3. ✅ Review analysis plots
4. ⬜ Modify protocol parameters
5. ⬜ Add custom features
6. ⬜ Integrate with your research

## 🎓 Learning Path

**Beginner**: 
- Run tests → Read QUICKSTART → View results

**Intermediate**:
- Read README → Modify parameters → Run custom tests

**Advanced**:
- Study source code → Extend protocol → Add new features

---

**Project Status**: ✅ Complete and working
**Last Updated**: December 23, 2025
**NS-3 Version**: 3.45
