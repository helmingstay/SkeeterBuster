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

//#define USE_GOMP 1

extern FILE *Short_Output, *Output, *Output_per_House, *Output_per_Container_Type, *Output_per_Container, *Output_Water_Level, *EPS_Output, *EPS_Wolbachia_Output, *EPS_Density_Output, *EPS_XT_Output;
extern FILE *test1, *test2, *forSbeed, *gentime, *xt_adulticide;
extern double CV_Pupae,Total_Eggs,Total_Male_Larvae,Total_Female_Larvae,Total_Male_Pupae,Total_Female_Pupae,Total_Nulliparous_Female_Adults,Total_Parous_Female_Adults,Total_Male_Adults, CV_WILD_FEMALES, CV_WILD_ALLELES, NO_WILD_FEMALES, NO_WILD_ALLELES;
extern double *allele_frequency;
extern double **allele_frequencies;
extern int *cumul_pupae_per_house;
extern int Number_of_Columns, Number_of_Rows;
extern int House_position,Date;
extern double Census_by_container_type[11][200] ;

extern double **Sherlock ;
// 0 is reference : pupae production
// 1 is total food
// 2 is average food per container
// 3 is total surface available
// 4 is total no. containers
// 5 is nulliparous females dead
// 6 is total females dead

extern vector<Building> House;
extern list<int> edges;

extern vector<double> Total_Genotypes;
extern vector<double> Total_Adult_Genotypes;
extern vector<double> Total_wolbachia_frequency;


extern long ra[M+1], nd;

extern int xt_selected_line ;
extern int transect_width;
extern double *transect_values;
extern double *transect_densities;
extern int *mask, *divisor ;
extern int number_of_genotypes ;
extern int number_of_medea_masks ;
extern double gen_prob ;

int bc_Date,bc_House_position;
double bc_Development_26,bc_Development_134;

void Building::SetBuildingCalculationParameters(int Date, double Development_26, double Development_134, int House_position)
{
	bc_Date = Date;
	bc_House_position = House_position;
	bc_Development_26 = Development_26;
	bc_Development_134 = Development_134;
}

void Building::BuildingCalculations()
{
	BuildingCalculations(bc_Date, bc_Development_26, bc_Development_134, bc_House_position);
}

void Building::BuildingCalculations(int Date, double Development_26, double Development_134, int House_position)
{
	double Air_Average,Air_Maximum,Air_Minimum;
	double RH,Rain,SD;
	double EggInputLevel;
	double ContainerSum; //variables used when chosing the container for oviposition: sum of density*(ln(volume)+1) for all containers, number of randomly chosen container in the stochastic version, counter used in the stochastic version
	double TopContainerLevel;
	double TopEggInputLevel;
	double Standard_Deviation_for_fecundity;
	double Variance_for_fecundity;
	double Egg_Number;
	double sumeggs_laid=0;
	int Number_of_Suitable_Containers = 0;

	//local variables of the house are transferred to global variables that can be passed on to functions
	Air_Minimum=this->Minimum_Daily_Air_Temperature[Date];
	Air_Maximum=this->Maximum_Daily_Air_Temperature[Date];
	Air_Average=this->Average_Daily_Air_Temperature[Date];
	RH=this->Relative_Humidity[Date];
	Rain=this->Precipitation[Date];
	SD=this->Saturation_Deficit[Date];
	//clearing of some house variables that are recalculated each day

	this->Total_NewAdultNumber[MALE]=0.0;
	this->Total_NewAdultNumber[FEMALE]=0.0;
	this->Total_NewAdultWeight[MALE]=0.0;
	this->Total_NewAdultWeight[FEMALE]=0.0;

	//this is the start of the container loop
	/*
		#ifdef USE_GOMP

		for (int j=0;j<this->Number_of_Containers;j++)
		{
		this->Container[j].ReceptacleCalculations(Date,SD,RH,Rain,Air_Maximum,Air_Minimum,Air_Average,Development_26,Development_134,this); // Also need a pointer to House...
		}
		#else
	 */
	for (int j=0;j<this->Number_of_Containers;j++)
	{

		if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END))
		{
			fprintf(Output,"%s %d","Container ",j);
		}

		this->Container[j].ReceptacleCalculations(Date,SD,RH,Rain,Air_Maximum,Air_Minimum,Air_Average,Development_26,Development_134,this); // Also need a pointer to House...
	}

	//#endif
	//this is the end of the container cycle


	//ORIGINAL CIMSIM //the below code creates one female/male cohort for all the pupae that emerge on the given day with an average weight

	//this part establishes the new female adults into a new female adult cohort, and marks it as nulliparous //#17

	if (this->Total_NewAdultNumber[FEMALE]>0)
		{
		if (SIMULATION_MODE1==CIMSIM)
			{
				this->Daily_Average_Female_Adult_Weight=this->Total_NewAdultWeight[FEMALE]/this->Total_NewAdultNumber[FEMALE]; //calculates the average weight of the female adult cohort


				double number = this->Total_NewAdultNumber[FEMALE];
				double avgWeight = this->Daily_Average_Female_Adult_Weight*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT;
				double fecundity = avgWeight * FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT;

				// create new female adult cohort as nulliparous, with specified number, weight and fecundit
				for (int f=0 ; f<number ; f++)
				{
					Female_Adult new_female_adult( NULLIPAROUS, avgWeight, fecundity, 0, 0, 0, 0, 0, 0, 0, false, 0, 0 );
					// age_since_hatch cannot be calculated here, average age would have to be calculated
					this->Female_Adults.push_back(new_female_adult);
				}

			}
		else
			{
				this->Daily_Average_Female_Adult_Weight=0;
			}
		}


	//this establishes the new adult males to their adult male cohort //#18
	if (this->Total_NewAdultNumber[MALE]>0)
	{
		if (SIMULATION_MODE1==CIMSIM)
		{
			this->Total_NewAdultWeight[MALE]=this->Total_NewAdultWeight[MALE]/this->Total_NewAdultNumber[MALE]; //this calculates the average weight of the new adult males
			//this locates the first empty adult male cohort




			Male_Adult_Cohort new_male_adult_cohort( this->Total_NewAdultNumber[MALE], this->Total_NewAdultWeight[MALE]*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT,1,0,0,0,0,0,0);
			// age_since_hatch cannot be calculated here, average age would have to be calculated
			this->Male_Adult.push_back(new_male_adult_cohort);

		}
		else
		{
			this->Total_NewAdultWeight[MALE]=0;
		}
	}

//+
	//this part adds up the container indeces that determine choice of oviposition container by size


	ContainerSum=0;
	int Number_of_Containers=this->Number_of_Containers; //why?
	double *container_probabilities = new double[Number_of_Containers];
	int *eggs_laid_in_each;
	if (SIMULATION_MODE2==STOCHASTIC) eggs_laid_in_each = new int[Number_of_Containers];

	for (int k=0 ; k<Number_of_Containers ; k++)
	{
		eggs_laid_in_each[k]=0;
		container_probabilities[k]=0.;
	}

	Number_of_Suitable_Containers = 0;
	if (Number_of_Containers)
	{
		for (int j=0;j<Number_of_Containers;j++)
		{
			if (this->Container[j].Container_Type_Number!=102)
				// skip specific container types (see ctypes.txt)
			{
				*(container_probabilities+j) = log(1+((this->Container[j].Surface)*(this->Container[j].Height))); // combine food and suface (food/surface) ? (food/volume) ?
				if (this->Container[j].Covered==1) *(container_probabilities+j)*=OVIPOSITION_REDUCTION_IN_COVERED_CONTAINERS ; // reduced oviposition in covered containers

				Number_of_Suitable_Containers++;

				// strong reduction in probabilities if eggs are present
				list<Eggs_Cohort>::size_type total_number_of_egg_cohorts = this->Container[j].Eggs.size() ;
				if (total_number_of_egg_cohorts)
					{
					*(container_probabilities+j)*=0.1;
					}
				// prototype to enhance probability if larvae present
				/*
				list<Larvae_Cohort>::size_type total_number_of_larvae_cohorts = this->Container[j].Larvae.size() ;
				if (total_number_of_larvae_cohorts)
					{
					double temporary_container_probability = 0;
					temporary_container_probability = *(container_probabilities+j);
					temporary_container_probability = temporary_container_probability * INCREASED_LARVAL_EFFECT;
					*(container_probabilities+j) = temporary_container_probability;
					}
				 */
				ContainerSum += *(container_probabilities+j);

			}
		}

		// Now normalize the container probabilities
		if (ContainerSum>0)

			for (int j=0;j<Number_of_Containers;j++)
			{
				*(container_probabilities+j) /= ContainerSum ;
			}
		else

			for (int j=0;j<Number_of_Containers;j++)
			{
				*(container_probabilities+j)=1./Number_of_Containers ;
				//if (!Number_of_Containers) *(container_probabilities+j)=0;
			}


	}

	//ORIGINAL CIMSIM //to calculate the moving average of newly emerged female weights for fecundity
	//this part calculates daily average female fecundity, based on the CIMSIM code //#19
	//new average female nulliparous weight only added if new adults have emerged

	if (this->Daily_Average_Female_Adult_Weight>0)
	{
		//move 4 earlier average female adult weights of emerging females, discard the 5th, and make place for the average female adult weight of the newly emerged nulliparous females
		int x=4;
		do
		{
			if (this->Average_New_Female_Adult_Weight[x-1]>0) this->Average_New_Female_Adult_Weight[x]=this->Average_New_Female_Adult_Weight[x-1];
			x--;
		}
		while (x>0);
		//insert the average female adult weight of the newly emerged females into the empty slot
		this->Average_New_Female_Adult_Weight[0]=this->Daily_Average_Female_Adult_Weight*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT;
	}

	this->Daily_Average_Female_Adult_Weight=0;
	//calculate an average female adult weight by calculating a moving average of the average female asult weight of the last max. five newly emerged nulliparous female adult cohorts
	int x=0;

	for (int j=0;j<5;j++)
	{
		this->Daily_Average_Female_Adult_Weight+=this->Average_New_Female_Adult_Weight[j];
		if (this->Average_New_Female_Adult_Weight[j]>0) x++;
	}
	if (x>0) this->Daily_Average_Female_Adult_Weight=this->Daily_Average_Female_Adult_Weight/x; else this->Daily_Average_Female_Adult_Weight=0;
	//calculate daily average female fecundity
	this->Daily_Average_Female_Fecundity=this->Daily_Average_Female_Adult_Weight*FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT;


	//clear total number of newly parous and female nulliparous adults

	this->TotalNewlyParous=0;
	this->TotalFemaleNulliparousAdults=0;
	this->TotalOvipositing=0;

	//sum the number of female nulliparous adults
	for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin(); female_adult_it != this->Female_Adults.end(); female_adult_it++)
	{
		if (female_adult_it->Nulliparous==NULLIPAROUS)
		{
			this->TotalFemaleNulliparousAdults++;
		}
	}

	//ORIGINAL CIMSIM

	if (SIMULATION_MODE1==CIMSIM) if (this->TotalFemaleNulliparousAdults==0) this->Daily_Average_Female_Fecundity=0.5*this->Daily_Average_Female_Fecundity;

	if (SIMULATION_MODE1==CIMSIM) if (Date==57) this->Daily_Average_Female_Fecundity=0.5*this->Daily_Average_Female_Fecundity;

	//this is the start of the calculations for female adult cohorts //#19,20,21,22


	// make a set of new egg cohorts, one for each container
	// go through adult cohorts, figure out where each lay they eggs
	// then, later on, add these new egg cohorts to the containers

	map<int,Eggs_Cohort> containers_new_eggs_cohort;
	map<int,Eggs_Cohort>::iterator itNewEggCohort;
	int number_of_genotypes;

	number_of_genotypes=(int) pow((double)2,2*CHROM_NUMBER);

	int number_of_wolbachia_status;
	if (!NUMBER_OF_INCOMPATIBLE_WOLBACHIA) number_of_wolbachia_status = 1; // just making sure that 2^0=1
	else number_of_wolbachia_status = (int) pow((double)2,NUMBER_OF_INCOMPATIBLE_WOLBACHIA);



	//containers_new_eggs_cohort.assign(Number_of_Containers*number_of_genotypes*number_of_medea_masks*number_of_wolbachia_status,Eggs_Cohort() );
	// all the Ages, Numbers, Developmental_Percent etc of what will become the new egg cohorts are zeroed by the above line

	this->Recaptured_Female=this->Recaptured_Male=0;

	// need to initialize Medea_Mom array within each possible cohort
	// not sure this is the best way to go, we could probably spare a lot of those ...
	/*for (int cohort=0 ; cohort<Number_of_Containers*number_of_genotypes ; cohort++)
				for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
				containers_new_eggs_cohort[cohort].Medea_Mom[mom_type]=0 ;
	 */

	for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin(); female_adult_it != this->Female_Adults.end(); ) // do female_adult_it++ at the end of the loop
	{

		double daily_average_female_fecundity_fudge_factor=1;

		bool is_female_nulliparous=female_adult_it->Nulliparous;


		int NewlyParousOvipositing= (int) female_adult_it->Female_Adult_Calculations(Air_Maximum,Air_Minimum,Air_Average,SD, Number_of_Suitable_Containers);

		if (NewlyParousOvipositing==-99) //female is dead, calculate generation time
		{
			female_adult_it=this->Female_Adults.erase(female_adult_it); //pointer is placed to whichever female replaces the erased one
			continue;
		}

		//if eggs are laid //#23
		if (NewlyParousOvipositing>0)
		{

			bool trapped=false;


			if (SIMULATION_MODE1==SKEETER_BUSTER) this->Daily_Average_Female_Fecundity=female_adult_it->Fecundity;

			// determine a binary mask describing mom's MEDEA elements
			int mom_had_medeas = 0;
			if (GENETIC_CONTROL_METHOD==MEDEA)
				for (int chrom=1 ; chrom<MEDEA_UNITS+1 ; chrom++)
				{
					int medea_mask=3u<<(2*chrom) ;
					if (female_adult_it->Genotype&medea_mask)
					{
						mom_had_medeas+=(1u<<(chrom-1)) ;
						if ((((female_adult_it->Genotype&medea_mask)>>(2*chrom))%4)==3) // homozygous
							this->Daily_Average_Female_Fecundity*=(1-MEDEA_MATERNAL_FECUNDITY_LOSS)*(1-MEDEA_MATERNAL_FECUNDITY_LOSS);
						else // heterozygous
							this->Daily_Average_Female_Fecundity*=pow((double)(1-MEDEA_MATERNAL_FECUNDITY_LOSS),(2*MEDEA_MATERNAL_FECUNDITY_LOSS_DOMINANCE));
					}
				}

			// Wolbachia fecundity cost and CI

			double non_wolbachia_eggs = 0;
			// because of imperfect vertical transmission, some eggs will not receive wolbachia
			// these WILL be subject to CI even if the mother is infected (based on Turelli & Hoffman, 1999)

			if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
			{
				int number_of_infections=0;
				for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
					if (((female_adult_it->Wolbachia)>>ww)%2)
						number_of_infections++;
				this->Daily_Average_Female_Fecundity*=pow((double)(1-WOLBACHIA_INFECTED_FEMALE_FECUNDITY_LOSS),number_of_infections);


				int number_of_incompatibilities = 0;
				for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
					if ((((female_adult_it->Wolbachia)>>ww)%2)==0) // uninfected female
					if (((female_adult_it->Male_Wolbachia)>>ww)%2) // infected male
						number_of_incompatibilities++;
				this->Daily_Average_Female_Fecundity*=pow((double)(1-WOLBACHIA_LEVEL_OF_INCOMPATIBILITY),number_of_incompatibilities);
			}

			int (*female_gamete_type)[2]= new int [CHROM_NUMBER][2]; //female gametes per chromosome
			int (*male_gamete_type)[2]= new int [CHROM_NUMBER][2]; //male gametes per chromosome
			int offspring_genotype;
			int (*offspring_chrom_genotype)[4]= new int [CHROM_NUMBER][4]; //offspring genotype individual chromose per Punett square



			if (SIMULATION_MODE2==STOCHASTIC)
			{


				//builds up distribution of available males

				double *offspring_genotype_probabilities = new double [number_of_genotypes];
				int *eggs_with_genotype= new int[number_of_genotypes];



				//applying variation in fecundity for individual females laying eggs
				Standard_Deviation_for_fecundity=this->Daily_Average_Female_Fecundity*SKEETER_BUSTER_FECUNDITY_RATIO_STANDARD_DEVIATION_MEAN;
				Variance_for_fecundity=Standard_Deviation_for_fecundity*Standard_Deviation_for_fecundity;



				for (int l=0;l<NewlyParousOvipositing;l++)

					/************************
									// change here: this for loop should be unnecessary (check)
									/*************************/
				{




					for (int genotype=0;genotype<number_of_genotypes;genotype++) eggs_with_genotype[genotype]=0;

					for (int genotype=0;genotype<number_of_genotypes;genotype++) offspring_genotype_probabilities[genotype]=0;

					Egg_Number=ROUND_(Normal_Deviate(this->Daily_Average_Female_Fecundity,Standard_Deviation_for_fecundity));
					if (Egg_Number<0) Egg_Number=0 ;

					double *Egg_Batch_Wolbachia_Type = new double[number_of_wolbachia_status];
					for (int w=0 ; w<number_of_wolbachia_status ; w++)
						Egg_Batch_Wolbachia_Type[w]=0;

					double *Egg_Batch_Wolbachia_Probas = new double[number_of_wolbachia_status];
					for (int w=0 ; w<number_of_wolbachia_status ; w++)
						Egg_Batch_Wolbachia_Probas[w]=1.;

					if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
					{
						// calculate here (1) imperfect transmission, then (2) CI

					// (1) imperfect transmission

						// first populate the table with probabilities

						for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
							for (int w=0 ; w<number_of_wolbachia_status ; w++)
							{
								if (((female_adult_it->Wolbachia)>>ww)%2) // female infected
								{
									if ((w>>ww)%2) // corresponds to an infected egg type
										Egg_Batch_Wolbachia_Probas[w]*=WOLBACHIA_MATERNAL_TRANSMISSION;
									else
										Egg_Batch_Wolbachia_Probas[w]*=(1-WOLBACHIA_MATERNAL_TRANSMISSION);
								}
								else // female non infected
								{
									if ((w>>ww)%2) // corresponds to an infected egg type
										Egg_Batch_Wolbachia_Probas[w]=0.;
								}
							}

						//fprintf(stdout, "Mom_Type : %d ; Egg_Number : %.3f ; Probas %.3f %.3f\n", female_adult_it->Wolbachia, Egg_Number, Egg_Batch_Wolbachia_Probas[0],Egg_Batch_Wolbachia_Probas[1] );


						// then distribute the egge batch in their respective types

						//genmul(Egg_Number, Egg_Batch_Wolbachia_Probas, number_of_wolbachia_status, Egg_Batch_Wolbachia_Type);

						for (int w=0 ; w<number_of_wolbachia_status ; w++)
							Egg_Batch_Wolbachia_Type[w] = (int) Egg_Number*(Egg_Batch_Wolbachia_Probas[w]);

						//fprintf(stdout, "Egg_Number : %.3f ; Probas %.3f %.3f\n", Egg_Number, Egg_Batch_Wolbachia_Type[0],Egg_Batch_Wolbachia_Type[1] );

						// (2) CI applied to different egg types

						for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
							for (int w=0 ; w<number_of_wolbachia_status ; w++)
								if (((w>>ww)%2)==0) // uninfected egg type
									if (((female_adult_it->Male_Wolbachia)>>ww)%2) // infected male
										//Egg_Batch_Wolbachia_Type[w] = (int) Binomial_Deviate(Egg_Batch_Wolbachia_Type[w], (1-WOLBACHIA_LEVEL_OF_INCOMPATIBILITY));
										Egg_Batch_Wolbachia_Type[w]*=(1-WOLBACHIA_LEVEL_OF_INCOMPATIBILITY);

				//	fprintf(stdout, "Egg_Number : %.3f ; Probas %.3f %.3f\n", Egg_Number, Egg_Batch_Wolbachia_Type[0],Egg_Batch_Wolbachia_Type[1] );

						// calculate new number of eggs

						Egg_Number = 0;
						for (int w=0 ; w<number_of_wolbachia_status ; w++)
							Egg_Number+=Egg_Batch_Wolbachia_Type[w];
					}

					if (Egg_Number<0) Egg_Number=0 ; // should be obsolete

					// store this female's oviposition event
					Oviposition this_event;
					this_event.Date = Date;
					this_event.Age_since_hatch = female_adult_it->Age_since_hatch;
					this_event.Number = Egg_Number;
					this_event.House_Position = this->Position;
					female_adult_it->Oviposition_Events.push_back(this_event);

					int dx = ((this_event.House_Position)%NUMBER_OF_COLUMNS) - ((female_adult_it->Emergence_Place)%NUMBER_OF_COLUMNS);
					dx=(dx<0?-dx:dx);
					int dy = ((this_event.House_Position)/NUMBER_OF_COLUMNS) - ((female_adult_it->Emergence_Place)/NUMBER_OF_COLUMNS);
					dy=(dy<0?-dy:dy);
					int distance = dx+dy;


					//fprintf(gentime, "%4d\t%4d\t%.0f\t%4d\t%4d\t%3d\t%.0f\n", Date, female_adult_it->ID, this_event.Age_since_hatch, female_adult_it->Emergence_Place, this_event.House_Position, distance, this_event.Number);
					//for (int e=0 ; e < this_event.Number ; e++) fprintf(gentime, "%d\n", distance);

					if (OVIPOSIT_EGGS_INDEPENDENTLY)
						{
						genmul( (int) Egg_Number , container_probabilities, Number_of_Containers , eggs_laid_in_each );
						}
					//Multinomial_Deviate( (int) Egg_Number, container_probabilities, Number_of_Containers, eggs_laid_in_each);

					if (!OVIPOSIT_EGGS_INDEPENDENTLY) // If females are to partition their eggs and lay it in one container
						{
					/* reset container probabilities per female; might be redundant */
						ContainerSum=0;
						for (int k=0 ; k<Number_of_Containers ; k++)
							{
							container_probabilities[k]=0.;
							}

						Number_of_Suitable_Containers = 0;
						if (Number_of_Containers)
							{
							for (int j=0;j<Number_of_Containers;j++)
								{
								if (this->Container[j].Container_Type_Number!=102)
								// skip specific container types (see ctypes.txt)
									{
									*(container_probabilities+j) = log(1+((this->Container[j].Surface)*(this->Container[j].Height))); // combine food and suface (food/surface) ? (food/volume) ?
									if (this->Container[j].Covered==1) *(container_probabilities+j)*=OVIPOSITION_REDUCTION_IN_COVERED_CONTAINERS ; // reduced oviposition in covered containers

									Number_of_Suitable_Containers++;

									// strong reduction in probabilities if eggs are present
									list<Eggs_Cohort>::size_type total_number_of_egg_cohorts = this->Container[j].Eggs.size() ;
									if (total_number_of_egg_cohorts)
											{
											*(container_probabilities+j)*=0.1;
											}

									//  to enhance probability if larvae present

									list<Larvae_Cohort>::size_type total_number_of_larvae_cohorts = this->Container[j].Larvae.size() ;
									if (total_number_of_larvae_cohorts)
										{
										double temporary_container_probability = 0;
										temporary_container_probability = *(container_probabilities+j);
										temporary_container_probability = temporary_container_probability * LARVAL_EFFECT_ON_OVIPOSITION_PROBABILITY;
										*(container_probabilities+j) = temporary_container_probability;
										}
									   ContainerSum += *(container_probabilities+j);
									   }
									}

									// Now normalize the container probabilities
									if (ContainerSum>0)
											{
											for (int j=0;j<Number_of_Containers;j++)
												{
												*(container_probabilities+j) /= ContainerSum ;
												}
											}
										else
											{
											for (int j=0;j<Number_of_Containers;j++)
												{
												*(container_probabilities+j)=1./Number_of_Containers ;
															//if (!Number_of_Containers) *(container_probabilities+j)=0;
												}
											}
								}

						/* The idea here is eggs are partitioned into some number of partitions. Each partition gets laid in a container as if it were multinomial */

						int partition_clutches = 0;
						partition_clutches = Binomial_Deviate(1,PROBABILITY_CLUTCH_PARTITIONED);

						/* if the clutch is going to be partitioned */
						if (partition_clutches > 0)
							{
							/* This annoying step is required because the previous implementation when eggs are laid independently relied on the multinomial generator to reset eggs_laid_in_each to zero. Because genmul gets called multiple times for each female now, however, we cannot reset the eggs_laid_in_each array values to zero */
							for (int k=0; k < Number_of_Containers; k++)
								{
								eggs_laid_in_each[k] = 0;
								}

							int number_of_partitions = 0;
							if (Number_of_Containers < MAXIMUM_NUMBER_OF_EGG_PARTITIONS)
								{
								number_of_partitions = Number_of_Containers;
								}
							else
								{
								number_of_partitions = MAXIMUM_NUMBER_OF_EGG_PARTITIONS;
								}

							//printf("No. partitions %d\n", number_of_partitions);
							double *partition_probabilities;
							partition_probabilities = new double[number_of_partitions];
							int *eggs_in_partitions;
							eggs_in_partitions = new int[number_of_partitions];


						for (int k=0 ; k<number_of_partitions ; k++)
								{
									eggs_in_partitions[k]=0;
								}


							partition_probabilities[0] = PROPORTION_IN_PARTITION_0;
							if (number_of_partitions == 1)
								{
								partition_probabilities[0] = 1;
								}
							if (number_of_partitions > 1)
								{
								partition_probabilities[1] = PROPORTION_IN_PARTITION_1;
								if (number_of_partitions == 2)
									{
									partition_probabilities[1]=(1-partition_probabilities[0]);
									}
								}
							if (number_of_partitions > 2)
								{
								partition_probabilities[2] = PROPORTION_IN_PARTITION_2;
								if (number_of_partitions == 3)
									{
									partition_probabilities[2] = (1-(partition_probabilities[0] + partition_probabilities[1]));
									}
								}
							if (number_of_partitions > 3)
								{
								partition_probabilities[3] = PROPORTION_IN_PARTITION_3;
								if (number_of_partitions == 4)
									{
									partition_probabilities[3] = (1-(partition_probabilities[0] + partition_probabilities[1] + partition_probabilities[2]));
									}
								}
							if (number_of_partitions > 4)
								{
								partition_probabilities[4] = PROPORTION_IN_PARTITION_4;
								if (number_of_partitions == 5)
										{
										partition_probabilities[4] = (1-(partition_probabilities[0] + partition_probabilities[1] + partition_probabilities[2] + partition_probabilities[3]));
										}
								}
							if (number_of_partitions > 5)
								{
								partition_probabilities[5] = PROPORTION_IN_PARTITION_5;
								if (number_of_partitions == 6)
									{
									partition_probabilities[5] = (1-(partition_probabilities[0] + partition_probabilities[1] + partition_probabilities[2] + partition_probabilities[3] + partition_probabilities[4]));
									}
								}
							if (number_of_partitions > 6)
								{
								partition_probabilities[6] = PROPORTION_IN_PARTITION_6;
								}
							if (number_of_partitions > 7)
								{
								partition_probabilities[7] = PROPORTION_IN_PARTITION_7;
								}
							if (number_of_partitions > 8)
								{
								partition_probabilities[8] = PROPORTION_IN_PARTITION_8;
								}
							if (number_of_partitions == 9)
								{
								partition_probabilities[9] = PROPORTION_IN_PARTITION_9;
								}

							genmul(Egg_Number , partition_probabilities, number_of_partitions , eggs_in_partitions );

							for (int egg_partition = 0; egg_partition < number_of_partitions;  egg_partition++)
								{
								//printf("Eggs in partition %d %d\n", (int) Egg_Number, eggs_in_partitions[egg_partition]);

								int *destination_container;
								destination_container = new int[Number_of_Containers];
								for (int k=0; k < Number_of_Containers; k++)
									{
									destination_container[k] = 0;
									}

								genmul( 1 , container_probabilities, Number_of_Containers , destination_container );

								for (int k=0; k < Number_of_Containers; k++)
										{
										if (destination_container[k] == 1)
											{
											int eggs_before = eggs_laid_in_each[k];
											eggs_laid_in_each[k] += (int) eggs_in_partitions[egg_partition]; // Lay all the eggs in the partition in the container such that eggs_laid_in_each = 1
											//printf("Eggs laid in container %d %d %d: %d %d\n", k, this->Container[k].Larvae.size(), eggs_before,  eggs_laid_in_each[k], (int) eggs_in_partitions[egg_partition]);
											}
										}
								delete[] destination_container;
								}
							delete[] partition_probabilities;
							//if(eggs_laid_in_each==NULL) fprintf(stdout, "NULL eggs_laid_in_each\n") ;
							delete[] eggs_in_partitions;
							}

						// If the female will lay her entire clutch in a single container
						if (partition_clutches == 0)
							{
							genmul( 1 , container_probabilities, Number_of_Containers , eggs_laid_in_each ); // Determine which container gets the eggs
							for (int k=0; k < Number_of_Containers; k++)
								{
								if (eggs_laid_in_each[k] == 1)
									{
									eggs_laid_in_each[k] = (int) Egg_Number; // Lay all the eggs in the container such that eggs_laid_in_each = 1
									}
								}
							}
						}/* end if OVIPOSIT_EGGS_INDEPENDENTLY == 0 */

					//for (int k=0 ; k<Number_of_Containers ; k++) eggs_laid_in_each[k]=10;

					// case of oviposition traps

					for (int k=0 ; k<Number_of_Containers ; k++)
					{
						//fprintf(stdout, "before: %d (k=%d) - ", eggs_laid_in_each[k], k);
						if ((this->Container[k].Container_Type_Number)==101)
							if((eggs_laid_in_each[k])>0)
							{
								eggs_laid_in_each[k]=0;
								if (Random_Number()<OVITRAP_FEMALE_KILLING_EFFICIENCY)
									trapped=true;
							}
						//fprintf(stdout, "after: %d (k=%d)\n", eggs_laid_in_each[k], k);
					}



					//choosing male

					// int male_genotype = female_adult_it->Sperm_Genotypes[l] ;
					// female_adult_it->Sperm_Genotypes.push_back(male_genotypes[male_cohort_number]) ;


					int male_genotype=female_adult_it->Male_Genotype;

					// tmp
					// fprintf(stdout, "Female with genotype %d mates with male genotype %d\n", female_adult_it->Genotype, male_genotype);

					//this part calculates the possible female and male gametes on each chromosome

					if (GENETIC_CONTROL_METHOD!=MEIOTIC_DRIVE)
					{
						int a=1;
						int b=2;

						// we will store female gamete types in the more
						// significant bit  (i.e. in the places occupied by
						// b), so we multiply by 2 when we look at a for female

						// for male, we divide by 2 when we look at b

						for (int chrom=0;chrom<CHROM_NUMBER;chrom++)
						{
							female_gamete_type[chrom][0]=(female_adult_it->Genotype & a)*2;
							female_gamete_type[chrom][1]=(female_adult_it->Genotype & b);

							male_gamete_type[chrom][0]=(male_genotype & a);
							male_gamete_type[chrom][1]=(male_genotype & b)/2;

							offspring_chrom_genotype[chrom][0]=female_gamete_type[chrom][0]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][1]=female_gamete_type[chrom][0]+male_gamete_type[chrom][1];
							offspring_chrom_genotype[chrom][2]=female_gamete_type[chrom][1]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][3]=female_gamete_type[chrom][1]+male_gamete_type[chrom][1];


							a=a*4;
							b=b*4;
						}
					}
					else
					{
						female_gamete_type[0][0]=0;
						female_gamete_type[0][1]=0;

						male_gamete_type[0][0]=0;
						male_gamete_type[0][1]=1;

						female_gamete_type[1][0]=(female_adult_it->Genotype & 48)/16;
						female_gamete_type[1][1]=(female_adult_it->Genotype & 12)/4;

						male_gamete_type[1][0]=(male_genotype & 48)/16;
						male_gamete_type[1][1]=(male_genotype & 12)/4;

						if (MD_TYPE==3)
						{
							female_gamete_type[2][0]=(female_adult_it->Genotype & 128)/128;
							female_gamete_type[2][1]=(female_adult_it->Genotype & 64)/64;

							male_gamete_type[2][0]=(male_genotype & 128)/128;
							male_gamete_type[2][1]=(male_genotype & 64)/64;
						}

						offspring_chrom_genotype[0][0]=0;
						offspring_chrom_genotype[0][1]=1;
						offspring_chrom_genotype[0][2]=0;
						offspring_chrom_genotype[0][3]=1;

						offspring_chrom_genotype[1][0]=female_gamete_type[1][0]*4+male_gamete_type[1][0];
						offspring_chrom_genotype[1][1]=female_gamete_type[1][0]*4+male_gamete_type[1][1];
						offspring_chrom_genotype[1][2]=female_gamete_type[1][1]*4+male_gamete_type[1][0];
						offspring_chrom_genotype[1][3]=female_gamete_type[1][1]*4+male_gamete_type[1][1];

						if (MD_TYPE==3)
						{
							offspring_chrom_genotype[2][0]=female_gamete_type[2][0]*2+male_gamete_type[2][0];
							offspring_chrom_genotype[2][1]=female_gamete_type[2][0]*2+male_gamete_type[2][1];
							offspring_chrom_genotype[2][2]=female_gamete_type[2][1]*2+male_gamete_type[2][0];
							offspring_chrom_genotype[2][3]=female_gamete_type[2][1]*2+male_gamete_type[2][1];
						}

					}

					//calculate offspring genotype probabilities
					if (REPRODUCTION_MODE==CALCULATION)
					{


						//this combines the offspring genotypes per chromosome into the actual offspring chromosome
						if (GENETIC_CONTROL_METHOD!=MEIOTIC_DRIVE)
						{
							for (int punett=0;punett<(int) pow((double)4,CHROM_NUMBER);punett++)
							{

								offspring_genotype=0;

								for (int chrom=0;chrom<CHROM_NUMBER;chrom++)
								{
									offspring_genotype+=offspring_chrom_genotype[chrom][ (punett & mask[chrom])/divisor[chrom] ];
								}
								offspring_genotype_probabilities[offspring_genotype]+=gen_prob;


							}
							//fprintf(stdout, "P(0) = %.3f - P(1) = %.3f -- P(2) = %.3f\n", offspring_genotype_probabilities[0], offspring_genotype_probabilities[1], offspring_genotype_probabilities[2]);
						}
						else
						{
							for (int punett1=0;punett1<3;punett1+=2)
							{
								for (int punett2=0;punett2<3;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (MD_TYPE==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=DRIVE_STRENGTH_SN;
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=DRIVE_STRENGTH_IT;
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=DRIVE_STRENGTH_IN;
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=DRIVE_STRENGTH_MN;
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_probabilities[offspring_genotype]+=0.125*0.25;
									}
								}
							}
							for (int punett1=1;punett1<4;punett1+=2)
							{
								for (int punett2=1;punett2<4;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (MD_TYPE==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=DRIVE_STRENGTH_SN;
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=DRIVE_STRENGTH_IT;
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=DRIVE_STRENGTH_IN;
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=DRIVE_STRENGTH_MN;
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_probabilities[offspring_genotype]+=0.125*0.25;
									}
								}
							}
						}

						//probabilities adjustments in case of embryonic fitness costs with soft selection

						if ((FITNESS_COST_STAGE==0)&&(EMBRYONIC_COST_SELECTION_TYPE==SOFT))
						{

							double total_probabilities=0.;
							// modify probabilities according to each genotype specific fitness
							for (int genotype=0 ; genotype<number_of_genotypes ; genotype++)
							{
								offspring_genotype_probabilities[genotype]*=Fitness_Calculation(genotype);
								total_probabilities+=offspring_genotype_probabilities[genotype];
							}

							// renormalize probabilities
							for (int genotype=0 ; genotype<number_of_genotypes ; genotype++)
								offspring_genotype_probabilities[genotype]/=total_probabilities;

						}



						for (int k=0;k<(Number_of_Containers); k++)
						{
							// changes here for individualized egg cohorts by container, by genotype AND by mom genotype
							// -> genmul stays, but either (1) change the structure of containers_new_eggs_cohort
							//    or (2) change the procedure, and push_back the cohorts somewhere here (layers?)
							//    (probably .2.?)
							genmul ( *(eggs_laid_in_each+k), offspring_genotype_probabilities, number_of_genotypes, eggs_with_genotype);
							//							Multinomial_Deviate ( *(eggs_laid_in_each+k), offspring_genotype_probabilities, number_of_genotypes, eggs_with_genotype);

							for (int genotype=0;genotype<number_of_genotypes;genotype++)
								if ((*(eggs_with_genotype+genotype))>0)
								{
									int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_had_medeas+female_adult_it->Wolbachia;
									//fprintf(stdout, "Genotype %d -- Index %d\n", genotype, index);

									if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
									{
										int *eggs_with_wolbachia_type = new int[number_of_wolbachia_status];
										genmul( *(eggs_with_genotype+genotype), Egg_Batch_Wolbachia_Type, number_of_wolbachia_status, eggs_with_wolbachia_type);

										for (int w=0 ; w<number_of_wolbachia_status ; w++)
										{
											index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_had_medeas+w;
											itNewEggCohort=containers_new_eggs_cohort.find(index);
											if (itNewEggCohort==containers_new_eggs_cohort.end())
											{
												itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
											}
											containers_new_eggs_cohort[index].Number += *(eggs_with_wolbachia_type+w);
										}
										sumeggs_laid += *(eggs_with_genotype+genotype);
										delete[] eggs_with_wolbachia_type;
									}
									else
									{
										itNewEggCohort=containers_new_eggs_cohort.find(index);
										//fprintf(stdout, "Search index %d ... ", index);
										if (itNewEggCohort==containers_new_eggs_cohort.end())
										{
											itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
											//fprintf(stdout, "Not found, inserted ... \n");
										}
										containers_new_eggs_cohort[index].Number += *(eggs_with_genotype+genotype);
										sumeggs_laid += *(eggs_with_genotype+genotype);
									}
								}

						}



					}

					if (REPRODUCTION_MODE==SAMPLING)
					{
						double *Egg_Batch_Wolbachia_Cumul = new double[number_of_wolbachia_status];
						Egg_Batch_Wolbachia_Cumul[0]=Egg_Batch_Wolbachia_Type[0];
						for (int w=1 ; w<number_of_wolbachia_status ; w++)
							Egg_Batch_Wolbachia_Cumul[w]=Egg_Batch_Wolbachia_Cumul[w-1]+Egg_Batch_Wolbachia_Type[w];



						for (int container=0;container<Number_of_Containers;container++)
						{
							for (int egg=0;egg<*(eggs_laid_in_each+container);egg++)
							{

								if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE)
								{
									double chromatid = Random_Number();


									if (chromatid<0.5)
									{
										if (chromatid<0.25)
										{
											offspring_genotype=offspring_chrom_genotype[1][0]*4;
										}
										else
										{
											offspring_genotype=offspring_chrom_genotype[1][1]*4+1;
										}
									}
									else
									{
										if (chromatid<0.75)
										{
											offspring_genotype=offspring_chrom_genotype[1][2]*4;
										}
										else
										{
											offspring_genotype=offspring_chrom_genotype[1][3]*4+1;
										}
									}
								}
								else
								{
									int a,b;
									a=2;
									b=1;
									offspring_genotype=0;

									// the idea here is that a and b hold the binary bit locations of the different alleles, so if we take X & a (binary and) we will pull off one, and X & b will pull off the other.
									// the only issue is that we will store the allele from the mother in positions corresponding to 2,8,32,128,... and those from the father in positions corresponding to 1,4,16,64,...
									// So we need to multiply by 2 when we pick the female off using &b and divide by two when we pick the male off using &a.

									for (int chrom=0;chrom<CHROM_NUMBER;chrom++)
									{
										double chromatid = Random_Number();


										if (chromatid<0.5)
										{
											if (chromatid<0.25)
											{
												offspring_genotype+=(female_adult_it->Genotype & a)+(male_genotype & a)/2;
											}
											else
											{
												offspring_genotype+=(female_adult_it->Genotype & a)+(male_genotype & b);
											}
										}
										else
										{
											if (chromatid<0.75)
											{
												offspring_genotype+=(female_adult_it->Genotype & b)*2+(male_genotype & a)/2;
											}
											else
											{
												offspring_genotype+=(female_adult_it->Genotype & b)*2+(male_genotype & b);
											}

										}
										a=a*4;
										b=b*4; // multiplying by 4 moves us to the next locus
									}
								}


								int index;
								if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
								{
									double r=Egg_Number*Random_Number();
									int w=0;
									while (Egg_Batch_Wolbachia_Cumul[w++]<r);

									index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*container+number_of_wolbachia_status*number_of_medea_masks*offspring_genotype+number_of_wolbachia_status*mom_had_medeas+(w-1);

								}
								else index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*container+number_of_wolbachia_status*number_of_medea_masks*offspring_genotype+number_of_wolbachia_status*mom_had_medeas+female_adult_it->Wolbachia;

								itNewEggCohort=containers_new_eggs_cohort.find(index);
								if (itNewEggCohort==containers_new_eggs_cohort.end())
								{
									itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
								}
								containers_new_eggs_cohort[index].Number++;
								sumeggs_laid++;

							}
						}

						delete[] Egg_Batch_Wolbachia_Cumul;
					}

					delete[] Egg_Batch_Wolbachia_Type;
					delete[] Egg_Batch_Wolbachia_Probas;


				}

				delete[] offspring_genotype_probabilities;
				delete[] eggs_with_genotype;


			}


			//deterministic implementation of the egg laying for female adult cohorts
			if (SIMULATION_MODE2==DETERMINISTIC)
			{
				double *offspring_genotype_fraction= new double [number_of_genotypes];
				for (int l=0;l<this->Number_of_Containers;l++)
				{
					for (int male_genotype=0;male_genotype<number_of_genotypes;male_genotype++)
					{

						for (int genotype=0;genotype<number_of_genotypes;genotype++) offspring_genotype_fraction[genotype]=0;

						int a=1;
						int b=2;

						// we will store female gamete types in the more
						// significant bit  (i.e. in the places occupied by
						// b), so we multiply by 2 when we look at a for female

						// for male, we divide by 2 when we look at b

						for (int chrom=0;chrom<CHROM_NUMBER;chrom++)
						{
							female_gamete_type[chrom][0]=(female_adult_it->Genotype & a)*2;
							female_gamete_type[chrom][1]=(female_adult_it->Genotype & b);

							male_gamete_type[chrom][0]=(male_genotype & a);
							male_gamete_type[chrom][1]=(male_genotype & b)/2;


							offspring_chrom_genotype[chrom][0]=female_gamete_type[chrom][0]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][1]=female_gamete_type[chrom][0]+male_gamete_type[chrom][1];
							offspring_chrom_genotype[chrom][2]=female_gamete_type[chrom][1]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][3]=female_gamete_type[chrom][1]+male_gamete_type[chrom][1];


							a=a*4;
							b=b*4;
						}

						if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE)
						{
							female_gamete_type[0][0]=0;
							female_gamete_type[0][1]=0;

							male_gamete_type[0][0]=0;
							male_gamete_type[0][1]=1;

							female_gamete_type[1][0]=(female_adult_it->Genotype & 48)/16;
							female_gamete_type[1][1]=(female_adult_it->Genotype & 12)/4;

							male_gamete_type[1][0]=(male_genotype & 48)/16;
							male_gamete_type[1][1]=(male_genotype & 12)/4;

							if (MD_TYPE==3)
							{
								female_gamete_type[2][0]=(female_adult_it->Genotype & 128)/128;
								female_gamete_type[2][1]=(female_adult_it->Genotype & 64)/64;

								male_gamete_type[2][0]=(male_genotype & 128)/128;
								male_gamete_type[2][1]=(male_genotype & 64)/64;

							}

							//for (int i=0;i<3;i++) printf("%d %d\n",female_gamete_type[i][0],female_gamete_type[i][1]);
							//for (int i=0;i<3;i++) printf("%d %d\n",male_gamete_type[i][0],male_gamete_type[i][1]);

							offspring_chrom_genotype[0][0]=0;
							offspring_chrom_genotype[0][1]=1;
							offspring_chrom_genotype[0][2]=0;
							offspring_chrom_genotype[0][3]=1;

							offspring_chrom_genotype[1][0]=female_gamete_type[1][0]*4+male_gamete_type[1][0];
							offspring_chrom_genotype[1][1]=female_gamete_type[1][0]*4+male_gamete_type[1][1];
							offspring_chrom_genotype[1][2]=female_gamete_type[1][1]*4+male_gamete_type[1][0];
							offspring_chrom_genotype[1][3]=female_gamete_type[1][1]*4+male_gamete_type[1][1];

							if (MD_TYPE==3)
							{
								offspring_chrom_genotype[2][0]=female_gamete_type[2][0]*2+male_gamete_type[2][0];
								offspring_chrom_genotype[2][1]=female_gamete_type[2][0]*2+male_gamete_type[2][1];
								offspring_chrom_genotype[2][2]=female_gamete_type[2][1]*2+male_gamete_type[2][0];
								offspring_chrom_genotype[2][3]=female_gamete_type[2][1]*2+male_gamete_type[2][1];
							}

							//for (int i=0;i<3;i++) printf("%d %d %d %d\n",offspring_chrom_genotype[i][0],offspring_chrom_genotype[i][1],offspring_chrom_genotype[i][2],offspring_chrom_genotype[i][3]);


						}



						//this combines the offspring genotypes per chromosome into the actual offspring chromosome
						if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE)
						{
							for (int punett1=0;punett1<3;punett1+=2)
							{
								for (int punett2=0;punett2<3;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (MD_TYPE==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=DRIVE_STRENGTH_SN;
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=DRIVE_STRENGTH_IT;
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=DRIVE_STRENGTH_IN;
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=DRIVE_STRENGTH_MN;
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_fraction[offspring_genotype]+=0.125*0.25;
									}
								}
							}
							for (int punett1=1;punett1<4;punett1+=2)
							{
								for (int punett2=1;punett2<4;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (MD_TYPE==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=DRIVE_STRENGTH_SN;
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=DRIVE_STRENGTH_IT;
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=DRIVE_STRENGTH_IN;
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=DRIVE_STRENGTH_MN;
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_fraction[offspring_genotype]+=0.5*(0.5-drive_strength)*0.25;
											else offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_fraction[offspring_genotype]+=0.125*0.25;
									}
								}
							}
						}

						if (GENETIC_CONTROL_METHOD!=MEIOTIC_DRIVE)
						{
							for (int punett=0;punett<(int) pow((double)4,CHROM_NUMBER);punett++)
							{

								offspring_genotype=0;


								for (int chrom=0;chrom<CHROM_NUMBER;chrom++)
								{
									offspring_genotype+=offspring_chrom_genotype[chrom][ (punett & mask[chrom])/divisor[chrom] ];
								}
								offspring_genotype_fraction[offspring_genotype]+=gen_prob;


							}
						}


						for (int genotype=0;genotype<number_of_genotypes;genotype++)
						{
							double neweggs = NewlyParousOvipositing*this->Daily_Average_Female_Fecundity*daily_average_female_fecundity_fudge_factor* *(container_probabilities+l)*offspring_genotype_fraction[genotype];
							//double neweggs=0 ;
							int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*l+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_had_medeas ;

							containers_new_eggs_cohort[index].Number += neweggs ;
							sumeggs_laid += neweggs ;
						}
					}
				}
				delete[] offspring_genotype_fraction;
			}

			// IF WE HAVE A NULLIPAROUS COHORT, transfer ovipositing adult females into a new adult female cohort, mark it parous //#24

			delete[] female_gamete_type;
			delete[] male_gamete_type;
			delete[] offspring_chrom_genotype;

			if (trapped)
			{
				female_adult_it=this->Female_Adults.erase(female_adult_it);
				continue;
			}
			if (is_female_nulliparous==NULLIPAROUS)
			{
				female_adult_it->Nulliparous=PAROUS;
				this->TotalNewlyParous++; //increase the total number of newly parous females
			}


		} //end of the part that runs if eggs are laid




		// mating
		if ((female_adult_it->Mated==false)
				||
				(
						(!(female_adult_it->Nulliparous))
						&&
						(Random_Number()<DAILY_REMATING_PROBABILITY_PAROUS_FEMALES)
				))
			// assumes monogamy, change here if necessary
		{
			// now

			double *male_mating_probabilities = new double [Male_Adult.size()]; // An array of size Male_Adult.size(), i.e., the number of male adult cohorts in the house
			int *male_genotypes = new int[Male_Adult.size()];
			int *male_wolbachia = new int[Male_Adult.size()];
			//int *males_chosen=new int[Male_Adult.size()];
			double cum_sum_probability=0;
			int k=0;

			for (list<Male_Adult_Cohort>::iterator male_adult_it=this->Male_Adult.begin(); male_adult_it != this->Male_Adult.end(); male_adult_it++)
			{
				double mating_probability=male_adult_it->Number*male_adult_it->Average_Adult_Weight;
				// for MEDEA, make that more general
				if (male_adult_it->Genotype>1)
				{
					int maskcopies = ((male_adult_it->Genotype)&12)>>2;
					if (maskcopies==3)
						mating_probability*=pow((double)REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS, 2.);
					else
						mating_probability*=pow((double)REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS, 2*REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS_DOMINANCE);

				}
				if (male_adult_it->Age>1)
				{
					male_mating_probabilities[k]=mating_probability+cum_sum_probability;
					male_genotypes[k]=male_adult_it->Genotype;
					male_wolbachia[k]=male_adult_it->Wolbachia;
					cum_sum_probability+=mating_probability;
				}
				else
				{
					male_mating_probabilities[k]=cum_sum_probability;
				}
				k++; // k is the number of male adult cohorts in the house
			}

			if (cum_sum_probability>0)
			{

				for (int male_cohort_number=0;male_cohort_number<k;male_cohort_number++) male_mating_probabilities[male_cohort_number]/=cum_sum_probability;
				// transform into cumulative probabilities
				// more convenient for stochastic simulations
				// (but less for deterministic runs)
				double r=Random_Number();
				int s=0;

				while (male_mating_probabilities[s++]<r); // goes through male adult cohorts and stops at first s such that male_mating_probabilities[s] >= r

				female_adult_it->Male_Genotype = male_genotypes[s-1];
				female_adult_it->Male_Wolbachia = male_wolbachia[s-1];
				female_adult_it->Mated=true ;
			}



			delete[] male_mating_probabilities;
			delete[] male_genotypes;
			delete[] male_wolbachia;

		}


		//mark-release-recapture
		if (SIMULATION_MODE2==STOCHASTIC)
			if (MARK_RELEASE_RECAPTURE)
				if (this->Recapture)
				{
					double Recaptured_Female=0;
					Recaptured_Female=Binomial_Deviate(1,FEMALE_DAILY_RECAPTURE_RATE);
					this->Recaptured_Female+=Recaptured_Female;
					// female_adult_it->Number-=Recaptured_Female;
				}

		//local dispersal
		if (SIMULATION_MODE2==STOCHASTIC)
		{
			bool Dispersing;
			double Dispersing_Probability ;
			int original = this->Position;
			int current = original;
			int direction = 3;
			bool has_moved = 0;

			for (int dispersal_step = 0 ; dispersal_step < MAX_NUMBER_OF_DISPERSAL_STEPS ; ) // disperal step increment done further
			{
				// calculate dispersal probability based on characteristics of the current house

				if (House[current].Number_of_Containers)
				{
					if (female_adult_it->Nulliparous) Dispersing_Probability=NULLIPAROUS_FEMALE_ADULT_DISPERSAL;		
					else Dispersing_Probability=PAROUS_FEMALE_ADULT_DISPERSAL;
				}
				else
				{
					if (female_adult_it->Nulliparous) Dispersing_Probability=NULLIPAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE;		
					else Dispersing_Probability=PAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE;
				}
				Dispersing = (Random_Number()<Dispersing_Probability);

				// if that female moves on this event 

				if (Dispersing)
				{
					// int direction= (int) 4*Random_Number(); // unbiased

					// old calculations -- before attractiveness		

					//int bias = DISPERSAL_DIRECTION_BIAS ;
					//int direction = (int) ((2*(bias+1))*Random_Number()) ; // with bias
					//direction = (2*(direction/(bias+1))) + ((direction%(bias+1))/bias) ; // repartition in 4 directions according to bias

					// calculate direction (0=north, 1=east, 2=south, 3=west)

					direction = 3;
					double rand_direction = House[current].neighbour_cumul_attractiveness[3]*Random_Number();

					if (rand_direction < House[current].neighbour_cumul_attractiveness[0]) direction=0;
					else if (rand_direction < House[current].neighbour_cumul_attractiveness[1]) direction=1;
					else if (rand_direction < House[current].neighbour_cumul_attractiveness[2]) direction=2;

					// calculate the corresponding destination house according to existing border conditions

					int destination ;

					if (House[current].neighbour_position[direction]==-1) // case RANDOM_EDGES
					{
						// choosing a random element in the edges list

						int dest = (int)(edges.size()*Random_Number()) ;
						list<int>::const_iterator iter ;
						iter=edges.begin() ;
						while (--dest>0) iter++ ;
						destination = (*iter) ;
					}
					else destination = House[current].neighbour_position[direction] ;

					// if the destination house is a good house, push if necessary and stop

					if (House[destination].Number_of_Containers > 0)
					{
						if (destination!=original) 
						{
							House[destination].Migrating_Female_Adult.push_back(*female_adult_it); 
							female_adult_it=this->Female_Adults.erase(female_adult_it);
							has_moved=true;
						}
						break;
					}

					// if the destination house is a bad house, go to another step if possible
					// if not (too many steps already), push anyway

					else
					{
						// increment done here
						if (++dispersal_step < MAX_NUMBER_OF_DISPERSAL_STEPS)
							current = destination ;
						else
						{
							if (destination!=original) 
							{
								House[destination].Migrating_Female_Adult.push_back(*female_adult_it); 
								female_adult_it=this->Female_Adults.erase(female_adult_it);
								has_moved=true;
							}
							break;
						}
					}

				}

				// if that female does not move on this dispersal event
				// check if she has moved before, and if yes, push 
				// then stop

				else 
				{
					if (current!=original) 
					{
						House[current].Migrating_Female_Adult.push_back(*female_adult_it);
						female_adult_it=this->Female_Adults.erase(female_adult_it);
						has_moved=true;
					}
					break;
				}


			}


			if (has_moved) continue;		
		}



		/**********************************/
		/* LONG RANGE DISPERSAL (females) */
		/**********************************/

		if (SIMULATION_MODE2==STOCHASTIC)
		{
			bool LR_Dispersing;
			if (female_adult_it->Nulliparous) LR_Dispersing=(Random_Number()<NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL);
			else LR_Dispersing=(Random_Number()<PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL);
			if (LR_Dispersing)
			{

				// 5 steps to calculate destination :

				// (1) get the distance as a number of steps
				// (random between 1 and LR_DISPERSAL_MAX_DISTANCE) ;
				int steps = 1+ (int) ((LR_DISPERSAL_MAX_DISTANCE-1)*Random_Number())  ;

				// (2) a certain proportion of these steps go horizontally,
				// left if dx<0, right if dx>0 ;
				int dx = (int) ((2*Random_Number()>1 ? -1 : 1) * steps * Random_Number() );
				int xpos = (House_position%NUMBER_OF_COLUMNS)+dx ;

				// (3) the rest of the steps go vertically,
				// down if dy>0, up if dy<0 ;
				int dy = (2*Random_Number()>1 ? -1 : 1) * (steps-(dx>0?dx:-dx)) ;
				int ypos = (House_position/NUMBER_OF_COLUMNS)+dy ;

				// (4) calculate the actual coordinates of the destination house
				// depending on the topology of the grid
				int xdest, ydest ;
				switch(DISPERSAL_MODE)
				{
				case STICKY_BORDER:

					xdest = ( xpos < 0 ? 0 :
					(xpos >= NUMBER_OF_COLUMNS ? NUMBER_OF_COLUMNS-1 :
					xpos)) ;

					ydest = ( ypos < 0 ? 0 :
					(ypos >= NUMBER_OF_ROWS ? NUMBER_OF_ROWS-1 :
					ypos)) ;

					break;

				case BOUNCE:
					do {
						xpos = ( xpos < 0 ? -(xpos) :
								(xpos >= NUMBER_OF_COLUMNS ? NUMBER_OF_COLUMNS-1 +(NUMBER_OF_COLUMNS-1 -(xpos)) :
										xpos)) ;
					} while ((xpos<0) || (xpos>NUMBER_OF_COLUMNS-1)) ;
					xdest=xpos ;


					do {
						ypos = ( ypos < 0 ? -(ypos) :
								(ypos >= NUMBER_OF_ROWS ? NUMBER_OF_ROWS-1 + (NUMBER_OF_ROWS-1 -(ypos)) :
										ypos)) ;
					} while ((ypos<0) || (ypos>NUMBER_OF_ROWS-1)) ;
					ydest=ypos ;

					break;

				case TORE:

					xdest = xpos%NUMBER_OF_COLUMNS ;
					xdest = (xdest<0 ? xdest+NUMBER_OF_COLUMNS : xdest);
					ydest = ypos%NUMBER_OF_ROWS ;
					ydest = (ydest<0 ? ydest+NUMBER_OF_ROWS : ydest);

					break ;

				case RANDOM_EDGES:

					// randomly coming back on one of the 4 edges
					// note that each edge include only one corner
					// so that the corners are not counted twice

					int edge = (int)(4*Random_Number()) ;
					switch(edge)
					{
					case 0: //north
						xdest = (int)((NUMBER_OF_COLUMNS-1)*Random_Number()) ;
						ydest = 0 ;
						break;
					case 1: //east
						xdest = NUMBER_OF_COLUMNS-1 ;
						ydest = (int)((NUMBER_OF_ROWS-1)*Random_Number()) ;
						break;
					case 2: //south
						xdest = 1+(int)((NUMBER_OF_COLUMNS-1)*Random_Number()) ;
						ydest = NUMBER_OF_ROWS-1 ;
						break;
					case 3: //west
						xdest = 0 ;
						ydest = 1+(int)((NUMBER_OF_ROWS-1)*Random_Number()) ;
						break;
					}
					break;
				}

				// (5) convert these coordinates into the destination house number
				int destination = ydest*NUMBER_OF_COLUMNS + xdest ;


				House[destination].Migrating_Female_Adult.push_back(*female_adult_it);
				female_adult_it=this->Female_Adults.erase(female_adult_it);
				continue;


			}
		}

		/***********************************************************************/



		female_adult_it++; // if female hasn't died or moved, skip pointer to next female

	}

	//this is the end of the adult female cohort calculations

	delete[] container_probabilities;
	//if(eggs_laid_in_each==NULL) fprintf(stdout, "NULL eggs_laid_in_each\n") ;
	if (SIMULATION_MODE2==STOCHASTIC) delete[] eggs_laid_in_each;

	//this part sets the properties of the new egg cohorts //#25

	//double sumeggs_laid=0;
	//for (int egg_group=0; egg_group<(number_of_genotypes*this->Number_of_Containers); egg_group++) sumeggs_laid+=containers_new_eggs_cohort[egg_group].Number;


	if (sumeggs_laid>0)
	{
		for (int k=0;k<this->Number_of_Containers;k++)
		{
			//this calculates the level of eggs laid in the container based on CIMSIM code (not perfectly matching)
			if (SIMULATION_MODE1==CIMSIM) EggInputLevel=FLOOR_(this->Container[k].Water_Level/2)+1;
			if (SIMULATION_MODE1==SKEETER_BUSTER) EggInputLevel=FLOOR_(this->Container[k].Water_Level/0.2)+1;
			if (SIMULATION_MODE1==CIMSIM) TopContainerLevel=ROUND_(this->Container[k].Height/2+0.2);
			if (SIMULATION_MODE1==SKEETER_BUSTER) TopContainerLevel=ROUND_(this->Container[k].Height/0.2+2);

			if (EggInputLevel>TopContainerLevel) EggInputLevel=TopContainerLevel;

			// oviposition test
			bool oviposited = false;
			double sum_over_layers = 0.;
			//fprintf(test1, "%d %d %d ", Date, House_position, k);
			//

			int container_mask = (number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes);
			int genotype_mask = (number_of_wolbachia_status*number_of_medea_masks);
			int medea_mask = number_of_wolbachia_status;

			if (EggInputLevel==TopContainerLevel)
			{

				// with the use of map, no need to go through all possible values of 'index'
				// only the existing ones

				//for (int genotype=0;genotype<number_of_genotypes;genotype++)
				//	for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
				//		for (int w=0 ; w<number_of_wolbachia_status ; w++)

				for (itNewEggCohort=containers_new_eggs_cohort.begin() ; itNewEggCohort!=containers_new_eggs_cohort.end() ; itNewEggCohort++)
				{
					//int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_type+w;
					int index = itNewEggCohort->first;

					// some explanation for this index :
					// for some reason, containers_new_eggs_cohort is a 1-dimension only array, even though
					// it truly has 4 dimensions :
					// - containers ;
					// - eggs genotype ;
					// - medea_mom (medea status of the mother)
					// - wolbachia (wolbachia status of the mother)
					// 
					// The array is then hierarchised linearly, that is, big blocks representing one container, 
					// each of these blocks subdivided by genotype,
					// each of these subblocks subsubdivided by medea_mom type,
					// each of these subsubblobks subsubsubdivided (!) by wolbachia type.
					//
					// The above variable 'index' calculates the position in the array once and for all
					// for a given combination of all of the above.

					//if (containers_new_eggs_cohort[index].Number>0)

					if ((index / container_mask)==k)
						// this checks that we are in the right container
						// there has to be a better way to do this

						if (itNewEggCohort->second.Number>0) // this should be redundant
						{
							itNewEggCohort->second.Level_laid_in_Container=EggInputLevel;
							itNewEggCohort->second.Genotype=(index % container_mask)/genotype_mask;
							//fprintf(stdout, "index %d genotype %d\n", itNewEggCohort->first, itNewEggCohort->second.Genotype);
							itNewEggCohort->second.Medea_Mom=(index % genotype_mask)/medea_mask;
							itNewEggCohort->second.Wolbachia=(index % medea_mask);

							this->Container[k].Eggs.push_back(itNewEggCohort->second);
							// oviposition test
							//fprintf(test1, "%.3f ", containers_new_eggs_cohort[number_of_medea_masks*number_of_genotypes*k+number_of_medea_masks*genotype+mom_type].Number);
							oviposited = true;
							//
						}
				}
			}
			else
			{
				if (SIMULATION_MODE1==CIMSIM) TopEggInputLevel=1; else TopEggInputLevel=TopContainerLevel-EggInputLevel;
				if (TopEggInputLevel>19) TopEggInputLevel=19;
				if (SIMULATION_MODE2==DETERMINISTIC)
				{
					for (int genotype=0;genotype<number_of_genotypes;genotype++)
						for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
							for (int w=0; w<number_of_wolbachia_status ; w++)
							{
								int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_type+w;
								containers_new_eggs_cohort[index].Number = (double) containers_new_eggs_cohort[index].Number/(TopEggInputLevel+1);
							}
					for (int l=0;l<(TopEggInputLevel+1);l++)
					{
						for (int genotype=0;genotype<number_of_genotypes;genotype++)
							for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
								for (int w=0 ; w<number_of_wolbachia_status ; w++)
								{
									int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_type+w;
									if (containers_new_eggs_cohort[index].Number>0)
									{
										containers_new_eggs_cohort[index].Level_laid_in_Container=EggInputLevel+l;
										containers_new_eggs_cohort[index].Genotype=genotype;
										containers_new_eggs_cohort[index].Medea_Mom=mom_type;
										containers_new_eggs_cohort[index].Wolbachia=w;
										this->Container[k].Eggs.push_back(containers_new_eggs_cohort[index]);
										// oviposition test
										//fprintf(test1, "%.3f ",containers_new_eggs_cohort[number_of_medea_masks*number_of_genotypes*k+number_of_medea_masks*genotype+mom_type].Number);
										oviposited = true;//
									}
								}
					}
				}
				else
				{
					double *layer_probabilities= new double [(int) (TopEggInputLevel+1)];
					int *eggs_laid_per_layer= new int [(int) (TopEggInputLevel+1)];
					int number_of_layers = (int) ((TopEggInputLevel+1)) ;
					// this array is organized like this : first, layers for mom type 0, then layers for mom type 1, then ... etc. for all mom types

					for (int layer=0;layer<number_of_layers;layer++)
					{
						layer_probabilities[layer]=(double) (1/(TopEggInputLevel+1));
						eggs_laid_per_layer[layer]=0 ;
					}

					//for (int genotype=0;genotype<number_of_genotypes;genotype++)
					//	for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
					//		for (int w=0 ; w<number_of_wolbachia_status ; w++)
					for (itNewEggCohort=containers_new_eggs_cohort.begin() ; itNewEggCohort!=containers_new_eggs_cohort.end() ; itNewEggCohort++)
					{
						//int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_type+w;

						int index = itNewEggCohort->first;

						// debug
						// fprintf(Short_Output, "%d ", containers_new_eggs_cohort[number_of_genotypes*k+genotype].Number) ;


						//debug
						// fprintf(Short_Output, "%d ", containers_new_eggs_cohort[number_of_genotypes*k+genotype].Medea_Mom[mom_type]) ;	

						// doesn't work
						// genmul ( (int) containers_new_eggs_cohort[number_of_genotypes*k+genotype].Medea_Mom[mom_type], layer_probabilities, (int) (TopEggInputLevel+1), eggs_laid_per_layer+(mom_type*number_of_layers));

						// int index = number_of_wolbachia_status*number_of_medea_masks*number_of_genotypes*k+number_of_wolbachia_status*number_of_medea_masks*genotype+number_of_wolbachia_status*mom_type+w;					
						// see above for details

						// much needed control to avoid running genmul 
						// if no eggs of this type (genotype+wolbachia) exist

						if ((index / container_mask)==k)
							// this checks that we are in the right container
							// there has to be a better way to do this

							if (itNewEggCohort->second.Number>0) // this should be redundant
							{

								genmul ( (int) itNewEggCohort->second.Number, layer_probabilities, (int) (TopEggInputLevel+1), eggs_laid_per_layer);

								itNewEggCohort->second.Genotype=(index % container_mask)/genotype_mask;
								//fprintf(stdout, "index %d genotype %d\n", itNewEggCohort->first, itNewEggCohort->second.Genotype);
								itNewEggCohort->second.Medea_Mom=(index % genotype_mask)/medea_mask;
								itNewEggCohort->second.Wolbachia=(index % medea_mask);


								for (int layer=0;layer<(TopEggInputLevel+1);layer++)
									if (eggs_laid_per_layer[layer])
									{
										itNewEggCohort->second.Level_laid_in_Container=EggInputLevel+layer;
										itNewEggCohort->second.Number=eggs_laid_per_layer[layer];

										this->Container[k].Eggs.push_back(itNewEggCohort->second);

										if (this->Container[k].Container_Type_Number==102)
											{
											// Where did the error occur?
											printf("Attempted mating on house: %d", House_position);
											ErrorMessage(102);
											}
										// oviposition test
										oviposited = true;
										sum_over_layers+=itNewEggCohort->second.Number;
										//
									}
							}
					}
					//
					//fprintf(test1, "%.0f ", sum_over_layers);
					//
					delete[] layer_probabilities;
					delete[] eggs_laid_per_layer;
				}
			}

			// oviposition test
			//if (!oviposited) fprintf(test1, "0");
			//fprintf(test1, "\n");
			//

		}

	}

	//log the properties of female adult cohorts into the detailed log file
//+++++++++
	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END))
		for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin() ; female_adult_it != this->Female_Adults.end() ; female_adult_it++)
		{
			fprintf(Output,"%s %d %s %d %s %d %s %.3g %s %.8g %s %.8g %s %8g %s %d\n","Female adult #",-999," nulliparous ",female_adult_it->Nulliparous," Age: ",female_adult_it->Age," Age since hatch: ",female_adult_it->Age_since_hatch, " Phys. Dev.: ",female_adult_it->Physiological_Development_Percent," Ave. Weight: ",female_adult_it->Average_Adult_Weight, " Fecundity: ", female_adult_it->Fecundity," Genotype: ", female_adult_it->Genotype);
		}

	/* OMP PARALLEL CHECK OK */
	//this is the calculation for male adult cohorts //#26,#27

	for (list<Male_Adult_Cohort>::iterator male_adult_it=this->Male_Adult.begin(); male_adult_it != this->Male_Adult.end(); ) // do male_adult_it++ at end of loop
	{
		male_adult_it->Male_Adult_Calculations(Air_Maximum,Air_Minimum,Air_Average,SD);



		//mark-release-recapture
		if (SIMULATION_MODE2==STOCHASTIC)
			if (MARK_RELEASE_RECAPTURE)
				if (this->Recapture)
					if (male_adult_it->Number>0)
					{
						double Recaptured_Male=0;

						Recaptured_Male=Binomial_Deviate(male_adult_it->Number,MALE_DAILY_RECAPTURE_RATE);
						if (Recaptured_Male>male_adult_it->Number) Recaptured_Male=male_adult_it->Number;
						this->Recaptured_Male+=Recaptured_Male;
						male_adult_it->Number-=Recaptured_Male;
					}

		if (SIMULATION_MODE2==STOCHASTIC)
			if (male_adult_it->Number>0)
			{
				double Dispersing_Males,Dispersing[4], Dispersing_Males_Probability;
				for (int direction=0;direction<4;direction++) Dispersing[direction]=0;
				if (this->Female_Adults.empty())
				{
					Dispersing_Males_Probability = MALE_ADULT_DISPERSAL_WHEN_NO_FEMALE;
				}
				else
					Dispersing_Males_Probability = MALE_ADULT_DISPERSAL;

				Dispersing_Males= Binomial_Deviate(male_adult_it->Number,Dispersing_Males_Probability);
				if (Dispersing_Males>0)
				{
					for (int dispersal=0;dispersal<Dispersing_Males;dispersal++)
					{
						// int direction= (int) 4*Random_Number(); // unbiased
						// old calculations - no atractiveness

						//int bias=DISPERSAL_DIRECTION_BIAS ;
						//int direction = (int) ((2*(bias+1))*Random_Number()) ; // with bias
						//direction = (2*(direction/(bias+1))) + ((direction%(bias+1))/bias); // repartition in 4 directions according to bias

						int direction = 3;
						double rand_direction = this->neighbour_cumul_attractiveness[3]*Random_Number();
						if (rand_direction < this->neighbour_cumul_attractiveness[0]) direction=0;
						else if (rand_direction < this->neighbour_cumul_attractiveness[1]) direction=1;
						else if (rand_direction < this->neighbour_cumul_attractiveness[2]) direction=2;

						Dispersing[direction]++;
					}
					for (int direction=0;direction<4;direction++)
					{
						if (Dispersing[direction]>0)
						{
							Male_Adult_Cohort dispersing_males(Dispersing[direction],male_adult_it->Average_Adult_Weight,male_adult_it->Genotype,male_adult_it->Wolbachia,male_adult_it->Physiological_Development_Percent,male_adult_it->Age,male_adult_it->Age_since_hatch,male_adult_it->Mature,male_adult_it->Release_Date);

							int destination ;

							if (this->neighbour_position[direction]==-1) // case RANDOM_EDGES
									{
								// choosing a random element in the edges list

								int dest = (int)(edges.size()*Random_Number()) ;
								list<int>::const_iterator iter ;
								iter=edges.begin() ;
								while (--dest>0) iter++ ;
								destination = (*iter) ;
									}
							else destination = this->neighbour_position[direction] ;

							if (destination!=House_position)
							{
								House[destination].Migrating_Male_Adult.push_back(dispersing_males);
								male_adult_it->Number-=Dispersing[direction];
							}
						}
					}
				}
			}

		/********************************/
		/* LONG RANGE DISPERSAL (males) */
		/********************************/

		if (SIMULATION_MODE2==STOCHASTIC)
			if (male_adult_it->Number>0)
			{
				double LR_Dispersing_Males; //,Dispersing[4];

				LR_Dispersing_Males=Binomial_Deviate(male_adult_it->Number,MALE_ADULT_LONG_RANGE_DISPERSAL);
				if (LR_Dispersing_Males>0)
				{
					for (int lrd=0 ; lrd<LR_Dispersing_Males ; lrd++)
					{
						// 5 steps to calculate destination :

						// (1) get the distance as a number of steps
						// (random between 1 and LR_DISPERSAL_MAX_DISTANCE) ;
						int steps = 1+ (int) ((LR_DISPERSAL_MAX_DISTANCE-1)*Random_Number())  ;

						// (2) a certain proportion of these steps go horizontally,
						// left if dx<0, right if dx>0 ;
						int dx = (int) ((2*Random_Number()>1 ? -1 : 1) * steps * Random_Number() );
						int xpos = (House_position%NUMBER_OF_COLUMNS)+dx ;

						// (3) the rest of the steps go vertically,
						// down if dy>0, up if dy<0 ;
						int dy = (2*Random_Number()>1 ? -1 : 1) * (steps-(dx>0?dx:-dx)) ;
						int ypos = (House_position/NUMBER_OF_COLUMNS)+dy ;

						// (4) calculate the actual coordinates of the destination house
						// depending on the topology of the grid
						int xdest, ydest ;
						switch(DISPERSAL_MODE)
						{
						case STICKY_BORDER:

							xdest = ( xpos < 0 ? 0 :
							(xpos >= NUMBER_OF_COLUMNS ? NUMBER_OF_COLUMNS-1 :
							xpos)) ;

							ydest = ( ypos < 0 ? 0 :
							(ypos >= NUMBER_OF_ROWS ? NUMBER_OF_ROWS-1 :
							ypos)) ;

							break;

						case BOUNCE:
							do {
								xpos = ( xpos < 0 ? -(xpos) :
										(xpos >= NUMBER_OF_COLUMNS ? NUMBER_OF_COLUMNS-1 +(NUMBER_OF_COLUMNS-1 -(xpos)) :
												xpos)) ;
							} while ((xpos<0) || (xpos>NUMBER_OF_COLUMNS-1)) ;
							xdest=xpos ;


							do {
								ypos = ( ypos < 0 ? -(ypos) :
										(ypos >= NUMBER_OF_ROWS ? NUMBER_OF_ROWS-1 + (NUMBER_OF_ROWS-1 -(ypos)) :
												ypos)) ;
							} while ((ypos<0) || (ypos>NUMBER_OF_ROWS-1)) ;
							ydest=ypos ;

							break;

						case TORE:

							xdest = xpos%NUMBER_OF_COLUMNS ;
							xdest = (xdest<0 ? xdest+NUMBER_OF_COLUMNS : xdest);
							ydest = ypos%NUMBER_OF_ROWS ;
							ydest = (ydest<0 ? ydest+NUMBER_OF_ROWS : ydest);

							break ;

						case RANDOM_EDGES:

							// randomly coming back on one of the 4 edges
							// note that each edge include only one corner
							// so that the corners are not counted twice

							int edge = (int)(4*Random_Number()) ;
							switch(edge)
							{
							case 0: //north
								xdest = (int)((NUMBER_OF_COLUMNS-1)*Random_Number()) ;
								ydest = 0 ;
								break;
							case 1: //east
								xdest = NUMBER_OF_COLUMNS-1 ;
								ydest = (int)((NUMBER_OF_ROWS-1)*Random_Number()) ;
								break;
							case 2: //south
								xdest = 1+(int)((NUMBER_OF_COLUMNS-1)*Random_Number()) ;
								ydest = NUMBER_OF_ROWS-1 ;
								break;
							case 3: //west
								xdest = 0 ;
								ydest = 1+(int)((NUMBER_OF_ROWS-1)*Random_Number()) ;
								break;
							}
							break;

						}

						// (5) convert these coordinates into the destination house number
						int destination = ydest*NUMBER_OF_COLUMNS + xdest ;

						Male_Adult_Cohort lr_dispersing_males(1,male_adult_it->Average_Adult_Weight,male_adult_it->Genotype,male_adult_it->Wolbachia,male_adult_it->Physiological_Development_Percent,male_adult_it->Age,male_adult_it->Age_since_hatch,male_adult_it->Mature,male_adult_it->Release_Date);

						House[destination].Migrating_Male_Adult.push_back(lr_dispersing_males);
						male_adult_it->Number--;

					}

				}
			}

		/***********************************************************************/


		double temp;
		temp=male_adult_it->Number;

		if (male_adult_it->Number==0)
		{
			male_adult_it=this->Male_Adult.erase(male_adult_it);
		}
		else
		{
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END))
			{
				fprintf(Output,"%s %d %s %.8g %s %d %s %.3g %s %.8g %s %d\n","Male adult cohort #",-999," Number: ",male_adult_it->Number," Age: ",male_adult_it->Age," Age since hatch: ",male_adult_it->Age_since_hatch," Ave. Weight: ",male_adult_it->Average_Adult_Weight," Genotype: ",male_adult_it->Genotype);
			}

			male_adult_it++;
		}

	}


	return;
};

void Building::BuildingSummation()
{
	//this part makes a final tally at the end of the day of the number of all mosquitoes in all life-stages //#41
	int number_of_genotypes;

	number_of_genotypes=(int) pow((double)2,2*CHROM_NUMBER);

	int number_of_wolbachia_status ;
	if (!NUMBER_OF_INCOMPATIBLE_WOLBACHIA) number_of_wolbachia_status = 1; // just making sure that 2^0=1
	else number_of_wolbachia_status = (int) pow((double)2,NUMBER_OF_INCOMPATIBLE_WOLBACHIA);

	double *wolbachia_frequency = new double[number_of_wolbachia_status];
	double *wolbachia_prevalence = new double[NUMBER_OF_INCOMPATIBLE_WOLBACHIA];

	for (int w=0 ; w<number_of_wolbachia_status ; w++)
		wolbachia_frequency[w]=0.;
	for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
		wolbachia_prevalence[ww]=0.;

	this->TotalEggs=this->TotalFemalePupae=this->TotalMalePupae=this->TotalFemaleNulliparousAdults=this->TotalFemaleParousAdults=this->TotalMaleAdults=0;
	for (int instar = 1 ; instar <4.5 ; instar++)
	{
		this->TotalLarvae[MALE][instar-1]=0;
		this->TotalLarvae[FEMALE][instar-1]=0;
	}

	double *Total_Genotype_per_House= new double [number_of_genotypes];
	double *Total_Adult_Genotype_per_House = new double [number_of_genotypes];

	for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Genotype_per_House[genotype]=Total_Adult_Genotype_per_House[genotype]=0;





	double New_Pupae_in_Container=0;

	for (int j=0;j<this->Number_of_Containers;j++)
	{
		int thisctype = this->Container[j].Container_Type_Number ;
		Census_by_container_type[0][thisctype]++ ;
		Census_by_container_type[6][thisctype]+=this->Container[j].Water_Level;
		Census_by_container_type[7][thisctype]+=this->Container[j].Maximum_Daily_Water_Temperature;
		Census_by_container_type[8][thisctype]+=this->Container[j].Minimum_Daily_Water_Temperature;
		Census_by_container_type[9][thisctype]+=this->Container[j].Average_Daily_Water_Temperature;

		Census_by_container_type[10][thisctype]+=this->Container[j].Food;


		double Total_Eggs_in_Container=0;
		for (list<Eggs_Cohort>::iterator eggs_cohort_it=this->Container[j].Eggs.begin(); eggs_cohort_it != this->Container[j].Eggs.end() ; eggs_cohort_it++)
		{
			this->TotalEggs+=eggs_cohort_it->Number;
			Total_Genotype_per_House[eggs_cohort_it->Genotype]+=eggs_cohort_it->Number;
			Total_Eggs_in_Container+=eggs_cohort_it->Number ;
			wolbachia_frequency[eggs_cohort_it->Wolbachia]+=eggs_cohort_it->Number;
		}
		Census_by_container_type[1][thisctype]+=Total_Eggs_in_Container;

		double Total_Larvae_in_Container=0;
		for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Container[j].Larvae.begin(); larvae_cohort_it != this->Container[j].Larvae.end() ; larvae_cohort_it++)
		{
			int sex=larvae_cohort_it->Sex;
			this->TotalLarvae[sex][larvae_cohort_it->Instar -1] += larvae_cohort_it->Number;
			Total_Genotype_per_House[larvae_cohort_it->Genotype]+=larvae_cohort_it->Number;
			Total_Larvae_in_Container+=larvae_cohort_it->Number ;
			wolbachia_frequency[larvae_cohort_it->Wolbachia]+=larvae_cohort_it->Number;
		}
		Census_by_container_type[2][thisctype]+=Total_Larvae_in_Container;

		double Total_Pupae_in_Container=0;
		for (list<Pupae_Cohort>::iterator pupae_cohort_it=this->Container[j].Pupae.begin(); pupae_cohort_it != this->Container[j].Pupae.end() ; pupae_cohort_it++)
		{
			if (pupae_cohort_it->Sex==FEMALE)
				this->TotalFemalePupae+=pupae_cohort_it->Number;
			else
				this->TotalMalePupae+=pupae_cohort_it->Number;
			Total_Genotype_per_House[pupae_cohort_it->Genotype]+=pupae_cohort_it->Number;
			Total_Pupae_in_Container+=pupae_cohort_it->Number ;
			if (pupae_cohort_it->Age==1) New_Pupae_in_Container+=pupae_cohort_it->Number;
			wolbachia_frequency[pupae_cohort_it->Wolbachia]+=pupae_cohort_it->Number;
			if (pupae_cohort_it->Number) Census_by_container_type[5][thisctype]+=pupae_cohort_it->Number*pupae_cohort_it->Average_Pupal_Weight ;
		}

		Census_by_container_type[3][thisctype]+=Total_Pupae_in_Container;
		if (Total_Pupae_in_Container)
		{
			Census_by_container_type[4][thisctype]++ ;
			Census_by_container_type[5][thisctype]/=Total_Pupae_in_Container ;
		}

	}

	cumul_pupae_per_house[this->Position]+= (int)New_Pupae_in_Container;


	Total_Eggs+=this->TotalEggs;
	for (int instar=1 ; instar<4.5 ; instar++)
	{
		Total_Female_Larvae+=this->TotalLarvae[0][instar-1];
		Total_Male_Larvae+=this->TotalLarvae[1][instar-1];
	}
	Total_Female_Pupae+=this->TotalFemalePupae;
	Total_Male_Pupae+=this->TotalMalePupae;


	for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin(); female_adult_it != this->Female_Adults.end(); female_adult_it++)
	{
		if (female_adult_it->Nulliparous==NULLIPAROUS)
		{
			this->TotalFemaleNulliparousAdults++;
			Total_Genotype_per_House[female_adult_it->Genotype]++;
			Total_Adult_Genotype_per_House[female_adult_it->Genotype]++;

		}
		else
		{
			this->TotalFemaleParousAdults++;
			Total_Genotype_per_House[female_adult_it->Genotype]++;
			Total_Adult_Genotype_per_House[female_adult_it->Genotype]++;
		}
		wolbachia_frequency[female_adult_it->Wolbachia]++;
	}

	for (list<Male_Adult_Cohort>::iterator male_adult_it=this->Male_Adult.begin(); male_adult_it != this->Male_Adult.end(); male_adult_it++)
	{
		this->TotalMaleAdults+=male_adult_it->Number;
		Total_Genotype_per_House[male_adult_it->Genotype]+=male_adult_it->Number;
		Total_Adult_Genotype_per_House[male_adult_it->Genotype]+=male_adult_it->Number;
		wolbachia_frequency[male_adult_it->Wolbachia]+=male_adult_it->Number;
	}

	Total_Nulliparous_Female_Adults+=this->TotalFemaleNulliparousAdults;
	Total_Parous_Female_Adults+=this->TotalFemaleParousAdults;
	Total_Male_Adults+=this->TotalMaleAdults;
	for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Genotypes[genotype]+=Total_Genotype_per_House[genotype];
	for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Adult_Genotypes[genotype]+=Total_Adult_Genotype_per_House[genotype];

	double *allele_frequency_per_House= new double [2*CHROM_NUMBER];
	for (int allele=0;allele<2*CHROM_NUMBER;allele++) allele_frequency_per_House[allele]=0;
	double Total_Mosquitoes=0;

	//for (int allele=0;allele<2*CHROM_NUMBER;allele++)
		// // steps are 2, another allele++ at the end of the loop
	//{
	// //int mask1 = ((int) pow((double)2,2*CHROM_NUMBER-1-allele)) ;
	// //int mask2 = ((int) pow((double)2,2*CHROM_NUMBER-2-allele)) ; // 1 mask per homologous chromosome
	// //for (int genotype=0;genotype<number_of_genotypes;genotype++)
	// //{
	// //if (((genotype/mask1)%2)==0) allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype]; else allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
	// //if (((genotype/mask2)%2)==0) allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype]; else allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
	// //}
	// //allele++;
	//}

	// above version very time consuming
	// new version:

	for (int genotype=0 ; genotype<number_of_genotypes ; genotype++)
		if (Total_Genotype_per_House[genotype])
			// advantage for looping through genotypes first: skip absent genotypes
		{
			int bit_position = 0;
			int mask=(1u<<(2*CHROM_NUMBER-1));

			do
			{
				int chrom = bit_position/2;
				int allele = 2*chrom;
				if (genotype&mask) allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
				else allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype];

				mask>>=1;
				bit_position++;
			}
			while (mask>0);
		}

	for (int genotype=0;genotype<number_of_genotypes;genotype++) Total_Mosquitoes+=Total_Genotype_per_House[genotype];
	for (int allele=0;allele<2*CHROM_NUMBER;allele++)
	{
		if (Total_Mosquitoes)
			allele_frequency_per_House[allele]/=2*Total_Mosquitoes;
		else if (allele_frequency_per_House[allele]!=0.)
		{
			fprintf(stdout, "*** Warning: non-zero allele frequency in empty house\n");
			fprintf(stdout, "*** House: %d - Allele %d on chromosome %d\n", House_position, allele%2, allele/2);
			fprintf(stdout, "*** Frequency set to zero - Continuing ...\n");

			allele_frequency_per_House[allele]=0.;
		}

		allele_frequencies[allele][House_position]=allele_frequency_per_House[allele];
	}

	for (int w=0 ; w<number_of_wolbachia_status ; w++)
	{
		Total_wolbachia_frequency[w]+=wolbachia_frequency[w];
		if (Total_Mosquitoes) wolbachia_frequency[w]/=Total_Mosquitoes;
		else wolbachia_frequency[w]=0.;
	}
	for (int ww=0 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
		for (int w=0 ; w<number_of_wolbachia_status ; w++)
			if ((w>>ww)%2)
				wolbachia_prevalence[ww]+=wolbachia_frequency[w];

	double Total_Number_of_Adult_Females=0;
	double Average_Female_Weight=0;
	for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin(); female_adult_it != this->Female_Adults.end(); female_adult_it++)
	{
		Total_Number_of_Adult_Females++;
		Average_Female_Weight+=female_adult_it->Average_Adult_Weight;
	}
	Average_Female_Weight/=Total_Number_of_Adult_Females;

	double Total_Number_of_Adult_Males=0;
	double Average_Male_Weight=0;
	for (list<Male_Adult_Cohort>::iterator male_adult_it=this->Male_Adult.begin(); male_adult_it != this->Male_Adult.end(); male_adult_it++)
	{
		Total_Number_of_Adult_Males+=male_adult_it->Number;
		Average_Male_Weight+=male_adult_it->Average_Adult_Weight*male_adult_it->Number;
	}
	Average_Male_Weight/=Total_Number_of_Adult_Males;


	CV_Pupae += (this->TotalMalePupae+this->TotalFemalePupae)*(this->TotalMalePupae+this->TotalFemalePupae) ;

	if (GENETIC_CONTROL_METHOD == ReduceReplace)
		{
// What is the number of females that don't carry the replacement allele?
			if (CHROM_NUMBER==3)
				{
				CV_WILD_FEMALES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48])*(Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48]);
				}
			if (CHROM_NUMBER==5) /* two gene case - revision Apr. 7, 2012 */
				{
				CV_WILD_FEMALES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[64]+Total_Adult_Genotype_per_House[128]+Total_Adult_Genotype_per_House[192] + Total_Adult_Genotype_per_House[256] + Total_Adult_Genotype_per_House[320] + Total_Adult_Genotype_per_House[384]+Total_Adult_Genotype_per_House[448] + Total_Adult_Genotype_per_House[576] + Total_Adult_Genotype_per_House[512] + Total_Adult_Genotype_per_House[640] + Total_Adult_Genotype_per_House[704] + Total_Adult_Genotype_per_House[832] + Total_Adult_Genotype_per_House[768] + Total_Adult_Genotype_per_House[896] + Total_Adult_Genotype_per_House[960]);
				}
// What is the number of females that carry at least one wild allele? - don't measure for more than two genes as cases get astronomical
			CV_WILD_ALLELES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48] + Total_Adult_Genotype_per_House[20]+Total_Adult_Genotype_per_House[4]+Total_Adult_Genotype_per_House[36]+Total_Adult_Genotype_per_House[52] + Total_Adult_Genotype_per_House[24]+Total_Adult_Genotype_per_House[8]+Total_Adult_Genotype_per_House[40]+Total_Adult_Genotype_per_House[56]);
// Have the wild-type females gone extinct in this house?
			if (CHROM_NUMBER==3)
				{
				if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48])==0)
					{
					NO_WILD_FEMALES++;
					}
				}

			if (CHROM_NUMBER==5) /* two gene case - revision Apr. 7, 2012 */
							{
							if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[64]+Total_Adult_Genotype_per_House[128]+Total_Adult_Genotype_per_House[192] + Total_Adult_Genotype_per_House[256] + Total_Adult_Genotype_per_House[320] + Total_Adult_Genotype_per_House[384]+Total_Adult_Genotype_per_House[448] + Total_Adult_Genotype_per_House[576] + Total_Adult_Genotype_per_House[512] + Total_Adult_Genotype_per_House[640] + Total_Adult_Genotype_per_House[704] + Total_Adult_Genotype_per_House[832] + Total_Adult_Genotype_per_House[768] + Total_Adult_Genotype_per_House[896] + Total_Adult_Genotype_per_House[960])==0)
								{
								NO_WILD_FEMALES++;
								}
							}

			// Has the wild-type allele among adult females gone extinct in this house? - don't measure for more than two genes as cases get astronomical
			if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48] + Total_Adult_Genotype_per_House[20]+Total_Adult_Genotype_per_House[4]+Total_Adult_Genotype_per_House[36]+Total_Adult_Genotype_per_House[52] + Total_Adult_Genotype_per_House[24]+Total_Adult_Genotype_per_House[8]+Total_Adult_Genotype_per_House[40]+Total_Adult_Genotype_per_House[56])==0)
				{
				NO_WILD_ALLELES++;
				}
		}

	// test larvae age structure
	/*for (int k=0 ; k<this->Number_of_Containers ; k++)
			{
			vector<int> larvae_by_age;
			int maximum_age=0;
			for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Container[k].Larvae.begin(); larvae_cohort_it != this->Container[k].Larvae.end() ; larvae_cohort_it++)
			{
			if (larvae_cohort_it->Age > maximum_age)
			{
			maximum_age = larvae_cohort_it->Age;
			larvae_by_age.resize(maximum_age,0);
			}
			larvae_by_age[larvae_cohort_it->Age - 1]+=(int)larvae_cohort_it->Number;

			}
			int age=0;
			for (vector<int>::iterator this_age=larvae_by_age.begin() ; this_age!=larvae_by_age.end() ; this_age++)
			{
			age++;
			fprintf(test2, "%d %d %d %d %d\n", Date, House_position, k, age, *this_age);
			}
			fprintf(test2, "\n");
			}
	 */

	// debug //


	/*
			for (int j=0;j<this->Number_of_Containers;j++)
			{
			Sherlock[House_position][1]+=this->Container[j].Food ;
			Sherlock[House_position][3]+=((this->Container[j].Surface)*(this->Container[j].Height)) ;
			}

			if (Sherlock[House_position][1]>Sherlock[NUMBER_OF_HOUSES][1])
			Sherlock[NUMBER_OF_HOUSES][1]=Sherlock[House_position][1] ;
			if (Sherlock[House_position][3]>Sherlock[NUMBER_OF_HOUSES][3])
			Sherlock[NUMBER_OF_HOUSES][3]=Sherlock[House_position][3] ;

			Sherlock[House_position][4]=this->Number_of_Containers ;
			Sherlock[House_position][2]=Sherlock[House_position][1]/Sherlock[House_position][4] ;

			if (Sherlock[House_position][2]>Sherlock[NUMBER_OF_HOUSES][2])
			Sherlock[NUMBER_OF_HOUSES][2]=Sherlock[House_position][2] ;
			if (Sherlock[House_position][4]>Sherlock[NUMBER_OF_HOUSES][4])
			Sherlock[NUMBER_OF_HOUSES][4]=Sherlock[House_position][4] ;

			Sherlock[House_position][0] = this->TotalFemalePupae+this->TotalMalePupae ;
			if (Sherlock[House_position][0]>Sherlock[NUMBER_OF_HOUSES][0])
			Sherlock[NUMBER_OF_HOUSES][0]=Sherlock[House_position][0] ;
	 */
	// end of debug //



	//log the final tally into the detailed log file
	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g %s %.8g %s %.8g %s %.8g %s %.8g\n","Total eggs: ",this->TotalEggs,"Total female larvae: ",this->TotalLarvae[0][0]+this->TotalLarvae[0][1]+this->TotalLarvae[0][2]+this->TotalLarvae[0][3],"Total male larvae: ",this->TotalLarvae[1][0]+this->TotalLarvae[1][1]+this->TotalLarvae[1][2]+this->TotalLarvae[1][3],"Total female pupae: ",this->TotalFemalePupae,"Total male pupae: ",this->TotalMalePupae);
	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g %s %.8g %s %.8g\n","Total nulliparous adult females: ",this->TotalFemaleNulliparousAdults,"Total adult parous females: ",this->TotalFemaleParousAdults,"Total adult males: ",this->TotalMaleAdults);
	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s\n","+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");

	//log the final tally into the short log file
	if ((LOG_PER_HOUSE)) // if ((LOG_PER_HOUSE) && (Date%%50)) // Log every 50 days
	{
		//fprintf(Output_per_House,"%d %d %d %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g",Date,House_position,this->Number_of_Containers, this->TotalEggs,this->TotalLarvae[FEMALE],this->TotalLarvae[MALE],this->TotalFemalePupae,this->TotalMalePupae,this->TotalFemaleNulliparousAdults,this->TotalFemaleParousAdults,this->TotalMaleAdults,this->Container[0].Food,this->Container[0].Maximum_Daily_Water_Temperature,this->Container[0].Minimum_Daily_Water_Temperature,Average_Female_Weight,Average_Male_Weight,this->Recaptured_Female,this->Recaptured_Male);
		fprintf(Output_per_House,"%d %d %d %.8g  %.8g %.8g %.8g %.8g  %.8g %.8g %.8g %.8g  %.8g  %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g",Date,House_position,this->Number_of_Containers, this->TotalEggs,this->TotalLarvae[0][0],this->TotalLarvae[0][1],this->TotalLarvae[0][2],this->TotalLarvae[0][3],this->TotalLarvae[1][0],this->TotalLarvae[1][1],this->TotalLarvae[1][2],this->TotalLarvae[1][3], New_Pupae_in_Container, this->TotalFemalePupae,this->TotalMalePupae,this->TotalFemaleNulliparousAdults,this->TotalFemaleParousAdults,this->TotalMaleAdults,Average_Female_Weight,Average_Male_Weight,this->Recaptured_Female,this->Recaptured_Male);
		//for (int cont=0 ; cont<this->Number_of_Containers ; cont++) fprintf(Output_per_House, " %.8g", this->Container[cont].Food) ;
		//fprintf(Output_per_House, " %.3g", Sherlock[House_position][1]) ;
		for (int genotype=0;genotype<number_of_genotypes;genotype++) fprintf(Output_per_House," %.8g",Total_Genotype_per_House[genotype]);
		for (int genotype=0;genotype<number_of_genotypes;genotype++) fprintf(Output_per_House," %.8g",Total_Adult_Genotype_per_House[genotype]);
		for (int allele=0;allele<2*CHROM_NUMBER;allele++) fprintf(Output_per_House," %.8g",allele_frequency_per_House[allele]);
		fprintf(Output_per_House,"\n");
	}

	// output per container

	if ((OUTPUT_PER_CONTAINER) && (Date>=OUTPUT_PER_CONTAINER_START_DATE))
	{
		for (vector<Receptacle>::iterator itCont=this->Container.begin() ; itCont!=this->Container.end() ; itCont++)
		{

			fprintf(Output_per_Container, "%d %d ", Date, this->Position);
			fprintf(Output_per_Container, "%d %.5f %", itCont->ID, itCont->Food);
			fprintf(Output_per_Container, "%d %d ", itCont->Container_Type_Number, itCont->Filling_Method);

			int Eggs_in_container = 0;
			for (list<Eggs_Cohort>::iterator itEggs=itCont->Eggs.begin(); itEggs != itCont->Eggs.end(); itEggs++)
				Eggs_in_container += (int) itEggs->Number;

			int Larvae_in_Container[4];
			for (int inst=0 ; inst<4 ; inst++) Larvae_in_Container[inst]=0;

			for (list<Larvae_Cohort>::iterator itLarvae=itCont->Larvae.begin() ; itLarvae!=itCont->Larvae.end() ; itLarvae++)
				Larvae_in_Container[itLarvae->Instar -1]+=itLarvae->Number;

			int Pupae_in_Container=0;
			for (list<Pupae_Cohort>::iterator itPupae=itCont->Pupae.begin(); itPupae != itCont->Pupae.end() ; itPupae++)
				Pupae_in_Container+=(itPupae->Number);

			fprintf(Output_per_Container, "%d ", Eggs_in_container);
			fprintf(Output_per_Container, "%d %d %d %d ", Larvae_in_Container[0], Larvae_in_Container[1], Larvae_in_Container[2], Larvae_in_Container[3]);
			fprintf(Output_per_Container, "%d ", Pupae_in_Container);
			fprintf(Output_per_Container, "%d %d %d ", (int)(itCont->Total_NewlyPupated[0]+itCont->Total_NewlyPupated[1]), (int)(itCont->Total_NewlyPupated[0]), (int)(itCont->Total_NewlyPupated[1]));
			fprintf(Output_per_Container, "%.6f %.6f ", itCont->Total_NewPupaeWeight[0],itCont->Total_NewPupaeWeight[1]);
			fprintf(Output_per_Container, "%.6f ", itCont->Water_Level);
			fprintf(Output_per_Container, "%.6f ", itCont->Food );
			fprintf(Output_per_Container, "\n");

		}
	}

	// output for Sbeed at the beginning of the day
	if (OUTPUT_FOR_SBEED)
	{
		for (list<Female_Adult>::iterator female_adult_it=this->Female_Adults.begin(); female_adult_it != this->Female_Adults.end(); female_adult_it++)
		{

			fprintf(forSbeed, "%d %d %d %d %.8g %.8g\n", House_position, female_adult_it->ID, female_adult_it->Age, female_adult_it->Genotype, female_adult_it->Average_Adult_Weight, female_adult_it->Physiological_Development_Percent);

		}
	}

	// custom xt output, e.g. for invasion wave representation or adulticidal control

	//if ((TRADITIONAL_CONTROL_METHOD==2)&&(ADULTICIDE_REGION==1))
	//{
	// if (((House_position/NUMBER_OF_COLUMNS)>=xt_selected_line)&&((House_position/NUMBER_OF_COLUMNS)<xt_selected_line+transect_width))
	// {
	//  transect_values[House_position%NUMBER_OF_COLUMNS]+=Total_Mosquitoes;
	// }
	//}

	if ((GENETIC_CONTROL_METHOD==7 || NUMBER_OF_INCOMPATIBLE_WOLBACHIA==1) && EPS_YT_OUTPUT)
	{
		double Total_in_House = this->TotalEggs+this->TotalLarvae[0][0]+this->TotalLarvae[0][1]+this->TotalLarvae[0][2]+this->TotalLarvae[0][3]+this->TotalLarvae[1][0]+this->TotalLarvae[1][1]+this->TotalLarvae[1][2]+this->TotalLarvae[1][3]+this->TotalFemalePupae+this->TotalMalePupae+this->TotalFemaleNulliparousAdults+this->TotalFemaleParousAdults+this->TotalMaleAdults ;

		if (GENETIC_CONTROL_METHOD==7)
			transect_values[House_position/NUMBER_OF_COLUMNS]+=(allele_frequency_per_House[0]*Total_in_House);
		else if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
			transect_values[House_position/NUMBER_OF_COLUMNS]+=(wolbachia_prevalence[0]*Total_in_House);
		transect_densities[House_position/NUMBER_OF_COLUMNS]+=Total_in_House;
		if ((House_position%NUMBER_OF_COLUMNS)==(NUMBER_OF_COLUMNS-1))
		{
			//fprintf(stdout, "%d %.6f %.6f\n", House_position, transect_values[House_position/NUMBER_OF_COLUMNS], transect_densities[House_position/NUMBER_OF_COLUMNS]);
			transect_values[House_position/NUMBER_OF_COLUMNS]/=transect_densities[House_position/NUMBER_OF_COLUMNS];
		}
	}


	// EPS output

	if (((EPS_OUTPUT)&&((Date % EPS_OUTPUT_FREQ)==0) || (Date == 365)) || ((EPS_XT_OUTPUT)&&((House_position/NUMBER_OF_COLUMNS)==xt_selected_line)))

	{
		double Total_in_House = this->TotalEggs+this->TotalLarvae[0][0]+this->TotalLarvae[0][1]+this->TotalLarvae[0][2]+this->TotalLarvae[0][3]+this->TotalLarvae[1][0]+this->TotalLarvae[1][1]+this->TotalLarvae[1][2]+this->TotalLarvae[1][3]+this->TotalFemalePupae+this->TotalMalePupae+this->TotalFemaleNulliparousAdults+this->TotalFemaleParousAdults+this->TotalMaleAdults ;
		// change wild type frequency calculation for MEDEA here
		double wtf = (Total_Genotype_per_House[0]+Total_Genotype_per_House[1])/Total_in_House ;

		double aef=0. ;
		int ae_gen_female=0, ae_gen_male=0;
		if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE)
		{
			ae_gen_female = 20 ;
			ae_gen_male = 21 ;
		}
		else
		{
			ae_gen_female = number_of_genotypes-4 ;
			ae_gen_male = number_of_genotypes-3 ;
		}

		aef = (Total_Genotype_per_House[ae_gen_female]+Total_Genotype_per_House[ae_gen_male])/Total_in_House ;

		if (GENETIC_CONTROL_METHOD==FKR) // for FKR, only plot the individuals carrying rescue+transgene
		{
			aef = 0 ;
			for (int genotype=0 ; genotype<number_of_genotypes ; genotype++)
				if ((genotype>>2)&3u)
					aef+=Total_Genotype_per_House[genotype];
			aef/=Total_in_House;
		}

		if (GENETIC_CONTROL_METHOD==ReduceReplace) // for ReduceReplace, only plot the replacement allele
		{
			aef = 0 ;
			for (int genotype=0 ; genotype<number_of_genotypes ; genotype++)
			{
			if (NUMBER_OF_ADDITIONAL_LOCI)
			{
			for (int locus = 1; locus < NUMBER_OF_ADDITIONAL_LOCI + 1; locus++)
				{
				if ((genotype>>(2*locus))&3u) // Shift the genotype over by 2xchrom. If any of the first two values are equal to one, then add to aef.
					aef+=Total_Genotype_per_House[genotype];
				}
			}
			}
			aef/=Total_in_House;
		}

		wtf = (1-aef) ; // if we want to plot engineered genotypes -- to be improved

		if (GENETIC_CONTROL_METHOD==SELECTION) // plot allele frequency
			wtf = allele_frequency_per_House[0];

		char* c ;

		if (12*wtf < 1) c="0" ;
		else if (12*wtf < 2) c="1" ;
		else if (12*wtf < 3) c="2" ;
		else if (12*wtf < 4) c="3" ;
		else if (12*wtf < 5) c="4" ;
		else if (12*wtf < 6) c="5" ;
		else if (12*wtf < 7) c="6" ;
		else if (12*wtf < 8) c="7" ;
		else if (12*wtf < 9) c="8" ;
		else if (12*wtf < 10) c="9" ;
		else if (12*wtf < 11) c="A" ;
		else if (Total_in_House==0) c="F" ;
		else c="B" ;


		if ((EPS_OUTPUT)&&((Date % EPS_OUTPUT_FREQ)==0))
		{
			switch(GENETIC_CONTROL_METHOD)
			{
			case ENGINEERED_UNDERDOMINANCE:
				switch(EU_TYPE)
				{
				case 1:
					if (wtf==1.)
						fprintf(EPS_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_Output, "000") ;
					else
						fprintf(EPS_Output, "%s%sF", c, c) ;
					break ;
				case 2:fprintf(stdout, "\n*** TEST ***");

					fprintf(EPS_Output, "%sF%s", c, c) ;
					break ;
				case 3:
					fprintf(EPS_Output, "F%s%s", c, c) ;
					break ;
				case 4:
					fprintf(EPS_Output, "F%sF", c) ; break ;
				}
				break;
				case MEIOTIC_DRIVE:
					if (wtf==1.)
						fprintf(EPS_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_Output, "000") ;
					else fprintf(EPS_Output, "%sF%s", c, c) ;
					break;
				case MEDEA:
					if (wtf==1.)
						fprintf(EPS_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_Output, "000") ;
					else fprintf(EPS_Output, "%sF%s", c, c) ;
					break;
				case ReduceReplace:
					if (wtf==1.)
						fprintf(EPS_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_Output, "000") ;
					else fprintf(EPS_Output, "%sF%s", c, c) ;
					break;
				default:
					fprintf(EPS_Output, "%s%s%s", c, c, c) ;
					break ;
			}

			if (!((1+House_position)%NUMBER_OF_COLUMNS))
				if (NUMBER_OF_COLUMNS%2) fprintf(EPS_Output, "FFF\n") ; // correcting for odd number of columns
				else fprintf(EPS_Output, "\n") ;
		}




		if ((EPS_XT_OUTPUT)&&((House_position/NUMBER_OF_COLUMNS)==xt_selected_line))
		{
			switch(GENETIC_CONTROL_METHOD)
			{
			case ENGINEERED_UNDERDOMINANCE:
				switch(EU_TYPE)
				{
				case 1:
					if (wtf==1.)
						fprintf(EPS_XT_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_XT_Output, "000") ;
					else
						fprintf(EPS_XT_Output, "%s%sF", c, c) ;
					break ;
				case 2:
					fprintf(EPS_XT_Output, "%sF%s", c, c) ;
					break ;
				case 3:
					fprintf(EPS_XT_Output, "F%s%s", c, c) ;
					break ;
				case 4:
					fprintf(EPS_XT_Output, "F%sF", c) ; break ;
				}
				break;

				case MEIOTIC_DRIVE:
					if (wtf==1.)
						fprintf(EPS_XT_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_XT_Output, "000") ;
					else fprintf(EPS_XT_Output, "%sF%s", c, c) ;
					break;

				case MEDEA:
					if (wtf==1.)
						fprintf(EPS_XT_Output, "DDD") ;
					else if (wtf==0.)
						fprintf(EPS_XT_Output, "000") ;
					else fprintf(EPS_XT_Output, "%sF%s", c, c) ;
					break;

				default:
					fprintf(EPS_XT_Output, "%s%s%s", c, c, c) ;
					break ;
			}

			if (!((1+House_position)%NUMBER_OF_COLUMNS))
				if (NUMBER_OF_COLUMNS%2) fprintf(EPS_XT_Output, "FFF\n") ; // correcting for odd number of columns
				else fprintf(EPS_XT_Output, "\n") ;
		}

	}

	// EPS Wolbachia output

	if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
		if ((EPS_WOLBACHIA_OUTPUT) && ((Date%EPS_WOLBACHIA_OUTPUT_FREQ)==0))
		{
			double Total_in_House = this->TotalEggs+this->TotalLarvae[0][0]+this->TotalLarvae[0][1]+this->TotalLarvae[0][2]+this->TotalLarvae[0][3]+this->TotalLarvae[1][0]+this->TotalLarvae[1][1]+this->TotalLarvae[1][2]+this->TotalLarvae[1][3]+this->TotalFemalePupae+this->TotalMalePupae+this->TotalFemaleNulliparousAdults+this->TotalFemaleParousAdults+this->TotalMaleAdults ;
			double prev=wolbachia_prevalence[0];

			char* c ;

			if (12*prev < 1) c="E" ;
			else if (12*prev < 2) c="D" ;
			else if (12*prev < 3) c="B" ;
			else if (12*prev < 4) c="A" ;
			else if (12*prev < 5) c="9" ;
			else if (12*prev < 6) c="8" ;
			else if (12*prev < 7) c="7" ;
			else if (12*prev < 8) c="6" ;
			else if (12*prev < 9) c="5" ;
			else if (12*prev < 10) c="4" ;
			else if (12*prev < 11) c="2" ;
			else c="0" ;


			if (Total_in_House==0)
				fprintf(EPS_Wolbachia_Output,"FFF") ;
			else if (prev==0.)
				fprintf(EPS_Wolbachia_Output, "DDD");
			else if (prev==1.)
				fprintf(EPS_Wolbachia_Output, "000");
			else
				fprintf(EPS_Wolbachia_Output, "%s%sF", c, c);

			if (!((1+House_position)%NUMBER_OF_COLUMNS))
				if (NUMBER_OF_COLUMNS%2) fprintf(EPS_Wolbachia_Output, "FFF\n") ; // correcting for odd number of columns
				else fprintf(EPS_Wolbachia_Output, "\n") ;


		}

	// EPS density output

	if ((EPS_DENSITY_OUTPUT) && ((Date%EPS_DENSITY_OUTPUT_FREQ)==0))
	{
		char *c,*d,*e ;
		//double Total_in_House = this->TotalEggs+this->TotalLarvae[FEMALE]+this->TotalLarvae[MALE]+this->TotalFemalePupae+this->TotalMalePupae+this->TotalFemaleNulliparousAdults+this->TotalFemaleParousAdults+this->TotalMaleAdults ;
		double Total_Adults_in_House = this->TotalFemaleNulliparousAdults+this->TotalFemaleParousAdults+this->TotalMaleAdults ;
		double Total_Pupae_in_House = this->TotalFemalePupae+this->TotalMalePupae ;

		int plotstage = 3;
		// 0 for eggs
		// 1 for larvae
		// 2 for pupae
		// 3 for adults
		// 4 for WT adults
		// 5 for WT all stages
		double selected ;

		if (Date==0) // first map is containers
		{
			selected = this->Number_of_Containers;

			if (selected==0) {c="F"; d="F"; e="F";} // white
			else if (selected==1) {c="F"; d="F"; e="5";} // yellow
			else if (selected<4.5) {c="F"; d="A"; e="5";} // light orange
			else if (selected<10.5) {c="F"; d="5"; e="0";} // bright orange
			else if (selected<40.5) {c="C"; d="0"; e="0";} // red
			else {c="5"; d="0"; e="0";} // dark red
			fprintf(EPS_Density_Output, "%s%s%s", c, d, e);

		}

		else // plot pupae or adults
		{
			if (plotstage==2)
				selected = Total_Pupae_in_House ;
			else if (plotstage==5)
				selected = (Total_Genotype_per_House[0]+Total_Genotype_per_House[1]);
			else if (plotstage==4)
				selected = (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[1]);
			else
				selected = Total_Adults_in_House ;

			if (selected==0) {c="F"; d="F"; e="F";}
			else
			{
				if (selected<2.5) {c="B" ; d="F"; e="D";}
				else if (selected<5.5) {c="8" ; d="F"; e="D";}
				else if (selected<10.5) {c="5" ; d="F"; e="D";}
				else if (selected<20.5) {c="2" ; d="F"; e="D";}
				else if (selected<50.5) {c="0" ; d="E"; e="C";}
				else {c="0" ; d="A"; e="8";}
			}

			if (plotstage==2)
				fprintf(EPS_Density_Output, "%s%s%s", d, c, c) ; // red for pupae
			else
				fprintf(EPS_Density_Output, "%s%s%s", d, c, d) ; // purple for adults


		}

		if (!((1+House_position)%NUMBER_OF_COLUMNS)) 
		{

			if (NUMBER_OF_COLUMNS%2) fprintf(EPS_Density_Output, "FFF\n") ; // correcting for odd number of columns		 
			else fprintf(EPS_Density_Output, "\n") ;

		}
	}


	delete[] Total_Genotype_per_House;
	delete[] Total_Adult_Genotype_per_House;
	delete[] allele_frequency_per_House;
	delete[] wolbachia_frequency;
	delete[] wolbachia_prevalence;

	return;

};
