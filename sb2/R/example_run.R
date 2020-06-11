##    Copyright 2015-2020 Christian Gunning (code@x14n.org)
##    This file is part of Skeeter Buster.

source('Sb.R')
## test_run() in Test.R contains full example:
## calls sb_dir_out(), proto_read(), and sb_run()
## do_weather=T only needed on first run / once per dir
a.test <- test_run(
    ## long...
    dir='../test/rangefind/days.480',
    verbose=T, do_weather=T, do.debug=F
) 
## return value of test_run() is a list:
## test differences and protobuf msg
summary(a.test)
## diff names show expected output files
summary(a.test$diff)
## sb_run() is function that *actually* runs sim
## return value of sb_run() is modified protobuf msg
## shows non-default param values (including paths)
## all other values are defaults
proto_cat(a.test$msg)
