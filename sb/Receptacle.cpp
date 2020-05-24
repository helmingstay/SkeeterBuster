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

extern int Date;
extern FILE *Output, *test1, *larvtime;

// TEMP
extern double tmp_totalpupae;
extern double tmp_avg_weight_pupae;
extern double tmp_totaladults;
extern double tmp_avg_weight_adults;
extern double tmp_totaleggs;
//


extern long ra[M+1], nd;

Receptacle::Receptacle()
{
	this->Container_Type_Number = 0;
	this->House_Indicator = 0;
	this->Release_Date = 0;
	this->Maximum_Daily_Water_Temperature = 0.0;
	this->Minimum_Daily_Water_Temperature = 0.0;
	this->Average_Daily_Water_Temperature = 0.0;
	this->Evaporative_Loss = 0.0;
	this->Water_Level = 0.0; //this is mm
	this->Food = 0.0; //mg
	this->Weight_Cadavers_From_Yesterday = 0.0; //mg
	this->Total_NewlyHatched = 0.0;
	this->Total_NewlyPupated[0] = 0.0;
	this->Total_NewlyPupated[1] = 0.0;
	this->Total_NewPupaeWeight[0] = 0.0;
	this->Total_NewPupaeWeight[1] = 0.0;

	this->Eggs = list<Eggs_Cohort>();
	this->Larvae = list<Larvae_Cohort>();
	this->Pupae = list<Pupae_Cohort>();
}



//#3
//this procedure calculates the weather-driven variables of the container on a given day
void Receptacle::WeatherCalculations(double Air_Minimum, double Air_Maximum, double Sun_Exposure, double RH, double Rain, double WaterShedRatio, double CoverReduction, double Sur_face, double DrawDown, double Top)
{

	this->Maximum_Daily_Water_Temperature=15.033426+0.266995*Air_Minimum+0.007053*Air_Maximum*Air_Maximum+7.685039*Sun_Exposure*Sun_Exposure;
	this->Minimum_Daily_Water_Temperature=5.019385-1.363636*Sun_Exposure+0.807682*Air_Minimum+0.000982*Air_Maximum*Air_Maximum;
	this->Average_Daily_Water_Temperature=(this->Maximum_Daily_Water_Temperature+this->Minimum_Daily_Water_Temperature)/2;
	this->Evaporative_Loss=0.925636+0.275191*Sun_Exposure-0.009041*RH;
	this->Water_Level=this->Water_Level-(this->Evaporative_Loss*(1-CoverReduction));
	//this->Water_Level=this->Water_Level-(DrawDown*1000)/Sur_face;
	this->Water_Level = this->Water_Level*(1-DrawDown);
	if (this->Water_Level<0) this->Water_Level=0;
	// to artificially add rainfall
	//this->Water_Level=this->Water_Level+max(1.,Rain)*0.1*WaterShedRatio;

	// containers with method other than 2 and 3 get filled with rain. If a container is has filling method 3, it gets filled manually at frequency FILLING_FREQUENCY_TYPE_3_CONTAINERS
	 if ((this->Filling_Method!=2)&&(this->Filling_Method!=3))
	 {
     if (!IS_IQUITOS) /* If we are not basing it on the Iquitos dataset */
		 {
    	 this->Water_Level=this->Water_Level+Rain*0.1*WaterShedRatio;
		 }
	 }
	 // if of type 2 or 3
	  else
	  {
		if (this->Filling_Method==3) /* Only manually filled containers of type 3 ever get filled */
		{
		  if (Random_Number()<(1./FILLING_FREQUENCY_TYPE_3_CONTAINERS))
			  {
			  this->Water_Level=Top;
			  }
		}
	  }

	 if (IS_IQUITOS) /* If we are basing it on the Iquitos dataset, even manually filled containers get rainwater; this filling has already been done for type 3 containers (which are absent in Iquitos anyway) so their filling is not repeated */
		 {
		 if (this->Filling_Method!=3)
			 {
			 this->Water_Level=this->Water_Level+Rain*0.1*WaterShedRatio;
			 }
		 }
	 if (this->Water_Level>Top) this->Water_Level=Top;


	//this->Average_Daily_Water_Temperature=FixedWaterAverageTemperature; //to make constant temperature
	/*this->Water_Level=10;
	this->Average_Daily_Water_Temperature=26;
	this->Maximum_Daily_Water_Temperature=26;
	this->Minimum_Daily_Water_Temperature=26;*/

}

//#4
//this procedure calculates the amount of food in the container in the given day
void Receptacle::FoodCalculations(double FoodGain,double FoodFromCadavers,double FoodDecay)
{
	this->Food=this->Food*(1-FoodDecay);
	if (this->Food<0)  this->Food=0;
	this->Food=this->Food+FoodGain+FoodFromCadavers;
};


void Receptacle::ReceptacleCalculations(int Date, double SD, double RH, double Rain, double Air_Maximum, double Air_Minimum, double Air_Average, double Development_26, double Development_134, class Building *building_pointer)
{
	double WeightCadaversFromYesterday;
	pair<double,double> return_value;
	Trio pupae_return_value;
	double Sun_Exposure,WaterShedRatio,CoverReduction,Sur_face,DrawDown,Top;
	double Water_Average_Temp,Water_Minimum_Temp,Water_Maximum_Temp;
	double Water_Depth;
	double FoodGain,FoodFromCadavers,FoodDecay;
	double NewlyHatched,NewlyPupated,NewlyEmerged=0;
	double Larvae_Conversion_rate_of_cadavers_to_food=FEMALE_LARVAE_CONVERSION_RATE_OF_CADAVERS_TO_FOOD; // !!!! CHECK THIS !!!
	double Larval_food,Sum_larval_food_decrease;
	int MoveAve,TempDay;
	double Moving_Average_Max_Temp, Moving_Average_Min_Temp;
	double Total_Eggs_in_Container,Total_Larvae_in_Container,Total_Pupae_in_Container;
	double Maturation;
	double Matured_Larvae_Number, Matured_Pupae_Number;

	// in case of a manually filled container, determine if it is going to be cleaned at the end of the day
	bool emptied=false ;
	if (this->Filling_Method==2)
	{
		  if (IS_IQUITOS) /* carve out exceptions for Container_Type_Number 6 and 8 that appear in Iquitos */
			  {
				  if ((this->Container_Type_Number!=6)&&(this->Container_Type_Number!=8))
				  {
					 if (Random_Number()<PROBABILITY_OF_EMPTYING_MANUALLY_FILLED_CONTAINERS)
						 emptied=true;
				  }
			  }
		  else
			  {
			  if (Random_Number()<PROBABILITY_OF_EMPTYING_MANUALLY_FILLED_CONTAINERS)
				  emptied = true;
			  }
	}
	//#3
	//the characteristics of the particular container are set based on its container type

	Sun_Exposure=this->SunExposure;
	WaterShedRatio=this->Watershed_Ratio;
	CoverReduction=this->Cover_Reduction;
	Sur_face=this->Surface;
	DrawDown=this->Draw_Down;
	Top=this->Height;

	//the water temperature and depth is calculated for the container
	MoveAve=1;
	if ((Sur_face*Top)>5000) MoveAve=2;
	if ((Sur_face*Top)>100000) MoveAve=3;
	if ((Sur_face*Top)>500000) MoveAve=4;

	if ((MoveAve==1)||(((Date-MoveAve+1)<0)&&(NUMBER_OF_DAYS<365)))
	{
		this->WeatherCalculations(Air_Minimum,Air_Maximum,Sun_Exposure,RH,Rain,WaterShedRatio,CoverReduction,Sur_face,DrawDown,Top);
	}
	else
	{
		Moving_Average_Max_Temp=0;
		Moving_Average_Min_Temp=0;
		if ((Date-MoveAve+1)<0)
			// for earlier time steps values for days -1, -2, ... are taken one year later
			// provided simulation runs at least one year (otherwise, no moving average is calculated)
		{
			TempDay=365-MoveAve+Date+1;
			for (int i=TempDay;i<365;i++) Moving_Average_Max_Temp+=15.033426+0.266995*building_pointer->Minimum_Daily_Air_Temperature[i]+0.007053*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i]+7.685039*Sun_Exposure*Sun_Exposure;
			for (int i=0;i<Date+1;i++) Moving_Average_Max_Temp+=15.033426+0.266995*building_pointer->Minimum_Daily_Air_Temperature[i]+0.007053*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i]+7.685039*Sun_Exposure*Sun_Exposure;
			for (int i=TempDay;i<365;i++) Moving_Average_Min_Temp+=5.019385-1.363636*Sun_Exposure+0.807682*building_pointer->Minimum_Daily_Air_Temperature[i]+0.000982*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i];
			for (int i=0;i<Date+1;i++) Moving_Average_Min_Temp+=5.019385-1.363636*Sun_Exposure+0.807682*building_pointer->Minimum_Daily_Air_Temperature[i]+0.000982*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i];
		}
		else
		{
			for (int i=Date-MoveAve+1;i<Date+1;i++) Moving_Average_Max_Temp+=15.033426+0.266995*building_pointer->Minimum_Daily_Air_Temperature[i]+0.007053*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i]+7.685039*Sun_Exposure*Sun_Exposure;
			for (int i=Date-MoveAve+1;i<Date+1;i++) Moving_Average_Min_Temp+=5.019385-1.363636*Sun_Exposure+0.807682*building_pointer->Minimum_Daily_Air_Temperature[i]+0.000982*building_pointer->Maximum_Daily_Air_Temperature[i]*building_pointer->Maximum_Daily_Air_Temperature[i];
		}
		this->WeatherCalculations(Air_Minimum,Air_Maximum,Sun_Exposure,RH,Rain,WaterShedRatio,CoverReduction,Sur_face,DrawDown,Top);
		this->Maximum_Daily_Water_Temperature=Moving_Average_Max_Temp/MoveAve;
		this->Minimum_Daily_Water_Temperature=Moving_Average_Min_Temp/MoveAve;
		this->Average_Daily_Water_Temperature=(this->Maximum_Daily_Water_Temperature+this->Minimum_Daily_Water_Temperature)/2;
	}



	Water_Depth=this->Water_Level;
	Water_Average_Temp=this->Average_Daily_Water_Temperature;
	Water_Maximum_Temp=this->Maximum_Daily_Water_Temperature;
	Water_Minimum_Temp=this->Minimum_Daily_Water_Temperature;
	//printout of the water container weather-driven variables into the detailed log file
	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output," %s %.8g %s %.8g %s %.8g %s %.8g\n"," Water_depth: ",Water_Depth," Water_Average_Temp: ",Water_Average_Temp, " Water_Max_Temp: ",Water_Maximum_Temp," Water_Min_Temp: ",Water_Minimum_Temp);

	//#4
	//the following routine mimics the procedures in CIMSIM
	//fixed: adds food into the container every 3 days (3*daily_amount) or 1 day (1*daily_amount)
	//random: adds food every day (amount between 0 and 2*daily)


	if (FOOD_INPUT_METHOD==3)
	{
		if (!(Date%3))	
			FoodGain=3*(this->Daily_Food_Gain);
		else FoodGain=0;
	}
	else if (FOOD_INPUT_METHOD==1) FoodGain=this->Daily_Food_Gain;
	else FoodGain=2*Daily_Food_Gain*Random_Number();

	if (Date==this->Release_Date) FoodGain+=this->Initial_Food_Present;


	FoodFromCadavers=this->Weight_Cadavers_From_Yesterday*Larvae_Conversion_rate_of_cadavers_to_food;
	// debug
	if (Weight_Cadavers_From_Yesterday<0)
		{
		fprintf(stdout, "*** HERE ***\n") ;
		}
	//
	this->Weight_Cadavers_From_Yesterday=0;
	FoodDecay=this->Daily_Food_Loss;
	this->FoodCalculations(FoodGain,FoodFromCadavers,FoodDecay);

	//#5+#6
	//the following procedure calculates the amount of newly hatched eggs, and generally handles egg cohorts
	this->Total_NewlyHatched=0; //sum of all eggs hatched on the day in that container

	int number_of_genotypes;
	number_of_genotypes=(int) pow((double)2,2*CHROM_NUMBER);

	int number_of_wolbachia_status;
	if (!NUMBER_OF_INCOMPATIBLE_WOLBACHIA) number_of_wolbachia_status = 1; // just making sure that 2^0=1
	else number_of_wolbachia_status = (int) pow((double)2,NUMBER_OF_INCOMPATIBLE_WOLBACHIA);

	//////////
	// Old method was to create an array with all possible larval cohort types (by genotype and wolbachia)
	// so that egg cohorts with similar types could be merged into a single larval cohort
	// Inconvenient: very time consuming to create that array for every container every day.
	//
	// New method: 1 egg cohort = 1 larval cohort
	// Inconvenient: more larval cohorts
	// Improves speed when the number of chromosomes is high,


	//double ***new_larval_cohort_number = new double** [number_of_genotypes] ;
	//for (int g=0 ; g<number_of_genotypes ; g++)
	//{
	// new_larval_cohort_number[g] = new double*[number_of_wolbachia_status];
	// for (int w=0 ; w<number_of_wolbachia_status ; w++)
	// {
	//	 new_larval_cohort_number[g][w] = new double[2];
	//	 new_larval_cohort_number[g][w][0] = 0.;
	//	 new_larval_cohort_number[g][w][1] = 0.;
	// }
	//}
	// 2 columns, [0] is for number, [1] is for age;


	for (list<Eggs_Cohort>::iterator eggs_cohort_it=this->Eggs.begin(); eggs_cohort_it != this->Eggs.end() ; ) //start of egg cohort loop
		// eggs_cohort_it will be increased at the end of the loop
	{
		NewlyHatched = eggs_cohort_it->Egg_Calculations( Air_Maximum, Air_Minimum, Air_Average, Water_Maximum_Temp, Water_Minimum_Temp, Water_Depth, Sun_Exposure, SD, Water_Average_Temp);
		//#7
		//placing the new larvae into the larval cohorts

		if (NewlyHatched>0)
		{
			//new_larval_cohort_number[eggs_cohort_it->Genotype][eggs_cohort_it->Wolbachia][0]+=NewlyHatched;
			//new_larval_cohort_number[eggs_cohort_it->Genotype][eggs_cohort_it->Wolbachia][1]+=NewlyHatched*eggs_cohort_it->Age;

			Larvae_Cohort new_larval_cohort(((eggs_cohort_it->Genotype)%2==1), NewlyHatched, SKEETER_BUSTER_FEMALE_LARVAE_WEIGHT_AT_HATCH, 0, eggs_cohort_it->Age, 1, eggs_cohort_it->Genotype, eggs_cohort_it->Wolbachia); 
			this->Larvae.push_back(new_larval_cohort);

		}





		//this procedure makes sure that if all eggs hatch from an egg cohort, then it gets cleared
		if (NewlyHatched >= eggs_cohort_it->Number) // !!!!!!!!!!!! Can this ever be > ???? [rounding error?]
		{
			eggs_cohort_it=this->Eggs.erase(eggs_cohort_it); // this leaves eggs_cohort_it pointing to the next cohort, so don't increment eggs_cohort_it
		}
		else
		{
			eggs_cohort_it->Number -= NewlyHatched; //this adjusts the number of eggs in the egg cohort by the number of hatching eggs

			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %d %s %.8g %s %d %s %.8g %s %d %s %.8g %s %.8g %s %d\n","Egg cohort #",-999," Number: ",eggs_cohort_it->Number," Age: ",eggs_cohort_it->Age," Phys. Dev.: ",eggs_cohort_it->Physiological_Development_Percent," Mature: ",eggs_cohort_it->Mature," Food in container: ",this->Food," Level: ",eggs_cohort_it->Level_laid_in_Container," Genotype: ",eggs_cohort_it->Genotype);

			eggs_cohort_it++; // remember, only increment if we have a non-empty cohort
		}



	} //end of egg cohort loop


	// this is the old method of pushing new larval cohorts

	//double initialweight;
	//if (SIMULATION_MODE1==CIMSIM) initialweight=CIMSIM_FEMALE_LARVAE_WEIGHT_AT_HATCH; else initialweight=SKEETER_BUSTER_FEMALE_LARVAE_WEIGHT_AT_HATCH;

	//for (int genotype=0;genotype<number_of_genotypes;genotype++)
	//{
	//	for (int w=0 ; w<number_of_wolbachia_status ; w++)
	//	{
	//		if (new_larval_cohort_number[genotype][w][0]>0)
	//		{
	//			new_larval_cohort_number[genotype][w][1] /= new_larval_cohort_number[genotype][w][0] ; //gives average age of hatching eggs
	//			Larvae_Cohort new_larval_cohort(FEMALE,new_larval_cohort_number[genotype][w][0],initialweight,0,new_larval_cohort_number[genotype][w][1],1,genotype,w);
	//			if (genotype&1) new_larval_cohort.Sex=MALE;
	//			this->Larvae.push_back(new_larval_cohort);
	//		}
	//	}
	//}


	//for (int g=0 ; g<number_of_genotypes ; g++)
	//{
	//	for (int w=0 ; w<number_of_wolbachia_status ; w++)
	//		delete[] new_larval_cohort_number[g][w];
	//	delete[] new_larval_cohort_number[g];
	//}
	//delete[] new_larval_cohort_number;





	//this is where the larval calculations start
	Larval_food=this->Food; //local variable for larval food in the container trasnferred to a global variable
	WeightCadaversFromYesterday=0; //weight of cadavers for the next day in this container


	//calculation of physiological developmental percent increase and maturation for larval cohorts
	//#8

	for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end() ; larvae_cohort_it++)
	{
		if (SIMULATION_MODE1==CIMSIM) larvae_cohort_it->Larval_Pupation_Weight_Calculation(Water_Average_Temp); //#8 //ORIGINAL CIMSIM
		Maturation=larvae_cohort_it->Larvae_Calculations(Water_Average_Temp,Development_26,Development_134); //#8
		//Maturation=0; //this switches the variation in the larval maturation on and off
		if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(!larvae_cohort_it->Mature)&&(Maturation>0))
		{
			if (SIMULATION_MODE2==DETERMINISTIC) Matured_Larvae_Number=larvae_cohort_it->Number*Maturation;
			if (SIMULATION_MODE2==STOCHASTIC) Matured_Larvae_Number=Binomial_Deviate(larvae_cohort_it->Number,Maturation);
			Larvae_Cohort new_mature_cohort(larvae_cohort_it->Sex,Matured_Larvae_Number,larvae_cohort_it->Average_Larval_Weight,larvae_cohort_it->Age, larvae_cohort_it->Age_since_hatch,larvae_cohort_it->Instar,larvae_cohort_it->Genotype,larvae_cohort_it->Wolbachia);
			new_mature_cohort.Average_Lipid_Reserve=larvae_cohort_it->Average_Lipid_Reserve;
			new_mature_cohort.Larvae_Chronological_basis=larvae_cohort_it->Larvae_Chronological_basis;
			new_mature_cohort.Mature=1;
			new_mature_cohort.Prefasting_Larval_Weight=larvae_cohort_it->Prefasting_Larval_Weight;
			new_mature_cohort.Prefasting_Lipid_Reserve=larvae_cohort_it->Prefasting_Lipid_Reserve;
			new_mature_cohort.Previous_Day_Fasting_Survival=larvae_cohort_it->Previous_Day_Fasting_Survival;
			new_mature_cohort.Previous_Day_Larval_Weight=larvae_cohort_it->Previous_Day_Larval_Weight;
			new_mature_cohort.Previous_Larval_Weight=larvae_cohort_it->Previous_Larval_Weight;
			new_mature_cohort.Pupation_Weight=larvae_cohort_it->Pupation_Weight;
			new_mature_cohort.Physiological_Development_Percent=larvae_cohort_it->Physiological_Development_Percent;
			this->Larvae.push_front(new_mature_cohort);

			larvae_cohort_it->Number-=Matured_Larvae_Number;
		}
	}

	//Euler integration of the Gilpin-McClelland equations
	//#9

	for (int k=0;k<LARVAE_NUMBER_OF_EULER_STEPS;k++)  // !!! DO WE ASSUME SAME NUMBER OF STEPS FOR BOTH????
	{
		Sum_larval_food_decrease=0; //initialize counter for total food consumption
		//one set of the Gilpin and McClelland equations calculated for all larval cohorts

		for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end() ; larvae_cohort_it++)
		{
			Sum_larval_food_decrease += larvae_cohort_it->Larval_growth(Larval_food, this->Surface*this->Height); //actual calculation of the Gilpin-McClelland equations
		}


		//adjustment of the amount of larval food in the container based on total food consumption
		Larval_food=Larval_food-Sum_larval_food_decrease;
		if (Larval_food<0) Larval_food=0;
		this->Food=Larval_food;
	}
	//end of the Euler integration cycle

	//calculating survival for larval cohorts //#10

	for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end() ; larvae_cohort_it++ )
	{
		this->Weight_Cadavers_From_Yesterday+=larvae_cohort_it->Larval_survival(Water_Maximum_Temp,Water_Minimum_Temp,Water_Depth);
	}
	if (Weight_Cadavers_From_Yesterday<0)
			{
			fprintf(stdout, "*** HERE ***\n") ;
			}
	// now done in above loop

	//calculating the amount of new pupae, based on the CIMSIM model, with average pupal weight //#12
	WeightCadaversFromYesterday=0;

	// ############################## THE WAY WE DO THIS: HAVE AN ARRAY OF SIZE 2, and use .Sex as an index for this array (or .Sex==FEMALE)


	for (int k=0;k<2;k++)
	{
		this->Total_NewlyPupated[k]=0; //total number of new pupae // Need to track male, female separately.
		this->Total_NewPupaeWeight[k]=0; //total weight of new pupae
	}

	for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end() ; )   // larvae_cohort_it++ done at end
	{
		//OUR CHANGE no. 2. larval pupation and instar transition weight calculation using the physiological developmental percent at the end of the day instead of the beginning of the day
		if (SIMULATION_MODE1==SKEETER_BUSTER) larvae_cohort_it->Larval_Pupation_Weight_Calculation(Water_Average_Temp); //#11

		// calculate instar
		// note the 'else' structure that prevent larvae from moving more than one instar up each day
		// note also that larvae cannot move back instars

		if (larvae_cohort_it->Instar==0)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_IV)
				larvae_cohort_it->Instar=4;
			else if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_III)
				larvae_cohort_it->Instar=3;
			else if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_II)
				larvae_cohort_it->Instar=2;
			else larvae_cohort_it->Instar=1;
		}

		else if (larvae_cohort_it->Instar<1.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_II)
				larvae_cohort_it->Instar=2;
		}
		else if (larvae_cohort_it->Instar<2.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_III)
				larvae_cohort_it->Instar=3;
		}
		else if (larvae_cohort_it->Instar<3.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_IV)
				larvae_cohort_it->Instar=4;
		}

		// check for pupating larvae
		return_value=larvae_cohort_it->Larval_pupation();  


		NewlyPupated=return_value.first;
		this->Weight_Cadavers_From_Yesterday+=return_value.second; //weight of cadavers updated for next day
		if (Weight_Cadavers_From_Yesterday<0)
				{
				fprintf(stdout, "*** HERE ***\n") ;
				}
		int sex=larvae_cohort_it->Sex; // figure out whether cohort is male or female
		this->Total_NewlyPupated[sex]+=NewlyPupated; //increasing total number of new female pupae
		this->Total_NewPupaeWeight[sex]+=larvae_cohort_it->Previous_Day_Larval_Weight*NewlyPupated; //increasing total weight of new female pupae

		//OUR CHANGE #1, pupal weight is the average of the larval weight at the beginning and at the end of the day when the larvae pupate
		//Pupae_Cohort new_pupae_cohort( sex, NewlyPupated,(larvae_cohort_it->Previous_Day_Larval_Weight+larvae_cohort_it->Average_Larval_Weight)/2 );



		//CIMSIM ORIGINAL, Pupal weight is larval weight at the beginning of the day when the larvae pupate
		//this is to put pupae that come from individual larval cohorts in separate pupal cohorts //COMMON CHANGE
		if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(NewlyPupated>0))
		{
			double newpupae_weight;
			if (!sex) newpupae_weight=(larvae_cohort_it->Average_Larval_Weight+larvae_cohort_it->Previous_Day_Larval_Weight)/2;
			else newpupae_weight=larvae_cohort_it->Previous_Day_Larval_Weight;
			Pupae_Cohort new_pupae_cohort( (sex!=0), NewlyPupated,newpupae_weight,larvae_cohort_it->Genotype, larvae_cohort_it->Wolbachia, larvae_cohort_it->Age_since_hatch);
			this->Pupae.push_back(new_pupae_cohort);
			//fprintf(larvtime, "%d %.3g %d %.8g\n", Date, NewlyPupated, larvae_cohort_it->Age, larvae_cohort_it->Average_Larval_Weight);
		}

		if (larvae_cohort_it->Number==0)
		{
			larvae_cohort_it=this->Larvae.erase(larvae_cohort_it);
		}
		else
		{
			larvae_cohort_it++;
		}
	}


	// put the newly pupated males and females (if there are any) into new pupal cohorts //#13

	//this is needed if all larvae that pupate on the same day in the same container all go into the
	//same pupal cohort with an average weight //ORIGINAL CIMSIM 
	if (SIMULATION_MODE1==CIMSIM)
	{
		for (int k=0;k<2;k++)
		{
			if (this->Total_NewlyPupated[k]>0)
			{
				Pupae_Cohort new_pupae_cohort( (k!=0), this->Total_NewlyPupated[k],this->Total_NewPupaeWeight[k]/this->Total_NewlyPupated[k], k, 0, 0);
				// age_since_hatch not available here, average age of pupae put into the new cohort would have to be calculated
				this->Pupae.push_back(new_pupae_cohort);

			}
		}
	}



	//detailed logging of larval cohort properties

	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END))
	{
		for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end(); larvae_cohort_it++ )
		{
			fprintf(Output,"%s %d %s %i %s %.8g %s %d %s %.3g %s %d %s %.8g %s %d %s %.8g %s %.8g %s %.8g %s %.8g %s %.8g %s %d\n","larva cohort #",-999," Sex ",larvae_cohort_it->Sex,", Number: ",larvae_cohort_it->Number," Age: ",larvae_cohort_it->Age," Age since hatch: ",larvae_cohort_it->Age_since_hatch," Instar : ",larvae_cohort_it->Instar," Phys. Dev.: ",larvae_cohort_it->Physiological_Development_Percent," Mature: ",larvae_cohort_it->Mature," Ave. Weight: ",larvae_cohort_it->Average_Larval_Weight," Ave. Lipid Reserve: ",larvae_cohort_it->Average_Lipid_Reserve," Previous Weight: ",larvae_cohort_it->Previous_Larval_Weight," Prefasting Weight: ",larvae_cohort_it->Prefasting_Larval_Weight," Larval_food: ",this->Food," Genotype: ",larvae_cohort_it->Genotype);
		}
	}


	//this is the end of the larval calculations



	//this is the start of the pupal cohort calculations //#14,#15,#16
	WeightCadaversFromYesterday=0;

	for ( list<Pupae_Cohort>::iterator pupae_cohort_it=this->Pupae.begin(); pupae_cohort_it != this->Pupae.end() ;  ) // pupae_cohort_it++ done at end of loop
	{

		pupae_return_value=pupae_cohort_it->Pupae_Calculations(Air_Maximum,Air_Minimum,Water_Maximum_Temp, Water_Minimum_Temp,Water_Average_Temp);
		Maturation=pupae_return_value.Maturation;
		//Maturation=0; //this switches on and off the pupal maturation variation
		if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(!pupae_cohort_it->Mature)&&(Maturation>0))
		{
			if (SIMULATION_MODE2==DETERMINISTIC) Matured_Pupae_Number=pupae_cohort_it->Number*Maturation;
			if (SIMULATION_MODE2==STOCHASTIC) Matured_Pupae_Number=Binomial_Deviate(pupae_cohort_it->Number,Maturation);
			if (Matured_Pupae_Number>0)
			{
				Pupae_Cohort new_mature_cohort(pupae_cohort_it->Sex,Matured_Pupae_Number,pupae_cohort_it->Average_Pupal_Weight,pupae_cohort_it->Genotype, pupae_cohort_it->Wolbachia, pupae_cohort_it->Age_since_hatch);
				new_mature_cohort.Age=pupae_cohort_it->Age;
				new_mature_cohort.Average_Pupal_Lipid_Reserve=pupae_cohort_it->Average_Pupal_Lipid_Reserve;
				new_mature_cohort.Mature=1;
				new_mature_cohort.Physiological_Development_Percent=pupae_cohort_it->Physiological_Development_Percent;
				this->Pupae.push_front(new_mature_cohort);

				pupae_cohort_it->Number-=Matured_Pupae_Number;
			}
		}

		NewlyEmerged=pupae_return_value.NewlyEmerged;
		this->Weight_Cadavers_From_Yesterday+=pupae_return_value.WeightCadavers; //update the weight of cadavers for the next day
		if (Weight_Cadavers_From_Yesterday<0)
				{
				fprintf(stdout, "*** HERE ***\n") ;
				}
		int sex=pupae_cohort_it->Sex;

		building_pointer->Total_NewAdultNumber[sex] += NewlyEmerged; //increase the number of new female adults by those that emerge from the female pupal cohort on that day
		building_pointer->Total_NewAdultWeight[sex] += pupae_cohort_it->Average_Pupal_Weight*NewlyEmerged;	//increase the total weight of new female adult that emerge from the female pupal cohort on that day

		//COMMON CHANGES no. 4 //this part creates a separate adult female/male cohort for any pupal cohort that emerges instead of merging and averaging them //#17,#18
		if (SIMULATION_MODE1==SKEETER_BUSTER)
		{
			if (sex)
			{
				Male_Adult_Cohort new_male_adult_cohort(NewlyEmerged,pupae_cohort_it->Average_Pupal_Weight*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT,pupae_cohort_it->Genotype, pupae_cohort_it->Wolbachia, 0, 0, pupae_cohort_it->Age_since_hatch, 0, 0);
				building_pointer->Male_Adult.push_back(new_male_adult_cohort);
				//tmp_totaladults+=NewlyEmerged;
			}
			else
				for (int f=0 ; f<NewlyEmerged ; f++,tmp_totaladults++)
				{
					Female_Adult new_female_adult( NULLIPAROUS, pupae_cohort_it->Average_Pupal_Weight*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT,pupae_cohort_it->Average_Pupal_Weight*FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT*FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT,pupae_cohort_it->Genotype, pupae_cohort_it->Wolbachia,0,0,pupae_cohort_it->Age_since_hatch,0,0,false,0,0);
					new_female_adult.Emergence_Place = building_pointer->Position;
					new_female_adult.Source_Container_Type = this->Container_Type_Number;

					building_pointer->Female_Adults.push_back(new_female_adult);
				}
		}

		if (pupae_cohort_it->Number==0)
		{
			pupae_cohort_it=this->Pupae.erase(pupae_cohort_it);
		}
		else
		{
			pupae_cohort_it++;
		}
	}



	//log the properties of the pupal cohorts into the detailed log

	for ( list<Pupae_Cohort>::iterator pupae_cohort_it=this->Pupae.begin(); pupae_cohort_it != this->Pupae.end() ; pupae_cohort_it++)
	{
		if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %d %s %i %s %.8g %s %d %s %.3g %s %.8g %s %d %s %.8g %s %.8g %s %d\n","Pupae cohort #",-999," sex ",pupae_cohort_it->Sex," Number: ",pupae_cohort_it->Number," Age: ",pupae_cohort_it->Age," Age since hatch: ",pupae_cohort_it->Age_since_hatch," Phys. Dev.: ",pupae_cohort_it->Physiological_Development_Percent," Mature: ",pupae_cohort_it->Mature," Ave. Weight: ",pupae_cohort_it->Average_Pupal_Weight," Ave. Lipid Reserve: ",pupae_cohort_it->Average_Pupal_Lipid_Reserve," Genotype: ",pupae_cohort_it->Genotype);
	}
	//this is the end of the pupal cohort calculations

	// cleaning manually filled containers

	if (this->Filling_Method==2) 
		if (emptied)
		{
			for (list<Eggs_Cohort>::iterator eggs_cohort_it=this->Eggs.begin(); eggs_cohort_it != this->Eggs.end() ; ) 
				eggs_cohort_it=this->Eggs.erase(eggs_cohort_it);

			for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end(); )
				larvae_cohort_it=this->Larvae.erase(larvae_cohort_it);

			for ( list<Pupae_Cohort>::iterator pupae_cohort_it=this->Pupae.begin(); pupae_cohort_it != this->Pupae.end() ; )
				pupae_cohort_it=this->Pupae.erase(pupae_cohort_it);

			this->Water_Level=Top;
			this->Food=this->Initial_Food_Present ;

		}

	Total_Eggs_in_Container=0;
	for (list<Eggs_Cohort>::iterator eggs_cohort_it=this->Eggs.begin(); eggs_cohort_it != this->Eggs.end(); eggs_cohort_it++ )
	{
		Total_Eggs_in_Container+=eggs_cohort_it->Number;
	}


	Total_Larvae_in_Container=0;
	for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Larvae.begin(); larvae_cohort_it != this->Larvae.end(); larvae_cohort_it++ )
	{
		Total_Larvae_in_Container+=larvae_cohort_it->Number;
	}

	Total_Pupae_in_Container=0;
	for ( list<Pupae_Cohort>::iterator pupae_cohort_it=this->Pupae.begin(); pupae_cohort_it != this->Pupae.end() ; pupae_cohort_it++)
	{
		Total_Pupae_in_Container+=pupae_cohort_it->Number;
	}

	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g %s %.8g %s %.8g\n","Total eggs in container:",Total_Eggs_in_Container,"Total larvae in container:",Total_Larvae_in_Container,"Total pupae in container:",Total_Pupae_in_Container);

	if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s\n","---------------------------------------------------------------------------------");

	// TEMP: checking the dynamics in one single container
	//fprintf(test1, "%d %.3f %.3f %.3f  %.3f\n", Date, Total_Eggs_in_Container, Total_Larvae_in_Container, Total_Pupae_in_Container, this->Total_NewlyPupated[0]+this->Total_NewlyPupated[1]);
	//tmp_totalpupae+=this->Total_NewlyPupated[0]+this->Total_NewlyPupated[1];
	//tmp_avg_weight_pupae+=this->Total_NewPupaeWeight[0]+this->Total_NewPupaeWeight[1];
	//tmp_avg_weight_adults+=building_pointer->Total_NewAdultWeight[0]+building_pointer->Total_NewAdultWeight[1];
	//if (Date==NUMBER_OF_DAYS-1) fprintf(test1, "%.0f %.0f %.0f %.3f %.3f\n", tmp_totaleggs, tmp_totalpupae, tmp_totaladults, tmp_avg_weight_pupae/tmp_totalpupae, tmp_avg_weight_adults/tmp_totaladults);
	//


};		       
