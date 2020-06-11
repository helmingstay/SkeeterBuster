//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once
#include <vector>

class Building;
using std::vector;

// DEPRECATED (old, grid-based)
void Larvicide_grid (int, int, int, vector<Building> & House);
void Adulticide_grid (int, int, int, int Date, vector<Building> & House);
void Source_Removal_grid (int, int, int, vector<Building> & House);
void apply_traditional_control(int, vector<int> &Sprayed_Houses_Vector, vector<Building> & House);
void apply_residual_control(int Date, vector<Building> & House);
// TODO: Deprecate:
void apply_traditional_control(int, vector<Building> & House);
