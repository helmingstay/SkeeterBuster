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
#include "Fitness.h"
#include "Male_Adult_Cohort.h" 
#include "DevelKinetics.h" 
#include "Weather.h" 
void ErrorMessage (int);


// authoritative, all others point here
Male_Adult_Cohort::Male_Adult_Cohort(size_t release_date, double num, double physdev, int age, double avgWt, bool mature, int genotype, int wolbachia,  int age_since_hatch) :
    // Age, mature, genotype, wolbachia, date
    Cohort(num, age, mature, genotype, wolbachia, release_date),
    ID(Next_ID++),
    Emergence_Place(MAX_SIZE_T),
    Age_since_hatch(age_since_hatch),
    Physiological_Development_Percent(physdev),
    Average_Adult_Weight(avgWt)
{};

// initialization, should be unused
Male_Adult_Cohort::Male_Adult_Cohort():
    Male_Adult_Cohort(0, 0, 0, 0, 0, false, 0, 0, 0)
{};

// spawn during sim 
Male_Adult_Cohort::Male_Adult_Cohort( int num, double avgWt, int genotype, int wolbachia, int age_since_hatch) :
    Male_Adult_Cohort(0, num, 0, 0, avgWt, false, genotype, wolbachia, age_since_hatch)
{};

//date,number,phys_devel_percent,age,average_weight,mature,genotype,wolbachia
Male_Adult_Cohort::Male_Adult_Cohort( vector<double> spec) :
    Male_Adult_Cohort(spec[0], spec[1], spec[2], spec[3], spec[4], spec[5], spec[6], spec[7], 0) 
{};

void Male_Adult_Cohort::Male_Adult_Calculations(const WeatherDay & thisWeather)
{
    if (this->Number==0) return; //if the male adult cohort is empty, end procedure
    if (this->Age==0) {this->Age++;return;} //if the male adult cohort contains new adult males, increment age and end procedure

    double Nominal_Survival(1), Temperature_Survival(1),Dessication_Survival(1),Overall_Survival(1);
    double Fitness_Survival;

    //#26

    double age_dependence=1. ;
    switch(PbMsg.agedepsurv().male_age_dependent_survival())
    {
    case 0: //none
      age_dependence=1. ;
      break;

    case 1: //linear
      age_dependence=1-((this->Age)/PbMsg.agedepsurv().male_maximum_age_1()) ;
      if (age_dependence<0) {
        age_dependence=0. ;
      }
      break;

    case 2: //constant then linear 
      if ((this->Age)<PbMsg.agedepsurv().male_start_senescence_2()) {
        age_dependence=1. ;
      } else {
        age_dependence=1-(((this->Age)-PbMsg.agedepsurv().male_start_senescence_2())/(PbMsg.agedepsurv().male_maximum_age_2()-PbMsg.agedepsurv().male_start_senescence_2()));
      }
      if (age_dependence<0) {
            age_dependence=0. ;
      }
      break;
    }

    double wolbachia_effects=1.;
    if (this->Wolbachia) {
      // survival reduction effects of Wolbachia infection
        if ((this->Age)<PbMsg.wolbachia().wolbachia_survival_reduction_male_onset_age()) 
        {
            wolbachia_effects=1.;
        } else {
            wolbachia_effects=PbMsg.wolbachia().wolbachia_survival_reduction_male_factor();
        }
    }

    //nominal male adult nulliparous survival
    if (PbMsg.sim().simulation_mode1()==CIMSIM) {
        Nominal_Survival=PbMsg.nominaldailysurvival().cimsim_male_adults();
    } else {
        Nominal_Survival=PbMsg.nominaldailysurvival().skeeter_buster_male_adults()*age_dependence*wolbachia_effects;
    }

    //temperature related mortality
    if (thisWeather.maxTempC>=PbMsg.dailysurvival().male_adult_high_temperature_limit())
    {
        Temperature_Survival=PbMsg.dailysurvival().male_adult_high_temperature();
    }
    if ((thisWeather.maxTempC<PbMsg.dailysurvival().male_adult_high_temperature_limit())&&(thisWeather.maxTempC>=PbMsg.dailysurvival().male_adult_normal_temperature_upper_limit()))    
    {
        Temperature_Survival=1-(1-PbMsg.dailysurvival().male_adult_high_temperature())/(PbMsg.dailysurvival().male_adult_high_temperature_limit()-PbMsg.dailysurvival().male_adult_normal_temperature_upper_limit())*(thisWeather.maxTempC-PbMsg.dailysurvival().male_adult_normal_temperature_upper_limit());
    }

    if (thisWeather.minTempC<=PbMsg.dailysurvival().male_adult_low_temperature_limit()) 
    {
        Temperature_Survival=Temperature_Survival*PbMsg.dailysurvival().male_adult_low_temperature();
    }
    if ((thisWeather.minTempC>PbMsg.dailysurvival().male_adult_low_temperature_limit())&&(thisWeather.minTempC<=PbMsg.dailysurvival().male_adult_normal_temperature_lower_limit())) 
    {
        Temperature_Survival=Temperature_Survival*(1-(1-PbMsg.dailysurvival().male_adult_low_temperature())/(PbMsg.dailysurvival().male_adult_normal_temperature_lower_limit()-PbMsg.dailysurvival().female_adult_low_temperature_limit())*(PbMsg.dailysurvival().male_adult_normal_temperature_lower_limit()-thisWeather.minTempC));
    }
    // CORRECTED (see larvae)				

    //dessication related mortality
    if (thisWeather.satDef<=PbMsg.dailysurvival().male_adult_low_saturationdeficit_limit()) 
    {
        Dessication_Survival=PbMsg.dailysurvival().male_adult_low_saturationdeficit();
    }
    if (thisWeather.satDef>=PbMsg.dailysurvival().male_adult_high_saturationdeficit_limit()) 
    {
        Dessication_Survival=PbMsg.dailysurvival().male_adult_high_saturationdeficit();
    }
    if ((thisWeather.satDef>PbMsg.dailysurvival().male_adult_low_saturationdeficit_limit())&&(thisWeather.satDef<PbMsg.dailysurvival().male_adult_high_saturationdeficit_limit())) 
    {
        Dessication_Survival=PbMsg.dailysurvival().male_adult_low_saturationdeficit()-((PbMsg.dailysurvival().male_adult_low_saturationdeficit()-PbMsg.dailysurvival().male_adult_high_saturationdeficit())/(PbMsg.dailysurvival().male_adult_high_saturationdeficit_limit()-PbMsg.dailysurvival().male_adult_low_saturationdeficit_limit()))*(thisWeather.satDef-PbMsg.dailysurvival().male_adult_low_saturationdeficit_limit());
    }

    // fitness costs (if applicable at this stage)
    if (PbMsg.geneticcontrol().fitness_cost_stage()==3)
    {
        if (this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age()) 
        {
            if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
            {	
                fprintf(stdout, "\n\nError: male adult fitness costs are incompatible with MEDEA genetic control method\n");
                ErrorMessage(833);
            } else 
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
    if (PbMsg.sim().simulation_mode2()==STOCHASTIC) 
    {
        this->Number=Binomial_Deviate(this->Number,Overall_Survival);
    }

    //deterministic implementation for male adult cohorts
    if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) 
    {
        this->Number=this->Number*Overall_Survival;
    }
                    
    //#27
    //calculate physiological developmental rate based on the enzyme kinetics approach
    double DevelRatio = DevelKineticsFemale(thisWeather.meanTempC, true);
    this->Physiological_Development_Percent += DevelRatio;
    //check for maturation
    if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_later_gonotrophic_cycles()) {
        this->Mature=1;
    }
                        
    //increment age
    this->Age++;
    this->Age_since_hatch++;
};
