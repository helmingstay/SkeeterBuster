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
#include "YoungCohorts.h"
#include "DevelKinetics.h"
void ErrorMessage (int);

//this is the calculations for unisex pupal cohorts
Trio Pupae_Cohort::Pupae_Calculations( double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Average_Temp)
{
    Trio Pupal_Data;
    Pupal_Data.Maturation=0;
    Pupal_Data.NewlyEmerged=0;
    Pupal_Data.WeightCadavers=0;
    //if the pupal cohort is empty, end procedure
    if (this->Number==0) 
        return Pupal_Data; 

    double Nominal_Survival,Temperature_Survival,Emergence_Survival,Overall_Survival,Fitness_Survival,Pupae_Number;
    double Emergence_Probability=0;
    double NewlyEmerged,NewlyEmerged_unadj;
    double Cummulative_Maturation(0.0), Previous_Cummulative_Maturation(0.0),Maturation(0.0),z;

    double Nominal_daily_survival_Pupae, Pupae_daily_survival_High_temperature_limit;
    double Pupae_daily_survival_High_temperature, Pupae_daily_survival_Low_temperature_limit;
    double Pupae_daily_survival_Low_temperature, Pupae_daily_survival_Normal_temperature_upper_limit;
    double Pupae_daily_survival_Normal_temperature_lower_limit, Pupae_survival_at_emergence;
    double WeightCadavers=0;

    // !!static, like larva GrowthParams
    if (this->Sex==FEMALE)
    {
      Nominal_daily_survival_Pupae=PbMsg.nominaldailysurvival().female_pupae();
      Pupae_daily_survival_High_temperature_limit=PbMsg.dailysurvival().female_pupae_high_temperature_limit();
      Pupae_daily_survival_High_temperature=PbMsg.dailysurvival().female_pupae_high_temperature();
      Pupae_daily_survival_Low_temperature_limit=PbMsg.dailysurvival().female_pupae_low_temperature_limit();
      Pupae_daily_survival_Low_temperature=PbMsg.dailysurvival().female_pupae_low_temperature();
      Pupae_daily_survival_Normal_temperature_upper_limit=PbMsg.dailysurvival().female_pupae_normal_temperature_upper_limit();
      Pupae_daily_survival_Normal_temperature_lower_limit=PbMsg.dailysurvival().female_pupae_normal_temperature_lower_limit();
      Pupae_survival_at_emergence=PbMsg.devel().female_pupae_survival_at_emergence();
    }
    else
    {
      Nominal_daily_survival_Pupae=PbMsg.nominaldailysurvival().male_pupae();
      Pupae_daily_survival_High_temperature_limit=PbMsg.dailysurvival().male_pupae_high_temperature_limit();
      Pupae_daily_survival_High_temperature=PbMsg.dailysurvival().male_pupae_high_temperature();
      Pupae_daily_survival_Low_temperature_limit=PbMsg.dailysurvival().male_pupae_low_temperature_limit();
      Pupae_daily_survival_Low_temperature=PbMsg.dailysurvival().male_pupae_low_temperature();
      Pupae_daily_survival_Normal_temperature_upper_limit=PbMsg.dailysurvival().male_pupae_normal_temperature_upper_limit();
      Pupae_daily_survival_Normal_temperature_lower_limit=PbMsg.dailysurvival().male_pupae_normal_temperature_lower_limit();
      Pupae_survival_at_emergence=PbMsg.devel().male_pupae_survival_at_emergence();
    }

    //#14
    //calculation of physiological developmental rate based on the enzyme kinetics approach

    if ((PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)&&(!this->Mature))
    {
        if (this->Physiological_Development_Percent<=PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first()) 
            Previous_Cummulative_Maturation=0;
        if (this->Physiological_Development_Percent>=PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()) 
            Previous_Cummulative_Maturation=1;
        if ((this->Physiological_Development_Percent>PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first())&&(this->Physiological_Development_Percent<PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last())) 
        {
            z=(PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()-this->Physiological_Development_Percent)/(PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()-PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first());
            Previous_Cummulative_Maturation=pow((1-z),PbMsg.enzyme().skeeter_buster_physiological_development_threshold_shape()*pow(z,2));
        }
    }

    double DevelRatio = DevelKineticsPupa( Water_Average_Temp, true);
    this->Physiological_Development_Percent += DevelRatio;

    if (this->Age==0) {
        this->Age=1; return Pupal_Data;
    } //if  cohort contains new pupae, increment age only and end procedure (in CIMSIM, new pupae only develop, but do not have mortality, and do not emerge)

    //#15
    NewlyEmerged=NewlyEmerged_unadj=0;
    Nominal_Survival=Temperature_Survival=Emergence_Survival=Overall_Survival=1;

                    
    //nominal pupae survival
    Nominal_Survival=Nominal_daily_survival_Pupae;



    //temperature related mortality using water temperature //COMMON CHANGES no. 2
    if (Water_Maximum_Temp>=Pupae_daily_survival_High_temperature_limit) 
        Temperature_Survival=Pupae_daily_survival_High_temperature;
    if ((Water_Maximum_Temp<Pupae_daily_survival_High_temperature_limit)&&(Water_Maximum_Temp>=Pupae_daily_survival_Normal_temperature_upper_limit)) 
        Temperature_Survival=1-(1-Pupae_daily_survival_High_temperature)/(Pupae_daily_survival_High_temperature_limit-Pupae_daily_survival_Normal_temperature_upper_limit)*(Water_Maximum_Temp-Pupae_daily_survival_Normal_temperature_upper_limit);

    if (Water_Minimum_Temp<=Pupae_daily_survival_Low_temperature_limit) 
        Temperature_Survival=Temperature_Survival*Pupae_daily_survival_Low_temperature;
    if ((Water_Minimum_Temp>Pupae_daily_survival_Low_temperature_limit)&&(Water_Minimum_Temp<=Pupae_daily_survival_Normal_temperature_lower_limit)) 
        Temperature_Survival=Temperature_Survival*(1-(1-Pupae_daily_survival_Low_temperature)/(Pupae_daily_survival_Normal_temperature_lower_limit-Pupae_daily_survival_Low_temperature_limit)*(Pupae_daily_survival_Normal_temperature_lower_limit-Water_Minimum_Temp));

    // CORRECTED used to be : if ((Water_Minimum_Temp>Pupae_daily_survival_Low_temperature_limit)&&(Water_Minimum_Temp<=Pupae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Pupae_daily_survival_Low_temperature)/(Pupae_daily_survival_Normal_temperature_lower_limit-Pupae_daily_survival_Low_temperature_limit)*(Water_Minimum_Temp-Pupae_daily_survival_Low_temperature_limit));


    // fitness costs (if applicable at this stage)
    if (PbMsg.geneticcontrol().fitness_cost_stage()==2)
    {
        if (this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age())
        {
            if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
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
    }


    //overall survival
    Overall_Survival=Nominal_Survival*Temperature_Survival;
    //stochastic implementation of survival for pupal cohorts
    if (PbMsg.sim().simulation_mode2()==STOCHASTIC) 
    {
      Pupae_Number=Binomial_Deviate(this->Number,Overall_Survival);
      WeightCadavers+=(this->Number-Pupae_Number)*this->Average_Pupal_Weight; //CIMSIM error here
      this->Number=Pupae_Number;
    }
    //deterministic implementation of survival for pupal cohorts
    if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) 
    {
      if (PbMsg.sim().simulation_mode1()==CIMSIM) 
            WeightCadavers+=(this->Number-this->Number*Overall_Survival*Pupae_survival_at_emergence)*this->Average_Pupal_Weight; //weight of cadavers for next day increased by weight of dead female pupae + 1-EmergenceSuccess //ERROR in CIMSIM code
      if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) 
        WeightCadavers+=(this->Number-this->Number*Overall_Survival)*this->Average_Pupal_Weight; //weight of cadavers for next day increased by weight of dead female pupae, no error 
      this->Number=this->Number*Overall_Survival;
    }

    //WARNING! Difference between deterministic and stochastic routine in CIMSIM, lack of EmergenceSuccess in CadaverWeightContribution
    //possible solution of EmergenceSuccess*this->Average_Pupal_Weight, but this needs testing

                    
    //#16
    //emergence
    //all  pupae emerge if  pupal cohort is mature 
    if (this->Mature) 
        Emergence_Probability=1;				
    //stochastic implementation to determine the number of new  adults based on the Emergence probability
    if (PbMsg.sim().simulation_mode2()==STOCHASTIC)
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
    if (PbMsg.sim().simulation_mode2()==DETERMINISTIC)
    {
        NewlyEmerged=this->Number*Emergence_Probability*Pupae_survival_at_emergence; //this calculates the number of new adults, adjusted for mortality
        if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) 
            WeightCadavers+=this->Number*Emergence_Probability*(1-Pupae_survival_at_emergence)*this->Average_Pupal_Weight; //this accounts for the pupae that die during emergence as larval food //COMMON CHANGE no. 3
        this->Number=this->Number-this->Number*Emergence_Probability; //decrease the number of pupae in the pupal cohorts due to emergence
    }

    // pupal cohort only becomes mature one day after physiological developmental percent has crossed the developmental threshold
    if (PbMsg.sim().simulation_mode1()==CIMSIM) 
        if (this->Physiological_Development_Percent>=PbMsg.enzyme().physiological_development_threshold_emergence()) 
            this->Mature=1; //check for the maturation of the  pupal cohort

    if ((PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)&&(!this->Mature))
    {
        if (this->Physiological_Development_Percent<=PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first()) 
            Cummulative_Maturation=0;
        if (this->Physiological_Development_Percent>=PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()) 
            Cummulative_Maturation=1;
        if ((this->Physiological_Development_Percent>PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first())&&(this->Physiological_Development_Percent<PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last())) 
        {
            z=(PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()-this->Physiological_Development_Percent)/(PbMsg.enzyme().skeeter_buster_physiological_development_threshold_last()-PbMsg.enzyme().skeeter_buster_physiological_development_threshold_first());
            Cummulative_Maturation=pow((1-z),PbMsg.enzyme().skeeter_buster_physiological_development_threshold_shape()*pow(z,2));
        }
        Maturation=Cummulative_Maturation-Previous_Cummulative_Maturation;
        if (Cummulative_Maturation==1) 
            this->Mature=1;	
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
