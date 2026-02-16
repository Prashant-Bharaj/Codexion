#!/bin/bash
# Test case that shows EDF vs FIFO difference
# More coders than dongles, tight burnout window, high contention

echo "=== FIFO Test ==="
echo "All coders treated equally, strict order"
./codexion 5 600 200 150 100 3 80 fifo
echo ""

echo "=== EDF Test ==="
echo "Coders close to burnout get priority"
./codexion 5 600 200 150 100 3 80 edf
echo ""

echo "=== Analysis ==="
echo "FIFO: Watch if a coder waits too long and burns out"
echo "EDF: Urgent coders should jump the queue and avoid burnout"
