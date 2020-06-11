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
#include "Building.h"
#include "DevelKinetics.h"
#include "LarvalParamStructs.h"
void ErrorMessage (int);
 
extern size_t Date;

//extern long ra[M+1], nd;

//#8
double Larvae_Cohort::Larvae_Calculations(double Water_Average_Temp) {
    double Larvae_Chronological_basis_at_26C;
    double Devel_Ratio(0);
    double Cummulative_Maturation(0.0), Previous_Cummulative_Maturation(0.0), Maturation(0.0);
    double z;

    //if the larval cohort is empty, end procedure
    if (this->Number==0) return 0;

    if (this->Sex==FEMALE)
    {
        Larvae_Chronological_basis_at_26C=PbMsg.femalelarvae().female_larvae_chronological_basis_at_26c();
    }
    else
    {
        Larvae_Chronological_basis_at_26C=PbMsg.malelarvae().male_larvae_chronological_basis_at_26c();
    }

          
    //calculation of developmental rate for larval cohorts based on the enzyme kinetics approach
    if (PbMsg.sim().development_mode()==FOCKSDEVEL) {
        // true: convert to kelvin
        Devel_Ratio = DevelKineticsLarva(Water_Average_Temp, true);
    }
    //calculation of developmental rate for larval cohorts based on the degree-day approach
    else {
        Devel_Ratio=(Water_Average_Temp-PbMsg.devel().minimum_developmental_temperature())/PbMsg.devel().degree_hours_to_be_reached();
    }

    //update physiological development percent //OUR CHANGE #4 variation in larval maturation Rueda, Wilhoit

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

    this->Physiological_Development_Percent=this->Physiological_Development_Percent+Devel_Ratio;
    if (PbMsg.sim().simulation_mode1()==CIMSIM) 
        if (this->Physiological_Development_Percent>=PbMsg.enzyme().cimsim_physiological_development_threshold_pupation()) 
            this->Mature=1;

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
        if (Previous_Cummulative_Maturation <1) // if it is 1, then Cumulative Maturation will also be 1 and the cohort will mature
            Maturation=(Cummulative_Maturation-Previous_Cummulative_Maturation)/(1-Previous_Cummulative_Maturation);
        if (Cummulative_Maturation==1) 
            this->Mature=1;
    }


    //if the larval cohort contains new larvae then end procedure
    if (this->Age==0) 
        return Maturation;
                    

    //prepare for Gilpin-McClelland equation Euler integration calculations by calculating the chronological basis based on the developmental rate of the day
    this->Previous_Day_Larval_Weight=this->Average_Larval_Weight; //store the current weight as the weight on the day before

    //Larval Chronological Basis calculation according without error
    this->Larvae_Chronological_basis=Larvae_Chronological_basis_at_26C*(Devel_Ratio-Building::Development_134)/(Building::Development_26-Building::Development_134);

    //increment age
    this->Age++;
    this->Age_since_hatch++;

    return Maturation; 
}

//inline double Larvae_Cohort::Larval_growth(double food_exploit, double Container_Volume) noexcept
				
//this procedure calculates the survival of larval cohorts //#10
double Larvae_Cohort::Larval_survival(double Water_Maximum, double Water_Minimum, double Water_Depth)
{
    double Nominal_Survival,Temperature_Survival,Dessication_Survival,Fasting_Survival,Fitness_Survival,Overall_Survival,LarvaeNumber;

    double WeightCadavers=0;

    double Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one,Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two;
    double Larvae_Minimum_lipid_ratio_of_living_larvae,Larvae_daily_survival_fasting_with_lipid_reserve,Larvae_daily_survival_fasting_without_lipid_reserve;
    double Nominal_daily_survival_larvae,Larvae_daily_survival_High_temperature_limit,Larvae_daily_survival_High_temperature,Larvae_daily_survival_Low_temperature_limit;
    double Larvae_daily_survival_Low_temperature,Larvae_daily_survival_Normal_temperature_upper_limit,Larvae_daily_survival_Normal_temperature_lower_limit;
    double Larvae_daily_survival_Dry_container,Larvae_smallest_weight_for_survival;
        
    if (this->Number==0) return 0; //if the larval cohort is empty, end procedure
    if (this->Age==0) {this->Age++;return 0;} //if the larval cohort contains new larvae, increment age and end procedure (in CIMSIM, new larvae on the day they hatch only increase their development) 

    if (this->Sex==FEMALE)
    {
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one=PbMsg.femalelarvae().female_larvae_conversion_of_dry_body_weight_to_lipid_reserve_parameter_one();
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two=PbMsg.femalelarvae().female_larvae_conversion_of_dry_body_weight_to_lipid_reserve_parameter_two();
      Larvae_Minimum_lipid_ratio_of_living_larvae=PbMsg.femalelarvae().female_larvae_minimum_lipid_ratio_of_living_larvae();
      Larvae_daily_survival_fasting_with_lipid_reserve=PbMsg.femalelarvae().female_larvae_daily_survival_fasting_with_lipid_reserve();
      Larvae_daily_survival_fasting_without_lipid_reserve=PbMsg.femalelarvae().female_larvae_daily_survival_fasting_without_lipid_reserve();
      Nominal_daily_survival_larvae=PbMsg.nominaldailysurvival().female_larvae();
      Larvae_daily_survival_High_temperature_limit=PbMsg.dailysurvival().female_larvae_high_temperature_limit();
      Larvae_daily_survival_High_temperature=PbMsg.dailysurvival().female_larvae_high_temperature();
      Larvae_daily_survival_Low_temperature_limit=PbMsg.dailysurvival().female_larvae_low_temperature_limit();
      Larvae_daily_survival_Low_temperature=PbMsg.dailysurvival().female_larvae_low_temperature();
      Larvae_daily_survival_Normal_temperature_upper_limit=PbMsg.dailysurvival().female_larvae_normal_temperature_upper_limit();
      Larvae_daily_survival_Normal_temperature_lower_limit=PbMsg.dailysurvival().female_larvae_normal_temperature_lower_limit();
      Larvae_daily_survival_Dry_container=PbMsg.dailysurvival().female_larvae_dry_container();	
      Larvae_smallest_weight_for_survival=PbMsg.femalelarvae().female_larvae_smallest_weight_for_survival();
    }
    else
    {
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one=PbMsg.malelarvae().male_larvae_conversion_of_dry_body_weight_to_lipid_reserve_parameter_one();
      Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two=PbMsg.malelarvae().male_larvae_conversion_of_dry_body_weight_to_lipid_reserve_parameter_two();
      Larvae_Minimum_lipid_ratio_of_living_larvae=PbMsg.malelarvae().male_larvae_minimum_lipid_ratio_of_living_larvae();
      Larvae_daily_survival_fasting_with_lipid_reserve=PbMsg.malelarvae().male_larvae_daily_survival_fasting_with_lipid_reserve();
      Larvae_daily_survival_fasting_without_lipid_reserve=PbMsg.malelarvae().male_larvae_daily_survival_fasting_without_lipid_reserve();
      Nominal_daily_survival_larvae=PbMsg.nominaldailysurvival().male_larvae();
      Larvae_daily_survival_High_temperature_limit=PbMsg.dailysurvival().male_larvae_high_temperature_limit();
      Larvae_daily_survival_High_temperature=PbMsg.dailysurvival().male_larvae_high_temperature();
      Larvae_daily_survival_Low_temperature_limit=PbMsg.dailysurvival().male_larvae_low_temperature_limit();
      Larvae_daily_survival_Low_temperature=PbMsg.dailysurvival().male_larvae_low_temperature();
      Larvae_daily_survival_Normal_temperature_upper_limit=PbMsg.dailysurvival().male_larvae_normal_temperature_upper_limit();
      Larvae_daily_survival_Normal_temperature_lower_limit=PbMsg.dailysurvival().male_larvae_normal_temperature_lower_limit();
      Larvae_daily_survival_Dry_container=PbMsg.dailysurvival().male_larvae_dry_container();	
      Larvae_smallest_weight_for_survival=PbMsg.malelarvae().male_larvae_smallest_weight_for_survival();
    }


    Overall_Survival=Nominal_Survival=Temperature_Survival=Dessication_Survival=Fasting_Survival=1;
                    
    //calculation of fasting survival for the larval cohort, based on CIMSIM code

    if (this->Previous_Day_Larval_Weight<=this->Average_Larval_Weight) //if weight of the female larval cohort increased
    {
        if (!(this->Prefasting_Lipid_Reserve==0)) //if lipid reserve was positive - previous fasting occured
        {
            this->Average_Lipid_Reserve=this->Average_Lipid_Reserve+this->Average_Larval_Weight-this->Previous_Day_Larval_Weight; //add weight increase to average lipid reserve
            if (this->Average_Lipid_Reserve>this->Prefasting_Lipid_Reserve) 
            {
                this->Prefasting_Lipid_Reserve=0; 
                if (PbMsg.sim().simulation_mode1()==CIMSIM) 
                    this->Average_Lipid_Reserve=0;
            } //if average lipid reserve surpassed the lipid reserve at the onset of fasting, then resupplying over, clear lipid reserves values
        }
      Fasting_Survival=1; //no mortality due to fasting
    }
    if (this->Previous_Day_Larval_Weight>this->Average_Larval_Weight) //if weight of the larval cohort decreased - fasting
    {
        if (this->Prefasting_Lipid_Reserve==0) //if this is the first day of fasting
        {
          this->Prefasting_Lipid_Reserve=this->Previous_Day_Larval_Weight*(Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_one*(log(this->Previous_Day_Larval_Weight)+Larvae_Conversion_of_dry_body_weight_to_lipid_reserve_parameter_two)-Larvae_Minimum_lipid_ratio_of_living_larvae); //calculate lipid reserve at the onset of fasting
          if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) 
            if (this->Prefasting_Lipid_Reserve<0) 
                this->Prefasting_Lipid_Reserve=0;
          this->Average_Lipid_Reserve=this->Prefasting_Lipid_Reserve-this->Previous_Day_Larval_Weight+this->Average_Larval_Weight; //substract weight loss from prefasting lipid reserve
        }
        else //if previous fasting 
        {
          this->Average_Lipid_Reserve=this->Average_Lipid_Reserve-this->Previous_Day_Larval_Weight+this->Average_Larval_Weight; //decreae lipid reserve by weight loss
        }

        // Jung koo edit / bug
        // some effect on sim??
        Average_Lipid_Reserve = std::max(0.0, Average_Lipid_Reserve);

        if (this->Average_Lipid_Reserve>0) Fasting_Survival=Larvae_daily_survival_fasting_with_lipid_reserve; //if average lipid reserve positive
        if (this->Average_Lipid_Reserve<=0) Fasting_Survival=Larvae_daily_survival_fasting_without_lipid_reserve; //if average lipid reserve zero
    }
    //ORIGINAL CIMSIM, HAS NO REASON WHY Fasting Survival should not be recalculated when the Previous Day's larval weight is higher than the pupation weight threshold
    if (PbMsg.sim().simulation_mode1()==CIMSIM) 
        if (this->Previous_Day_Larval_Weight>this->Pupation_Weight) 
            Fasting_Survival=this->Previous_Day_Fasting_Survival; //if weight is above pupation threshold, the last calculated fasting survival is applied
                    

    //nominal larval survival

    Nominal_Survival=Nominal_daily_survival_larvae;
    //Nominal_Survival=1; //alternative setting

    //temperature related mortality

    if (Water_Maximum>=Larvae_daily_survival_High_temperature_limit) 
        Temperature_Survival=Larvae_daily_survival_High_temperature;
    if ((Water_Maximum<Larvae_daily_survival_High_temperature_limit)&&(Water_Maximum>=Larvae_daily_survival_Normal_temperature_upper_limit)) 
        Temperature_Survival=1-(1-Larvae_daily_survival_High_temperature)/(Larvae_daily_survival_High_temperature_limit-Larvae_daily_survival_Normal_temperature_upper_limit)*(Water_Maximum-Larvae_daily_survival_Normal_temperature_upper_limit);

    if (Water_Minimum<=Larvae_daily_survival_Low_temperature_limit) 
        Temperature_Survival=Temperature_Survival*Larvae_daily_survival_Low_temperature;
    if ((Water_Minimum>Larvae_daily_survival_Low_temperature_limit)&&(Water_Minimum<=Larvae_daily_survival_Normal_temperature_lower_limit)) 
        Temperature_Survival=Temperature_Survival*(1-(1-Larvae_daily_survival_Low_temperature)/(Larvae_daily_survival_Normal_temperature_lower_limit-Larvae_daily_survival_Low_temperature_limit)*(Larvae_daily_survival_Normal_temperature_lower_limit-Water_Minimum));

    // CORRECTED: used to be if ((Water_Minimum>Larvae_daily_survival_Low_temperature_limit)&&(Water_Minimum<=Larvae_daily_survival_Normal_temperature_lower_limit)) Temperature_Survival=Temperature_Survival*(1-(1-Larvae_daily_survival_Low_temperature)/(Larvae_daily_survival_Normal_temperature_lower_limit-Larvae_daily_survival_Low_temperature_limit)*(Water_Minimum-Larvae_daily_survival_Low_temperature_limit));

    //dessication related mortality

    if (Water_Depth==0) 
        Dessication_Survival=Larvae_daily_survival_Dry_container;

    // fitness costs (if applicable at this stage)
    if (PbMsg.geneticcontrol().fitness_cost_stage()==1)
    {
        if ((this->Age==PbMsg.geneticcontrol().fitness_cost_stage_specific_age())&&(Date!=0)) 
        {
            if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
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
    }

    //overall survival
    Overall_Survival=Nominal_Survival*Temperature_Survival*Dessication_Survival*Fasting_Survival;
    //all larvae are killed if the physiological developmental threshold is higher then a maximum threshold

    if (this->Physiological_Development_Percent>PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival) 
        Overall_Survival=0;
    //all larvae are killed if their weight goes below a minimum threshold

    if (this->Average_Larval_Weight<Larvae_smallest_weight_for_survival) 
        Overall_Survival=0;
    //stochastic implementation for survival of the larval cohort

    if (PbMsg.sim().simulation_mode2()==STOCHASTIC)	
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
    if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) 
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

    if (this->Number==0) return 0;

    Wmin=PupWeightParams.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T+PupWeightParams.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
    Slope=(Wmin-PupWeightParams.Larvae_Absolute_minimum_weight_for_pupation)/(PupWeightParams.Pupation_Physiological_Development_Threshold_for_Wmin-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival);

    if (PbMsg.sim().simulation_mode1()==CIMSIM) {
        this->Pupation_Weight=Wmin+Slope*this->Physiological_Development_Percent; //ORIGINAL INCORRECT CIMSIM FUNCTION
    }
    if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) { 
        Wmin_25=PupWeightParams.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25+PupWeightParams.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
        Wmin_50=PupWeightParams.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50+PupWeightParams.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
        Wmin_75=PupWeightParams.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75+PupWeightParams.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;
        Wmin_100=PupWeightParams.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100+PupWeightParams.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T*Water_Average_Temp;

        Slope_25=(Wmin_25-PupWeightParams.Larvae_Absolute_minimum_weight_for_pupation)/(PupWeightParams.Pupation_Physiological_Development_Threshold_for_Wmin-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival);
        Slope_50=(Wmin_50-PupWeightParams.Larvae_Absolute_minimum_weight_for_pupation)/(PupWeightParams.Pupation_Physiological_Development_Threshold_for_Wmin-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival);
        Slope_75=(Wmin_75-PupWeightParams.Larvae_Absolute_minimum_weight_for_pupation)/(PupWeightParams.Pupation_Physiological_Development_Threshold_for_Wmin-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival);
        Slope_100=(Wmin_100-PupWeightParams.Larvae_Absolute_minimum_weight_for_pupation)/(PupWeightParams.Pupation_Physiological_Development_Threshold_for_Wmin-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival);

        this->Pupation_Weight = Slope*this->Physiological_Development_Percent+(PupWeightParams.tmpNumerator-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival*Wmin)/PupWeightParams.tmpDenom;
        this->Pupation_Weight_25=Slope_25*this->Physiological_Development_Percent+(PupWeightParams.tmpNumerator-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_25)/PupWeightParams.tmpDenom;
        this->Pupation_Weight_50=Slope_50*this->Physiological_Development_Percent+(PupWeightParams.tmpNumerator-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_50)/PupWeightParams.tmpDenom;
        this->Pupation_Weight_75=Slope_75*this->Physiological_Development_Percent+(PupWeightParams.tmpNumerator-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_75)/PupWeightParams.tmpDenom;
        this->Pupation_Weight_100=Slope_100*this->Physiological_Development_Percent+(PupWeightParams.tmpNumerator-PupWeightParams.Larvae_Maximal_physiological_developmental_time_for_survival*Wmin_100)/PupWeightParams.tmpDenom;

        // instars (Skeeter Buster mode only)
        // calculates weight thresholds to move from one instar to the next
        // hard coded values from Christophers 1960
        // MAGIC NUMBERS!!
        Wmin_II_min = (Wmin_50-PbMsg.femalelarvae().female_larvae_smallest_weight_for_survival())*0.0189255;  
        Wmin_III_min = (Wmin_50-PbMsg.femalelarvae().female_larvae_smallest_weight_for_survival())*0.0739927;
        Wmin_IV_min = (Wmin_50-PbMsg.femalelarvae().female_larvae_smallest_weight_for_survival())*0.4151404;

        Wmin_II_max = (PupWeightParams.tmpWeightDiff)*0.0189255;
        Wmin_III_max = (PupWeightParams.tmpWeightDiff)*0.0739927;
        Wmin_IV_max = (PupWeightParams.tmpWeightDiff)*0.4151404;

        Slope_II = (Wmin_II_min-Wmin_II_max)/(PupWeightParams.tmpSlope);
        Slope_III = (Wmin_III_min-Wmin_III_max)/(PupWeightParams.tmpSlope);
        Slope_IV = (Wmin_IV_min-Wmin_IV_max)/(PupWeightParams.tmpSlope);

        if (this->Physiological_Development_Percent<PbMsg.femalelarvae().skeeter_buster_female_larvae_physiological_developmental_percent_for_wmin())
        {
            this->Weight_II = Wmin_II_min;
            this->Weight_III = Wmin_III_min;
            this->Weight_IV = Wmin_IV_min;
        } else {
            double tmpPercent1 = this->Physiological_Development_Percent-PbMsg.femalelarvae().skeeter_buster_female_larvae_physiological_developmental_percent_for_wmin();
            this->Weight_II = Wmin_II_min + Slope_II*(tmpPercent1);
            this->Weight_III = Wmin_III_min + Slope_III*(tmpPercent1);
            this->Weight_IV = Wmin_IV_min + Slope_IV*(tmpPercent1);
        }
    }
    return 0;
}

				
//this procedure calculates the number of new pupae for larval cohorts //#12
std::pair<double,double> Larvae_Cohort::Larval_pupation(const outLogs & logs)
{
    if (this->Number==0) return std::pair<double,double>(0,0); //if the larval cohort is empty, end procedure

    double Pupation_Probability=0;
    double Larvae_survival_at_pupation_due_to_birth_defects;
    double NewlyPupated=0;
    double NewlyPupated_unadj=0;
    double WeightCadavers=0;

    if (this->Sex==FEMALE)
    {
      Larvae_survival_at_pupation_due_to_birth_defects=PbMsg.femalelarvae().female_larvae_survival_at_pupation_due_to_birth_defects();
    }
    else
    {
      Larvae_survival_at_pupation_due_to_birth_defects=PbMsg.malelarvae().male_larvae_survival_at_pupation_due_to_birth_defects();
    }

    //all larvae pupate if the average weight of the larval cohort on the previous day exceeds the pupation weight threshold and if the larval cohort is mature

    if (PbMsg.sim().simulation_mode1()==CIMSIM)
    {
        if ((this->Previous_Day_Larval_Weight>this->Pupation_Weight)&&(this->Mature))
        {
             Pupation_Probability=1;	
             if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) 
                fprintf(logs.Output,"%s %.8g\n","Minimum weight for pupation:",this->Pupation_Weight);
        }
    }

    if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)
    {
        /// ???
        if ((this->Average_Larval_Weight>this->Pupation_Weight_100)&&(this->Mature)) 
        {
            Pupation_Probability=1; 
            if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) 
                fprintf(logs.Output,"%s %.8g\n","Minimum weight for pupation(100):",this->Pupation_Weight_100);
        }
        if ((this->Average_Larval_Weight>this->Pupation_Weight_75)&&(this->Mature)&&(Pupation_Probability==0)) 
        {
            Pupation_Probability=0.75; 
            if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) 
                fprintf(logs.Output,"%s %.8g\n","Minimum weight for pupation(75):",this->Pupation_Weight_75);
        }
        if ((this->Average_Larval_Weight>this->Pupation_Weight_50)&&(this->Mature)&&(Pupation_Probability==0)) 
        {
            Pupation_Probability=0.5; 
            if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end()))
                fprintf(logs.Output,"%s %.8g\n","Minimum weight for pupation(50):",this->Pupation_Weight_50);
        }
        if ((this->Average_Larval_Weight>this->Pupation_Weight_25)&&(this->Mature)&&(Pupation_Probability==0)) 
        {
            Pupation_Probability=0.25; 
            if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) 
                fprintf(logs.Output,"%s %.8g\n","Minimum weight for pupation(25):",this->Pupation_Weight_25);
        }
    }

    if (NewlyPupated<0)
    {
        fprintf(stdout, "*** LARVAE COHORT HERE ***\n") ;
    }
    if (PbMsg.sim().nopupation()) 
        Pupation_Probability=0;
    //stochastic implementation of pupation for larval cohorts
    if (PbMsg.sim().simulation_mode2()==STOCHASTIC)
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
    if (PbMsg.sim().simulation_mode2()==DETERMINISTIC)
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
    return std::pair<double,double>(NewlyPupated,WeightCadavers); 
};//this is the end of the pupation procedure for larval cohorts
    
//this is the end of the definition of the objects for unisex larval cohorts
