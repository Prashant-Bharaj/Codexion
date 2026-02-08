#!/bin/bash
# Codexion tester - adapted from philosopher-style testers
# Tests: compilation, invalid args (incl. <2 coders), burnout, success, log format
# Rule: compiling quantum code requires two dongles (left + right) — at least 2 coders required

MAGENTA='\033[0;35m'
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'
BOLD='\033[1m'

total_tests=0
successful_tests=0
CODEXION_DIR="$(cd "$(dirname "$0")" && pwd)"
OUT="$CODEXION_DIR/out"

handle_ctrl_c() {
	echo -e "\nStopped tester"
	rm -f "$OUT"
	exit 1
}
trap handle_ctrl_c INT

check_compilation() {
	echo -e "\n${MAGENTA}=== Compilation ===${NC}"
	cd "$CODEXION_DIR" && make re > "$OUT" 2>&1
	if [ $? -ne 0 ]; then
		echo -e "Compilation: ${RED}KO${NC}"
		cat "$OUT"
		rm -f "$OUT"
		exit 1
	fi
	echo -e "Compilation: ${GREEN}OK${NC}"
	rm -f "$OUT"
}

invalid_arg_test() {
	((total_tests++))
	cd "$CODEXION_DIR"
	./codexion "$@" 2> "$OUT"
	exit_code=$?
	if [ $exit_code -ne 0 ] || grep -qi "usage\|error\|invalid" "$OUT" 2>/dev/null; then
		echo -e "[ TEST $total_tests ] Invalid args ($*): ${GREEN}OK${NC} (rejected)"
		((successful_tests++))
	else
		echo -e "[ TEST $total_tests ] Invalid args ($*): ${RED}KO${NC} (should reject)"
	fi
	rm -f "$OUT"
}

burnout_test() {
	((total_tests++))
	timeout_sec=$1
	shift
	cd "$CODEXION_DIR"
	timeout $timeout_sec ./codexion "$@" > "$OUT" 2>&1
	exit_code=$?
	if [ $exit_code -eq 124 ]; then
		echo -e "[ TEST $total_tests ] Burnout ($*): ${RED}KO${NC} (timeout - no burnout)"
	elif grep -q "burned out" "$OUT" && [ $exit_code -ne 0 ]; then
		burnout_line=$(grep "burned out" "$OUT")
		echo -e "[ TEST $total_tests ] Burnout ($*): ${GREEN}OK${NC} ($burnout_line)"
		((successful_tests++))
	else
		echo -e "[ TEST $total_tests ] Burnout ($*): ${RED}KO${NC} (exit=$exit_code, expected burnout)"
	fi
	rm -f "$OUT"
}

success_test() {
	((total_tests++))
	timeout_sec=$1
	shift
	cd "$CODEXION_DIR"
	timeout $timeout_sec ./codexion "$@" > "$OUT" 2>&1
	exit_code=$?
	if [ $exit_code -eq 124 ]; then
		echo -e "[ TEST $total_tests ] Success ($*): ${RED}KO${NC} (timeout)"
	elif [ $exit_code -eq 0 ] && ! grep -q "burned out" "$OUT"; then
		echo -e "[ TEST $total_tests ] Success ($*): ${GREEN}OK${NC}"
		((successful_tests++))
	else
		echo -e "[ TEST $total_tests ] Success ($*): ${RED}KO${NC} (exit=$exit_code)"
	fi
	rm -f "$OUT"
}

log_format_test() {
	((total_tests++))
	cd "$CODEXION_DIR"
	timeout 8 ./codexion 2 3000 100 50 50 2 0 fifo > "$OUT" 2>&1
	if [ $? -ne 0 ]; then
		echo -e "[ TEST $total_tests ] Log format: ${RED}KO${NC} (program failed)"
		rm -f "$OUT"
		return
	fi
	ok=1
	while IFS= read -r line; do
		if [[ "$line" =~ ^[0-9]+[[:space:]]+[0-9]+[[:space:]]+(has taken a dongle|is compiling|is debugging|is refactoring)$ ]]; then
			:
		else
			ok=0
			break
		fi
	done < "$OUT"
	if [ $ok -eq 1 ] && [ -s "$OUT" ]; then
		echo -e "[ TEST $total_tests ] Log format: ${GREEN}OK${NC}"
		((successful_tests++))
	else
		echo -e "[ TEST $total_tests ] Log format: ${RED}KO${NC}"
	fi
	rm -f "$OUT"
}

valgrind_test() {
	if ! command -v valgrind &> /dev/null; then
		echo -e "[ VALGRIND ] ${YELLOW}skipped${NC} (valgrind not found)"
		return
	fi
	((total_tests++))
	cd "$CODEXION_DIR"
	timeout 15 valgrind --leak-check=full --show-leak-kinds=definite ./codexion 2 3000 100 50 50 2 0 fifo > "$OUT" 2> "${OUT}.vg"
	exit_code=$?
	if grep -qE "definitely lost: 0 bytes|no leaks are possible" "${OUT}.vg" 2>/dev/null; then
		echo -e "[ TEST $total_tests ] Valgrind: ${GREEN}OK${NC}"
		((successful_tests++))
	else
		echo -e "[ TEST $total_tests ] Valgrind: ${RED}KO${NC}"
		grep -E "definitely lost|ERROR SUMMARY" "${OUT}.vg" 2>/dev/null | head -5
	fi
	rm -f "$OUT" "${OUT}.vg"
}

print_summary() {
	echo -e "\n${MAGENTA}========== Summary ==========${NC}"
	echo -e "Tests: $successful_tests / $total_tests"
	if [ $successful_tests -eq $total_tests ]; then
		echo -e "${GREEN}All tests passed${NC}"
	else
		echo -e "${RED}Some tests failed${NC}"
	fi
}

# Main
echo -e "${BOLD}Codexion Tester${NC}"
check_compilation

echo -e "\n${MAGENTA}=== Invalid arguments ===${NC}"
invalid_arg_test ""
invalid_arg_test 1 2 3 4 5
invalid_arg_test -1 1000 200 200 100 5 0 fifo
invalid_arg_test 2 1000 200 200 100 -1 0 fifo
invalid_arg_test 2 1000 200 200 100 5 0 invalid
invalid_arg_test 2 1000 200 200 100 5 0 EDF
invalid_arg_test 0 1000 200 200 100 5 0 fifo
# Compiling quantum code requires two dongles (one left, one right) — need at least 2 coders
invalid_arg_test 1 5000 100 50 50 3 0 fifo
invalid_arg_test 2 abc 200 200 100 5 0 fifo

echo -e "\n${MAGENTA}=== Success tests ===${NC}"
success_test 15 2 3000 100 50 50 3 0 fifo
success_test 25 4 3000 200 200 1 3 2 fifo
success_test 15 2 3000 100 50 50 2 0 edf

echo -e "\n${MAGENTA}=== Burnout test ===${NC}"
# 4 coders, 200ms burnout: coders 2&4 can't get dongles until 1&3 release ~201ms
# So 2 and 4 burn out (deadline 200ms)
burnout_test 5 4 200 200 200 1 5 0 fifo

echo -e "\n${MAGENTA}=== Log format ===${NC}"
log_format_test

echo -e "\n${MAGENTA}=== Valgrind (optional) ===${NC}"
valgrind_test

print_summary
