//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
// base class: male, youngcohorts
// not females

class Cohort
{
public:
    size_t Number;
    size_t Age;
    bool Mature;
    int Genotype;
    int Wolbachia;
    size_t Release_Date;
    //double Age_since_hatch;
    //double Average_Weight; ??each has own name...
    // genotype, release date not set by default??
    Cohort() : 
        Number(0), Age(0), Mature(false), Wolbachia(0)
    {
        // things that use this are terrible
        // uncomment the following line to see them
        // throw std::runtime_error("Cohort default constructor: deprecated");
    }
    Cohort( size_t number, size_t age, bool mature, int genotype, int wolbachia) 
        : Number(number), Age(age), Mature(mature), Genotype(genotype), Wolbachia(wolbachia)
    {}
    Cohort( size_t number, size_t age,  bool mature, int genotype, int wolbachia, size_t release_date) 
        : Number(number), Age(age), Mature(mature), Genotype(genotype), Wolbachia(wolbachia), Release_Date(release_date)
    {}
};

// convenience function
// add up totals of each cohort in vector
template <typename T>
size_t cohort_sum(list<T> & xx) {
    size_t total(0);
    for (auto & item : xx) {
        total += item.Number;
    }
    return total;
}

// getter function 
// use lambda at callpoint of cohort_sum_fun
template <typename T>
using cohort_get_double = double(*)(T&);

// convenience function
// add up totals of each cohort in vector
template <typename T>
double cohort_sum_fun(list<T> & xx, cohort_get_double<T> getter) {
    double total(0);
    for (auto & item : xx) {
        total += (*getter)(item);
    }
    return total;
}
