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

Male_Adult_Cohort::Male_Adult_Cohort()
{
  this->Number = 0.0;
  this->Age = 0;
  this->Physiological_Development_Percent = 0.0;
  this->Mature = false;
  this->Average_Adult_Weight=0;
  this->Wolbachia=0;
}


Male_Adult_Cohort::Male_Adult_Cohort( double num, double avgWt, int genotype, int wolbachia, double physdev, int age, double age_since_hatch, bool mature, int release_date )
{
  this->Number = num;
  this->Age = age;
  this->Age_since_hatch = age_since_hatch ;
  this->Physiological_Development_Percent = physdev;
  this->Mature = mature;
  this->Average_Adult_Weight=avgWt;
  this->Genotype=genotype;
  this->Wolbachia=wolbachia;
  this->Release_Date= release_date;
}


void Male_Adult_Cohort::Male_Adult_Calculations(double Air_Maximum, double Air_Minimum, double Air_Average, double SD)
{
  double Nominal_Survival,Temperature_Survival,Dessication_Survival,Fitness_Survival,Overall_Survival,Air_Average_K;
  double Development_Numerator, Development_Denominator;				

  if (this->Number==0) return; //if the male adult cohort is empty, end procedure
  if (this->Age==0) {this->Age++;return;} //if the male adult cohort contains new adult males, increment age and end procedure
					
  Nominal_Survival=Temperature_Survival=Dessication_Survival=Overall_Survival=1;

  //#26

  double age_dependence=1. ;
  switch(MALE_AGE_DEPENDENT_SURVIVAL)
  {
  case 0: //none
	  age_dependence=1. ;
	  break;

  case 1: //linear
	  age_dependence=1-((this->Age)/MALE_MAXIMUM_AGE_1) ;
	  if (age_dependence<0) age_dependence=0. ;
	  break;

  case 2: //constant then linear 
	  if ((this->Age)<MALE_START_SENESCENCE_2) age_dependence=1. ;
	  else age_dependence=1-(((this->Age)-MALE_START_SENESCENCE_2)/(MALE_MAXIMUM_AGE_2-MALE_START_SENESCENCE_2));
	  if (age_dependence<0) age_dependence=0. ;
	  break;
  }

  double wolbachia_effects=1.;
  if (this->Wolbachia)
	  // survival reduction effects of Wolbachia infection
  {
	if ((this->Age)<WOLBACHIA_SURVIVAL_REDUCTION_MALE_ONSET_AGE) wolbachia_effects=1.;
	else wolbachia_effects=WOLBACHIA_SURVIVAL_REDUCTION_MALE_FACTOR;
  }

  //nominal male adult nulliparous survival
  if (SIMULATION_MODE1==CIMSIM) Nominal_Survival=CIMSIM_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS;
  else Nominal_Survival=SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS*age_dependence*wolbachia_effects;

  //temperature related mortality
  if (Air_Maximum>=MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT) Temperature_Survival=MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE;
  if ((Air_Maximum<MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT)&&(Air_Maximum>=MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)) Temperature_Survival=1-(1-MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE)/(MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT-MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)*(Air_Maximum-MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT);
  
  if (Air_Minimum<=MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT) Temperature_Survival=Temperature_Survival*MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE;
  if ((Air_Minimum>MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)&&(Air_Minimum<=MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT)) Temperature_Survival=Temperature_Survival*(1-(1-MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE)/(MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)*(MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-Air_Minimum));
  // CORRECTED (see larvae)				

  //dessication related mortality
  if (SD<=MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT) Dessication_Survival=MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT;
  if (SD>=MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT) Dessication_Survival=MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT;
  if ((SD>MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT)&&(SD<MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT)) Dessication_Survival=MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT-((MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT-MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT)/(MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT-MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT))*(SD-MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT);
					
	
  // fitness costs (if applicable at this stage)
  if (FITNESS_COST_STAGE==3)
	  {
		if (this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE) 
		{
			if (GENETIC_CONTROL_METHOD==MEDEA)
			{	
				fprintf(stdout, "\n\nError: male adult fitness costs are incompatible with MEDEA genetic control method\n");
				ErrorMessage(833);
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);
				this->Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
			}
		}
	  }
  //overall survival
  Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival;
  //stochastic implementation of survival for male adult cohorts
  if (SIMULATION_MODE2==STOCHASTIC) this->Number=Binomial_Deviate(this->Number,Overall_Survival);
    
  //deterministic implementation for male adult cohorts
  if (SIMULATION_MODE2==DETERMINISTIC) this->Number=this->Number*Overall_Survival;
					
  //#27
  //calculate physiological developmental rate based on the enzyme kinetics approach
  Air_Average_K=Air_Average+273.15; //conversion from Celsius to Kelvin
  Development_Numerator=RO25_GONOTROPHIC_CYCLE*(Air_Average_K/298)*exp((DHA_GONOTROPHIC_CYCLE/R)*(((double)1/298)-(1/Air_Average_K)));
  Development_Denominator=1+exp((DHH_GONOTROPHIC_CYCLE/R)*((1/THALF_GONOTROPHIC_CYCLE)-(1/Air_Average_K)));
  this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;
  //check for maturation
  if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LATER_GONOTROPHIC_CYCLES) this->Mature=1;
     					
  //increment age
  this->Age++;
  this->Age_since_hatch++;

};	
//end of the definition of the object for male adult cohorts
	


