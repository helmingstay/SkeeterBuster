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
#include "Output.h"
#include "Receptacle.h"
#include "Building.h"
#include "Binomial.h"
#include "helpers.h"
#include "Weather.h"
#include "Release.h"
#include "Cohort.h"


// lambda for reporting each Larvae
auto reportLarva = [](const outLogs & logs, const Larvae_Cohort & larv, double cont_food) {
    fprintf(logs.Output,"%s %d %s %u.l %s %zu %s %zu %s %f %s %u.l %s %.8g %s %d %s %.8g %s %.8g %s %.8g %s %.8g %s %.8d\n","larva cohort #",-999," Sex ", larv.Sex,", Number: ", larv.Number," Age: ", larv.Age," Age since hatch: ", larv.Age_since_hatch," Instar : ", larv.Instar," Phys. Dev.: ", larv.Physiological_Development_Percent," Mature: ", larv.Mature," Ave. Weight: ", larv.Average_Larval_Weight," Ave. Lipid Reserve: ", larv.Average_Lipid_Reserve," Prefasting Weight: ", larv.Prefasting_Larval_Weight," Larval_food: ", cont_food, " Genotype: ", larv.Genotype);
};
// and pupa
auto reportPupa = [](const outLogs & logs, const Pupae_Cohort & pup) {
    fprintf(logs.Output,"%s %d %s %i %s %zu %s %zu %s %.3g %s %.8g %s %d %s %.8g %s %.8g %s %d\n","Pupae cohort #",-999," sex ", pup.Sex," Number: ", pup.Number," Age: ", pup.Age," Age since hatch: ", pup.Age_since_hatch," Phys. Dev.: ", pup.Physiological_Development_Percent," Mature: ", pup.Mature," Ave. Weight: ", pup.Average_Pupal_Weight," Ave. Lipid Reserve: ", pup.Average_Pupal_Lipid_Reserve," Genotype: ", pup.Genotype);
};


//#3
//this procedure calculates the weather-driven variables of the container on a given day
//?? why not use member variables of receptacle?
//e.g. SunExposure (member) == Sun_Exposure (param)
//pass Date, access members in method??
void Receptacle::WeatherCalculations(
    const Weather & vecWeather, size_t Date
) {
    // water temp helper functions (lambdas)
    // used by moving average and regular
    //!MAGIC NUMBERS
    //??What in gods name are all these constants??
    auto max_temp_fn = [](const WeatherDay & day, double sun ) {
	    return 15.033426+0.266995*day.minTempC+0.007053*std::pow(day.maxTempC,2)+7.685039*std::pow(sun,2);
    };
    // as above, min
	auto min_temp_fn = [](const WeatherDay & day, double sun ) {
        return 5.019385-1.363636*sun+0.807682*day.minTempC+0.000982*std::pow(day.maxTempC,2);
    };

	//the characteristics of the particular container are set based on its container type
	//the water temperature and depth is calculated for the container
	// moving average window
	double MoveAve=1;
    //!MAGIC NUMBERS
	if ((Surface*Height)>5000) MoveAve=2;
	if ((Surface*Height)>100000) MoveAve=3;
	if ((Surface*Height)>500000) MoveAve=4;
    
    // today's weather
    const auto & thisWeather = vecWeather[Date];

    // no moving average
    // ?? do these conditions make sense?
	if ((MoveAve==1)||(((Date-MoveAve+1)<0)&&(PbMsg.sim().number_of_days()<365))){
        Maximum_Daily_Water_Temperature = max_temp_fn(thisWeather, SunExposure);
        Minimum_Daily_Water_Temperature = min_temp_fn(thisWeather, SunExposure);
        //
        Average_Daily_Water_Temperature = (Maximum_Daily_Water_Temperature+Minimum_Daily_Water_Temperature)/2;
    } else {
        // moving average
        double Moving_Average_Max_Temp=0;
        double Moving_Average_Min_Temp=0;
		//if ((Date-MoveAve+1)<0)
			// for earlier time steps values for days -1, -2, ... are taken one year later
			//  (.e.g. add 365
			// provided simulation runs at least one year (otherwise, no moving average is calculated)
		int today(Date);
        for (int ii=today-MoveAve+1;ii<today+1;ii++) {
            // i is counter, j is temporary index
            int jj = ii ;
            // what about leap years??
            if (ii < 0 ) jj += 365;
            const auto & tmpWeather = vecWeather[jj];
            Moving_Average_Max_Temp+=max_temp_fn(tmpWeather, SunExposure);
            Moving_Average_Min_Temp+=min_temp_fn(tmpWeather, SunExposure);
        }
        Maximum_Daily_Water_Temperature=Moving_Average_Max_Temp/MoveAve;
        Minimum_Daily_Water_Temperature=Moving_Average_Min_Temp/MoveAve;
        Average_Daily_Water_Temperature=(Maximum_Daily_Water_Temperature+Minimum_Daily_Water_Temperature)/2;
    }  
    // water 
    double mm_to_cm = 0.1;
	Evaporative_Loss = 0.925636+0.275191*SunExposure-0.009041*thisWeather.relHum;
	Water_Level -= Evaporative_Loss*(1-Cover_Reduction);
	//this->Water_Level=this->Water_Level-(Draw_Down*1000)/Sur_face;
	//??min, *=
	Water_Level = std::max(Water_Level*(1.0-Draw_Down), 0.0);
	// to artificially add rainfall
	//this->Water_Level=this->Water_Level+max(1.,thisWeather.Rain)*0.1*Watershed_Ratio;
	

	// containers with method other than 2 and 3 get filled with rain. If a container is has filling method 3, it gets filled manually at frequency FILLING_FREQUENCY_TYPE_3_CONTAINERS
	 if ((Filling_Method!=2)&&(Filling_Method!=3)) {
         if (!PbMsg.sim().is_iquitos()) /* If we are not basing it on the Iquitos dataset */
         {
             Water_Level += thisWeather.precipMm*mm_to_cm*Watershed_Ratio;
         }
	 } else {
	 // if of type 2 or 3
		if (Filling_Method==3) /* Only manually filled containers of type 3 ever get filled */
		{
            if (Random_Number()<(1./PbMsg.container().filling_frequency_type_3_containers())) {
                Water_Level=Height;
            }
		}
	  }

	 if (PbMsg.sim().is_iquitos()) /* If we are basing it on the Iquitos dataset, even manually filled containers get rainwater; this filling has already been done for type 3 containers (which are absent in Iquitos anyway) so their filling is not repeated */
     {
         if (Filling_Method!=3) {
             Water_Level += thisWeather.precipMm*mm_to_cm*Watershed_Ratio;
         }
     }
     Water_Level = std::min(Water_Level, Height);
	//this->Average_Daily_Water_Temperature=FixedWaterAverageTemperature; //to make constant temperature
	/*this->Water_Level=10;
	this->Average_Daily_Water_Temperature=26;
	this->Maximum_Daily_Water_Temperature=26;
	this->Minimum_Daily_Water_Temperature=26;*/
}

void Receptacle::ReceptacleCalculations_Init(size_t Date, const outLogs & logs, const Weather & vecWeather, bool doDetailedLogToday)
{
	//double WeightCadaversFromYesterday;
	//double Sun_Exposure,Watershed_Ratio,Cover_Reduction,Draw_Down;
	//double Water_Average_Temp,Water_Minimum_Temp,Water_Maximum_Temp;
	//double Water_Depth;
	double FoodGain,FoodFromCadavers;
	//double Larvae_Conversion_rate_of_cadavers_to_food=PbMsg.femalelarvae().female_larvae_conversion_rate_of_cadavers_to_food(); // !!!! CHECK THIS !!!
    // Grab today's weather
    const WeatherDay & thisWeather = vecWeather[Date];

	// in case of a manually filled container, determine if it is going to be cleaned at the end of the day
	Emptied=false ;
	if (Filling_Method==2) {
        if (PbMsg.sim().is_iquitos()) /* carve out exceptions for Container_Type_Number 6 and 8 that appear in Iquitos */
        {
          if ((Container_Type_Number!=6)&&(Container_Type_Number!=8))
          {
             if (Random_Number()<PbMsg.container().probability_of_emptying_manually_filled_containers())
                 Emptied=true;
          }
        }
        else {
          if (Random_Number()<PbMsg.container().probability_of_emptying_manually_filled_containers())
              Emptied = true;
        }
    }
	//#3

    // water temperature, moving average
    WeatherCalculations(vecWeather, Date);


	//#4
	//the following routine mimics the procedures in CIMSIM
	//fixed: adds food into the container every 3 days (3*daily_amount) or 1 day (1*daily_amount)
	//random: adds food every day (amount between 0 and 2*daily)


			//!MAGIC NUMBERS
	if (PbMsg.sim().food_input_method()==3)
	{
		if (!(Date%3))	
			FoodGain=3*(Daily_Food_Gain);
		else FoodGain=0;
	}
	else if (PbMsg.sim().food_input_method()==1) FoodGain=Daily_Food_Gain;
	else FoodGain=2*Daily_Food_Gain*Random_Number();

	if (Date==Release_Date) FoodGain += Initial_Food_Present;


    //#4
    //calculate the amount of food in the container in the given day
    //Larvae_Conversion_rate_of_cadavers_to_food was copied from female
	FoodFromCadavers = Weight_Cadavers_From_Yesterday*PbMsg.femalelarvae().female_larvae_conversion_rate_of_cadavers_to_food();
	Food = std::max(Food*(1.0-Daily_Food_Loss), 0.0);
	Food += FoodGain+FoodFromCadavers;
    // reset
	Weight_Cadavers_From_Yesterday=0;

    //@@ oldchunk 020 - was commented out
	//delete[] new_larval_cohort_number;

    Egg_Calc(logs, thisWeather, doDetailedLogToday);
    //this is where the larval calculations start
    Larva_Calc_Init(logs);
    // call Larva_Calc_Determ() separately
}
// call Larva_Calc_Determ() between these;

// survival, reporting
void Receptacle::ReceptacleCalculations_Fin(const outLogs & logs, bool doDetailedLogToday) {
    Larva_Calc_Fin(logs);
    // pupal devel
    Pupa_Calc();

    if (doDetailedLogToday) {
        fprintf(logs.Output,"%s %zu","Container ",ID);
        //printout of the water container weather-driven variables into the detailed log file
	    fprintf(logs.Output," %s %.8g %s %.8g %s %.8g %s %.8g\n"," Water_depth: ",Water_Level," Water_Average_Temp: ",Average_Daily_Water_Temperature, " Water_Max_Temp: ",Maximum_Daily_Water_Temperature," Water_Min_Temp: ",Minimum_Daily_Water_Temperature);
	//log the properties of the larval/pupal cohorts into the detailed log
	    for (const auto & the_larva : Larvae) {
            reportLarva(logs, the_larva, Food);
        }
	    for (const auto & the_pupa : Pupae) {
            reportPupa(logs, the_pupa);
        }
    }
	// cleaning manually filled containers
	if ((Filling_Method==2) && (Emptied)) {
        Eggs.clear();
        Larvae.clear();
        Pupae.clear();
        Water_Level=Height;
        Food=Initial_Food_Present ;
    }

    // cleanup - use new cohort_sum
    // Sum everything up,
    // just grabs .Number
	double Total_Eggs_in_Container = cohort_sum(Eggs);
	double Total_Larvae_in_Container = cohort_sum(Larvae);
    double Total_Pupae_in_Container = cohort_sum(Pupae);

	if (doDetailedLogToday) {
        fprintf(logs.Output,"%s %.8g %s %.8g %s %.8g\n","Total eggs in container:",Total_Eggs_in_Container,"Total larvae in container:",Total_Larvae_in_Container,"Total pupae in container:",Total_Pupae_in_Container);

         fprintf(logs.Output,"%s\n","---------------------------------------------------------------------------------");
    }

    //@@ oldchunk 002

};	

// helper function in ReceptacleCalculations
//#5+#6
//the following procedure calculates the amount of newly hatched eggs, and generally handles egg cohorts
void Receptacle::Egg_Calc(const outLogs & logs, const WeatherDay & thisWeather, bool doDetailedLogToday){

	Total_NewlyHatched=0; //sum of all eggs hatched on the day in that container

	//////////
	// Old method was to create an array with all possible larval cohort types (by genotype and wolbachia)
	// so that egg cohorts with similar types could be merged into a single larval cohort
	// Inconvenient: very time consuming to create that array for every container every day.
	//
	// New method: 1 egg cohort = 1 larval cohort
	// Inconvenient: more larval cohorts
	// Improves speed when the number of chromosomes is high,


    //?? is this old or new method?
    //
	//double ***new_larval_cohort_number = new double** [number_of_genotypes] ;
	//for (int g=0 ; g<number_of_genotypes ; g++)
	//{
	// new_larval_cohort_number[g] = new double*[number_of_wolbachia_status];
	// for (int w=0 ; w<number_of_wolbachia_status ; w++)
	// {
	//	 new_larval_cohort_number[g][w] = new double[2];
	//	 new_larval_cohort_number[g][w][0] = 0.;
	//	 new_larval_cohort_number[g][w][1] = 0.;
	// }
	//}
	// 2 columns, [0] is for number, [1] is for age;


	for (auto eggIt=Eggs.begin(); eggIt != Eggs.end() ; ) //start of egg cohort loop
		// eggIt will be increased at the end of the loop
	{
		double NewlyHatched = eggIt->Egg_Calculations( thisWeather, Maximum_Daily_Water_Temperature, Minimum_Daily_Water_Temperature, Water_Level, SunExposure, Average_Daily_Water_Temperature);
		//#7
		//placing the new larvae into the larval cohorts

		if (NewlyHatched>0) {
			//new_larval_cohort_number[eggIt->Genotype][eggIt->Wolbachia][0]+=NewlyHatched;
			//new_larval_cohort_number[eggIt->Genotype][eggIt->Wolbachia][1]+=NewlyHatched*eggIt->Age;

            // use Larvae_Cohort ctor
            // to construct in-place
			Larvae.emplace_back(
                NewlyHatched, PbMsg.femalelarvae().skeeter_buster_female_larvae_weight_at_hatch(), eggIt->Genotype, eggIt->Wolbachia, eggIt->Maternal_ID 
            );
		}

		//this procedure makes sure that if all eggs hatch from an egg cohort, then it gets cleared
		if (NewlyHatched >= eggIt->Number) // !!!!!!!!!!!! Can this ever be > ???? [rounding error?]
		{
			eggIt = Eggs.erase(eggIt); // this leaves eggs_cohort_it pointing to the next cohort, so don't increment eggs_cohort_it
		} else {
			eggIt->Number -= NewlyHatched; //this adjusts the number of eggs in the egg cohort by the number of hatching eggs

			if (doDetailedLogToday) fprintf(logs.Output,"%s %d %s %zu %s %zu %s %.8g %s %d %s %.8g %s %.8g %s %d\n","Egg cohort #",-999," Number: ",eggIt->Number," Age: ",eggIt->Age," Phys. Dev.: ",eggIt->Physiological_Development_Percent," Mature: ",eggIt->Mature," Food in container: ", Food," Level: ",eggIt->Level_laid_in_Container," Genotype: ",eggIt->Genotype);

			eggIt++; // remember, only increment if we have a non-empty cohort
		}
	} //end of egg cohort loop
};

// helper function in ReceptacleCalculations
// number newly matured
void Receptacle::Larva_Calc_Init(const outLogs & logs){

	//calculation of physiological developmental percent increase and maturation for larval cohorts
	//#8
	for (auto & the_larva : Larvae) {
		if (PbMsg.sim().simulation_mode1()==CIMSIM) {
            the_larva.Larval_Pupation_Weight_Calculation(Average_Daily_Water_Temperature); //#8 //ORIGINAL CIMSIM
        }
		double Maturation=the_larva.Larvae_Calculations(Average_Daily_Water_Temperature); //#8
		//Maturation=0; //this switches the variation in the larval maturation on and off
		double Matured_Larvae_Number;
		if ((PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)&&(!the_larva.Mature)&&(Maturation>0))
		{
			if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) {
                Matured_Larvae_Number=the_larva.Number*Maturation;
            } else if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
                Matured_Larvae_Number=Binomial_Deviate(the_larva.Number,Maturation);
            } else {
                throw std::runtime_error("simulation_model2 not recognized");
            }
            if (Matured_Larvae_Number == 0) {
                // don't construct empty cohorts
                continue;
            }
            // copy current cohort
			Larvae_Cohort new_mature_cohort = the_larva;
            // mark as matured, update numbers
			new_mature_cohort.Mature=1;
			the_larva.Number-=Matured_Larvae_Number;
            new_mature_cohort.Number = Matured_Larvae_Number;
            //??why push_front?
            // MOVE!
			Larvae.push_front(std::move(new_mature_cohort));
		}
	}
}

// inline void Receptacle::Larva_Calc_Determ() 

void Receptacle::Larva_Calc_Fin(const outLogs & logs){
	//calculating survival for larval cohorts //#10

	for (auto & the_larva : Larvae) {
		Weight_Cadavers_From_Yesterday += the_larva.Larval_survival(Maximum_Daily_Water_Temperature,Minimum_Daily_Water_Temperature,Water_Level);
	}
	if (Weight_Cadavers_From_Yesterday<0) {
        fprintf(stdout, "*** HERE ***\n") ;
    }

	// larval growth and pupation

	//calculating the amount of new pupae, based on the CIMSIM model, with average pupal weight //#12
	//WeightCadaversFromYesterday=0;

	// ############################## THE WAY WE DO THIS: HAVE AN ARRAY OF SIZE 2, and use .Sex as an index for this array (or .Sex==FEMALE)

    //reset 
    Total_NewlyPupated.assign(2, 0.0); //total number of new pupae // Need to track male, female separately.
    Total_NewPupaeWeight.assign(2, 0.0); //total weight of new pupae

	for (auto larvae_cohort_it=Larvae.begin(); larvae_cohort_it != Larvae.end() ; )   // larvae_cohort_it++ done at end
	{
		//OUR CHANGE no. 2. larval pupation and instar transition weight calculation using the physiological developmental percent at the end of the day instead of the beginning of the day
		if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) {
            larvae_cohort_it->Larval_Pupation_Weight_Calculation(Average_Daily_Water_Temperature); //#11
        }

		// calculate instar
		// note the 'else' structure that prevent larvae from moving more than one instar up each day
		// note also that larvae cannot move back instars

		if (larvae_cohort_it->Instar==0) {
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_IV)
				larvae_cohort_it->Instar=4;
			else if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_III)
				larvae_cohort_it->Instar=3;
			else if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_II)
				larvae_cohort_it->Instar=2;
			else larvae_cohort_it->Instar=1;
		}

		else if (larvae_cohort_it->Instar<1.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_II)
				larvae_cohort_it->Instar=2;
		}
		else if (larvae_cohort_it->Instar<2.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_III)
				larvae_cohort_it->Instar=3;
		}
		else if (larvae_cohort_it->Instar<3.5)
		{
			if (larvae_cohort_it->Average_Larval_Weight > larvae_cohort_it->Weight_IV)
				larvae_cohort_it->Instar=4;
		}

		// check for pupating larvae
		// pupation returns a pair
		auto return_value = larvae_cohort_it->Larval_pupation(logs);

		double NewlyPupated=return_value.first;
		Weight_Cadavers_From_Yesterday+=return_value.second; //weight of cadavers updated for next day
		if (Weight_Cadavers_From_Yesterday<0) {
            fprintf(stdout, "*** HERE ***\n") ;
        }
		int sex=larvae_cohort_it->Sex; // figure out whether cohort is male or female
		Total_NewlyPupated[sex] += NewlyPupated; //increasing total number of new female pupae
		Total_NewPupaeWeight[sex] += larvae_cohort_it->Previous_Day_Larval_Weight*NewlyPupated; //increasing total weight of new female pupae

		//OUR CHANGE #1, pupal weight is the average of the larval weight at the beginning and at the end of the day when the larvae pupate
		//Pupae_Cohort new_pupae_cohort( sex, NewlyPupated,(larvae_cohort_it->Previous_Day_Larval_Weight+larvae_cohort_it->Average_Larval_Weight)/2 );


		//CIMSIM ORIGINAL, Pupal weight is larval weight at the beginning of the day when the larvae pupate
		//this is to put pupae that come from individual larval cohorts in separate pupal cohorts //COMMON CHANGE
		if ((PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)&&(NewlyPupated>0))
		{
			double newpupae_weight;
			if (!sex) {
                newpupae_weight=(larvae_cohort_it->Average_Larval_Weight+larvae_cohort_it->Previous_Day_Larval_Weight)/2;
            } else {
                newpupae_weight=larvae_cohort_it->Previous_Day_Larval_Weight;
            }
            // construct in-place
			Pupae.emplace_back( //Pupae_Cohort( 
                NewlyPupated, newpupae_weight,
                larvae_cohort_it->Genotype, larvae_cohort_it->Wolbachia, larvae_cohort_it->Age_since_hatch, larvae_cohort_it->Maternal_ID
                //)
            );
			//fprintf(larvtime, "%d %.3g %d %.8g\n", Date, NewlyPupated, larvae_cohort_it->Age, larvae_cohort_it->Average_Larval_Weight);
		}

		if (larvae_cohort_it->Number==0) {
			larvae_cohort_it = Larvae.erase(larvae_cohort_it);
            continue; // erase advances
		} 
        larvae_cohort_it++;
	}

	// put the newly pupated males and females (if there are any) into new pupal cohorts //#13

	//this is needed if all larvae that pupate on the same day in the same container all go into the
	//same pupal cohort with an average weight //ORIGINAL CIMSIM 
	// k == genotype (sex)
	if (PbMsg.sim().simulation_mode1()==CIMSIM) {
		for (int k=0;k<2;k++) {
			if (Total_NewlyPupated[k]>0)
			{
				// age_since_hatch not available here, average age of pupae put into the new cohort would have to be calculated
				// construct in-place
				Pupae.emplace_back( // Pupae_Cohort (
                    Total_NewlyPupated[k], Total_NewPupaeWeight[k]/Total_NewlyPupated[k], k, 0, 0, -1
                    //)
                );
			}
		}
	}
	//this is the end of the larval calculations
}

void Receptacle::Pupa_Calc() {
	//this is the start of the pupal cohort calculations //#14,#15,#16
	//WeightCadaversFromYesterday=0;

	for ( auto pupae_cohort_it = Pupae.begin(); pupae_cohort_it != Pupae.end() ;  ) // pupae_cohort_it++ done at end of loop
	{
		Trio pupae_return_value=pupae_cohort_it->Pupae_Calculations(Maximum_Daily_Water_Temperature, Minimum_Daily_Water_Temperature,Average_Daily_Water_Temperature);

		double Maturation=pupae_return_value.Maturation;
        double Matured_Pupae_Number;
		//Maturation=0; //this switches on and off the pupal maturation variation
		if ((PbMsg.sim().simulation_mode1()==SKEETER_BUSTER)&&(!pupae_cohort_it->Mature)&&(Maturation>0))
		{
			if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) {
                Matured_Pupae_Number=pupae_cohort_it->Number*Maturation;
            } else if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
                Matured_Pupae_Number=Binomial_Deviate(pupae_cohort_it->Number,Maturation);
            } else {
                throw std::runtime_error("simulation_model2 not recognized");
            }
			if (Matured_Pupae_Number>0) {
                // copy and update
                Pupae_Cohort new_mature_cohort = *pupae_cohort_it;
				new_mature_cohort.Mature=1;
                new_mature_cohort.Number = Matured_Pupae_Number;
				pupae_cohort_it->Number-=Matured_Pupae_Number;
                // MOVE!
				Pupae.push_front(std::move(new_mature_cohort));
			}
		}

		double NewlyEmerged = pupae_return_value.NewlyEmerged;
		Weight_Cadavers_From_Yesterday += pupae_return_value.WeightCadavers; //update the weight of cadavers for the next day
		if (Weight_Cadavers_From_Yesterday<0) {
            fprintf(stdout, "*** HERE ***\n") ;
        }
		int sex=pupae_cohort_it->Sex;

		house_ptr->Total_NewAdultNumber[sex] += NewlyEmerged; //increase the number of new female adults by those that emerge from the female pupal cohort on that day
		house_ptr->Total_NewAdultWeight[sex] += pupae_cohort_it->Average_Pupal_Weight*NewlyEmerged;	//increase the total weight of new female adult that emerge from the female pupal cohort on that day

		//COMMON CHANGES no. 4 //this part creates a separate adult female/male cohort for any pupal cohort that emerges instead of merging and averaging them //#17,#18
		if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER && NewlyEmerged >0)
		{
            //!Male == 1??
			if (sex) {
                // construct in-place
				house_ptr->Male_Adult.emplace_back(
                    NewlyEmerged,pupae_cohort_it->Average_Pupal_Weight*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult(),pupae_cohort_it->Genotype, pupae_cohort_it->Wolbachia, pupae_cohort_it->Age_since_hatch
                );
				//tmp_totaladults+=NewlyEmerged;
			} else {
				//for (int f=0 ; f<NewlyEmerged ; f++,tmp_totaladults++)
				for (int f=0 ; f<NewlyEmerged ; f++)
				{
					Female_Adult new_female_adult(pupae_cohort_it->Average_Pupal_Weight*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult(),NULLIPAROUS, pupae_cohort_it->Average_Pupal_Weight*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult()*PbMsg.devel().female_adult_daily_fecundity_factor_for_wet_weight(),pupae_cohort_it->Genotype, pupae_cohort_it->Wolbachia,pupae_cohort_it->Age_since_hatch,false,0,0, pupae_cohort_it->Maternal_ID);
					new_female_adult.Emergence_Place = house_ptr->unique_id;
					new_female_adult.Source_Container_Type = Container_Type_Number;
                    // MOVE!
					house_ptr->Female_Adults.push_back(std::move(new_female_adult));
				}
            }
		}

		if (pupae_cohort_it->Number==0) {
			pupae_cohort_it=Pupae.erase(pupae_cohort_it);
            continue; // erase advances
		} 
        pupae_cohort_it++;
	}
	//this is the end of the pupal cohort calculations
}

void Receptacle::ApplyLarvicide(size_t today) {
    // kill eggs, larva, pupa
    // use helper sample_list_cohort to remove from list
    // Update the larvicide efficacy
    if (larvicide_date != today) {
        // ?? exponential?
        larvicide_efficacy *= PbMsg.larvicide().daily_larvicide_decay_rate();
    }
    size_t kill_egg = sample_list_cohort(Eggs, (larvicide_efficacy)*(PbMsg.larvicide().larvicide_efficacy_on_eggs()));
    //
    size_t kill_larva = sample_list_cohort(Larvae, (larvicide_efficacy)*(PbMsg.larvicide().larvicide_efficacy_on_larvae()));
    //
    size_t kill_pupa = sample_list_cohort(Pupae, (larvicide_efficacy)*(PbMsg.larvicide().larvicide_efficacy_on_pupae()));
    size_t kill_total = kill_egg + kill_larva + kill_pupa;
    // unused at present
    (void) kill_total;
    //return ntotal;
}

Receptacle::Receptacle(
    Building * _phouse,
    size_t _unique_id, const vector<double> & dat,
    const ReleaseInsect & release
) : 
    house_ptr(_phouse),
    unique_id(_unique_id), 
    // to be deprecated, 
    // only for reporting
    ID(0), 
    // row of container input csv
    // see Input.cpp: in_cont for colnames
    Container_Type_Number(dat[1]), 
    Cover_Reduction(dat[2]), 
    Covered(dat[3]), Daily_Food_Gain(dat[4]), 
    Daily_Food_Loss(dat[5]), Draw_Down(dat[6]), 
    Filling_Method(dat[7]), Height(dat[8]), 
    Initial_Food_Present(dat[9]), 
    Water_Level(dat[10]), Monthly_Turnover_Rate(dat[11]), 
    SunExposure(dat[12]), Surface(dat[13]), 
    Watershed_Ratio(dat[14]), 
    Release_Date(dat[15]), id_egg(dat[16]),
    id_larva(dat[17]),
    id_pupa(dat[18]),
	larvicide_present(false)
{
    // add container release to house
    house_ptr->ContainerReleaseDates.push_back(Release_Date);
    // use id to find rows in release.x,
    // and add to ReleaseX
    // in Release.h
    SetupRelease(id_egg, release.egg, ReleaseEgg);
    SetupRelease(id_larva, release.larva, ReleaseLarva);
    SetupRelease(id_pupa, release.pupa, ReleasePupa);
    UpdateReleaseDone();
    // other init
	Maximum_Daily_Water_Temperature = 0.0;
	Minimum_Daily_Water_Temperature = 0.0;
	Average_Daily_Water_Temperature = 0.0;
	Evaporative_Loss = 0.0;
    Emptied = false;
	Food = 0.0; //mg
	Weight_Cadavers_From_Yesterday = 0.0; //mg
	Total_NewlyHatched = 0.0;
	Total_NewlyPupated = vector<double>(2);
	Total_NewPupaeWeight = vector<double>(2);
};
