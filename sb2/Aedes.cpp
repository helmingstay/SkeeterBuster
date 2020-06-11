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
//

// global 
#include "Globals.h"  
// system
//#include <memory>

// protocol buffers setup
#include "PbRead.h"
#include "Output.h"
#include "OutputFuns.h"
#include "Input.h"
#include "Sampled.h"
#include "Link.h"
#include "Block.h"
#include "Building.h"
#include "Binomial.h"  
#include "helpers.h"  
#include "Weather.h"  
#include "Release.h"  
#include "LarvalParamStructs.h"
// #include "Traditional.h"

// Used by Binomial.cpp
// orig
// gsl_rng * pRNG = gsl_rng_alloc(gsl_rng_gfsr4);
// modern
gsl_rng * pRNG = gsl_rng_alloc(gsl_rng_mt19937);

//#define USE_GOMP 1

// Globals that are a necessary evil
// xian 2015: s/necessary evil/poor design/
size_t Date;

// static counters
size_t Female_Adult::Next_ID = 0;
size_t Male_Adult_Cohort::Next_ID = 0;
// used by the buildings...
// set in main()
// LR migration type, default to false (old, broken, for tests)
bool Building::do_LR_block = false;
// report migration?
bool Building::do_report_migrate = false;
ptr_csv Building::out_migrate;
// replace "Date" with today?
size_t Building::today = 0;

// constants for reuse
Mask Building::the_mask;
double Building::Development_26 = 0;
double Building::Development_134 = 0;

vector<LarvalGrowthParams> Larvae_Cohort::vec_growth;
vector<LarvalPupationWeightParams> Larvae_Cohort::vec_pupation;
double Larvae_Cohort::tmpSlope;
double Larvae_Cohort::tmpWeightDiff;

// use get_id and set_id
std::map<std::string, size_t> Building::id_map;
// block neighbor link map
bmap_t Block::blocks;

// House should be in main, and sized accordingly
vector<Building> House;

// set static ref to above vector 
vector<Building> & Building::houses(House);

    //@@ oldchunk 007

// Globals
// Protocol buffer messages
// Put these in a namespace?
// See PbInput.h
// msg containing all other msgs
PbInput::PbBuild PbMsg;

// methods for in/out path handling
// this is used to open files for reading from indir, to outdir
fopenDirFuns fopenF;

int main(int argc, char **argv)
{
    if (argc != 2) throw std::runtime_error("From Aedes.cpp, 1 argument required: path to PbMsg file");
    // setup code
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // Load msgs from serialized file prepared by python
    // defaults to .proto.PbBuild.dat?
    // protobuff fields are lowercase in cpp: PbMsg.submsg().field()
    // see msg PbRuntime in proto/PbInput.proto for details
    PbRead(PbMsg, argv[1]);
    // initialize IO directories 
    fopenF.set(PbMsg.runtime().dir_in(), PbMsg.runtime().dir_out());
    //time(0) initializing random variable seed
	//seed(PbMsg.sim().seed());
    //
    gsl_rng_set(pRNG, PbMsg.sim().seed());

    // use PbMsg to fill static members
    Larvae_Cohort::set_GrowthParams();

    // ??
	//if (PbMsg.sim().simulation_mode2()==DETERMINISTIC) PbMsg.sim().n_house()=1; 
	//PbMsg.sim().n_col()=(int) pow((double) PbMsg.sim().n_house(),0.5); 
	//PbMsg.sim().n_row()=PbMsg.sim().n_house()/PbMsg.sim().n_col();
	//printf("%d %d %d\n",PbMsg.sim().n_house(),PbMsg.sim().n_row(),PbMsg.sim().n_col()); 
    if (PbMsg.runtime().verbose()) {
        cout << "## Config : " << PbMsg.sim().n_house() << " houses, " << PbMsg.sim().number_of_days() << " days" << "\n";
    }

    // struct containing file handles of log files
	// FILE handles close when they go out of scope?
	// e.g. outLogs dtor
	// http://stackoverflow.com/questions/28300716/is-there-a-scope-managed-file-stream-in-c
    outLogs logs;
    //@@ oldchunk 005
    // ref # defaults to 1.  disable do_simlog to keep at 1
    // Setup functions class to  open/read from in/out dirs
    //
    if (PbMsg.output().do_simlog()) {
        // write simulation log info, return ref of next sim number
        logs.sim_log = fopenF.out(PbMsg.output().sim_log().c_str(), "a+");
        fopenF.fill_sim_log(logs);
    } else {
        fopenF.set_ref(0);
    }

    //////////////////////
    // read weather from csv
    Weather vecWeather(
        fopenF.inpath(PbMsg.runtime().file_weather()),
        PbMsg.sim().number_of_days()
    );

    // read csv specs for 
    // releases into containers / houses
    ReleaseInsect release(
        fopenF.inpath(PbMsg.runtime().file_release_egg()),
        fopenF.inpath(PbMsg.runtime().file_release_larva()),
        fopenF.inpath(PbMsg.runtime().file_release_pupa()),
        fopenF.inpath(PbMsg.runtime().file_release_adult_m()),
        fopenF.inpath(PbMsg.runtime().file_release_adult_f())
    );
   
    // get houses, containers from input files
	ReadHouseSetup(
        House, 
        fopenF.inpath(PbMsg.runtime().file_house()),
        fopenF.inpath(PbMsg.runtime().file_container()),
        release
    );
	CheckParameters();
    // must come after reading setup
	fopenF.OpenLogFiles(logs);

    // larvicide is very different than sampling...
    SampledContainers larvicide_schedule(   
        fopenF.inpath(PbMsg.runtime().file_larvicide())
    );
    if ((PbMsg.sim().traditional_control_method()==0) && PbMsg.larvicide().prespecify_larval_treatment()){
        // query db, fill sampled_houses_adults
        larvicide_schedule.init();
    }

    // if we conduct destructive sampling
    // initialize dates, house indexes, outfiles
    // from database
    // 3 measured fields: nAdult, nFem, nNullFem
    sample_adult_t sampled_houses_adults(
        fopenF.inpath(PbMsg.sample().in_adult_sampling())
    );
    if (PbMsg.sample().do_adult_sampling()) {
        // query db, fill sampled_houses_adults
        sampled_houses_adults.init(logs.sample_adult);
    }

    // immature sampling
    sample_cont_t sampled_houses_cont(
        fopenF.inpath(PbMsg.sample().in_cont_sampling())
    );
    if (PbMsg.sample().do_cont_sampling()) {
        sampled_houses_cont.init(logs.sample_cont);
    }

    // spraying is just like sampling, 
    // only we don't care about the results...
    sample_adult_t  sprayed_houses(
        fopenF.inpath(PbMsg.sample().adult_spraying())
    );
    if (PbMsg.sample().do_adult_spraying()) {
        // no output
        sprayed_houses.init(nullptr);
    }

    //@@ oldchunk 011

    // passed to BuildingCalculations
    // was global vars
    Building::the_mask = Mask(
        PbMsg.sim().chrom_number(),
	    (PbMsg.geneticcontrol().genetic_control_method()==MEDEA),
        PbMsg.medea().medea_units(),
        PbMsg.wolbachia().number_of_incompatible_wolbachia() 
    );

    //@@ oldchunk 006
    Building::SetDevel(
        PbMsg.sim().development_mode(),
        PbMsg.sim().simulation_mode1()
    );

    //??
	//#2
	vector<Building>::iterator itHouse;

	for ( auto & the_house : House) {
		the_house.ContainerReleaseDates.sort();
		the_house.ContainerReleaseDates.unique();
	}

    // fill dispersal networks of houses in House 
    // from database
        
    InDispersalHouse(
        fopenF.inpath(PbMsg.runtime().file_dispersal_house()), 
        House
    );
    if (PbMsg.sim().dispersal_lr_block()) {
        Building::do_LR_block = true;
        Block::blocks = InDispersalBlock(
            fopenF.inpath(PbMsg.runtime().file_dispersal_block()),
            House
        );
    }
    // tell Building
    if (PbMsg.output().do_migrate()) {
        Building::do_report_migrate = true;
        Building::out_migrate = logs.migrate;
    }
	//this is where the actual simulation starts, with the house cycle
    if (!PbMsg.runtime().quiet()) {
        fprintf(stdout, "\nStarting the simulation\n");
    }

	//!! Main day loop
	for (Date=0;Date<PbMsg.sim().number_of_days();Date++) {
        Building::today = Date;
        // reporting struct,
        // initialize daily
        TotalStruct Totals(Building::the_mask.n_genotypes, Building::the_mask.n_wolbachia_status, PbMsg.sim().chrom_number());
        // The state of the RNG helps debug
        //cout << "## Date, RNG: " << Date << ", " << Rng::nd << "\n"; 
		//this writes the number of date in the detailed log file
        //?? check start <= end earlier, in setup (R)?
		if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) {
             fprintf(logs.Output,"%s %zu\n","Day ",Date);
        }

		if (PbMsg.output().do_output_for_sbeed())
		{
			fprintf(logs.forSbeed, "\n%%%zu\n\n", Date);
			itHouse = House.begin();
			fprintf(logs.forSbeed, "\nAVERAGE TEMPERATURE = %lg\n\n", vecWeather[Date].meanTempC);
		}

        // variable decls for loops:
        // for ( itHouse )
		// pushing the released containers (including the initial setup on Date 0)
		for ( auto & the_house : House) {
            the_house.ContainerRelease();
        }

		// releases at stable age distribution
		if (    
            PbMsg.stableagedist().sad_introduction() 
            &&
			(Date==PbMsg.stableagedist().sad_introduction_date())
        ) {
            for (auto & the_house : House) {
                the_house.StableAgeDist();
            }
        }

		// pushing the released cohorts
		// includes immatures into containers
		// and adults into houses 
		for ( auto & the_house : House) {
            the_house.InsectRelease();
        }

        if (PbMsg.sim().mark_release_recapture()) {
            House[0].MarkRecapture();
        }
		if (PbMsg.sim().mark_release_recapture()) {
			//selecting houses for recapture
			for ( auto the_house : House) {
                // why is every house marked??
				itHouse->Recapture=1;
			}
		}

/*
#ifdef USE_GOMP
		int num_houses = (int)House.size();
		//Building buildings[num_houses];
		Building ** buildings = new Building*[num_houses];
		int current_b = 0;
		for (int current_b = 0; current_b < num_houses; current_b++)
		{
			buildings[current_b] = &House[current_b];
		}
// #pragma omp parallel for
		for (current_b = 0; current_b < num_houses; current_b++)
		{
			buildings[current_b]->BuildingCalculations(Date,current_b);
		}
		// 
		// for (int current_b = 0; current_b < num_houses; current_b++)
		// {
		// 	House[current_b] = buildings[current_b];
		// }
#else
*/

		// main house loop
		for (auto & the_house : House) {
            if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) {
                fprintf(logs.Output,"%s %zu\n","House ",the_house.unique_id);
            }
            the_house.BuildingCalculations(logs, vecWeather, Totals, House);
        }
//#endif

        //?? add dispersal reporter
        // the_house == destination
        // source address lives in migrating
		//moving dispersing adult mosquitoes to their destination houses
		for ( auto & hh : House) {
            splice_end(hh.Female_Adults, hh.Migrating_Female_Adult);
            splice_end(hh.Male_Adult, hh.Migrating_Male_Adult);
		}

        // destructive sampling of containers
        // larval is contingent on pupal 
        // same houses / dates, different obj
        if (PbMsg.sample().do_cont_sampling()){
            // called for side effect
            // sample and write
            take_cont(
                sampled_houses_cont, Date, 
                PbMsg.sample().sample_container_probability(),
                PbMsg.sample().proportion_of_larvae_sampled(),
                PbMsg.sample().proportion_of_pupae_sampled()
            );
        }
        if (PbMsg.sample().do_adult_sampling()) {
            take_adult(
                sampled_houses_adults, Date, 
                PbMsg.sample().proportion_of_adult_females_sampled(), PbMsg.sample().proportion_of_adult_males_sampled(), PbMsg.sample().do_adult_sampling_observe()
            );
        }

        /*
        DEPRECATED
		// traditional control methods
		// see proto/PbInput.proto for details
		if (PbMsg.sim().traditional_control_method() >0 )
		{
			// larvicides (position 0)
			if ((PbMsg.sim().traditional_control_method())%2) {
				if ((Date >= PbMsg.larvicide().larvicide_begin_date()) && (Date <= PbMsg.larvicide().larvicide_end_date()) && (Date%PbMsg.larvicide().larvicide_frequency()==0))
					Larvicide_grid(PbMsg.larvicide().larvicide_type(), PbMsg.larvicide().larvicide_region(), PbMsg.larvicide().larvicide_size(), House);
            }

			// adulticide (position 1)
			if ((PbMsg.sim().traditional_control_method()>>1)%2) {
				if ((Date >= PbMsg.adulticide().adulticide_begin_date())&&(Date <= PbMsg.adulticide().adulticide_end_date()))
					Adulticide_grid(PbMsg.adulticide().adulticide_type(), PbMsg.adulticide().adulticide_region(), PbMsg.adulticide().adulticide_size(), Date, House);
            }

			// source removal (position 2)
			if ((PbMsg.sim().traditional_control_method()>>2)%2) {
				if (Date == PbMsg.sourceremoval().source_removal_begin_date())
					Source_Removal_grid(PbMsg.sourceremoval().source_removal_type(), PbMsg.sourceremoval().source_removal_region(), PbMsg.sourceremoval().source_removal_size(), House);
            }
		}
        */

        // spraying is just like adult sampling
        // only don't write results 
		if (PbMsg.sample().do_adult_spraying()) {
            // separate probs for male and female
            take_adult(
                sprayed_houses, Date, 
                PbMsg.sample().adult_spraying_efficacy(), PbMsg.sample().adult_spraying_efficacy(),  PbMsg.sample().do_adult_sampling_observe(), true);
            // Deprecated
            if (PbMsg.adulticide().do_residual_control()) {
                apply_residual_control(Date, House);
            }
        }

        // treat houses
		if ((PbMsg.sim().traditional_control_method()==0) && PbMsg.larvicide().prespecify_larval_treatment()){
            larvicide_schedule.larv_update(Date);
            larvicide_schedule.larv_apply(Date);
        }

		// removing special containers
		for (auto & the_house : House)  {
			for( auto itContainer = the_house.Container.begin() ; itContainer!=the_house.Container.end() ; ) {
				if (itContainer->Container_Type_Number==102)
				{
					if (
                        itContainer->Eggs.empty()
                        &&
                        itContainer->Larvae.empty()
                        &&
                        itContainer->Pupae.empty()
					) {
						itContainer=the_house.Container.erase(itContainer);
					} else itContainer++;
				}
				else itContainer++;
			}
        }

		//moving containers between houses
		// FIXME - if (prob > 0 ) breaks test, RNG
        //if ( PbMsg.dispersal().container_movement_probability() > 0 ) {
            BuildingMoveContainers(House);
        //}
		

        //?? end of dynamics, beginning of daily summaries/reporting?
        DayEndReport(Date, fopenF, logs, House, Totals);
	}
	//this is the end of the day loop
	
    //////////////////////////////////////////
    // Write out final output
    //////////////////////////////////////////

	if (PbMsg.output().do_save_final_state_as_setup_file()) {
        fopenF.SaveHouseSetupOut(logs.HouseSetupOut, House);
    }
	return 0;
}//this is the end of the main function
