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
extern int ref ;
extern int Date;

void Larvicide (int type, int region, int size)
{
	vector<Building>::iterator itHouse = House.begin(); 
	int House_Number = 0;
	int lower_row=0,upper_row=0,left_column=0,right_column=0;

	if (region==1)
	{
		if (!(size>0))
		{
			ErrorMessage(801);
		}
		else
		{
			lower_row = (NUMBER_OF_ROWS/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (NUMBER_OF_COLUMNS/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}

	if (region==2)
	{
		if (
			(LARVICIDE_REGION_2_YMIN > LARVICIDE_REGION_2_YMAX) 
			|| 
			(LARVICIDE_REGION_2_XMIN > LARVICIDE_REGION_2_XMAX) 
			|| 
			(LARVICIDE_REGION_2_YMIN < 0) 
			|| 
			(LARVICIDE_REGION_2_YMAX > NUMBER_OF_ROWS) 
			|| 
			(LARVICIDE_REGION_2_XMIN < 0) 
			|| 
			(LARVICIDE_REGION_2_XMAX > NUMBER_OF_COLUMNS)
			)
			{
				ErrorMessage(801);
			}
		else
			{
				lower_row = LARVICIDE_REGION_2_YMIN;
				upper_row = LARVICIDE_REGION_2_YMAX;
				left_column = LARVICIDE_REGION_2_XMIN;
				right_column = LARVICIDE_REGION_2_XMAX;
			}
	}

	for (House_Number = 0 ; House_Number < NUMBER_OF_HOUSES ; House_Number++)
	if (
		(region == 0) // if region is everywhere, pass
		||
		(
		 ((region==1)||(region==2)) // if region is central square or custom rectangle, check if house is in the square
		 &&
		 ((House_Number/NUMBER_OF_COLUMNS)>=lower_row)
		 &&
		 ((House_Number/NUMBER_OF_COLUMNS)<=upper_row)
		 &&
		 ((House_Number%NUMBER_OF_COLUMNS)>=left_column)
		 &&
		 ((House_Number%NUMBER_OF_COLUMNS)<=right_column)
		 )
		)
		if (House_Number%((int)(1./(1-LARVICIDE_COMPLIANCE))))
		{
			vector<Receptacle>::iterator itContainer;
			for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; itContainer++)
				if (Random_Number()<LARVICIDE_CONTAINER_PROBABILITY)
				{
					list<Eggs_Cohort>::iterator itEggCohort;
					for (itEggCohort=itContainer->Eggs.begin() ; itEggCohort!=itContainer->Eggs.end() ; )
					{
						itEggCohort->Number = Binomial_Deviate(itEggCohort->Number, 1-LARVICIDE_EFFICACY_ON_EGGS);
						if (itEggCohort->Number==0) 
							itEggCohort=itContainer->Eggs.erase(itEggCohort);
						else
							itEggCohort++;
					}

					list<Larvae_Cohort>::iterator itLarvaCohort;
					for (itLarvaCohort=itContainer->Larvae.begin() ; itLarvaCohort!=itContainer->Larvae.end() ; )
					{
						itLarvaCohort->Number = Binomial_Deviate(itLarvaCohort->Number, 1-LARVICIDE_EFFICACY_ON_LARVAE);
						if (itLarvaCohort->Number==0)
							itLarvaCohort=itContainer->Larvae.erase(itLarvaCohort);
						else
							itLarvaCohort++;
					}

					list<Pupae_Cohort>::iterator itPupaCohort;
					for (itPupaCohort=itContainer->Pupae.begin() ; itPupaCohort!=itContainer->Pupae.end() ; )
					{
						itPupaCohort->Number = Binomial_Deviate(itPupaCohort->Number, 1-LARVICIDE_EFFICACY_ON_PUPAE);
						if (itPupaCohort->Number==0)
							itPupaCohort = itContainer->Pupae.erase(itPupaCohort);
						else
							itPupaCohort++;
					}
				}
	
		}

	return;
}

void Adulticide (int type, int region, int size)
{
	vector<Building>::iterator itHouse = House.begin();
	int House_Number = 0;
	int lower_row=0,upper_row=0,left_column=0,right_column=0;

	int days_after = (Date-ADULTICIDE_BEGIN_DATE)%ADULTICIDE_FREQUENCY ;
	double killing = 1.;

	if (ADULTICIDE_EFFICACY==2.) // special case for Amy's values
	{
		if (days_after>3.5) return ;
		else if (days_after>2.5) killing = 0.25; // KWO: Set to 0.25 from 0.2 to be consistent with NIH_november 2010
		else if (days_after>1.5) killing = 0.6;
		else if (days_after>0.5) killing = 0.95;
		else killing = 0.99;
	}
	else
	{
		killing = ADULTICIDE_EFFICACY ;
		if (days_after>0)
		{
			if (days_after<(0.5+ADULTICIDE_RESIDUAL_LENGTH))
				killing *= pow((double)(1-ADULTICIDE_EFFICACY_DECREASE),days_after);
			else return;
		}
	}

	if (region==1)
	{
		if (size==0)
		{
			ErrorMessage(801);
		}
		else
		{
			lower_row = (NUMBER_OF_ROWS/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (NUMBER_OF_COLUMNS/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}

	if (region==2)
	{
		if (
			(ADULTICIDE_REGION_2_YMIN > ADULTICIDE_REGION_2_YMAX) 
			|| 
			(ADULTICIDE_REGION_2_XMIN > ADULTICIDE_REGION_2_XMAX) 
			|| 
			(ADULTICIDE_REGION_2_YMIN < 0) 
			|| 
			(ADULTICIDE_REGION_2_YMAX > NUMBER_OF_ROWS) 
			|| 
			(ADULTICIDE_REGION_2_XMIN < 0) 
			|| 
			(ADULTICIDE_REGION_2_XMAX > NUMBER_OF_COLUMNS)
			)
			{
				ErrorMessage(801);
			}
		else
			{
				lower_row = ADULTICIDE_REGION_2_YMIN;
				upper_row = ADULTICIDE_REGION_2_YMAX;
				left_column = ADULTICIDE_REGION_2_XMIN;
				right_column = ADULTICIDE_REGION_2_XMAX;
			}
	}

	for (House_Number = 0 ; House_Number < NUMBER_OF_HOUSES ; House_Number++)
		if (
			(region == 0) // if region is everywhere, pass
			||
			(
			 (region==1) // if region is central square, check if house is in the square
			 &&
			 ((House_Number/NUMBER_OF_COLUMNS)>=lower_row)
			 &&
			 ((House_Number/NUMBER_OF_COLUMNS)<=upper_row)
			 &&
			 ((House_Number%NUMBER_OF_COLUMNS)>=left_column)
			 &&
			 ((House_Number%NUMBER_OF_COLUMNS)<=right_column)
			 )
			)
			if (House_Number%((int)(1./(1-ADULTICIDE_COMPLIANCE))))
	{
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

void Source_Removal (int type, int region, int size)
{
	vector<Building>::iterator itHouse = House.begin();
	int House_Number = 0;
	int lower_row=0,upper_row=0,left_column=0,right_column=0;

	if (region==1)
	{
		if (size==0)
		{
			ErrorMessage(803);
		}
		else
		{
			lower_row = (NUMBER_OF_ROWS/2)-(size/2);
			upper_row = lower_row + size - 1;
			left_column = (NUMBER_OF_COLUMNS/2)-(size/2);
			right_column = left_column + size - 1;
		}
	}

	if (region==2)
	{
		if (
			(SOURCE_REMOVAL_REGION_2_YMIN > SOURCE_REMOVAL_REGION_2_YMAX) 
			|| 
			(SOURCE_REMOVAL_REGION_2_XMIN > SOURCE_REMOVAL_REGION_2_XMAX) 
			|| 
			(SOURCE_REMOVAL_REGION_2_YMIN < 0) 
			|| 
			(SOURCE_REMOVAL_REGION_2_YMAX > NUMBER_OF_ROWS) 
			|| 
			(SOURCE_REMOVAL_REGION_2_XMIN < 0) 
			|| 
			(SOURCE_REMOVAL_REGION_2_XMAX > NUMBER_OF_COLUMNS)
			)
			{
				ErrorMessage(803);
			}
		else
			{
				lower_row = SOURCE_REMOVAL_REGION_2_YMIN;
				upper_row = SOURCE_REMOVAL_REGION_2_YMAX;
				left_column = SOURCE_REMOVAL_REGION_2_XMIN;
				right_column = SOURCE_REMOVAL_REGION_2_XMAX;
			}
	}

	for (House_Number = 0 ; House_Number < NUMBER_OF_HOUSES ; House_Number++)
		if (
			(region == 0) // if region is everywhere, pass
			||
			(
			 (region==1) // if region is central square, check if house is in the square
			 &&
			 ((House_Number/NUMBER_OF_COLUMNS)>=lower_row)
			 &&
			 ((House_Number/NUMBER_OF_COLUMNS)<=upper_row)
			 &&
			 ((House_Number%NUMBER_OF_COLUMNS)>=left_column)
			 &&
			 ((House_Number%NUMBER_OF_COLUMNS)<=right_column)
			 )
			)
			if (House_Number%((int)(1./(1-SOURCE_REMOVAL_COMPLIANCE))))
			{
				vector<Receptacle>::iterator itContainer;
				
				for (itContainer=House[House_Number].Container.begin() ; itContainer!=House[House_Number].Container.end() ; )
				{
					if (Random_Number()<SOURCE_REMOVAL_PROPORTION)
					{
						itContainer=House[House_Number].Container.erase(itContainer);
						House[House_Number].Number_of_Containers--;
					}
					else
						itContainer++;
				}
			}

	return;

}
