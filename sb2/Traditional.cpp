//    DEPRECATED
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
#include "Receptacle.h"
#include "Building.h"
#include "Binomial.h"
void ErrorMessage (int);

// old grid-based code
// deprecated
void Larvicide_grid (int type, int region, int size, vector<Building> & House )
{
	size_t House_Number = 0;
	size_t lower_row=0,upper_row=0,left_column=0,right_column=0;

	if (region==1) {
		if (!(size>0)) {
			ErrorMessage(801);
		} else {
			lower_row = (PbMsg.sim().n_row()/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (PbMsg.sim().n_col()/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}
	if (region==2) {
		if (
			(PbMsg.larvicide().larvicide_region_2_ymin() > PbMsg.larvicide().larvicide_region_2_ymax()) 
			|| 
			(PbMsg.larvicide().larvicide_region_2_xmin() > PbMsg.larvicide().larvicide_region_2_xmax()) 
			|| 
			(PbMsg.larvicide().larvicide_region_2_ymax() > PbMsg.sim().n_row()) 
			|| 
			(PbMsg.larvicide().larvicide_region_2_xmax() > PbMsg.sim().n_col())
        ) {
            ErrorMessage(801);
        } else {
            lower_row = PbMsg.larvicide().larvicide_region_2_ymin();
            upper_row = PbMsg.larvicide().larvicide_region_2_ymax();
            left_column = PbMsg.larvicide().larvicide_region_2_xmin();
            right_column = PbMsg.larvicide().larvicide_region_2_xmax();
        }
	}
	for (House_Number = 0 ; House_Number < PbMsg.sim().n_house() ; House_Number++)
	if (
		(region == 0) // if region is everywhere, pass
		||
		(
		 ((region==1)||(region==2)) // if region is central square or custom rectangle, check if house is in the square
		 &&
		 ((House_Number/PbMsg.sim().n_col())>=lower_row)
		 &&
		 ((House_Number/PbMsg.sim().n_col())<=upper_row)
		 &&
		 ((House_Number%PbMsg.sim().n_col())>=left_column)
		 &&
		 ((House_Number%PbMsg.sim().n_col())<=right_column)
		 )
    )
		if (House_Number%((int)(1./(1-PbMsg.larvicide().larvicide_compliance()))))
		{
			vector<Receptacle>::iterator itContainer;
			for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; itContainer++)
				if (Random_Number()<PbMsg.larvicide().larvicide_container_probability())
				{
					list<Eggs_Cohort>::iterator itEggCohort;
					for (itEggCohort=itContainer->Eggs.begin() ; itEggCohort!=itContainer->Eggs.end() ; )
					{
						itEggCohort->Number = Binomial_Deviate(itEggCohort->Number, 1-PbMsg.larvicide().larvicide_efficacy_on_eggs());
						if (itEggCohort->Number==0) 
							itEggCohort=itContainer->Eggs.erase(itEggCohort);
						else
							itEggCohort++;
					}

					list<Larvae_Cohort>::iterator itLarvaCohort;
					for (itLarvaCohort=itContainer->Larvae.begin() ; itLarvaCohort!=itContainer->Larvae.end() ; )
					{
						itLarvaCohort->Number = Binomial_Deviate(itLarvaCohort->Number, 1-PbMsg.larvicide().larvicide_efficacy_on_larvae());
						if (itLarvaCohort->Number==0)
							itLarvaCohort=itContainer->Larvae.erase(itLarvaCohort);
						else
							itLarvaCohort++;
					}

					list<Pupae_Cohort>::iterator itPupaCohort;
					for (itPupaCohort=itContainer->Pupae.begin() ; itPupaCohort!=itContainer->Pupae.end() ; )
					{
						itPupaCohort->Number = Binomial_Deviate(itPupaCohort->Number, 1-PbMsg.larvicide().larvicide_efficacy_on_pupae());
						if (itPupaCohort->Number==0)
							itPupaCohort = itContainer->Pupae.erase(itPupaCohort);
						else
							itPupaCohort++;
					}
				}
		}
	return;
}

void Adulticide_grid (int type, int region, int size, int Date, vector<Building> & House )
{
	size_t House_Number = 0;
	int lower_row=0,upper_row=0,left_column=0,right_column=0;

	int days_after = (Date-PbMsg.adulticide().adulticide_begin_date())%PbMsg.adulticide().adulticide_frequency() ;
	double killing = 1.;

	if (PbMsg.adulticide().adulticide_efficacy()==2.) // special case for Amy's values
	{
		if (days_after>3.5)     
            return ;
		else if (days_after>2.5) 
            killing = 0.25; // KWO: Set to 0.25 from 0.2 to be consistent with NIH_november 2010
		else if (days_after>1.5) 
            killing = 0.6;
		else if (days_after>0.5) 
            killing = 0.95;
		else 
            killing = 0.99;
	} else {
		killing = PbMsg.adulticide().adulticide_efficacy() ;
		if (days_after>0) {
			if (days_after<(0.5+PbMsg.adulticide().adulticide_residual_length()))
				killing *= pow((double)(1-PbMsg.adulticide().adulticide_efficacy_decrease()),days_after);
			else return;
		}
	}
	if (region==1) {
		if (size==0) {
			ErrorMessage(801);
		} else {
			lower_row = (PbMsg.sim().n_row()/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (PbMsg.sim().n_col()/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}
	if (region==2) {
		if (
			(PbMsg.adulticide().adulticide_region_2_ymin() > PbMsg.adulticide().adulticide_region_2_ymax()) 
			|| 
			(PbMsg.adulticide().adulticide_region_2_xmin() > PbMsg.adulticide().adulticide_region_2_xmax()) 
			|| 
			(PbMsg.adulticide().adulticide_region_2_ymax() > PbMsg.sim().n_row()) 
			|| 
			(PbMsg.adulticide().adulticide_region_2_xmax() > PbMsg.sim().n_col())
        ) {
            ErrorMessage(801);
        } else {
            lower_row = PbMsg.adulticide().adulticide_region_2_ymin();
            upper_row = PbMsg.adulticide().adulticide_region_2_ymax();
            left_column = PbMsg.adulticide().adulticide_region_2_xmin();
            right_column = PbMsg.adulticide().adulticide_region_2_xmax();
        }
	}

	for (House_Number = 0 ; House_Number < PbMsg.sim().n_house() ; House_Number++)
		if (
			(region == 0) // if region is everywhere, pass
			||
			(
			 (region==1) // if region is central square, check if house is in the square
			 &&
			 ((House_Number/PbMsg.sim().n_col())>=lower_row)
			 &&
			 ((House_Number/PbMsg.sim().n_col())<=upper_row)
			 &&
			 ((House_Number%PbMsg.sim().n_col())>=left_column)
			 &&
			 ((House_Number%PbMsg.sim().n_col())<=right_column)
			 )
        )
			if (House_Number%((int)(1./(1-PbMsg.adulticide().adulticide_compliance()))))
	{ //?? indent level totally off??
		list<Female_Adult>::iterator itFemale ;

		for (itFemale = House[House_Number].Female_Adults.begin() ; itFemale != House[House_Number].Female_Adults.end() ; )
		{
			if (Random_Number()<killing)
			{
				itFemale = House[House_Number].Female_Adults.erase(itFemale);
			}
			else itFemale++;
		}

		list<Male_Adult_Cohort>::iterator itMaleCohort;

		for (itMaleCohort = House[House_Number].Male_Adult.begin() ; itMaleCohort != House[House_Number].Male_Adult.end() ; )
		{
			int nkilled = Binomial_Deviate(itMaleCohort->Number, killing);
			itMaleCohort->Number -= nkilled ;

			if (itMaleCohort->Number ==0)
				itMaleCohort = House[House_Number].Male_Adult.erase(itMaleCohort);
			else itMaleCohort++;
		}
	}
	return;
}

void Source_Removal_grid (int type, int region, int size, vector<Building> & House )
{
	size_t House_Number = 0;
	size_t lower_row=0,upper_row=0,left_column=0,right_column=0;

	if (region==1) {
		if (size==0) {
			ErrorMessage(803);
		} else {
			lower_row = (PbMsg.sim().n_row()/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (PbMsg.sim().n_col()/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}
	if (region==2) {
		if (
			(PbMsg.sourceremoval().source_removal_region_2_ymin() > PbMsg.sourceremoval().source_removal_region_2_ymax()) 
			|| 
			(PbMsg.sourceremoval().source_removal_region_2_xmin() > PbMsg.sourceremoval().source_removal_region_2_xmax()) 
			|| 
			(PbMsg.sourceremoval().source_removal_region_2_ymax() > PbMsg.sim().n_row()) 
			|| 
			(PbMsg.sourceremoval().source_removal_region_2_xmax() > PbMsg.sim().n_col())
        ) {
            ErrorMessage(803);
        } else {
            lower_row = PbMsg.sourceremoval().source_removal_region_2_ymin();
            upper_row = PbMsg.sourceremoval().source_removal_region_2_ymax();
            left_column = PbMsg.sourceremoval().source_removal_region_2_xmin();
            right_column = PbMsg.sourceremoval().source_removal_region_2_xmax();
        }
	}
	for (House_Number = 0 ; House_Number < PbMsg.sim().n_house() ; House_Number++)
		if (
			(region == 0) // if region is everywhere, pass
			||
			(
			 (region==1) // if region is central square, check if house is in the square
			 &&
			 ((House_Number/PbMsg.sim().n_col())>=lower_row)
			 &&
			 ((House_Number/PbMsg.sim().n_col())<=upper_row)
			 &&
			 ((House_Number%PbMsg.sim().n_col())>=left_column)
			 &&
			 ((House_Number%PbMsg.sim().n_col())<=right_column)
			 )
        )
			if (House_Number%((int)(1./(1-PbMsg.sourceremoval().source_removal_compliance()))))
			{
				vector<Receptacle>::iterator itContainer;
				
				for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; )
				{
					if (Random_Number()<PbMsg.sourceremoval().source_removal_proportion())
					{
						itContainer=House[House_Number].Container.erase(itContainer);
					} else
						itContainer++;
				}
			}
	return;
}

