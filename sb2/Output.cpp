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

#include "Globals.h" 
#include "Output.h"
#include "Receptacle.h"
#include "Building.h"
#include "Female_Adult.h"
#include "Male_Adult_Cohort.h"
#include "Cohort.h"

// check that path points to directory
void checkIsDir (string path, string error_msg) {
    // if problem, print error_msg + path & throw 
    bool problem;
    struct stat s;
    int err = stat(path.c_str(), &s);
    if(-1 == err) {
        problem = true;
    } else {
        if(S_ISDIR(s.st_mode)) {
            problem = false;
        } else {
            /* exists but is no dir */
            problem = true;
        }
    }
    if (problem) {
        std::cout << error_msg << path << std::endl;
        throw std::runtime_error("path not a directory (check stdout)");
    }
}

// open csv writer from file basename 
// in output directory, set writer delimiters
ptr_csv fopenDirFuns::OpenCsv(
    std::string fn, std::vector<string> header,
    const char fieldsep = ',', 
    const std::string & quotesep="$$"
) {
    // full path
    const std::string full_path = outpath(fn);
    //const char * full_path = outpath(fn).data();
    ptr_csv ret = std::make_shared<csv::ofstream>(full_path.data());
    ret->set_delimiter(fieldsep, quotesep);
    // write header line
    for (auto field : header) {
        (*ret) << field;
    } 
    (*ret) << NEWLINE;
    return(ret);
}

void fopenDirFuns::OpenLogFiles(outLogs &logs) {
  //opening of log files
	if (PbMsg.output().do_output_for_sbeed()) {
		logs.forSbeed=outref(PbMsg.output().output_for_sbeed());
		fprintf(logs.forSbeed, "NUMBER OF HOUSES = %d\n\n", PbMsg.sim().n_house());
		fprintf(logs.forSbeed, "NUMBER_OF_COLUMNS = %d\n\n", PbMsg.sim().n_col());
		fprintf(logs.forSbeed, "NUMBER_OF_ROWS = %d\n\n", PbMsg.sim().n_row());
		fprintf(logs.forSbeed, "NUMBER OF DAYS = %d\n\n", PbMsg.sim().number_of_days());
		fprintf(logs.forSbeed, "GENETIC_CONTROL_METHOD = %d\n\n", PbMsg.geneticcontrol().genetic_control_method());
	}

	if (PbMsg.output().do_output_per_container()) {
		logs.Output_per_Container=outref(PbMsg.output().output_per_container());
		fprintf(logs.Output_per_Container, "Day House Container Food Container_Type Filling_Method Eggs 1st 2nd 3rd 4th Pupae_Total NewPupae_Total NewPupae_Female NewPupae_Male NewPupaeWeight_Female NewPupaeWeight_Male Water_Level Food\n");
	}
    if( PbMsg.output().do_short_output() ) {
        logs.Short_Output.open(outpath(PbMsg.output().short_output()), std::fstream::out);
        logs.Short_Output.precision(8);
        printer(logs.Short_Output,"", "Date Totals.Eggs Totals.Female_Larvae Totals.Male_Larvae Totals.Female_Pupae Totals.Male_Pupae Totals.Nulliparous_Female_Adults Totals.Parous_Female_Adults Totals.Male_Adults\t Totals.Genotypes...  Totals.Adult_Genotypes...  Totals.allele_frequency...  variable... ","\n");
    }
    if( PbMsg.output().do_minimal_output() ) {
        logs.minimal_Output = OpenCsv(
            PbMsg.output().minimal_output(),
            {"Date","Total_Eggs", "Total_Female_Larvae", "Total_Male_Larvae", "Total_Female_Pupae", "Total_Male_Pupae", "Total_Nulliparous_Female_Adults", "Total_Parous_Female_Adults", "Total_Male_Adults"},
            ' '
        );
    }
    // for model comparison, per day
    if( PbMsg.output().do_compare_cont() ) {
        logs.compare_cont = OpenCsv(
            PbMsg.output().compare_cont(),
            {"date","container","egg", "larva", "larva_weight", "pupa", "food", "water_ht", "water_temp"}
        );
    }
    if( PbMsg.output().do_compare_house() ) {
        logs.compare_house = OpenCsv(
            PbMsg.output().compare_house(),
            {"date","house","adult_f","adult_m","eggs_laid","extinct"}
        );
    }
    if( PbMsg.output().do_day_house() ) {
        logs.day_house = OpenCsv(
            PbMsg.output().day_house(),
            {"date","house","ncont", "ncont_pos","egg","larva","pupa","adult","adult_f", "adult_fnull"}
        );
    }
    if( PbMsg.output().do_migrate() ) {
        logs.migrate = OpenCsv(
            PbMsg.output().migrate(),
            {"date","self","link","sex","type"}
        );
    }

    // old, too big / messy to be very interesting.
    if (PbMsg.output().do_detailed_log()) {
        // check start < stop
        if ( PbMsg.output().detailed_log_start() > PbMsg.output().detailed_log_start() ) {
            throw std::range_error("Detailed log start day later than end day");
        } 
        logs.Output=outref(PbMsg.output().detailed_log()); //this opens the detailed log file
    }
    if (PbMsg.output().do_output_per_container_type()) {
        logs.Output_per_Container_Type=outref(PbMsg.output().output_per_container_type());
    }
    if (PbMsg.output().do_output_water_level()) {
        logs.Output_Water_Level=outref(PbMsg.output().output_water_level());
    }
    if (PbMsg.output().do_output_per_house()) {
      logs.Output_per_House=outref(PbMsg.output().output_per_house());
    }
    // sampling
    if (PbMsg.sample().do_adult_sampling()) {
        // space-separated
        logs.sample_adult =  OpenCsv(
            PbMsg.sample().adult_sampling(),
            {"Date","House","nAdult","nFem","nNullFem"}, 
            ' '
        );
    }
    if (PbMsg.sample().do_cont_sampling()) {
        // space-separated
        logs.sample_cont =  OpenCsv(
            PbMsg.sample().cont_sampling(),
            {"Date","House","nCont", "nPos", "nLarva", "nPupa"},
            ' '
        );
    }
    if (PbMsg.output().do_simlog()) {
        logs.HouseSetupOut=outref(PbMsg.output().save_final_state_as_setup_file(), ".setup", "w+");
    }
    if (PbMsg.output().do_csv_house()) {
        // open file, write header
        logs.csv_house = OpenCsv(
            PbMsg.output().csv_house(),
            {"Date","Position","nFemale","nMale", "nEgg", "nCont", "Sprayed" }
        );
    }
}

// write out one per house
// see OpenLogFiles for header
void fopenDirFuns::write_csv_house(ptr_csv pcsv, int date, std::vector<Building> & houses ) {
    if (!pcsv->is_open()){
        throw std::runtime_error("file for csv_house not open");
    }
    // each house gets a row
    for (auto& hh: houses)  {
        (*pcsv) << date << hh.unique_id << 
        hh.TotalFemaleNulliparousAdults + hh.TotalFemaleParousAdults <<
        hh.TotalMaleAdults <<
        hh.TotalEggs <<
        hh.Container.size() <<
        (hh.last_date_sprayed == date) <<
        NEWLINE;
    }
}

// was Get_Number
// fill simlog with info from Pb
// also get ref number
void fopenDirFuns::fill_sim_log(outLogs &logs) {
    int i=1;
    char c=' ';
    fseek(logs.sim_log, -1, SEEK_END) ;
    while (++i, c!='#' && ftell(logs.sim_log) && !feof(logs.sim_log)) {
        fseek(logs.sim_log, -i, SEEK_END) ;
        fscanf(logs.sim_log, "%c", &c) ;
    }

    // Search for refnum in log, increment
    // set member ref
    if (c!='#') ref=0 ;
    else fscanf(logs.sim_log, "%d", &ref) ;
    ref++ ;

    fseek(logs.sim_log, 0L, SEEK_SET) ; 
    // logging the simulation parameters description
    fprintf(logs.sim_log, "\n===================================================================================\n") ;
    fprintf(logs.sim_log, "Simulation #%3d :\n", ref);
    fprintf(logs.sim_log, "-----------------\n\n") ;
    fprintf(logs.sim_log, "%s\n", PbMsg.runtime().run_datetime().c_str());
    fprintf(logs.sim_log, "Based on SkeeterBuster with Compile Date, Time = %s, %s\n", __DATE__, __TIME__);
    fprintf(logs.sim_log, "%d days, %d houses\n", PbMsg.sim().number_of_days(), PbMsg.sim().n_house()) ;
    fprintf(logs.sim_log, "%d chromosomes\n", PbMsg.sim().chrom_number()) ;
    fprintf(logs.sim_log, "Genetic control method : ");
    switch(PbMsg.geneticcontrol().genetic_control_method()) {
    case 0:
          fprintf(logs.sim_log, "NEUTRAL\n");
          break;

    case 1:
          fprintf(logs.sim_log, "ENGINEERED UNDERDOMINANCE -- TYPE ") ;
          if (PbMsg.engunderdom().eu_type() >=1 && PbMsg.engunderdom().eu_type()<=4) fprintf(logs.sim_log, "%d\n", PbMsg.engunderdom().eu_type()) ;
          else fprintf(logs.sim_log, "unknown\n") ;
          fprintf(logs.sim_log, "\tFitness cost construct A: %.3f\n", PbMsg.engunderdom().fitness_cost_construct_alpha());
          fprintf(logs.sim_log, "\tFitness cost construct B: %.3f\n",  PbMsg.engunderdom().fitness_cost_construct_beta());
          if (PbMsg.engunderdom().eu_type()>=2) fprintf(logs.sim_log, "\tFitness cost construct C: %.3f\n", PbMsg.engunderdom().fitness_cost_construct_gamma());
          if (PbMsg.engunderdom().eu_type()>=3) fprintf(logs.sim_log, "\tFitness cost construct D: %.3f\n", PbMsg.engunderdom().fitness_cost_construct_delta());
          fprintf(logs.sim_log, "\tDominance of fitness costs: %.3f\n", PbMsg.engunderdom().fitness_cost_dominance());
          break;

    case 2:
          fprintf(logs.sim_log, "WOLBACHIA\n") ;
          break;

    case 3:
          fprintf(logs.sim_log, "MEIOTIC DRIVE -- SCENARIO ") ;
          if (PbMsg.meioticdrive().md_type() >=1 && PbMsg.meioticdrive().md_type()<=3) fprintf(logs.sim_log, "%d\n", PbMsg.meioticdrive().md_type()) ;
          else fprintf(logs.sim_log, "unknown\n") ;
          fprintf(logs.sim_log, "\tFitness cost Xit: %.3f\n", PbMsg.meioticdrive().fitness_cost_insensitive_transgene() );
          if (PbMsg.meioticdrive().md_type()>=2) fprintf(logs.sim_log, "\tFitness cost Xin: %.3f\n", PbMsg.meioticdrive().fitness_cost_insensitive_natural() );
          fprintf(logs.sim_log, "\tFitness cost YD: %.3f\n", PbMsg.meioticdrive().fitness_cost_drive_gene() );
          fprintf(logs.sim_log, "\tDominance of fitness costs: %.3f\n", PbMsg.meioticdrive().md_fitness_cost_dominance());
          fprintf(logs.sim_log, "\tDrive strength on Xsn: %.3f\n", PbMsg.meioticdrive().drive_strength_sn() );
          fprintf(logs.sim_log, "\tDrive strength on Xit: %.3f\n", PbMsg.meioticdrive().drive_strength_it() );
          if (PbMsg.meioticdrive().md_type()>=2) fprintf(logs.sim_log, "\tDrive strength on Xin: %.3f\n", PbMsg.meioticdrive().drive_strength_in() );
          if (PbMsg.meioticdrive().md_type()>=3) fprintf(logs.sim_log, "\tDrive strength on Xsn+modifier: %.3f\n", PbMsg.meioticdrive().drive_strength_mn() );
          break;

    case 4:
          fprintf(logs.sim_log, "MEDEA\n") ;
          fprintf(logs.sim_log, "\tNumber of MEDEA constructs: %d\n", PbMsg.medea().medea_units());
          fprintf(logs.sim_log, "\tCross rescuing elements: %s\n", PbMsg.medea().medea_cross_rescue()?"YES":"NO");
          fprintf(logs.sim_log, "\tMaternal lethality: %.3f\n", PbMsg.medea().medea_maternal_lethality());
          fprintf(logs.sim_log, "\tFitness cost per construct: %.3f\n", PbMsg.medea().medea_fitness_cost_per_construct());
          fprintf(logs.sim_log, "\tFitness cost dominance: %.3f\n", PbMsg.medea().medea_fitness_cost_dominance()); 
          break;
      
    case 6:
          if (PbMsg.femkillrescue().is_rescue()) fprintf(logs.sim_log, "KILLING+RESCUE");
          else fprintf(logs.sim_log, "KILLING");
          fprintf(logs.sim_log, "\tNumber of killing loci: %d\n", PbMsg.femkillrescue().number_of_female_killing_loci());
          fprintf(logs.sim_log, "\tFemale-killing only: %s\n", PbMsg.femkillrescue().fkr_is_embryo_killing()?"NO":"YES");
          fprintf(logs.sim_log, "\tKilling efficiency: %.3f\n", PbMsg.femkillrescue().female_killing_efficiency());
          fprintf(logs.sim_log, "\tKilling alleles homozygous fitness cost: %.3f\n", PbMsg.femkillrescue().killing_allele_homozygous_fitness_cost());
          fprintf(logs.sim_log, "\tKilling alleles fitness cost dominance: %.3f\n", PbMsg.femkillrescue().killing_allele_fitness_cost_dominance());
          if (PbMsg.femkillrescue().is_rescue()) {
              fprintf(logs.sim_log, "\tRescue alleles homozygous fitness cost: %.3f\n", PbMsg.femkillrescue().rescue_allele_homozygous_fitness_cost());
              fprintf(logs.sim_log, "\tRescue alleles fitness cost dominance: %.3f\n", PbMsg.femkillrescue().rescue_allele_fitness_cost_dominance());
          }
          break;

    case 8:
          fprintf(logs.sim_log, "REDUCE_REPLACE\n") ;
          //fprintf(logs.sim_log, "\tScenario: %d\n", 0 );
          fprintf(logs.sim_log, "Construct gene fitness cost applied at stage: %d\n", PbMsg.geneticcontrol().fitness_cost_stage_rr());
          break;

    default:
          fprintf(logs.sim_log, "unknown\n") ;
          break;
    }

    fprintf(logs.sim_log, "Fitness costs are applied at stage %d at age %d\n", PbMsg.geneticcontrol().fitness_cost_stage(), PbMsg.geneticcontrol().fitness_cost_stage_specific_age());

    fprintf(logs.sim_log, "Border condition type : ") ;
    switch(PbMsg.sim().dispersal_mode()) {
    case 0:
          fprintf(logs.sim_log, "STICKY BORDERS\n") ;
          break;
    case 1:
          fprintf(logs.sim_log, "BOUNCING BORDERS\n") ;
          break;
    case 2:
          fprintf(logs.sim_log, "TORE\n") ;
          break;
    case 3:
          fprintf(logs.sim_log, "RANDOM EDGES\n") ;
          break;
    default:
          fprintf(logs.sim_log, "unknown\n") ;
    }
      
    fprintf(logs.sim_log, "Dispersal rates :\n") ;
    fprintf(logs.sim_log, "\t- short range : ") ;
    if (PbMsg.dispersal().nulliparous_female_adult_dispersal() || PbMsg.dispersal().parous_female_adult_dispersal() || PbMsg.dispersal().male_adult_dispersal())
    {
        fprintf(logs.sim_log, "\n") ;
        fprintf(logs.sim_log, "\t    n = %.3lf\n", PbMsg.dispersal().nulliparous_female_adult_dispersal()) ;
        fprintf(logs.sim_log, "\t    p = %.3lf\n", PbMsg.dispersal().parous_female_adult_dispersal()) ;
        fprintf(logs.sim_log, "\t    m = %.3lf\n", PbMsg.dispersal().male_adult_dispersal()) ;
    } else fprintf(logs.sim_log, "NONE\n") ;
    fprintf(logs.sim_log, "\t- long range : ") ;
    if (PbMsg.dispersal().nulliparous_female_adult_long_range_dispersal() || PbMsg.dispersal().parous_female_adult_long_range_dispersal() || PbMsg.dispersal().male_adult_long_range_dispersal())
    {
        fprintf(logs.sim_log, "\n") ;
        fprintf(logs.sim_log, "\t    n = %.3lf\n", PbMsg.dispersal().nulliparous_female_adult_long_range_dispersal()) ;
        fprintf(logs.sim_log, "\t    p = %.3lf\n", PbMsg.dispersal().parous_female_adult_long_range_dispersal()) ;
        fprintf(logs.sim_log, "\t    m = %.3lf\n", PbMsg.dispersal().male_adult_long_range_dispersal()) ;
        fprintf(logs.sim_log, "\t    maxdist = %d\n", PbMsg.dispersal().lr_dispersal_max_distance()) ;
    } else fprintf(logs.sim_log, "NONE\n") ;

    fprintf(logs.sim_log, "Container properties:\n");
    fprintf(logs.sim_log, "\t- Ct turnover rate: %.5f\n", PbMsg.dispersal().container_movement_probability());
    fprintf(logs.sim_log, "\t- emptying rate of manually filled Ct: %.3f\n", PbMsg.container().probability_of_emptying_manually_filled_containers());
    fprintf(logs.sim_log, "\t- oviposition reduction in covered Ct: %.3f\n", PbMsg.container().oviposition_reduction_in_covered_containers());

    fprintf(logs.sim_log, "Female adult survival type : ") ;
    switch(PbMsg.agedepsurv().female_age_dependent_survival())
    {
    case 0:
      fprintf(logs.sim_log, "CONSTANT at %.3lf\n", PbMsg.nominaldailysurvival().skeeter_buster_female_adults());
      break;
    case 1:
      fprintf(logs.sim_log, "LINEAR DECREASE from %.3lf at age 0 to 0 at age %d\n", PbMsg.nominaldailysurvival().skeeter_buster_female_adults(), PbMsg.agedepsurv().female_maximum_age_1()) ;
      break ;
    case 2:
      fprintf(logs.sim_log, "CONSTANT at %.3lf from age 0 to age %d THEN LINEAR DECREASE to 0 at age %d\n", PbMsg.nominaldailysurvival().skeeter_buster_female_adults(), PbMsg.agedepsurv().female_start_senescence_2(), PbMsg.agedepsurv().female_maximum_age_2()) ;
      break ;
    }
    fprintf(logs.sim_log, "Male adult survival type : ") ;
    switch(PbMsg.agedepsurv().male_age_dependent_survival())
    {
    case 0:
      fprintf(logs.sim_log, "CONSTANT at %.3lf\n", PbMsg.nominaldailysurvival().skeeter_buster_male_adults());
      break;
    case 1:
      fprintf(logs.sim_log, "LINEAR DECREASE from %.3lf at age 0 to 0.000 at age %d\n", PbMsg.nominaldailysurvival().skeeter_buster_male_adults(), PbMsg.agedepsurv().male_maximum_age_1()) ;
      break ;
    case 2:
      fprintf(logs.sim_log, "CONSTANT at %.3lf from age 0 to age %d THEN LINEAR DECREASE to 0.000 at age %d\n", PbMsg.nominaldailysurvival().skeeter_buster_male_adults(), PbMsg.agedepsurv().male_start_senescence_2(), PbMsg.agedepsurv().male_maximum_age_2()) ;
      break ;
    }
    fclose(logs.sim_log);
}

//! pairs with ReadHouseSetup in Input.cpp
// ??deprecated? 
void fopenDirFuns::SaveHouseSetupOut(FILE *HouseSetupOut, vector<Building> & the_houses)
{
	// saves the final state of the population into a SETUP file for future simulations
	//
	// note that some information is not carried over
	// e.g. mated status of females, and their mate's genotype
	//
	// passing those would require a change in the SETUP file structure
	//
	// !! this is a good reference for the setup file format

	fprintf(stdout, "Creating final state SETUP file ... ");
	fprintf(HouseSetupOut,"# SKEETER BUSTER house setup file\n");
	fprintf(HouseSetupOut,"\n\n##\n");
	fprintf(HouseSetupOut,"Initialized by simulation %d on %s\n",ref,PbMsg.runtime().run_datetime().c_str());
	fprintf(HouseSetupOut,"##\n\n");
	fprintf(HouseSetupOut,"NUMBER_OF_HOUSES = %d\n",PbMsg.sim().n_house());
	fprintf(HouseSetupOut,"NUMBER_OF_COLUMNS = %d\n",PbMsg.sim().n_col());
	fprintf(HouseSetupOut,"NUMBER_OF_ROWS = %d\n",PbMsg.sim().n_row());
	fprintf(HouseSetupOut,"\n\n\n");
	int House_Number=0;
	for ( const auto & the_house : the_houses) {
		fprintf(HouseSetupOut,"# HOUSE %d\n",House_Number);
		fprintf(HouseSetupOut,"\n\n\n");
		fprintf(HouseSetupOut,"HOUSE_NUMBER = %d\n\n",House_Number);
		fprintf(HouseSetupOut,"NUMBER_OF_CONTAINERS = %lu\n",the_house.Container.size());
		fprintf(HouseSetupOut,"\n\n\n");
		vector<Receptacle>::iterator itContainer;
		int Container_Number=0;
		for (const auto & the_cont: the_house.Container) {
			fprintf(HouseSetupOut,"# CONTAINER %d\n",Container_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"CONTAINER_NUMBER = %d\n\n",Container_Number);
			fprintf(HouseSetupOut,"CONTAINER_TYPE = %d\n\n",the_cont.Container_Type_Number);
			fprintf(HouseSetupOut,"RELEASE_DATE = %zu\n",the_cont.Release_Date);
			fprintf(HouseSetupOut,"\n\n");
			fprintf(HouseSetupOut,"HEIGHT = %lf\n\n",the_cont.Height);
			fprintf(HouseSetupOut,"SURFACE = %lf\n\n",the_cont.Surface);
			fprintf(HouseSetupOut,"COVERED = %d\n\n",the_cont.Covered);
			fprintf(HouseSetupOut,"MONTHLY_TURNOVER = %lf\n\n",the_cont.Monthly_Turnover_Rate);
			fprintf(HouseSetupOut,"SUNEXPOSURE = %lf\n\n",the_cont.SunExposure);
			fprintf(HouseSetupOut,"DAILY_FOOD_LOSS = %lf\n\n",the_cont.Daily_Food_Loss);
			fprintf(HouseSetupOut,"DAILY_FOOD_GAIN = %lf\n\n",the_cont.Daily_Food_Gain);
			fprintf(HouseSetupOut,"COVER_REDUCTION = %lf\n\n",the_cont.Cover_Reduction);
			fprintf(HouseSetupOut,"FILLING_METHOD = %d\n\n",the_cont.Filling_Method);
			fprintf(HouseSetupOut,"DRAW_DOWN = %lf\n\n",the_cont.Draw_Down);
			fprintf(HouseSetupOut,"WATERSHED_RATIO = %lf\n\n",the_cont.Watershed_Ratio);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"INITIAL_WATER_LEVEL = %lf\n\n",the_cont.Water_Level);
			fprintf(HouseSetupOut,"INITIAL_LARVAL_FOOD = %lf\n\n",the_cont.Food);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_EGG_COHORTS = %zu\n\n",the_cont.Eggs.size());		
			fprintf(HouseSetupOut,"\n\n\n");
			int EggCohort_Number=0;
			for (const auto & the_eggs : the_cont.Eggs) {
				fprintf(HouseSetupOut,"# EGG COHORT %d\n",EggCohort_Number);
				fprintf(HouseSetupOut,"\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_EGGS = %zu\n\n",the_eggs.Number);
				fprintf(HouseSetupOut,"LEVEL_LAID = %lf\n\n",the_eggs.Level_laid_in_Container);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",the_eggs.Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %zu\n\n",the_eggs.Age);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",the_eggs.Mature);
				fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",the_eggs.Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",the_eggs.Wolbachia);
				fprintf(HouseSetupOut,"\n\n");
				fprintf(HouseSetupOut,"# end of EGG COHORT %d\n",EggCohort_Number);
				fprintf(HouseSetupOut,"\n\n");
				EggCohort_Number++;
			}
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_LARVAL_COHORTS = %zu\n",the_cont.Larvae.size());
			fprintf(HouseSetupOut,"\n\n\n");
			int LarvaeCohort_Number=0;
			for (auto the_larva : the_cont.Larvae) {
				fprintf(HouseSetupOut,"# LARVAE COHORT %d\n",LarvaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_LARVAE = %zu\n\n",the_larva.Number);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",the_larva.Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %zu\n\n",the_larva.Age);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",the_larva.Mature);
				fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
				fprintf(HouseSetupOut,"AVERAGE_LARVAL_WEIGHT = %lf\n\n",the_larva.Average_Larval_Weight);
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",the_larva.Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",the_larva.Wolbachia);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"# end of LARVAE COHORT %d\n",LarvaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				LarvaeCohort_Number++;
			}
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_PUPAL_COHORTS = %zu\n",the_cont.Pupae.size());
			fprintf(HouseSetupOut,"\n\n\n");
			int PupaeCohort_Number=0;
			for (const auto & the_pupa : the_cont.Pupae) {
				fprintf(HouseSetupOut,"# PUPAE COHORT %d\n",PupaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_PUPAE = %zu\n\n",the_pupa.Number);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",the_pupa.Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %zu\n\n",the_pupa.Age);
				fprintf(HouseSetupOut,"AVERAGE_PUPAL_WEIGHT = %lf\n\n",the_pupa.Average_Pupal_Weight);
				fprintf(HouseSetupOut,"AVERAGE_LIPID_RESERVE = %lf\n\n",the_pupa.Average_Pupal_Lipid_Reserve);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",the_pupa.Mature);
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",the_pupa.Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",the_pupa.Wolbachia);
				fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"# end of PUPAE COHORT %d\n",PupaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				PupaeCohort_Number++;
			}
			
			fprintf(HouseSetupOut,"# end of CONTAINER %d\n",Container_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			Container_Number++;
		}
		fprintf(HouseSetupOut,"NUMBER_OF_FEMALE_ADULT_COHORTS = %zu\n",the_house.Female_Adults.size());
		fprintf(HouseSetupOut,"\n\n\n");
		int FemaleAdult_Number=0;
		for (const auto & the_fem : the_house.Female_Adults) {
			fprintf(HouseSetupOut,"# FEMALE ADULT %d\n",FemaleAdult_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_FEMALE_ADULTS = 1\n\n");
			fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",the_fem.Physiological_Development_Percent);
			fprintf(HouseSetupOut,"AGE = %zu\n\n",the_fem.Age);
			fprintf(HouseSetupOut,"AVERAGE_ADULT_WEIGHT = %lf\n\n",the_fem.Average_Adult_Weight);
			fprintf(HouseSetupOut,"MATURE = %d\n\n",the_fem.Mature);
			fprintf(HouseSetupOut,"NULLIPAROUS = %d\n\n",the_fem.Nulliparous);
			fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",the_fem.Genotype);
			fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",the_fem.Wolbachia);
			fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"# end of FEMALE ADULT COHORT %d\n",FemaleAdult_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			FemaleAdult_Number++;
		}
		fprintf(HouseSetupOut,"NUMBER_OF_MALE_ADULT_COHORTS = %zu\n",the_house.Male_Adult.size());
		fprintf(HouseSetupOut,"\n\n\n");
		int MaleAdultCohort_Number=0;
		for (const auto & the_male : the_house.Male_Adult) {
			fprintf(HouseSetupOut,"# MALE ADULT COHORT %d\n",MaleAdultCohort_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_MALE_ADULTS = %zu\n\n",the_male.Number);
			fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",the_male.Physiological_Development_Percent);
			fprintf(HouseSetupOut,"AGE = %zu\n\n",the_male.Age);
			fprintf(HouseSetupOut,"AVERAGE_ADULT_WEIGHT = %lf\n\n",the_male.Average_Adult_Weight);
			fprintf(HouseSetupOut,"MATURE = %d\n\n",the_male.Mature);
			fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",the_male.Genotype);
			fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",the_male.Wolbachia);
			fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"# end of MALE ADULT COHORT %d\n",MaleAdultCohort_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			MaleAdultCohort_Number++;
		}
		fprintf(HouseSetupOut,"# end of HOUSE %d\n",House_Number);
		fprintf(HouseSetupOut,"\n\n\n");		
		House_Number++;
	}
	fclose(HouseSetupOut);
	fprintf(stdout, "done.\n");
}
        //@@ oldchunk 003
        //

void DayEndReport(
    int Date, 
    fopenDirFuns & fopenF, 
    outLogs & logs, 
    std::vector<Building> & House,
    TotalStruct & Totals
) {
    size_t House_position=0;

    //@@ oldchunk 001

    if (PbMsg.output().do_output_for_sbeed()){
        // @ sign brackets building reporting
        fprintf(logs.forSbeed, "@\n");
    }
    for ( auto & hs : House) {
        hs.BuildingSummation(logs, Date, Totals);
        if ( PbMsg.output().do_day_house()){
            (*(logs.day_house)) 
                << Date
                << hs.location_code
                << hs.Container.size() 
                << hs.PosCont 
                << hs.TotalEggs 
                << hs.SumLarva 
                << hs.TotalMalePupae + hs.TotalFemalePupae 
                << hs.TotalMaleAdults + hs.TotalFemaleAdults
                << hs.TotalFemaleAdults
                << hs.TotalFemaleNulliparousAdults
                << NEWLINE;
        }
    }
    if (PbMsg.output().do_output_for_sbeed()){
        // @ sign brackets building reporting
        fprintf(logs.forSbeed, "@\n");
    }
    //@@ oldchunk 002

    if (Totals.has_insects) {
        for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++)
        {
            for (unsigned genotype=0;genotype<Totals.Genotypes.size(); genotype++)
            {
                //?? WTF
                if ((genotype/((int) pow(2,2*PbMsg.sim().chrom_number()-1-allele))%2)==0) Totals.allele_frequency[allele]+=Totals.Genotypes[genotype]; else Totals.allele_frequency[allele+1]+=Totals.Genotypes[genotype];
                if ((genotype/((int) pow(2,2*PbMsg.sim().chrom_number()-2-allele))%2)==0) Totals.allele_frequency[allele]+=Totals.Genotypes[genotype]; else Totals.allele_frequency[allele+1]+=Totals.Genotypes[genotype];
            }
            allele++;
        }
        for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++) {
            Totals.allele_frequency[allele]/=2*Totals.Total_Mosquitoes;
        }
        for (auto & wol_freq : Totals.wolbachia_frequency ) {
            wol_freq /= Totals.Total_Mosquitoes;
        }
    }

    //!! MAGIC NUMBER
    // recapture function
    double Recaptured_Females_per_distance[6],Recaptured_Males_per_distance[6];
    if (PbMsg.sim().mark_release_recapture())
    {
        for (int distance=0; distance<6;distance++) Recaptured_Females_per_distance[distance]=Recaptured_Males_per_distance[distance]=0;
        House_position=0;
        for ( auto & itHouse : House) {
            if (itHouse.Recaptured_Female>0) {
                int distance;
                distance=(int) ROUND_(pow((double) ((House_position%6)*(House_position%6)+(House_position/6)*(House_position/6)),0.5));
                Recaptured_Females_per_distance[distance]+=itHouse.Recaptured_Female;
            }
            if (itHouse.Recaptured_Male>0) {
                int distance;
                distance=(int) ROUND_(pow((double) ((House_position%6)*(House_position%6)+(House_position/6)*(House_position/6)),0.5));
                Recaptured_Males_per_distance[distance]+=itHouse.Recaptured_Male;
            }
            House_position++;
        }
    }

    //?? what/why this?
    Totals.CV_Pupae /= (double)PbMsg.sim().n_house() ;
    double Average_number_of_pupae = (double)(Totals.Female_Pupae+Totals.Male_Pupae)/(double)PbMsg.sim().n_house() ;
    Totals.CV_Pupae -= Average_number_of_pupae*Average_number_of_pupae ;
    Totals.CV_Pupae = sqrt(Totals.CV_Pupae) ;
    Totals.CV_Pupae /= Average_number_of_pupae;

    //!! MAGIC NUMBERS
    if (PbMsg.geneticcontrol().genetic_control_method()==ReduceReplace)
    {
        Totals.CV_WILD_FEMALES /= (double)PbMsg.sim().n_house();
        Totals.CV_WILD_ALLELES /= (double)PbMsg.sim().n_house();
        double Average_Num_Wild_Female_Genotype = (double) (Totals.Adult_Genotypes[0] + Totals.Adult_Genotypes[16] + Totals.Adult_Genotypes[32] + Totals.Adult_Genotypes[48])/ (double)PbMsg.sim().n_house();
        double Average_Num_Wild_Female_ALLELE = (double) (Totals.Adult_Genotypes[0]+Totals.Adult_Genotypes[16]+Totals.Adult_Genotypes[32]+Totals.Adult_Genotypes[48] + Totals.Adult_Genotypes[20]+Totals.Adult_Genotypes[4]+Totals.Adult_Genotypes[36]+Totals.Adult_Genotypes[52] + Totals.Adult_Genotypes[24]+Totals.Adult_Genotypes[8]+Totals.Adult_Genotypes[40]+Totals.Adult_Genotypes[56])/ (double)PbMsg.sim().n_house();
        Totals.CV_WILD_FEMALES -= Average_Num_Wild_Female_Genotype*Average_Num_Wild_Female_Genotype;
        Totals.CV_WILD_ALLELES -= Average_Num_Wild_Female_ALLELE*Average_Num_Wild_Female_ALLELE;
        Totals.CV_WILD_FEMALES = sqrt(Totals.CV_WILD_FEMALES);
        Totals.CV_WILD_ALLELES = sqrt(Totals.CV_WILD_ALLELES);
        if (Average_Num_Wild_Female_Genotype)
            Totals.CV_WILD_FEMALES /= Average_Num_Wild_Female_Genotype;
        else
            Totals.CV_WILD_FEMALES = 0;

        if (Average_Num_Wild_Female_ALLELE)
            Totals.CV_WILD_ALLELES /= Average_Num_Wild_Female_ALLELE;
        else
            Totals.CV_WILD_ALLELES = 0;

        // so this is really a frequency?
        Totals.NO_WILD_FEMALES /= (double)PbMsg.sim().n_house();
        Totals.NO_WILD_ALLELES /= (double)PbMsg.sim().n_house();
    }

    //@@ oldchunk 009

    //!! function, daily reporting starts here
    if( PbMsg.output().do_minimal_output() ) {
        (*(logs.minimal_Output)) << Date << Totals.Eggs << Totals.Female_Larvae << Totals.Male_Larvae << Totals.Female_Pupae << Totals.Male_Pupae << Totals.Nulliparous_Female_Adults << Totals.Parous_Female_Adults << Totals.Male_Adults << NEWLINE;
    }
    if( PbMsg.output().do_compare_cont() ) {
        // sum over all containers, 
        // print number of containers & means
        if( PbMsg.output().compare_cont_mean() ) {
            double ncont(0);
            double negg(0), nlarva(0), nlarva_cohort(0), larva_weight(0), npupa(0), food(0), water(0), temp(0);
            auto get_larva_weight = [](Larvae_Cohort & item) { return item.Average_Larval_Weight; };
            for ( auto & hh : House) {
                ncont += hh.Container.size();
                for (auto & cc: hh.Container) { 
                    negg += cohort_sum(cc.Eggs);
                    nlarva += cohort_sum(cc.Larvae);
                    // divide by total cohorts at end
                    nlarva_cohort += cc.Larvae.size();
                    larva_weight += cohort_sum_fun<Larvae_Cohort>(cc.Larvae, get_larva_weight);
                    npupa += cohort_sum(cc.Pupae);
                    food += cc.Food;
                    water += cc.Water_Level;
                    temp += cc.Average_Daily_Water_Temperature;
                }
            }
            // zero if no larva, mean otherwise
            larva_weight = (nlarva==0) ? 
               0 : (larva_weight/nlarva_cohort);
            // write output
            (*(logs.compare_cont)) << Date << ncont << negg << nlarva << larva_weight << npupa << food << water/ncont << temp/ncont << NEWLINE;
        } else {
            // otherwise container id 
            // and per-container details
            for ( auto & hh : House) {
                for (auto & cc: hh.Container) { 
                    (*(logs.compare_cont)) << Date << cc.unique_id << cohort_sum(cc.Eggs) << cohort_sum(cc.Larvae) << "TODO" << cohort_sum(cc.Pupae) << cc.Food << cc.Water_Level << cc.Average_Daily_Water_Temperature << NEWLINE;
                }
            }
        }
    }
    if( PbMsg.output().do_compare_house() ) {
        for ( auto & hh : House) {
            (*(logs.compare_house)) << Date << hh.unique_id << hh.Female_Adults.size() << cohort_sum(hh.Male_Adult) << Totals.EggsLaid << !Totals.has_insects << NEWLINE;
        }
    }
    if( PbMsg.output().do_short_output() ) {
        printer(logs.Short_Output," ",Date,Totals.Eggs,Totals.Female_Larvae,Totals.Male_Larvae,Totals.Female_Pupae,Totals.Male_Pupae,Totals.Nulliparous_Female_Adults,Totals.Parous_Female_Adults,Totals.Male_Adults);
        logs.Short_Output << "\t" ;
        for (int genotype : Totals.Genotypes) {
            logs.Short_Output << " " << genotype;
        }
        logs.Short_Output << "\t";
        for (int genotype : Totals.Adult_Genotypes) {
            logs.Short_Output << " " << genotype;
        }
        logs.Short_Output << "\t";
        for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++) {
            logs.Short_Output << " " << Totals.allele_frequency[allele];
        }
        logs.Short_Output << "\t";
        for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
        {
            double wolbfreq = 0;
            for (unsigned w=0 ; w<Totals.wolbachia_frequency.size(); w++) {
                if ((w>>ww)%2)
                    wolbfreq+=Totals.wolbachia_frequency[w];
            }
            logs.Short_Output << " " << wolbfreq;
        }

        if (PbMsg.sim().mark_release_recapture())
        {
            for (auto distance=0;distance<6;distance++) { 
                logs.Short_Output << " " << Recaptured_Females_per_distance[distance];
            }
            for (auto distance=0;distance<6;distance++) { 
                logs.Short_Output << " " << Recaptured_Males_per_distance[distance];
            }
        }
        logs.Short_Output << "\t";
        logs.Short_Output << "  " << Totals.CV_Pupae ;

        if (PbMsg.geneticcontrol().genetic_control_method()==ReduceReplace) {
            logs.Short_Output << "  ";
            printer(logs.Short_Output, "  ", Totals.CV_WILD_FEMALES, Totals.CV_WILD_ALLELES, Totals.NO_WILD_FEMALES, Totals.NO_WILD_ALLELES);
        }

        //fprintf(logs.Short_Output,"\n");
        // temp -- percent positive container
        //int positive=0;
        //int totalnumber=0;
        //for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )
        //	for (vector<Receptacle>::iterator itContainer = itHouse->Container.begin() ; itContainer != itHouse->Container.end() ; itContainer++)
        //	{
        //		totalnumber++;
        //		if (itContainer->Eggs.size())
        //		{
        //			for (list<Eggs_Cohort>::iterator itEggs = itContainer->Eggs.begin() ; itEggs != itContainer->Eggs.end() ; itEggs++)
        //				if (itEggs->Age <7)
        //				{
        //					positive++;
        //					break;
        //				}
        //		}
        //	}
        //double percentpositive = positive / (double) totalnumber;
        //fprintf(logs.Short_Output, " %.5g", percentpositive);
        logs.Short_Output << "\n";
    }
    // log into the log per container type file (only pupae for now)
    if (PbMsg.output().do_output_per_container_type())
    {
        fprintf(logs.Output_per_Container_Type, "%d", Date);
        for (int type=0 ; type<200 ; type++) fprintf(logs.Output_per_Container_Type, " %.0f %.0f %.0f", Totals.Census_by_container_type[0][type], Totals.Census_by_container_type[4][type], Totals.Census_by_container_type[3][type]) ;
        for (int type=0 ; type<200 ; type++) fprintf(logs.Output_per_Container_Type, " %.6g", Totals.Census_by_container_type[5][type]) ;
        fprintf(logs.Output_per_Container_Type, "\n") ;
    }
    if (PbMsg.output().do_output_water_level())
    {
        for (int type=0; type<200; type++)
            if (Totals.Census_by_container_type[0][type] >0)
                fprintf(logs.Output_Water_Level, "%d %d %.3f %.1f %.1f %.1f %.3f\n", Date, type,
                        Totals.Census_by_container_type[6][type]/Totals.Census_by_container_type[0][type], // water level
                        Totals.Census_by_container_type[7][type]/Totals.Census_by_container_type[0][type], // maximum water temp
                        Totals.Census_by_container_type[8][type]/Totals.Census_by_container_type[0][type], // minimum water temp
                        Totals.Census_by_container_type[9][type]/Totals.Census_by_container_type[0][type], // average water temp
                        Totals.Census_by_container_type[10][type]/Totals.Census_by_container_type[0][type] // food
                );
    }
    //@@ oldchunk 010
    //@@ oldchunk 008
//@@ oldchunk 004

    // progress monitor
    double perComplete = (100.0*Date)/PbMsg.sim().number_of_days();
    // control reporting frequency
    if (PbMsg.runtime().verbose() && (Date % PbMsg.runtime().report_freq() ) == 0 ) {
        // star if extinct
        printer(cout, "", "\r## Day ", Date, " Done, ", perComplete, "\% complete", (Totals.has_insects ? "" : "**extinct**"), "\n") ;
    }

    // example stub 
    // at the end of each day, 
    // write all houses to csv
    if (PbMsg.output().do_csv_house() ) {
        fopenF.write_csv_house(logs.csv_house, Date, House);
    }
}
