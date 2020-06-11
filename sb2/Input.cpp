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

// Container_type numbers ={'BATH':1, 'BOTTLE':2, 'CAN':3, 'COOK':4, 'DPLAS':5, 'LTANK':6, 'MISC':7, 'MSTOR':8, 'NATURAL':9, 'ODD':10, 'PET':11, 'TIRE':12, 'VASE':13, 'WELL':14}

#include "Globals.h"
// InputContainer
#include "Input.h"
#include "Link.h"
#include "Block.h"
#include "Building.h"
#include "Release.h"
#include "Receptacle.h"

using std::move;

void ErrorMessage (int);

// return the filled vector of houses
void ReadHouseSetup(
    vector<Building> & houses,
    const std::string & file_house,
    const std::string & file_cont,
    ReleaseInsect & release
) {
    if (PbMsg.runtime().verbose()) {
        cout << "\n## Reading house config file: " << file_house << "\n## Reading container config file: " << file_cont << "\n";
    }
    // house, container csv reader
    csv_reader<4> in_house(file_house,
        "location_code","block_code","id_adult_m","id_adult_f"
    );
    // specialized reader, returns string and vector of doubles
    InputCsv<20> in_cont(file_cont, 
        "location_code","container_number","container_type","cover_reduction","covered","daily_food_gain","daily_food_loss","drawdown","filling_method","height","initial_larval_food","initial_water_level","monthly_turnover","sunexposure","surface","watershed_ratio","release_date", "id_egg","id_larva","id_pupa"
    );
        
    // convenience
    size_t nhouse = PbMsg.sim().n_house();
    size_t ncont_row = PbMsg.sim().n_cont_row();
    // 
    houses.reserve(nhouse);
    // make these once, then fill from csv
    string a_loc, a_block;
    // releases
    size_t id_m, id_f;
    // vector to be filled with input row
    for ( size_t ii(0); ii < nhouse; ii++){
        // read the row into the house
        // fix!!
        auto ok = in_house.read_row(a_loc, a_block, id_m, id_f);
        if (!ok) { 
            std::cerr << "##!! Error reading house csv. Incorrect number of houses?\n";
            throw std::runtime_error("In Input.cpp");
        }
        // construct object in place
        houses.emplace_back(release, ii, a_loc, a_block, id_m, id_f);
    }
    // resize a_datvec, reuse a_loc
    vector<double> a_datvec = in_cont.make_vector();
    // unique_id counter
    size_t icont(0);
    // fill containers from input csv
    for (size_t irow(0); irow < ncont_row; irow++){
        // pass refs to fill here!!
        bool ok = in_cont.read_row(a_loc, a_datvec);
        if (!ok) { 
            throw std::runtime_error("Input.cpp: Problem reading container.");
        }
        // grab pointer to this container's house, by name
        auto * phouse = &(houses[Building::get_id(a_loc)]);
        // place one copy of this row for each number
        for (size_t ncont(a_datvec[0]); ncont >0; ncont--){
            // construct container, adjust food gain from param, them move
            auto the_cont = Receptacle(phouse, icont++, a_datvec, release);
            the_cont.Daily_Food_Gain *= PbMsg.container().food_input_multiplier();
            phouse->Reserve_Containers.push_back(move(the_cont));
        }
    }
    if (PbMsg.sim().n_container() != icont) {
        throw std::runtime_error("Input.cpp: Container number mismatch.");
    }
}

hrows_t ReadLinkHouse(std::string path) {
    // Dispersal csv fields (block, house)
    string in_self, in_link;
    hid_t id_self;
    double in_weight;
    // multimap of rows keyed by string 
    hrows_t spec;
    // reader
    csv_reader<3> in_csv(path, "self", "link", "weight");
    while( in_csv.read_row(in_self, in_link, in_weight)) {
        // one row: fill, move
        hrow_t item;
        id_self = Building::get_id(in_self);
        item.first = Building::get_id(in_link);
        item.second = in_weight;
        spec.emplace(id_self, move(item));
    }
    return spec;
};


// differs from above by return-type, no name-to-index
brows_t ReadLinkBlock(std::string path) {
    // Dispersal csv fields (block, house)
    string in_self, in_link;
    double in_weight;
    // multimap of rows keyed by string 
    brows_t spec;
    // reader
    csv_reader<3> in_csv(path, "self", "link", "weight");
    while( in_csv.read_row(in_self, in_link, in_weight)) {
        // one row
        brow_t item;
        item.first = in_link;
        item.second = in_weight;
        spec.emplace(in_self, move(item));
    }
    return spec;
};

// read neighbor network from db into houses
// normalize by house
void InDispersalHouse(
    std::string path_house,  
    vbuild_t & houses
) {
    auto spec = ReadLinkHouse(path_house);
    // for each key
    for( auto it_key = spec.begin(), end = spec.end(); it_key != end; it_key = spec.upper_bound(it_key->first)) {
        // self
        auto iself = it_key->first;
        // last of this key
        auto it_up = spec.upper_bound(iself);
        Link<hid_t> link(it_key, it_up);
        houses[iself].neighbor = link;
    }
};

// dispersal between blocks
// leave id key as string
// fill and return map
bmap_t InDispersalBlock(
    std::string path,
    vbuild_t & houses
) {
    auto spec = ReadLinkBlock(path);
    bmap_t fin;
    // for each key
    for( auto it_key = spec.begin(), end = spec.end(); it_key != end; it_key = spec.upper_bound(it_key->first)) {
        // self
        bid_t iself = it_key->first;
        // last of this key
        auto it_up = spec.upper_bound(iself);
        // construct link with iterators
        bptr_t pblock(new Block(iself, it_key, it_up));
        fin.emplace(iself, move(pblock));
    }
    // register each house with resp block, and vise versa
    for( auto & house : houses) {
        bptr_t pblock = fin.at(house.block_code);
        pblock->add_house(house.unique_id);
        // store block ref in house
        house.block_ptr = pblock;
    }
    // sanity, check each block after filling
    for( auto the_pair: fin) {
        bptr_t check = the_pair.second;
        if (check->isEmpty()) {
            std::cerr << "##!! Error: block contains no houses: " << check->self << std::endl;
            throw std::runtime_error("In Input.cpp");
        }
    }
    return fin;
};

void CheckParameters()
// this function to list parameters check and return errors 
// if some parameters values are non realistic or incompatible
{
    if (!PbMsg.runtime().quiet()) {
        fprintf(stdout, "Checking parameters ...");
    }

	// calculation mode required for soft selection with embryonic costs
	if ((PbMsg.geneticcontrol().fitness_cost_stage()==0)&&(PbMsg.geneticcontrol().embryonic_cost_selection_type()==SOFT)&&(PbMsg.sim().reproduction_mode()==SAMPLING))
	{
		fprintf(stdout, "\n*** Error: Embryonic costs with soft selection require CALCULATION");
		fprintf(stdout, "\n*** Please make sure CALCULATION mode is selected, or ");
		fprintf(stdout, "\n*** set PbMsg.sim().reproduction_mode() to 0 in the CONF file\n");
		ErrorMessage(1);
	}
	// costs have to be costs (ie. 0 < values < 1) with certain selection types

	if ((PbMsg.geneticcontrol().fitness_cost_stage()==1) || (PbMsg.geneticcontrol().fitness_cost_stage()==2) || (PbMsg.geneticcontrol().fitness_cost_stage()==3) || ((PbMsg.geneticcontrol().fitness_cost_stage()==0)&&(PbMsg.geneticcontrol().embryonic_cost_selection_type()==HARD)))
		if ((
			((PbMsg.geneticcontrol().genetic_control_method()==1) && 
			(!CheckProba(PbMsg.engunderdom().fitness_cost_construct_alpha()) 
				|| !CheckProba( PbMsg.engunderdom().fitness_cost_construct_beta()) 
				|| !CheckProba(PbMsg.engunderdom().fitness_cost_construct_gamma()) 
				|| !CheckProba(PbMsg.engunderdom().fitness_cost_construct_delta())))
			||
			((PbMsg.geneticcontrol().genetic_control_method()==3) &&
			(!CheckProba(PbMsg.meioticdrive().fitness_cost_insensitive_transgene())
				|| !CheckProba(PbMsg.meioticdrive().fitness_cost_insensitive_natural())
				|| !CheckProba(PbMsg.meioticdrive().fitness_cost_drive_gene())))
			||
			((PbMsg.geneticcontrol().genetic_control_method()==4) &&
			(!CheckProba(PbMsg.medea().medea_fitness_cost_per_construct())))
			||
			((PbMsg.geneticcontrol().genetic_control_method()==6) &&
			(!CheckProba(PbMsg.femkillrescue().killing_allele_homozygous_fitness_cost())
				|| !CheckProba(PbMsg.femkillrescue().rescue_allele_homozygous_fitness_cost())))
			||
			((PbMsg.geneticcontrol().genetic_control_method()==7) &&
			(!CheckProba(PbMsg.control().selective_fitness_cost())))
			))
	{
		fprintf(stdout, "\n*** Error: Fitness costs at stage %d", PbMsg.geneticcontrol().fitness_cost_stage());
		if (PbMsg.geneticcontrol().fitness_cost_stage()==0)
			fprintf(stdout, " with hard selection");
		fprintf(stdout, "\n*** require costs to be strictly between 0 and 1.");
		fprintf(stdout, "\n*** Please check the fitness costs for your selected control mechanism\n");
		ErrorMessage(1);
	}

	// MEDEA is incompatible with fitness costs othe than embryonic
	if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA)
	  if (PbMsg.geneticcontrol().fitness_cost_stage()!=0)
		{
			fprintf(stdout, "\n*** Error: fitness costs other than embryonic are incompatible with MEDEA genetic control method\n");
			ErrorMessage(1);
		}

	// defining region for introduction at stable age distribution
	if ((PbMsg.stableagedist().sad_introduction())&&(PbMsg.stableagedist().sad_introduction_region()==1))
	{
		if ( PbMsg.stableagedist().sad_introduction_x_min() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Minimum X coordinate (PbMsg.stableagedist().sad_introduction_x_min())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.stableagedist().sad_introduction_x_max() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.stableagedist().sad_introduction_x_max())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.stableagedist().sad_introduction_x_max() < PbMsg.stableagedist().sad_introduction_x_min())
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.stableagedist().sad_introduction_x_max())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.stableagedist().sad_introduction_x_min())\n");
			ErrorMessage(1);
		}
		if (PbMsg.stableagedist().sad_introduction_y_min() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Minimum Y coordinate (PbMsg.stableagedist().sad_introduction_y_min())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if ( PbMsg.stableagedist().sad_introduction_y_max() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.stableagedist().sad_introduction_y_max())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.stableagedist().sad_introduction_y_max() < PbMsg.stableagedist().sad_introduction_y_min())
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.stableagedist().sad_introduction_y_max())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.stableagedist().sad_introduction_y_min())\n");
			ErrorMessage(1);
		}
	}
	// larvicidal control -- check region size

	if ((PbMsg.sim().traditional_control_method()%2)&&(PbMsg.larvicide().larvicide_region()==1)&&(!(PbMsg.larvicide().larvicide_size()>0)))
	{
		fprintf(stdout, "\n*** Error: Immature control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of PbMsg.larvicide().larvicide_size()\n");
		ErrorMessage(1);
	}
	if ((PbMsg.sim().traditional_control_method()%2)&&(PbMsg.larvicide().larvicide_region()==2))
	{
		if (PbMsg.larvicide().larvicide_region_2_xmin() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Minimum X coordinate (PbMsg.larvicide().larvicide_region_2_xmin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.larvicide().larvicide_region_2_xmax() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.larvicide().larvicide_region_2_xmax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.larvicide().larvicide_region_2_xmax() < PbMsg.larvicide().larvicide_region_2_xmin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.larvicide().larvicide_region_2_xmax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.larvicide().larvicide_region_2_xmin())\n");
			ErrorMessage(1);
		}
		if (PbMsg.larvicide().larvicide_region_2_ymin() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Minimum Y coordinate (PbMsg.larvicide().larvicide_region_2_ymin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.larvicide().larvicide_region_2_ymax() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.larvicide().larvicide_region_2_ymax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.larvicide().larvicide_region_2_ymax() < PbMsg.larvicide().larvicide_region_2_ymin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.larvicide().larvicide_region_2_ymax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.larvicide().larvicide_region_2_ymin())\n");
			ErrorMessage(1);
		}
	}
	// adulticidal control -- check region size

	if (((PbMsg.sim().traditional_control_method()>>1)%2)&&(PbMsg.adulticide().adulticide_region()==1)&&(!(PbMsg.adulticide().adulticide_size()>0)))
	{
		fprintf(stdout, "\n*** Error: Adulticidal control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of PbMsg.adulticide().adulticide_size()\n");
		ErrorMessage(1);
	}
	if (((PbMsg.sim().traditional_control_method()>>1)%2)&&(PbMsg.adulticide().adulticide_region()==2))
	{
		if (PbMsg.adulticide().adulticide_region_2_xmin() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Minimum X coordinate (PbMsg.adulticide().adulticide_region_2_xmin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.adulticide().adulticide_region_2_xmax() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.adulticide().adulticide_region_2_xmax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.adulticide().adulticide_region_2_xmax() < PbMsg.adulticide().adulticide_region_2_xmin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.adulticide().adulticide_region_2_xmax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.adulticide().adulticide_region_2_xmin())\n");
			ErrorMessage(1);
		}
		if (PbMsg.adulticide().adulticide_region_2_ymin() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Minimum Y coordinate (PbMsg.adulticide().adulticide_region_2_ymin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.adulticide().adulticide_region_2_ymax() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.adulticide().adulticide_region_2_ymax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.adulticide().adulticide_region_2_ymax() < PbMsg.adulticide().adulticide_region_2_ymin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.adulticide().adulticide_region_2_ymax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.adulticide().adulticide_region_2_ymin())\n");
			ErrorMessage(1);
		}
	}
	// source removal control -- check region size

	if (((PbMsg.sim().traditional_control_method()>>2)%2)&&(PbMsg.sourceremoval().source_removal_region()==1)&&(!(PbMsg.sourceremoval().source_removal_size()>0)))
	{
		fprintf(stdout, "\n*** Error: Source removal control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of PbMsg.sourceremoval().source_removal_size()\n");
		ErrorMessage(1);
	}
	if (((PbMsg.sim().traditional_control_method()>>2)%2)&&(PbMsg.sourceremoval().source_removal_region()==2))
	{
		if (PbMsg.sourceremoval().source_removal_region_2_xmin() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Minimum X coordinate (PbMsg.sourceremoval().source_removal_region_2_xmin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.sourceremoval().source_removal_region_2_xmax() > PbMsg.sim().n_col()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.sourceremoval().source_removal_region_2_xmax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_col()-1);
			ErrorMessage(1);
		}
		if (PbMsg.sourceremoval().source_removal_region_2_xmax() < PbMsg.sourceremoval().source_removal_region_2_xmin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum X coordinate (PbMsg.sourceremoval().source_removal_region_2_xmax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.sourceremoval().source_removal_region_2_xmin())\n");
			ErrorMessage(1);
		}
		if (PbMsg.sourceremoval().source_removal_region_2_ymin() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Minimum Y coordinate (PbMsg.sourceremoval().source_removal_region_2_ymin())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.sourceremoval().source_removal_region_2_ymax() > PbMsg.sim().n_row()-1)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.sourceremoval().source_removal_region_2_ymax())");
			fprintf(stdout, "\n*** must be between 0 and %d\n", PbMsg.sim().n_row()-1);
			ErrorMessage(1);
		}
		if (PbMsg.sourceremoval().source_removal_region_2_ymax() < PbMsg.sourceremoval().source_removal_region_2_ymin())
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (PbMsg.sourceremoval().source_removal_region_2_ymax())");
			fprintf(stdout, "\n*** must be higher than minimum (PbMsg.sourceremoval().source_removal_region_2_ymin())\n");
			ErrorMessage(1);
		}
	}
	// if everything is passed
    if (!PbMsg.runtime().quiet()) {
        fprintf(stdout, "Checked Parameters, OK.\n");
    }
}
