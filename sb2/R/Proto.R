##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## prefer to source with chdir=T
source('Input.R')

## Initialize, return nothing
proto_init <- function(
    fn='PbInput.proto', dir=NULL
) {
    if (!is.null(dir)) {
        fn <- paste(dir, fn, sep='/')
    }
    readProtoFiles(fn)
    invisible()
}
## initialize
## depends on chdir=T at top of call chain?
proto_init()

## take message and add values based on csv file
proto_read <- function(
    msg, verbose=F, dir_in=NULL, file_config=NULL
){
    if (is.null(dir_in) == is.null(file_config)) {
        stop('Must provide either dir_in or file_config')
    }
    if (!is.null(dir_in)) {
        msg$runtime$dir_in <- dir_in 
        msg$runtime$file_config <- paste0(msg$runtime$dir_in, '/',  basename(msg$runtime$file_config))
    } 
    ## if not given, use dir+file in msg 
    if (!is.null(file_config)) {
        ## manually specify
        msg$runtime$file_config <- file_config
        msg$runtime$dir_in <- dirname(file_config)
    } 
    ## fill PbBuild message fields from (default) config.csv.  
    
    ## Called just before writeProto, model run, after CLI / test settings are applied"""
    .config <- msg$runtime$file_config 
    csv.dat <- input_read_csv(.config)
    ## sanity check
    .expected <- c('message','field','value')
    .test <- identical( colnames(csv.dat), .expected)
    if (!.test) {
        paste0('Incorrect header format\nFirst line of csv should be:\n %s', paste(.expected, collapse=','))
    }
    ## for each row of file, try to fill with data
    for ( irow in 1:nrow(csv.dat)) {
        ## cludge for empty file / all comments
        if(nrow(csv.dat)==0) break
        dat <- csv.dat[irow,]
        tryCatch(
            ## try to fill field w/supplied value
            ## parse text of value
            msg[[dat$message]][[dat$field]] <- eval(parse(text=dat$value)), 
            ## if problem, print row
            error = function(e) {
                cat(paste0('Problem with row:\n', paste(dat, collapse=',')))
                print(e)
                return(e)
            }
        )
    }
    if(verbose) {
        cat('## Final message:\n')
        cat(as.character(msg))
    }
    return(msg)
}

## helper, don't call directly
## write msg to specified file
proto_write <- function(msg, outfile) {
    ## set file location in msg
    msg$file <- outfile
    ## then write
    con <- file( outfile, open = "wb")
    msg$serialize( con )
    close(con) 
}

## regenerate msg by reading binary file 
## ie result of proto_write
proto_from_bin <- function(fn) {
    ret <- PbInput.PbBuild$read(fn)
    ret
}

## convenience function, "pretty print" 
proto_cat <- function(msg) cat(as.character(msg))
