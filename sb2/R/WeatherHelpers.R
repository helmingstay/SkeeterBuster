##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## convenience functions
## convert between WMO input (empirial) and metric
## Round to specified precision (prevent false precision)

conv.F_C <- function(x, digits=1) {
    ret <- ((x - 32)/9)*5
    ret <- round(ret, digits)
    return(ret)
}

conv.In_Mm <- function(x, digits=1) {
    ret <- 25.4 * x
    ret <- round(ret, digits)
    return(ret)
}

conv._relhum <- function(tempC, dewC, digits=1) {
    ## get relative humidity from tempC and dewC, 
    ## http://andrew.rsmas.miami.edu/bmcnoldy/Humidity.html
    ## and http://en.wikipedia.org/wiki/Clausius%E2%80%93Clapeyron_relation#Meteorology_and_climatology
    RH <- 100*(exp((17.625*dewC)/(243.04+dewC))/exp((17.625*tempC)/(243.04+tempC)))
    RH <- round(RH, digits)
    return(RH)
}

conv._satdef <- function(tempC, RH, digits=3) {
    ## return saturation deficit in mBar for sb
    # http://cronklab.wikidot.com/calculation-of-vapour-pressure-deficit
    svp_mbar <- (610.7*10**((7.5*tempC)/(237.3 + tempC)))/100
    SatDef <- (1-RH/100)*svp_mbar
    SatDef <- round(SatDef, digits)
    return(SatDef)
}


