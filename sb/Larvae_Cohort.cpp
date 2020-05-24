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
 
extern FILE *Output, *test1;
extern int Date;

extern long ra[M+1], nd;


Larvae_Cohort::Larvae_Cohort()
{
  this->Sex = MALE;
  this->Number = 0.0;
  this->Physiological_Development_Percent = 0.0;
  this->Age = 0;
  this->Age_since_hatch = 0;
  this->Instar = 0;
  this->Mature = false;
  this->Average_Larval_Weight = 0.0;
  this->Prefasting_Larval_Weight = 0.0;
  this->Prefasting_Lipid_Reserve = 0.0;
  this->Average_Lipid_Reserve = 0.0;
  this->Previous_Larval_Weight = 0.0;
  this->Previous_Day_Larval_Weight = 0.0;
  this->Pupation_Weight = 0.0;
  this->Previous_Day_Fasting_Survival = 0.0;
  this->Larvae_Chronological_basis = 0.0;
  this->Wolbachia=0;
}

Larvae_Cohort::Larvae_Cohort( bool sex, double num, double avgWeight , int age, double age_since_hatch, int instar, int genotype, int wolbachia)
{
  this->Sex = sex;
  this->Number = num;
  this->Physiological_Development_Percent = 0.0;
  this->Age = age;
  this->Age_since_hatch = age_since_hatch ;
  this->Instar = instar ;
  this->Mature = false;
  this->Average_Larval_Weight = avgWeight;
  this->Prefasting_Larval_Weight = 0.0;
  this->Prefasting_Lipid_Reserve = 0.0;
  this->Average_Lipid_Reserve = 0.0;
  this->Previous_Larval_Weight = 0.0;
  this->Previous_Day_Larval_Weight = 0.0;
  this->Pupation_Weight = 0.0;
  this->Previous_Day_Fasting_Survival = 0.0;
  this->Larvae_Chronological_basis = 0.0;
  this->Genotype=genotype;
  this->Wolbachia=wolbachia;
  }


//#8
double Larvae_Cohort::Larvae_Calculations(double Water_Average_Temp, double Development_26, double Development_134)
{
  double Larvae_Chronological_basis_at_26C;
  double Development_Numerator=0.0, Development_Denominator=0.0;;
  double Water_Average_Temp_K;
  double Cummulative_Maturation, Previous_Cummulative_Maturation, Maturation;
  double z;

  //if the larval cohort is empty, end procedure
  if (this->Number==0) return 0;

  if (this->Sex==FEMALE)
    {
      Larvae_Chronological_basis_at_26C=FEMALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C;
    }
  else
    {
      Larvae_Chronological_basis_at_26C=MALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C;
    }

	      
  //calculation of developmental rate for larval cohorts based on the enzyme kinetics approach
  if (DEVELOPMENT_MODE==FOCKSDEVEL)
    {
      Water_Average_Temp_K=Water_Average_Temp+273.15; //conversion from Celsius to Kelvin
      Development_Numerator=(DHA_LARVAL_DEVELOPMENT/R)*(((double)1/298)-(1/Water_Average_Temp_K));
      Development_Numerator=RO25_LARVAL_DEVELOPMENT*(Water_Average_Temp_K/298)*exp(Development_Numerator);
      Development_Denominator=(DHH_LARVAL_DEVELOPMENT/R)*((1/THALF_LARVAL_DEVELOPMENT)-(1/Water_Average_Temp_K));
      Development_Denominator=1+exp(Development_Denominator);
    }
  //calculation of developmental rate for larval cohorts based on the degree-day approach
  else
    {
      Development_Numerator=(Water_Average_Temp-MINIMUM_DEVELOPMENTAL_TEMPERATURE)/DEGREE_HOURS_TO_BE_REACHED;
      Development_Denominator=1;
    }

  //update physiological development percent //OUR CHANGE #4 variation in larval maturation Rueda, Wilhoit
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

  this->Physiological_Development_Percent=this->Physiological_Development_Percent+(Development_Numerator/Development_Denominator)*24;
if (SIMULATION_MODE1==CIMSIM) if (this->Physiological_Development_Percent>=CIMSIM_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_PUPATION) this->Mature=1;

if ((SIMULATION_MODE1==SKEETER_BUSTER)&&(!this->Mature))
{
	if (this->Physiological_Development_Percent<=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST) Cummulative_Maturation=0;
	if (this->Physiological_Development_Percent>=SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST) Cummulative_Maturation=1;
	if ((this->Physiological_Development_Percent>SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST)&&(this->Physiological_Development_Percent<SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST)) 
		{
			z=(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-this->Physiological_Development_Percent)/(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST-SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST);
			Cummulative_Maturation=pow((1-z),SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_SHAPE*pow(z,2));
		}
	if (Previous_Cummulative_Maturation <1) // if it is 1, then Cumulative Maturation will also be 1 and the cohort will mature
		Maturation=(Cummulative_Maturation-Previous_Cummulative_Maturation)/(1-Previous_Cummulative_Maturation);
	if (Cummulative_Maturation==1) this->Mature=1;
}



  //if the larval cohort contains new larvae then end procedure
  if (this->Age==0) return Maturation;
					
	
  //prepare for Gilpin-McClelland equation Euler integration calculations by calculating the chronological basis based on the developmental rate of the day
  this->Previous_Day_Larval_Weight=this->Average_Larval_Weight; //store the current weight as the weight on the day before

  //Larval Chronological Basis calculation according without error
  this->Larvae_Chronological_basis=Larvae_Chronological_basis_at_26C*((Development_Numerator/Development_Denominator)*24-Development_134)/(Development_26-Development_134);
  

  				
  //increment age
  this->Age++;
  this->Age_since_hatch++;

  return Maturation; 
}
				
//this procedure calculated the Gilpin-McClelland equations for larval cohorts	//#9
double Larvae_Cohort::Larval_growth(double Larval_food, double Container_Volume)
{
  double Weight_Increase;

  double Larvae_Increment_of_increase_of_rate_of_food_exploitation,Larvae_Asymptotic_rate_of_food_exploitation_at_high_food_densities;
  double Larvae_Rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae, Larvae_Exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae;
  double Larvae_Conversion_rate_of_consumed_food_to_biomass; 
  double Larval_food_decrease;

  double a, b,c, d1,d2;
  
  if (this->Sex==FEMALE)
    {
      b=FEMALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION;
      c=FEMALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES;
      d1=FEMALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      d2=FEMALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      a=FEMALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS;
    }
  else
    {
      b=MALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION;
      c=MALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES;
      d1=MALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      d2=MALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      a=MALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS;
    }

  if (this->Number==0) return 0; //if the larval cohort is empty, end procedure
  if (this->Age==0) return 0; //if the larval cohort contains new larvae, end procedure (in CIMSIM, weight of larvae does not increase on the day they are hatched)
  //equation 6 of CIMSIM as the Gilpin-McClelland equation for weight increase of a larval cohort


  this->Previous_Larval_Weight=this->Average_Larval_Weight;	

  Weight_Increase=a*this->Larvae_Chronological_basis*2880*pow(this->Average_Larval_Weight,b)*(1.0-exp(-c*Larval_food));

  Larval_food_decrease=this->Number*Weight_Increase/(a*LARVAE_NUMBER_OF_EULER_STEPS);
  
  Weight_Increase -= a*this->Larvae_Chronological_basis*2880*d1*pow(this->Average_Larval_Weight,d2);

  Weight_Increase /=(double)LARVAE_NUMBER_OF_EULER_STEPS;
  this->Average_Larval_Weight +=Weight_Increase; //update the weight of the larval cohort

  //equation 7 of CIMSIM as the Gilpin-McClelland equation for food decrease

  /*if (this ->Sex==FEMALE)
    {
      Larvae_Increment_of_increase_of_rate_of_food_exploitation=FEMALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION;
      Larvae_Asymptotic_rate_of_food_exploitation_at_high_food_densities=FEMALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES;
      Larvae_Rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae=FEMALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      Larvae_Exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae=FEMALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      Larvae_Conversion_rate_of_consumed_food_to_biomass=FEMALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS;
    }
  else
    {
      Larvae_Increment_of_increase_of_rate_of_food_exploitation=MALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION;
      Larvae_Asymptotic_rate_of_food_exploitation_at_high_food_densities=MALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES;
      Larvae_Rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae=MALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      Larvae_Exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae=MALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE;
      Larvae_Conversion_rate_of_consumed_food_to_biomass=MALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS;
    }

  if (this->Number==0) return 0; //if the larval cohort is empty, end procedure
  if (this->Age==0) return 0; //if the larval cohort contains new larvae, end procedure (in CIMSIM, weight of larvae does not increase on the day they are hatched)
  //equation 6 of CIMSIM as the Gilpin-McClelland equation for weight increase of a larval cohort


  this->Previous_Larval_Weight=this->Average_Larval_Weight;	

  Weight_Increase=pow(this->Average_Larval_Weight,Larvae_Increment_of_increase_of_rate_of_food_exploitation)*(1-exp(-Larvae_Asymptotic_rate_of_food_exploitation_at_high_food_densities*Larval_food)); // add /Container_Volume here if needed

  Weight_Increase=Weight_Increase-Larvae_Rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae*pow(this->Average_Larval_Weight,Larvae_Exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae);

  Weight_Increase=Weight_Increase*2880*((double) 1/(double)LARVAE_NUMBER_OF_EULER_STEPS)*Larvae_Conversion_rate_of_consumed_food_to_biomass*this->Larvae_Chronological_basis;
  this->Average_Larval_Weight=this->Average_Larval_Weight+Weight_Increase; //update the weight of the larval cohort
  //equation 7 of CIMSIM as the Gilpin-McClelland equation for food decrease

  //Larval_food_decrease=(1-exp(-Larvae_Asymptotic_rate_of_food_exploitation_at_high_food_densities*Larval_food/Container_Volume));
  //Larval_food_decrease=Larval_food_decrease*2880*((double) 1/(double)LARVAE_NUMBER_OF_EULER_STEPS)*this->Number*this->Larvae_Chronological_basis*pow(this->Previous_Larval_Weight,Larvae_Increment_of_increase_of_rate_of_food_exploitation);

  // rewrite dF(t)/dt equation using Weight_Increase (fewer operations)
  Larval_food_decrease = (1/Larvae_Conversion_rate_of_consumed_food_to_biomass)*this->Number*Weight_Increase;
  */

  return Larval_food_decrease;
};
//end of the procedure for the calculation of the Gilpin-McClelland equations

//this procedure calculates the survival of larval cohorts //#10
double Larvae_Cohort::Larval_survival(double Water_Maximum, double Water_Minimum, double Water_Depth)
{
  double Nominal_Survival,Temperature_Survival,Dessication_Survival,Fasting_Survival,Fitness_Survival,Overall_Survival,LarvaeNumber;

  double WeightCadavers=0;

  double Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one,Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two;
  double Larvae_Minimum_lipid_ratio_of_living_larvae,Larvae_daily_survival_fasting_with_lipid_reserve,Larvae_daily_survival_fasting_without_lipid_reserve;
  double Nominal_daily_survival_larvae,Larvae_daily_survival_High_temperature_limit,Larvae_daily_survival_High_temperature,Larvae_daily_survival_Low_temperature_limit;
  double Larvae_daily_survival_Low_temperature,Larvae_daily_survival_Normal_temperature_upper_limit,Larvae_daily_survival_Normal_temperature_lower_limit;
  double Larvae_daily_survival_Dry_container,Larvae_Maximal_physiological_developmental_time_for_survival,Larvae_smallest_weight_for_survival;
		
  if (this->Number==0) return 0; //if the larval cohort is empty, end procedure
  if (this->Age==0) {this->Age++;return 0;} //if the larval cohort contains new larvae, increment age and end procedure (in CIMSIM, new larvae on the day they hatch only increase their development) 

  if (this->Sex==FEMALE)
    {
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one=FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE;
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two=FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO;
      Larvae_Minimum_lipid_ratio_of_living_larvae=FEMALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE;
      Larvae_daily_survival_fasting_with_lipid_reserve=FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE;
      Larvae_daily_survival_fasting_without_lipid_reserve=FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE;
      Nominal_daily_survival_larvae=NOMINAL_DAILY_SURVIVAL_FEMALE_LARVAE;
      Larvae_daily_survival_High_temperature_limit=FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT;
      Larvae_daily_survival_High_temperature=FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE;
      Larvae_daily_survival_Low_temperature_limit=FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT;
      Larvae_daily_survival_Low_temperature=FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE;
      Larvae_daily_survival_Normal_temperature_upper_limit=FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT;
      Larvae_daily_survival_Normal_temperature_lower_limit=FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT;
	  Larvae_daily_survival_Dry_container=FEMALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER;	
      if (SIMULATION_MODE1==CIMSIM) Larvae_Maximal_physiological_developmental_time_for_survival=CIMSIM_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
      if (SIMULATION_MODE1==SKEETER_BUSTER) Larvae_Maximal_physiological_developmental_time_for_survival=SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
      Larvae_smallest_weight_for_survival=FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL;
    }
  else
    {
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one=MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE;
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two=MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO;
      Larvae_Minimum_lipid_ratio_of_living_larvae=MALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE;
      Larvae_daily_survival_fasting_with_lipid_reserve=MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE;
      Larvae_daily_survival_fasting_without_lipid_reserve=MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE;
      Nominal_daily_survival_larvae=NOMINAL_DAILY_SURVIVAL_MALE_LARVAE;
      Larvae_daily_survival_High_temperature_limit=MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT;
      Larvae_daily_survival_High_temperature=MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE;
      Larvae_daily_survival_Low_temperature_limit=MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT;
      Larvae_daily_survival_Low_temperature=MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE;
      Larvae_daily_survival_Normal_temperature_upper_limit=MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT;
	  Larvae_daily_survival_Normal_temperature_lower_limit=MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT;
      Larvae_daily_survival_Dry_container=MALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER;	
      if (SIMULATION_MODE1==CIMSIM) Larvae_Maximal_physiological_developmental_time_for_survival=CIMSIM_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
      if (SIMULATION_MODE1==SKEETER_BUSTER) Larvae_Maximal_physiological_developmental_time_for_survival=SKEETER_BUSTER_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
      Larvae_smallest_weight_for_survival=MALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL;
    }


  Overall_Survival=Nominal_Survival=Temperature_Survival=Dessication_Survival=Fasting_Survival=1;
					
  //calculation of fasting survival for the larval cohort, based on CIMSIM code

  if (this->Previous_Day_Larval_Weight<=this->Average_Larval_Weight) //if weight of the female larval cohort increased
    {
      if (!this->Prefasting_Lipid_Reserve==0) //if lipid reserve was positive - previous fasting occured
	{
	  this->Average_Lipid_Reserve=this->Average_Lipid_Reserve+this->Average_Larval_Weight-this->Previous_Day_Larval_Weight; //add weight increase to average lipid reserve
	  if (this->Average_Lipid_Reserve>this->Prefasting_Lipid_Reserve) {this->Prefasting_Lipid_Reserve=0; if (SIMULATION_MODE1==CIMSIM) this->Average_Lipid_Reserve=0;} //if average lipid reserve surpassed the lipid reserve at the onset of fasting, then resupplying over, clear lipid reserves values
	}
      Fasting_Survival=1; //no mortality due to fasting
    }
  if (this->Previous_Day_Larval_Weight>this->Average_Larval_Weight) //if weight of the larval cohort decreased - fasting
    {
      if (this->Prefasting_Lipid_Reserve==0) //if this is the first day of fasting
	{

	  this->Prefasting_Lipid_Reserve=this->Previous_Day_Larval_Weight*(Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one*(log(this->Previous_Day_Larval_Weight)+Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two)-Larvae_Minimum_lipid_ratio_of_living_larvae); //calculate lipid reserve at the onset of fasting
	  if (SIMULATION_MODE1==SKEETER_BUSTER) if (this->Prefasting_Lipid_Reserve<0) this->Prefasting_Lipid_Reserve=0;
	  this->Average_Lipid_Reserve=this->Prefasting_Lipid_Reserve-this->Previous_Day_Larval_Weight+this->Average_Larval_Weight; //substract weight loss from prefasting lipid reserve
	}
      else //if previous fasting 
	{
	  this->Average_Lipid_Reserve=this->Average_Lipid_Reserve-this->Previous_Day_Larval_Weight+this->Average_Larval_Weight; //decreae lipid reserve by weight loss
	}

      if (this->Average_Lipid_Reserve>0) Fasting_Survival=Larvae_daily_survival_fasting_with_lipid_reserve; //if average lipid reserve positive
      if (this->Average_Lipid_Reserve<=0) Fasting_Survival=Larvae_daily_survival_fasting_without_lipid_reserve; //if average lipid reserve zero
    }
	//ORIGINAL CIMSIM, HAS NO REASON WHY Fasting Survival should not be recalculated when the Previous Day's larval weight is higher than the pupation weight threshold
 if (SIMULATION_MODE1==CIMSIM) if (this->Previous_Day_Larval_Weight>this->Pupation_Weight) Fasting_Survival=this->Previous_Day_Fasting_Survival; //if weight is above pupation threshold, the last calculated fasting survival is applied
					

  //nominal larval survival

  Nominal_Survival=Nominal_daily_survival_larvae;
  //Nominal_Survival=1; //alternative setting

  //temperature related mortality

  if (Water_Maximum>=Larvae_daily_survival_High_temperature_limit) Temperature_Survival=Larvae_daily_survival_High_temperature;
  if ((Water_Maximum<Larvae_daily_survival_High_temperature_limit)&&(Water_Maximum>=Larvae_daily_survival_Normal_temperature_upper_limit)) Temperature_Survival=1-(1-Larvae_daily_survival_High_temperature)/(Larvae_daily_survival_High_temperature_limit-Larvae_daily_survival_Normal_temperature_upper_limit)*(Water_Maximum-Larvae_daily_survival_Normal_temperature_upper_limit);

  if (Water_Minimum<=Larvae_daily_survival_Low_temperature_limit) Temperature_Survival=Temperature_Survival*Larvae_daily_survival_Low_temperature;
  if ((Water_Minimum>Larvae_daily_survival_Low_temperature_limit)&&(Water_Minimum<=Larvae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Larvae_daily_survival_Low_temperature)/(Larvae_daily_survival_Normal_temperature_lower_limit-Larvae_daily_survival_Low_temperature_limit)*(Larvae_daily_survival_Normal_temperature_lower_limit-Water_Minimum));

  // CORRECTED: used to be if ((Water_Minimum>Larvae_daily_survival_Low_temperature_limit)&&(Water_Minimum<=Larvae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Larvae_daily_survival_Low_temperature)/(Larvae_daily_survival_Normal_temperature_lower_limit-Larvae_daily_survival_Low_temperature_limit)*(Water_Minimum-Larvae_daily_survival_Low_temperature_limit));

  //dessication related mortality

  if (Water_Depth==0) Dessication_Survival=Larvae_daily_survival_Dry_container;

  // fitness costs (if applicable at this stage)
  if (FITNESS_COST_STAGE==1)
		if ((this->Age==FITNESS_COST_STAGE_SPECIFIC_AGE)&&(Date!=0)) 
		{
			if (GENETIC_CONTROL_METHOD==MEDEA)
			{	
				/*Fitness_Survival = Fitness_Calculation_Medea(this->Genotype, this->Medea_Mom) ;
				LarvaeNumber=Binomial_Deviate(this->Number, Fitness_Survival) ;
				WeightCadavers+=this->Previous_Day_Larval_Weight*(this->Number-LarvaeNumber);
				this->Number=LarvaeNumber;*/
				fprintf(stdout, "\n\nError: larval fitness costs are incompatible with MEDEA genetic control method\n");
				ErrorMessage(833);
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);
				LarvaeNumber=Binomial_Deviate(this->Number, Fitness_Survival) ;
				WeightCadavers+=this->Previous_Day_Larval_Weight*(this->Number-LarvaeNumber);
				this->Number=LarvaeNumber;
			}
		}


  //overall survival
  Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival*Fasting_Survival;
  //all larvae are killed if the physiological developmental threshold is higher then a maximum threshold

  
   if (this->Physiological_Development_Percent>Larvae_Maximal_physiological_developmental_time_for_survival) Overall_Survival=0;
  //all larvae are killed if their weight goes below a minimum threshold

 
  
  if (this->Average_Larval_Weight<Larvae_smallest_weight_for_survival) Overall_Survival=0;
  //stochastic implementation for survival of the larval cohort

  if (SIMULATION_MODE2==STOCHASTIC)	
    {
		LarvaeNumber=Binomial_Deviate(this->Number,Overall_Survival);
		WeightCadavers+=this->Previous_Day_Larval_Weight*(this->Number-LarvaeNumber);
		this->Number=LarvaeNumber;
    }
  //deterministic implementation for survival of the larval cohort 
  if (WeightCadavers<0)
    		{
    		fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
    		}
  if (SIMULATION_MODE2==DETERMINISTIC) 
    {
      WeightCadavers+=(this->Number-this->Number*Overall_Survival)*this->Previous_Day_Larval_Weight; //weight of cadavers for the next day is increased by the weight of the dead larvae on the previous day, according to CIMSIM
      this->Number=this->Number*Overall_Survival;
    }
  this->Previous_Day_Fasting_Survival=Fasting_Survival; //set the new fasting survival as the previous day fasting survival
  
  //if (Date>200) fprintf(test1, "%d %.5f %.5f %.5f\n", Age, this->Average_Larval_Weight, Overall_Survival, Fasting_Survival);
  if (WeightCadavers<0)
  		{
  		fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
  		}
  return WeightCadavers;
};
//end of the survival procedure for larval cohorts

//this procedure calculates the pupation weight threshold for larval cohorts //#11
// as well as the thresholds for instar transition
double Larvae_Cohort::Larval_Pupation_Weight_Calculation(double Water_Average_Temp)
{
  double Wmin, Slope;
  double Wmin_25,Wmin_50,Wmin_75,Wmin_100;
  double Wmin_II_min, Wmin_III_min, Wmin_IV_min, Wmin_II_max, Wmin_III_max, Wmin_IV_max;
  double Slope_25,Slope_50,Slope_75,Slope_100;
  double Slope_II, Slope_III, Slope_IV;
  double Larvae_Absolute_minimum_weight_for_pupation,Larvae_Maximal_physiological_developmental_time_for_survival;
  double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T,Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T;
  double Pupation_Physiological_Development_Threshold_for_Wmin;
  double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25,Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50,Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75,Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100;

  if (this->Number==0) return 0;

  if (this->Sex==FEMALE)
    {
      if (SIMULATION_MODE1==CIMSIM) 
	  {
		  Larvae_Absolute_minimum_weight_for_pupation=CIMSIM_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION;
		  Larvae_Maximal_physiological_developmental_time_for_survival=CIMSIM_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
		  Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=CIMSIM_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		  Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=CIMSIM_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		  Pupation_Physiological_Development_Threshold_for_Wmin=CIMSIM_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN;
	  }
      if (SIMULATION_MODE1==SKEETER_BUSTER) 
	  {
		  Larvae_Absolute_minimum_weight_for_pupation=SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION;
		  Larvae_Maximal_physiological_developmental_time_for_survival=SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
		  Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		  Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25=SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25;
          Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50=SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50;
          Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75=SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75;
          Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100=SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100;
		  Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=SKEETER_BUSTER_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		  Pupation_Physiological_Development_Threshold_for_Wmin=SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN;
      } 
    }
  else
    {
		if (SIMULATION_MODE1==CIMSIM)
		{
		 Larvae_Absolute_minimum_weight_for_pupation=CIMSIM_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION;
		 Larvae_Maximal_physiological_developmental_time_for_survival=CIMSIM_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=CIMSIM_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		 Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=CIMSIM_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		 Pupation_Physiological_Development_Threshold_for_Wmin=CIMSIM_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN;
		}
		if (SIMULATION_MODE1==SKEETER_BUSTER)
		{
		 Larvae_Absolute_minimum_weight_for_pupation=SKEETER_BUSTER_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION;
		 Larvae_Maximal_physiological_developmental_time_for_survival=SKEETER_BUSTER_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25=SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50=SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75=SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75;
		 Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100=SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100;
		 Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=SKEETER_BUSTER_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T;
		 Pupation_Physiological_Development_Threshold_for_Wmin=SKEETER_BUSTER_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN;
		}
    }


  Wmin=Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T+Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;

  Wmin_25=Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25+Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
  Wmin_50=Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50+Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
  Wmin_75=Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75+Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
  Wmin_100=Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100+Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;


  Slope=(Wmin-Larvae_Absolute_minimum_weight_for_pupation)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
	Slope_25=(Wmin_25-Larvae_Absolute_minimum_weight_for_pupation)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
	Slope_50=(Wmin_50-Larvae_Absolute_minimum_weight_for_pupation)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
	Slope_75=(Wmin_75-Larvae_Absolute_minimum_weight_for_pupation)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
	Slope_100=(Wmin_100-Larvae_Absolute_minimum_weight_for_pupation)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);

  if (SIMULATION_MODE1==CIMSIM) this->Pupation_Weight=Wmin+Slope*this->Physiological_Development_Percent; //ORIGINAL INCORRECT CIMSIM FUNCTION
  if (SIMULATION_MODE1==SKEETER_BUSTER) this->Pupation_Weight=Slope*this->Physiological_Development_Percent+(Pupation_Physiological_Development_Threshold_for_Wmin*Larvae_Absolute_minimum_weight_for_pupation-Larvae_Maximal_physiological_developmental_time_for_survival*Wmin)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
  if (SIMULATION_MODE1==SKEETER_BUSTER) this->Pupation_Weight_25=Slope_25*this->Physiological_Development_Percent+(Pupation_Physiological_Development_Threshold_for_Wmin*Larvae_Absolute_minimum_weight_for_pupation-Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_25)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
  if (SIMULATION_MODE1==SKEETER_BUSTER) this->Pupation_Weight_50=Slope_50*this->Physiological_Development_Percent+(Pupation_Physiological_Development_Threshold_for_Wmin*Larvae_Absolute_minimum_weight_for_pupation-Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_50)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
  if (SIMULATION_MODE1==SKEETER_BUSTER) this->Pupation_Weight_75=Slope_75*this->Physiological_Development_Percent+(Pupation_Physiological_Development_Threshold_for_Wmin*Larvae_Absolute_minimum_weight_for_pupation-Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_75)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
  if (SIMULATION_MODE1==SKEETER_BUSTER) this->Pupation_Weight_100=Slope_100*this->Physiological_Development_Percent+(Pupation_Physiological_Development_Threshold_for_Wmin*Larvae_Absolute_minimum_weight_for_pupation-Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_100)/(Pupation_Physiological_Development_Threshold_for_Wmin-Larvae_Maximal_physiological_developmental_time_for_survival);
  
  // instars (Skeeter Buster mode only)
  // calculates weight thresholds to move from one instar to the next

	Wmin_II_min = (Wmin_50-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.0189255;  // hard coded values from Christophers 1960
	Wmin_III_min = (Wmin_50-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.0739927;
	Wmin_IV_min = (Wmin_50-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.4151404;

	Wmin_II_max = (SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.0189255;
	Wmin_III_max = (SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.0739927;
	Wmin_IV_max = (SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION-FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL)*0.4151404;

	Slope_II = (Wmin_II_min-Wmin_II_max)/(SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN-SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL);
	Slope_III = (Wmin_III_min-Wmin_III_max)/(SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN-SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL);
	Slope_IV = (Wmin_IV_min-Wmin_IV_max)/(SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN-SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL);

	if (this->Physiological_Development_Percent<SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN)
	{
		this->Weight_II = Wmin_II_min;
		this->Weight_III = Wmin_III_min;
		this->Weight_IV = Wmin_IV_min;
	}
	else
	{
		this->Weight_II = Wmin_II_min + Slope_II*(this->Physiological_Development_Percent-SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN);
		this->Weight_III = Wmin_III_min + Slope_III*(this->Physiological_Development_Percent-SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN);
		this->Weight_IV = Wmin_IV_min + Slope_IV*(this->Physiological_Development_Percent-SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN);
	}


 return 0;
}

				
//this procedure calculates the number of new pupae for larval cohorts //#12
pair<double,double> Larvae_Cohort::Larval_pupation()
{
  double Pupation_Probability=0;
	
  double Larvae_survival_at_pupation_due_to_birth_defects;
  double NewlyPupated=0;
  double NewlyPupated_unadj=0;
  double WeightCadavers=0;

  if (this->Number==0) return pair<double,double>(0,0); //if the larval cohort is empty, end procedure

  if (this->Sex==FEMALE)
    {
      Larvae_survival_at_pupation_due_to_birth_defects=FEMALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS;
    }
  else
    {
      Larvae_survival_at_pupation_due_to_birth_defects=MALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS;
    }

  //all larvae pupate if the average weight of the larval cohort on the previous day exceeds the pupation weight threshold and if the larval cohort is mature
	

	if (SIMULATION_MODE1==CIMSIM)
	{
		if ((this->Previous_Day_Larval_Weight>this->Pupation_Weight)&&(this->Mature))
		{
		 Pupation_Probability=1;	
		 if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g\n","Minimum weight for pupation:",this->Pupation_Weight);
		}
	}

	if (SIMULATION_MODE1==SKEETER_BUSTER)
	{
		/// ???
		if ((this->Average_Larval_Weight>this->Pupation_Weight_100)&&(this->Mature)) 
		{
			Pupation_Probability=1; 
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g\n","Minimum weight for pupation(100):",this->Pupation_Weight_100);
		}
		if ((this->Average_Larval_Weight>this->Pupation_Weight_75)&&(this->Mature)&&(Pupation_Probability==0)) 
		{
			Pupation_Probability=0.75; 
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g\n","Minimum weight for pupation(75):",this->Pupation_Weight_75);
		}
		if ((this->Average_Larval_Weight>this->Pupation_Weight_50)&&(this->Mature)&&(Pupation_Probability==0)) 
		{
			Pupation_Probability=0.5; 
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END))fprintf(Output,"%s %.8g\n","Minimum weight for pupation(50):",this->Pupation_Weight_50);
		}
		if ((this->Average_Larval_Weight>this->Pupation_Weight_25)&&(this->Mature)&&(Pupation_Probability==0)) 
		{
			Pupation_Probability=0.25; 
			if ((DETAILED_LOG)&&(Date>=DETAILED_LOG_START)&&(Date<=DETAILED_LOG_END)) fprintf(Output,"%s %.8g\n","Minimum weight for pupation(25):",this->Pupation_Weight_25);
		}
		
		
	}

	 if (NewlyPupated<0)
	        		{
	        		fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
	        		}
	if (NOPUPATION) Pupation_Probability=0;
  //stochastic implementation of pupation for larval cohorts
  if (SIMULATION_MODE2==STOCHASTIC)
    {
		NewlyPupated_unadj=Binomial_Deviate(this->Number,Pupation_Probability);
		this->Number-=NewlyPupated_unadj;
		NewlyPupated=Binomial_Deviate(NewlyPupated_unadj,Larvae_survival_at_pupation_due_to_birth_defects);
		WeightCadavers+=(NewlyPupated_unadj-NewlyPupated)*this->Previous_Day_Larval_Weight;
    }
  //deterministic implementation of pupation for the larval cohorts
  if (NewlyPupated<0)
        		{
        		fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
        		}
  if (SIMULATION_MODE2==DETERMINISTIC)
    {
      NewlyPupated=this->Number*Pupation_Probability; //number of new pupae determine   
      this->Number=this->Number-this->Number*Pupation_Probability; //number of pupae remaining in the larval cohort

      WeightCadavers+=(NewlyPupated-NewlyPupated*Larvae_survival_at_pupation_due_to_birth_defects)*this->Previous_Day_Larval_Weight; //weight of cadavers fo next day increased by the total previous-day weight of the dead larvae due to birth defects during pupation

      NewlyPupated=NewlyPupated*Larvae_survival_at_pupation_due_to_birth_defects; //adjust the number of new pupae due to birth defects due to pupation
    }
  if (NewlyPupated<0 || WeightCadavers<0)
      		{
      		fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
      		}
  return pair<double,double>(NewlyPupated,WeightCadavers); 
};//this is the end of the pupation procedure for larval cohorts
    
//this is the end of the definition of the objects for unisex larval cohorts
