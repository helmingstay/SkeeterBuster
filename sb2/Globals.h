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

#pragma once

//??
/* Note: #ifdef must come after this line for MS VISUAL 
		     ** This means that this file must be included even for
             ** g++  . BUT the standard file contains a file that
		     ** messes up g++, so you have to delete the line that
		     ** includes tchar.h
		     */

#ifdef _MSC_VER
#error POSIX build environment is required
//#define ROUND_ System::Math::Round
//#define _MAIN_ _tmain
//#define FLOOR_	System::Math::Floor
//#define GETCWD _getcwd
// Deprecated - Microsoft build env
// #include <direct.h>
//#using <mscorlib.dll>
//using namespace System;
// #include <tchar.h>            // I had to remove this from stdafx.

#else

#define ROUND_ round
#define _MAIN_ main
#define FLOOR_ floor

#endif

// common headers
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <algorithm>
#include <map>
#include <vector>
#include <list>
#include <string>
// #include <time.h>
#include <cmath>
#include <numeric>
#include <utility>

using std::vector;
using std::list;
using std::cout;
using std::size_t;
//using std::pair;
//using std::map;

// custom protocol buffers messages
#include "PbInput.pb.h"

// defined in Aedes.cpp
// Global message of messages - sim params
// Note, field access is all lowercase: PbMsg.submsg().field()
extern PbInput::PbBuild PbMsg;

//extern long ra[M+1], nd;
extern size_t Date;

// lots of defines, should go in a namespace...
#define CheckProba(p) (!((p<0.)||(p>1.))) // check that a given value is acceptable as a probability

#define MALE true
#define FEMALE false

#define N_SEX 2
#define N_INSTAR 4

// use for "undefined" index
#define MAX_SIZE_T std::numeric_limits<std::size_t>::max()

inline bool SexFromGenotype(int genotype) {
    return genotype % 2;
}

#define NULLIPAROUS true
#define PAROUS false

#define CIMSIM false
#define SKEETER_BUSTER true

#define DETERMINISTIC false
#define STOCHASTIC true

#define CALCULATION false
#define SAMPLING true

#define FOCKSDEVEL false
#define DEGREEDAYDEVEL true

#define CUSTOM_DESTINATION false
#define DEFAULT_DESTINATION true

#define NEUTRAL 0
#define ENGINEERED_UNDERDOMINANCE 1
#define WOLBACHIA 2
#define MEIOTIC_DRIVE 3
#define MEDEA 4
#define FEMALE_KILLING 5
#define FKR 6
#define SELECTION 7
#define ReduceReplace 8
#define Male_Killing 9

#define SOFT 0
#define HARD 1

#define STICKY_BORDER 0
#define BOUNCE 1
#define TORE 2
#define RANDOM_EDGES 3
