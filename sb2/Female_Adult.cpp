//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
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

#include "Globals.h"
#include "Binomial.h"
#include "Female_Adult.h"
#include "Fitness.h"
#include "DevelKinetics.h"
#include "Weather.h"
void ErrorMessage (int);

// default initializes to zero
Female_Adult::Female_Adult():  Female_Adult(0, 0, 0, 0, false, NULLIPAROUS, 0, 0, false, 0, 0, 0, 0, -1) 
{}

// authoritative
// all others route here
Female_Adult::Female_Adult(size_t rel_date, double devel_percent, int age, double avgWt, bool mature, int nullipar, int genotype,   int wolbachia,  bool mated, int male_genotype, double fecundity, int age_since_hatch, int male_wolbachia, int maternal_id) : 
  ID(Next_ID++),
  Release_Date(rel_date),
  Physiological_Development_Percent(devel_percent),
  Age(age),
  Average_Adult_Weight(avgWt),
  Mature(mature),
  Nulliparous(nullipar),
  Genotype(genotype),
  Wolbachia(wolbachia),
  Mated(mated),
  // unused?? set to zero?
  Male_Genotype(male_genotype ),
  // end csv
  Fecundity(fecundity),
  Age_since_hatch(age_since_hatch),
  Male_Wolbachia(male_wolbachia),
  Maternal_ID(maternal_id),
  // initialize to defaults
  Emergence_Place(MAX_SIZE_T),
  Source_Container_Type(-1),
  Oviposition_Events()
{}

// csv input, see test/base/release_adult_f.csv
// ?? male wolbachia and fecundity = 0?
// no number (spec[1]), emplace repeatedly
Female_Adult::Female_Adult( const vector<double> & spec) :
  Female_Adult(spec[0], spec[2], spec[3], spec[4], spec[5], spec[6], spec[7], spec[8], spec[9], spec[10], 0, 0, 0, -1)
{}

// spawn new females during sim
Female_Adult::Female_Adult( double avgWt, int nullipar, double fecundity, int genotype, int wolbachia, int age_since_hatch, bool mated, int male_genotype, int male_wolbachia, int maternal_id) : 
  Female_Adult(0, 0, 0, avgWt, 0, nullipar, genotype, wolbachia, mated, male_genotype, fecundity, age_since_hatch, male_wolbachia, maternal_id) 
{}
//these are the calculations for female adults individually
int Female_Adult::Female_Adult_Calculations(int Number_of_Suitable_Containers, const WeatherDay & thisWeather)
{
  double Nominal_Survival,Temperature_Survival,Dessication_Survival,Fitness_Survival,Overall_Survival;
  int OvipositingNewlyParous; // either Ovipositing or NewlyParous from original code!
  //calculate the physiological developmental percentage based on the enzyme kinetics approach
  double DevelRatio = DevelKineticsFemale(thisWeather.meanTempC, true);
  if (this->Age==0) //if the female adult nulliparous cohort contains new nulliparous female adults
    {
      this->Age++; 
        // REPEATED CODE!! see below #21
	  //this->Age_since_hatch++;//increment age
      //#21
      this->Physiological_Development_Percent += DevelRatio;

      //test for maturation

      if (this->Nulliparous==NULLIPAROUS)
		{
		 if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_first_gonotrophic_cycle()) this->Mature=1;
		}
      else
		{
		 if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_later_gonotrophic_cycles()) this->Mature=1;
		}				
      return 0; //end procedure (in CIMSIM, newly emerged nulliparous female adults do develop, but there's no mortality)
    }

  //#19
  OvipositingNewlyParous=0;

  Nominal_Survival=Temperature_Survival=Dessication_Survival=Overall_Survival=1;

  double age_dependence=1. ;
  switch(PbMsg.agedepsurv().female_age_dependent_survival())
  {
  case 0: //none
	  age_dependence=1. ;
	  break;

  case 1: //linear
	  age_dependence=1-((this->Age)/PbMsg.agedepsurv().female_maximum_age_1()) ;
	  if (age_dependence<0) age_dependence=0. ;
	  break;

  case 2: //constant then linear 
	  if ((this->Age)<PbMsg.agedepsurv().female_start_senescence_2()) age_dependence=1. ;
	  else age_dependence=1-(((this->Age)-PbMsg.agedepsurv().female_start_senescence_2())/(PbMsg.agedepsurv().female_maximum_age_2()-PbMsg.agedepsurv().female_start_senescence_2()));
	  if (age_dependence<0) age_dependence=0. ;
	  break;
  }

  double wolbachia_effects=1.;
  if (this->Wolbachia)
	  // survival reduction effects of Wolbachia infection
  {
	if ((this->Age)<PbMsg.wolbachia().wolbachia_survival_reduction_female_onset_age()) wolbachia_effects=1.;
	else wolbachia_effects=PbMsg.wolbachia().wolbachia_survival_reduction_female_factor();
  }


  //nominal (and age-dependent) female adult survival
  if (PbMsg.sim().simulation_mode1()==CIMSIM) Nominal_Survival=PbMsg.nominaldailysurvival().cimsim_female_adults();
  else Nominal_Survival=PbMsg.nominaldailysurvival().skeeter_buster_female_adults()*age_dependence*wolbachia_effects;
					
  //temperature related mortality
  if (thisWeather.maxTempC>=PbMsg.dailysurvival().female_adult_high_temperature_limit()) Temperature_Survival=PbMsg.dailysurvival().female_adult_high_temperature();
  if ((thisWeather.maxTempC<PbMsg.dailysurvival().female_adult_high_temperature_limit())&&(thisWeather.maxTempC>=PbMsg.dailysurvival().female_adult_normal_temperature_upper_limit())) Temperature_Survival=1-(1-PbMsg.dailysurvival().female_adult_high_temperature())/(PbMsg.dailysurvival().female_adult_high_temperature_limit()-PbMsg.dailysurvival().female_adult_normal_temperature_upper_limit())*(thisWeather.maxTempC-PbMsg.dailysurvival().female_adult_normal_temperature_upper_limit());

  if (thisWeather.minTempC<=PbMsg.dailysurvival().female_adult_low_temperature_limit()) Temperature_Survival=Temperature_Survival*PbMsg.dailysurvival().female_adult_low_temperature();
  if ((thisWeather.minTempC>PbMsg.dailysurvival().female_adult_low_temperature_limit())&&(thisWeather.minTempC<=PbMsg.dailysurvival().female_adult_normal_temperature_lower_limit())) Temperature_Survival=Temperature_Survival*(1-(1-PbMsg.dailysurvival().female_adult_low_temperature())/(PbMsg.dailysurvival().female_adult_normal_temperature_lower_limit()-PbMsg.dailysurvival().female_adult_low_temperature_limit())*(PbMsg.dailysurvival().female_adult_normal_temperature_lower_limit()-thisWeather.minTempC));
  // CORRECTED (see larvae)
					
  //dessication related mortality
  if (thisWeather.satDef<=PbMsg.dailysurvival().female_adult_low_saturationdeficit_limit()) Dessication_Survival=PbMsg.dailysurvival().female_adult_low_saturationdeficit();
  if (thisWeather.satDef>=PbMsg.dailysurvival().female_adult_high_saturationdeficit_limit()) Dessication_Survival=PbMsg.dailysurvival().female_adult_high_saturationdeficit();
  if ((thisWeather.satDef>PbMsg.dailysurvival().female_adult_low_saturationdeficit_limit())&&(thisWeather.satDef<PbMsg.dailysurvival().female_adult_high_saturationdeficit_limit())) Dessication_Survival=PbMsg.dailysurvival().female_adult_low_saturationdeficit()-((PbMsg.dailysurvival().female_adult_low_saturationdeficit()-PbMsg.dailysurvival().female_adult_high_saturationdeficit())/(PbMsg.dailysurvival().female_adult_high_saturationdeficit_limit()-PbMsg.dailysurvival().female_adult_low_saturationdeficit_limit()))*(thisWeather.satDef-PbMsg.dailysurvival().female_adult_low_saturationdeficit_limit());

  // fitness costs (if applicable at this stage)
  if (PbMsg.geneticcontrol().fitness_cost_stage()==3)
		if (this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age()) 
		{
			if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
			{	
				ErrorMessage(833);
			}
			else 
				// if not MEDEA, just apply fitness costs
			{
				Fitness_Survival=Fitness_Calculation(this->Genotype);

				if ((PbMsg.geneticcontrol().genetic_control_method()==ReduceReplace)&&(this->Source_Container_Type==102)&&(PbMsg.geneticcontrol().tetracycline()==1)) // If a female was raised with tetracycline, you can over-ride this fitness deduction
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
  {
	  return -99;
  }
	
  //deterministic implementation of survival for female adult cohorts
  //if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) this->Number=this->Number*Overall_Survival;
  // has to move (?)				

					
  //#20
  //procedure for oviposition if the female adult is mature, mated, has containers available and air is warm enough

  if ((this->Mated)&&(this->Mature)&&(Number_of_Suitable_Containers)&&(thisWeather.meanTempC>=PbMsg.devel().female_adult_minimum_temperature_for_oviposition()))
    {
      //stochastic implementation of ovipositing for female adult cohorts
      if (PbMsg.sim().simulation_mode2()==STOCHASTIC)
		{
		  OvipositingNewlyParous++;
		}  	
	
	  //deterministic implementation of ovipositing for female adult cohorts
      if (PbMsg.sim().simulation_mode2()==DETERMINISTIC)
		{
			  OvipositingNewlyParous++;
	    }
	

      // bug found by jungkoo, 
      // fixed in repo xian_compare
      //if (this->Nulliparous==PAROUS) 
      // should be:
      if (OvipositingNewlyParous)
		{
			this->Physiological_Development_Percent=0;
			this->Mature=0;
      }
    }

  //#21
  //calculation of physiological developmental rate based on the enzyme kinetics approach
  //development is after oviposition so that female only lay eggs a day after they become mature
  this->Physiological_Development_Percent += DevelRatio;
      
  if (this->Nulliparous==NULLIPAROUS)
    {
      if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_first_gonotrophic_cycle()) this->Mature=1;
    }
  else
    {
      if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_later_gonotrophic_cycles()) this->Mature=1;
    }
					
  //increment age
  this->Age++;
  this->Age_since_hatch++;

  //calculate fecundity //#22
  this->Fecundity=this->Average_Adult_Weight*PbMsg.devel().female_adult_daily_fecundity_factor_for_wet_weight();
  if (this->Age>25)
	  this->Fecundity+=PbMsg.devel().skeeter_buster_fecundity_drop_by_age()*(this->Age - 25);

  if ((PbMsg.geneticcontrol().fitness_cost_stage()==4)) // case of female fecundity fitness costs
  {
	  this->Fecundity*=Fitness_Calculation(this->Genotype);
  }

  if (this->Fecundity<0) this->Fecundity=0;

  return OvipositingNewlyParous;

};


//this is the end of the definition of the object for female nulliparous adult cohorts

//*********************************************
