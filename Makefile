.PHONY: test clean-test

test:
	bash tests/run_tests.sh

clean-test:
	rm -rf tests/build tests/tmp
