//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.

#pragma once

#include "Cohort.h"
#include "Globals.h"
#include "LarvalParamStructs.h"

struct WeatherDay;
struct outLogs;

struct Trio
{
	double NewlyEmerged;
	double WeightCadavers;
	double Maturation;
};

class Eggs_Cohort : public Cohort
{
public:
    // Members
    double Level_laid_in_Container; //in mm
    double Physiological_Development_Percent; 
    int Medea_Mom;
    int Maternal_ID;
    // ctors
    Eggs_Cohort() : 
        Cohort(), Level_laid_in_Container(0.0), Physiological_Development_Percent(0), Medea_Mom(0), Maternal_ID(-1)
    {
        // things that use this are terrible
        // uncomment the following line to see them
        // throw std::runtime_error("Egg default constructor: deprecated");
    }

    // release specification, see Release.h
    // Cohort ctor is ( num, age, mature, genotype, wolbachia, release_date) 
    explicit Eggs_Cohort( vector<double> spec) : 
        // num, age, mature, genotype, wolbach, date
        Cohort(spec[1], spec[4], spec[5], spec[6], spec[7], spec[0]),
        Level_laid_in_Container(spec[2]), 
        Physiological_Development_Percent(spec[3]), 
        Medea_Mom(0), 
        Maternal_ID(-1)
    {}
    // Method declarations
    double Egg_Calculations( const WeatherDay & thisWeather,  double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Depth, double Sun_Exposure,  double Water_Average_Temp);
};


class Larvae_Cohort : public Cohort
{
public:
    // static set from PbMsg,
    // partly convenience
    static vector<LarvalGrowthParams> vec_growth;
    static vector<LarvalPupationWeightParams> vec_pupation;
    static double tmpSlope;
    static double tmpWeightDiff;
    //members
    // top 3 in input csv
    double Average_Larval_Weight; // add to cohort?
    double Physiological_Development_Percent;
    // Sex depends on genotype, excess info
    const bool Sex;
    const LarvalGrowthParams & GrowthParams;
    const LarvalPupationWeightParams & PupWeightParams;
    double Age_since_hatch;
    int Instar;
    double Prefasting_Larval_Weight;
    double Prefasting_Lipid_Reserve;
    double Average_Lipid_Reserve;
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
    int Maternal_ID;
    //ctors
    Larvae_Cohort( ) = delete;

    // authoritative ctor - others dispatch here
    Larvae_Cohort(size_t rel_date, size_t number,  double avg_weight, double devel_percent, int age, bool mature, int genotype, int wolbach,  int maternal_id) :
        // age, mature, genotype, wolbach, date
        Cohort(number, age, mature, genotype, wolbach, rel_date),
        Average_Larval_Weight(avg_weight), 
        Physiological_Development_Percent(devel_percent),
        Sex(SexFromGenotype(Genotype)),
        GrowthParams(vec_growth[Sex]),
        PupWeightParams(vec_pupation[Sex]),
        Age_since_hatch(0), 
        Instar(1), 
        Prefasting_Larval_Weight(0.0), 
        Prefasting_Lipid_Reserve(0.0), 
        Average_Lipid_Reserve(0.0), 
        Previous_Day_Larval_Weight(0.0), 
        Pupation_Weight(0.0), 
        Previous_Day_Fasting_Survival(0.0),
        Larvae_Chronological_basis(0.0),
        Maternal_ID(maternal_id)
    {}
    // create new larvae from eggs
    Larvae_Cohort(int num, double avgWeight, int genotype, int wolbachia, int maternal_id) : 
        // 
        Larvae_Cohort( 0, num, avgWeight, 0.0, 0, 0, genotype, wolbachia,  maternal_id)
    {
        // ?? hard-coded, what does this even mean?
        Age_since_hatch = 1;
        // if (num == 0 ) throw std::runtime_error("## In Larvae_Cohort: Attempt to construct empty cohort."); 
    }
    // input from csv / Release
    // vector of length 8 with elements:
    // date, number, weight, phys, age, mature, genotype, wolbach
    explicit Larvae_Cohort( vector<double> spec) : 
        Larvae_Cohort(spec[0], spec[1], spec[2], spec[3], spec[4], spec[5], spec[6], spec[7], -1)
    {}
    //methods:
    double Larvae_Calculations(double Water_Average_Temp);
    double Larval_growth(double food_exploit, double Container_Volume) noexcept;
    double Larval_survival(double Water_Maximum, double Water_Minimum, double Water_Depth);
    double Larval_Pupation_Weight_Calculation(double Water_Average_Temp);
    std::pair<double,double> Larval_pupation(const outLogs & logs);
    // set static members, each vec contains M/F
    // store a lot of messy / ugly code in one place
    static void set_GrowthParams() {
        vec_growth.resize(2);
        {
            auto & _fg = vec_growth[FEMALE]; 
            auto & _mg = vec_growth[MALE];
            //FEMALE growth
            _fg.b=PbMsg.femalelarvae().female_larvae_increment_of_increase_of_rate_of_food_exploitation();
            _fg.c=PbMsg.femalelarvae().female_larvae_asymptotic_rate_of_food_exploitation_at_high_food_densities();
            _fg.d1=PbMsg.femalelarvae().female_larvae_rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae();
            _fg.d2=PbMsg.femalelarvae().female_larvae_exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae();
            _fg.a=PbMsg.femalelarvae().female_larvae_conversion_rate_of_consumed_food_to_biomass();
            //MALE growth
            _mg.b=PbMsg.malelarvae().male_larvae_increment_of_increase_of_rate_of_food_exploitation();
            _mg.c=PbMsg.malelarvae().male_larvae_asymptotic_rate_of_food_exploitation_at_high_food_densities();
            _mg.d1=PbMsg.malelarvae().male_larvae_rate_of_metabolic_weight_loss_as_a_function_of_dry_weight_of_larvae();
            _mg.d2=PbMsg.malelarvae().male_larvae_exponent_of_metabolic_requirements_of_larvae_as_a_function_of_dry_weight_of_larvae();
            _mg.a=PbMsg.malelarvae().male_larvae_conversion_rate_of_consumed_food_to_biomass();
        }
        // pupation
        vec_pupation.resize(2);
        {
            auto & _fp = vec_pupation[FEMALE]; 
            auto & _mp = vec_pupation[MALE];
            // convenience var
            size_t _model1 = PbMsg.sim().simulation_mode1();
            if (_model1==CIMSIM) {
                _fp.Larvae_Absolute_minimum_weight_for_pupation=PbMsg.femalelarvae().cimsim_female_larvae_absolute_minimum_weight_for_pupation();
                _fp.Larvae_Maximal_physiological_developmental_time_for_survival=PbMsg.femalelarvae().cimsim_female_larvae_maximal_physiological_developmental_time_for_survival();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.femalelarvae().cimsim_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _fp.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.femalelarvae().cimsim_female_larvae_slope_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _fp.Pupation_Physiological_Development_Threshold_for_Wmin=PbMsg.femalelarvae().cimsim_female_larvae_physiological_developmental_percent_for_wmin();
                // male
                _mp.Larvae_Absolute_minimum_weight_for_pupation=PbMsg.malelarvae().cimsim_male_larvae_absolute_minimum_weight_for_pupation();
                _mp.Larvae_Maximal_physiological_developmental_time_for_survival=PbMsg.malelarvae().cimsim_male_larvae_maximal_physiological_developmental_time_for_survival();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.malelarvae().cimsim_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _mp.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.malelarvae().cimsim_male_larvae_slope_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _mp.Pupation_Physiological_Development_Threshold_for_Wmin=PbMsg.malelarvae().cimsim_male_larvae_physiological_developmental_percent_for_wmin();
            } else if (_model1==SKEETER_BUSTER) {
                _fp.Larvae_Absolute_minimum_weight_for_pupation=PbMsg.femalelarvae().skeeter_buster_female_larvae_absolute_minimum_weight_for_pupation();
                _fp.Larvae_Maximal_physiological_developmental_time_for_survival=PbMsg.femalelarvae().skeeter_buster_female_larvae_maximal_physiological_developmental_time_for_survival();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.femalelarvae().skeeter_buster_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25=PbMsg.femalelarvae().skeeter_buster_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_25();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50=PbMsg.femalelarvae().skeeter_buster_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_50();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75=PbMsg.femalelarvae().skeeter_buster_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_75();
                _fp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100=PbMsg.femalelarvae().skeeter_buster_female_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_100();
                _fp.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.femalelarvae().skeeter_buster_female_larvae_slope_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _fp.Pupation_Physiological_Development_Threshold_for_Wmin=PbMsg.femalelarvae().skeeter_buster_female_larvae_physiological_developmental_percent_for_wmin();
                // MALE
                _mp.Larvae_Absolute_minimum_weight_for_pupation=PbMsg.malelarvae().skeeter_buster_male_larvae_absolute_minimum_weight_for_pupation();
                _mp.Larvae_Maximal_physiological_developmental_time_for_survival=PbMsg.malelarvae().skeeter_buster_male_larvae_maximal_physiological_developmental_time_for_survival();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.malelarvae().skeeter_buster_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25=PbMsg.malelarvae().skeeter_buster_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_25();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50=PbMsg.malelarvae().skeeter_buster_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_50();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75=PbMsg.malelarvae().skeeter_buster_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_75();
                _mp.Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100=PbMsg.malelarvae().skeeter_buster_male_larvae_intercept_of_minimal_larval_weight_for_pupation_as_function_of_t_100();
                _mp.Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T=PbMsg.malelarvae().skeeter_buster_male_larvae_slope_of_minimal_larval_weight_for_pupation_as_function_of_t();
                _mp.Pupation_Physiological_Development_Threshold_for_Wmin=PbMsg.malelarvae().skeeter_buster_male_larvae_physiological_developmental_percent_for_wmin();
            } else {
                throw std::runtime_error("simulation_model1 not recognized");
            }
            // depends on above
            _fp.tmpDenom = _fp.Pupation_Physiological_Development_Threshold_for_Wmin - _fp.Larvae_Maximal_physiological_developmental_time_for_survival;
            _mp.tmpDenom = _mp.Pupation_Physiological_Development_Threshold_for_Wmin - _mp.Larvae_Maximal_physiological_developmental_time_for_survival;
            //
            _fp.tmpNumerator = _fp.Pupation_Physiological_Development_Threshold_for_Wmin * _fp.Larvae_Absolute_minimum_weight_for_pupation;
            _mp.tmpNumerator = _mp.Pupation_Physiological_Development_Threshold_for_Wmin * _mp.Larvae_Absolute_minimum_weight_for_pupation;
        }
        // mainly convenience
        // same for male and female??
        tmpSlope = PbMsg.femalelarvae().skeeter_buster_female_larvae_physiological_developmental_percent_for_wmin()-PbMsg.femalelarvae().skeeter_buster_female_larvae_maximal_physiological_developmental_time_for_survival();
        tmpWeightDiff = PbMsg.femalelarvae().skeeter_buster_female_larvae_absolute_minimum_weight_for_pupation()-PbMsg.femalelarvae().female_larvae_smallest_weight_for_survival();
    };
};

// expensive, inline
//this procedure calculated the Gilpin-McClelland equations for larval cohorts	//#9
inline double Larvae_Cohort::Larval_growth(double food_exploit, double Container_Volume) noexcept
{
    if (this->Number==0) return 0; //if the larval cohort is empty, end procedure
    if (this->Age==0) return 0; //if the larval cohort contains new larvae, end procedure (in CIMSIM, weight of larvae does not increase on the day they are hatched)
    //
    //equation 6 of CIMSIM as the Gilpin-McClelland equation for weight increase of a larval cohort
    //
    double log2_weight = std::log2(this->Average_Larval_Weight);
    double tmp_chron = this->Larvae_Chronological_basis*2880.0;
    double tmp_steps = PbMsg.devel().larvae_number_of_euler_steps();
    //MAGIC 2800??
    // used for return value, below
    double weight_increase_base = tmp_chron * std::exp2(log2_weight*GrowthParams.b)*food_exploit/tmp_steps;
    // used to update
    double weight_increase = weight_increase_base - (tmp_chron * GrowthParams.d1 * std::exp2(log2_weight*GrowthParams.d2) / tmp_steps); 
    weight_increase *= GrowthParams.a;
    this->Average_Larval_Weight += weight_increase; //update the weight of the larval cohort
    //equation 7 of CIMSIM as the Gilpin-McClelland equation for food decrease
    // Larval_food_decrease
    return  Number*weight_increase_base;
    //
    // rewrite dF(t)/dt equation using Weight_Increase (fewer operations)
    //Larval_food_decrease = (1/Larvae_Conversion_rate_of_consumed_food_to_biomass)*this->Number*Weight_Increase;
};

class Pupae_Cohort : public Cohort
{
public:
    //members
    double Physiological_Development_Percent;
    double Age_since_hatch;
    double Average_Pupal_Weight;
    double Average_Pupal_Lipid_Reserve;
    bool Sex;
    int Maternal_ID;
    //ctors
    Pupae_Cohort( size_t num, double avgWeight, int genotype, int wolbachia, double age_since_hatch, int maternal_id) :
        // num, age, mature, genotype, wolbach, date
        Cohort(num, 0, false, genotype, wolbachia), 
        Physiological_Development_Percent(0.0),
        Age_since_hatch(age_since_hatch),
        Average_Pupal_Weight(avgWeight),
        Average_Pupal_Lipid_Reserve(0.0),
        Sex(SexFromGenotype(genotype)),
        Maternal_ID(maternal_id)
    {
        // if (num == 0 ) throw std::runtime_error("## In Pupae_Cohort: Attempt to construct empty cohort."); 
    };
    // input from csv / Release
    explicit Pupae_Cohort( vector<double> spec):
        // num, age, mature, genotype, wolbach, date
        Cohort(spec[1], spec[3], spec[4], spec[5], spec[6], spec[0]),
        Physiological_Development_Percent(spec[2]),
        Age_since_hatch(0),
        Average_Pupal_Weight(spec[7]),
        Average_Pupal_Lipid_Reserve(spec[8]),
        Sex(SexFromGenotype(Genotype)),
        Maternal_ID(-1)
    {
        // if (num == 0 ) throw std::runtime_error("## In Pupae_Cohort: Attempt to construct empty cohort."); 
    };
    Trio Pupae_Calculations(double Water_Maximum_Temp, double Water_Minimum_Temp, double Water_Average_Temp);
};
