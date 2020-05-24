//    Copyright 2006-2010 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould
//
//    This file is part of Skeeter Buster.
//
//    Skeeter Buster is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Skeeter Buster is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Skeeter Buster.  If not, see <http://www.gnu.org/licenses/>.


#include "stdafx.h"
#include "Aedes.h"
#include "Aedes_class_defs.h"
#include "ConfigFile.h"

//#define USE_GOMP 1

// Globals that are a necessary evil

// TEMP
double tmp_totalpupae;
double tmp_avg_weight_pupae;
double tmp_totaladults;
double tmp_avg_weight_adults;
double tmp_totaleggs;
//

int Date;
int Number_of_Container_Types;
int House_position;
int Female_ID;

int ref ;
char date[12]="" ;
char *hour;

FILE *Output, *Output_per_House, *Short_Output, *Output_per_Container_Type, *Output_per_Container, *Output_Water_Level, //
*EPS_Output, *EPS_Density_Output, *EPS_Wolbachia_Output, *EPS_XT_Output ; //log files

FILE *test1, *test2, *gentime, *forSbeed, *fstbydist, *larvtime, *xt_adulticide;

/************** ad-hoc FIX; should be refactored *****************/
SampledHouses *sampled_houses; // Annoyingly this has to be declared outside of the main for loop irrespective of whether destructive sampling is done or not - leads to redunandancy and is inelegent


vector<Building> House;
list<int> edges ; // list of houses on the edges



double CV_Pupae,Total_Eggs,Total_Male_Larvae,Total_Female_Larvae,Total_Male_Pupae,Total_Female_Pupae,Total_Nulliparous_Female_Adults,Total_Parous_Female_Adults,Total_Male_Adults, CV_WILD_FEMALES, CV_WILD_ALLELES, NO_WILD_FEMALES, NO_WILD_ALLELES;
vector<double> Total_Genotypes;
vector<double> Total_Adult_Genotypes;
vector<double> Total_wolbachia_frequency;

double Census_by_container_type[11][200] ;
// first dimension : no. containers, no.eggs, no. larvae, no.pupae, no. pupae-positive containers, average pupae weight, water level, average food amount
// second dimension : by container type

double **Sherlock ;

int xt_selected_line, transect_width;
double *transect_values;
double *transect_densities;
int *mask, *divisor ;
int number_of_genotypes ;
int number_of_medea_masks ;
double gen_prob ;
double *allele_frequency;
double **allele_frequencies;

int *cumul_pupae_per_house;

void doSomeCalculation()
{
	double a = 2;
	for(int i=0;i<1000;i++)
	{
		a = 2*a;
	}
}

double FoodGain_Calculation(int Day, double Daily_Food_Gain, double Initial_Food_Present)
{
	double FoodGain=0.0;

	if (Day<31)
	{
		if (Day%3==0) FoodGain=3*Daily_Food_Gain;
		if (Day==0) FoodGain=Initial_Food_Present+2*Daily_Food_Gain;
	}
	else if (Day>303)
	{
		if ((Day-304)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>272)
	{
		if ((Day-273)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>333)
	{
		if ((Day-334)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>242)
	{
		if ((Day-243)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>211)
	{
		if ((Day-212)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>180)
	{
		if ((Day-181)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>150)
	{
		if ((Day-151)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>119)
	{
		if ((Day-120)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>89)
	{
		if ((Day-90)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>58)
	{
		if ((Day-59)%3==0) FoodGain=3*Daily_Food_Gain;
	}
	else if (Day>30)
	{
		if ((Day-31)%3==0) FoodGain=3*Daily_Food_Gain;
	}



	return(FoodGain);
}


/***********************************************************
 * Random generator initialization                         *
 *                      by a simple Congruential generator *
 ***********************************************************/


long ra[M+1], nd;

void seed(long seed)
{
	int  i;

	if(seed<0) { puts("SEED error."); return; }
	ra[0]= (long) fmod(16807.0*(double)seed, 2147483647.0);
	for(i=1; i<=M; i++)
	{
		ra[i] = (long)fmod( 16807.0 * (double) ra[i-1], 2147483647.0);
	}
}



int main(int argc,char **argv)
{
	double Development_Numerator,Development_Denominator,Development_134,Development_26=0.0;
	double Water_Average_Temp_K;
	double Img = 0.;

	//#1


	try {ReadConfigFile();}
	catch (struct ConfigFile::file_not_found e)
	{
		ErrorMessage(2);
	}
	catch (struct ConfigFile::key_not_found ek)
	{
		//fprintf(stdout, "\n*** Key not found in file: %s\n", ek.key);
		ErrorMessage(3);
	}

	ReadHouseSetup();

	CheckParameters();

	WeatherInput();

	seed(time(0));  //initializing random variable seed

	//opening of log files

	Get_Date() ;


	CreateDirectories();

	ref=Get_Number();

	OpenLogFiles();


	// setting some constant values

	// TEMP
	tmp_totalpupae=0;
	tmp_avg_weight_pupae=0;
	tmp_totaladults=0;
	tmp_avg_weight_adults=0;
	//

	char sref[10]="";
	sprintf(sref,"%d",ref);

	if (TEST_1)
	{
		char path_test[50]="";
		strcat(path_test,"test1.ref");
		strcat(path_test,sref);
		strcat(path_test,".txt");
		test1=fopen(path_test, "w");
	}

	if (TEST_2)
	{
		char path_test2[50]="";
		strcat(path_test2,"test2.ref");
		strcat(path_test2,sref);
		strcat(path_test2,".txt");
		test2=fopen(path_test2, "w");
	}

	if (GENTIME)
	{
		char path_gentime[50]="";
		strcat(path_gentime,"gentime.ref");
		strcat(path_gentime,sref);
		strcat(path_gentime,".txt");
		gentime=fopen(path_gentime, "w");
	}

	if (LARVTIME)
	{
		char path_larvtime[50]="";
		strcat(path_larvtime,"larvtime.ref");
		strcat(path_larvtime,sref);
		strcat(path_larvtime,".txt");
		larvtime=fopen(path_larvtime, "w");
	}

	if (OUTPUT_FOR_SBEED)
	{
		char path_sbeed[50]="";
		strcat(path_sbeed,"forSbeed.ref");
		strcat(path_sbeed,sref);
		strcat(path_sbeed,".txt");
		forSbeed=fopen(path_sbeed, "w+");
		fprintf(forSbeed, "NUMBER OF HOUSES = %d\n\n", NUMBER_OF_HOUSES);
		fprintf(forSbeed, "NUMBER_OF_COLUMNS = %d\n\n", NUMBER_OF_COLUMNS);
		fprintf(forSbeed, "NUMBER_OF_ROWS = %d\n\n", NUMBER_OF_ROWS);
		fprintf(forSbeed, "NUMBER OF DAYS = %d\n\n", NUMBER_OF_DAYS);
		fprintf(forSbeed, "GENETIC_CONTROL_METHOD = %d\n\n", GENETIC_CONTROL_METHOD);
	}

	if (OUTPUT_PER_CONTAINER)
	{
		char path_output_per_container[200]="";
		strcat(path_output_per_container,"Output_per_Container.ref");
		strcat(path_output_per_container,sref);
		strcat(path_output_per_container,".txt");
		Output_per_Container=fopen(path_output_per_container, "w");
		fprintf(Output_per_Container, "Day House Container Food Container_Type Filling_Method Eggs 1st 2nd 3rd 4th Pupae_Total NewPupae_Total NewPupae_Female NewPupae_Male NewPupaeWeight_Female NewPupaeWeight_Male Water_Level Food\n");
	}


	transect_width=5;
	xt_selected_line = (NUMBER_OF_ROWS/2)-(transect_width/2);
	int max_dimension = (NUMBER_OF_ROWS>NUMBER_OF_COLUMNS ? NUMBER_OF_ROWS : NUMBER_OF_COLUMNS);
	transect_values=new double[max_dimension];
	transect_densities=new double[max_dimension];
	for (int c=0 ; c<max_dimension ; c++)
	{
		transect_values[c]=0.;
		transect_densities[c]=0.;
	}

	if (EPS_YT_OUTPUT && (((TRADITIONAL_CONTROL_METHOD==2)&&(ADULTICIDE_REGION==1))||(GENETIC_CONTROL_METHOD==7)||(NUMBER_OF_INCOMPATIBLE_WOLBACHIA)))
	{
		char path_xt_adulticide[50]="";
		strcat(path_xt_adulticide,"xt_adulticide.ref");
		strcat(path_xt_adulticide,sref);
		strcat(path_xt_adulticide,".txt");
		xt_adulticide=fopen(path_xt_adulticide, "w+");
	}



	mask=new int[CHROM_NUMBER];
	divisor=new int[CHROM_NUMBER];
	allele_frequency= new double [2*CHROM_NUMBER];
	allele_frequencies = new double* [2*CHROM_NUMBER];
	for (int chrom = 0 ; chrom<2*CHROM_NUMBER ; chrom++)
	{
		allele_frequencies[chrom]=new double [NUMBER_OF_HOUSES];
		for (int house = 0 ; house<NUMBER_OF_HOUSES ; house++)
			allele_frequencies[chrom][house]=0.;
	}

	cumul_pupae_per_house = new int [NUMBER_OF_HOUSES];

	int a=3,b=1;

	for (int c=0; c<CHROM_NUMBER; ++c)
	{
		mask[c]=a;
		divisor[c]=b;
		a=4*a;
		b=4*b;
	}
	number_of_genotypes=(int) pow(2.0,2*CHROM_NUMBER);
	if (GENETIC_CONTROL_METHOD==MEDEA) number_of_medea_masks=(int) pow(2.0,MEDEA_UNITS) ;
	else number_of_medea_masks=1 ;
	gen_prob=pow(0.25,CHROM_NUMBER);

	Sherlock = new double* [NUMBER_OF_HOUSES+1] ;
	for (int v=0 ; v<NUMBER_OF_HOUSES+1 ; v++)
	{
		Sherlock[v] = new double [7] ;
		for (int w=0 ; w<7 ; w++)
			Sherlock[v][w]=0. ;
	}


	double *average_pupae_per_house;
	average_pupae_per_house = new double[NUMBER_OF_HOUSES] ;
	for (int v=0 ; v<NUMBER_OF_HOUSES ; v++)
		average_pupae_per_house[v] = 0.;

	//calculation of larval developmental rate at 13.4C

	if (DEVELOPMENT_MODE==FOCKSDEVEL)
	{
		Water_Average_Temp_K=286.55;
		Development_Numerator=RO25_LARVAL_DEVELOPMENT*(Water_Average_Temp_K/298)*exp((DHA_LARVAL_DEVELOPMENT/R)*(((double)1/298)-(1/Water_Average_Temp_K)));
		Development_Denominator=1+exp((DHH_LARVAL_DEVELOPMENT/R)*((1/THALF_LARVAL_DEVELOPMENT)-(1/Water_Average_Temp_K)));
		Development_134=(Development_Numerator/Development_Denominator)*24;
	}
	else Development_134=0;


	if (SIMULATION_MODE1==CIMSIM) Development_134=0.00146;

	//calculation of larval developmental rate at 26C
	if (DEVELOPMENT_MODE==FOCKSDEVEL)
	{
		if (SIMULATION_MODE1==CIMSIM) Water_Average_Temp_K=299; //according to CIMSIM with error
		if (SIMULATION_MODE1==SKEETER_BUSTER) Water_Average_Temp_K=299.15;	//without error
		Development_Numerator=RO25_LARVAL_DEVELOPMENT*(Water_Average_Temp_K/298)*exp((DHA_LARVAL_DEVELOPMENT/R)*(((double)1/298)-(1/Water_Average_Temp_K)));
		Development_Denominator=1+exp((DHH_LARVAL_DEVELOPMENT/R)*((1/THALF_LARVAL_DEVELOPMENT)-(1/Water_Average_Temp_K)));
		Development_26=(Development_Numerator/Development_Denominator)*24;
	}
	else Development_26=12.6/DEGREE_HOURS_TO_BE_REACHED;


	//#2








	int number_of_genotypes;

	number_of_genotypes=(int) pow(2.0,2*CHROM_NUMBER);

	int number_of_wolbachia_status;

	number_of_wolbachia_status = (int) pow(2.0,NUMBER_OF_INCOMPATIBLE_WOLBACHIA);


	for (int genotype=0;genotype<number_of_genotypes;genotype++) {Total_Genotypes.push_back(0); Total_Adult_Genotypes.push_back(0);}

	for (int w=0 ; w<number_of_wolbachia_status ; w++)
		Total_wolbachia_frequency.push_back(0);


	vector<Building>::iterator itHouse;
	House_position=0;
	for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
	{
		if (NUMBER_OF_HOUSES>1)
		{
			//north
			itHouse->neighbour_position[0]=House_position-NUMBER_OF_COLUMNS;
			if (itHouse->neighbour_position[0]<0)
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[0]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[0]=House_position+NUMBER_OF_COLUMNS;
				if (DISPERSAL_MODE==TORE) itHouse->neighbour_position[0]+=NUMBER_OF_HOUSES ;
				if (DISPERSAL_MODE==RANDOM_EDGES) itHouse->neighbour_position[0]= -1 ;
			}

			//east
			itHouse->neighbour_position[1]=House_position+1;
			/*if (itHouse->neighbour_position[1]>(NUMBER_OF_HOUSES-1))
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[1]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[1]=House_position-1;
			} */
			if (((House_position+1)%NUMBER_OF_COLUMNS)==0)
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[1]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[1]=House_position-1;
				if (DISPERSAL_MODE==TORE) itHouse->neighbour_position[1]-=NUMBER_OF_COLUMNS ;
				if (DISPERSAL_MODE==RANDOM_EDGES) itHouse->neighbour_position[1]= -1 ;
			}

			//south
			itHouse->neighbour_position[2]=House_position+NUMBER_OF_COLUMNS;
			if (itHouse->neighbour_position[2]>(NUMBER_OF_HOUSES-1))
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[2]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[2]=House_position-NUMBER_OF_COLUMNS;
				if (DISPERSAL_MODE==TORE) itHouse->neighbour_position[2]-=NUMBER_OF_HOUSES ;
				if (DISPERSAL_MODE==RANDOM_EDGES) itHouse->neighbour_position[2]= -1 ;
			}

			//west
			itHouse->neighbour_position[3]=House_position-1;
			/*if (itHouse->neighbour_position[3]<0)
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[3]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[3]=House_position+1;
			}*/
			if ((House_position%NUMBER_OF_COLUMNS)==0)
			{
				if (DISPERSAL_MODE==STICKY_BORDER) itHouse->neighbour_position[3]=House_position;
				if (DISPERSAL_MODE==BOUNCE) itHouse->neighbour_position[3]=House_position+1;
				if (DISPERSAL_MODE==TORE) itHouse->neighbour_position[3]+= NUMBER_OF_COLUMNS ;
				if (DISPERSAL_MODE==RANDOM_EDGES) itHouse->neighbour_position[3]= -1 ;
			}

			//printf("%d %d %d %d\n",itHouse->neighbour_position[0],itHouse->neighbour_position[1],itHouse->neighbour_position[2],itHouse->neighbour_position[3]);
		}
		else
		{
			itHouse->neighbour_position[0]=0;
			itHouse->neighbour_position[1]=0;
			itHouse->neighbour_position[2]=0;
			itHouse->neighbour_position[3]=0;
		}

		itHouse->neighbour_cumul_attractiveness[0]=House[itHouse->neighbour_position[0]].Attractiveness;
		for (int i=1 ; i<4 ; i++)
		{
			itHouse->neighbour_cumul_attractiveness[i]=itHouse->neighbour_cumul_attractiveness[i-1]+House[itHouse->neighbour_position[i]].Attractiveness;
		}
		House_position++;

	}


	// populating list of the houses on the edges

	int i=0 ;
	do{
		edges.push_back(i); //north
		edges.push_front(NUMBER_OF_HOUSES-1-i); //south
	} while (++i < NUMBER_OF_COLUMNS-1) ;
	i=0 ;
	do{
		edges.push_back((1+i)*NUMBER_OF_COLUMNS) ; //west
		edges.push_back(((1+i)*NUMBER_OF_COLUMNS)-1) ; //east
	} while (++i < NUMBER_OF_ROWS-1) ;

	/*
		cout << "Houses on the edge : " << endl ;
		list<int>::const_iterator iter ;
		for (iter=edges.begin() ; iter!=edges.end() ; iter++)
			cout << (*iter) << " " ;*/


	for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
	{
		itHouse->ContainerReleaseDates.sort();
		itHouse->ContainerReleaseDates.unique();
	}


	//this is where the actual simulation starts, with the house cycle

	fprintf(stdout, "\nStarting the simulation\n");

	Female_ID=0;

	// Main day loop

	for (Date=0;Date<NUMBER_OF_DAYS;Date++)
	{

		for (int row=0 ; row<11 ; row++)
			for (int type=0 ; type<200 ; type++)
				Census_by_container_type[row][type]=0 ;
		// this initializes the counting per container type

		for (int c=0 ; c<max_dimension ; c++)
		{
			transect_values[c]=0.;
			transect_densities[c]=0.;
		}
		// this initializes transect calculations every day

		if (Date%365==0)
			for (int h=0 ; h<NUMBER_OF_HOUSES ; h++)
				cumul_pupae_per_house[h]=0;


		// debug
		/*for (int v=0 ; v<NUMBER_OF_HOUSES+1 ; v++)
		for (int w=0 ; w<5 ; w++)
			Sherlock[v][w]=0. ;
		 */
		// end of debug

		//this writes the number of date in the detailed log file

		if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %d\n","Day ",Date);

		if (OUTPUT_FOR_SBEED)
		{
			fprintf(forSbeed, "\n%%%d\n\n", Date);
			itHouse = House.begin();
			fprintf(forSbeed, "\nAVERAGE TEMPERATURE = %lg\n\n", itHouse->Average_Daily_Air_Temperature[Date]);
		}

		// pushing the released containers (including the initial setup on Date 0)
		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
			if (*(itHouse->ContainerReleaseDates.begin())==Date)
				// go through the reserve containers only if there is any container
				// to release on that date, otherwise, no need.
			{
				//itHouse->ContainerReleaseDates.pop_front(); // remove that date, next event becomes first in list
				itHouse->ContainerReleaseDates.remove(Date); // Tudor's comment April 8, 2011: pop_front causes a core dump under cygwin

				vector<Receptacle>::iterator itContainer;

				int Container_ID = itHouse->Number_of_Containers;

				bool special_house=false;
				if (INCREASED_CONTAINER_VARIANCE)
					special_house=(Random_Number()<(1./INCREASED_VARIANCE_EVERY_N_HOUSES));

				for (itContainer = itHouse->Reserve_Containers.begin() ; itContainer != itHouse->Reserve_Containers.end() ; ) // no increment, erase will move pointer
				{
					if (itContainer->Release_Date==Date)
					{
						if (Date==0)
						{
							// specific cases for initial container setup

							if (INCREASED_CONTAINER_VARIANCE)
								// 1. increased variance
								// in 1/N houses, containers are multiplied by N-1
								// in the other houses, containers are "divided" by N-1
								// expectation of total number of containers remains the same
								// (1/N)*(N-1) + ((N-1)/N)*(1/(N-1)) = 1
							{
								if (special_house)
									for (int c=0 ; c<INCREASED_VARIANCE_EVERY_N_HOUSES-1 ; c++)
									{
										itContainer->ID=Container_ID++;
										itHouse->Container.push_back(*itContainer);
									}
								else
									if (Random_Number()<(1./(INCREASED_VARIANCE_EVERY_N_HOUSES-1)))
									{
										itContainer->ID=Container_ID++;
										itHouse->Container.push_back(*itContainer);
									}

							}
							else
								// 2. container multiplication
							{
								for (int c=0 ; c<CONTAINER_MULTIPLIER_FACTOR ; c++)
								{
									itContainer->ID=Container_ID++;
									itHouse->Container.push_back(*itContainer);
								}
							}
						}
						else
							itHouse->Container.push_back(*itContainer);

						itContainer=itHouse->Reserve_Containers.erase(itContainer); // this moves the pointer
						//itHouse->Number_of_Containers++;
					}
					else itContainer++;
				}
				itHouse->Number_of_Containers=itHouse->Container.size();
			}


		// releases at stable age distribution
		// converted cohorts are stacked into releases lists and vectors
		// and pushed back in the next block

		if (SAD_INTRODUCTION==true)
			if (Date==SAD_INTRODUCTION_DATE)
				for (itHouse=House.begin() ; itHouse!=House.end() ; itHouse++)
					if (
							(SAD_INTRODUCTION_REGION==0) // in every house
							||
							(
									(SAD_INTRODUCTION_REGION==1) // in defined region
									&&
									((itHouse->Position/NUMBER_OF_COLUMNS)>=SAD_INTRODUCTION_Y_MIN)
									&&
									((itHouse->Position/NUMBER_OF_COLUMNS)<=SAD_INTRODUCTION_Y_MAX)
									&&
									((itHouse->Position%NUMBER_OF_COLUMNS)>=SAD_INTRODUCTION_X_MIN)
									&&
									((itHouse->Position%NUMBER_OF_COLUMNS)<=SAD_INTRODUCTION_X_MAX)
							)
					)
					{
						vector<Receptacle>::iterator itContainer;
						for (itContainer=itHouse->Container.begin() ; itContainer!=itHouse->Container.end() ; itContainer++)
						{
							list<Eggs_Cohort>::iterator itEggs;
							for (itEggs = itContainer->Eggs.begin(); itEggs != itContainer->Eggs.end(); )
							{
								int converted = Binomial_Deviate(itEggs->Number, SAD_INTRODUCTION_FREQUENCY);
								if (converted)
								{
									Eggs_Cohort Converted_Eggs = *itEggs;
									Converted_Eggs.Genotype = SAD_INTRODUCTION_FEMALE_GENOTYPE+(itEggs->Genotype%2);
									Converted_Eggs.Number = converted;
									Converted_Eggs.Release_Date = Date;
									itContainer->ReleaseEggs.push_back(Converted_Eggs);
									itEggs->Number-=converted;
									if (itEggs->Number==0)
										itEggs=itContainer->Eggs.erase(itEggs);
									else itEggs++;
								}
								else itEggs++;
							}

							list<Larvae_Cohort>::iterator itLarvae;
							for (itLarvae = itContainer->Larvae.begin(); itLarvae != itContainer->Larvae.end(); )
							{
								int converted = Binomial_Deviate(itLarvae->Number, SAD_INTRODUCTION_FREQUENCY);
								if (converted)
								{
									Larvae_Cohort Converted_Larvae = *itLarvae;
									Converted_Larvae.Genotype = SAD_INTRODUCTION_FEMALE_GENOTYPE+(itLarvae->Genotype%2);
									Converted_Larvae.Number = converted;
									Converted_Larvae.Release_Date = Date;
									itContainer->ReleaseLarvae.push_back(Converted_Larvae);
									itLarvae->Number-=converted;
									if (itLarvae->Number==0)
										itLarvae=itContainer->Larvae.erase(itLarvae);
									else itLarvae++;
								}
								else itLarvae++;
							}

							list<Pupae_Cohort>::iterator itPupae;
							for (itPupae = itContainer->Pupae.begin(); itPupae != itContainer->Pupae.end(); )
							{
								int converted = Binomial_Deviate(itPupae->Number, SAD_INTRODUCTION_FREQUENCY);
								if (converted)
								{
									Pupae_Cohort Converted_Pupae = *itPupae;
									Converted_Pupae.Genotype = SAD_INTRODUCTION_FEMALE_GENOTYPE+(itPupae->Genotype%2);
									Converted_Pupae.Number = converted;
									Converted_Pupae.Release_Date = Date;
									itContainer->ReleasePupae.push_back(Converted_Pupae);
									itPupae->Number-=converted;
									if (itPupae->Number==0)
										itPupae=itContainer->Pupae.erase(itPupae);
									else itPupae++;
								}
								else itPupae++;
							}
						}

						list<Male_Adult_Cohort>::iterator itMaleAdult;
						for (itMaleAdult = itHouse->Male_Adult.begin() ; itMaleAdult != itHouse->Male_Adult.end() ; )
						{
							int converted = Binomial_Deviate(itMaleAdult->Number, SAD_INTRODUCTION_FREQUENCY);
							if (converted)
							{
								Male_Adult_Cohort Converted_MaleAdult = *itMaleAdult;
								Converted_MaleAdult.Genotype = SAD_INTRODUCTION_FEMALE_GENOTYPE+(itMaleAdult->Genotype%2);
								Converted_MaleAdult.Number = converted;
								Converted_MaleAdult.Release_Date = Date;
								itHouse->Release_Male_Adult.push_back(Converted_MaleAdult);
								itMaleAdult->Number-=converted;
								if (itMaleAdult->Number==0)
									itMaleAdult=itHouse->Male_Adult.erase(itMaleAdult);
								else itMaleAdult++;
							}
							else itMaleAdult++;
						}

						list<Female_Adult>::iterator itFemale;
						for (itFemale = itHouse->Female_Adults.begin() ; itFemale != itHouse->Female_Adults.end() ; itFemale++)
						{
							if (Random_Number()<SAD_INTRODUCTION_FREQUENCY)
								itFemale->Genotype = SAD_INTRODUCTION_FEMALE_GENOTYPE;
						}
					}


		// pushing the released cohorts
		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
		{
			vector<Receptacle>::iterator itContainer;
			for (itContainer = itHouse->Container.begin(); itContainer != itHouse->Container.end(); itContainer++)
			{
				list<Eggs_Cohort>::iterator itReleaseEggs;
				for (itReleaseEggs = itContainer->ReleaseEggs.begin(); itReleaseEggs != itContainer->ReleaseEggs.end(); itReleaseEggs++)
				{
					if (itReleaseEggs->Release_Date==Date)
					{
						itContainer->Eggs.push_back(*itReleaseEggs);
						tmp_totaleggs+=(itReleaseEggs->Number);
					}
				}

				list<Larvae_Cohort>::iterator itReleaseLarvae;
				for (itReleaseLarvae = itContainer->ReleaseLarvae.begin(); itReleaseLarvae != itContainer->ReleaseLarvae.end(); itReleaseLarvae++)
				{
					if (itReleaseLarvae->Release_Date==Date)
					{
						itContainer->Larvae.push_back(*itReleaseLarvae);
					}
				}

				list<Pupae_Cohort>::iterator itReleasePupae;
				for (itReleasePupae = itContainer->ReleasePupae.begin(); itReleasePupae != itContainer->ReleasePupae.end(); itReleasePupae++)
				{
					if (itReleasePupae->Release_Date==Date)
					{
						itContainer->Pupae.push_back(*itReleasePupae);
					}
				}

			}

			list<Female_Adult>::iterator itRelease_Female_Adult;
			for (itRelease_Female_Adult = itHouse->Release_Female_Adult.begin(); itRelease_Female_Adult != itHouse->Release_Female_Adult.end(); itRelease_Female_Adult++)
			{
				if (itRelease_Female_Adult->Release_Date==Date)
				{
					itRelease_Female_Adult->ID = Female_ID++;
					itRelease_Female_Adult->Emergence_Place = itHouse->Position;
					// REMOVE
					//itRelease_Female_Adult->Mated=true;
					//itRelease_Female_Adult->Male_Genotype=1;
					//
					itHouse->Female_Adults.push_back(*itRelease_Female_Adult);
				}
			}

			list<Male_Adult_Cohort>::iterator itRelease_Male_Adult;
			for (itRelease_Male_Adult = itHouse->Release_Male_Adult.begin(); itRelease_Male_Adult != itHouse->Release_Male_Adult.end(); itRelease_Male_Adult++)
			{
				if (itRelease_Male_Adult->Release_Date==Date)
				{
					itHouse->Male_Adult.push_back(*itRelease_Male_Adult);
				}
			}


		}



		if (Date==0)
		{


			if (MARK_RELEASE_RECAPTURE)
			{
				Female_Adult release_female(NULLIPAROUS,0,0* FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT,0,0,0,0,0,0,0, false, 0, 0);
				release_female.Age=0;
				release_female.Mature=0;
				release_female.Physiological_Development_Percent=0;
				//  House[Number_of_Rows/2*Number_of_Columns+Number_of_Columns/2].Female_Adult.push_back(release_females);
				House[0].Female_Adults.push_back(release_female);

				Male_Adult_Cohort release_males(6950,0.5,1,0,0,0,0,0,0);
				release_males.Age=0;
				release_males.Mature=0;
				release_males.Physiological_Development_Percent=0;
				//  House[Number_of_Rows/2*Number_of_Columns+Number_of_Columns/2].Male_Adult.push_back(release_males);
				House[0].Male_Adult.push_back(release_males);
			}
		}
		if (MARK_RELEASE_RECAPTURE)
		{
			//selecting houses for recapture
			for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
			{
				itHouse->Recapture=1;
			}
		}


		Total_Eggs=Total_Male_Larvae=Total_Female_Larvae=Total_Male_Pupae=Total_Female_Pupae=Total_Nulliparous_Female_Adults=Total_Parous_Female_Adults=Total_Male_Adults=0;
		CV_Pupae=0 ;
		CV_WILD_FEMALES = 0;
		CV_WILD_ALLELES = 0;

		NO_WILD_FEMALES = 0;
		NO_WILD_ALLELES = 0;
		for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Genotypes[genotype]=Total_Adult_Genotypes[genotype]=0;
		for (int allele=0;allele<2*CHROM_NUMBER;allele++) allele_frequency[allele]=0;
		for (int w=0 ; w<number_of_wolbachia_status ; w++) Total_wolbachia_frequency[w]=0;

#ifdef USE_GOMP
		int num_houses = (int)House.size();
		//Building buildings[num_houses];
		Building ** buildings = new Building*[num_houses];
		int current_b = 0;
		for (int current_b = 0; current_b < num_houses; current_b++)
		{
			buildings[current_b] = &House[current_b];
		}
#pragma omp parallel for
		for (current_b = 0; current_b < num_houses; current_b++)
		{
			buildings[current_b]->BuildingCalculations(Date,Development_26,Development_134,current_b);
		}
		/*
		for (int current_b = 0; current_b < num_houses; current_b++)
		{
			House[current_b] = buildings[current_b];
		}*/
#else
		// main house loop
		House_position=0;
		for (itHouse = House.begin(); itHouse != House.end(); itHouse++ )
		{
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %d\n","House ",House_position);
			itHouse->BuildingCalculations(Date,Development_26,Development_134,House_position);
			House_position++;
		}
#endif

		//moving dispersing adult mosquitoes to their destination houses
		House_position=0;
		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
		{
			for (list<Female_Adult>::iterator migrating_female_adult_it=itHouse->Migrating_Female_Adult.begin(); migrating_female_adult_it != itHouse->Migrating_Female_Adult.end();)
			{
				itHouse->Female_Adults.push_back(*migrating_female_adult_it);
				migrating_female_adult_it=itHouse->Migrating_Female_Adult.erase(migrating_female_adult_it);
			}
			for (list<Male_Adult_Cohort>::iterator migrating_male_adult_it=itHouse->Migrating_Male_Adult.begin(); migrating_male_adult_it != itHouse->Migrating_Male_Adult.end();)
			{
				itHouse->Male_Adult.push_back(*migrating_male_adult_it);
				migrating_male_adult_it=itHouse->Migrating_Male_Adult.erase(migrating_male_adult_it);
			}
			House_position++;
		} //this is the end of the dispersal loop

		// traditional control methods
		//
		// parameter TRADITIONAL_CONTROL_METHOD (integer)is read as a binary sequence
		// each position corresponds to a boolean for a given method
		//
		// e.g. TRADITIONAL_CONTROL_METHOD=0 is read as 000 -> none of the methods are turned on
		//		TRADITIONAL_CONTROL_METHOD=7 is read as 111 -> all three methods are turned on
		//		TRADITIONAL_CONTROL_METHOD=5 is read as 101 -> only the first and third are turned on

		if (TRADITIONAL_CONTROL_METHOD)
		{
			// larvicides (position 0)
			if ((TRADITIONAL_CONTROL_METHOD)%2)
				if ((Date >= LARVICIDE_BEGIN_DATE) && (Date <= LARVICIDE_END_DATE) && (Date%LARVICIDE_FREQUENCY==0))
					Larvicide(LARVICIDE_TYPE, LARVICIDE_REGION, LARVICIDE_SIZE);

			// adulticide (position 1)
			if ((TRADITIONAL_CONTROL_METHOD>>1)%2)
				if ((Date >= ADULTICIDE_BEGIN_DATE)&&(Date <= ADULTICIDE_END_DATE))
					Adulticide(ADULTICIDE_TYPE, ADULTICIDE_REGION, ADULTICIDE_SIZE);

			// source removal (position 2)
			if ((TRADITIONAL_CONTROL_METHOD>>2)%2)
				if (Date == SOURCE_REMOVAL_BEGIN_DATE)
					Source_Removal(SOURCE_REMOVAL_TYPE, SOURCE_REMOVAL_REGION, SOURCE_REMOVAL_SIZE);
		}

		// Potentially conduct destructive sampling
		if (CONDUCT_DESTRUCTIVE_SAMPLING)
			{
			if (Date==0)
				{
				sampled_houses = new SampledHouses(NUMBER_OF_DAYS_SAMPLED);
				sampled_houses->ReadSamplingFile();
				sampled_houses->SetupSamplingResults(ref);
				}

			/* check if it is the first date of sampling, the second date of sampling, etc... If no sampling, return -1
			  * Note these do not equal "Date" */
			int date_of_sampling = sampled_houses->CheckSamplingDate(Date);

			if (date_of_sampling >= 0) /* if there is going to be sampling on this date, */
				{
				if (DESTRUCTIVE_LARVAE_SAMPLING)
					{
					Destructively_Sample_Larvae (sampled_houses->indices_of_houses_sampled_on_Date[date_of_sampling], sampled_houses->Number_of_Larvae_Sampled_on_Date[date_of_sampling]);
					}

				if (DESTRUCTIVE_PUPAE_SAMPLING)
					{
					Destructively_Sample_Pupae (sampled_houses->indices_of_houses_sampled_on_Date[date_of_sampling], sampled_houses->Number_of_Pupae_Sampled_on_Date[date_of_sampling]);
					}
				if (DESTRUCTIVE_ADULT_SAMPLING)
					{
					Destructively_Sample_Adults (sampled_houses->indices_of_houses_sampled_on_Date[date_of_sampling], sampled_houses->Number_of_Adults_Sampled_on_Date[date_of_sampling]);
					}
				sampled_houses->writeSamplingResults(date_of_sampling);
				}
			if (Date==NUMBER_OF_DAYS)
				{
				sampled_houses->closeSamplingFiles();
				}
			}

		// removing special containers

		for (itHouse=House.begin(); itHouse!=House.end() ; itHouse++)
			for(vector<Receptacle>::iterator itContainer=itHouse->Container.begin() ; itContainer!=itHouse->Container.end() ; )
			{
				if (itContainer->Container_Type_Number==102)
				{
					if (
							itContainer->Eggs.empty()
							&&
							itContainer->Larvae.empty()
							&&
							itContainer->Pupae.empty()
					)
					{
						itContainer=itHouse->Container.erase(itContainer);
						itHouse->Number_of_Containers--;
					}
					else itContainer++;
				}
				else itContainer++;
			}

		//moving containers between houses

		int containers_in_house = 0 ;
		vector<Receptacle> tmp_containers;

		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
		{
			//vector<Receptacle>::iterator itContainer;
			//for (itContainer = itHouse->Container.begin(); itContainer != itHouse->Container.end(); itContainer++)

			containers_in_house = itHouse->Container.size() ;
			int idContainer = 0 ; // counter, not iterator anymore


			if (containers_in_house>1)
				//why this condition ?
				// (1) problems arise if adults are in the house and the only container is removed
				// (2) it is not unrealistic to assume that if only one container
				//     is available, it will not be moved
				//       (possible improvement : probability of container movement functin of no. containers ...

			{
				do
				{
					if ((Random_Number()<CONTAINER_MOVEMENT_PROBABILITY) && (itHouse->Container[idContainer].Container_Type_Number==5))
					{
						// choosing destination house
						// (randomly in the whole city, could be distance-dependent to focal house)
						int destination=(int)(NUMBER_OF_HOUSES*Random_Number()) ;

						Receptacle moving_container=itHouse->Container[idContainer];

						moving_container.Food = 0;
						moving_container.Water_Level = moving_container.Height ;
						//moving_container.Eggs.clear() ;
						moving_container.Larvae.clear() ;
						moving_container.Pupae.clear() ;

						moving_container.House_Indicator = destination ;

						//House[destination].Container.push_back(moving_container) ;
						//House[destination].Number_of_Containers++ ;
						tmp_containers.push_back(moving_container) ;
						itHouse->Container.erase(itHouse->Container.begin()+idContainer);
						itHouse->Number_of_Containers-- ;
						containers_in_house-- ;
					}

					// alternative : independant birth/death of containers
					// (then no. containers varies throughout simulation)
				}
				while (++idContainer < containers_in_house) ;
			}
		}
		for (vector<Receptacle>::iterator itmovContainer=tmp_containers.begin() ; itmovContainer!=tmp_containers.end() ; itmovContainer++)
		{
			int destination = itmovContainer->House_Indicator ;
			House[destination].Container.push_back(*itmovContainer) ;
			House[destination].Number_of_Containers++ ;
		}


		//making tallies by Houses
		House_position=0;


		if ((EPS_OUTPUT)&&((Date % EPS_OUTPUT_FREQ)==0))
		{
			EPS_Header(NUMBER_OF_COLUMNS,NUMBER_OF_ROWS) ;
		}
		if ((EPS_WOLBACHIA_OUTPUT)&&((Date % EPS_WOLBACHIA_OUTPUT_FREQ)==0))
		{
			EPS_Wolbachia_Header(NUMBER_OF_COLUMNS,NUMBER_OF_ROWS) ;
		}
		if ((EPS_DENSITY_OUTPUT)&&((Date % EPS_DENSITY_OUTPUT_FREQ)==0))
		{
			EPS_Density_Header(NUMBER_OF_COLUMNS,NUMBER_OF_ROWS) ;
		}
		if (((EPS_XT_OUTPUT)||(EPS_YT_OUTPUT))&&(Date==0))
		{
			EPS_XT_Header(NUMBER_OF_COLUMNS,NUMBER_OF_DAYS) ;
		}

		if (OUTPUT_FOR_SBEED)
			fprintf(forSbeed, "@\n");

		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ ) {itHouse->BuildingSummation(); House_position++;}
		if ((EPS_OUTPUT)&&((Date % EPS_OUTPUT_FREQ)==0))
		{
			EPS_Trailer() ;
			fclose(EPS_Output) ;
		}
		if ((EPS_WOLBACHIA_OUTPUT)&&((Date % EPS_WOLBACHIA_OUTPUT_FREQ)==0))
		{
			EPS_Wolbachia_Trailer() ;
			fclose(EPS_Wolbachia_Output) ;
		}
		if ((EPS_DENSITY_OUTPUT)&&((Date % EPS_DENSITY_OUTPUT_FREQ)==0))
		{
			EPS_Density_Trailer() ;
			fclose(EPS_Density_Output) ;
		}
		if ((EPS_XT_OUTPUT)||(EPS_YT_OUTPUT))
		{
			EPS_XT_Trailer() ;
			fclose(EPS_XT_Output) ;
		}

		if (OUTPUT_FOR_SBEED)
			fprintf(forSbeed, "@\n");


		double Total_Mosquitoes=0;

		for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Mosquitoes+=Total_Genotypes[genotype];

		if (Total_Mosquitoes>0)
		{
			for (int allele=0;allele<2*CHROM_NUMBER;allele++)
			{
				for (int genotype=0;genotype<number_of_genotypes;genotype++)
				{
					if ((genotype/((int) pow(2.0,2*CHROM_NUMBER-1-allele))%2)==0) allele_frequency[allele]+=Total_Genotypes[genotype]; else allele_frequency[allele+1]+=Total_Genotypes[genotype];
					if ((genotype/((int) pow(2.0,2*CHROM_NUMBER-2-allele))%2)==0) allele_frequency[allele]+=Total_Genotypes[genotype]; else allele_frequency[allele+1]+=Total_Genotypes[genotype];
				}
				allele++;
			}
			for (int allele=0;allele<2*CHROM_NUMBER;allele++) allele_frequency[allele]/=2*Total_Mosquitoes;

			for (int w=0; w<number_of_wolbachia_status ; w++)
				Total_wolbachia_frequency[w]/=Total_Mosquitoes;
		}

		double Recaptured_Females_per_distance[6],Recaptured_Males_per_distance[6];
		if (MARK_RELEASE_RECAPTURE)
		{
			for (int distance=0; distance<6;distance++) Recaptured_Females_per_distance[distance]=Recaptured_Males_per_distance[distance]=0;
			House_position=0;
			for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
			{
				if (itHouse->Recaptured_Female>0)
				{
					int distance;
					distance=(int) ROUND_(pow((double) ((House_position%6)*(House_position%6)+(House_position/6)*(House_position/6)),0.5));
					Recaptured_Females_per_distance[distance]+=itHouse->Recaptured_Female;
				}
				if (itHouse->Recaptured_Male>0)
				{
					int distance;
					distance=(int) ROUND_(pow((double) ((House_position%6)*(House_position%6)+(House_position/6)*(House_position/6)),0.5));
					Recaptured_Males_per_distance[distance]+=itHouse->Recaptured_Male;
				}
				House_position++;
			}
		}

		CV_Pupae /= (double)NUMBER_OF_HOUSES ;
		double Average_number_of_pupae = (double)(Total_Female_Pupae+Total_Male_Pupae)/(double)NUMBER_OF_HOUSES ;
		CV_Pupae -= Average_number_of_pupae*Average_number_of_pupae ;
		CV_Pupae = sqrt(CV_Pupae) ;
		CV_Pupae /= Average_number_of_pupae;

		if (GENETIC_CONTROL_METHOD==ReduceReplace)
			{
			CV_WILD_FEMALES /= (double)NUMBER_OF_HOUSES;
			CV_WILD_ALLELES /= (double)NUMBER_OF_HOUSES;
			double Average_Num_Wild_Female_Genotype = (double) (Total_Adult_Genotypes[0] + Total_Adult_Genotypes[16] + Total_Adult_Genotypes[32] + Total_Adult_Genotypes[48])/ (double)NUMBER_OF_HOUSES;
			double Average_Num_Wild_Female_ALLELE = (double) (Total_Adult_Genotypes[0]+Total_Adult_Genotypes[16]+Total_Adult_Genotypes[32]+Total_Adult_Genotypes[48] + Total_Adult_Genotypes[20]+Total_Adult_Genotypes[4]+Total_Adult_Genotypes[36]+Total_Adult_Genotypes[52] + Total_Adult_Genotypes[24]+Total_Adult_Genotypes[8]+Total_Adult_Genotypes[40]+Total_Adult_Genotypes[56])/ (double)NUMBER_OF_HOUSES;
			CV_WILD_FEMALES -= Average_Num_Wild_Female_Genotype*Average_Num_Wild_Female_Genotype;
			CV_WILD_ALLELES -= Average_Num_Wild_Female_ALLELE*Average_Num_Wild_Female_ALLELE;
			CV_WILD_FEMALES = sqrt(CV_WILD_FEMALES);
			CV_WILD_ALLELES = sqrt(CV_WILD_ALLELES);
			if (Average_Num_Wild_Female_Genotype)
				CV_WILD_FEMALES /= Average_Num_Wild_Female_Genotype;
			else
				CV_WILD_FEMALES = 0;

			if (Average_Num_Wild_Female_ALLELE)
				CV_WILD_ALLELES /= Average_Num_Wild_Female_ALLELE;
			else
				CV_WILD_ALLELES = 0;

			NO_WILD_FEMALES /= (double)NUMBER_OF_HOUSES;
			NO_WILD_ALLELES /= (double)NUMBER_OF_HOUSES;
			}
		// F-stats

		double fst=0.;

		if (FSTATS)
		{
			int width=4; // Why?
			int subpop_rows=NUMBER_OF_ROWS/width;
			int subpop_columns=NUMBER_OF_COLUMNS/width;

			double **pairfsts=new double*[subpop_rows*subpop_columns];
			for (int s1=0 ; s1<subpop_rows*subpop_columns ; s1++)
				pairfsts[s1] = new double[subpop_rows*subpop_columns];

			double *freq_in_subpop=new double[subpop_rows*subpop_columns];

			int chrom = 0 ;
			double vf=0;
			double aaf=0;

			for (int s1=0 ; s1<subpop_rows*subpop_columns ; s1++)
			{
				freq_in_subpop[s1]=0.;
				for (int s2=0 ; s2<subpop_rows*subpop_columns ; s2++)
					pairfsts[s1][s2]=0.;
			}

			for (int si=0 ; si<subpop_rows ; si++)
				for (int sj=0 ; sj<subpop_columns ; sj++)
				{
					double af=0.;
					int tf=0;
					for (int i=width*si ; i<width*(si+1) ; i++)
						for (int j=width*sj ; j<width*(sj+1) ; j++)
						{
							int house = i*NUMBER_OF_COLUMNS+j;
							Building *thisHouse = &(House[house]);
							int Total_Larvae = 0;
							for (int instar = 1 ; instar < 4.5 ; instar++)
								Total_Larvae += (thisHouse->TotalLarvae[0][instar-1] + thisHouse->TotalLarvae[1][instar-1]);
							int Total_Number = thisHouse->TotalEggs + Total_Larvae +thisHouse->TotalMalePupae+thisHouse->TotalFemalePupae+thisHouse->TotalFemaleNulliparousAdults+thisHouse->TotalFemaleParousAdults+thisHouse->TotalMaleAdults;
							//fprintf(stdout, "af in house %.3f\n", allele_frequencies[2*chrom][house]);
							if (Total_Number) af+=(Total_Number*allele_frequencies[2*chrom][house]);
							//fprintf(stdout, "tot: %d, al_freq: %.5f, af %.5f\n", Total_Number, allele_frequencies[2*chrom][house], af);
							//vf+=(allele_frequencies[2*chrom][house])*(allele_frequencies[2*chrom][house]);
							tf+=Total_Number;
						}
					if (tf) af/=tf; else af=0.;
					/*vf/=(width*width);
			vf-=(af*af);
			fprintf(stdout, "af %.3f\n", af);
			fprintf(stdout, "fis: %.3f\n", vf/(af*(1-af)));*/

					// for global FSTs
					aaf+=af;
					vf+=(af*af);

					// for pairwise FSTs
					freq_in_subpop[si*subpop_columns+sj]=af;

					//fprintf(stdout, "vf: %.3f\n", vf);

					//fprintf(stdout, "*af %.5f\n", af);


				}

			// for global FSTs
			aaf/=(subpop_rows*subpop_columns);
			vf/=(subpop_rows*subpop_columns);
			//fprintf(stdout, "*vf: %.3f\n", vf);
			vf-=(aaf*aaf);
			//fprintf(stdout, "*vf: %.3f\n", vf);
			fst=vf/(aaf*(1-aaf));

			// for pairwise FSTs
			for (int s1=0 ; s1<subpop_rows*subpop_columns ; s1++)
				for (int s2=0 ; s2<subpop_rows*subpop_columns ; s2++)
				{
					double p1=freq_in_subpop[s1];
					double p2=freq_in_subpop[s2];
					double pfst=((p1-p2)*(p1-p2)/4); // var(p)
					pfst/=((p1+p2)/2); // div p
					pfst/=(1-((p1+p2)/2)); // div (1-p)

					pairfsts[s1][s2]=pfst;
				}


			//fprintf(stdout, "aaf = %.3f, vf = %.3f\n", aaf, vf);
			//fprintf(stdout, "vf: %.5f - aaf: %.5f - Fst: %.9f\n", vf, aaf, fst);


			if (Date==NUMBER_OF_DAYS-1)
			{
				FILE *zt_genetic=fopen("zt_genetic.dat", "w+");
				FILE *zt_dist=fopen("zt_dist.dat", "w+");

				fprintf(zt_genetic, "%d\n", subpop_rows*subpop_columns);
				fprintf(zt_dist, "%d\n", subpop_rows*subpop_columns);

				for (int s1=0 ; s1<subpop_rows*subpop_columns ; s1++)
				{
					for (int s2=0 ; s2<s1 ; s2++)
					{
						int c1=s1%subpop_columns;
						int r1=s1/subpop_columns;
						int c2=s2%subpop_columns;
						int r2=s2/subpop_columns;
						int dist=(abs(c2-c1)+abs(r2-r1)); // valid only with solid borders !
						if (dist)
						{
							fprintf(fstbydist, "%.8f %.8f ", log((double)dist), (pairfsts[s1][s2]/(1-pairfsts[s1][s2])));
							fprintf(fstbydist, "\n");
							fprintf(zt_genetic, "%.8f ", (pairfsts[s1][s2]/(1-pairfsts[s1][s2])));
							fprintf(zt_dist, "%.8f ", log((double)dist));
						}
					}
					fprintf(zt_genetic, "\n");
					fprintf(zt_dist, "\n");
				}
				fclose(zt_genetic);
				fclose(zt_dist);
			}

			for (int s1=0 ; s1<subpop_rows*subpop_columns ; s1++)
				delete[] pairfsts[s1];
			delete[] pairfsts;
			delete[] freq_in_subpop;

		}


		fprintf(Short_Output,"%d %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g\t",Date,Total_Eggs,Total_Female_Larvae,Total_Male_Larvae,Total_Female_Pupae,Total_Male_Pupae,Total_Nulliparous_Female_Adults,Total_Parous_Female_Adults,Total_Male_Adults);
		for (int genotype=0;genotype<number_of_genotypes;genotype++) fprintf(Short_Output," %.8g",Total_Genotypes[genotype]);
		fprintf(Short_Output,"\t") ;
		for (int genotype=0;genotype<number_of_genotypes;genotype++) fprintf(Short_Output," %.8g",Total_Adult_Genotypes[genotype]);
		fprintf(Short_Output,"\t") ;
		for (int allele=0;allele<2*CHROM_NUMBER;allele++) fprintf(Short_Output," %.8g",allele_frequency[allele]);
		fprintf(Short_Output,"\t") ;
		for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
		{
			double wolbfreq = 0;
			for (int w=0 ; w<number_of_wolbachia_status ; w++)
				if ((w>>ww)%2)
					wolbfreq+=Total_wolbachia_frequency[w];
			fprintf(Short_Output, " %.8g", wolbfreq);
		}

		if (MARK_RELEASE_RECAPTURE)
		{
			for (int distance=0;distance<6;distance++) fprintf(Short_Output," %.8g",Recaptured_Females_per_distance[distance]);
			for (int distance=0;distance<6;distance++) fprintf(Short_Output," %.8g",Recaptured_Males_per_distance[distance]);
		}
		fprintf(Short_Output,"\t") ;
		fprintf(Short_Output,"  %.8g", CV_Pupae);

		if (GENETIC_CONTROL_METHOD==ReduceReplace)
			{
			fprintf(Short_Output,"  %.8g", CV_WILD_FEMALES);
			fprintf(Short_Output,"  %.8g", CV_WILD_ALLELES);
			fprintf(Short_Output,"  %.8g", NO_WILD_FEMALES);
			fprintf(Short_Output,"  %.8g", NO_WILD_ALLELES);
			}

		if (FSTATS) fprintf(Short_Output,"  %.8g", fst);
		//fprintf(Short_Output,"\n");

		// temp -- percent positive container

		//int positive=0;
		//int totalnumber=0;
		//for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
		//	for (vector<Receptacle>::iterator itContainer = itHouse->Container.begin() ; itContainer != itHouse->Container.end() ; itContainer++)
		//	{
		//		totalnumber++;
		//		if (itContainer->Eggs.size())
		//		{
		//			for (list<Eggs_Cohort>::iterator itEggs = itContainer->Eggs.begin() ; itEggs != itContainer->Eggs.end() ; itEggs++)
		//				if (itEggs->Age <7)
		//				{
		//					positive++;
		//					break;
		//				}
		//		}
		//	}
		//double percentpositive = positive / (double) totalnumber;
		//fprintf(Short_Output, " %.5g", percentpositive);


		fprintf(Short_Output,"\n");

		// log into the log per container type file (only pupae for now)
		if (OUTPUT_PER_CONTAINER_TYPE)
		{
			fprintf(Output_per_Container_Type, "%d %d ", Date, House_position);
			for (int type=0 ; type<200 ; type++) fprintf(Output_per_Container_Type, " %.0f %.0f %.0f", Census_by_container_type[0][type], Census_by_container_type[4][type], Census_by_container_type[3][type]) ;
			for (int type=0 ; type<200 ; type++) fprintf(Output_per_Container_Type, " %.6g", Census_by_container_type[5][type]) ;
			fprintf(Output_per_Container_Type, "\n") ;
		}

		if (OUTPUT_WATER_LEVEL)
		{
			for (int type=0; type<200; type++)
				if (Census_by_container_type[0][type] >0)
					fprintf(Output_Water_Level, "%d %d %.3f %.1f %.1f %.1f %.3f\n", Date, type,
							Census_by_container_type[6][type]/Census_by_container_type[0][type], // water level
							Census_by_container_type[7][type]/Census_by_container_type[0][type], // maximum water temp
							Census_by_container_type[8][type]/Census_by_container_type[0][type], // minimum water temp
							Census_by_container_type[9][type]/Census_by_container_type[0][type], // average water temp
							Census_by_container_type[10][type]/Census_by_container_type[0][type] // food
					);

		}

		// xt output

		if (EPS_YT_OUTPUT)
		{
			if (((TRADITIONAL_CONTROL_METHOD>>1)%2)&&(ADULTICIDE_REGION==1))
			{
				for (int c=0 ; c<NUMBER_OF_COLUMNS ; c++)
					fprintf(xt_adulticide, "%.3f ", transect_values[c]/(double)transect_width);
				fprintf(xt_adulticide, "\n");
			}

			if ((GENETIC_CONTROL_METHOD==7)||(NUMBER_OF_INCOMPATIBLE_WOLBACHIA))
			{
				for (int r=0 ; r<NUMBER_OF_ROWS ; r++)
					fprintf(xt_adulticide, "%.3f ", transect_values[r]);
				fprintf(xt_adulticide, "\n");
			}
		}

		// output for cumulative pupae production map

		if ((Date%365==364)&&(TEST_1))
		{
			for (int h=0 ; h<NUMBER_OF_HOUSES ; h++)
			{
				if (!(cumul_pupae_per_house[h]))
					fprintf(test1, "NaN ");
				else
					fprintf(test1, "%6d ", cumul_pupae_per_house[h]);
				if ((h%NUMBER_OF_COLUMNS)==(NUMBER_OF_COLUMNS-1))
					fprintf(test1, "NaN \n"); // add a column of zeros for heat map in MATLAB
			}
			for (int h=0 ; h<NUMBER_OF_COLUMNS ; h++)
				fprintf(test1, "NaN") ; // add a row of zeros for heat map in MATLAB
			fprintf(test1, "\n\n");
		}

		// debug

		/*

	int suspect = 1 ;
	// 1 is total food
	// 2 is average food per container
	// 3 is total surface available
	// 4 is total no. containers

	double error=0 ;

	for ( itHouse = House.begin(), House_position=0; itHouse != House.end(); itHouse++, House_position++ )
		{
			error += (((Sherlock[NUMBER_OF_HOUSES][suspect]/Sherlock[NUMBER_OF_HOUSES][0])*Sherlock[House_position][0])-Sherlock[House_position][suspect])/Sherlock[House_position][suspect] ;
			average_pupae_per_house[House_position] += Sherlock[House_position][0] ;
		}
	 //fprintf(stdout, "Suspect %d: error = %.5f\n", suspect, error) ;
	 //fprintf(stdout, "Sherlock[House 0] : %.0lf %.3lf %.3lf %.3lf %.0lf\n", Sherlock[0][0], Sherlock[0][1], Sherlock[0][2], Sherlock[0][3], Sherlock[0][4]) ;
		 */


		// end of debug


		fprintf(stdout, "Day %d done ", Date) ;
		if (!Total_Mosquitoes) fprintf(stdout, "*") ; // indicator of population extinction
		fprintf(stdout, "\n") ;

	} //this is the end of the day loop

	//spatial statistics
	if (SPATIAL_STATS)
	{
		fprintf(stdout, "Spatial statistics calculations ...") ;
		Img = SpatialStats() ;
		fprintf(stdout, " done.\n");
	}


	//close log files
	if (DETAILED_LOG) fclose(Output);
	if (LOG_PER_HOUSE) fclose(Output_per_House);
	fclose(Short_Output);
	if (OUTPUT_PER_CONTAINER_TYPE) fclose(Output_per_Container_Type) ;
	if (OUTPUT_WATER_LEVEL) fclose(Output_Water_Level);

	if (TEST_1) fclose(test1);
	if (TEST_2) fclose(test2);
	if (GENTIME) fclose(gentime);
	if (LARVTIME) fclose(larvtime);
	if (OUTPUT_FOR_SBEED) fclose(forSbeed);
	if (OUTPUT_PER_CONTAINER) fclose(Output_per_Container);

	if (FSTATS) fclose(fstbydist);


	if (EPS_YT_OUTPUT && ((((TRADITIONAL_CONTROL_METHOD>>1)%2)&&(ADULTICIDE_REGION==1))||(GENETIC_CONTROL_METHOD==7)||(NUMBER_OF_INCOMPATIBLE_WOLBACHIA)))
		fclose(xt_adulticide);

	PrintOutput(Img);

	if (SAVE_FINAL_STATE_AS_SETUP_FILE) SaveHouseSetupOut();

	if (ZIPPED_OUTPUT) CompressOutputs();

	return 0;
}//this is the end of the main function












