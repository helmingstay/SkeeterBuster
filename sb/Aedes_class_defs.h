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

#include<vector>
#include<utility>
#include<list>
#include<fstream>



using namespace std;

extern int number_of_genotypes;

struct Trio
{
	double NewlyEmerged;
	double WeightCadavers;
	double Maturation;
};

struct Oviposition
{
	int Date;
	double Age_since_hatch;
	double Number;
	int House_Position;
};



class Eggs_Cohort
{
 public:

 // Members

  double Number;
  double Level_laid_in_Container; //in mm
  double Physiological_Development_Percent; 
  int Age;
  bool Mature;
  int Genotype;
  int Wolbachia;
  int Medea_Mom;
  int Release_Date;

  // Methods
   Eggs_Cohort();
  Eggs_Cohort( double num, double level, double dev, int age, bool mature, int genotype, int wolbachia, int medea_mom );
  double Egg_Calculations( double, double, double, double, double, double, double, double, double);
};


class Larvae_Cohort
{
 public:
 
//members

  bool Sex;
  double Number;
  double Physiological_Development_Percent;
  int Age;
  double Age_since_hatch;
  int Instar;
  bool Mature;
  int Release_Date;
  double Average_Larval_Weight;
  double Prefasting_Larval_Weight;
  double Prefasting_Lipid_Reserve;
  double Average_Lipid_Reserve;
  double Previous_Larval_Weight;
  double Previous_Day_Larval_Weight;
  double Pupation_Weight;
  double Pupation_Weight_25;
  double Pupation_Weight_50;
  double Pupation_Weight_75;
  double Pupation_Weight_100;
  double Weight_II;
  double Weight_III;
  double Weight_IV;
  double Previous_Day_Fasting_Survival;
  double Larvae_Chronological_basis;
  int Genotype;
  int Wolbachia;
  
//methods:
  Larvae_Cohort( );
  Larvae_Cohort( bool sex, double num, double avgWeight, int age, double age_since_hatch, int instar, int genotype, int wolbachia);
  double Larvae_Calculations(double, double, double);
  double Larval_growth(double, double);
  double Larval_survival(double, double, double);
  double Larval_Pupation_Weight_Calculation(double);
  pair<double,double> Larval_pupation(void);
};


class Pupae_Cohort
{
 public:
 
//members

  double Number;
  double Physiological_Development_Percent;
  int Age;
  double Age_since_hatch;
  double Average_Pupal_Weight;
  double Average_Pupal_Lipid_Reserve;
  bool Mature;
  bool Sex;
  int Genotype;
  int Wolbachia;
  int Release_Date;

//methods
  Pupae_Cohort();
  Pupae_Cohort( bool sex, double num, double avgWeight, int genotype, int wolbachia, double age_since_hatch);
  Trio Pupae_Calculations(double, double, double, double, double);
};

class Female_Adult
{
 public:
  
//members

  int ID;
  int Emergence_Place;
  int Source_Container_Type;
  double Physiological_Development_Percent;
  int Age;
  double Age_since_hatch;
  double Average_Adult_Weight;
  double Fecundity;
  bool Mature;
  bool Nulliparous;
  int Genotype;
  int Wolbachia;
  int Release_Date;
  bool Mated;
  int Male_Genotype ;
  int Male_Wolbachia ;
  list<Oviposition> Oviposition_Events;
  int Number_Released_Along_With; // This only applies to when females are actively released to prevent having to store millions of female individuals at the start of the model run.

//methods
  Female_Adult();
  Female_Adult( bool nulliOrParous, double avgWt, double fecundity, int genotype, int wolbachia, double Physiological_Development_Percent, int Age, double age_since_hatch, bool Mature, int Release_Date, bool mated, int male_genotype, int male_wolbachia );
  double Female_Adult_Calculations(double, double, double, double, int);

};

class Male_Adult_Cohort
{
 public:

//members

  double Number;
  int Age;
  double Age_since_hatch;
  double Physiological_Development_Percent;
  bool Mature;
  double Average_Adult_Weight;
  int Genotype;
  int Wolbachia;
  int Release_Date;

//methods
 Male_Adult_Cohort();
  Male_Adult_Cohort( double num, double avgWt, int genotype, int wolbachia, double physdev, int age, double age_since_hatch, bool Mature, int Release_Date);
   void Male_Adult_Calculations(double, double, double, double);
			
};


class Receptacle
{
 public:

//members
  
  int ID;

  int Container_Type_Number;
  int House_Indicator;
  int Release_Date;
  
  double Height;
  double Surface;
  bool Covered;
  double Monthly_Turnover_Rate;
  double SunExposure;
  double Daily_Food_Loss;
  double Daily_Food_Gain;
  double Initial_Food_Present;
  double Cover_Reduction;
  int Filling_Method;
  double Draw_Down;
  double Length_of_Moving_Average;
  double Watershed_Ratio;
  double Maximum_Number_of_Egg_Levels;

  double Maximum_Daily_Water_Temperature;
  double Minimum_Daily_Water_Temperature;
  double Average_Daily_Water_Temperature;
  double Evaporative_Loss;
  double Water_Level; //this is mm
  double Food; //mg
  double Weight_Cadavers_From_Yesterday; //mg
  double Total_NewlyHatched;
  double Total_NewlyPupated[2];
  double Total_NewPupaeWeight[2];
  list<Eggs_Cohort> Eggs;
  list<Larvae_Cohort> Larvae;
  list<Pupae_Cohort> Pupae;

  int Number_of_Release_Egg_Cohorts;
  int Number_of_Release_Larval_Cohorts;
  int Number_of_Release_Pupal_Cohorts;

  list<Eggs_Cohort> ReleaseEggs;
  list<Larvae_Cohort> ReleaseLarvae;
  list<Pupae_Cohort> ReleasePupae;

//methods
 Receptacle();
  void  WeatherCalculations(double, double, double, double, double, double, double, double, double, double);
  void FoodCalculations(double,double,double);
  
  void ReceptacleCalculations(int,double,double,double,double,double,double,double,double,class Building *);
};


class Building
{	
 public:

//members

  int Position;

  vector<double> Maximum_Daily_Air_Temperature;
  vector<double> Minimum_Daily_Air_Temperature;
  vector<double> Average_Daily_Air_Temperature;
  vector<double> Precipitation; //this is mm
  vector<double> Relative_Humidity;
  vector<double> Saturation_Deficit;
  int Number_of_Containers;
  double Attractiveness;
  double TotalEggs;
  double TotalLarvae[2][4];
  double TotalFemalePupae;
  double TotalMalePupae;
  double TotalFemaleNulliparousAdults;
  double TotalFemaleParousAdults;
  double TotalMaleAdults;
  double TotalNewlyParous;
  double TotalOvipositing;
  double Total_NewAdultNumber[2];
  double Total_NewAdultWeight[2];
 double Daily_Average_Female_Adult_Weight;
  double Daily_Average_Female_Fecundity;
  double Average_New_Female_Adult_Weight[5];
  bool Recapture;
  double Recaptured_Female,Recaptured_Male;
  list<Female_Adult> Female_Adults;
  list<Male_Adult_Cohort> Male_Adult;
  vector<Receptacle> Container; 
  vector<Receptacle> Reserve_Containers;
  list<int> ContainerReleaseDates;
  int neighbour_position[4];
  double neighbour_cumul_attractiveness[4];

  int Number_of_Release_Female_Adult_Cohorts;
  int Number_of_Release_Male_Adult_Cohorts;
  list<Female_Adult> Release_Female_Adult;
  list<Male_Adult_Cohort> Release_Male_Adult;

  list<Female_Adult> Migrating_Female_Adult;
  list<Male_Adult_Cohort> Migrating_Male_Adult;

//methods
  //Building();
  void SetBuildingCalculationParameters(int, double, double, int);
  void BuildingCalculations();
  void BuildingCalculations(int, double, double, int);
  void BuildingSummation();
};


class SampledHouses
{
 public:

//members
  int Number_of_Days_Sampled;
  // dates houses sampled
  vector<int> dates_houses_sampled;
  vector<int> number_of_houses_sampled_on_Date;
  vector<int> *indices_of_houses_sampled_on_Date; // An array of vectors to store houses sampled - elements are accessed as houses_sampled_on_date[day][0]

  // sampling results
  vector<int> *Number_of_Eggs_Sampled_on_Date; // An array of vectors to store eggs sampled
  vector<int> *Number_of_Larvae_Sampled_on_Date; // An array of vectors to store eggs sampled
  vector<int> *Number_of_Pupae_Sampled_on_Date;
  vector<int> *Number_of_Adults_Sampled_on_Date;

  // Output files
  std::ofstream larvae_sampling_file;
  std::ofstream pupae_sampling_file;
  std::ofstream adult_sampling_file;

//methods
  SampledHouses(int number_of_dates_sampled);
  void ReadSamplingFile();
  int CheckSamplingDate(int day);
  void SetupSamplingResults(int output_number);
  void writeSamplingResults(int date_of_sampling);
  void closeSamplingFiles();
  ~SampledHouses();
};


double FoodGain_Calculation(int,double,double);
//int Binomial_Deviate(int n, double pp);
double Binomial_Deviate(double n,double pp);
void genmul(int,double *,int, int *);
void Multinomial_Deviate(double n, double *p, int ncat, double *ix);
double Normal_Deviate(double av, double sd);
double Random_Number();
int gen_poisson (double mu);
double gen_gamma(const unsigned int a);
double gamma_large(const double a);

void Write_HTML(double) ;
void Write_GNUplot (void) ;
int Get_Number(void) ;
void Get_Date(void) ;

void ReadConfigFile();
void ReadHouseSetup();
void WeatherInput();
void CheckParameters();
void ErrorMessage (int);

void CreateDirectories();
void OpenLogFiles();

double Fitness_Calculation(int);
double Fitness_Calculation_Medea(int, int) ;
double Fitness_Calculation_RR(int);
double Fitness_Calculation_Environmental_Effect(int);

double SpatialStats(void) ;

void Larvicide (int, int, int);
void Adulticide (int, int, int);
void Source_Removal (int, int, int);

void Destructively_Sample_Larvae (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House);
void Destructively_Sample_Adults (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House);
void Destructively_Sample_Pupae (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House);

void EPS_Header(int, int) ;
void EPS_Trailer(void) ;
void EPS_Wolbachia_Header(int, int) ;
void EPS_Wolbachia_Trailer(void) ;
void EPS_Density_Header(int, int) ;
void EPS_Density_Trailer(void) ;
void EPS_XT_Header(int, int) ;
void EPS_XT_Trailer(void) ;

void PrintOutput(double);
void SaveHouseSetupOut(void);
void CompressOutputs(void);
