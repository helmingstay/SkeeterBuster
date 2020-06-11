//    Copyright 2006-2020 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould, Kenichi Okamoto, Christian Gunning
//
//    This file is part of Skeeter Buster.
#pragma once 

#include <vector>
#include <list>
#include <cmath>
#include "YoungCohorts.h"
#include "Globals.h"
#include "Link_type.h"
#include "Block.h"
// totalstruct
using std::vector;
using std::array;
using std::list;

class Building;

// forward decls
// includes in Building.cpp
#include "Female_Adult.h" 
#include "Male_Adult_Cohort.h" 
#include "YoungCohorts.h" 
#include "Output.h"
class Weather;
class Receptacle;
struct ReleaseInsect;
struct outLogs;
// convenience type
using vbuild_t = vector<Building>;

// runtime invariants used by Building member functions
// was globals
class Mask {	
public:
    Mask() {};
    Mask(
        size_t _nchrom, 
        bool do_medea, size_t medea_units,
        size_t n_incompat_wolbachia
    ) : 
        mask(_nchrom), divisor(_nchrom),
        n_medea_masks(1), n_wolbachia_status(1),
        mask_gen(3), divisor_gen(1), stride(4)
    {
        n_genotypes= pow(2.0,2*_nchrom);
        n_wolbachia_status = pow(2.0,n_incompat_wolbachia);
        if (do_medea) {
            n_medea_masks = pow(2.0,medea_units);
        }
        gen_prob=pow(0.25,_nchrom);
        for (size_t ii=0; ii<_nchrom; ++ii) {
            mask[ii] = mask_gen;
            divisor[ii] = divisor_gen;
            mask_gen *= stride;
            divisor_gen *= stride;
        }
    }
    vector<size_t> mask;
    vector<size_t> divisor;
    // put here to pass to BuildingCalculations
    // defaults to 1
    int n_genotypes;
    int n_medea_masks;
    int n_wolbachia_status;
    double gen_prob;
private:
    int mask_gen;
    int divisor_gen;
    int stride;
};

class Building {
public:
  // long-range migration mode, set from PbMsg
  static bool do_LR_block;
  static bool do_report_migrate;
  static ptr_csv out_migrate;
  // for reporting from inside buildings
  static size_t today;
  // members set by ctor 
  // all in csv except unique_id (which is computed)
  const size_t unique_id;
  const string location_code;
  const string block_code;
  bptr_t block_ptr;
  // release ids
  size_t id_adult_m;
  size_t id_adult_f;
  // end of csv input 
  vector<Receptacle> Container; 
  vector<Receptacle> Reserve_Containers;
private:
  static double Development_26;
  static double Development_134;
  // uses the devels 
  friend double Larvae_Cohort::Larvae_Calculations(double avg_water_temp);
  static double ComputeDevel(double water_temp);
  // location_code to unique_id
  static std::map<std::string, hid_t> id_map;
public:
  static vector<Building> & houses;
  // location to building reference
  static Building & get_ref(std::string loc_code) {
        auto id = get_id(loc_code);
        return houses[id];
  }
  // location to numeric index
  static size_t get_id(std::string loc_code) {
        // look up string
        auto it = id_map.find(loc_code) ;
        if (it == id_map.end()) {
            cout << "## Location code not found: " << loc_code << "\n";
            throw std::runtime_error("## Error in Building.h");
        }
        return it->second;
  }
// static members, 
  static Mask the_mask;
  static void SetDevel(size_t devel_mode, size_t sim_mode1);
  // 
  size_t TotalEggs;
  // 2 is sex, 4 is instar?
  array< array<size_t, N_INSTAR>, N_SEX> TotalLarvae;
  // summed over instar, sex
  size_t SumLarva; 
  // positive containers in this house
  size_t PosCont; 
  size_t TotalFemalePupae;
  size_t TotalMalePupae;
  size_t TotalFemaleNulliparousAdults;
  size_t TotalFemaleParousAdults;
  size_t TotalFemaleAdults;
  size_t TotalMaleAdults;
  size_t TotalNewlyParous;
  size_t TotalOvipositing;
  size_t Total_NewAdultNumber[2];
  double Total_NewAdultWeight[2];
  double Average_New_Female_Adult_Weight[5];
  bool Recapture;
  size_t Recaptured_Female,Recaptured_Male;
  list<Female_Adult> Female_Adults;
  list<Male_Adult_Cohort> Male_Adult;
  list<size_t> ContainerReleaseDates;

  // object containing ids and weights to neighbors
  hlink_t neighbor;

  list<Male_Adult_Cohort> ReleaseAdultM;
  list<Female_Adult> ReleaseAdultF;
  // mark true when finished releasing
  bool ReleaseDone;

  list<Female_Adult> Migrating_Female_Adult;
  list<Male_Adult_Cohort> Migrating_Male_Adult;

	// Stuff added from the experiment. This needs to be refactored to take an "Adulticide_regime" class specific to this house.

  int last_date_sprayed;
  double killing_efficacy;
//methods
  // ctors
  Building() = delete; 
  Building(const ReleaseInsect & release, const size_t _id, const string & _loc, const string & _block, const size_t id_m, const size_t id_f);
  //void SetBuildingCalculationParameters(int, double, double, int);
  //void BuildingCalculations();
  void BuildingCalculations(outLogs & logs, const Weather & vecWeather, TotalStruct & Totals, vbuild_t & housevec);
  void BuildingSummation(outLogs & logs, size_t Date, TotalStruct & Totals);
  void ContainerRelease();
  void StableAgeDist();
  void InsectRelease();
  void GridSetup(
    std::size_t & hposition, vbuild_t & the_houses
  );
  void MarkRecapture();
  template <typename Tlist, typename Tdest>
  void PushAdults(Tlist & release, Tdest & dest);
  // call after SetupRelease
  void UpdateReleaseDone() {
      ReleaseDone = (ReleaseAdultF.empty() && ReleaseAdultM.empty() );
  }
  hid_t LRGetRandomNeighbor();
  bool FemaleDispersal(lfem_t::iterator & itFem, vbuild_t & houses);
  bool LRFemaleDispersal(lfem_t::iterator & itFem, vbuild_t & houses);
  void MaleDispersal( lmale_t::iterator & itMale, vbuild_t & houses);
  void LRMaleDispersal( lmale_t::iterator & itMale, vbuild_t & houses);
  // migration reporter: defaults for female
  // s=short, l=long-range
  void ReportDispersal(size_t link, const char* type="", const char* sex="f") {
    if (do_report_migrate) {
        (*out_migrate) << today << unique_id << link << sex<< type << NEWLINE;
    }
  };
  void PlaceEggs(size_t sumeggs, std::map<int,Eggs_Cohort> & cohort_map );
};

// misc functions operating on vector of Buildings
void BuildingMoveContainers(vbuild_t & the_houses);

// deprecated?
// void BuildingSetupEdges (std::list<int> & the_edges);
void apply_residual_control(size_t Date, vbuild_t & houses);

// like list_move_to, 
// with adult-specific bits
template <typename Tlist, typename Tdest>
void Building::PushAdults(Tlist & release, Tdest & dest){
    //  walk through list, emptying as we go
    // consume list from end, 
    // removing elements as released
    auto it = release.begin();
    while (it != release.end()) {
        if (it->Release_Date!=today) {
            it++;
            continue;
        }
        // set from house id
        it->Emergence_Place = unique_id;
        // get next item
        auto tmp_it = std::next(it);
        // move to end
        dest.splice(dest.end(), release, it);
        // advance 
        it = tmp_it;
    }
}

