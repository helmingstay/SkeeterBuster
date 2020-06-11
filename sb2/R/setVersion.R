##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

## called by make
## Fill version info from git at compile time
source('Sb.R')
## read in proto definitions
not.used <- sb_msg()
## new message
pb_ver <- new(PbInput.PbVersion)
## get branch
pb_ver$branch <- system("git rev-parse --abbrev-ref HEAD", intern=T)
## commit hash
pb_ver$commit <- system("git log -1 --pretty=%h", intern=T)
## build time
pb_ver$build_datetime = format(Sys.time())
## write results to pre-specified file
con <- file( pb_ver$version_file, open = "wb")
pb_ver$serialize( con )
close(con)
## Final Message
not.used <- proto_cat(pb_ver)
