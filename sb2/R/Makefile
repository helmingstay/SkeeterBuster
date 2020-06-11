version:
	R -q --vanilla -f setVersion.R 2> /dev/null
test: 
	time R --slave --vanilla -f run.tests.R #2> /dev/null


PACKAGES_INSTALLED:
	R --vanilla < install.packages.R  
	## "Note: ignore warning 'package parallel not available'"
	touch PACKAGES_INSTALLED
