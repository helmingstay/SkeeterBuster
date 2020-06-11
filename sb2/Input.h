//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#pragma once
#include <vector>
#include "fastcsv_type.h"
#include "Link.h"
#include "Block.h"
#include "Building.h"

struct ReleaseInsect;

// class returned by factory function
// each call to read_row
// fills a string and a vector of doubles
//
// used for containers and insects, 
// helps with large number of columns
template <size_t Ncol>
class InputCsv {
private:
    constexpr static unsigned char Ndat = Ncol-1;
    csv_reader<Ncol> in_csv;
    using datvec_t = vector<double>;
public:
    // return appropriate sized vector to be filled
    datvec_t make_vector() {return datvec_t(Ncol);};
    // ctor 
    template <class ...ColNames>
    InputCsv( const std::string & infile, ColNames ...cols) : in_csv(infile, cols...) {}
    // end of ctor
    // fill refs with a row of csv
    bool read_row(string & _loc, vector<double> & _dat ) {
        return _read_row(_loc, _dat, indices_t::gen_seq<Ndat>());
    }
    // helper - pass indices to vector<double> as param pack
    template<int... Is>
    bool _read_row(string & _loc, vector<double> & _dat, indices_t::seq<Is...>) {
        // fill with text, parse to double
        vector<char *> row_text(Ndat);
        // fill - string id, everything else as double
        bool ok = in_csv.read_row(_loc, row_text[Is]...);
        // manually parse string to double
        for (size_t ii(0); ii < Ndat ; ii++) {
            _dat[ii] = std::stod(row_text[ii]); 
        }
        return ok;
    }
};

// factory function,
// to handle auto return
// fails... why?
/*
template <class ...ColNames>
InputCsv<sizeof...(ColNames)> make_input_csv(
    const string & _fn, ColNames & ...cols
) {
    return InputCsv<sizeof...(ColNames)>(_fn, cols...);
}
*/

void ReadHouseSetup( 
    std::vector<Building> & House, const std::string & file_house, const std::string & file_cont, ReleaseInsect & release
);

// csv readers, return multimap for processing
hrows_t ReadLinkHouse(std::string path);
brows_t ReadLinkBlock(std::string path);

// read neighbor network from db into houses
void InDispersalHouse(
    std::string path, 
    vbuild_t & houses
);

// 
bmap_t InDispersalBlock(
    std::string path,
    vbuild_t & houses
);

void CheckParameters();
