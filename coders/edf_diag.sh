#!/bin/bash
# EDF diagnostic: find burnout boundary for cooldown 60
cd "$(dirname "$0")"
echo "EDF with cooldown=60, 4 coders, 200 compile 100 debug 60 refactor, 3 compiles"
for b in 1000 1200 1500 2000 2500 3000; do
	echo -n "burnout=$b: "
	timeout 20 ./codexion 4 $b 200 100 60 3 60 edf > /tmp/edf_o 2>&1
	ex=$?
	if [ $ex -eq 0 ] && ! grep -q "burned out" /tmp/edf_o; then
		echo "OK"
	else
		echo "FAIL exit=$ex"
	fi
done
echo ""
echo "FIFO with cooldown=60 (same params)"
for b in 1000 1200 1500 2000 2500 3000; do
	echo -n "burnout=$b: "
	timeout 20 ./codexion 4 $b 200 100 60 3 60 fifo > /tmp/edf_o 2>&1
	ex=$?
	if [ $ex -eq 0 ] && ! grep -q "burned out" /tmp/edf_o; then
		echo "OK"
	else
		echo "FAIL exit=$ex"
	fi
done
echo ""
echo "EDF with cooldown=0 (find boundary)"
for b in 600 700 800 900 1000 1200; do
	echo -n "burnout=$b: "
	timeout 20 ./codexion 4 $b 200 100 60 3 0 edf > /tmp/edf_o 2>&1
	ex=$?
	if [ $ex -eq 0 ] && ! grep -q "burned out" /tmp/edf_o; then
		echo "OK"
	else
		echo "FAIL exit=$ex"
	fi
done
