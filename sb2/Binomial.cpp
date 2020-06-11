//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
//
//    Skeeter Buster is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    Skeeter Buster is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with Skeeter Buster.  If not, see <http://www.gnu.org/licenses/>.

//just for CheckProba macro
#include "Globals.h"
#include <cmath>
#include <cstdio>
#include "Binomial.h" 
void ErrorMessage (int);

extern gsl_rng * pRNG;

//These are definitions for the random number generator that is used for stochastic 
//calculations.
/******* RANDOM NUMBER GENERATOR BY ZIFF **********/
// originally gsl_rng_gfsr4, see Issue #13 for details
//
// GSL http://git.savannah.gnu.org/cgit/gsl.git/tree/rng/gfsr4.c?h=release-2-3&id=b47e97528030d957a05ae833773fd5b140862748
//NOTE: gsl_rng_gfsr4 diff max, RIMAX = 4294967295.0;

size_t ignbin(size_t n,double pp) {
   return gsl_ran_binomial(pRNG, pp, n);
}

size_t Binomial_Deviate(size_t n, double pp)
{
    if (!CheckProba(pp)){
        ErrorMessage(101);
    }
    //?? why cast to double?
    return ignbin( n, pp);
}

void genmul(int n,double *p,int ncat,unsigned int *ix) {
    gsl_ran_multinomial(pRNG, ncat, n, p, ix);
}


double Normal_Deviate(double av, double sd) {
    double draw = gsl_ran_gaussian(pRNG, sd);
    return (draw + av);
}

double Random_Number() {
   return gsl_ran_flat(pRNG, 0, 1);
}

size_t RandomIndexFlat(size_t size) {
    double draw = std::floor(gsl_ran_flat(pRNG, 0, size));
    size_t index = static_cast<int>(draw);
    return index;
}

// takes sorted vector of cumulative probabilities
// returns index
size_t RandomIndexWeight(vector<double> & weights) {
            auto idraw = std::lower_bound(weights.begin(), weights.end(), Random_Number());
            // get index of match
            size_t ii = std::distance(weights.begin(), idraw);
            return ii;
}
