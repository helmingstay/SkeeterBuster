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
#include "YoungCohorts.h"
#include "Fitness.h"
#include "DevelKinetics.h"
#include "Weather.h"


/*
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
*/

double Eggs_Cohort::Egg_Calculations( const WeatherDay & thisWeather, double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Depth, double Sun_Exposure, double Water_Average_Temp)
{
	if (this->Number==0)
		return 0; //this quits the procedure if the cohort is empty

	//clearing of variables
	//Nominal_Survival=Temperature_Survival=Dessication_Survival=Predation_Survival=Fitness_Survival=Overall_Survival=1;
	double Nominal_Survival(1);
	double Temperature_Survival(1);
	double Dessication_Survival(1);
	double Predation_Survival(1);
	double Fitness_Survival(1);
	double Overall_Survival(1);
	double NewlyHatched=0;
	double Hatch_Probability=0; 
	int EggHatch_Level; //level in the container where eggs still hatch
	
	//#5
	//nominal egg survival
	Nominal_Survival=PbMsg.nominaldailysurvival().eggs();

	//temperature related mortality, based on CIMSIM code
	if (Water_Maximum_Temp>=PbMsg.dailysurvival().egg_high_temperature_limit()) Temperature_Survival=PbMsg.dailysurvival().egg_high_temperature();
	if ((Water_Maximum_Temp<PbMsg.dailysurvival().egg_high_temperature_limit())&&(Water_Maximum_Temp>=PbMsg.dailysurvival().egg_normal_temperature_upper_limit())) Temperature_Survival=1-(1-PbMsg.dailysurvival().egg_high_temperature())/(PbMsg.dailysurvival().egg_high_temperature_limit()-PbMsg.dailysurvival().egg_normal_temperature_upper_limit())*(Water_Maximum_Temp-PbMsg.dailysurvival().egg_normal_temperature_upper_limit());

	if (Water_Minimum_Temp<=PbMsg.dailysurvival().egg_low_temperature_limit()) Temperature_Survival=Temperature_Survival*PbMsg.dailysurvival().egg_low_temperature();
	if ((Water_Minimum_Temp>PbMsg.dailysurvival().egg_low_temperature_limit())&&(Water_Minimum_Temp<=PbMsg.dailysurvival().egg_normal_temperature_lower_limit())) Temperature_Survival=Temperature_Survival*(1-(1-PbMsg.dailysurvival().egg_low_temperature())/(PbMsg.dailysurvival().egg_normal_temperature_lower_limit()-PbMsg.dailysurvival().egg_low_temperature_limit())*(PbMsg.dailysurvival().egg_normal_temperature_lower_limit()-Water_Minimum_Temp));	
	// CORRECTED (see larvae)

	if (Water_Depth==0)
	{
		if (thisWeather.maxTempC>=PbMsg.dailysurvival().egg_high_temperature_limit()) Temperature_Survival=PbMsg.dailysurvival().egg_high_temperature();
		if (thisWeather.minTempC<=PbMsg.dailysurvival().egg_low_temperature_limit()) Temperature_Survival=PbMsg.dailysurvival().egg_low_temperature();
		if ((thisWeather.maxTempC<PbMsg.dailysurvival().egg_high_temperature_limit())&&(thisWeather.maxTempC>=PbMsg.dailysurvival().egg_normal_temperature_upper_limit())) Temperature_Survival=1-(1-PbMsg.dailysurvival().egg_high_temperature())/(PbMsg.dailysurvival().egg_high_temperature_limit()-PbMsg.dailysurvival().egg_normal_temperature_upper_limit())*(thisWeather.maxTempC-PbMsg.dailysurvival().egg_normal_temperature_upper_limit());
		if ((thisWeather.minTempC>PbMsg.dailysurvival().egg_low_temperature_limit())&&(thisWeather.minTempC<=PbMsg.dailysurvival().egg_normal_temperature_lower_limit())) Temperature_Survival=1-(1-PbMsg.dailysurvival().egg_low_temperature())/(PbMsg.dailysurvival().egg_normal_temperature_lower_limit()-PbMsg.dailysurvival().egg_low_temperature_limit())*(PbMsg.dailysurvival().egg_normal_temperature_lower_limit()-thisWeather.minTempC);
	}
	
	
	//dessication related mortality
	if (Water_Depth==0)
	{
		if (Sun_Exposure>PbMsg.dailysurvival().egg_dry_container_sunexposure_limit()) Dessication_Survival=PbMsg.dailysurvival().egg_dry_container_high_sunexposure();
		if (Sun_Exposure<=PbMsg.dailysurvival().egg_dry_container_sunexposure_limit()) 
		{
			if (thisWeather.satDef<=PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit_limit()) Dessication_Survival=PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit();
			if (thisWeather.satDef>=PbMsg.dailysurvival().egg_dry_container_low_sunexposure_high_saturationdeficit_limit()) Dessication_Survival=PbMsg.dailysurvival().egg_dry_container_low_sunexposure_high_saturationdeficit();
			if ((thisWeather.satDef>PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit_limit())&&(thisWeather.satDef<PbMsg.dailysurvival().egg_dry_container_low_sunexposure_high_saturationdeficit_limit())) Dessication_Survival=PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit()-((PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit()-PbMsg.dailysurvival().egg_dry_container_low_sunexposure_high_saturationdeficit())/(PbMsg.dailysurvival().egg_dry_container_low_sunexposure_high_saturationdeficit_limit()-PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit_limit()))*(thisWeather.satDef-PbMsg.dailysurvival().egg_dry_container_low_sunexposure_low_saturationdeficit_limit());
		}
	}
	else Dessication_Survival=1;
	
	//mortality due to predation, based on CIMSIM code
	if (Water_Average_Temp>=PbMsg.dailysurvival().egg_topredation_high_temperature_limit()) Predation_Survival=PbMsg.dailysurvival().egg_topredation_high_temperature();
	if ((Water_Average_Temp<PbMsg.dailysurvival().egg_topredation_high_temperature_limit())&&(Water_Average_Temp>PbMsg.dailysurvival().egg_topredation_low_temperature_limit())) Predation_Survival=1-(1-PbMsg.dailysurvival().egg_topredation_high_temperature())/(PbMsg.dailysurvival().egg_topredation_high_temperature_limit()-PbMsg.dailysurvival().egg_topredation_low_temperature_limit())*(Water_Average_Temp-PbMsg.dailysurvival().egg_topredation_low_temperature_limit());
	if (Water_Average_Temp<PbMsg.dailysurvival().egg_topredation_low_temperature_limit()) Predation_Survival=1;
	
	if (Water_Depth==0)
	{
		if (thisWeather.meanTempC>=PbMsg.dailysurvival().egg_topredation_high_temperature_limit()) Predation_Survival=PbMsg.dailysurvival().egg_topredation_high_temperature();
		if ((thisWeather.meanTempC<PbMsg.dailysurvival().egg_topredation_high_temperature_limit())&&(thisWeather.meanTempC>PbMsg.dailysurvival().egg_topredation_low_temperature_limit())) Predation_Survival=1-(1-PbMsg.dailysurvival().egg_topredation_high_temperature())/(PbMsg.dailysurvival().egg_topredation_high_temperature_limit()-PbMsg.dailysurvival().egg_topredation_low_temperature_limit())*(thisWeather.meanTempC-PbMsg.dailysurvival().egg_topredation_low_temperature_limit());
		if (thisWeather.meanTempC<PbMsg.dailysurvival().egg_topredation_low_temperature_limit()) Predation_Survival=1;
	}
	
	if ((PbMsg.geneticcontrol().fitness_cost_stage()==0)&&(PbMsg.geneticcontrol().embryonic_cost_selection_type()==HARD))
		// case of embryonic fitness costs with hard selection
		if ((this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age())&&(Date!=0)) 
		{
			if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
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
	if (PbMsg.geneticcontrol().genetic_control_method()==ReduceReplace)
    {
        if ((PbMsg.geneticcontrol().fitness_cost_stage_rr()==0)&&(PbMsg.geneticcontrol().embryonic_cost_selection_type()==HARD))
        {
            // case of embryonic fitness costs with hard selection
            if ((this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age())&&(Date!=0))
            {
                Fitness_Survival=Fitness_Calculation_RR(this->Genotype);
                this->Number=Binomial_Deviate(this->Number, Fitness_Survival) ;
            }
        }
    }

	// wolbachia egg survival -- affects Dessication_Survival factor

	if (PbMsg.wolbachia().number_of_incompatible_wolbachia())
		if (this->Wolbachia) // no effect of multiple infections
			if (this->Age > PbMsg.wolbachia().wolbachia_egg_survival_reduction_onset_age())
				Dessication_Survival *= PbMsg.wolbachia().wolbachia_egg_survival_reduction_factor();


	//overall survival
	Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival*Predation_Survival;
	//stochastic implementation of egg survival
	if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
	//	fprintf(stdout, "Before: %.3f - ", this->Number);
		this->Number=Binomial_Deviate(this->Number,Overall_Survival);
	// 	fprintf(stdout, "After: %.3f\n", this->Number);
	}
	//deterministic implementation of egg survival
	if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) 
        this->Number=this->Number*Overall_Survival;
	
	//#6
	//hatch
	if (PbMsg.sim().simulation_mode1()==CIMSIM)  {
        EggHatch_Level=(int) ROUND_(Water_Depth/2+0.2); //based on CIMSIM, this is the level at which eggs still hatch 
	} else if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) {
        EggHatch_Level=(int) ROUND_(Water_Depth/0.2+2);  
    } else {
        throw std::runtime_error("simulation_mode1 not recognized");
    }
	
	if (!this->Mature) //if egg cohort is not yet mature
	{
		if (this->Physiological_Development_Percent>PbMsg.enzyme().physiological_development_threshold_embryonation()) //if egg is developed above threshold
		{
			if (Water_Average_Temp>=PbMsg.devel().minimum_temperature_for_egghatch()) //if water temperature allows egghatch
			{
				if (this->Level_laid_in_Container<=EggHatch_Level) 
                    Hatch_Probability=1; //if eggs are not above the level where eggs still hatch, then all eggs hatch
				else //if they are above water
				{
					Hatch_Probability=PbMsg.devel().ratio_of_eggs_hatching_without_flooding(); //a portion of them hatches
					this->Mature=1; //the rest becomes mature
				}
				
			}
			if (Water_Average_Temp<PbMsg.devel().minimum_temperature_for_egghatch()) //water is too cold to hatch
			{
				this->Mature=1; //all eggs become mature
				Hatch_Probability=0; //none of them hatches
			}
		}
		else //eggs are not yet developed above threshold
		{
			//development
			
			double DevelRatio = DevelKineticsEgg(Water_Average_Temp, true);
			this->Physiological_Development_Percent += DevelRatio;
		}
	}
	else //eggs are already mature
	{
		if (Water_Average_Temp>=PbMsg.devel().minimum_temperature_for_egghatch()) if (this->Level_laid_in_Container<=EggHatch_Level) Hatch_Probability=PbMsg.devel().daily_ratio_of_embryonated_eggs_that_hatch_if_submerged(); //if temperature is above threshold and eggs are under water then a portion of the eggs hatch
	}
	if (Water_Depth==0) 
        Hatch_Probability=0; //if the container is dry, no eggs hatch
	
	//calculation of hatching eggs
	
	if (Hatch_Probability>0)  
	{
	    //stochastic implementation of egg hatch
		if (PbMsg.sim().simulation_mode2()==STOCHASTIC) 
            NewlyHatched=Binomial_Deviate(this->Number,Hatch_Probability);
        //deterministic implementation of egg hatch
        if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) 
            NewlyHatched=this->Number*Hatch_Probability;    
	}
	
	//age incremented
	this->Age++;
	
	return NewlyHatched;
}
