#!/bin/bash
# Test: with EDF and same deadlines, coder 4 should appear first
cd "$(dirname "$0")"
echo "First 8 log lines (expect coder 4 'has taken' or 'is compiling' first):"
timeout 3 ./codexion 4 2000 200 100 60 2 0 edf 2>&1 | head -8
echo ""
echo "First coder_id in output:"
timeout 3 ./codexion 4 2000 200 100 60 2 0 edf 2>&1 | head -1 | awk '{print "coder_id="$2}'
