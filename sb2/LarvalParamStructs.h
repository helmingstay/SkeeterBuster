//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once

struct LarvalGrowthParams {
    // conveninece parameter container for larval growth method
    //equation 6 of CIMSIM as the Gilpin-McClelland equation 
    //for weight increase of a larval cohort
    // params are constants differing by sex
    double a;
    double b;
    double c;
    double d1;
    double d2;
};

struct LarvalPupationWeightParams {
    double Larvae_Absolute_minimum_weight_for_pupation;
    double Larvae_Maximal_physiological_developmental_time_for_survival;
    double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T;
    double Larvae_Slope_of_minimal_larval_weight_for_pupation_as_function_of_T;
    double Pupation_Physiological_Development_Threshold_for_Wmin;
    // here down not used by CIMSIM
    double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_25;
    double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_50;
    double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_75;
    double Larvae_Intercept_of_minimal_larval_weight_for_pupation_as_function_of_T_100;
    double tmpDenom; 
    double tmpNumerator;
    double tmpSlope;
    double tmpWeightDiff; 
};

