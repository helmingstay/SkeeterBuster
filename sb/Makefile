export LD_LIBRARY_PATH=/usr/local/lib 

OBJECTS := Aedes.o stdafx.o Globals.o Output.o ConfigFile.o Traditional.o Binomial.o Building.o EggsCohort.o Female_Adult_Cohort.o Fitness.o Larvae_Cohort.o Male_Adult_Cohort.o Pupae_Cohort.o Receptacle.o stats.o Input.o Destructive_Sampling.o SampledHouses.o
SkeeterBuster : $(OBJECTS)
	g++ Aedes.o stdafx.o Globals.o Output.o ConfigFile.o Traditional.o Binomial.o Building.o EggsCohort.o Female_Adult_Cohort.o Fitness.o Larvae_Cohort.o Male_Adult_Cohort.o Pupae_Cohort.o Receptacle.o stats.o Input.o Destructive_Sampling.o SampledHouses.o -Wall -o SkeeterBuster 


Aedes.o : Aedes.cpp
	g++ -O0 -g3 -Wall -c  Aedes.cpp

stdafx.o : stdafx.cpp
	g++ -O0 -g3 -Wall -c  stdafx.cpp 

Globals.o : Globals.cpp
	g++ -O0 -g3 -Wall -c  Globals.cpp 

ConfigFile.o : ConfigFile.cpp
	g++ -O0 -g3 -Wall -c  ConfigFile.cpp 

Output.o : Output.cpp
	g++ -O0 -g3 -Wall -c  Output.cpp 

Building.o : Building.cpp
	g++ -O0 -g3 -Wall -c  Building.cpp

Traditional.o : Traditional.cpp
	g++ -O0 -g3 -Wall -c  Traditional.cpp 

Binomial.o : Binomial.cpp
	g++ -O0 -g3 -Wall -c  Binomial.cpp 

EggsCohort.o : EggsCohort.cpp
	g++ -O0 -g3 -Wall -c  EggsCohort.cpp 

Female_Adult_Cohort.o : Female_Adult_Cohort.cpp
	g++ -O0 -g3 -Wall -c  Female_Adult_Cohort.cpp 

Fitness.o : Fitness.cpp
	g++ -O0 -g3 -Wall -c  Fitness.cpp 

Male_Adult_Cohort.o : Male_Adult_Cohort.cpp
	g++ -O0 -g3 -Wall -c  Male_Adult_Cohort.cpp 

Pupae_Cohort.o : Pupae_Cohort.cpp
	g++ -O0 -g3 -Wall -c  Pupae_Cohort.cpp 

Receptacle.o : Receptacle.cpp
	g++ -O0 -g3 -Wall -c  Receptacle.cpp 

resource.o : resource.cpp
	g++ -O0 -g3 -Wall -c  resource.cpp 

stats.o : stats.cpp
	g++ -O0 -g3 -Wall -c  stats.cpp 

Input.o : Input.cpp
	g++ -O0 -g3 -Wall -c  Input.cpp 

Destructive_Sampling.o : Destructive_Sampling.cpp
	g++ -O0 -g3 -Wall -c  Destructive_Sampling.cpp 

Sampled_Houses.o : Sampled_Houses.cpp
	g++ -O0 -g3 -Wall -c  Sampled_Houses.cpp

Larvae_Cohort.o : Larvae_Cohort.cpp
	g++ -O0 -g3 -Wall -c Larvae_Cohort.cpp


.PHONY : clean

clean :
	rm SkeeterBuster *.o
