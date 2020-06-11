##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## load convenience functions
source('WeatherHelpers.R')
source('Input.R')
source('Proto.R')

## turn crazy crazy FWF WMO file into csv
## take protobuf msg
## Read in specified WMO weather file
## deal with NAs, extra characters, whitespace
## write to temp csv file, return new filename
weather_clean <- function(msg) {
    ##
    ## first prep filename
    weather.fn <- paste(msg$install$bin_path, msg$install$file_weather_all, sep='/')
    ## temporary filename 
    tmp.fn <- paste0(tempfile(), '_weather.cvs')
    ## read text by lines, sub NAs 
    ## and strip extra characters
    ## need to skip first row
    patts <- list(
        c(msg$install$weather_regex_strip, ''),
        c(msg$install$weather_regex_na, 'NA')
    )
    ## read file by line as text, cleanup as 
    tmp.weather <- readLines(weather.fn)
    ## Strip whitespace
    tmp.weather <- gsub( " ", "", tmp.weather)
    ## break off header, don't sub it
    header <- tmp.weather[1]
    tmp.weather <- tmp.weather[-1]
    ##
    ## Strip extra chars, process NAs
    for (patt in patts) {
        tmp.weather <- gsub(patt[1], patt[2], tmp.weather)
    }
    ## add back header
    tmp.weather <- c(header, tmp.weather)
    writeLines(tmp.weather, tmp.fn)
    return(tmp.fn)
}

## take file from weather_clean
## get final cols, load into db w/sqldf
## add index
weather_fill <- function(msg, conn, do.locf=TRUE) {
    ## cleanup weather as text, write to tempfile
    clean.fn <- weather_clean(msg)
    ## insert clean weather into database
    weather <- input_read_csv(clean.fn)
    ## 
    ## Clean data frame using exising weather
    weather <- with(weather, within(list(), {
        #rowid <- 0:(length(TEMP)-1)
        station <- STN...
        ## turn to character later
        date <- as.Date(as.character(YEARMODA), format='%Y%m%d')
        julianday <- julian(date)
        meanTempC <- conv.F_C(TEMP)
        minTempC <- conv.F_C(MIN)
        maxTempC <- conv.F_C(MAX)
        dewC <- conv.F_C(DEWP)
        ## fill in missing dewpoints
        dewC[is.na(dewC)] <- meanTempC[is.na(dewC)]
        ##
        relHum <- conv._relhum(meanTempC, dewC)
        satDef <- conv._satdef(meanTempC, relHum)
        precipMm <- conv.In_Mm(PRCP)
        precipMm[is.na(precipMm)] <- 0
        meanWindKn <- WDSP
        meanWindKn[is.na(meanWindKn)] <- 0
        ## is the 366th day of year
        ## how different from leap years?
        #disable_366_days <- as.numeric(strftime(date, '%j')) == 366
        ## is a leap-day
        disable_leap_years <- strftime(date, '%m-%d') != '02-29'
        ## use col to subset 
        ## >= 0 (enable ly) or >= 1 (disable ly)
        disable_leap_years <- as.numeric(disable_leap_years)
        date <- as.character(date)
    }))
    weather <- as.data.table(weather)
    setkey(weather, 'julianday')
    if (do.locf) {
        ## fix gaps using LOCF
        ## join data.table, all days
        .range = range(weather$julianday)
        .join = data.table(
            day=.range[1]:.range[2], key='day'
        )
        ## locf to fill gaps in weather
        weather <- weather[.join, roll=TRUE]
        ## back to data.frame
    }
    weather <- setcolorder(weather, rev(colnames(weather)))
    .tab.name <- msg$install$weather_table
    ## write table 
    dbWriteTable(conn, .tab.name, weather, append=TRUE)
    ## index on date
    .idx <- sprintf('CREATE INDEX idx_weather_date ON %s(date)', .tab.name)
    ## called for effects, res unused
    res <- dbSendQuery(conn, .idx)
    ## cleanup (prevent open connection message)
    dbClearResult(res)
}

## initialize sqlite file-backed db
## return connection
## schema file (or link to) must be in current dir
weather_init_db <- function(msg){
    ## temporary sqlite3 file, shared via connection
    the_db <- paste0(sb_dir_temp(), 'weather.dat')
    ## store name (recall that message is reference)
    msg$install$weather_db <- the_db
    ## use native sqlite to read in schema
    .cmd <- sprintf(
        "sqlite3 -init %s '%s' .quit 2>&1", 
        msg$install$file_schema, the_db
    )
    ## capture output, here if needed.
    .sql.msg <- system(.cmd, intern=T)
    ## basic error checking of schema read-in
    if (length(.sql.msg) > 2) {
        cat("\n") 
        cat(paste(.sql.msg, collapse='\n'))
        cat("\n") 
        stop("\nProblem initializing database")
    }
    ## in R, open connection to db 
    conn <- dbConnect(RSQLite::SQLite(), the_db)
    return(conn)
}

## write csv for this run
weather_write_csv <- function(msg, conn, .dir) {
    outfile <- paste0(.dir, '/', msg$runtime$file_weather)
    qry <- sprintf("
        select * from %s 
        where date >= '%s'
        and disable_leap_years >= %d
        order by julianday limit %d 
        ", 
        msg$install$weather_table,
        msg$sim$start_date, 
        ## if false, get all rows
        ## otherwise, omit 0-rows (leap-days)
        as.integer(msg$sim$disable_leap_years),
        msg$sim$number_of_days
    )
    ret <- sqldf(qry, connection=conn)
    ## cleanup
    if (msg$sim$disable_humid) {
        ret$relHum = 100.0;
        ret$satDef = 0.0;
    }
    if (nrow(ret) < msg$sim$number_of_days){
        stop("Error in Weather.R: not enough rows of weather")
    }
    ## just grab needed cols
    ## (plus possibly-useful)
    ret <- subset(ret, select=c(
        station, julianday, meanTempC, 
        minTempC, maxTempC, relHum, satDef, 
        precipMm, meanWindKn
    ))
    write.csv(ret, file=outfile, quote=F, row.names=F)
}

## write weather csv for each config file in dirs
weather_write_all <- function(dirs, msg=NULL) {
    msg <- sb_msg(msg)
    ## generate file-backed sqlite connection (read schema)
    conn <- weather_init_db(msg)
    ## clean and fill db
    weather_fill(msg, conn)
    for (.dir in dirs){
        imsg <- sb_msg(msg)
        ## get runtime config
        imsg <- proto_read(imsg, dir_in=.dir)
        ## finale weather csv based on config
        weather_write_csv(imsg, conn, .dir)
    }
}
