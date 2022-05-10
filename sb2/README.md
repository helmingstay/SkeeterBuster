# SkeeterBuster 2 (SB2) #
* Pleasei contact code@x14n.org ATTN SkeeterBuster for more information.

# Requirements
A POSIX compliant OS and a GNU build environment is strongly recommended (e.g. OSX + XCODE or Linux). On Windows, use a Virtual Machine such as VirtualBox to run Linux.


## Installation Details
The following software is required ( ```commands``` shown for Debian/ubuntu ).

* Standard build chain:
```sudo apt install build-essential```

* Google Protocol Buffers v2: https://developers.google.com/protocol-buffers/
* GNU Scientific Library
* sqlite3: http://www.sqlite.org/download.html
* R (and various packages, see R/install.packages.R)
```sudo apt install time libgsl-dev libsqlite3-dev libprotobuf-dev protobuf-compiler r-base r-base-dev```

* To view the above instructions, run:
--```make install```

# Running Skeeterbuster

* For SkeeterBuster C++ source and executables see Makefile in ./src/: 
-- ```make``` and ```make test```
* Examples & regression tests in ./test/*
* See functions in ./R, especially ./R/Sb.R
* Runtime parameter definitions & defaults live in ./R/PbInput.proto
-- Do not modify this file to change runtime behavior
-- A config.csv file is typically provided to over-ride default values (see examples in ./test)
* The most realistic example is provided in ./test/rangefind (drawn from Iquitos)
-- In R, run ```source('./R/example_run.R', chdir=T)```
* By default, weather is provided for Iquitos, Peru (see `./test/base/IquitosWeatherAll.csv.gz`)
-- Users can provide an alterate weather file in GSOD format (for details, see  https://cran.r-project.org/web/packages/GSODR/vignettes/GSODR.html)
