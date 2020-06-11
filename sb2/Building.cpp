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
// main classes of stuff
#include "Receptacle.h"  
#include "Binomial.h"
#include "helpers.h"
#include "Fitness.h" 
#include "DevelKinetics.h"
#include "Weather.h" 
#include "Block.h"
// thread test
//#include <future> 
struct ReleaseInsect;

// IO
#include "fastcsv_type.h"
#include "Output.h" // logs
void ErrorMessage (int);

using vec_uint = vector<unsigned int>;

Building::Building(const ReleaseInsect & release, const size_t _id, const string & _loc, const string & _block, const size_t _id_m, const size_t _id_f):
    unique_id(_id), 
    location_code(_loc), 
    // row of house input csv
    // see Input.cpp: in_house for colnames
    block_code(_block), 
    id_adult_m(_id_m), 
    id_adult_f(_id_f),
    // default ctor
    Container(0),
    Reserve_Containers(0),
    Female_Adults(0),
    Male_Adult(0),
    ContainerReleaseDates(0),
    ReleaseDone(false),
    last_date_sprayed(-1)
{
    id_map.emplace(_loc, _id);
    //  use default ctor
    // in Release.h
    SetupRelease(id_adult_m, release.adult_m, ReleaseAdultM);
    SetupRelease(id_adult_f, release.adult_f, ReleaseAdultF, true);
    // sanity check genotypes on read-in
    for (const auto & adult_m: ReleaseAdultM) {
        if( SexFromGenotype(adult_m.Genotype) != MALE) {
            throw std::runtime_error("## In release file: adult_m with female genotype.");
        }
    }
    for (const auto & adult_f: ReleaseAdultF) {
        if( SexFromGenotype(adult_f.Genotype) != FEMALE) {
            throw std::runtime_error("## In release file: adult_f with male genotype.");
        }
    }
    UpdateReleaseDone();
};


//#define USE_GOMP 1

//@@ oldchunk 006

//@@ oldchunk 005

void Building::BuildingCalculations(outLogs & logs, const Weather & vecWeather, TotalStruct & Totals, vbuild_t & housevec)
{
    // rolling mean of temperature required by receptacles
    //  pass them full weather record
    //  otherwise just vecWeather[Date]
    /// variables used when chosing the container for oviposition:
    /// sum of density*(ln(volume)+1) for all containers, number of randomly chosen container in the stochastic version, counter used in the stochastic version
	double ContainerSum(0); 
	double Standard_Deviation_for_fecundity;
	//double Variance_for_fecundity;
	double Egg_Number;
	double sumeggs_laid=0;
	int Number_of_Suitable_Containers = 0;

    double Daily_Average_Female_Adult_Weight(0);
    double Daily_Average_Female_Fecundity(0);
	//local variables of the house are transferred to global variables that can be passed on to functions
	//clearing of some house variables that are recalculated each day

	this->Total_NewAdultNumber[MALE]=0.0;
	this->Total_NewAdultNumber[FEMALE]=0.0;
	this->Total_NewAdultWeight[MALE]=0.0;
	this->Total_NewAdultWeight[FEMALE]=0.0;

    bool doDetailedLogToday = PbMsg.output().do_detailed_log()&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end());

	//this is the start of the container loop
	for (auto & the_cont : this->Container) {
		the_cont.ReceptacleCalculations_Init(Date,logs, vecWeather, doDetailedLogToday); 
        // deterministic
		the_cont.Larva_Calc_Determ();
		the_cont.ReceptacleCalculations_Fin(logs, doDetailedLogToday); 
    }

    // break apart steps
    // works but is laughably slow
    // opencl makes more sense?
    //std::vector<std::future<void>> futures;
    //futures.reserve(this->Container.size());

	//for (auto & the_cont : this->Container) {
		//the_cont.Larva_Calc_Determ();
		//Receptacle * pcont = &the_cont;
        //auto the_fut = std::async(std::launch::async,&Receptacle::Larva_Calc_Determ, pcont); 
        //futures.push_back(std::move(the_fut));
    //}
    //
	//for (auto & fut : futures) {
        //fut.get();
    //}
    //
	//for (auto & the_cont : this->Container) {
		//the_cont.ReceptacleCalculations_Fin(logs, doDetailedLogToday); 
	//}

    //#endif
	//this is the end of the container cycle


	//ORIGINAL CIMSIM //the below code creates one female/male cohort for all the pupae that emerge on the given day with an average weight

	//this part establishes the new female adults into a new female adult cohort, and marks it as nulliparous //#17

	if (this->Total_NewAdultNumber[FEMALE]>0) {
		if (PbMsg.sim().simulation_mode1()==CIMSIM) {
            Daily_Average_Female_Adult_Weight=this->Total_NewAdultWeight[FEMALE]/this->Total_NewAdultNumber[FEMALE]; //calculates the average weight of the female adult cohort
            double number = this->Total_NewAdultNumber[FEMALE];
            double avgWeight = Daily_Average_Female_Adult_Weight*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult();
            double fecundity = avgWeight * PbMsg.devel().female_adult_daily_fecundity_factor_for_wet_weight();

            // create new female adult cohort as nulliparous, with specified number, weight and fecundit
            for (int f=0 ; f<number ; f++) {
                // age_since_hatch cannot be calculated here, average age would have to be calculated
                this->Female_Adults.emplace_back(avgWeight, NULLIPAROUS, fecundity, 0, 0, 0, false, 0, 0, -1);
            }
        } else {
            Daily_Average_Female_Adult_Weight=0;
        }
    }

	//this establishes the new adult males to their adult male cohort //#18
	if (this->Total_NewAdultNumber[MALE]>0) {
		if (PbMsg.sim().simulation_mode1()==CIMSIM) {
			this->Total_NewAdultWeight[MALE]=this->Total_NewAdultWeight[MALE]/this->Total_NewAdultNumber[MALE]; //this calculates the average weight of the new adult males
			//this locates the first empty adult male cohort

			// age_since_hatch cannot be calculated here, average age would have to be calculated
			this->Male_Adult.emplace_back(this->Total_NewAdultNumber[MALE], this->Total_NewAdultWeight[MALE]*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult(),1,0,0);
		} else {
			this->Total_NewAdultWeight[MALE]=0;
		}
	}

	//this part adds up the container indeces that determine choice of oviposition container by size

	ContainerSum=0;
    // initialize, zero-filled
	vector<double> container_probabilities(Container.size(), 0);
	vec_uint eggs_laid_in_each(Container.size(),0);

	Number_of_Suitable_Containers = 0;
	if (Container.size()) {
		for (size_t j=0;j<Container.size();j++) {
			if (this->Container[j].Container_Type_Number!=102)
				// skip specific container types (see ctypes.txt)
			{
                // combine food and suface (food/surface) ? (food/volume) ?
				container_probabilities[j] = log(1+((this->Container[j].Surface)*(this->Container[j].Height)));
				if (this->Container[j].Covered==1) container_probabilities[j] *= PbMsg.container().oviposition_reduction_in_covered_containers() ; // reduced oviposition in covered containers

				Number_of_Suitable_Containers++;

				// strong reduction in probabilities if eggs are present
				auto total_number_of_egg_cohorts = this->Container[j].Eggs.size() ;
				if (total_number_of_egg_cohorts) {
                    //??const 0.1
					container_probabilities[j]*=PbMsg.egg().egg_effect_on_oviposition_probability();
                }
				// prototype to enhance probability if larvae present
				/*
				auto total_number_of_larvae_cohorts = this->Container[j].Larvae.size() ;
				if (total_number_of_larvae_cohorts)
					{
					double temporary_container_probability = 0;
					temporary_container_probability = container_probabilities[j];
					temporary_container_probability = temporary_container_probability * INCREASED_LARVAL_EFFECT;
					container_probabilities[j] = temporary_container_probability;
					}
				 */
                auto total_number_of_larvae_cohorts = this->Container[j].Larvae.size();
                if (total_number_of_larvae_cohorts) {
                    container_probabilities[j] *= PbMsg.egg().larval_effect_on_oviposition_probability();
                }
				ContainerSum += container_probabilities[j];
			}
		}

		// Now normalize the container probabilities
		if (ContainerSum>0){
			for (size_t j=0;j<Container.size();j++)
			{
				container_probabilities[j] /= ContainerSum ;
			}
		} else {
            std::fill(container_probabilities.begin(), container_probabilities.end(), 1./Container.size() );
        }
	}

	//ORIGINAL CIMSIM //to calculate the moving average of newly emerged female weights for fecundity
	//this part calculates daily average female fecundity, based on the CIMSIM code //#19
	//new average female nulliparous weight only added if new adults have emerged

	if (Daily_Average_Female_Adult_Weight>0) {
		//move 4 earlier average female adult weights of emerging females, discard the 5th, and make place for the average female adult weight of the newly emerged nulliparous females
		int x=4;
		do
		{
			if (this->Average_New_Female_Adult_Weight[x-1]>0) {
                this->Average_New_Female_Adult_Weight[x]=this->Average_New_Female_Adult_Weight[x-1];
            }
			x--;
		}
		while (x>0);
		//insert the average female adult weight of the newly emerged females into the empty slot
		this->Average_New_Female_Adult_Weight[0]=Daily_Average_Female_Adult_Weight*PbMsg.devel().female_adult_conversion_factor_of_dry_weight_of_larvae_to_wet_weight_of_adult();
	}

	Daily_Average_Female_Adult_Weight=0;
	//calculate an average female adult weight by calculating a moving average of the average female asult weight of the last max. five newly emerged nulliparous female adult cohorts
	int x=0;

    // MAGIC NUMBER!!
	for (int j=0;j<5;j++) {
		Daily_Average_Female_Adult_Weight+=this->Average_New_Female_Adult_Weight[j];
		if (this->Average_New_Female_Adult_Weight[j]>0) x++;
	}
	if (x>0)  {
        Daily_Average_Female_Adult_Weight=Daily_Average_Female_Adult_Weight/x; 
    } else { 
        Daily_Average_Female_Adult_Weight=0;
    }
    
	//calculate daily average female fecundity
	Daily_Average_Female_Fecundity=Daily_Average_Female_Adult_Weight*PbMsg.devel().female_adult_daily_fecundity_factor_for_wet_weight();

	//clear total number of newly parous and female nulliparous adults

	this->TotalNewlyParous=0;
	this->TotalFemaleNulliparousAdults=0;
	this->TotalOvipositing=0;

	//sum the number of female nulliparous adults
	for (const auto & the_fem : Female_Adults) {
        this->TotalFemaleNulliparousAdults += (the_fem.Nulliparous==NULLIPAROUS);
	}

	//ORIGINAL CIMSIM
	if (
        (PbMsg.sim().simulation_mode1()==CIMSIM) && 
        (this->TotalFemaleNulliparousAdults==0)
    ) {
        Daily_Average_Female_Fecundity=0.5*Daily_Average_Female_Fecundity;
    }

	if (
        (PbMsg.sim().simulation_mode1()==CIMSIM) && 
        (Date==57)
    ) {
        Daily_Average_Female_Fecundity=0.5*Daily_Average_Female_Fecundity;
    }

	//this is the start of the calculations for female adult cohorts //#19,20,21,22

	// make a set of new egg cohorts, one for each container
	// go through adult cohorts, figure out where each lay they eggs
	// then, later on, add these new egg cohorts to the containers

	std::map<int,Eggs_Cohort> containers_new_eggs_cohort;
	std::map<int,Eggs_Cohort>::iterator itNewEggCohort;

	// all the Ages, Numbers, Developmental_Percent etc of what will become the new egg cohorts are zeroed by the above line

	this->Recaptured_Female = this->Recaptured_Male = 0;

	// need to initialize Medea_Mom array within each possible cohort
	// not sure this is the best way to go, we could probably spare a lot of those ...
	/*for (int cohort=0 ; cohort<Container.size()*the_mask.n_genotypes ; cohort++)
				for (int mom_type=0 ; mom_type<number_of_medea_masks ; mom_type++)
				containers_new_eggs_cohort[cohort].Medea_Mom[mom_type]=0 ;
	 */

    // method?? 
    size_t n_female = Female_Adults.size(); 
    // go through loop once for each female (ii)
    // but list modified in-flight
    // ipos points to current female
	for (size_t ii(0), ipos(0); ii < n_female; ii++) {
        // work from beginning
        auto female_adult_it = this->Female_Adults.begin();
        // ipos = # processed so far but not dead/moved
        std::advance(female_adult_it, ipos);
        //
		double daily_average_female_fecundity_fudge_factor=1;

		int NewlyParousOvipositing = female_adult_it->Female_Adult_Calculations( Number_of_Suitable_Containers, vecWeather[Date]);

		if (NewlyParousOvipositing==-99) //female is dead, calculate generation time
		{
            // remove and continue
			Female_Adults.erase(female_adult_it); 
			continue;
		}

		//if eggs are laid //#23
		if (NewlyParousOvipositing>0) {

			bool trapped=false;

			if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) Daily_Average_Female_Fecundity=female_adult_it->Fecundity;

			// determine a binary mask describing mom's MEDEA elements
			int mom_had_medeas = 0;
			if (PbMsg.geneticcontrol().genetic_control_method()==MEDEA){
				for (int chrom=1 ; chrom<PbMsg.medea().medea_units()+1 ; chrom++)
				{
					int medea_mask=3u<<(2*chrom) ;
					if (female_adult_it->Genotype&medea_mask)
					{
						mom_had_medeas+=(1u<<(chrom-1)) ;
						if ((((female_adult_it->Genotype&medea_mask)>>(2*chrom))%4)==3) // homozygous
                        {
							Daily_Average_Female_Fecundity*=(1-PbMsg.medea().medea_maternal_fecundity_loss())*(1-PbMsg.medea().medea_maternal_fecundity_loss());
						} else { // heterozygous
                            // precompute!!
							Daily_Average_Female_Fecundity*=pow((double)(1-PbMsg.medea().medea_maternal_fecundity_loss()),(2*PbMsg.medea().medea_maternal_fecundity_loss_dominance()));
                        }
					}
				}
            }

			// Wolbachia fecundity cost and CI

			//double non_wolbachia_eggs = 0;
			// because of imperfect vertical transmission, some eggs will not receive wolbachia
			// these WILL be subject to CI even if the mother is infected (based on Turelli & Hoffman, 1999)

			if (PbMsg.wolbachia().number_of_incompatible_wolbachia())
			{
				int number_of_infections=0;
				for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
					if (((female_adult_it->Wolbachia)>>ww)%2)
						number_of_infections++;
				Daily_Average_Female_Fecundity*=pow((double)(1-PbMsg.wolbachia().wolbachia_infected_female_fecundity_loss()),number_of_infections);


				int number_of_incompatibilities = 0;
				for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
					if ((((female_adult_it->Wolbachia)>>ww)%2)==0) // uninfected female
					if (((female_adult_it->Male_Wolbachia)>>ww)%2) // infected male
						number_of_incompatibilities++;
				Daily_Average_Female_Fecundity*=pow((double)(1-PbMsg.wolbachia().wolbachia_level_of_incompatibility()),number_of_incompatibilities);
			}

            // gametes per chromosome
			vector< vector<int> > female_gamete_type(PbMsg.sim().chrom_number(), vector<int>(2)); 
			vector< vector<int> > male_gamete_type(PbMsg.sim().chrom_number(), vector<int>(2)); 
			int offspring_genotype;
			vector< vector<int> > offspring_chrom_genotype(PbMsg.sim().chrom_number(), vector<int>(2)); 
            //offspring genotype individual chromose per Punett square
			//int (*offspring_chrom_genotype)[4]= new int [PbMsg.sim().chrom_number()][4]; 

			if (PbMsg.sim().simulation_mode2()==STOCHASTIC)
			{

				//builds up distribution of available males
				vector<double> offspring_genotype_probabilities(the_mask.n_genotypes, 0);
				vec_uint eggs_with_genotype(the_mask.n_genotypes, 0);

				//applying variation in fecundity for individual females laying eggs
				Standard_Deviation_for_fecundity=Daily_Average_Female_Fecundity*PbMsg.devel().skeeter_buster_fecundity_ratio_standard_deviation_mean();
				//Variance_for_fecundity=Standard_Deviation_for_fecundity*Standard_Deviation_for_fecundity;

				for (int l=0;l<NewlyParousOvipositing;l++)
                /************************
                // change here: this for loop should be unnecessary (check)
                *************************/
				{
					Egg_Number=ROUND_(Normal_Deviate(Daily_Average_Female_Fecundity,Standard_Deviation_for_fecundity));
					if (Egg_Number<0) Egg_Number=0 ;
					vector<double> Egg_Batch_Wolbachia_Type(the_mask.n_wolbachia_status, 0);
					vector<double> Egg_Batch_Wolbachia_Probas(the_mask.n_wolbachia_status, 1.);
					if (PbMsg.wolbachia().number_of_incompatible_wolbachia())
					{
						// calculate here (1) imperfect transmission, then (2) CI
					// (1) imperfect transmission
						// first populate the table with probabilities
						for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
                        {
							for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
							{
								if (((female_adult_it->Wolbachia)>>ww)%2) // female infected
								{
									if ((w>>ww)%2) // corresponds to an infected egg type
                                    {
										Egg_Batch_Wolbachia_Probas[w]*=PbMsg.wolbachia().wolbachia_maternal_transmission();
									} else {
										Egg_Batch_Wolbachia_Probas[w]*=(1-PbMsg.wolbachia().wolbachia_maternal_transmission());
                                    }
								} else { // female non infected
									if ((w>>ww)%2) { 
                                        // corresponds to an infected egg type
										Egg_Batch_Wolbachia_Probas[w]=0.;
                                    }
								}
                            }
                        }
						//fprintf(stdout, "Mom_Type : %d ; Egg_Number : %.3f ; Probas %.3f %.3f\n", female_adult_it->Wolbachia, Egg_Number, Egg_Batch_Wolbachia_Probas[0],Egg_Batch_Wolbachia_Probas[1] );

						// then distribute the egge batch in their respective types

						//genmul(Egg_Number, Egg_Batch_Wolbachia_Probas, the_mask.n_wolbachia_status, Egg_Batch_Wolbachia_Type);

						for (int w=0 ; w<the_mask.n_wolbachia_status ; w++) {
							Egg_Batch_Wolbachia_Type[w] = (int) Egg_Number*(Egg_Batch_Wolbachia_Probas[w]);
                        }

						//fprintf(stdout, "Egg_Number : %.3f ; Probas %.3f %.3f\n", Egg_Number, Egg_Batch_Wolbachia_Type[0],Egg_Batch_Wolbachia_Type[1] );

						// (2) CI applied to different egg types

						for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
                        {
							for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
                            {
								if (((w>>ww)%2)==0) // uninfected egg type
                                {
									if (((female_adult_it->Male_Wolbachia)>>ww)%2) // infected male
                                    {
										//Egg_Batch_Wolbachia_Type[w] = (int) Binomial_Deviate(Egg_Batch_Wolbachia_Type[w], (1-PbMsg.wolbachia().wolbachia_level_of_incompatibility()));
										Egg_Batch_Wolbachia_Type[w]*=(1-PbMsg.wolbachia().wolbachia_level_of_incompatibility());
                                    }
                                }
                            }
                        }

				//	fprintf(stdout, "Egg_Number : %.3f ; Probas %.3f %.3f\n", Egg_Number, Egg_Batch_Wolbachia_Type[0],Egg_Batch_Wolbachia_Type[1] );

						// calculate new number of eggs

                        // 
						Egg_Number = std::accumulate(Egg_Batch_Wolbachia_Type.begin(),Egg_Batch_Wolbachia_Type.end(),0.0);
					}

					if (Egg_Number<0) Egg_Number=0 ; // should be obsolete

					// store this female's oviposition event
					Oviposition this_event;
					this_event.Date = Date;
					this_event.Age_since_hatch = female_adult_it->Age_since_hatch;
					this_event.Number = Egg_Number;
					this_event.House_Position = this->unique_id;
					female_adult_it->Oviposition_Events.push_back(this_event);

					int dx = ((this_event.House_Position)%PbMsg.sim().n_col()) - ((female_adult_it->Emergence_Place)%PbMsg.sim().n_col());
					dx=(dx<0?-dx:dx);
					int dy = ((this_event.House_Position)/PbMsg.sim().n_col()) - ((female_adult_it->Emergence_Place)/PbMsg.sim().n_col());
					dy=(dy<0?-dy:dy);
					//int distance = dx+dy;
					//fprintf(gentime, "%4d\t%4d\t%.0f\t%4d\t%4d\t%3d\t%.0f\n", Date, female_adult_it->ID, this_event.Age_since_hatch, female_adult_it->Emergence_Place, this_event.House_Position, distance, this_event.Number);
					//for (int e=0 ; e < this_event.Number ; e++) fprintf(gentime, "%d\n", distance);

					if (PbMsg.egg().oviposit_eggs_independently()) {
						genmul( (int) Egg_Number , container_probabilities.data(), Container.size() , eggs_laid_in_each.data() );
                    }
					//Multinomial_Deviate( (int) Egg_Number, container_probabilities, Container.size(), eggs_laid_in_each);

					if (!PbMsg.egg().oviposit_eggs_independently()) // If females are to partition their eggs and lay it in one container
                    {
						/* The idea here is eggs are partitioned into some number of partitions. Each partition gets laid in a container as if it were multinomial */
                        // number of partitions varies 
                        // The previous version of this was deeply nested for loops

						int partition_clutches = Binomial_Deviate(1,PbMsg.egg().probability_clutch_partitioned());

						/* if the clutch is going to be partitioned */
						if (partition_clutches > 0) {
							/* This annoying step is required because the previous implementation when eggs are laid independently relied on the multinomial generator to reset eggs_laid_in_each to zero. Because genmul gets called multiple times for each female now, however, we cannot reset the eggs_laid_in_each array values to zero */
                            std::fill(eggs_laid_in_each.begin(), eggs_laid_in_each.end(), 0);
							int number_of_partitions;
							if (Container.size() < PbMsg.egg().maximum_number_of_egg_partitions())
                            {
								number_of_partitions = Container.size();
                            } else {
								number_of_partitions = PbMsg.egg().maximum_number_of_egg_partitions();
                            }
                            // target to fill
							vec_uint eggs_in_partitions (number_of_partitions, 0);
							//printf("No. partitions %d\n", number_of_partitions);
							// fillnig probability per partition
							// size temp prob holder to max nparts = 10
							vector<double> part_probs(10,0.0);
                            // final prob vector
							if (number_of_partitions == 1) {
                                // all eggs go here (w/prob 1.0)
								part_probs[0] = 1.0;
                            } else {
                                // manually fill by name
                                part_probs[0] = PbMsg.egg().proportion_in_partition_0();
                                part_probs[1] = PbMsg.egg().proportion_in_partition_1();
                                part_probs[2] = PbMsg.egg().proportion_in_partition_2();
                                part_probs[3] = PbMsg.egg().proportion_in_partition_3();
                                part_probs[4] = PbMsg.egg().proportion_in_partition_4();
                                part_probs[5] = PbMsg.egg().proportion_in_partition_5();
                                part_probs[6] = PbMsg.egg().proportion_in_partition_6();
                                part_probs[7] = PbMsg.egg().proportion_in_partition_7();
                                part_probs[8] = PbMsg.egg().proportion_in_partition_8();
                                part_probs[9] = PbMsg.egg().proportion_in_partition_9();
                                // done filling, just grab the ones we're using
								part_probs.resize(number_of_partitions);
                                // sum them to next to last
                                double total_prob = std::accumulate(part_probs.begin(), part_probs.end()-1, 0.0);
                                if ( total_prob > 1.0) {
                                    throw std::runtime_error("PbInput PbEgg proportion_in_partition sum to more than 1");
                                } else {
                                    // assign remaining probability to last partition
                                    part_probs[number_of_partitions] = 1.0-total_prob;
                                }
                            }

							genmul(Egg_Number , part_probs.data(), number_of_partitions , eggs_in_partitions.data() );

							for (int egg_partition = 0; egg_partition < number_of_partitions;  egg_partition++)
                            {
								//printf("Eggs in partition %d %d\n", (int) Egg_Number, eggs_in_partitions[egg_partition]);

								vec_uint destination_container(Container.size(),0);

								genmul( 1 , container_probabilities.data(), Container.size(), destination_container.data() );

								for (size_t k=0; k < Container.size(); k++) {
                                    if (destination_container[k] == 1) {
                                        //int eggs_before = eggs_laid_in_each[k];
                                        eggs_laid_in_each[k] += (int) eggs_in_partitions[egg_partition]; // Lay all the eggs in the partition in the container such that eggs_laid_in_each = 1
                                        //printf("Eggs laid in container %d %d %d: %d %d\n", k, this->Container[k].Larvae.size(), eggs_before,  eggs_laid_in_each[k], (int) eggs_in_partitions[egg_partition]);
                                    }
                                }
                            }
							//if(eggs_laid_in_each==NULL) fprintf(stdout, "NULL eggs_laid_in_each\n") ;
                        }

						// If the female will lay her entire clutch in a single container
						if (partition_clutches == 0) {
							genmul( 1 , container_probabilities.data(), Container.size() , eggs_laid_in_each.data() ); // Determine which container gets the eggs
							for (size_t k=0; k < Container.size(); k++) {
								if (eggs_laid_in_each[k] == 1) {
									eggs_laid_in_each[k] = (int) Egg_Number; // Lay all the eggs in the container such that eggs_laid_in_each = 1
                                }
                            }
                        }
                    }/* end if PbMsg.egg().oviposit_eggs_independently() == 0 */

					//for (int k=0 ; k<Container.size() ; k++) eggs_laid_in_each[k]=10;

					// case of oviposition traps

					for (size_t k=0 ; k<Container.size() ; k++)
					{
						//fprintf(stdout, "before: %d (k=%d) - ", eggs_laid_in_each[k], k);
						if ((this->Container[k].Container_Type_Number)==101)
							if((eggs_laid_in_each[k])>0)
							{
								eggs_laid_in_each[k]=0;
								if (Random_Number()<PbMsg.control().ovitrap_female_killing_efficiency())
									trapped=true;
							}
						//fprintf(stdout, "after: %d (k=%d)\n", eggs_laid_in_each[k], k);
					}


					//choosing male
					// int male_genotype = female_adult_it->Sperm_Genotypes[l] ;
					// female_adult_it->Sperm_Genotypes.push_back(male_genotypes[male_cohort_number]) ;

					int male_genotype=female_adult_it->Male_Genotype;
					// tmp
					// fprintf(stdout, "Female with genotype %d mates with male genotype %d\n", female_adult_it->Genotype, male_genotype);

					//this part calculates the possible female and male gametes on each chromosome

					if (PbMsg.geneticcontrol().genetic_control_method()!=MEIOTIC_DRIVE)
					{
                        //?? unhelpful variable names
						int a=1;
						int b=2;

						// we will store female gamete types in the more
						// significant bit  (i.e. in the places occupied by
						// b), so we multiply by 2 when we look at a for female

						// for male, we divide by 2 when we look at b
						//?? does it make sense to use bitshif/bitmask operators here?

						for (int chrom=0;chrom<PbMsg.sim().chrom_number();chrom++)
						{
							female_gamete_type[chrom][0]=(female_adult_it->Genotype & a)*2;
							female_gamete_type[chrom][1]=(female_adult_it->Genotype & b);

							male_gamete_type[chrom][0]=(male_genotype & a);
							male_gamete_type[chrom][1]=(male_genotype & b)/2;

							offspring_chrom_genotype[chrom][0]=female_gamete_type[chrom][0]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][1]=female_gamete_type[chrom][0]+male_gamete_type[chrom][1];
							offspring_chrom_genotype[chrom][2]=female_gamete_type[chrom][1]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][3]=female_gamete_type[chrom][1]+male_gamete_type[chrom][1];

							a=a*4;
							b=b*4;
						}
					} else {
						female_gamete_type[0][0]=0;
						female_gamete_type[0][1]=0;

						male_gamete_type[0][0]=0;
						male_gamete_type[0][1]=1;

						female_gamete_type[1][0]=(female_adult_it->Genotype & 48)/16;
						female_gamete_type[1][1]=(female_adult_it->Genotype & 12)/4;

						male_gamete_type[1][0]=(male_genotype & 48)/16;
						male_gamete_type[1][1]=(male_genotype & 12)/4;

						if (PbMsg.meioticdrive().md_type()==3)
						{
							female_gamete_type[2][0]=(female_adult_it->Genotype & 128)/128;
							female_gamete_type[2][1]=(female_adult_it->Genotype & 64)/64;

							male_gamete_type[2][0]=(male_genotype & 128)/128;
							male_gamete_type[2][1]=(male_genotype & 64)/64;
						}

						offspring_chrom_genotype[0][0]=0;
						offspring_chrom_genotype[0][1]=1;
						offspring_chrom_genotype[0][2]=0;
						offspring_chrom_genotype[0][3]=1;

						offspring_chrom_genotype[1][0]=female_gamete_type[1][0]*4+male_gamete_type[1][0];
						offspring_chrom_genotype[1][1]=female_gamete_type[1][0]*4+male_gamete_type[1][1];
						offspring_chrom_genotype[1][2]=female_gamete_type[1][1]*4+male_gamete_type[1][0];
						offspring_chrom_genotype[1][3]=female_gamete_type[1][1]*4+male_gamete_type[1][1];

						if (PbMsg.meioticdrive().md_type()==3)
						{
							offspring_chrom_genotype[2][0]=female_gamete_type[2][0]*2+male_gamete_type[2][0];
							offspring_chrom_genotype[2][1]=female_gamete_type[2][0]*2+male_gamete_type[2][1];
							offspring_chrom_genotype[2][2]=female_gamete_type[2][1]*2+male_gamete_type[2][0];
							offspring_chrom_genotype[2][3]=female_gamete_type[2][1]*2+male_gamete_type[2][1];
						}

					}

					//calculate offspring genotype probabilities
					if (PbMsg.sim().reproduction_mode()==CALCULATION)
					{

						//this combines the offspring genotypes per chromosome into the actual offspring chromosome
						if (PbMsg.geneticcontrol().genetic_control_method()!=MEIOTIC_DRIVE)
						{
                            // precompute!!
							for (int punett=0;punett<(int) pow((double)4,PbMsg.sim().chrom_number());punett++)
							{

								offspring_genotype=0;

								for (int chrom=0;chrom<PbMsg.sim().chrom_number();chrom++)
								{
									offspring_genotype+=offspring_chrom_genotype[chrom][ (punett & the_mask.mask[chrom])/the_mask.divisor[chrom] ];
								}
								offspring_genotype_probabilities[offspring_genotype]+=the_mask.gen_prob;

							}
							//fprintf(stdout, "P(0) = %.3f - P(1) = %.3f -- P(2) = %.3f\n", offspring_genotype_probabilities[0], offspring_genotype_probabilities[1], offspring_genotype_probabilities[2]);
						} else {
							for (int punett1=0;punett1<3;punett1+=2)
							{
								for (int punett2=0;punett2<3;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (PbMsg.meioticdrive().md_type()==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=PbMsg.meioticdrive().drive_strength_sn();
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=PbMsg.meioticdrive().drive_strength_it();
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=PbMsg.meioticdrive().drive_strength_in();
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=PbMsg.meioticdrive().drive_strength_mn();
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_probabilities[offspring_genotype]+=0.125*0.25;
									}
								}
							}
							for (int punett1=1;punett1<4;punett1+=2)
							{
								for (int punett2=1;punett2<4;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (PbMsg.meioticdrive().md_type()==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=PbMsg.meioticdrive().drive_strength_sn();
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=PbMsg.meioticdrive().drive_strength_it();
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=PbMsg.meioticdrive().drive_strength_in();
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=PbMsg.meioticdrive().drive_strength_mn();
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_probabilities[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_probabilities[offspring_genotype]+=0.125*0.25;
									}
								}
							}
						}

						//probabilities adjustments in case of embryonic fitness costs with soft selection

						if ((PbMsg.geneticcontrol().fitness_cost_stage()==0)&&(PbMsg.geneticcontrol().embryonic_cost_selection_type()==SOFT))
						{

							double total_probabilities=0.;
							// modify probabilities according to each genotype specific fitness
							for (int genotype=0 ; genotype<the_mask.n_genotypes ; genotype++)
							{
								offspring_genotype_probabilities[genotype]*=Fitness_Calculation(genotype);
								total_probabilities+=offspring_genotype_probabilities[genotype];
							}

							// renormalize probabilities
							for (int genotype=0 ; genotype<the_mask.n_genotypes ; genotype++)
								offspring_genotype_probabilities[genotype]/=total_probabilities;

						}
						for (size_t k=0;k<(Container.size()); k++)
						{
							// changes here for individualized egg cohorts by container, by genotype AND by mom genotype
							// -> genmul stays, but either (1) change the structure of containers_new_eggs_cohort
							//    or (2) change the procedure, and push_back the cohorts somewhere here (layers?)
							//    (probably .2.?)
							genmul ( eggs_laid_in_each[k], offspring_genotype_probabilities.data(), the_mask.n_genotypes, eggs_with_genotype.data());
							//							Multinomial_Deviate ( eggs_laid_in_each[k], offspring_genotype_probabilities, the_mask.n_genotypes, eggs_with_genotype);

                            // !!CLEANUP bracing
							for (int genotype=0;genotype<the_mask.n_genotypes;genotype++)
                            {
								if ((eggs_with_genotype[genotype])>0)
								{
									int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*k+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_had_medeas+female_adult_it->Wolbachia;
									//fprintf(stdout, "Genotype %d -- Index %d\n", genotype, index);

									if (PbMsg.wolbachia().number_of_incompatible_wolbachia())
									{
										vec_uint eggs_with_wolbachia_type(the_mask.n_wolbachia_status, 0);
										genmul( eggs_with_genotype[genotype], Egg_Batch_Wolbachia_Type.data(), the_mask.n_wolbachia_status, eggs_with_wolbachia_type.data());

										for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
										{
											index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*k+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_had_medeas+w;
											itNewEggCohort=containers_new_eggs_cohort.find(index);
											if (itNewEggCohort==containers_new_eggs_cohort.end())
											{
												itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
											}
											containers_new_eggs_cohort[index].Number += eggs_with_wolbachia_type[w];
											containers_new_eggs_cohort[index].Maternal_ID = female_adult_it->ID;
										}
										sumeggs_laid += eggs_with_genotype[genotype];
									} else {
										itNewEggCohort=containers_new_eggs_cohort.find(index);
										//fprintf(stdout, "Search index %d ... ", index);
										if (itNewEggCohort==containers_new_eggs_cohort.end())
										{
											itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
											//fprintf(stdout, "Not found, inserted ... \n");
										}
										containers_new_eggs_cohort[index].Number += eggs_with_genotype[genotype];
										containers_new_eggs_cohort[index].Maternal_ID = female_adult_it->ID;
										sumeggs_laid += eggs_with_genotype[genotype];
									}
								}
                            }
						}
					}

					if (PbMsg.sim().reproduction_mode()==SAMPLING)
					{
						vector<double> Egg_Batch_Wolbachia_Cumul(the_mask.n_wolbachia_status);
                        // cummulative sum
                        partial_sum(Egg_Batch_Wolbachia_Type.begin(), Egg_Batch_Wolbachia_Type.end(), Egg_Batch_Wolbachia_Cumul.begin());

						for (size_t container=0;container<Container.size();container++)
						{
							for (size_t egg=0;egg<eggs_laid_in_each[container];egg++)
							{

								if (PbMsg.geneticcontrol().genetic_control_method()==MEIOTIC_DRIVE)
								{
									double chromatid = Random_Number();

									if (chromatid<0.5) {
										if (chromatid<0.25) {
											offspring_genotype=offspring_chrom_genotype[1][0]*4;
										} else {
											offspring_genotype=offspring_chrom_genotype[1][1]*4+1;
										}
									} else {
										if (chromatid<0.75) {
											offspring_genotype=offspring_chrom_genotype[1][2]*4;
										} else {
											offspring_genotype=offspring_chrom_genotype[1][3]*4+1;
										}
									}
								} else {
									int a,b;
									a=2;
									b=1;
									offspring_genotype=0;

									// the idea here is that a and b hold the binary bit locations of the different alleles, so if we take X & a (binary and) we will pull off one, and X & b will pull off the other.
									// the only issue is that we will store the allele from the mother in positions corresponding to 2,8,32,128,... and those from the father in positions corresponding to 1,4,16,64,...
									// So we need to multiply by 2 when we pick the female off using &b and divide by two when we pick the male off using &a.

									for (int chrom=0;chrom<PbMsg.sim().chrom_number();chrom++)
									{
										double chromatid = Random_Number();

										if (chromatid<0.5) {
											if (chromatid<0.25) {
												offspring_genotype+=(female_adult_it->Genotype & a)+(male_genotype & a)/2;
											} else {
												offspring_genotype+=(female_adult_it->Genotype & a)+(male_genotype & b);
											}
										} else {
											if (chromatid<0.75) {
												offspring_genotype+=(female_adult_it->Genotype & b)*2+(male_genotype & a)/2;
											} else {
												offspring_genotype+=(female_adult_it->Genotype & b)*2+(male_genotype & b);
											}

										}
                                        //!! this is crazy
										a=a*4;
										b=b*4; // multiplying by 4 moves us to the next locus
									}
								}

								int index;
								if (PbMsg.wolbachia().number_of_incompatible_wolbachia())
								{
									double r=Egg_Number*Random_Number();
									int w=0;
									while (Egg_Batch_Wolbachia_Cumul[w++]<r);

									index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*container+the_mask.n_wolbachia_status*the_mask.n_medea_masks*offspring_genotype+the_mask.n_wolbachia_status*mom_had_medeas+(w-1);

								}
								else index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*container+the_mask.n_wolbachia_status*the_mask.n_medea_masks*offspring_genotype+the_mask.n_wolbachia_status*mom_had_medeas+female_adult_it->Wolbachia;

								itNewEggCohort=containers_new_eggs_cohort.find(index);
								if (itNewEggCohort==containers_new_eggs_cohort.end())
								{
									itNewEggCohort=containers_new_eggs_cohort.insert(itNewEggCohort, pair<int,Eggs_Cohort>(index,Eggs_Cohort()));
								}
								containers_new_eggs_cohort[index].Number++;
								containers_new_eggs_cohort[index].Maternal_ID = female_adult_it->ID;
								sumeggs_laid++;

							}
						}
					}
				}
			}

			//deterministic implementation of the egg laying for female adult cohorts
			if (PbMsg.sim().simulation_mode2()==DETERMINISTIC)
			{
				vector<double> offspring_genotype_fraction(the_mask.n_genotypes, 0);
				for (size_t l=0;l<this->Container.size();l++)
				{
					for (int male_genotype=0;male_genotype<the_mask.n_genotypes;male_genotype++)
					{
						for (int genotype=0;genotype<the_mask.n_genotypes;genotype++){
                            offspring_genotype_fraction[genotype]=0;
                        }
						int a=1;
						int b=2;

						// we will store female gamete types in the more
						// significant bit  (i.e. in the places occupied by
						// b), so we multiply by 2 when we look at a for female

						// for male, we divide by 2 when we look at b

						for (int chrom=0;chrom<PbMsg.sim().chrom_number();chrom++)
						{
							female_gamete_type[chrom][0]=(female_adult_it->Genotype & a)*2;
							female_gamete_type[chrom][1]=(female_adult_it->Genotype & b);

							male_gamete_type[chrom][0]=(male_genotype & a);
							male_gamete_type[chrom][1]=(male_genotype & b)/2;

							offspring_chrom_genotype[chrom][0]=female_gamete_type[chrom][0]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][1]=female_gamete_type[chrom][0]+male_gamete_type[chrom][1];
							offspring_chrom_genotype[chrom][2]=female_gamete_type[chrom][1]+male_gamete_type[chrom][0];
							offspring_chrom_genotype[chrom][3]=female_gamete_type[chrom][1]+male_gamete_type[chrom][1];

							a=a*4;
							b=b*4;
						}

						if (PbMsg.geneticcontrol().genetic_control_method()==MEIOTIC_DRIVE)
						{
							female_gamete_type[0][0]=0;
							female_gamete_type[0][1]=0;

							male_gamete_type[0][0]=0;
							male_gamete_type[0][1]=1;

							female_gamete_type[1][0]=(female_adult_it->Genotype & 48)/16;
							female_gamete_type[1][1]=(female_adult_it->Genotype & 12)/4;

							male_gamete_type[1][0]=(male_genotype & 48)/16;
							male_gamete_type[1][1]=(male_genotype & 12)/4;

							if (PbMsg.meioticdrive().md_type()==3)
							{
								female_gamete_type[2][0]=(female_adult_it->Genotype & 128)/128;
								female_gamete_type[2][1]=(female_adult_it->Genotype & 64)/64;

								male_gamete_type[2][0]=(male_genotype & 128)/128;
								male_gamete_type[2][1]=(male_genotype & 64)/64;
							}

							//for (int i=0;i<3;i++) printf("%d %d\n",female_gamete_type[i][0],female_gamete_type[i][1]);
							//for (int i=0;i<3;i++) printf("%d %d\n",male_gamete_type[i][0],male_gamete_type[i][1]);

							offspring_chrom_genotype[0][0]=0;
							offspring_chrom_genotype[0][1]=1;
							offspring_chrom_genotype[0][2]=0;
							offspring_chrom_genotype[0][3]=1;

							offspring_chrom_genotype[1][0]=female_gamete_type[1][0]*4+male_gamete_type[1][0];
							offspring_chrom_genotype[1][1]=female_gamete_type[1][0]*4+male_gamete_type[1][1];
							offspring_chrom_genotype[1][2]=female_gamete_type[1][1]*4+male_gamete_type[1][0];
							offspring_chrom_genotype[1][3]=female_gamete_type[1][1]*4+male_gamete_type[1][1];

							if (PbMsg.meioticdrive().md_type()==3)
							{
								offspring_chrom_genotype[2][0]=female_gamete_type[2][0]*2+male_gamete_type[2][0];
								offspring_chrom_genotype[2][1]=female_gamete_type[2][0]*2+male_gamete_type[2][1];
								offspring_chrom_genotype[2][2]=female_gamete_type[2][1]*2+male_gamete_type[2][0];
								offspring_chrom_genotype[2][3]=female_gamete_type[2][1]*2+male_gamete_type[2][1];
							}
							//for (int i=0;i<3;i++) printf("%d %d %d %d\n",offspring_chrom_genotype[i][0],offspring_chrom_genotype[i][1],offspring_chrom_genotype[i][2],offspring_chrom_genotype[i][3]);
						}

						//this combines the offspring genotypes per chromosome into the actual offspring chromosome
						if (PbMsg.geneticcontrol().genetic_control_method()==MEIOTIC_DRIVE)
						{
							for (int punett1=0;punett1<3;punett1+=2)
							{
								for (int punett2=0;punett2<3;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (PbMsg.meioticdrive().md_type()==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=PbMsg.meioticdrive().drive_strength_sn();
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=PbMsg.meioticdrive().drive_strength_it();
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=PbMsg.meioticdrive().drive_strength_in();
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=PbMsg.meioticdrive().drive_strength_mn();
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5-drive_strength)*0.25;
											else offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_fraction[offspring_genotype]+=0.125*0.25;
									}
								}
							}
							for (int punett1=1;punett1<4;punett1+=2)
							{
								for (int punett2=1;punett2<4;punett2+=2)
								{
									for (int punett3=0;punett3<4;punett3++)
									{
										offspring_genotype=offspring_chrom_genotype[0][punett1]+offspring_chrom_genotype[1][punett2]*4;
										if (PbMsg.meioticdrive().md_type()==3) offspring_genotype+=offspring_chrom_genotype[2][punett3]*64;
										double drive_strength=-10;
										if (male_genotype==5) drive_strength=PbMsg.meioticdrive().drive_strength_sn();
										if ((male_genotype==21)||(male_genotype==149)||(male_genotype==85)||(male_genotype==113)) drive_strength=PbMsg.meioticdrive().drive_strength_it();
										if ((male_genotype==53)||(male_genotype==181)||(male_genotype==117)||(male_genotype==245)) drive_strength=PbMsg.meioticdrive().drive_strength_in();
										if ((male_genotype==197)||(male_genotype==133)||(male_genotype==69)) drive_strength=PbMsg.meioticdrive().drive_strength_mn();
										if (drive_strength!=-10)
										{
											if ((offspring_genotype & 1)==0) offspring_genotype_fraction[offspring_genotype]+=0.5*(0.5-drive_strength)*0.25;
											else offspring_genotype_fraction[offspring_genotype]+=0.25*(0.5+drive_strength)*0.25;
										}
										else offspring_genotype_fraction[offspring_genotype]+=0.125*0.25;
									}
								}
							}
						}
						if (PbMsg.geneticcontrol().genetic_control_method()!=MEIOTIC_DRIVE)
						{
							for (int punett=0;punett<(int) pow((double)4,PbMsg.sim().chrom_number());punett++)
							{
								offspring_genotype=0;

								for (int chrom=0;chrom<PbMsg.sim().chrom_number();chrom++)
								{
									offspring_genotype+=offspring_chrom_genotype[chrom][ (punett & the_mask.mask[chrom])/the_mask.divisor[chrom] ];
								}
								offspring_genotype_fraction[offspring_genotype]+=the_mask.gen_prob;

							}
						}
						for (int genotype=0;genotype<the_mask.n_genotypes;genotype++)
						{
							double neweggs = NewlyParousOvipositing*Daily_Average_Female_Fecundity*daily_average_female_fecundity_fudge_factor*container_probabilities[l]*offspring_genotype_fraction[genotype];
							//double neweggs=0 ;
							int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*l+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_had_medeas ;

							containers_new_eggs_cohort[index].Number += neweggs ;
							sumeggs_laid += neweggs ;
						}
					}
				}
			}
			// IF WE HAVE A NULLIPAROUS COHORT, transfer ovipositing adult females into a new adult female cohort, mark it parous //#24

			if (trapped) {
				Female_Adults.erase(female_adult_it);
				continue;
			}
			if (female_adult_it->Nulliparous==NULLIPAROUS)
			{
				female_adult_it->Nulliparous=PAROUS;
				this->TotalNewlyParous++; //increase the total number of newly parous females
			}
		} //end of the part that runs if eggs are laid

		// mating
		if ((female_adult_it->Mated==false)
				||
				(
						(!(female_adult_it->Nulliparous))
						&&
						(Random_Number()<PbMsg.devel().daily_remating_probability_parous_females())
				))
			// assumes monogamy, change here if necessary
		{
			// now
            int tmpsize = Male_Adult.size();
			vector<double> male_mating_probabilities(tmpsize,0.); // An array of size Male_Adult.size(), i.e., the number of male adult cohorts in the house
			vector<int> male_genotypes(tmpsize,0);
			vector<int> male_wolbachia(tmpsize,0);
			//int *males_chosen=new int[Male_Adult.size()];
			double cum_sum_probability=0;
			int k=0;

			for (auto & the_male : Male_Adult) {
				double mating_probability=the_male.Number*the_male.Average_Adult_Weight;
				// for MEDEA, make that more general
				if (the_male.Genotype>1)
				{
					int maskcopies = ((the_male.Genotype)&12)>>2;
					if (maskcopies==3) {
                        // precompute!!
						mating_probability*=pow((double)PbMsg.control().reduction_in_transgenic_males_mating_competitiveness(), 2.);
					} else {
                        // precompute!!
						mating_probability*=pow((double)PbMsg.control().reduction_in_transgenic_males_mating_competitiveness(), 2*PbMsg.control().reduction_in_transgenic_males_mating_competitiveness_dominance());
                    }

				}
				if (the_male.Age>1)
				{
					male_mating_probabilities[k]=mating_probability+cum_sum_probability;
					male_genotypes[k]=the_male.Genotype;
					male_wolbachia[k]=the_male.Wolbachia;
					cum_sum_probability+=mating_probability;
				} else {
					male_mating_probabilities[k]=cum_sum_probability;
				}
				k++; // k is the number of male adult cohorts in the house
			}

			if (cum_sum_probability>0) {

                // normalize
				for (auto & the_prob : male_mating_probabilities) {
                    the_prob /= cum_sum_probability;
                }
				// transform into cumulative probabilities
				// more convenient for stochastic simulations
				// (but less for deterministic runs)
				double r=Random_Number();
				int s=0;

				while (male_mating_probabilities[s++]<r); // goes through male adult cohorts and stops at first s such that male_mating_probabilities[s] >= r

				female_adult_it->Male_Genotype = male_genotypes[s-1];
				female_adult_it->Male_Wolbachia = male_wolbachia[s-1];
				female_adult_it->Mated=true ;
			}
		}
		//mark-release-recapture
		if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
			if (PbMsg.sim().mark_release_recapture()) {
				if (this->Recapture) {
					size_t new_fem = Binomial_Deviate(1,PbMsg.sim().female_daily_recapture_rate());
					this->Recaptured_Female += new_fem;
					// female_adult_it->Number-=Recaptured_Female;
				}
            }
        }
		//dispersal
		if (PbMsg.sim().simulation_mode2()==STOCHASTIC){
            bool has_moved = FemaleDispersal(female_adult_it, housevec);
            if (has_moved) continue;
            has_moved = LRFemaleDispersal(female_adult_it, housevec);
            if (has_moved) continue; 
        }

		/***********************************************************************/
        // neither died nor moved, advance position counter
        ipos++;
	}
	//this is the end of the adult female cohort calculations
	
	//if(eggs_laid_in_each==NULL) fprintf(stdout, "NULL eggs_laid_in_each\n") ;

	//this part sets the properties of the new egg cohorts //#25

	//double sumeggs_laid=0;
	//for (int egg_group=0; egg_group<(the_mask.n_genotypes*this->Container.size()); egg_group++) sumeggs_laid+=containers_new_eggs_cohort[egg_group].Number;

    // method??
    PlaceEggs(sumeggs_laid, containers_new_eggs_cohort);
    // reporting
    Totals.EggsLaid += sumeggs_laid;

	//log the properties of female adult cohorts into the detailed log file
//+++++++++
	if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) {
		for (const auto & the_fem : this->Female_Adults) {
			fprintf(logs.Output,"%s %d %s %d %s %zu %s %.3g %s %.8g %s %.8g %s %8g %s %d\n","Female adult #",-999," nulliparous ",the_fem.Nulliparous," Age: ",the_fem.Age," Age since hatch: ",the_fem.Age_since_hatch, " Phys. Dev.: ",the_fem.Physiological_Development_Percent," Ave. Weight: ",the_fem.Average_Adult_Weight, " Fecundity: ", the_fem.Fecundity," Genotype: ", the_fem.Genotype);
		}
    }

	/* OMP PARALLEL CHECK OK */
	//this is the calculation for male adult cohorts //#26,#27

    // method??
	for (auto male_adult_it=this->Male_Adult.begin(); male_adult_it != this->Male_Adult.end(); ) // do male_adult_it++ at end of loop
	{
		male_adult_it->Male_Adult_Calculations(vecWeather[Date]);

		//mark-release-recapture
		if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
			if (PbMsg.sim().mark_release_recapture()) {
				if (this->Recapture) {
					if (male_adult_it->Number>0)
					{
						size_t new_male = Binomial_Deviate(male_adult_it->Number,PbMsg.sim().male_daily_recapture_rate());
						if (new_male > male_adult_it->Number) {
                             new_male = male_adult_it->Number;
                        }
						this->Recaptured_Male += new_male;
						male_adult_it->Number -= new_male;
					}
                }
            }
        }

		if (PbMsg.sim().simulation_mode2()==STOCHASTIC) {
            MaleDispersal(male_adult_it, housevec);
            LRMaleDispersal(male_adult_it, housevec);
        }

		/***********************************************************************/

		if (male_adult_it->Number==0) {
			male_adult_it=this->Male_Adult.erase(male_adult_it);
            continue; // redundant, for readability
		} else {
			if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end()))
			{
				fprintf(logs.Output,"%s %d %s %zu %s %zu %s %zu %s %.8g %s %d\n","Male adult cohort #",-999," Number: ",male_adult_it->Number," Age: ",male_adult_it->Age," Age since hatch: ",male_adult_it->Age_since_hatch," Ave. Weight: ",male_adult_it->Average_Adult_Weight," Genotype: ",male_adult_it->Genotype);
			}
			male_adult_it++;
		}
	}
	return;
};

void Building::BuildingSummation(
    outLogs & logs,  size_t Date, 
    TotalStruct & Totals
) {
	//this part makes a final tally at the end of the day of the number of all mosquitoes in all life-stages //#41

    vector<double> wolbachia_frequency(the_mask.n_wolbachia_status,0);
    vector<double> wolbachia_prevalence(PbMsg.wolbachia().number_of_incompatible_wolbachia(),0);

    // zero out counts
	this->TotalEggs=this->TotalFemalePupae=this->TotalMalePupae=this->TotalFemaleNulliparousAdults=this->TotalFemaleParousAdults=this->TotalFemaleAdults=this->TotalMaleAdults=0;
    this->TotalLarvae[MALE].fill(0);
    this->TotalLarvae[FEMALE].fill(0);
    // reporting members (set in BuildingSummation)
    this->SumLarva = 0;
    this->PosCont = 0;

	vector<double> Total_Genotype_per_House(the_mask.n_genotypes, 0);
	vector<double> Total_Adult_Genotype_per_House(the_mask.n_genotypes, 0);
 
	size_t New_Pupae_in_Container=0;

    // first index is category, e.g. water level, second is container type
    // sum up and find mean by category / type
	for (auto & the_cont : this->Container) {
        this->PosCont += the_cont.is_positive();
		size_t thisctype = the_cont.Container_Type_Number ;
		Totals.Census_by_container_type[0][thisctype]++ ;
		Totals.Census_by_container_type[6][thisctype]+=the_cont.Water_Level;
		Totals.Census_by_container_type[7][thisctype]+=the_cont.Maximum_Daily_Water_Temperature;
		Totals.Census_by_container_type[8][thisctype]+=the_cont.Minimum_Daily_Water_Temperature;
		Totals.Census_by_container_type[9][thisctype]+=the_cont.Average_Daily_Water_Temperature;

		Totals.Census_by_container_type[10][thisctype]+=the_cont.Food;

		double Total_Eggs_in_Container=0;
		for (const auto & the_eggs : the_cont.Eggs) {
			this->TotalEggs+=the_eggs.Number;
			Total_Genotype_per_House[the_eggs.Genotype]+=the_eggs.Number;
			Total_Eggs_in_Container+=the_eggs.Number ;
			wolbachia_frequency[the_eggs.Wolbachia]+=the_eggs.Number;
		}
		Totals.Census_by_container_type[1][thisctype]+=Total_Eggs_in_Container;

		double Total_Larvae_in_Container=0;
		for (const auto & the_larva : the_cont.Larvae) {
			int sex=the_larva.Sex;
			this->TotalLarvae[sex][the_larva.Instar -1] += the_larva.Number;
			this->SumLarva += the_larva.Number;
			Total_Genotype_per_House[the_larva.Genotype]+=the_larva.Number;
			Total_Larvae_in_Container+=the_larva.Number ;
			wolbachia_frequency[the_larva.Wolbachia]+=the_larva.Number;
		}
		Totals.Census_by_container_type[2][thisctype]+=Total_Larvae_in_Container;

		double Total_Pupae_in_Container=0;
		for (const auto & the_pupa : the_cont.Pupae){
			if (the_pupa.Sex==FEMALE) {
				this->TotalFemalePupae+=the_pupa.Number;
            } else {
				this->TotalMalePupae+=the_pupa.Number;
            }
			Total_Genotype_per_House[the_pupa.Genotype]+=the_pupa.Number;
			Total_Pupae_in_Container+=the_pupa.Number ;
			if (the_pupa.Age==1) {
                 New_Pupae_in_Container+=the_pupa.Number;
            }
			wolbachia_frequency[the_pupa.Wolbachia]+=the_pupa.Number;
			if (the_pupa.Number) {
                 Totals.Census_by_container_type[5][thisctype]+=the_pupa.Number*the_pupa.Average_Pupal_Weight ;
            }
		}

		Totals.Census_by_container_type[3][thisctype]+=Total_Pupae_in_Container;
		if (Total_Pupae_in_Container)
		{
			Totals.Census_by_container_type[4][thisctype]++ ;
			Totals.Census_by_container_type[5][thisctype]/=Total_Pupae_in_Container ;
		}
	}

    // not used??
	//Totals.cumul_pupae_per_house[this->unique_id] += (int)New_Pupae_in_Container;

	Totals.Eggs+=this->TotalEggs;
	for (int instar=0 ; instar<N_INSTAR ; instar++) {
        // MALE/FEMALE??
		Totals.Female_Larvae+=this->TotalLarvae[FEMALE][instar];
		Totals.Male_Larvae+=this->TotalLarvae[MALE][instar];
	}
	Totals.Female_Pupae+=this->TotalFemalePupae;
	Totals.Male_Pupae+=this->TotalMalePupae;

    TotalFemaleAdults = this->Female_Adults.size();
	for (const auto & the_fem : this->Female_Adults) {
		if (the_fem.Nulliparous==NULLIPAROUS)
		{
			this->TotalFemaleNulliparousAdults++;
			Total_Genotype_per_House[the_fem.Genotype]++;
			Total_Adult_Genotype_per_House[the_fem.Genotype]++;

		} else {
			this->TotalFemaleParousAdults++;
			Total_Genotype_per_House[the_fem.Genotype]++;
			Total_Adult_Genotype_per_House[the_fem.Genotype]++;
		}
		wolbachia_frequency[the_fem.Wolbachia]++;
	}

	for (const auto & the_male : this->Male_Adult) {
		this->TotalMaleAdults+=the_male.Number;
		Total_Genotype_per_House[the_male.Genotype]+=the_male.Number;
		Total_Adult_Genotype_per_House[the_male.Genotype]+=the_male.Number;
		wolbachia_frequency[the_male.Wolbachia]+=the_male.Number;
	}

	Totals.Nulliparous_Female_Adults+=this->TotalFemaleNulliparousAdults;
	Totals.Parous_Female_Adults+=this->TotalFemaleParousAdults;
	Totals.Male_Adults+=this->TotalMaleAdults;
	for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) {
        Totals.Genotypes[genotype]+=Total_Genotype_per_House[genotype];
        Totals.Adult_Genotypes[genotype]+=Total_Adult_Genotype_per_House[genotype];
    }

	vector<double> allele_frequency_per_House(2*PbMsg.sim().chrom_number(), 0);

	//for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++)
		// // steps are 2, another allele++ at the end of the loop
	//{
	// //int mask1 = ((int) pow((double)2,2*PbMsg.sim().chrom_number()-1-allele)) ;
	// //int mask2 = ((int) pow((double)2,2*PbMsg.sim().chrom_number()-2-allele)) ; // 1 mask per homologous chromosome
	// //for (int genotype=0;genotype<the_mask.n_genotypes;genotype++)
	// //{
	// //if (((genotype/mask1)%2)==0) allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype]; else allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
	// //if (((genotype/mask2)%2)==0) allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype]; else allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
	// //}
	// //allele++;
	//}

	// above version very time consuming
	// new version:

	for (int genotype=0 ; genotype<the_mask.n_genotypes ; genotype++) {
		if (Total_Genotype_per_House[genotype])
			// advantage for looping through genotypes first: skip absent genotypes
		{
			int bit_position = 0;
            //?? relation to extern int *mask?
			int mask=(1u<<(2*PbMsg.sim().chrom_number()-1));

			do {
				int chrom = bit_position/2;
				int allele = 2*chrom;
				if (genotype&mask) allele_frequency_per_House[allele+1]+=Total_Genotype_per_House[genotype];
				else allele_frequency_per_House[allele]+=Total_Genotype_per_House[genotype];

				mask>>=1;
				bit_position++;
			} while (mask>0);
		}
    }

	for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) {
         Totals.Total_Mosquitoes+=Total_Genotype_per_House[genotype];
    }
    Totals.has_insects = (Totals.Total_Mosquitoes > 0);
	for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++)
	{
		if (Totals.has_insects) {
			allele_frequency_per_House[allele]/=2*Totals.Total_Mosquitoes;
		} else if (allele_frequency_per_House[allele]!=0.)
		{
			fprintf(stdout, "*** Warning: non-zero allele frequency in empty house\n");
			fprintf(stdout, "*** House: %zu - Allele %d on chromosome %d\n", unique_id, allele%2, allele/2);
			fprintf(stdout, "*** Frequency set to zero - Continuing ...\n");

			allele_frequency_per_House[allele]=0.;
		}
        // not used?
		//Totals.allele_frequencies[allele][unique_id]=allele_frequency_per_House[allele];
	}

	for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
	{
		Totals.wolbachia_frequency[w]+=wolbachia_frequency[w];
		if (Totals.has_insects) {
             wolbachia_frequency[w]/=Totals.Total_Mosquitoes;
        } else  {
            wolbachia_frequency[w]=0.;
        }
	}
	for (int ww=0 ; ww<PbMsg.wolbachia().number_of_incompatible_wolbachia() ; ww++)
    {
		for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
        {
			if ((w>>ww)%2) {
				wolbachia_prevalence[ww]+=wolbachia_frequency[w];
            }
        }
    }

	double Total_Number_of_Adult_Females=0;
	double Average_Female_Weight=0;
	for (const auto & the_fem : this->Female_Adults) {
		Total_Number_of_Adult_Females++;
		Average_Female_Weight+=the_fem.Average_Adult_Weight;
	}
	Average_Female_Weight/=Total_Number_of_Adult_Females;

	double Total_Number_of_Adult_Males=0;
	double Average_Male_Weight=0;
	for (const auto & the_male : this->Male_Adult) {
		Total_Number_of_Adult_Males+=the_male.Number;
		Average_Male_Weight+=the_male.Average_Adult_Weight*the_male.Number;
	}
	Average_Male_Weight/=Total_Number_of_Adult_Males;

	Totals.CV_Pupae += (this->TotalMalePupae+this->TotalFemalePupae)*(this->TotalMalePupae+this->TotalFemalePupae) ;

    // sum larvae for reporting
    // convenience reference
    size_t totalFemLarv = 0;
    size_t totalMaleLarv = 0;
    for (int ii = 0; ii < 4; ii++) {
        totalFemLarv += this->TotalLarvae[FEMALE][ii];
        totalMaleLarv += this->TotalLarvae[MALE][ii];
    }

	if (PbMsg.geneticcontrol().genetic_control_method() == ReduceReplace)
    {
// What is the number of females that don't carry the replacement allele?
        if (PbMsg.sim().chrom_number()==3)
        {
              // 16*0:3  
            Totals.CV_WILD_FEMALES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48])*(Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48]);
        }
        if (PbMsg.sim().chrom_number()==5) /* two gene case - revision Apr. 7, 2012 */
        {
            Totals.CV_WILD_FEMALES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[64]+Total_Adult_Genotype_per_House[128]+Total_Adult_Genotype_per_House[192] + Total_Adult_Genotype_per_House[256] + Total_Adult_Genotype_per_House[320] + Total_Adult_Genotype_per_House[384]+Total_Adult_Genotype_per_House[448] + Total_Adult_Genotype_per_House[576] + Total_Adult_Genotype_per_House[512] + Total_Adult_Genotype_per_House[640] + Total_Adult_Genotype_per_House[704] + Total_Adult_Genotype_per_House[832] + Total_Adult_Genotype_per_House[768] + Total_Adult_Genotype_per_House[896] + Total_Adult_Genotype_per_House[960]);
        }
// What is the number of females that carry at least one wild allele? - don't measure for more than two genes as cases get astronomical
        Totals.CV_WILD_ALLELES += (Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48] + Total_Adult_Genotype_per_House[20]+Total_Adult_Genotype_per_House[4]+Total_Adult_Genotype_per_House[36]+Total_Adult_Genotype_per_House[52] + Total_Adult_Genotype_per_House[24]+Total_Adult_Genotype_per_House[8]+Total_Adult_Genotype_per_House[40]+Total_Adult_Genotype_per_House[56]);
// Have the wild-type females gone extinct in this house?
        if (PbMsg.sim().chrom_number()==3)
        {
            if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48])==0)
            {
                Totals.NO_WILD_FEMALES++;
            }
        }
        if (PbMsg.sim().chrom_number()==5) /* two gene case - revision Apr. 7, 2012 */
        {
            if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[64]+Total_Adult_Genotype_per_House[128]+Total_Adult_Genotype_per_House[192] + Total_Adult_Genotype_per_House[256] + Total_Adult_Genotype_per_House[320] + Total_Adult_Genotype_per_House[384]+Total_Adult_Genotype_per_House[448] + Total_Adult_Genotype_per_House[576] + Total_Adult_Genotype_per_House[512] + Total_Adult_Genotype_per_House[640] + Total_Adult_Genotype_per_House[704] + Total_Adult_Genotype_per_House[832] + Total_Adult_Genotype_per_House[768] + Total_Adult_Genotype_per_House[896] + Total_Adult_Genotype_per_House[960])==0)
            {
                Totals.NO_WILD_FEMALES++;
            }
        }
        // Has the wild-type allele among adult females gone extinct in this house? - don't measure for more than two genes as cases get astronomical
        if ((Total_Adult_Genotype_per_House[0]+Total_Adult_Genotype_per_House[16]+Total_Adult_Genotype_per_House[32]+Total_Adult_Genotype_per_House[48] + Total_Adult_Genotype_per_House[20]+Total_Adult_Genotype_per_House[4]+Total_Adult_Genotype_per_House[36]+Total_Adult_Genotype_per_House[52] + Total_Adult_Genotype_per_House[24]+Total_Adult_Genotype_per_House[8]+Total_Adult_Genotype_per_House[40]+Total_Adult_Genotype_per_House[56])==0)
        {
            Totals.NO_WILD_ALLELES++;
        }
    }
	// test larvae age structure
	/*for (int k=0 ; k<this->Container.size() ; k++)
			{
			vector<int> larvae_by_age;
			int maximum_age=0;
			for (list<Larvae_Cohort>::iterator larvae_cohort_it=this->Container[k].Larvae.begin(); larvae_cohort_it != this->Container[k].Larvae.end() ; larvae_cohort_it++)
			{
			if (larvae_cohort_it->Age > maximum_age)
			{
			maximum_age = larvae_cohort_it->Age;
			larvae_by_age.resize(maximum_age,0);
			}
			larvae_by_age[larvae_cohort_it->Age - 1]+=(int)larvae_cohort_it->Number;

			}
			int age=0;
			for (vector<int>::iterator this_age=larvae_by_age.begin() ; this_age!=larvae_by_age.end() ; this_age++)
			{
			age++;
			fprintf(test2, "%d %d %d %d %d\n", Date, unique_id, k, age, *this_age);
			}
			fprintf(test2, "\n");
			}
	 */

//@@ oldchunk 007
	//log the final tally into the detailed log file
	if ((PbMsg.output().do_detailed_log())&&(Date>=PbMsg.output().detailed_log_start())&&(Date<=PbMsg.output().detailed_log_end())) 
    {
        //double totalFemLarv = std::accumulate(tmpFemLarv.begin(), tmpFemLarv.end(), 0);
        // print results
        fprintf(logs.Output,"%s %zu %s %zu %s %zu %s %zu %s %zu\n","Total eggs: ",this->TotalEggs,"Total female larvae: ", totalFemLarv, "Total male larvae: ", totalMaleLarv,"Total female pupae: ",this->TotalFemalePupae,"Total male pupae: ",this->TotalMalePupae);
	    fprintf(logs.Output,"%s %zu %s %zu %s %zu\n","Total nulliparous adult females: ",this->TotalFemaleNulliparousAdults,"Total adult parous females: ",this->TotalFemaleParousAdults,"Total adult males: ",this->TotalMaleAdults);
	    fprintf(logs.Output,"%s\n","+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++");
    }
	//log the final tally into the short log file
	if ((PbMsg.output().do_output_per_house())) // if ((PbMsg.output().do_log_per_house()) && (Date%%50)) // Log every 50 days
	{
		//fprintf(logs.Output_per_House,"%d %d %d %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g %.8g",Date,unique_id,this->Container.size(), this->TotalEggs,this->TotalLarvae[FEMALE],this->TotalLarvae[MALE],this->TotalFemalePupae,this->TotalMalePupae,this->TotalFemaleNulliparousAdults,this->TotalFemaleParousAdults,this->TotalMaleAdults,this->Container[0].Food,this->Container[0].Maximum_Daily_Water_Temperature,this->Container[0].Minimum_Daily_Water_Temperature,Average_Female_Weight,Average_Male_Weight,this->Recaptured_Female,this->Recaptured_Male);
		fprintf(logs.Output_per_House,"%zu %zu %zu %zu  %zu %zu %zu %zu  %zu %zu %zu %zu  %zu  %zu %zu %zu %zu %zu %.8g %.8g %zu %zu",Date,unique_id,this->Container.size(), this->TotalEggs,this->TotalLarvae[0][0],this->TotalLarvae[0][1],this->TotalLarvae[0][2],this->TotalLarvae[0][3],this->TotalLarvae[1][0],this->TotalLarvae[1][1],this->TotalLarvae[1][2],this->TotalLarvae[1][3], New_Pupae_in_Container, this->TotalFemalePupae,this->TotalMalePupae,this->TotalFemaleNulliparousAdults,this->TotalFemaleParousAdults,this->TotalMaleAdults,Average_Female_Weight,Average_Male_Weight,this->Recaptured_Female,this->Recaptured_Male);
		//for (int cont=0 ; cont<this->Container.size() ; cont++) fprintf(logs.Output_per_House, " %.8g", this->Container[cont].Food) ;
		//fprintf(logs.Output_per_House, " %.3g", Sherlock[unique_id][1]) ;
		for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) fprintf(logs.Output_per_House," %.8g",Total_Genotype_per_House[genotype]);
		for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) fprintf(logs.Output_per_House," %.8g",Total_Adult_Genotype_per_House[genotype]);
		for (int allele=0;allele<2*PbMsg.sim().chrom_number();allele++) fprintf(logs.Output_per_House," %.8g",allele_frequency_per_House[allele]);
		fprintf(logs.Output_per_House,"\n");
	}

	// output per container

	if ((PbMsg.output().do_output_per_container()) && (Date>=PbMsg.output().output_per_container_start_date()))
	{
		for (vector<Receptacle>::iterator itCont=this->Container.begin() ; itCont!=this->Container.end() ; itCont++)
		{
            ////////////////////////////////////////
            // sum totals in this container
            ////////////////////////////////////////

			int Eggs_in_container = 0;
			for (const auto & the_eggs : itCont->Eggs){
				Eggs_in_container += (int) the_eggs.Number;
            }

			vector<int> Larvae_in_Container(N_INSTAR);
			for (const auto & the_larva : itCont->Larvae){
				Larvae_in_Container[the_larva.Instar -1]+=the_larva.Number;
            }
			int Pupae_in_Container=0;
			for (const auto & the_pupa : itCont->Pupae){
				Pupae_in_Container+=(the_pupa.Number);
            }
            ////////////////////////////////////////
            // reporting from here down
            ////////////////////////////////////////
			fprintf(logs.Output_per_Container, "%zu %zu ", Date, this->unique_id);
			fprintf(logs.Output_per_Container, "%zu %.5f ", itCont->ID, itCont->Food);
			fprintf(logs.Output_per_Container, "%d %d ", itCont->Container_Type_Number, itCont->Filling_Method);
			fprintf(logs.Output_per_Container, "%d ", Eggs_in_container);
			fprintf(logs.Output_per_Container, "%d %d %d %d ", Larvae_in_Container[0], Larvae_in_Container[1], Larvae_in_Container[2], Larvae_in_Container[3]);
			fprintf(logs.Output_per_Container, "%d ", Pupae_in_Container);
			fprintf(logs.Output_per_Container, "%d %d %d ", (int)(itCont->Total_NewlyPupated[0]+itCont->Total_NewlyPupated[1]), (int)(itCont->Total_NewlyPupated[0]), (int)(itCont->Total_NewlyPupated[1]));
			fprintf(logs.Output_per_Container, "%.6f %.6f ", itCont->Total_NewPupaeWeight[0],itCont->Total_NewPupaeWeight[1]);
			fprintf(logs.Output_per_Container, "%.6f ", itCont->Water_Level);
			fprintf(logs.Output_per_Container, "%.6f ", itCont->Food );
			fprintf(logs.Output_per_Container, "\n");
		}
	}
	// output for Sbeed at the beginning of the day
	if (PbMsg.output().do_output_for_sbeed())
	{
		for (const auto & the_fem : this->Female_Adults) {

			fprintf(logs.forSbeed, "%zu %zu %d %zu %d %.8g %.8g\n", unique_id, the_fem.ID, the_fem.Maternal_ID, the_fem.Age, the_fem.Genotype, the_fem.Average_Adult_Weight, the_fem.Physiological_Development_Percent);
		}
	}
	// custom xt output, e.g. for invasion wave representation or adulticidal control
    //@@ oldchunk 012
	
//@@ oldchunk 008
//grid-based 
//@@ oldchunk 003
//@@ oldchunk 002
    //@@ oldchunk 001
	return;
};

// TODO - lines 400 - ? Aedes.cpp
void Building::ContainerRelease() {
    if (ContainerReleaseDates.empty()) return;
    // assumes dates are sorted
    // check if today is a release day
    size_t the_date(*ContainerReleaseDates.begin());
    if (the_date!=today) {
        return;
    }
    // test condition
    // -1 -> not variance
    // 0 -> variance, not special
    // 1 = variance, special
    int special_house=-1;
    if (PbMsg.container().increased_container_variance()) {
        if (PbMsg.container().increased_variance_every_n_houses() < 2) {
            // see N-1 below
            throw std::runtime_error("Error in Building::ContainerRelease: increased_variance_every_n_houses < 2)"); 
        }
        // decide whether this house is special
        special_house=(Random_Number()<(1./PbMsg.container().increased_variance_every_n_houses()));
    }
    for (auto itContainer = Reserve_Containers.begin() ; itContainer != Reserve_Containers.end() ; ) // no increment, erase will move pointer
    {
        // if not today, do nothing
        if (itContainer->Release_Date != today) {
            itContainer++;
            continue;
        }
        if (today != 0 ) {
            // normal release
            Container.push_back(*itContainer);
        } else {
            // date == 0 is a bit of a shitshow
            // decide how many containers to add
            // (multiplier_factor, 0, 1, or increased_variance_every_n_houses()-1;
            // then add at end
            size_t n_cont_here(0);
            // not increased variance
            if ( special_house == -1) {
                n_cont_here = PbMsg.container().container_multiplier_factor() ; 
            } 
            // specific cases for initial container setup
            //!! Because this material regarding PbMsg.container().increased_container_variance() deals with initial conditions, this should be removed from the main SB source code and included in the setup file. 
            // 1. increased variance
            // in 1/N houses, containers are multiplied by N-1
            // in the other houses, containers are "divided" by N-1
            // expectation of total number of containers remains the same
            // (1/N)*(N-1) + ((N-1)/N)*(1/(N-1)) = 1
            else if ( special_house == 0) {
                // 1 if true, 0 otherwise
                // this is the "reduce" part
                n_cont_here = (Random_Number()<(1./(PbMsg.container().increased_variance_every_n_houses()-1)));
            } else if ( special_house == 1) {
                // add containers
                n_cont_here = PbMsg.container().increased_variance_every_n_houses()-1 ;
            } else {
                // sanity check
                throw std::logic_error("Error in Building::ContainerRelease: unexpected special_house");
            }
            for (size_t ii = 0; ii < n_cont_here; ii++) {
                // only thing changed..
                itContainer->ID += ii;
                this->Container.push_back(*itContainer);
            }
        }
        itContainer=Reserve_Containers.erase(itContainer); // this moves the pointer
    }
    // remove the date from the list
    ContainerReleaseDates.remove(today); 
};


// untested / deprecated??
// releases at stable age distribution
// converted cohorts are stacked into releases lists and vectors
// and pushed back in the next block
void Building::StableAgeDist() {
    // are we doing this?
    bool the_condition = (
        (PbMsg.stableagedist().sad_introduction_region()==0) // in every house
        ||
        (
                (PbMsg.stableagedist().sad_introduction_region()==1) // in defined region
                &&
                ((unique_id/PbMsg.sim().n_col())>=PbMsg.stableagedist().sad_introduction_y_min())
                &&
                ((unique_id/PbMsg.sim().n_col())<=PbMsg.stableagedist().sad_introduction_y_max())
                &&
                ((unique_id%PbMsg.sim().n_col())>=PbMsg.stableagedist().sad_introduction_x_min())
                &&
                ((unique_id%PbMsg.sim().n_col())<=PbMsg.stableagedist().sad_introduction_x_max())
        )
    );
    if (!the_condition) {
        // nothing to see here
        return;
    }
    // for each container in this house
    for (auto & the_cont : this->Container) {
        // list, use iterator not ref
        for (auto itEggs = the_cont.Eggs.begin(); itEggs != the_cont.Eggs.end(); )
        {
            int converted = Binomial_Deviate(itEggs->Number, PbMsg.stableagedist().sad_introduction_frequency());
            if (converted) {
                // ideally, genotype would be static
                Eggs_Cohort Converted_Eggs = *itEggs;
                Converted_Eggs.Genotype = PbMsg.stableagedist().sad_introduction_female_genotype()+(itEggs->Genotype%2);
                Converted_Eggs.Number = converted;
                Converted_Eggs.Release_Date = today;
                the_cont.ReleaseEgg.push_back(Converted_Eggs);
                itEggs->Number-=converted;
                if (itEggs->Number==0) {
                    itEggs=the_cont.Eggs.erase(itEggs);
                    continue; // erase advances
                } 
            }
            itEggs++;
        }

        // list, iterator
        for (auto itLarvae = the_cont.Larvae.begin(); itLarvae != the_cont.Larvae.end(); )
        {
            int converted = Binomial_Deviate(itLarvae->Number, PbMsg.stableagedist().sad_introduction_frequency());
            if (converted) {
                Larvae_Cohort Converted_Larvae = *itLarvae;
                Converted_Larvae.Genotype = PbMsg.stableagedist().sad_introduction_female_genotype()+(itLarvae->Genotype%2);
                Converted_Larvae.Number = converted;
                Converted_Larvae.Release_Date = today;
                the_cont.ReleaseLarva.push_back(Converted_Larvae);
                itLarvae->Number-=converted;
                if (itLarvae->Number==0) {
                    itLarvae=the_cont.Larvae.erase(itLarvae);
                    continue; // erase advances
                }
            }
            itLarvae++;
        }

        for (auto itPupae = the_cont.Pupae.begin(); itPupae != the_cont.Pupae.end(); )
        {
            int converted = Binomial_Deviate(itPupae->Number, PbMsg.stableagedist().sad_introduction_frequency());
            if (converted)
            {
                Pupae_Cohort Converted_Pupae = *itPupae;
                Converted_Pupae.Genotype = PbMsg.stableagedist().sad_introduction_female_genotype()+(itPupae->Genotype%2);
                Converted_Pupae.Number = converted;
                Converted_Pupae.Release_Date = today;
                the_cont.ReleasePupa.push_back(Converted_Pupae);
                itPupae->Number-=converted;
                if (itPupae->Number==0) {
                    itPupae=the_cont.Pupae.erase(itPupae);
                    continue; // erase advances
                }
            }
            else itPupae++;
        }
    }

    // then house males and females
    // list, use iterator
    for (auto thisMaleAdult = Male_Adult.begin() ; thisMaleAdult != Male_Adult.end() ; )
    {
        int converted = Binomial_Deviate(thisMaleAdult->Number, PbMsg.stableagedist().sad_introduction_frequency());
        if (converted) {
            Male_Adult_Cohort Converted_MaleAdult = *thisMaleAdult;
            Converted_MaleAdult.Genotype = PbMsg.stableagedist().sad_introduction_female_genotype()+(thisMaleAdult->Genotype%2);
            Converted_MaleAdult.Number = converted;
            Converted_MaleAdult.Release_Date = today;
            ReleaseAdultM.push_back(Converted_MaleAdult);
            ReleaseDone=false;
            thisMaleAdult->Number-=converted;
            if (thisMaleAdult->Number==0){
                thisMaleAdult=Male_Adult.erase(thisMaleAdult);
                continue; // erase advances
            }
        }
        thisMaleAdult++;
    }
    // go through every one, reference
    for (auto & thisFemAdult : Female_Adults) {
        if (Random_Number()<PbMsg.stableagedist().sad_introduction_frequency()) {
            thisFemAdult.Genotype = PbMsg.stableagedist().sad_introduction_female_genotype();
        }
    }
}

void Building::InsectRelease() {
    // release immatures into containers
    // check if *all* containers are done?
    for (auto & the_cont : Container) {
        // is this container done?
        if (the_cont.ReleaseDone) continue;
        // move any releases for today
        list_move_to(today, the_cont.ReleaseEgg, the_cont.Eggs);
        list_move_to(today, the_cont.ReleaseLarva, the_cont.Larvae);
        list_move_to(today, the_cont.ReleasePupa, the_cont.Pupae);
        // mark empty
        the_cont.UpdateReleaseDone();
    }

    // no more adults in house
    if (ReleaseDone) {
        return;
    }
    PushAdults(ReleaseAdultF, Female_Adults);
    PushAdults(ReleaseAdultM, Male_Adult);
    UpdateReleaseDone();
}

// make insects, mark and release into this house
// originally called with "House[0]"
void Building::MarkRecapture() {
    if (today==0) {
        // make female and release it
        Female_Adult release_female(NULLIPAROUS,0,0* PbMsg.devel().female_adult_daily_fecundity_factor_for_wet_weight(),0,0,0,false, 0, 0, -1);
        // redundant
        release_female.Age=0;
        release_female.Mature=0;
        release_female.Physiological_Development_Percent=0;
        //  House[Number_of_Rows/2*Number_of_Columns+Number_of_Columns/2].Female_Adult.push_back(release_females);
        Female_Adults.push_back(release_female);
        // male
        Male_Adult_Cohort release_males(6950,0.5,1,0,0);
        // redundant
        release_males.Age=0;
        release_males.Mature=0;
        release_males.Physiological_Development_Percent=0;
        //  House[Number_of_Rows/2*Number_of_Columns+Number_of_Columns/2].Male_Adult.push_back(release_males);
        Male_Adult.push_back(release_males);
    }
}



void Building::SetDevel(
    size_t devel_mode, size_t sim_mode1
){
	//calculation of larval developmental rate at 13.4C
	if (devel_mode==FOCKSDEVEL) {
		//Water_Average_Temp_K
		Development_134=DevelKineticsLarva(286.55);
	} else {
        Development_134=0.;
        Development_26=12.6/PbMsg.devel().degree_hours_to_be_reached();
    }
    //??
	if (sim_mode1==CIMSIM) {
        Development_134=0.00146;
    }
    //
	//calculation of larval developmental rate at 26C
	if (devel_mode==FOCKSDEVEL) {
		if (sim_mode1==CIMSIM) {
            //Water_Average_Temp_K according to CIMSIM with error
            Development_26=DevelKineticsLarva(299.);
        }
		if (sim_mode1==SKEETER_BUSTER) {
            // Water_Average_Temp_K	without error
            Development_26=DevelKineticsLarva(299.15);
        }
	}
}

//moving containers between houses
void BuildingMoveContainers(vbuild_t & the_houses) {
    // fill this, then move at end
    list<Receptacle> tmp_containers;
    //  find containers to move
    for ( auto & the_house : the_houses) {
        size_t containers_in_house = the_house.Container.size() ;
        // this should count down, not up??
        // container erased when moved...
        size_t idContainer = 0 ; // counter, not iterator anymore

        if (containers_in_house>1) {
            //why this condition ?
            // (1) problems arise if adults are in the house and the only container is removed
            // (2) it is not unrealistic to assume that if only one container
            //     is available, it will not be moved
            //       (possible improvement : probability of container movement functin of no. containers ...

            do
            {
                if ((Random_Number()<PbMsg.dispersal().container_movement_probability()) && (the_house.Container[idContainer].Container_Type_Number==5))
                {
                    // choosing destination house
                    // (randomly in the whole city, could be distance-dependent to focal house)
                    // LR container migration!! ??
                    size_t destination=(size_t)(PbMsg.sim().n_house()*Random_Number()) ;
                    // std::cout << "##Container moving!! From, to " << the_house.unique_id << ", " << destination << "\n";

                    // get iterator
                    auto contIt = the_house.Container.begin()+idContainer;

                    contIt->Food = 0;
                    contIt->Water_Level = contIt->Height ;
                    //moving_container.Eggs.clear() ;
                    contIt->Larvae.clear() ;
                    contIt->Pupae.clear() ;

                    contIt->house_ptr = &the_houses[destination];

                    // move??
                    tmp_containers.push_back(*contIt);
                    the_house.Container.erase(contIt);
                    containers_in_house-- ;
                }

                // alternative : independant birth/death of containers
                // (then no. containers varies throughout simulation)
            }
            while (++idContainer < containers_in_house) ;
        }
    }
    for (auto & movContainer : tmp_containers) {
			int destination = movContainer.house_ptr->unique_id ;
			the_houses[destination].Container.push_back(movContainer) ;
    }
}


// return true if female moved (advances iterator)
bool Building::FemaleDispersal(
    lfem_t::iterator & itFem, vbuild_t & houses
) {
    bool Dispersing;
    double Dispersing_Probability ;
    auto original = this->unique_id;
    auto current = original;
    //int direction = 3;
    bool has_moved = false;

    // possibly break / return early
    // max_number_of_dispersal_steps has no effect?
    for (int dispersal_step = 1 ; dispersal_step <= PbMsg.dispersal().max_number_of_dispersal_steps() ; dispersal_step++)
    {
        // calculate dispersal probability based on characteristics of the current house
        if (this->Container.size()) {
            Dispersing_Probability = ( itFem->Nulliparous ?  
                PbMsg.dispersal().nulliparous_female_adult_dispersal() 
                : PbMsg.dispersal().parous_female_adult_dispersal()
            );
        } else {
             Dispersing_Probability=( itFem->Nulliparous ?
                PbMsg.dispersal().nulliparous_female_adult_dispersal_from_empty_house()		
                : PbMsg.dispersal().parous_female_adult_dispersal_from_empty_house()
            );
        }
        Dispersing = (Random_Number()<Dispersing_Probability);

        // if that female moves on this event 
        if ( (Dispersing) && (this->neighbor.nlink > 0) )
        {
            size_t idest = this->neighbor.GetRandomNeighbor();
            // convenience
            auto & dest_house = houses[idest];
            // should this be possible??
            if (idest==original) {
                continue;
            }

            // if the destination house is a good house, push if necessary and stop
            // if too many steps already, push anyway
            if (
                (dest_house.Container.size() > 0) ||
                dispersal_step == PbMsg.dispersal().max_number_of_dispersal_steps()
            ){
                ReportDispersal(dest_house.unique_id, "");
                splice_end(dest_house.Migrating_Female_Adult, this->Female_Adults, itFem);
                has_moved=true;
                break; // finished dispersing loop
            } else {
                // increment 
                // if the destination house is a bad house, go to another step if possible
                // logic here possibly broken...
                current = idest ;
            }
        } else {
            // if that female does not move on this dispersal event
            // check if she has moved before, and if yes, push 
            // then stop
            // ?? move but don't move??

            if (current!=original) {
                ReportDispersal(this->unique_id, "self?");
                splice_end(this->Migrating_Female_Adult, this->Female_Adults, itFem);
                has_moved=true;
            }
            break;
        }
    }
    return has_moved; // erased from here, 
}

// return true if female moved (advances iterator)
bool Building::LRFemaleDispersal(
    lfem_t::iterator & itFem, vbuild_t & houses
) {
    bool move = ( itFem->Nulliparous ?
        Random_Number()<PbMsg.dispersal().nulliparous_female_adult_long_range_dispersal() 
        : Random_Number()<PbMsg.dispersal().parous_female_adult_long_range_dispersal()
    );
    if (!move) {
        // no move, done
        return false;
    } else {
        size_t idest = LRGetRandomNeighbor();
        ReportDispersal(idest, "LR");
        splice_end( houses[idest].Migrating_Female_Adult, this->Female_Adults, itFem);
        return true;
    }
}


// move males - cohort can split between neighbors
void Building::MaleDispersal(
    lmale_t::iterator & itMale, vbuild_t & houses
){
    if ((itMale->Number == 0) || (this->neighbor.nlink == 0)) {
        return; // done
    }
    // destination index, counts to destination
    std::map<size_t, size_t> Dispersing;

    // if/then/else
    double Dispersing_Males_Probability = ( this->Female_Adults.empty()? 
        PbMsg.dispersal().male_adult_dispersal_when_no_female():
        PbMsg.dispersal().male_adult_dispersal()
    );

    size_t ndisperse = Binomial_Deviate(itMale->Number,Dispersing_Males_Probability);

    if ( ndisperse == 0 ) {
        // done here
        return;
    } 
    for (size_t ii=0; ii<ndisperse; ii++) {
        // calculate the corresponding destination house
        size_t idest = this->neighbor.GetRandomNeighbor();
        Dispersing[idest]++;
    }

    // each pair always contains at least 1 moving male 
    for (auto & _pair : Dispersing) {
        // readability
        size_t idest = _pair.first;
        size_t nmove = _pair.second;
        // For purposes of testing, inherit the ID value here.
        // copy 
        Male_Adult_Cohort move_males(*itMale);
        move_males.Number = nmove;

        if (idest!=unique_id) {
            ReportDispersal(idest, "", "m");
            houses[idest].Migrating_Male_Adult.push_back(move_males);
            itMale->Number = itMale->Number - nmove;
        }
    }
}

// move males - cohort can split between neighbors
void Building::LRMaleDispersal(
    lmale_t::iterator & itMale, vbuild_t & houses
){
    if (itMale->Number==0) {
        return;
    }
    size_t ndisperse = Binomial_Deviate(
        itMale->Number, PbMsg.dispersal().male_adult_long_range_dispersal()
    );
    if (ndisperse == 0) {
        return;
    }
    for (size_t ii=0 ; ii<ndisperse ; ii++) {
        size_t idest = LRGetRandomNeighbor();
        ReportDispersal(idest, "LR", "m");
        // copy
        Male_Adult_Cohort move_males(*itMale);
        move_males.Number = 1;

        houses[idest].Migrating_Male_Adult.push_back(move_males);
        itMale->Number--;
    }
}


void Building::PlaceEggs(size_t sumeggs, std::map<int,Eggs_Cohort> & cohort_map ) {
    if (sumeggs == 0 ) { 
        return; 
    }
    // record-keeping, unused
    size_t sum_over_layers = 0;
    // 
	double EggInputLevel;
	double TopEggInputLevel;
	double TopContainerLevel;
    // main loop through containers
    for (size_t kk = 0; kk < this->Container.size(); kk++ ){
        // convenience
        auto & the_cont = this->Container[kk];
        //this calculates the level of eggs laid in the container based on CIMSIM code (not perfectly matching)
        if (PbMsg.sim().simulation_mode1()==CIMSIM){
            EggInputLevel=FLOOR_(the_cont.Water_Level/2)+1;
            TopContainerLevel=ROUND_(the_cont.Height/2+0.2);
        } else if (PbMsg.sim().simulation_mode1()==SKEETER_BUSTER) {
            EggInputLevel=FLOOR_(the_cont.Water_Level/0.2)+1;
            TopContainerLevel=ROUND_(the_cont.Height/0.2+2);
        } else {
            throw std::runtime_error("In Building.cpp, unrecognized simulation_mode1");
        }

        if (EggInputLevel>TopContainerLevel) {
            EggInputLevel=TopContainerLevel;
        }
        // oviposition test
        //bool oviposited = false;

        int container_mask = (the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes);
        int genotype_mask = (the_mask.n_wolbachia_status*the_mask.n_medea_masks);
        int medea_mask = the_mask.n_wolbachia_status;

        if (EggInputLevel==TopContainerLevel) {
            // with the use of map, no need to go through all possible values of 'index'
            // only the existing ones

            //for (int genotype=0;genotype<the_mask.n_genotypes;genotype++)
            //	for (int mom_type=0 ; mom_type<the_mask.n_medea_masks ; mom_type++)
            //		for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)

            for (auto & the_cohort : cohort_map) {
                size_t index = the_cohort.first;

                // some explanation for this index :
                // for some reason, containers_new_eggs_cohort is a 1-dimension only array, even though
                // it truly has 4 dimensions :
                // - containers ;
                // - eggs genotype ;
                // - medea_mom (medea status of the mother)
                // - wolbachia (wolbachia status of the mother)
                // 
                // The array is then hierarchised linearly, that is, big blocks representing one container, 
                // each of these blocks subdivided by genotype,
                // each of these subblocks subsubdivided by medea_mom type,
                // each of these subsubblobks subsubsubdivided (!) by wolbachia type.
                //
                // The above variable 'index' calculates the position in the array once and for all
                // for a given combination of all of the above.

                if ((index / container_mask)==kk) {
                    // this checks that we are in the right container
                    // there has to be a better way to do this

                    if (the_cohort.second.Number>0) // this should be redundant
                    {
                        the_cohort.second.Level_laid_in_Container=EggInputLevel;
                        the_cohort.second.Genotype=(index % container_mask)/genotype_mask;
                        //fprintf(stdout, "index %d genotype %d\n", the_cohort.first, the_cohort.second.Genotype);
                        the_cohort.second.Medea_Mom=(index % genotype_mask)/medea_mask;
                        the_cohort.second.Wolbachia=(index % medea_mask);

                        the_cont.Eggs.push_back(the_cohort.second);
                        // oviposition test
                        //fprintf(test1, "%.3f ", containers_new_eggs_cohort[the_mask.n_medea_masks*the_mask.n_genotypes*k+the_mask.n_medea_masks*genotype+mom_type].Number);
                        //oviposited = true;
                        //
                    }
                }
            }
        } else {
            if (PbMsg.sim().simulation_mode1()==CIMSIM) TopEggInputLevel=1; else TopEggInputLevel=TopContainerLevel-EggInputLevel;
            if (TopEggInputLevel>19) TopEggInputLevel=19;
            if (PbMsg.sim().simulation_mode2()==DETERMINISTIC)
            {
                for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) {
                    for (int mom_type=0 ; mom_type<the_mask.n_medea_masks ; mom_type++) {
                        for (int w=0; w<the_mask.n_wolbachia_status ; w++)
                        {
                            int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*kk+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_type+w;
                            cohort_map[index].Number = cohort_map[index].Number/(TopEggInputLevel+1);
                        }
                    }
                }
                for (int l=0;l<(TopEggInputLevel+1);l++)
                {
                    for (int genotype=0;genotype<the_mask.n_genotypes;genotype++) {
                        for (int mom_type=0 ; mom_type<the_mask.n_medea_masks ; mom_type++) {
                            for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
                            {
                                int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*kk+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_type+w;
                                if (cohort_map[index].Number>0)
                                {
                                    cohort_map[index].Level_laid_in_Container=EggInputLevel+l;
                                    cohort_map[index].Genotype=genotype;
                                    cohort_map[index].Medea_Mom=mom_type;
                                    cohort_map[index].Wolbachia=w;
                                    the_cont.Eggs.push_back(cohort_map[index]);
                                    //oviposited = true;//
                                }
                            }
                        }
                    }
                }
            } else {
                vector<double> layer_probabilities((int) (TopEggInputLevel+1), (1.0/(TopEggInputLevel+1.0)));
                vec_uint eggs_laid_per_layer((int) (TopEggInputLevel+1),0);
                // int number_of_layers = (int) ((TopEggInputLevel+1)) ;
                // this array is organized like this : first, layers for mom type 0, then layers for mom type 1, then ... etc. for all mom types

                //for (int genotype=0;genotype<the_mask.n_genotypes;genotype++)
                //	for (int mom_type=0 ; mom_type<the_mask.n_medea_masks ; mom_type++)
                //		for (int w=0 ; w<the_mask.n_wolbachia_status ; w++)
                for (auto & the_cohort: cohort_map) {
                    //int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*kk+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_type+w;

                    size_t index = the_cohort.first;
                    // doesn't work
                    // genmul ( (int) cohort_map[the_mask.n_genotypes*kk+genotype].Medea_Mom[mom_type], layer_probabilities, (int) (TopEggInputLevel+1), eggs_laid_per_layer+(mom_type*number_of_layers));

                    // int index = the_mask.n_wolbachia_status*the_mask.n_medea_masks*the_mask.n_genotypes*kk+the_mask.n_wolbachia_status*the_mask.n_medea_masks*genotype+the_mask.n_wolbachia_status*mom_type+w;					
                    // see above for details

                    // much needed control to avoid running genmul 
                    // if no eggs of this type (genotype+wolbachia) exist

                    if ((index / container_mask)==kk) {
                        // this checks that we are in the right container
                        // there has to be a better way to do this

                        if (the_cohort.second.Number>0) // this should be redundant
                        {

                            genmul ( (int) the_cohort.second.Number, layer_probabilities.data(), (int) (TopEggInputLevel+1), eggs_laid_per_layer.data());

                            the_cohort.second.Genotype=(index % container_mask)/genotype_mask;
                            //fprintf(stdout, "index %d genotype %d\n", the_cohort.first, the_cohort.second.Genotype);
                            the_cohort.second.Medea_Mom=(index % genotype_mask)/medea_mask;
                            the_cohort.second.Wolbachia=(index % medea_mask);

                            for (int layer=0;layer<(TopEggInputLevel+1);layer++) {
                                if (eggs_laid_per_layer[layer])
                                {
                                    the_cohort.second.Level_laid_in_Container=EggInputLevel+layer;
                                    the_cohort.second.Number=eggs_laid_per_layer[layer];

                                    the_cont.Eggs.push_back(the_cohort.second);

                                    if (the_cont.Container_Type_Number==102)
                                    {
                                        // Where did the error occur?
                                        std::cout << "Attempted mating on house: " << unique_id << "\n";
                                        ErrorMessage(102);
                                    }
                                    // oviposition test
                                    //oviposited = true;
                                    sum_over_layers+=the_cohort.second.Number;
                                    //
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

hid_t Building::LRGetRandomNeighbor() {
    if (do_LR_block) {
        hid_t dest = this->block_ptr->LRMigration();
        return dest;
    }
    // ELSE
    // xian 2016-11 : 
    // copy-pasta, completely broken, but needed for tests
    // (only dispersal mode STICKY_BORDER tested)
    // orig comments follow.
    //
    // 5 steps to calculate destination :
    // (1) get the distance as a number of steps
    // (random between 1 and PbMsg.dispersal().lr_dispersal_max_distance()) ;
    
    // convenience / compiler warnings
    int ncol = PbMsg.sim().n_col();
    int nrow = PbMsg.sim().n_row();
    int steps = 1+ static_cast<int>((PbMsg.dispersal().lr_dispersal_max_distance()-1)*Random_Number());

    // (2) a certain proportion of these steps go horizontally,
    // left if dx<0, right if dx>0 ;
    int dx = (int) ((2*Random_Number()>1 ? -1 : 1) * steps * Random_Number() );
    int xpos = (unique_id%ncol)+dx ;

    // (3) the rest of the steps go vertically,
    // down if dy>0, up if dy<0 ;
    int dy = (2*Random_Number()>1 ? -1 : 1) * (steps-(dx>0?dx:-dx)) ;
    int ypos = (unique_id/ncol)+dy ;

    // (4) calculate the actual coordinates of the destination house
    // depending on the topology of the grid
    int xdest(0), ydest(0);
    switch(PbMsg.sim().dispersal_mode())
    {
    case STICKY_BORDER:

        xdest = ( xpos < 0 ? 0 :
        (xpos >= ncol ? ncol-1 :
        xpos)) ;

        ydest = ( ypos < 0 ? 0 :
        (ypos >= nrow ? nrow-1 :
        ypos)) ;
        break;

    case BOUNCE:
        do {
            xpos = ( xpos < 0 ? -(xpos) :
                    (xpos >= ncol ? ncol-1 +(ncol-1 -(xpos)) :
                            xpos)) ;
        } while ((xpos<0) || (xpos>ncol-1)) ;
        xdest=xpos ;


        do {
            ypos = ( ypos < 0 ? -(ypos) :
                    (ypos >= nrow ? nrow-1 + (nrow-1 -(ypos)) :
                            ypos)) ;
        } while ((ypos<0) || (ypos>nrow-1)) ;
        ydest=ypos ;
        break;

    case TORE:

        xdest = xpos%ncol ;
        xdest = (xdest<0 ? xdest+ncol : xdest);
        ydest = ypos%nrow ;
        ydest = (ydest<0 ? ydest+nrow : ydest);
        break ;

    case RANDOM_EDGES:

        // randomly coming back on one of the 4 edges
        // note that each edge include only one corner
        // so that the corners are not counted twice

        int edge = (int)(4*Random_Number()) ;
        switch(edge)
        {
        case 0: //north
            xdest = (int)((ncol-1)*Random_Number()) ;
            ydest = 0 ;
            break;
        case 1: //east
            xdest = ncol-1 ;
            ydest = (int)((nrow-1)*Random_Number()) ;
            break;
        case 2: //south
            xdest = 1+(int)((ncol-1)*Random_Number()) ;
            ydest = nrow-1 ;
            break;
        case 3: //west
            xdest = 0 ;
            ydest = 1+(int)((nrow-1)*Random_Number()) ;
            break;
        }
    }
    // (5) convert these coordinates into the destination house number
    return ydest*ncol + xdest;
}

// all kinds of crazy
// what's the logic of when this happens?
void apply_residual_control(size_t Date, vbuild_t & House)
{
	for (size_t House_Number = 0 ; House_Number < House.size() ; House_Number++)
    {
		if (House[House_Number].last_date_sprayed > 0)
        {
			double days_after = (double) (Date-House[House_Number].last_date_sprayed);
			if (PbMsg.adulticide().adulticide_efficacy()==2.) // special case for Amy's values
            {
				if (days_after>3.5) return ;
				else if (days_after>2.5) 
                    House[House_Number].killing_efficacy = 0.25; // KWO: Set to 0.25 from 0.2 to be consistent with NIH_november 2010
				else if (days_after>1.5)
                    House[House_Number].killing_efficacy = 0.6;
				else if (days_after>0.5) 
                    House[House_Number].killing_efficacy = 0.95;
				else 
                    House[House_Number].killing_efficacy = 0.99;
            } else {
				House[House_Number].killing_efficacy = PbMsg.adulticide().adulticide_efficacy() ;
				if (days_after>0) {
					if (days_after<(0.5+PbMsg.adulticide().adulticide_residual_length()))
                        House[House_Number].killing_efficacy *= pow((double)(1-PbMsg.adulticide().adulticide_efficacy_decrease()),days_after);
					else return;
                }
            }
			double killing = House[House_Number].killing_efficacy;
			list<Female_Adult>::iterator itFemale ;
			int Number_of_Adults_Killed_in_House = 0;
			
			for (itFemale = House[House_Number].Female_Adults.begin() ; itFemale != House[House_Number].Female_Adults.end() ; )
            {
                if (Random_Number() < killing)
                {
                    Number_of_Adults_Killed_in_House++;
                    itFemale = House[House_Number].Female_Adults.erase(itFemale);
                }
                else itFemale++;
            }
			list<Male_Adult_Cohort>::iterator itMaleCohort;

			for (itMaleCohort = House[House_Number].Male_Adult.begin() ; itMaleCohort != House[House_Number].Male_Adult.end() ; )
            {
                int Number_of_Males_Killed = 0;
                Number_of_Males_Killed = Binomial_Deviate(itMaleCohort->Number, killing);
                itMaleCohort->Number -= Number_of_Males_Killed ;
                Number_of_Adults_Killed_in_House += Number_of_Males_Killed;
                if (itMaleCohort->Number ==0)
                    itMaleCohort = House[House_Number].Male_Adult.erase(itMaleCohort);
                else itMaleCohort++;
            }
            // by default runs only on the first date of spraying??
            // std::cout << "Date: " << Date << " House: " << House_Number << " killed: " << Number_of_Adults_Killed_in_House << " days after: " << days_after << "\n";
        }
    }
}
