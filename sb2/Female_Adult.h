//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include<vector>
 
class Female_Adult;
using lfem_t = list<Female_Adult>;

struct WeatherDay;

struct Oviposition
{
	size_t Date;
	size_t Age_since_hatch;
	size_t Number;
	size_t House_Position;
};

// Not a cohort!
class Female_Adult
{
 public:
//members
  // use for ID
  static std::size_t Next_ID;
  size_t ID;
  // csv order
  size_t Release_Date;
  double Physiological_Development_Percent;
  size_t Age;
  double Average_Adult_Weight;
  bool Mature;
  bool Nulliparous;
  int Genotype;
  int Wolbachia;
  bool Mated;
  int Male_Genotype;
  // non-csv
  double Fecundity;
  double Age_since_hatch;
  int Male_Wolbachia ;
  //?? why list
  int Maternal_ID;
  // used??
  size_t Emergence_Place;
  int Source_Container_Type;
  std::vector<Oviposition> Oviposition_Events;
//
//methods
  Female_Adult();
  // authoritative
  Female_Adult(size_t rel_date, double devel_percent, int age, double avgWt, bool mature, int nullipar, int genotype,   int wolbachia,  bool mated, int male_genotype, double fecundity, int age_since_hatch, int male_wolbachia, int maternal_id) ;
    // input from csv, routes to authoritative
  explicit Female_Adult( const vector<double> & spec);
  // spawn new during sim
  Female_Adult( double avgWt, int nullipar, double fecundity, int genotype, int wolbachia, int age_since_hatch, bool mated, int male_genotype, int male_wolbachia, int maternal_id) ; 
  int Female_Adult_Calculations(int Number_of_Suitable_Containers, const WeatherDay & thisWeather);
};

