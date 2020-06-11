//    Copyright 2015-2020 Christian Gunning (code@x14n.org)
//
//    This file is part of Skeeter Buster.
#pragma once
#include "Binomial.h"

// assume values are sorted and unique
// return iterator to first matching, or end if not found
// move to helper namespace?
template <typename T, typename Titer = typename T::iterator>
Titer find_sorted(T & the_obj, typename T::value_type the_cond) {
    std::pair<Titer, Titer> test = std::equal_range(the_obj.begin(), the_obj.end(), the_cond);
    if (test.first == test.second ) {
        // not found
        return(the_obj.end());
    } else {
        return(test.first);
    }
}


// helper function
// move elements from the_list to the_dest
// if Date matches today
template <typename Tlist, typename Tdest>
void splice_end(
    Tdest & the_dest, Tlist & the_list,  typename Tlist::iterator & it
) {
    the_dest.splice(the_dest.end(), the_list, it);
}

// ful list form
template <typename Tlist, typename Tdest>
void splice_end(Tdest & the_dest, Tlist & the_list
) {
    the_dest.splice(the_dest.end(), the_list);
}

// helper function
// move elements from the_list to the_dest
// if Date matches today
template <typename Tlist, typename Tdest>
void list_move_to( size_t today, 
    Tlist & the_list, Tdest & the_dest
) {
    auto it = the_list.begin();
    while (it != the_list.end()) {
        if (it->Release_Date!=today) {
            it++;
            continue;
        }
        // get next item
        auto tmp_it = std::next(it);
        splice_end(the_dest, the_list, it);
        // advance 
        it = tmp_it;
    }
}

template <typename T>
size_t sample_list_cohort(T & the_list, double the_prob, bool observe = false) {
    // manually increment iterator at end
    size_t ntotal(0);
    // set through
    for (auto it = the_list.begin(); it != the_list.end(); ) {
        // draw sample
        size_t nsample = Binomial_Deviate(it->Number, the_prob);
        if (!observe) {
            it->Number -= nsample;
        }
        ntotal += nsample;
        // delete if empty, fetch next
        if (it->Number == 0) {
            it = the_list.erase(it);
        } else {
            it++;
        }
    }
    return ntotal;
}

// for females (indiv, not cohorts)
// fill nfem, nnull
template <typename T>
void sample_list_f(T & the_list, double the_prob, bool observe, size_t & nfem, size_t & nnull) {
    // manually increment iterator at end
    // set through
    for (auto it = the_list.begin(); it != the_list.end(); ) {
        // draw sample
        if (Random_Number() < the_prob) {
            nfem++;
            nnull += it->Nulliparous;
            // erase individual, get next
            if (!observe) {
                it = the_list.erase(it);
            }
        } else {
            it++;
        }
    }
}

// zero out vector
template <typename T>
void fill0(T & vec) {
    std::fill(vec.begin(), vec.end(), 0);
}

