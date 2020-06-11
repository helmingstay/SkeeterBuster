##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## Root of code definitions
## load everything: source('Sb.R', chdir=T) 
## 

## keep required package list in docker
source('required.packages.R')
## use list to load package 
lapply(required.packages, function(x) 
    library(x, character.only=T)
)

## read Pb file
proto.fn='PbInput.proto'
readProtoFiles(proto.fn)
## 
source('Proto.R')
source('Input.R')
source('Weather.R')
source('Test.R')
## complex gis, requires rgdal install?
## source('Prep.R')

## convenience function
sb_dir_temp <- function(dir_parent=NULL) {
    ## get location and store
    ## guaranteed to be unique
    if (is.null(dir_parent)) {
        ## default temp dir
        .dir <- tempfile()
    } else {
        ## use specified parent
        .dir <- tempfile(tmpdir=dir_parent)
    }
    dir.create(.dir)
    ## add trailing slash for safety.
    .dir <- paste0(.dir,'/')
    return(.dir)
}

## simple helper function
## always return a fresh message,
## clone from provided if needed
sb_msg <- function(msg=NULL){
    if (is.null(msg)) {
        .msg <- new(PbInput.PbBuild)
    } else {
        ## otherwise clone
        .msg <- msg$clone()
    }
    return(.msg)
}

## clone provided msg, 
## set new dir_out (from arg or automatically)
## this is unique for every run, stores proto file
## if custom_temp, use runtime$dir_temp as parent tempdir
## else use R default
sb_dir_out <- function(msg=NULL, dir_out=NULL, custom_temp=NULL) {
    ## if needed, import defns and make new msg 
    ## always clone / create new if null
    .msg <- sb_msg(msg)
    if(!is.null(dir_out)){
        ## set manually
        .msg$runtime$dir_out  <- dir_out
    } else {
        ## passed to sb_dir_temp
        ## null == default
        ## otherwise always create new tempdir
        ## Default: use tempdir()
        .msg$runtime$dir_out <-  sb_dir_temp(dir_parent=custom_temp)
    }
    return(.msg)
}

## set final run datetime / dir
## read csv of custom settings into msg
## write message to file, run SB
sb_run <- function( msg, 
    ## extra info here, and set in message
    verbose=FALSE,
    ## TRUE initializes gdb 
    do.debug=F,
    ## run with valgrind performance?
    do.profile=F
){
    ## set counts of houses / containers from files
    dir_in <- paste0(msg$runtime$dir_in, '/')
    ## containers
    file_cont <- paste0(dir_in, msg$runtime$file_container)
    conts <- input_read_csv(file_cont)$container_number
    ## for error-checking
    msg$sim$n_cont_row <- length(conts)
    msg$sim$n_container <- sum(conts)
    ## houses
    file_house <- paste0(dir_in, msg$runtime$file_house)
    ## check to prevent repeated houses
    houses <- input_read_csv(file_house)$location_code
    msg$sim$n_house <- length(unique(houses))
    ## sanity check
    if (msg$sim$n_house != length(houses)) {
        stop(paste0("Repeated house id in file ", file_house))
    }
    ## run in debugger? profile?
    if (do.debug && do.profile) {
        stop("Cannot do debug and profile together")
    }
    .debug.str <- ''
    if (do.debug) {
        .debug.str <-  'gdb --args'
    } else if (do.profile) {
        .debug.str <- 'valgrind --tool=callgrind'
    }
        
    ## set from arg
    msg$runtime$verbose <- verbose
    ## Add runtime info to message, cleanup
    msg$runtime$run_datetime <- format(Sys.time())
    ## prep for run
    msg$sim$start_date_julian <- as.integer(julian(as.Date(msg$sim$start_date)))
    ## write proto msg to file (in unique outdir)
    .outfile <- paste0(msg$runtime$dir_out, basename(msg$file))
    ## optional report in R
    if(verbose) {
        cat('## Runtime message file: \n')
        cat(.outfile)
        cat('\n## Final message: \n')
        cat(as.character(msg))
    }
    ## the final message
    proto_write(msg, .outfile)
    ## prep command to run SB with proto file
    .tmp <- msg$install
    .cmd <- sprintf("%s %s/%s %s", .debug.str, .tmp$bin_path, .tmp$bin_name, .outfile)
    #.cmd <- sprintf("LD_LIBRARY_PATH=%s %s %s/%s %s", .tmp$ld_path, .debug.str, .tmp$bin_path, .tmp$bin_name, .outfile)
    system(.cmd)
    ## compress outfiles
    if (msg$runtime$compress_output) {
        .out.fn <- list.files(
            path=msg$runtime$dir_out,
            pattern='/*.(txt|csv)',
            full.names=T
        )
        .cmd <-paste0('gzip ', paste(.out.fn, collapse=' '))
        system(.cmd)
    }
    return(msg)
}
