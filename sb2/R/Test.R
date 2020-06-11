##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

source('Proto.R')
## get list of directories containing tests
## by default, take test locations from proto msg
test_getdirs <- function(msg, test_path, 
    ## by default, use values in msg
    test_file=NULL 
){ 
    if (is.null(test_file)){
        test_file <- msg$install$test_file 
    }
    .dirs <- list.files(path=test_path, pattern=test_file, recursive=T)
    .dirs <- paste0(test_path, '/', dirname(.dirs))
    return(.dirs)
}

## helper function, compare output between expected and sim
test_output <- function(
    msg, path.expected='out.expected'
) {
    ## convenience
    .scan <- function(x) scan(x, what='character', sep='\n', quiet=TRUE)
    ## sim results
    out.path <- msg$runtime$dir_out
    ## unit test expected
    in.path <- paste0(msg$runtime$dir_in, '/', path.expected, '/')
    ## basename of expected file output
    expected <- basename(list.files(path=in.path))
    diffs <- lapply(expected, function(.file) {
        files <- paste0(c(in.path, out.path), .file)
        ## read results by line into text vectors 
        in.res <- .scan(files[1])
        out.res <- 0
        ## if output file doesn't exist...
        tryCatch(
            out.res <- .scan(files[2]), 
            error = function(err) {warning('Expected test result file not found')}
        )
        if (length(in.res) != length(out.res) ) {
            return(-1)
        }
        ## compare, return differences
        diff <- in.res == out.res
        return(which(!diff))
    })
    ## clean up return
    names(diffs) <- expected
    ## add in totals as first element
    diffs <- c(list(diff.total=length(unlist(diffs))), diffs)
    return(diffs)
}


## run a single test, return list of differences + msg
## extra args passed to sb_run
test_run <- function(
    dir, msg=NULL, do_weather=F, proto_dir=NULL, ...
) {
    ## reset temp dir, store in new message
    imsg <- sb_dir_out(msg=msg)
    ## read csv config
    imsg <- proto_read(imsg, dir_in=dir)
    if (do_weather){
        weather_write_all(dir)
    }
    imsg <- sb_run( imsg, ...)
    diff <- test_output(imsg)
    inret <- list(diff=diff, msg=imsg)
    return(inret)
}

## get test locations, 
## run each test, collect messages
test_run_all <- function(
    test_path, 
    msg=NULL, dirs=NULL, verbose=F,
    ## pass through to test_getdirs 
    test_file=NULL,
    parallel=F
) {
    if (is.null(msg)) {
        msg <- sb_msg()
    }
    ## parallel lapply?
    if(parallel) {
        lfun <- mclapply
    } else {
        lfun <- lapply
    }
    ## fetch dirs unless a particular one is provided
    if (is.null(dirs)) {
        dirs <- test_getdirs(msg, test_path, test_file) 
    }
    ## run test for each dir
    ret <- lfun(dirs, function(.dir) {
        if (verbose) cat(paste0("\n## Running test: ", .dir, '\n'))
        imsg <- msg$clone()
        test_run(msg=msg, dir=.dir)
    })
    names(ret) <- dirs
    ## pull out each test's diffs into vector
    all.diff <- laply(ret, function(x){
        x$diff$diff.total
    })
    ## add sum of all failed cases
    fail.which <- which(all.diff>0)
    ret.summary = list(
        fail.which=fail.which, 
        success=length(fail.which)==0
    )
    ## add to end so above index is correct
    ret <- c(ret, ret.summary)
}

test_diff <- function(ret, which=1, cmd='vimdiff') {
    .diff <- ret$diff
    .rt <- ret$msg$runtime
    bad <- sapply(.diff, function(x) length(x) > 1)
    bad.names <- names(.diff)[bad]
    if (length(bad.names) == 0 ) {
        return()
    }
    .name <- bad.names[which]
    .fn.expect = paste(.rt$dir_in, 'out.expected', .name, sep='/')
    .fn.actual <- paste0(.rt$dir_out, .name)
    system(sprintf('%s %s %s', cmd, .fn.expect, .fn.actual))
    return()
}


