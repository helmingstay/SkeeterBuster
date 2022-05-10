##    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
##    This file is part of Skeeter Buster.

## add custom library directory here:
## possibly machine-specific
CUSTOM_LIB_DIRS = -L/usr/local/lib -L$(LD_LIBRARY_PATH)
CUSTOM_INCLUDE_DIRS = -I/usr/include
## for user-installed libs try:
#CUSTOM_LIB_DIRS =-L/usr/local/lib -L$(HOME)/local/lib 
#CUSTOM_INCLUDE_DIRS = -I/usr/local/include -I$(HOME)/local/include
# LIBS = -lprotobuf -lpthread -pthread -march=native
## See "man pkg-config"
LIBS = -lprotobuf -lpthread -pthread -lgsl -lgslcblas -lm 
LDFLAGS = $(CUSTOM_LIB_DIRS) $(CUSTOM_INCLUDE_DIRS) $(LIBS) 
#NOT_OSX_LDFLAGS = -Wl,-rpath=$(CUSTOM_LIB_DIRS) 
PREP_DIR = ./R/
BIN_NAME = SkeeterBuster

## Core programs
CC = g++ $(CUSTOM_INCLUDE_DIRS) -std=c++11 
## osx uses clang now
#CC = clang++ $(CUSTOM_INCLUDE_DIRS) -std=c++11 

ifeq ($(DEBUG), 1)
    CFLAGS= -Wall -ggdb
else
	ifeq ($(PERFORM), 1)
		CFLAGS= -O2 -Wall -Wno-format 
	else
		CFLAGS = -O1 -Wall -ggdb 
	endif
endif

## supercedes DEBUG

## uncomment one CFLAGS
# ## CFLAGS_DEBUG  = -Wall -ggdb ## none
# ## CFLAGS  = -O1 -Wall -ggdb ## normal
# ## CFLAGS  = -O1 -Wall -ggdb -Wno-format ## quiet
#CFLAGS = -O3 -Wall  -ggdb ## performance, debugging
#CFLAGS = -O3 -Wall  ## performance

OBJECTS := ErrorMessage.o PbInput.pb.o Aedes.o Output.o \
	Binomial.o Building.o EggsCohort.o Female_Adult.o \
	Fitness.o Larvae_Cohort.o Male_Adult_Cohort.o Pupae_Cohort.o \
	Receptacle.o Input.o 
	

$(BIN_NAME) : $(OBJECTS) 
    ## embed version, derived from git tag
	## build the executable
	$(CC) $(OBJECTS) $(LDFLAGS) $(CFLAGS) -o $(BIN_NAME)  

PbInput.pb.h : $(PREP_DIR)/PbInput.proto 
	## rebuild PbInput.pb.cc, PbInput.h,  and PbInput.py
	## assume that pretty much everyone else needs these
	protoc -I=$(PREP_DIR) --cpp_out=. $(PREP_DIR)/PbInput.proto
	## If you need python bindings:
	## protoc -I=proto --python_out=$(PREP_DIR) $(PREP_DIR)/PbInput.proto

PbInput.pb.o :  PbInput.pb.h
	$(CC) $(CFLAGS) -c PbInput.pb.cc

ErrorMessage.o : ErrorMessage.cpp    PbInput.pb.h 
	$(CC)  $(CFLAGS) -c  ErrorMessage.cpp 

Aedes.o : Aedes.cpp Globals.h PbRead.h Output.o Input.o Building.o \
	Binomial.o PbInput.pb.h OutputFuns.h \
	helpers.h fastcsv_type.h Weather.h Sampled.h 
	$(CC)  $(CFLAGS) -c  Aedes.cpp

Input.o : Input.cpp Input.h Output.cpp Output.h EggsCohort.o Larvae_Cohort.o Pupae_Cohort.o Link.h Building.h Globals.h Release.h Block.h
	$(CC) $(CFLAGS) -c  Input.cpp 

Output.o : Output.cpp Output.h Input.cpp Input.h  PbInput.pb.h  Globals.h 
	$(CC) $(CFLAGS) -c  Output.cpp 

Binomial.o : Binomial.cpp Binomial.h   PbInput.pb.h 
	$(CC) $(CFLAGS) -c  Binomial.cpp 

Building.o : Building.cpp  Building.h Link.h Link_type.h Receptacle.h Receptacle.cpp Input.o Output.o  PbInput.pb.h Male_Adult_Cohort.h Female_Adult.h helpers.h DevelKinetics.h Weather.h Globals.h Binomial.o
	$(CC) $(CFLAGS) -c  Building.cpp

EggsCohort.o : EggsCohort.cpp YoungCohorts.h Cohort.h  PbInput.pb.h DevelKinetics.h Weather.h Globals.h 
	$(CC) $(CFLAGS) -c  EggsCohort.cpp 

Larvae_Cohort.o : Larvae_Cohort.cpp YoungCohorts.h LarvalParamStructs.h Cohort.h  PbInput.pb.h  Globals.h 
	$(CC) $(CFLAGS) -c Larvae_Cohort.cpp

Pupae_Cohort.o : Pupae_Cohort.cpp YoungCohorts.h Cohort.h  PbInput.pb.h DevelKinetics.h Globals.h 
	$(CC) $(CFLAGS) -c  Pupae_Cohort.cpp 

Male_Adult_Cohort.o : Male_Adult_Cohort.cpp Male_Adult_Cohort.h Cohort.h PbInput.pb.h DevelKinetics.h Weather.h Globals.h 
	$(CC) $(CFLAGS) -c  Male_Adult_Cohort.cpp 

Female_Adult.o : Female_Adult.cpp Female_Adult.h Cohort.h  PbInput.pb.h DevelKinetics.h Weather.h Globals.h 
	$(CC) $(CFLAGS) -c  Female_Adult.cpp 

Fitness.o : Fitness.cpp  PbInput.pb.h FitnessDefines.h Globals.h 
	$(CC) $(CFLAGS) -c  Fitness.cpp 

Receptacle.o : Receptacle.cpp Receptacle.h YoungCohorts.h PbInput.pb.h Input.cpp helpers.h Weather.h Globals.h Male_Adult_Cohort.h 
	$(CC) $(CFLAGS) -c  Receptacle.cpp 

.PHONY : clean clean.obj clean.runtime clean.weather dev print.vars test test.run all

all : clean $(BIN_NAME) 

## convenience labels 
clean : clean.obj clean.runtime clean.weather

clean.runtime:
	## clean up runtime objects (-, no error if not)
	-rm $(BIN_NAME) 

clean.obj:
	## clean up intermediate files from build (-, no error if not)
	## including auto-generated pb files
	-rm *.o *.pb.cc *.pb.h 

clean.weather:
	-rm test/*/days.*/weather.csv

dev:
	@## http://cscope.sourceforge.net/cscope_vim_tutorial.html
	cscope -b -R -k
	@## see http://tbaggery.com/2011/08/08/effortless-ctags-with-git.html
	@## and http://andrew.stwrt.ca/posts/vim-ctags
	ctags -R -f tags --exclude=.git --languages=C++ .
	@#ctags --tag-relative -f tags --exclude=.git --languages=C++ .

print.vars :
	### Makefile Vars 
	###### names and paths
	## PREP_DIR: $(PREP_DIR)
	## BIN_NAME: $(BIN_NAME)
	###### compilation
	## CC: $(CC), 
	## CFLAGS: $(CFLAGS)
	## LDFLAGS (all of the below): $(LDFLAGS)
	## LIBS: $(LIBS) 
	## CUSTOM_LIB_DIRS: $(CUSTOM_LIB_DIRS)
	## CUSTOM_INCLUDE_DIRS: $(CUSTOM_INCLUDE_DIRS)

##############################
## installation
##############################
install : 
	##############################
	## Installation instructions
	##############################
	## On debian/ubuntu, run the following (i.e. copy/paste):
	## sudo apt-get install build-essential	
	## sudo apt-get install time libgsl-dev libsqlite3-dev libprotobuf-dev protobuf-compiler r-base r-base-dev
	##############################
	@read -r -p "## Hit enter to continue with R package installation." CONTINUE
	$(MAKE) -C $(PREP_DIR) PACKAGES_INSTALLED

##############################
## regression testing
##############################
##
## unused at present
TEST_PROCESSES = 4

test.run : print.vars $(BIN_NAME)
	$(MAKE) -C $(PREP_DIR) test

## clean weather after test
test : test.run clean.weather

version: 
	##?? git repo not on docker...
	$(MAKE) -C $(PREP_DIR) version
	## For version info, see PACKAGE_INFO and PbVersion protobuf
	@echo "## For version info, see also PbVersion protobuf" > PACKAGE_INFO
	@echo "## Git hash / commit date" >> PACKAGE_INFO
	@echo `git log --pretty=format:'%h %t %aD' -n 1` >> PACKAGE_INFO
	@echo "## Git version" >> PACKAGE_INFO
	@echo `git rev-parse --abbrev-ref HEAD` >> PACKAGE_INFO
