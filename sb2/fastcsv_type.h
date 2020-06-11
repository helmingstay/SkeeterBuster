//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#pragma once
#include "fastcsv.h"

// convenience policy definition
// trim spaces, comma sep, comments=#
template<size_t ncol>
using csv_t = io::CSVReader<ncol, io::trim_chars<' '>, io::no_quote_escape<','>, io::throw_on_overflow, io::single_line_comment<'#'>  >; 

// convenience class tamplate 
// include colnames & call to read_header in the ctor
template<size_t Ncol>
class csv_reader : public csv_t<Ncol> {
private:
    using parent_t = csv_t<Ncol>;
public:
    // construct and initialize,
    // emit error for colnames mismatch
    template<typename ...ColNames>
    csv_reader(const std::string & fn, ColNames ...cols) : 
        parent_t(fn)
    {
        parent_t::read_header(io::ignore_no_column, cols...);
    };
};

// http://stackoverflow.com/questions/16834851/passing-stdarray-as-arguments-of-template-variadic-function
// magic: generate integer sequence as function args
// pass to variadic template
namespace indices_t {
    template<int... Is>
    struct seq { };
    //
    template<int N, int... Is>
    struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };
    //
    template<int... Is>
    struct gen_seq<0, Is...> : seq<Is...> { };
}
