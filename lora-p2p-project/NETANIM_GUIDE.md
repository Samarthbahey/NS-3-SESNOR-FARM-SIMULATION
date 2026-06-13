# NetAnim Visualization Guide - LoRa Wireless P2P

## What You Have

✅ **NetAnim XML trace file**: `lora-p2p-transfer.xml`  
✅ **One-way LoRa wireless transmission**: Sender → Receiver  
✅ **Packet animations**: Data chunks transmitted wirelessly

## Understanding the Visualization

**Protocol**: LoRa-style wireless one-way communication  
- Sender transmits image chunks wirelessly (250kbps, LoRa-like speed)
- Receiver gets chunks over the air
- NetAnim shows packet flow between positioned nodes

**Note about the link line**: NetAnim draws a line between nodes to show they can communicate. This represents the **wireless radio range**, not a physical cable. The packets travel wirelessly at LoRa speeds (250kbps).

## How to View the Animation

### Method 1: Using NetAnim GUI (Recommended)

```bash
# Navigate to NetAnim directory
cd /home/samarth/ns-allinone-3.45/netanim-3.109

# Launch NetAnim
./NetAnim

# In the GUI:
# 1. Click "Open XML trace file" (folder icon)
# 2. Browse to: /home/samarth/ns-allinone-3.45/ns-3.45/lora-p2p-transfer.xml
# 3. Click "Play" to watch the animation
```

If NetAnim isn't built yet:
```bash
cd /home/samarth/ns-allinone-3.45/netanim-3.109
qmake NetAnim.pro
make
./NetAnim
```

### Method 2: Check if NetAnim Exists

```bash
find /home/samarth/ns-allinone-3.45 -name "NetAnim" -type f
```

## What You'll See in the Animation

**Nodes:**
- 🔴 **Red Node (left)**: Sender - transmits image chunks
- 🔵 **Blue Node (right)**: Receiver - receives chunks & sends ACKs

**Packet Flow:**
- **Red → Blue**: Data chunks (image pieces)
- **Blue → Red**: ACK packets (acknowledgments)
- **Speed**: Packets move at 250kbps (LoRa-like speed)

**Events to Watch:**
1. Burst of data packets (initial transmission)
2. Periodic ACK packets from receiver
3. Retransmission of missing chunks (if packet loss > 0%)
4. Final ACK exchange when complete

## Generate Animation for Different Scenarios

### No Packet Loss (smooth flow)
```bash
./ns3 run "lora-p2p-transfer --size=3 --loss=0.0 --time=60"
```
**What to see**: Clean, linear packet flow, no retransmissions

### 20% Packet Loss (realistic)
```bash
./ns3 run "lora-p2p-transfer --size=3 --loss=0.2 --time=80"
```
**What to see**: Some packets dropped, retransmissions visible

### 30% Packet Loss (challenging)
```bash
./ns3 run "lora-p2p-transfer --size=5 --loss=0.3 --time=120"
```
**What to see**: Heavy retransmission activity, multiple retry bursts

## Animation File Location

Each run creates/overwrites:
```
/home/samarth/ns-allinone-3.45/ns-3.45/lora-p2p-transfer.xml
```

To save different animations:
```bash
# After each run, rename the file
mv lora-p2p-transfer.xml animations/test_20loss.xml
```

## NetAnim Features You Can Use

In the NetAnim GUI:

**Playback Controls:**
- Play/Pause
- Speed adjustment (slower to see details)
- Step forward/backward
- Jump to specific time

**Display Options:**
- Show packet metadata
- Display node IDs
- Show grid
- Zoom in/out

**Statistics:**
- Total packets sent
- Packets in flight
- Throughput graph
- Packet loss visualization

## Troubleshooting

### NetAnim not installed?
```bash
cd /home/samarth/ns-allinone-3.45/netanim-3.109
# Install Qt5 if needed
sudo apt install qt5-default qtbase5-dev
qmake NetAnim.pro
make
```

### XML file empty or corrupt?
- Ensure simulation ran completely
- Check simulation didn't crash
- Verify `--time` parameter is sufficient

### Can't see packets?
- Increase playback speed in NetAnim
- Check "Show packet metadata" option
- Verify nodes are positioned correctly

## Quick Demo

```bash
# Generate a nice animation
cd /home/samarth/ns-allinone-3.45/ns-3.45
./ns3 run "lora-p2p-transfer --size=2 --loss=0.15 --time=60"

# Launch NetAnim
cd /home/samarth/ns-allinone-3.45/netanim-3.109
./NetAnim

# Load: /home/samarth/ns-allinone-3.45/ns-3.45/lora-p2p-transfer.xml
# Press Play and watch!
```

## Understanding the Visualization

**Timeline:**
- 0-1s: Setup, applications start
- 1s+: Initial chunk transmission (burst)
- Every 2s: ACK packets from receiver
- 10s intervals: Sender checks for missing chunks
- Retransmission: Scattered packets for missing data
- End: Final ACK exchange

**Packet Colors:**
- Usually alternating colors for different packet types
- Watch the flow direction (arrows)

**Performance Indicators:**
- Smooth flow = no packet loss
- Scattered retransmissions = packet loss present
- Delayed ACKs = network congestion

## Advanced: Animation with Custom Scenarios

Edit the test script to generate animations for all tests:

```bash
# In run-tests.sh, after each test:
mv lora-p2p-transfer.xml $RESULTS_DIR/${name}_animation.xml
```

Then you'll have animations for:
- 1KB_NoLoss_animation.xml
- 5KB_20Loss_animation.xml
- etc.

## File Size

Animation files are typically:
- 2-5 KB for small transfers (1-2 KB)
- 10-20 KB for medium (5-10 KB)
- 50+ KB for large transfers with high loss

## Alternative: Extract Stats from XML

```bash
# Count packets
grep "<packet" lora-p2p-transfer.xml | wc -l

# View first packet
grep -m 1 "<packet" lora-p2p-transfer.xml
```

---

**Enjoy visualizing your LoRa P2P transfers!** 🎬
