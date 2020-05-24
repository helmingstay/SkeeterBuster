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
extern double **Sherlock ;
extern int House_position ;
extern int number_of_genotypes ;
extern int Female_ID;
extern int Date;
extern FILE * test1 ;


Female_Adult::Female_Adult()
{
  this->Nulliparous = NULLIPAROUS;
  this->Average_Adult_Weight = 0.0;
  this->Age = 0;
  this->Age_since_hatch = 0;
  this->Physiological_Development_Percent = 0.0;
  this->Mature = false;
  this->Fecundity = 0.0;
  this->Mated = false ;
  this->Male_Genotype = 0 ;
  this->Wolbachia=0;
  this->Oviposition_Events = list<Oviposition>();
}


Female_Adult::Female_Adult( bool nulliOrParous, double avgWt, double fecundity, int genotype, int wolbachia, double physdev, int age, double age_since_hatch, bool mature, int release_date, bool mated, int male_genotype, int male_wolbachia )
{
  this->ID = Female_ID++;
  this->Nulliparous = nulliOrParous;
  this->Average_Adult_Weight = avgWt;
  this->Fecundity = fecundity;
  this->Genotype=genotype;
  this->Wolbachia=wolbachia;
  this->Physiological_Development_Percent=physdev;
  this->Age=age;
  this->Age_since_hatch=age_since_hatch;
  this->Mature=mature;
  this->Release_Date=release_date;
  this->Mated=mated ;
  this->Male_Genotype=male_genotype ;
  this->Male_Wolbachia=male_wolbachia;
  this->Oviposition_Events = list<Oviposition>();
}



//these are the calculations for female adults individually
double Female_Adult::Female_Adult_Calculations(double Air_Maximum, double Air_Minimum, double Air_Average, double SD, int Number_of_Suitable_Containers)
{
  double Nominal_Survival,Temperature_Survival,Dessication_Survival,Fitness_Survival,Overall_Survival,Air_Average_K;
  double Development_Numerator, Development_Denominator;
  double OvipositingNewlyParous; // either Ovipositing or NewlyParous from original code!

  if (this->Age==0) //if the female adult nulliparous cohort contains new nulliparous female adults
    {
      this->Age++; 
	  //this->Age_since_hatch++;//increment age
      //#21
      //calculate the physiological developmental percentage based on the enzyme kinetics approach
      Air_Average_K=Air_Average+273.15; //conversion from Celsius to Kelvin
      Development_Numerator=RO25_GONOTROPHIC_CYCLE*(Air_Average_K/298)*exp((DHA_GONOTROPHIC_CYCLE/R)*(((double)1/298)-(1/Air_Average_K)));
      Development_Denominator=1+exp((DHH_GONOTROPHIC_CYCLE/R)*((1/THALF_GONOTROPHIC_CYCLE)-(1/Air_Average_K)));
      this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;

      //test for maturation

      if (this->Nulliparous==NULLIPAROUS)
		{
		 if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST_GONOTROPHIC_CYCLE) this->Mature=1;
		}
      else
		{
		 if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LATER_GONOTROPHIC_CYCLES) this->Mature=1;
		}				
      return 0; //end procedure (in CIMSIM, newly emerged nulliparous female adults do develop, but there's no mortality)
    }

  //#19
  OvipositingNewlyParous=0;

  Nominal_Survival=Temperature_Survival=Dessication_Survival=Overall_Survival=1;

  double age_dependence=1. ;
  switch(FEMALE_AGE_DEPENDENT_SURVIVAL)
  {
  case 0: //none
	  age_dependence=1. ;
	  break;

  case 1: //linear
	  age_dependence=1-((this->Age)/FEMALE_MAXIMUM_AGE_1) ;
	  if (age_dependence<0) age_dependence=0. ;
	  break;

  case 2: //constant then linear 
	  if ((this->Age)<FEMALE_START_SENESCENCE_2) age_dependence=1. ;
	  else age_dependence=1-(((this->Age)-FEMALE_START_SENESCENCE_2)/(FEMALE_MAXIMUM_AGE_2-FEMALE_START_SENESCENCE_2));
	  if (age_dependence<0) age_dependence=0. ;
	  break;
  }

  double wolbachia_effects=1.;
  if (this->Wolbachia)
	  // survival reduction effects of Wolbachia infection
  {
	if ((this->Age)<WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_ONSET_AGE) wolbachia_effects=1.;
	else wolbachia_effects=WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_FACTOR;
  }


  //nominal (and age-dependent) female adult survival
  if (SIMULATION_MODE1==CIMSIM) Nominal_Survival=CIMSIM_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS;
  else Nominal_Survival=SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS*age_dependence*wolbachia_effects;
					
  //temperature related mortality
  if (Air_Maximum>=FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT) Temperature_Survival=FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE;
  if ((Air_Maximum<FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT)&&(Air_Maximum>=FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)) Temperature_Survival=1-(1-FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE)/(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT-FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT)*(Air_Maximum-FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT);

  if (Air_Minimum<=FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT) Temperature_Survival=Temperature_Survival*FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE;
  if ((Air_Minimum>FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)&&(Air_Minimum<=FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT)) Temperature_Survival=Temperature_Survival*(1-(1-FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE)/(FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT)*(FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT-Air_Minimum));
  // CORRECTED (see larvae)
					
  //dessication related mortality
  if (SD<=FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT) Dessication_Survival=FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT;
  if (SD>=FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT) Dessication_Survival=FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT;
  if ((SD>FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT)&&(SD<FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT)) Dessication_Survival=FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT-((FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT-FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT)/(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT-FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT))*(SD-FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT);

  // fitness costs (if applicable at this stage)
  if (FITNESS_COST_STAGE==3)
		if (this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE) 
		{
			if (GENETIC_CONTROL_METHOD==MEDEA)
			{	
				ErrorMessage(833);
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);

				if ((GENETIC_CONTROL_METHOD==ReduceReplace)&&(this->Source_Container_Type==102)&&(TETRACYCLINE==1)) // If a female was raised with tetracycline, you can over-ride this fitness deduction
					{
					Fitness_Survival = Fitness_Calculation_Environmental_Effect(this->Genotype);
					}
				//fprintf(stdout, "Dead: fitness is %.2f\n", Fitness_Survival);
				if (Random_Number()>Fitness_Survival) // is dead
				  return -99;
			}
		}

  //overall survival
  Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival;
  
  
  //stochastic implementation of survival for individual female adult
  
  if (Random_Number()>Overall_Survival) // is dead
	  return -99;
    
	
  //deterministic implementation of survival for female adult cohorts
  //if (SIMULATION_MODE2==DETERMINISTIC) this->Number=this->Number*Overall_Survival;
  // has to move (?)				

					
  //#20
  //procedure for oviposition if the female adult is mature, mated, has containers available and air is warm enough

  if ((this->Mated)&&(this->Mature)&&(Number_of_Suitable_Containers)&&(Air_Average>=FEMALE_ADULT_MINIMUM_TEMPERATURE_FOR_OVIPOSITION))
    {
      //stochastic implementation of ovipositing for female adult cohorts
      if (SIMULATION_MODE2==STOCHASTIC)
		{
		  OvipositingNewlyParous++;
		}  	
	
	  //deterministic implementation of ovipositing for female adult cohorts
      if (SIMULATION_MODE2==DETERMINISTIC)
		{
			  OvipositingNewlyParous++;
	    }
	

      if (this->Nulliparous==PAROUS)
		{
			this->Physiological_Development_Percent=0; this->Mature=0;
		}
    }

	

  //#21
  //calculation of physiological developmental rate based on the enzyme kinetics approach
  //development is after oviposition so that female only lay eggs a day after they become mature
  Air_Average_K=Air_Average+273.15; //conversion from Celsius to Kelvin
  Development_Numerator=RO25_GONOTROPHIC_CYCLE*(Air_Average_K/298)*exp((DHA_GONOTROPHIC_CYCLE/R)*(((double)1/298)-(1/Air_Average_K)));
  Development_Denominator=1+exp((DHH_GONOTROPHIC_CYCLE/R)*((1/THALF_GONOTROPHIC_CYCLE)-(1/Air_Average_K)));
  this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;
      
  if (this->Nulliparous==NULLIPAROUS)
    {
      if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST_GONOTROPHIC_CYCLE) this->Mature=1;
    }
  else
    {
      if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LATER_GONOTROPHIC_CYCLES) this->Mature=1;
    }
					
  //increment age
  this->Age++;
  this->Age_since_hatch++;

  //calculate fecundity //#22
  this->Fecundity=this->Average_Adult_Weight*FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT;
  if (this->Age>25) this->Fecundity+=SKEETER_BUSTER_FECUNDITY_DROP_BY_AGE*(this->Age - 25);

  if ((FITNESS_COST_STAGE==4)) // case of female fecundity fitness costs
  {
	  this->Fecundity*=Fitness_Calculation(this->Genotype);
  }

  if (this->Fecundity<0) this->Fecundity=0;

  return OvipositingNewlyParous;

};


//this is the end of the definition of the object for female nulliparous adult cohorts

//*********************************************
