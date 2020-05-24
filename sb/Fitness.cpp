//    Copyright 2006-2010 Krisztian Magori, Mathieu Legros, Alun Lloyd, Fred Gould
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

#include "stdafx.h"
#include "Aedes.h"

////////////////////////////////////////////////////////////////////////////
// The following function returns the fitness value for a given genotype, //
// depending on the considered genetic control method.                    //
//                                                                        //
// For details about the conditions used for these calculations,          //
// see their definition in Aedes.h                                        //
////////////////////////////////////////////////////////////////////////////

double Fitness_Calculation(int genotype)
{
	double fitness=1;
	switch (GENETIC_CONTROL_METHOD)
	{
	case NEUTRAL:
			fitness=1;
			break;
	case ENGINEERED_UNDERDOMINANCE:	
			switch (EU_TYPE)
			{
			case 1:		
				fitness=1;
				/*
				// calculation of the fitness costs depending on the number of constructs
				if (HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_ALPHA)*(1-FITNESS_COST_CONSTRUCT_ALPHA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_ALPHA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_BETA)*(1-FITNESS_COST_CONSTRUCT_BETA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_BETA,2*FITNESS_COST_DOMINANCE);

				// the following genotypes are not viable :
				// - AT LEAST ONE copy of alpha and NO copy of beta ;
				// - NO copy of alpha and AT LEAST ONE copy of beta ;
				if (IS_NOT_VIABLE_EU_TYPE_1(genotype)) fitness=0;
				*/

				//other solution :
				if (HAS_CONSTRUCT_ALPHA_EU_TYPE_1(genotype))
					if (HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_1(genotype))
						fitness*=(1-FITNESS_COST_CONSTRUCT_ALPHA)*(1-FITNESS_COST_CONSTRUCT_ALPHA);
					else // has one copy
						fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_ALPHA,2*FITNESS_COST_DOMINANCE);
				if (HAS_CONSTRUCT_BETA_EU_TYPE_1(genotype))
					if (HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_1(genotype))
						fitness*=(1-FITNESS_COST_CONSTRUCT_BETA)*(1-FITNESS_COST_CONSTRUCT_BETA);
					else // has one copy
						fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_BETA,2*FITNESS_COST_DOMINANCE);

				// the following genotypes are not viable :
				// - AT LEAST ONE copy of alpha and NO copy of beta ;
				// - NO copy of alpha and AT LEAST ONE copy of beta ;
				if (IS_NOT_VIABLE_EU_TYPE_1(genotype)) fitness=0;

				break;		
			case 2:
				fitness=1;
				// calculation of the fitness costs depending on the number of constructs
				if (HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_2(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_ALPHA)*(1-FITNESS_COST_CONSTRUCT_ALPHA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_2(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_ALPHA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_2(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_BETA)*(1-FITNESS_COST_CONSTRUCT_BETA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_2(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_BETA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_2(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_GAMMA)*(1-FITNESS_COST_CONSTRUCT_GAMMA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_2(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_GAMMA,2*FITNESS_COST_DOMINANCE);

				// the following genotypes are not viable :
				// - AT LEAST ONE copy of alpha or beta and NO copy of gamma ;
				// - NO copy of alpha nor beta and AT LEAST ONE copy of gamma ;
				if (IS_NOT_VIABLE_EU_TYPE_2(genotype)) fitness=0;
				
				break;
			case 3:
				fitness=1;
				// calculation of the fitness costs depending on the number of constructs
				if (HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_3(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_ALPHA)*(1-FITNESS_COST_CONSTRUCT_ALPHA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_3(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_ALPHA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_3(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_BETA)*(1-FITNESS_COST_CONSTRUCT_BETA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_3(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_BETA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_3(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_GAMMA)*(1-FITNESS_COST_CONSTRUCT_GAMMA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_3(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_GAMMA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_DELTA_EU_TYPE_3(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_DELTA)*(1-FITNESS_COST_CONSTRUCT_DELTA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_DELTA_EU_TYPE_3(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_DELTA,2*FITNESS_COST_DOMINANCE);

				// the following genotypes are not viable :
				// - AT LEAST ONE copy of alpha or beta or gamma and NO copy of delta ;
				// - NO copy of alpha nor beta nor gamma and AT LEAST ONE copy of delta ;
				if (IS_NOT_VIABLE_EU_TYPE_3(genotype)) fitness=0;
				
				break;
			case 4:
				fitness=1;
				// calculation of the fitness costs depending on the number of constructs
				if (HAS_TWO_COPIES_OF_CONSTRUCT_ALPHA_EU_TYPE_4(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_ALPHA)*(1-FITNESS_COST_CONSTRUCT_ALPHA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_ALPHA_EU_TYPE_4(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_ALPHA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_BETA_EU_TYPE_4(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_BETA)*(1-FITNESS_COST_CONSTRUCT_BETA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_BETA_EU_TYPE_4(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_BETA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_GAMMA_EU_TYPE_4(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_GAMMA)*(1-FITNESS_COST_CONSTRUCT_GAMMA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_GAMMA_EU_TYPE_4(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_GAMMA,2*FITNESS_COST_DOMINANCE);
				if (HAS_TWO_COPIES_OF_CONSTRUCT_DELTA_EU_TYPE_4(genotype)) fitness*=(1-FITNESS_COST_CONSTRUCT_DELTA)*(1-FITNESS_COST_CONSTRUCT_DELTA);
				if (HAS_ONE_COPY_OF_CONSTRUCT_DELTA_EU_TYPE_4(genotype)) fitness*=pow( (double)1-FITNESS_COST_CONSTRUCT_DELTA,2*FITNESS_COST_DOMINANCE);

				// the following genotypes are not viable :
				// - AT LEAST ONE copy of alpha or beta and NO copy of gamma nor delta ;
				// - NO copy of alpha nor beta and AT LEAST ONE copy of gamma or delta ;
				if (IS_NOT_VIABLE_EU_TYPE_4(genotype)) fitness=0;
				
				break;
			}
		break;

	case MEIOTIC_DRIVE:
		switch(MD_TYPE)
		{
		case 1:
			fitness=1;
			if ((genotype==4)||(genotype==16)) fitness=1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_TRANSGENE;
			if (genotype==20) fitness=1-FITNESS_COST_INSENSITIVE_TRANSGENE;
			if (genotype==5) fitness=1-FITNESS_COST_DRIVE_GENE;
			if (genotype==21) fitness=(1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_TRANSGENE)*(1-FITNESS_COST_DRIVE_GENE);
			break;
		case 2:
			fitness=1;
			if ((genotype==4)||(genotype==16)) fitness=1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_TRANSGENE;
			if ((genotype==48)||(genotype==12)) fitness=1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_NATURAL;
			if ((genotype==28)||(genotype==52)) fitness=(1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_TRANSGENE)*(1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_NATURAL);
			if (genotype==20) fitness=1-FITNESS_COST_INSENSITIVE_TRANSGENE;
			if (genotype==60) fitness=1-FITNESS_COST_INSENSITIVE_NATURAL;
			if (genotype==5) fitness=1-FITNESS_COST_DRIVE_GENE;
			if (genotype==53) fitness=(1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_NATURAL)*(1-FITNESS_COST_DRIVE_GENE);
			if (genotype==21) fitness=(1-MD_FITNESS_COST_DOMINANCE*FITNESS_COST_INSENSITIVE_TRANSGENE)*(1-FITNESS_COST_DRIVE_GENE);
			break;
		}
		break;


	case FKR: {
		fitness=1;
		if (((genotype>>2)&3u)&&IS_RESCUE) // at least one rescue allele
			fitness=1;
		else
		{
			int fk=1;
			int shift_rescue = 0;
			if (IS_RESCUE) shift_rescue = 2;
			// if there is rescue, one chromosome is reserved for it, we have to skip it when dealing with the killing
			do
			{
				if (FKR_IS_EMBRYO_KILLING || (!(genotype&3u))) // female if not embryo killing
					if ((genotype>>(shift_rescue+(2*fk)))&3u) // at least one killing allele
						fitness*=(1-FEMALE_KILLING_EFFICIENCY);
			}
			while (fk++<NUMBER_OF_FEMALE_KILLING_LOCI);
		}

		if (fitness) // process fitness costs if fitness is not already 0
		{
			int limit = NUMBER_OF_FEMALE_KILLING_LOCI+1;
			if (IS_RESCUE) limit++; // same here, if there is rescue, there is one more chromosome to deal with

			for (int chrom=1 ; chrom<limit ; chrom++)
			{
				double cost = KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST ;
				double dominance = KILLING_ALLELE_FITNESS_COST_DOMINANCE ;
				if (IS_RESCUE && (chrom==1)) 
				{
					cost = RESCUE_ALLELE_HOMOZYGOUS_FITNESS_COST ;
					dominance = RESCUE_ALLELE_FITNESS_COST_DOMINANCE ;
				}
				int check = (genotype>>(2*chrom))&3u ;
				if (check)
				{
					if (check==3) fitness*=(1-cost);
					else fitness*=(1-dominance*cost);
				}
			}
		}
	}
		break;

/* This is a very special case used for inferring what happens to female adults */
	case Male_Killing: {
			fitness=1;
			int mk=0;
			int shift_rescue = 0;
			do
			{
				if (genotype&3u) // male; to be applied at adult stage
					{
					if ((genotype>>(shift_rescue+(2*mk)))&3u) // at least one killing allele
						{
						fitness*=(1-MALE_KILLING_EFFICIENCY);
						}
					}
			}
			while (mk++<NUMBER_OF_MALE_KILLING_LOCI);
		}
		break;



	case ReduceReplace: {
		fitness=1;

		int fk=1;
		int shift_rescue = 2*(NUMBER_OF_ADDITIONAL_LOCI);
		// Since there are a neutral and sex-determining loci preceding the killing loci, begin the fitness calculations on the 3rd locus
		do
			{
				if (FKR_IS_EMBRYO_KILLING || (!(genotype&3u))) // female if not embryo killing
					if ((genotype>>(shift_rescue+(2*fk)))&3u) // at least one killing allele
						fitness*=(1-FEMALE_KILLING_EFFICIENCY);
			}
		while (fk++<NUMBER_OF_FEMALE_KILLING_LOCI);

		if (fitness) // process fitness costs if fitness is not already 0
		{
			int limit = NUMBER_OF_FEMALE_KILLING_LOCI+1+NUMBER_OF_ADDITIONAL_LOCI;
			double cost, dominance;
			for (int chrom=NUMBER_OF_ADDITIONAL_LOCI+1 ; chrom<limit ; chrom++)
			{
				cost = KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST ;
				dominance = KILLING_ALLELE_FITNESS_COST_DOMINANCE ;

				int check = (genotype>>(2*chrom))&3u ; // What is genotype at locus chrom?
				if (check) // check==0 corresponds to the case of homozygous with respect to the wildtype
				{
					if (check==3) fitness*=(1-cost); // check==3 corresponds to homozygous with respect to transgene
					else fitness*=(1-dominance*cost);
				}
			}
		}
		/* For the case of one replacement gene, make sure that females inheriting the killer gene from their fathers have a fitness value=0
		if ((genotype==16) || (genotype==20) || (genotype==24) || (genotype==28))
			std::cout << genotype << " " << fitness << std::endl;
		*/
	}
	break;

	case SELECTION:
		fitness = 1.;
		int check = (genotype>>2)&3u;
		if (check)
		{
			if (check==3) fitness*=(1-SELECTIVE_FITNESS_COST);
			else fitness*=(1-SELECTIVE_FITNESS_COST_DOMINANCE*SELECTIVE_FITNESS_COST);
		}
		break;
	}
	
	return fitness;
}


double Fitness_Calculation_RR(int genotype)
{
	double fitness=1;
	int limit =1+NUMBER_OF_ADDITIONAL_LOCI;
	double cost, dominance;
	for (int chrom=1 ; chrom<limit ; chrom++)
		{
		if ((NUMBER_OF_ADDITIONAL_LOCI) && (chrom < 1+NUMBER_OF_ADDITIONAL_LOCI))
			{
			cost = TRANSGENE_ALLELE_HOMOZYGOUS_FITNESS_COST ;
			dominance = TRANSGENE_ALLELE_FITNESS_COST_DOMINANCE ;
			}
		int check = (genotype>>(2*chrom))&3u ; // What is genotype at locus chrom?
		if (check) // check==0 corresponds to the case of homozygous with respect to the wildtype
			{
				if (check==3) fitness*=(1-cost); // check==3 corresponds to homozygous with respect to transgene
				else fitness*=(1-dominance*cost);
			}
		}
	return fitness;
}



double Fitness_Calculation_Environmental_Effect(int genotype)
{
	double fitness=1;
	/* for the replacement allele */
	int limit =1+NUMBER_OF_ADDITIONAL_LOCI;
		double cost, dominance;
		if (FITNESS_COST_STAGE_RR==FITNESS_COST_STAGE_SPECIFIC_AGE)
		{
			/* if the transgene gets applied at the adult stage upon emergence */
		for (int chrom=1 ; chrom<limit ; chrom++)
			{
			if ((NUMBER_OF_ADDITIONAL_LOCI) && (chrom < 1+NUMBER_OF_ADDITIONAL_LOCI))
				{
				cost = TRANSGENE_ALLELE_HOMOZYGOUS_FITNESS_COST ;
				dominance = TRANSGENE_ALLELE_FITNESS_COST_DOMINANCE ;
				}
			int check = (genotype>>(2*chrom))&3u ; // What is genotype at locus chrom?
			if (check) // check==0 corresponds to the case of homozygous with respect to the wildtype
				{
					if (check==3) fitness*=(1-cost); // check==3 corresponds to homozygous with respect to transgene
					else fitness*=(1-dominance*cost);
				}
			}
		}
		/* For the killer gene, remove the killing efficiency process*/
	limit = NUMBER_OF_FEMALE_KILLING_LOCI+1+NUMBER_OF_ADDITIONAL_LOCI;
		cost = 0;
		dominance = 0;
		for (int chrom=NUMBER_OF_ADDITIONAL_LOCI+1 ; chrom<limit ; chrom++)
			{
				cost = KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST ;
				dominance = KILLING_ALLELE_FITNESS_COST_DOMINANCE ;

				int check = (genotype>>(2*chrom))&3u ; // What is genotype at locus chrom?
				if (check) // check==0 corresponds to the case of homozygous with respect to the wildtype
					{
					if (check==3) fitness*=(1-cost); // check==3 corresponds to homozygous with respect to transgene
					else fitness*=(1-dominance*cost);
					}
			}

	return fitness;
}


double Fitness_Calculation_Medea (int genotype, int mom_type)
{
	double fitness=1.;
	int mask=0;
	// maternal lethality

	if(MEDEA_CROSS_RESCUE)
	{
		// mask is 11 on all MEDEA chromosomes
		for (int chrom=1 ; chrom<MEDEA_UNITS+1 ; chrom++)
			mask+=(3u<<(2*chrom)) ;
		// only cases of lethality: mom has any MEDEA (type other than 0) and offspring is WT on all MEDEA chromosomes
		if ((mom_type!=0)&&((genotype&mask)==0)) fitness*=(1-MEDEA_MATERNAL_LETHALITY);
	}
	else
	{
		// mask is 11 on each chromosome where mom had at least one MEDEA
		for (int chrom=1 ; chrom<MEDEA_UNITS+1 ; chrom++)
			if (mom_type&(1u<<(chrom-1)))
				mask+=(3u<<(2*chrom));
		// compare to genotype offspring:
		// (1) &=mask: turn off the bits on the neutral chromosomes, or those where mom had no MEDEA
		// (2) ^=mask: on the relevant chromosomes, turn on the bits that differ between offspring and mom_mask
		//     This way, the bits turned on denote a WT (0) allele on a chromosome where mom had MEDEA (1)
		//     If one of the chromosomes shows 11, maternal lethality applies.
		int compare = genotype;
		compare&=mask;
		compare^=mask;
		for (int chrom=1 ; chrom<MEDEA_UNITS+1 ; chrom++)
			if (((compare>>(2*chrom))&3u)==3)
				fitness*=(1-MEDEA_MATERNAL_LETHALITY) ; // may be changed
	}

	if (fitness) // for the survivors
	{
		for (int chrom=1 ; chrom<MEDEA_UNITS+1 ; chrom++)
			if ((MEDEA_FEMALE_SPECIFIC_FITNESS_COST && ((genotype%2)==0)) || (!MEDEA_FEMALE_SPECIFIC_FITNESS_COST))
		{
			int type=((genotype>>(2*chrom))&3u);
			// shifts genotype to the right until the chromosome of interest is at rightmost,
			// then masks to 0...0011 to extract the alleles on this chromosome

			if (type==3) // 11 on this chromosome, 2 MEDEA constructs
				fitness*=(1-MEDEA_FITNESS_COST_PER_CONSTRUCT)*(1-MEDEA_FITNESS_COST_PER_CONSTRUCT) ;
			else if ((type==2)||(type==1)) // 10 or 01 on this chromosome, 1 MEDEA construct
				fitness*=pow((double)(1-MEDEA_FITNESS_COST_PER_CONSTRUCT),(2*MEDEA_FITNESS_COST_DOMINANCE)) ;
			else // 00 on this chromosome, no MEDEA 
				if (type!=0) // this shouldn't happen, so raise an alarm if it does
					fprintf(stdout, "*** Something's wrong in the fitness calculation for MEDEA\n") ;
		}
	}

	return fitness;

}

