//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#pragma once
#include <map>
#include "Binomial.h"

template<class T>
using mmap_t = std::multimap< T, std::pair<T, double> >;


template<class T> class Link {
    public:
        typedef typename mmap_t<T>::iterator mmap_it;
        size_t nlink;
        vector<T> id;
        vector<double> weight;
        Link(){
            // zero-initialize
            nlink = 0;
        }
        Link(mmap_it it_first, const mmap_it it_last){
            nlink = std::distance(it_first, it_last);
            id.resize(nlink);
            weight.resize(nlink);
            size_t irec(0);    
            // total weight, use to normalize
            double cumsum(0.0);
            for ( auto it = it_first; it != it_last; it++, irec++) {
                // 
                // id of link
                id[irec] = it->second.first;
                // weight
                cumsum += it->second.second;
                weight[irec] = cumsum;
            }
            // normalize by total
            for ( auto & norm : weight) {
                norm /= cumsum;
            }
        }
        T GetRandomNeighbor() {
            // find matching destination based on random draw 
            size_t ii = RandomIndexWeight(weight);
            T draw = id[ii];
            return draw;
        }
    // end public
// end Link
};
