##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## called by Makefile, "make test"
source('Sb.R');

test_path <- '../test'

## read-in weather, 
## write csv files in indirs
weather_write_all(
    test_getdirs(sb_msg(), test_path)
)
## run tests in parallel
options(mc.cores=4)
results <- test_run_all(
    test_path,
    parallel=F, 
    ## restrict tests
    # test_path='../test/setup.minimal',
    verbose=T
)

if( results$success) { 
    cat('\nAll tests pass\n') 
} else { 
    cat("\n\n## Test failure: details\n")
    for( res in results[results$fail.which]) {
        print(res$diff)
        proto_cat(res$msg)
    }
    ##
    cat("\n\n## Test failure: overview\n")
    .which <- results$fail.which
    cat(sprintf('Failure in Test %d: %s\n\n', .which, names(results)[.which]));
}
