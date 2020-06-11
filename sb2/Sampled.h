//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#include "Globals.h"
#include "Binomial.h"
#include "Receptacle.h"
#include "Building.h"
#include "YoungCohorts.h"
// templates: sample_list*
#include "helpers.h"
// IO
#include "fastcsv_type.h"
#include "minicsv.h"
#include <set>

// forward decl
template <size_t Nresult>
class SampledHouses;

using loc_code_t = std::string;
// passed to take_adult
using sample_adult_t = SampledHouses<3>;
// passed to take_cont
using sample_cont_t = SampledHouses<4>;
using sample_key_t  = size_t;

// try to use standard container typenames
// http://www.cplusplus.com/reference/map/multimap/
template <typename Tmapped_t>
class Sampled
{
public:
    // for derived classes
    using key_t = sample_key_t;
    using mapped_t = Tmapped_t;
    using value_t = std::pair<key_t, mapped_t>;
    // date, (house, container)
    using DayMap = std::multimap<key_t, mapped_t>;
protected:
    size_t nhouses;
    std::string specfile_path;
    DayMap specs;
    // date of last sample day
    key_t end_sample_date;
    // true if sampled on Date, up to last date of sampling
    vector<bool> sample_today;
private:
    // fail on out-of-bounds spec input?
    bool quiet;
    using DayMapIt = typename DayMap::iterator;
    // pair of iterators (for equal_range)
    using itpair_t = std::pair<DayMapIt, DayMapIt>;
public:
    Sampled(std::string _fn, bool _quiet=true) : specfile_path(_fn), quiet(_quiet) {};
    // 
protected:
    void init_sample_dates() {
        if (specs.size()==0) {
            throw std::runtime_error("In Sampled.h, trying to initialize sample or spray dates, but none available");
        }
        // value of last key
        end_sample_date = specs.rbegin()->first;
        // initialize bool vector
        sample_today = vector<bool>(end_sample_date+1, false); 
        // update bool vector from unique keys
        for( auto it = specs.begin(), end = specs.end(); it != end; it = specs.upper_bound(it->first)) {
            sample_today[it->first] = true;
        }
    }
    bool IsSampleDate(const key_t the_date) {
        // check if there's sampling today
        if (the_date > end_sample_date) {
            return false;
        }
        if (!sample_today[the_date]) {
            return false;
        }
        // else
        return true;
    }
    void erase(itpair_t itpair){
        // erase after processing
        if (itpair.first != itpair.second) {
            specs.erase(itpair.first, itpair.second);
        }
    }
    // bounds-check input
    bool validate(Sampled::key_t date, loc_code_t val) {
        // throw if missing
        Building::get_id(val);
        // 
        if (
            (date >= PbMsg.sim().number_of_days() )
        ){
            if (!quiet) {
                std::cout << "## Date: " << date << " House: " << val << std::endl;
                throw std::runtime_error("## In Sampled:  invalid specification.");
            }
            return false;
        }
        // all is well
        return true;
    }
};

class SampledContainers : public Sampled<std::pair<loc_code_t, size_t>>
{
private:
    // set of map values, places where larvicide added
    std::set<Sampled::mapped_t> seen;
public:
    using Sampled::Sampled; // inherit ctor
    void init() {
        // element of multimap
        // ( date, (house, cont) )
        nhouses = Building::houses.size();
        Sampled::value_t item;
        // 3 columns: size_t, string, size_t
        csv_reader<3> in_csv(specfile_path, "date", "location_code", "container");
        while( in_csv.read_row( item.first, item.second.first, item.second.second)) {
            // bounds-check date, house 
            // (can't validate container until sample time
            if (validate(item.first, item.second.first)) {
                specs.insert(item);
            }
        }
        init_sample_dates();
    }
    // bounds-check container access within house
    Receptacle & get_cont_ref(Sampled::mapped_t house_cont) {
        auto & the_house = Building::get_ref(house_cont.first);
        if (house_cont.second >= the_house.Container.size()) {
            throw std::runtime_error("In SampledContainers: specified container not found");
        }
        // start of containers
        auto itCont = the_house.Container.begin();
        // count n forwards 
        // (idiom works for both list and vec)
        std::advance(itCont, house_cont.second);
        // get the reference
        auto & the_ref = *itCont;
        return the_ref;
    }
    // add larvicide, update efficiacies
    void larv_update(Sampled::key_t today) {
        // check if there's anything to do today 
        if (!IsSampleDate(today)) return;
        // search for entries for today
        auto begin_end = specs.equal_range(today);
        // all records for today
        for (auto it = begin_end.first;  it !=  begin_end.second; it++){
            // add this house/cont combo 
            // to set of seen
            Sampled::mapped_t house_cont = (*it).second;
            // grab ref to container to update
            Receptacle & the_cont = get_cont_ref(house_cont);
            // if everything ok, mark as seen
            seen.insert(house_cont);
            // update the container
            the_cont.larvicide_present = true;
            the_cont.larvicide_efficacy = 1.0;
            the_cont.larvicide_date = today;
        }
        // remove entries for today
        erase(begin_end);
    }
    // kill immature in each container with larvicide
    void larv_apply(Sampled::key_t today) {
        for (auto ipair : seen) {
            Receptacle & the_cont = get_cont_ref(ipair);
            the_cont.ApplyLarvicide(today);
        }
    }
};


template <size_t Nresult>
class SampledHouses : public Sampled<loc_code_t>
{
private: 
    // members / typedefs
    using ptr_csv = std::shared_ptr<csv::ofstream>;
    ptr_csv outfile;
    // house index, sample
    using obs_t = array<size_t, Nresult>;
    vector<std::pair<mapped_t, obs_t>> result;
    friend void take_adult (
        sample_adult_t & sample,
        sample_key_t today,
        // probability of sampling
        double prob_female, double prob_male,
        bool observe, bool is_spray
    );
    friend void take_cont (
        sample_cont_t & sample,
        sample_key_t today,
        double prob_container,
        double prob_larva,
        double prob_pupa
    );
    // methods
public:
    using Sampled::Sampled; // inherit ctor
    template <typename T>
    void destructive_sample ( 
        Sampled::key_t today, double prob_cont, double prob_insect, T ptr_cohort
    );
    void init(ptr_csv _outfile) {
        // set from static 
        nhouses = Building::houses.size();
        outfile = _outfile;
        // element of multimap
        // ( date, (house, cont) )
        Sampled::value_t item;
        csv_reader<2> in_csv(specfile_path, "date", "location_code");
        while( in_csv.read_row( item.first, item.second)) {
            // optionally throw if problems
            if ( validate(item.first, item.second)) {
                specs.insert(item);
            }
        }
        // get dates from specs
        init_sample_dates();
    }
private:
    // write observed results to file after one day of sampling
    // loop through sampled houses
    void writeSamplingResults(size_t today) {
        for (auto & the_row : result) {
            // date index, house index, the sample
            (*outfile) << today << the_row.first;
            for (auto item : the_row.second) {
                 (*outfile) << item;
            }
            (*outfile) << NEWLINE;
        }
    }
};

// similar to destructive_sample_adult
// for larva and pupa
void take_cont (
    sample_cont_t & sample,
    sample_key_t today,
    double prob_container,
    double prob_larva,
    double prob_pupa
) {
    // check if there's anything to do today 
    if (!sample.IsSampleDate(today)) return;
    // results 
    // convenience
    auto & res = sample.result;
    // search for entries for today
    auto begin_end = sample.specs.equal_range(today);
    // initialize sample results
    size_t nresult = std::distance(begin_end.first, begin_end.second);
    res.resize(nresult);
    // index to house result
    size_t iresult(0);
    // all records for today - span of equal_range
    for (auto it = begin_end.first;  it !=  begin_end.second; it++){
        // totals for each house
        size_t ncont(0), npos(0), nlarva(0), npupa(0);
        // index of house from location_code
        auto & shouse = (*it).second;
        // the data
        auto & the_house = Building::get_ref(shouse);
        // sum results from house
        // for each container
        for (auto & the_cont : the_house.Container) {
            // sample this container?
            if (Random_Number() < prob_container) {
                ncont++;
                // sample *this container*
                size_t tmplarva = sample_list_cohort(the_cont.Larvae, prob_larva);
                size_t tmppupa = sample_list_cohort(the_cont.Pupae, prob_pupa);
                // this container is positive
                if (tmplarva || tmppupa) {
                    npos++;
                }
                // record house totals
                nlarva += tmplarva;
                npupa += tmppupa;
            }
        }
        // end of house, record
        res[iresult].first = shouse;
        // length 1 array
        res[iresult].second = {ncont, npos, nlarva, npupa};
        iresult++;
    }
    // remove entries for today
    sample.erase(begin_end);
    if (sample.outfile) {
        sample.writeSamplingResults(today);
    }
}

void take_adult (
    sample_adult_t & sample,
    sample_key_t today,
    // probability of sampling
    double prob_female,
    double prob_male,
    bool observe,
    bool is_spray = false
) {
    // check if there's anything to do today 
    if (!sample.IsSampleDate(today)) return;
    // search for entries for today
    auto begin_end = sample.specs.equal_range(today);
    // sample size
    size_t nresult = std::distance(begin_end.first, begin_end.second);
    // convenience
    auto & res = sample.result;
    // initialize sample results
    res.resize(nresult);
    // index to house result
    size_t iresult(0);
    // all records for today - span of equal_range
    for (auto it = begin_end.first;  it !=  begin_end.second; it++){
        // running total of sample
        size_t nf(0);
        size_t nnull(0);
        size_t nadult(0);
        // index of house
        auto & shouse = (*it).second;
        auto & the_house = Building::get_ref(shouse);
        if (is_spray) {
            // code from Tradition.cpp, 
            // apply_traditional_control()
            the_house.last_date_sprayed = today;
            the_house.killing_efficacy = prob_female;
        }
        // lists to be sampled / looped through
        auto & the_f = the_house.Female_Adults;
        // females are individuals
        // fill n*
        sample_list_f(the_f, prob_female, observe, nf, nnull);
        nadult = nf;
        // males are cohorts, sample within
        auto & the_m = the_house.Male_Adult;
        // return nmale (add to total)
        nadult += sample_list_cohort(the_m, prob_male, observe);
        // end of house, record
        res[iresult].first = shouse;
        res[iresult].second = {nadult, nf, nnull};
        iresult++;
    }
    // remove entries for today
    sample.erase(begin_end);
    if (sample.outfile) {
        // more specific
        sample.writeSamplingResults(today);
    }
}
