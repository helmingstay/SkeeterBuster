## Regression tests

* Run with "make test"

* Actual tests in setup.X/days.Y for various X,Y
-- When possible, input files link to base, below
-- config.csv controls test behavior
-- File existence (e.g. "000TESTME") controls test running (given by PbInstall field "test_file")
-- Output / test results in out.expected
* Dir "base/" contains defaults
-- House, container config (named by X, below)
-- Release 
-- Weather
