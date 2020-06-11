//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include "YoungCohorts.h"
#include <vector>
#include <list>
#include "Release.h"
#include "Building.h"

using std::vector;
using std::list;

// forward decls
class Weather;
struct WeatherDay;
struct outLogs;
//class Building;

class Receptacle
{
 public:
//members
  Building * house_ptr;
  // this block is a row of csv file
  size_t unique_id;
  string location_code;
  size_t ID; // ID within houses; this is the "original" container ID variable
  int Container_Type_Number;
  double Cover_Reduction;
  bool Covered;
  // food in mg?
  double Daily_Food_Gain;
  double Daily_Food_Loss;
  double Draw_Down;
  int Filling_Method;
  double Height; // cm?
  // same as initial_larval_food??
  double Initial_Food_Present; // mg?
  double Water_Level; //cm
  double Monthly_Turnover_Rate; // unused?
  double SunExposure; // proportion
  double Surface; // cm^2?
  double Watershed_Ratio;
  size_t Release_Date;
  // release ids
  size_t id_egg;
  size_t id_larva;
  size_t id_pupa;
  // end csv row

  double Length_of_Moving_Average;
  double Maximum_Number_of_Egg_Levels;

  double Maximum_Daily_Water_Temperature;
  double Minimum_Daily_Water_Temperature;
  double Average_Daily_Water_Temperature;
  double Evaporative_Loss;
  bool Emptied;
  double Food; //mg
  double Weight_Cadavers_From_Yesterday; //mg
  double Total_NewlyHatched;
  // M/F
  vector<double> Total_NewlyPupated;
  vector<double> Total_NewPupaeWeight;
  list<Eggs_Cohort> Eggs;
  list<Larvae_Cohort> Larvae;
  list<Pupae_Cohort> Pupae;

  list<Eggs_Cohort> ReleaseEgg;
  list<Larvae_Cohort> ReleaseLarva;
  list<Pupae_Cohort> ReleasePupa;
  // mark as done once release lists are empty
  bool ReleaseDone;

// NB: It might make more sense to have the larvicide_plan class be independent of the containers, and have an observer class (I think it is called?) which is in charge of matching Larvicide_Regime with Receptacle.

  bool larvicide_present;
  size_t larvicide_date;
  double larvicide_efficacy;
  // ctors
  Receptacle() = default;
  // ctor used ReadHouseSetup()
  // unique_id is counter,
  // dat is row from csv
  Receptacle(
    Building * phouse,
    size_t _unique_id, const vector<double> & dat,
    const ReleaseInsect & release
  ); 
  //methods
  void ApplyLarvicide(size_t today);
  void WeatherCalculations(const Weather & vecWeather, size_t Date);
  void Egg_Calc(const outLogs & logs, const WeatherDay & thisWeather, bool doDetailedLog);
// 
  void Larva_Calc_Init(const outLogs & logs);
  // deterministic, very expensive
  void Larva_Calc_Determ() noexcept;
  void Larva_Calc_Fin(const outLogs & logs);
  void Pupa_Calc();
  
  void ReceptacleCalculations_Init(size_t Date, const outLogs & logs, const Weather & vecWeather, bool doDetailedLogToday);
  void ReceptacleCalculations_Fin(const outLogs & logs, bool doDetailedLogToday);
    // call after SetupRelease, or after release
    void UpdateReleaseDone() {
        ReleaseDone = (ReleaseEgg.empty() && ReleaseLarva.empty() && ReleasePupa.empty()); 
    }
    bool is_positive() noexcept;
};

// deterministic, expensive, inline
inline void Receptacle::Larva_Calc_Determ() noexcept {
	//Euler integration of the Gilpin-McClelland equations
	//#9
    //
	for (int k=0; k<PbMsg.devel().larvae_number_of_euler_steps(); k++)  // !!! DO WE ASSUME SAME NUMBER OF STEPS FOR BOTH????
	{
		//one set of the Gilpin and McClelland equations calculated for all larval cohorts
        // growth based on initial food
		double init_food(Food);
		vector<double> food_exploit(2);
        food_exploit[MALE] = (1.0-exp(-Larvae_Cohort::vec_growth[MALE].c*init_food));
        food_exploit[FEMALE] = (1.0-exp(-Larvae_Cohort::vec_growth[FEMALE].c*init_food));
        //adjustment available food in container based on total consumption
        //
		for (auto & the_larva : Larvae) {
			Food -= the_larva.Larval_growth(food_exploit[the_larva.Sex], Surface*Height); //actual calculation of the Gilpin-McClelland equations
		}
        Food = std::max(Food, 0.0);
	}
	//end of the Euler integration cycle
}


// does this container have any immatures?
inline bool Receptacle::is_positive() noexcept {
    if ( 
        (Eggs.size() > 0 ) 
        || (Larvae.size() > 0 ) 
        || (Pupae.size() > 0 ) 
    ) {
        return(true);
    } else {
        return(false);
    }
}
