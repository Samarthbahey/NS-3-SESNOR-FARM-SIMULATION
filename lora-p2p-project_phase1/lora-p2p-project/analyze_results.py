#!/usr/bin/env python3
"""
LoRa P2P Image Transfer - Results Analyzer
Parses test results and generates visualizations
"""

import re
import os
import sys
from pathlib import Path

try:
    import matplotlib.pyplot as plt
    import numpy as np
    HAS_MATPLOTLIB = True
except ImportError:
    HAS_MATPLOTLIB = False
    print("Warning: matplotlib not installed. Install with: pip install matplotlib")

class TestResults:
    def __init__(self, name, image_size, loss_rate):
        self.name = name
        self.image_size = image_size
        self.loss_rate = loss_rate
        self.chunks_sent = 0
        self.retransmissions = 0
        self.acks_sent = 0
        self.acks_received = 0
        self.duration = 0
        self.retransmit_rate = 0
        self.transfer_complete = False
        
    def __repr__(self):
        return f"Test({self.name}, size={self.image_size}KB, loss={self.loss_rate}%)"

def parse_results_file(filepath):
    """Parse a single test result file"""
    name = Path(filepath).stem
    
    # Extract parameters from filename
    match = re.match(r'(\d+)KB_(\d+)Loss', name)
    if not match:
        return None
    
    image_size = int(match.group(1))
    loss_rate = int(match.group(2))
    
    result = TestResults(name, image_size, loss_rate)
    
    with open(filepath, 'r') as f:
        content = f.read()
        
        # Parse metrics
        chunks_match = re.search(r'Chunks sent: (\d+)', content)
        retrans_match = re.search(r'Retransmissions: (\d+)', content)
        acks_sent_match = re.search(r'ACKs sent: (\d+)', content)
        acks_recv_match = re.search(r'ACKs received: (\d+)', content)
        retrans_rate_match = re.search(r'Retransmit rate: ([\d.]+)%', content)
        duration_match = re.search(r'Duration: ([\d.]+)s', content)
        complete_match = re.search(r'Transfer complete!', content)
        
        if chunks_match:
            result.chunks_sent = int(chunks_match.group(1))
        if retrans_match:
            result.retransmissions = int(retrans_match.group(1))
        if acks_sent_match:
            result.acks_sent = int(acks_sent_match.group(1))
        if acks_recv_match:
            result.acks_received = int(acks_recv_match.group(1))
        if retrans_rate_match:
            result.retransmit_rate = float(retrans_rate_match.group(1))
        if duration_match:
            result.duration = float(duration_match.group(1))
        if complete_match:
            result.transfer_complete = True
    
    return result

def generate_text_report(results):
    """Generate text summary report"""
    print("\n" + "="*70)
    print("  LoRa P2P Image Transfer - Test Results Summary")
    print("="*70 + "\n")
    
    print(f"{'Test Name':<20} {'Size':<8} {'Loss':<8} {'Chunks':<10} {'Retrans':<10} {'Duration':<12} {'Status'}")
    print("-"*90)
    
    for r in results:
        status = "✓ Complete" if r.transfer_complete else "✗ Failed"
        print(f"{r.name:<20} {r.image_size}KB{'':<4} {r.loss_rate}%{'':<5} "
              f"{r.chunks_sent:<10} {r.retransmissions:<10} {r.duration:.1f}s{'':<8} {status}")
    
    print("\n" + "="*70)
    print("  Performance Analysis")
    print("="*70 + "\n")
    
    # Group by loss rate
    loss_rates = sorted(set(r.loss_rate for r in results))
    
    for loss in loss_rates:
        tests = [r for r in results if r.loss_rate == loss]
        avg_retrans = sum(r.retransmit_rate for r in tests) / len(tests)
        avg_duration = sum(r.duration for r in tests) / len(tests)
        
        print(f"Loss Rate {loss}%:")
        print(f"  Average Retransmission Rate: {avg_retrans:.1f}%")
        print(f"  Average Duration: {avg_duration:.1f}s")
        print(f"  Tests: {len(tests)}")
        print()
    
    # Overall stats
    total_tests = len(results)
    successful = sum(1 for r in results if r.transfer_complete)
    
    print(f"Overall Success Rate: {successful}/{total_tests} ({100*successful/total_tests:.1f}%)")
    print()

def plot_results(results):
    """Generate visualization plots"""
    if not HAS_MATPLOTLIB:
        print("Skipping plots (matplotlib not installed)")
        return
    
    # Prepare data
    no_loss = [r for r in results if r.loss_rate == 0]
    with_loss = [r for r in results if r.loss_rate > 0]
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle('LoRa P2P Image Transfer - Performance Analysis', fontsize=16, fontweight='bold')
    
    # Plot 1: Retransmission Rate vs Loss Rate
    loss_groups = {}
    for r in results:
        if r.loss_rate not in loss_groups:
            loss_groups[r.loss_rate] = []
        loss_groups[r.loss_rate].append(r.retransmit_rate)
    
    loss_rates = sorted(loss_groups.keys())
    avg_retrans = [np.mean(loss_groups[l]) for l in loss_rates]
    
    axes[0, 0].bar([f"{l}%" for l in loss_rates], avg_retrans, color='coral')
    axes[0, 0].set_xlabel('Packet Loss Rate')
    axes[0, 0].set_ylabel('Retransmission Rate (%)')
    axes[0, 0].set_title('Impact of Packet Loss on Retransmissions')
    axes[0, 0].grid(axis='y', alpha=0.3)
    
    # Plot 2: Transfer Duration vs Image Size
    for loss in loss_rates:
        tests = [r for r in results if r.loss_rate == loss]
        sizes = [r.image_size for r in tests]
        durations = [r.duration for r in tests]
        axes[0, 1].plot(sizes, durations, marker='o', label=f'{loss}% loss')
    
    axes[0, 1].set_xlabel('Image Size (KB)')
    axes[0, 1].set_ylabel('Transfer Duration (seconds)')
    axes[0, 1].set_title('Transfer Time vs Image Size')
    axes[0, 1].legend()
    axes[0, 1].grid(True, alpha=0.3)
    
    # Plot 3: Throughput Analysis
    throughputs = []
    test_names = []
    colors = []
    
    for r in results:
        if r.duration > 0 and r.transfer_complete:
            # Throughput in bytes per second
            throughput = (r.image_size * 1024) / r.duration
            throughputs.append(throughput)
            test_names.append(r.name.replace('_', '\n'))
            colors.append('green' if r.loss_rate == 0 else 'orange')
    
    axes[1, 0].barh(test_names, throughputs, color=colors)
    axes[1, 0].set_xlabel('Throughput (bytes/second)')
    axes[1, 0].set_title('Effective Throughput by Test')
    axes[1, 0].grid(axis='x', alpha=0.3)
    
    # Plot 4: Efficiency (% of original chunks)
    efficiencies = []
    test_labels = []
    
    for r in results:
        if r.chunks_sent > 0:
            original_chunks = r.chunks_sent - r.retransmissions
            efficiency = (original_chunks / r.chunks_sent) * 100
            efficiencies.append(efficiency)
            test_labels.append(f"{r.image_size}KB\n{r.loss_rate}%")
    
    axes[1, 1].bar(range(len(efficiencies)), efficiencies, color='skyblue')
    axes[1, 1].set_xticks(range(len(test_labels)))
    axes[1, 1].set_xticklabels(test_labels, rotation=45, ha='right')
    axes[1, 1].set_ylabel('Efficiency (%)')
    axes[1, 1].set_title('Transmission Efficiency (Lower is more retransmissions)')
    axes[1, 1].axhline(y=100, color='g', linestyle='--', alpha=0.5, label='Ideal (no retrans)')
    axes[1, 1].grid(axis='y', alpha=0.3)
    axes[1, 1].legend()
    
    plt.tight_layout()
    
    output_file = 'test_results/analysis.png'
    plt.savefig(output_file, dpi=150, bbox_inches='tight')
    print(f"\n✓ Plots saved to: {output_file}")
    
    # Also try to display
    try:
        plt.show()
    except:
        pass

def main():
    results_dir = Path('test_results')
    
    if not results_dir.exists():
        print(f"Error: {results_dir} directory not found!")
        print("Run './run-tests.sh' first to generate test results.")
        sys.exit(1)
    
    # Parse all result files
    results = []
    for filepath in sorted(results_dir.glob('*.txt')):
        result = parse_results_file(filepath)
        if result:
            results.append(result)
    
    if not results:
        print("No test results found!")
        sys.exit(1)
    
    # Generate reports
    generate_text_report(results)
    plot_results(results)
    
    # Save detailed CSV
    csv_file = results_dir / 'summary.csv'
    with open(csv_file, 'w') as f:
        f.write('Test,ImageSize_KB,LossRate_%,ChunksSent,Retransmissions,RetransRate_%,Duration_s,ACKsSent,ACKsReceived,Complete\n')
        for r in results:
            f.write(f'{r.name},{r.image_size},{r.loss_rate},{r.chunks_sent},{r.retransmissions},'
                   f'{r.retransmit_rate},{r.duration},{r.acks_sent},{r.acks_received},{r.transfer_complete}\n')
    
    print(f"✓ CSV summary saved to: {csv_file}")
    print()

if __name__ == '__main__':
    main()
