//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

size_t Binomial_Deviate(size_t n, double pp); 
void genmul(int,double *,int, unsigned int *);
double Normal_Deviate(double av, double sd);
double Random_Number();
size_t RandomIndexFlat(size_t size);
size_t RandomIndexWeight(vector<double> & weights);
