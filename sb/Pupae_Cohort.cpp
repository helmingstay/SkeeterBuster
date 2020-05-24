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

Pupae_Cohort::Pupae_Cohort()
{
  this->Number = 0.0;
  this->Physiological_Development_Percent = 0.0;
  this->Age = 0;
  this->Age_since_hatch = 0;
  this->Average_Pupal_Weight = 0.0;
  this->Average_Pupal_Lipid_Reserve = 0.0;
  this->Mature = false;
  this->Sex = MALE;
  this->Wolbachia=0;
}

Pupae_Cohort::Pupae_Cohort( bool sex, double num, double avgWeight, int genotype, int wolbachia, double age_since_hatch)
{
  this->Number = num;
  this->Physiological_Development_Percent = 0.0;
  this->Age = 0;
  this->Age_since_hatch = age_since_hatch ;
  this->Average_Pupal_Weight = avgWeight;
  this->Average_Pupal_Lipid_Reserve = 0.0;
  this->Mature = false;
  this->Sex = sex;
  this->Genotype=genotype;
  this->Wolbachia=wolbachia;
}

//this is the calculations for unisex pupal cohorts
Trio Pupae_Cohort::Pupae_Calculations(double Air_Maximum, double Air_Minimum, double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Average_Temp)
{
  double Nominal_Survival,Temperature_Survival,Emergence_Survival,Overall_Survival,Fitness_Survival,Pupae_Number;
  double Emergence_Probability=0;
  double Development_Numerator, Development_Denominator;
  double Water_Average_Temp_K;
  double NewlyEmerged,NewlyEmerged_unadj;
  double Cummulative_Maturation, Previous_Cummulative_Maturation,Maturation,z;

  double Nominal_daily_survival_Pupae, Pupae_daily_survival_High_temperature_limit;
  double Pupae_daily_survival_High_temperature, Pupae_daily_survival_Low_temperature_limit;
  double Pupae_daily_survival_Low_temperature, Pupae_daily_survival_Normal_temperature_upper_limit;
  double Pupae_daily_survival_Normal_temperature_lower_limit, Pupae_survival_at_emergence;
  double WeightCadavers=0;
  Trio Pupal_Data;
  Pupal_Data.Maturation=0;
  Pupal_Data.NewlyEmerged=0;
  Pupal_Data.WeightCadavers=0;

  if (this->Number==0) return Pupal_Data; //if the pupal cohort is empty, end procedure

  if (this->Sex==FEMALE)
    {
      Nominal_daily_survival_Pupae=NOMINAL_DAILY_SURVIVAL_FEMALE_PUPAE;
      Pupae_daily_survival_High_temperature_limit=FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT;
      Pupae_daily_survival_High_temperature=FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE;
      Pupae_daily_survival_Low_temperature_limit=FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT;
      Pupae_daily_survival_Low_temperature=FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE;
      Pupae_daily_survival_Normal_temperature_upper_limit=FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT;
      Pupae_daily_survival_Normal_temperature_lower_limit=FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT;
      Pupae_survival_at_emergence=FEMALE_PUPAE_SURVIVAL_AT_EMERGENCE;
    }
  else
    {
      Nominal_daily_survival_Pupae=NOMINAL_DAILY_SURVIVAL_MALE_PUPAE;
      Pupae_daily_survival_High_temperature_limit=MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT;
      Pupae_daily_survival_High_temperature=MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE;
      Pupae_daily_survival_Low_temperature_limit=MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT;
      Pupae_daily_survival_Low_temperature=MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE;
      Pupae_daily_survival_Normal_temperature_upper_limit=MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT;
      Pupae_daily_survival_Normal_temperature_lower_limit=MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT;
      Pupae_survival_at_emergence=MALE_PUPAE_SURVIVAL_AT_EMERGENCE;
    }

  //#14
  //calculation of physiological developmental rate based on the enzyme kinetics approach
  	Maturation=0;

	if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(!this->Mature))
	{
		if (this->Physiological_Development_Percent<=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST) Previous_Cummulative_Maturation=0;
		if (this->Physiological_Development_Percent>=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST) Previous_Cummulative_Maturation=1;
		if ((this->Physiological_Development_Percent>SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST)&&(this->Physiological_Development_Percent<SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST)) 
			{
				z=(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-this->Physiological_Development_Percent)/(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST);
				Previous_Cummulative_Maturation=pow((1-z),SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_SHAPE*pow(z,2));
			}
	}
	
	
  Water_Average_Temp_K=Water_Average_Temp+273.15; //conversion from Celsius to Kelvin
  Development_Numerator=RO25_PUPAL_DEVELOPMENT*(Water_Average_Temp_K/298)*exp((DHA_PUPAL_DEVELOPMENT/R)*(((double)1/298)-(1/Water_Average_Temp_K)));
  Development_Denominator=1+exp((DHH_PUPAL_DEVELOPMENT/R)*((1/THALF_PUPAL_DEVELOPMENT)-(1/Water_Average_Temp_K)));
  this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;

  if (this->Age==0) {this->Age=1; return Pupal_Data;} //if  cohort contains new pupae, increment age only and end procedure (in CIMSIM, new pupae only develop, but do not have mortality, and do not emerge)

  //#15
  NewlyEmerged=NewlyEmerged_unadj=0;
  Nominal_Survival=Temperature_Survival=Emergence_Survival=Overall_Survival=1;

					
  //nominal pupae survival
  Nominal_Survival=Nominal_daily_survival_Pupae;



  //temperature related mortality using water temperature //COMMON CHANGES no. 2
  if (Water_Maximum_Temp>=Pupae_daily_survival_High_temperature_limit) Temperature_Survival=Pupae_daily_survival_High_temperature;
  if ((Water_Maximum_Temp<Pupae_daily_survival_High_temperature_limit)&&(Water_Maximum_Temp>=Pupae_daily_survival_Normal_temperature_upper_limit)) Temperature_Survival=1-(1-Pupae_daily_survival_High_temperature)/(Pupae_daily_survival_High_temperature_limit-Pupae_daily_survival_Normal_temperature_upper_limit)*(Water_Maximum_Temp-Pupae_daily_survival_Normal_temperature_upper_limit);

  if (Water_Minimum_Temp<=Pupae_daily_survival_Low_temperature_limit) Temperature_Survival=Temperature_Survival*Pupae_daily_survival_Low_temperature;
  if ((Water_Minimum_Temp>Pupae_daily_survival_Low_temperature_limit)&&(Water_Minimum_Temp<=Pupae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Pupae_daily_survival_Low_temperature)/(Pupae_daily_survival_Normal_temperature_lower_limit-Pupae_daily_survival_Low_temperature_limit)*(Pupae_daily_survival_Normal_temperature_lower_limit-Water_Minimum_Temp));

  // CORRECTED used to be : if ((Water_Minimum_Temp>Pupae_daily_survival_Low_temperature_limit)&&(Water_Minimum_Temp<=Pupae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Pupae_daily_survival_Low_temperature)/(Pupae_daily_survival_Normal_temperature_lower_limit-Pupae_daily_survival_Low_temperature_limit)*(Water_Minimum_Temp-Pupae_daily_survival_Low_temperature_limit));


  // fitness costs (if applicable at this stage)
  if (FITNESS_COST_STAGE==2)
		if (this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE)
		{
			if (GENETIC_CONTROL_METHOD==MEDEA)
			{	
				fprintf(stdout, "\n\nError: pupal fitness costs are incompatible with MEDEA genetic control method\n");
				ErrorMessage(833);
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);
				Pupae_Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
				WeightCadavers+=(this->Number-Pupae_Number)*this->Average_Pupal_Weight;
				this->Number=Pupae_Number;
			}
		}


  //overall survival
  Overall_Survival=Nominal_Survival*Temperature_Survival;
  //stochastic implementation of survival for pupal cohorts
  if (SIMULATION_MODE2==STOCHASTIC) 
  
  {
	  Pupae_Number=Binomial_Deviate(this->Number,Overall_Survival);
  	  WeightCadavers+=(this->Number-Pupae_Number)*this->Average_Pupal_Weight; //CIMSIM error here
	  this->Number=Pupae_Number;
  }
  //deterministic implementation of survival for pupal cohorts
  if (SIMULATION_MODE2==DETERMINISTIC) 
    {
      if (SIMULATION_MODE1==CIMSIM) WeightCadavers+=(this->Number-this->Number*Overall_Survival*Pupae_survival_at_emergence)*this->Average_Pupal_Weight; //weight of cadavers for next day increased by weight of dead female pupae + 1-EmergenceSuccess //ERROR in CIMSIM code
      if (SIMULATION_MODE1==SKEETER_BUSTER) WeightCadavers+=(this->Number-this->Number*Overall_Survival)*this->Average_Pupal_Weight; //weight of cadavers for next day increased by weight of dead female pupae, no error 
      this->Number=this->Number*Overall_Survival;
    }

  //WARNING! Difference between deterministic and stochastic routine in CIMSIM, lack of EmergenceSuccess in CadaverWeightContribution
  //possible solution of EmergenceSuccess*this->Average_Pupal_Weight, but this needs testing

					
  //#16
  //emergence
  //all  pupae emerge if  pupal cohort is mature 
  if (this->Mature) Emergence_Probability=1;				
  //stochastic implementation to determine the number of new  adults based on the Emergence probability
  if (SIMULATION_MODE2==STOCHASTIC)
    {
      //determines the number of new adults that emerge from the pupal cohort based on the Emergence_Probability
      if (Emergence_Probability>0) 
		{
		NewlyEmerged_unadj=Binomial_Deviate(this->Number,Emergence_Probability);
		this->Number-=NewlyEmerged_unadj;
		NewlyEmerged=Binomial_Deviate(NewlyEmerged_unadj,Pupae_survival_at_emergence);
		WeightCadavers+=(NewlyEmerged_unadj-NewlyEmerged)*this->Average_Pupal_Weight;
		}
    }
  //deterministic implementation of the procedure to calculate the number of new adults
  if (SIMULATION_MODE2==DETERMINISTIC)
    {
    	NewlyEmerged=this->Number*Emergence_Probability*Pupae_survival_at_emergence; //this calculates the number of new adults, adjusted for mortality
		if (SIMULATION_MODE1==SKEETER_BUSTER) WeightCadavers+=this->Number*Emergence_Probability*(1-Pupae_survival_at_emergence)*this->Average_Pupal_Weight; //this accounts for the pupae that die during emergence as larval food //COMMON CHANGE no. 3
		this->Number=this->Number-this->Number*Emergence_Probability; //decrease the number of pupae in the pupal cohorts due to emergence
	}

  // pupal cohort only becomes mature one day after physiological developmental percent has crossed the developmental threshold
  if (SIMULATION_MODE1==CIMSIM) if (this->Physiological_Development_Percent>=PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMERGENCE) this->Mature=1; //check for the maturation of the  pupal cohort
 
  if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(!this->Mature))
	{
		if (this->Physiological_Development_Percent<=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST) Cummulative_Maturation=0;
		if (this->Physiological_Development_Percent>=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST) Cummulative_Maturation=1;
		if ((this->Physiological_Development_Percent>SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST)&&(this->Physiological_Development_Percent<SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST)) 
			{
				z=(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-this->Physiological_Development_Percent)/(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST);
				Cummulative_Maturation=pow((1-z),SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_SHAPE*pow(z,2));
			}
		Maturation=Cummulative_Maturation-Previous_Cummulative_Maturation;
		if (Cummulative_Maturation==1) this->Mature=1;	
	}
  

  //increment the age of the  pupal cohort
  this->Age++;
  this->Age_since_hatch++;

  Pupal_Data.NewlyEmerged=NewlyEmerged;
  Pupal_Data.Maturation=Maturation;
  Pupal_Data.WeightCadavers=WeightCadavers;
  return Pupal_Data; 
};
//this is the end of the calculations for unisex pupal cohorts
    
//this is the end of the definition of the object for unisex pupal cohorts
 
