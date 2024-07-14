define HELP_TEXT
echo ""
echo "HLS_TEST_SUITE Make Targets"
echo "--------------------------------------------------------------------------------"
echo "run                    Compiles and runs all tests"
echo "run-base               Compiles and runs basic tests"
echo "run-dynamatic          Compiles and runs all test from the dynamitcs git repo"
echo "run-polybench          Compiles and runs supported polybench benchmarks"
echo "clean                  Deletes build directory [$(HLS_TEST_BUILD)]"
echo "help                   Prints this help text"
endef

.PHONY: help
help:
	@$(HELP_TEXT)

JHLS = jhls
HLS_TEST_ROOT ?= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

include Makefile.sub

.PHONY: run
run: hls-test-run

.PHONY: run-base
run-base: hls-test-base

.PHONY: run-dynamaitc
run-dynamiatic: hls-test-dynamatic

.PHONY: run-
run-polybench: hls-test-polybench

.PHONY: clean
clean: hls-test-clean
