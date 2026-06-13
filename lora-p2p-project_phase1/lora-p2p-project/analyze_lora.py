#!/usr/bin/env python3
"""
LoRa P2P Image Transfer Performance Analysis
Parses simulation output and generates performance statistics
"""

import subprocess
import re
import sys
from datetime import datetime

def run_simulation(size_kb, loss_pct, time_sec):
    """Run a single simulation and capture output"""
    cmd = f"./ns3 run 'lora-p2p-transfer --size={size_kb} --loss={loss_pct/100.0} --time={time_sec}'"
    
    print(f"Running: Image={size_kb}KB, Loss={loss_pct}%, Time={time_sec}s", flush=True)
    
    try:
        result = subprocess.run(cmd, shell=True, capture_output=True, text=True, timeout=time_sec+10)
        return result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return None

def parse_results(output):
    """Extract metrics from simulation output"""
    if not output:
        return None
    
    results = {}
    
    # Parse image size and chunks
    match = re.search(r'Image:\s+(\d+)\s+KB.*?Total chunks:\s+(\d+)', output, re.DOTALL)
    if match:
        results['image_kb'] = int(match.group(1))
        results['total_chunks'] = int(match.group(2))
    
    # Parse results section
    match = re.search(r'Chunks sent:\s+(\d+)', output)
    if match:
        results['chunks_sent'] = int(match.group(1))
    
    match = re.search(r'Chunks received:\s+(\d+)', output)
    if match:
        results['chunks_received'] = int(match.group(1))
    
    match = re.search(r'Retransmissions:\s+(\d+)', output)
    if match:
        results['retransmissions'] = int(match.group(1))
    
    match = re.search(r'ACKs sent:\s+(\d+)', output)
    if match:
        results['acks_sent'] = int(match.group(1))
    
    match = re.search(r'ACKs received:\s+(\d+)', output)
    if match:
        results['acks_received'] = int(match.group(1))
    
    match = re.search(r'Retransmit rate:\s+(\d+(?:\.\d+)?)%', output)
    if match:
        results['retransmit_rate'] = float(match.group(1))
    
    return results

def print_results(test_name, results):
    """Print formatted results"""
    print(f"\n{'='*60}")
    print(f"Test: {test_name}")
    print(f"{'='*60}")
    
    if not results:
        print("FAILED - No results captured")
        return
    
    print(f"Image size:        {results.get('image_kb', 'N/A')} KB")
    print(f"Total chunks:      {results.get('total_chunks', 'N/A')}")
    print(f"Chunks sent:       {results.get('chunks_sent', 'N/A')}")
    print(f"Chunks received:   {results.get('chunks_received', 'N/A')}")
    print(f"Retransmissions:   {results.get('retransmissions', 'N/A')}")
    print(f"ACKs sent:         {results.get('acks_sent', 'N/A')}")
    print(f"ACKs received:     {results.get('acks_received', 'N/A')}")
    print(f"Retransmit rate:   {results.get('retransmit_rate', 'N/A')}%")
    
    # Calculate success rate
    if 'total_chunks' in results and 'chunks_received' in results:
        if results['total_chunks'] > 0:
            success_rate = (results['chunks_received'] / results['total_chunks']) * 100
            print(f"Success rate:      {success_rate:.2f}%")

def main():
    print("\n" + "="*60)
    print("LoRa P2P Image Transfer - Performance Analysis")
    print("="*60)
    print(f"Date: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    print("="*60)
    
    # Test scenarios: (size_kb, loss_pct, time_sec, name)
    tests = [
        (50, 0, 40, "50KB - No Loss"),
        (50, 10, 50, "50KB - 10% Loss"),
        (50, 20, 60, "50KB - 20% Loss"),
        (100, 0, 60, "100KB - No Loss"),
        (100, 10, 80, "100KB - 10% Loss"),
    ]
    
    all_results = []
    
    for size, loss, time, name in tests:
        output = run_simulation(size, loss, time)
        results = parse_results(output)
        print_results(name, results)
        all_results.append((name, results))
    
    # Summary
    print("\n" + "="*60)
    print("SUMMARY")
    print("="*60)
    print(f"{'Test':<20} {'Chunks':<10} {'Sent':<8} {'Received':<10} {'Retx':<8} {'Rate':<8}")
    print("-"*60)
    
    for name, results in all_results:
        if results:
            print(f"{name:<20} {results.get('total_chunks', 'N/A'):<10} "
                  f"{results.get('chunks_sent', 'N/A'):<8} "
                  f"{results.get('chunks_received', 'N/A'):<10} "
                  f"{results.get('retransmissions', 'N/A'):<8} "
                  f"{results.get('retransmit_rate', 'N/A')}%")
        else:
            print(f"{name:<20} FAILED")
    
    print("\n" + "="*60)
    print("Analysis complete!")
    print("="*60 + "\n")

if __name__ == "__main__":
    main()
