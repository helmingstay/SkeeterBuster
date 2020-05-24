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


extern long ra[M+1], nd;
extern int number_of_medea_masks;
extern int Date ;


Eggs_Cohort::Eggs_Cohort()
{
  this->Number = 0.0;
  this->Level_laid_in_Container = 0.0;
  this->Physiological_Development_Percent = 0;
  this->Age = 0;
  this->Medea_Mom = 0;
  this->Mature = false;
  this->Wolbachia=0;
}

Eggs_Cohort::Eggs_Cohort( double num, double level, double dev, int age, bool mature, int genotype, int wolbachia, int medea_mom )
{
  this->Number = num;
  this->Level_laid_in_Container = level;
  this->Physiological_Development_Percent = dev;
  this->Age = age;
  this->Mature = mature;
  this->Genotype=genotype;
  this->Wolbachia=wolbachia;
  this->Medea_Mom=medea_mom;
}

double Eggs_Cohort::Egg_Calculations( double Air_Maximum, double Air_Minimum, double Air_Average, double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Depth, double Sun_Exposure, double SD, double Water_Average_Temp)
{
	double Nominal_Survival;
	double Temperature_Survival;
	double Dessication_Survival;
	double Predation_Survival;
	double Fitness_Survival;
	double Overall_Survival;
	double NewlyHatched=0;
	double Hatch_Probability=0; 
	int EggHatch_Level; //level in the container where eggs still hatch
	
	
	if (this->Number==0)
		return 0; //this quits the procedure if the cohort is empty
	
	//clearing of variables

	Nominal_Survival=Temperature_Survival=Dessication_Survival=Predation_Survival=Fitness_Survival=Overall_Survival=1;
	
	//#5
	//nominal egg survival
	Nominal_Survival=NOMINAL_DAILY_SURVIVAL_EGGS;

	//temperature related mortality, based on CIMSIM code
	if (Water_Maximum_Temp>=EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT) Temperature_Survival=EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE;
	if ((Water_Maximum_Temp<EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT)&&(Water_Maximum_Temp>=EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)) Temperature_Survival=1-(1-EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE)/(EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT-EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)*(Water_Maximum_Temp-EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT);

	if (Water_Minimum_Temp<=EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT) Temperature_Survival=Temperature_Survival*EGG_DAILY_SURVIVAL_LOW_TEMPERATURE;
	if ((Water_Minimum_Temp>EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)&&(Water_Minimum_Temp<=EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT)) Temperature_Survival=Temperature_Survival*(1-(1-EGG_DAILY_SURVIVAL_LOW_TEMPERATURE)/(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)*(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-Water_Minimum_Temp));	
	// CORRECTED (see larvae)

	if (Water_Depth==0)
	{
		if (Air_Maximum>=EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT) Temperature_Survival=EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE;
		if (Air_Minimum<=EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT) Temperature_Survival=EGG_DAILY_SURVIVAL_LOW_TEMPERATURE;
		if ((Air_Maximum<EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT)&&(Air_Maximum>=EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)) Temperature_Survival=1-(1-EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE)/(EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT-EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)*(Air_Maximum-EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT);
		if ((Air_Minimum>EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)&&(Air_Minimum<=EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT)) Temperature_Survival=1-(1-EGG_DAILY_SURVIVAL_LOW_TEMPERATURE)/(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)*(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-Air_Minimum);
	}
	
	
	//dessication related mortality
	if (Water_Depth==0)
	{
		if (Sun_Exposure>EGG_DAILY_SURVIVAL_DRY_CONTAINER_SUNEXPOSURE_LIMIT) Dessication_Survival=EGG_DAILY_SURVIVAL_DRY_CONTAINER_HIGH_SUNEXPOSURE;
		if (Sun_Exposure<=EGG_DAILY_SURVIVAL_DRY_CONTAINER_SUNEXPOSURE_LIMIT) 
		{
			if (SD<=EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT) Dessication_Survival=EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT;
			if (SD>=EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT_LIMIT) Dessication_Survival=EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT;
			if ((SD>EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT)&&(SD<EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT_LIMIT)) Dessication_Survival=EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT-((EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT-EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT)/(EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT_LIMIT-EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT))*(SD-EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT);
		}
	}
	else Dessication_Survival=1;
	
	//mortality due to predation, based on CIMSIM code
	if (Water_Average_Temp>=EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT) Predation_Survival=EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE;
	if ((Water_Average_Temp<EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT)&&(Water_Average_Temp>EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT)) Predation_Survival=1-(1-EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE)/(EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT-EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT)*(Water_Average_Temp-EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT);
	if (Water_Average_Temp<EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT) Predation_Survival=1;
	
	if (Water_Depth==0)
	{
		if (Air_Average>=EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT) Predation_Survival=EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE;
		if ((Air_Average<EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT)&&(Air_Average>EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT)) Predation_Survival=1-(1-EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE)/(EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT-EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT)*(Air_Average-EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT);
		if (Air_Average<EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT) Predation_Survival=1;
	}
	
	if ((FITNESS_COST_STAGE==0)&&(EMBRYONIC_COST_SELECTION_TYPE==HARD))
		// case of embryonic fitness costs with hard selection
		if ((this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE)&&(Date!=0)) 
		{
			if (GENETIC_CONTROL_METHOD==MEDEA)
			{	
				Fitness_Survival = Fitness_Calculation_Medea(this->Genotype, this->Medea_Mom) ;
				this->Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);
				this->Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
			}
		}

	// For reduce and replace, have a separate fitness cost that applies to the egg stage.
	if (GENETIC_CONTROL_METHOD==ReduceReplace)
	{
	if ((FITNESS_COST_STAGE_RR==0)&&(EMBRYONIC_COST_SELECTION_TYPE==HARD))
		// case of embryonic fitness costs with hard selection
		if ((this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE)&&(Date!=0))
		{
			Fitness_Survival=Fitness_Calculation_RR(this->Genotype);
			this->Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
		}
	}

	// wolbachia egg survival -- affects Dessication_Survival factor

	if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA)
		if (this->Wolbachia) // no effect of multiple infections
			if (this->Age > WOLBACHIA_EGG_SURVIVAL_REDUCTION_ONSET_AGE)
				Dessication_Survival *= WOLBACHIA_EGG_SURVIVAL_REDUCTION_FACTOR;


	//overall survival
	Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival*Predation_Survival;
	//stochastic implementation of egg survival
	if (SIMULATION_MODE2==STOCHASTIC) {
	//	fprintf(stdout, "Before: %.3f - ", this->Number);
		this->Number=Binomial_Deviate(this->Number,Overall_Survival);
	// 	fprintf(stdout, "After: %.3f\n", this->Number);
	}
	//deterministic implementation of egg survival
	if (SIMULATION_MODE2==DETERMINISTIC) this->Number=this->Number*Overall_Survival;
	
	
	
	//#6
	//hatch
	if (SIMULATION_MODE1==CIMSIM) EggHatch_Level=(int) ROUND_(Water_Depth/2+0.2); //based on CIMSIM, this is the level at which eggs still hatch 
	if (SIMULATION_MODE1==SKEETER_BUSTER) EggHatch_Level=(int) ROUND_(Water_Depth/0.2+2);  
	
	
	if (!this->Mature) //if egg cohort is not yet mature
	{
		if (this->Physiological_Development_Percent>PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMBRYONATION) //if egg is developed above threshold
		{
			if (Water_Average_Temp>=MINIMUM_TEMPERATURE_FOR_EGGHATCH) //if water temperature allows egghatch
			{
				if (this->Level_laid_in_Container<=EggHatch_Level) Hatch_Probability=1; //if eggs are not above the level where eggs still hatch, then all eggs hatch
				else //if they are above water
				{
					Hatch_Probability=RATIO_OF_EGGS_HATCHING_WITHOUT_FLOODING; //a portion of them hatches
					this->Mature=1; //the rest becomes mature
				}
				
			}
			if (Water_Average_Temp<MINIMUM_TEMPERATURE_FOR_EGGHATCH) //water is too cold to hatch
			{
				this->Mature=1; //all eggs become mature
				Hatch_Probability=0; //none of them hatches
			}
		}
		else //eggs are not yet developed above threshold
		{
			//development
			
			double Water_Average_Temp_K = Water_Average_Temp + 273.15; //conversion from Celsius to Kelvin
			double Development_Numerator=RO25_EMBRYONATION*(Water_Average_Temp_K/298)*exp((DHA_EMBRYONATION/R)*(((double)1/298)-(1/Water_Average_Temp_K))); 
			double Development_Denominator=1+exp((DHH_EMBRYONATION/R)*((1/THALF_EMBRYONATION)-(1/Water_Average_Temp_K)));
			this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;
		}
	}
	else //eggs are already mature
	{
		if (Water_Average_Temp>=MINIMUM_TEMPERATURE_FOR_EGGHATCH) if (this->Level_laid_in_Container<=EggHatch_Level) Hatch_Probability=DAILY_RATIO_OF_EMBRYONATED_EGGS_THAT_HATCH_IF_SUBMERGED; //if temperature is above threshold and eggs are under water then a portion of the eggs hatch
	}
	if (Water_Depth==0) Hatch_Probability=0; //if the container is dry, no eggs hatch
	
	//calculation of hatching eggs
	
	if (Hatch_Probability>0)  
	{
	  //stochastic implementation of egg hatch
		if (SIMULATION_MODE2==STOCHASTIC) NewlyHatched=Binomial_Deviate(this->Number,Hatch_Probability);
	  //deterministic implementation of egg hatch
	  if (SIMULATION_MODE2==DETERMINISTIC) NewlyHatched=this->Number*Hatch_Probability;    
	}
	
	
	//age incremented
	this->Age++;
	
	return NewlyHatched;
}
