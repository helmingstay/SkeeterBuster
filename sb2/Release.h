//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include <string>
#include <array>
#include <algorithm>
#include "YoungCohorts.h"
// IO
#include "fastcsv_type.h"


using std::string;

// like csv_reader, but more specific
// handle read_row internally for whole file
// read doubles into int-keyed multimap
// key = id, matches with release_id 
// (e.g. rel_egg in container)
template<size_t Ncol>
class ReleaseReader {
private:
    constexpr static unsigned char Ndat = Ncol-1;
    using reader_t = csv_reader<Ncol>;
public:
    ReleaseReader() = default;
    template<class ...Args>
    ReleaseReader(const string & filepath, Args...cols) {
        // names / # of columns
        reader_t in_csv(filepath, cols...);
        // read a single row, process to doubles, assign to map
        // first col is key,
        // remaining cols parsed to double
        _read_row(in_csv, indices_t::gen_seq<Ndat>());
    }
    // the key data
    std::multimap<size_t, vector<double> > release;
private:
    // magic template expands to indices 
    // used to pass refs to string vec to read_row
    // fill row_text, parse to doubles, add to record
    template<int... Is>
    void _read_row(reader_t & in_csv, indices_t::seq<Is...>) {
        // first col is key, parse to int
        std::string _key;
        // rest of row, read to string, parse to double
        vector<string> row_text(Ndat);
        vector<double> row_data(Ndat);
        while( 
            in_csv.read_row( _key, row_text[Is]...)
        ) {
            // parse each field to double
            for (size_t ii(0); ii < Ndat ; ii++) {
                row_data[ii] = std::stod(row_text[ii]); 
            }
            size_t key = std::stoi(_key);
            if (key <1 ) {
                // 0 means no release
                throw std::runtime_error("In Release.h, id should be > 0");
            }
            // store row by key
            // repeats allowed
            release.emplace(key, row_data);
        }
    }
};

// pass ref to struct
struct ReleaseInsect{
    ReleaseReader<9> egg;
    ReleaseReader<9> larva;
    ReleaseReader<10> pupa;
    ReleaseReader<9> adult_m;
    ReleaseReader<12> adult_f;
    ReleaseInsect() = delete;
    // column names for release id files
    ReleaseInsect(
        const string & fn_egg, 
        const string & fn_larva, 
        const string & fn_pupa, 
        const string & fn_adult_m, 
        const string & fn_adult_f
    ) :
        egg(fn_egg, "id_egg","date","number","level_laid","phys_devel_percent","age","mature","genotype","wolbachia"),
        larva(fn_larva, "id_larva","date","number","avg_weight","phys_devel_percent","age","mature","genotype","wolbachia"),
        pupa(fn_pupa, "id_pupa","date","number","phys_devel_percent","age","mature","genotype","wolbachia", "avg_weight", "avg_lipid_reserve"),
        adult_m(fn_adult_m, "id_adult_m","date","number","phys_devel_percent","age","average_weight","mature","genotype","wolbachia"),
        adult_f(fn_adult_f, "id_adult_f","date","number","phys_devel_percent","age","average_weight","mature","nulliparous","genotype","wolbachia","mated","male_genotype")
    {};
};


// take a release specification
// emplace into container or house (dest)
// for females (multiple = true), 
// emplace repeatedly 
// (use index_number to pull number from spec vector)
template <typename Trel, typename Tdest>
void SetupRelease(const size_t id, const Trel & rel_obj, Tdest & dest, bool multiple = false, size_t index_number = 1) {
    // first check if there's anything to do
    // 0 means no release
    if (id == 0) { return; }
    // otherwise, fill in all records
    auto _range = rel_obj.release.equal_range(id);
    if ( _range.first == _range.second) {
        cout << "## Release id not found: " << id << "\n";
        throw std::runtime_error("In Release.h: SetupRelease()");
    }
    // iterator
    for (auto relIt = _range.first; relIt != _range.second; relIt++) {
        // use custom ctor 
        // takes vector of doubles: the_release
        // females - one object per number: 
        if (multiple) {
            auto num = relIt->second[index_number];
            while (num-- > 0) {
                dest.emplace_back(relIt->second);
            }
        } else {
            // number in cohort
            dest.emplace_back(relIt->second);
        }
    }
};
