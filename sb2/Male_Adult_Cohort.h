//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include "Cohort.h"

struct WeatherDay;
class Male_Adult_Cohort;
using lmale_t = list<Male_Adult_Cohort>;
    
class Male_Adult_Cohort : public Cohort
{
public:
    //members
    // use for ID
    static std::size_t Next_ID;
    size_t ID; // not used, perhaps helpful for debugging
    size_t Emergence_Place;
    size_t Age_since_hatch;
    double Physiological_Development_Percent;
    double Average_Adult_Weight;
    //
    // ctors
    // authoritative, all others point here
    Male_Adult_Cohort(size_t release_date, double num, double physdev, int age, double avgWt, bool mature, int genotype, int wolbachia,  int age_since_hatch); 
    // unused, basic init
    Male_Adult_Cohort();
    // spawn during sim
    Male_Adult_Cohort( int num, double avgWt, int genotype, int wolbachia, int age_since_hatch ) ;
    // release, csv
    explicit Male_Adult_Cohort( vector<double> spec);
    void Male_Adult_Calculations(const WeatherDay & thisWeather);
};
