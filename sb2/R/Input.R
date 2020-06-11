##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## convenience (uniformity)
input_read_csv <- function(fn) {
    ret <- read.csv(fn, stringsAsFactors=F, comment.char='#')
    return(ret)
}

