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
#include "Aedes_class_defs.h"

extern vector<Building> House ;

void Destructively_Sample_Larvae (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House)
{
	int Sampled_House = 0;

	vector<Building>::iterator itHouse = House.begin();

	for (Sampled_House = 0 ; Sampled_House < ((int) Sampled_Houses_Vector.size()); Sampled_House++)
	{
	int House_Number = Sampled_Houses_Vector[Sampled_House];
		// if (House_Number%((int)(1./(1-SAMPLING_COMPLIANCE)))) /* reinstate if fraction of houses sporadically refuse sampling */
			int Number_of_Larvae_Sampled_in_House = 0;
			vector<Receptacle>::iterator itContainer;
			for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; itContainer++)
			{
			if (Random_Number() < SAMPLE_CONTAINER_PROBABILITY)
				{
					/*
					list<Eggs_Cohort>::iterator itEggCohort;
					for (itEggCohort=itContainer->Eggs.begin() ; itEggCohort!=itContainer->Eggs.end() ; )
					{
						itEggCohort->Number = Binomial_Deviate(itEggCohort->Number, 1-PROPORTION_OF_EGGS_SAMPLED);
						if (itEggCohort->Number==0)
							itEggCohort=itContainer->Eggs.erase(itEggCohort);
						else
							itEggCohort++;
					}
					 */
					list<Larvae_Cohort>::iterator itLarvaCohort;
					for (itLarvaCohort=itContainer->Larvae.begin() ; itLarvaCohort!=itContainer->Larvae.end() ; )
					{
						int Number_of_Larvae_Sampled_in_Container = 0;
						Number_of_Larvae_Sampled_in_Container = Binomial_Deviate(itLarvaCohort->Number, PROPORTION_OF_LARVAE_SAMPLED);
						itLarvaCohort->Number -= Number_of_Larvae_Sampled_in_Container;
						Number_of_Larvae_Sampled_in_House += Number_of_Larvae_Sampled_in_Container;

						if (itLarvaCohort->Number==0)
							itLarvaCohort=itContainer->Larvae.erase(itLarvaCohort);
						else
							itLarvaCohort++;
					}
				}
			}
	Sampling_Counts_Per_House[Sampled_House] = Number_of_Larvae_Sampled_in_House;
	}
	return;
}

void Destructively_Sample_Adults (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House)
{
	int Sampled_House = 0;

	vector<Building>::iterator itHouse = House.begin();

	for (Sampled_House = 0 ; Sampled_House < ((int) Sampled_Houses_Vector.size()); Sampled_House++)
		{
		int Number_of_Adults_Sampled_in_House = 0;
		// if (House_Number%((int)(1./(1-SAMPLING_COMPLIANCE)))) /* reinstate if fraction of houses sporadically refuse sampling */
			list<Female_Adult>::iterator itFemale ;
		int House_Number = Sampled_Houses_Vector[Sampled_House];

		for (itFemale = House[House_Number].Female_Adults.begin() ; itFemale != House[House_Number].Female_Adults.end() ; )
			{
				int Check_if_Female_Sampled = 0;
				if (Random_Number() < PROPORTION_OF_ADULT_FEMALES_SAMPLED)
								{
								Check_if_Female_Sampled=1;
								Number_of_Adults_Sampled_in_House++;
								}
				if (Check_if_Female_Sampled > 0)
				{
					itFemale = House[House_Number].Female_Adults.erase(itFemale);
				}
				else itFemale++;
			}

			list<Male_Adult_Cohort>::iterator itMaleCohort;

			for (itMaleCohort = House[House_Number].Male_Adult.begin() ; itMaleCohort != House[House_Number].Male_Adult.end() ; )
			{
				int Number_of_Males_Sampled = 0;
				Number_of_Males_Sampled = Binomial_Deviate(itMaleCohort->Number, PROPORTION_OF_ADULT_MALES_SAMPLED);
				itMaleCohort->Number -= Number_of_Males_Sampled ;
				Number_of_Adults_Sampled_in_House += Number_of_Males_Sampled;
				if (itMaleCohort->Number ==0)
					itMaleCohort = House[House_Number].Male_Adult.erase(itMaleCohort);
				else itMaleCohort++;
			}
		Sampling_Counts_Per_House[Sampled_House] = Number_of_Adults_Sampled_in_House;
		}
	return;
}


void Destructively_Sample_Pupae (vector<int> &Sampled_Houses_Vector, vector<int> &Sampling_Counts_Per_House)
{
	int Sampled_House = 0;
	vector<Building>::iterator itHouse = House.begin();

	for (Sampled_House = 0 ; Sampled_House < ((int) Sampled_Houses_Vector.size()); Sampled_House++)
	{
	int House_Number = Sampled_Houses_Vector[Sampled_House];
	int Number_of_Pupae_Sampled_in_House = 0;
		// if (House_Number%((int)(1./(1-SAMPLING_COMPLIANCE)))) /* reinstate if fraction of houses sporadically refuse sampling */
		vector<Receptacle>::iterator itContainer;
		for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; itContainer++)
		{
		if (Random_Number() < SAMPLE_CONTAINER_PROBABILITY)
			{
			list<Pupae_Cohort>::iterator itPupaCohort;

			for (itPupaCohort=itContainer->Pupae.begin() ; itPupaCohort!=itContainer->Pupae.end() ; )
				{
				int Number_of_Pupae_Sampled_in_Container = 0;
				Number_of_Pupae_Sampled_in_Container = Binomial_Deviate(itPupaCohort->Number, PROPORTION_OF_PUPAE_SAMPLED);
				itPupaCohort->Number -= Number_of_Pupae_Sampled_in_Container;
				Number_of_Pupae_Sampled_in_House += Number_of_Pupae_Sampled_in_Container;
				if (itPupaCohort->Number==0)
					itPupaCohort = itContainer->Pupae.erase(itPupaCohort);
				else
					itPupaCohort++;
				}
			}
		}
	Sampling_Counts_Per_House[Sampled_House] = Number_of_Pupae_Sampled_in_House;
	}
}
