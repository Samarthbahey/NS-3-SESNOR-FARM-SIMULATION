# Quick Start Guide - LoRa P2P Image Transfer

## 1. First Time Setup (30 seconds)

```bash
# Navigate to NS-3 directory
cd /home/samarth/ns-allinone-3.45/ns-3.45

# Build the project
./ns3 build lora-p2p-transfer
```

## 2. Run Your First Test (1 minute)

```bash
# Simple 2KB transfer with no packet loss
./ns3 run "lora-p2p-transfer --size=2 --loss=0.0 --time=40"
```

**Expected output:**
```
=== LoRa P2P Image Transfer ===
Image: 2 KB
Loss: 0%
Chunk size: 92 bytes

[SENDER] Starting transmission of 23 chunks
[RECEIVER] Got chunk 0/23 (total: 1)
[RECEIVER] Got chunk 1/23 (total: 2)
...
[RECEIVER] ✓ All chunks received!
[SENDER] ✓ Transfer complete!

=== Results ===
Chunks sent: 23
Retransmissions: 0
```

## 3. Verify It Worked

```bash
# Check the received file
ls -lh received_image.bin
# Should show: 2.0K file
```

## 4. Run Complete Test Suite (5 minutes)

```bash
cd scratch
./run-tests.sh
```

This runs 7 different scenarios automatically.

## 5. Analyze Results

```bash
./analyze_results.py
```

You'll see:
- Summary table of all tests
- Performance statistics
- Visualization plots (if matplotlib installed)

## Common Use Cases

### Test with Packet Loss

```bash
# 5KB image with 20% packet loss (realistic scenario)
./ns3 run "lora-p2p-transfer --size=5 --loss=0.2 --time=120"
```

### Large Image Transfer

```bash
# 10KB image (will take ~2 minutes)
./ns3 run "lora-p2p-transfer --size=10 --loss=0.1 --time=180"
```

### Extreme Conditions

```bash
# High packet loss test
./ns3 run "lora-p2p-transfer --size=5 --loss=0.3 --time=150"
```

## Troubleshooting

**Problem**: "command not found"
```bash
# Make scripts executable
chmod +x scratch/run-tests.sh
chmod +x scratch/analyze_results.py
```

**Problem**: "No such file or directory"
```bash
# Ensure you're in the correct directory
cd /home/samarth/ns-allinone-3.45/ns-3.45
pwd  # Should show the ns-3.45 path
```

**Problem**: "Transfer didn't complete"
```bash
# Increase simulation time
./ns3 run "lora-p2p-transfer --size=10 --loss=0.2 --time=300"
```

## What The Parameters Mean

- `--size=X` - Image size in KB (1-100)
- `--loss=Y` - Packet loss from 0.0 (none) to 1.0 (100%)
- `--time=Z` - How long simulation runs (seconds)

**Rule of thumb:** 
- Time needed ≈ (size × 10) + (loss × 100)
- Example: 5KB with 20% loss → (5×10) + (0.2×100) = 70 seconds

## Next Steps

1. **Read full documentation**: `cat scratch/README.md`
2. **Modify parameters**: Edit `scratch/lora-p2p-transfer.cc`
3. **Add custom tests**: Edit `scratch/run-tests.sh`
4. **Analyze patterns**: Review `test_results/analysis.png`

## Key Files

| File | Purpose |
|------|---------|
| `lora-p2p-transfer.cc` | Main simulation code |
| `run-tests.sh` | Automated test runner |
| `analyze_results.py` | Results analysis |
| `README.md` | Full documentation |
| `received_image.bin` | Output from simulation |
| `test_results/` | All test outputs |

---

**Happy Testing!** 🚀
