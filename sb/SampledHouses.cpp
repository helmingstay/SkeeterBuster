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


SampledHouses::SampledHouses(int number_of_dates_sampled)
	{
	Number_of_Days_Sampled = number_of_dates_sampled;

	dates_houses_sampled.resize( Number_of_Days_Sampled );
	number_of_houses_sampled_on_Date.resize( Number_of_Days_Sampled );

	//Allocate date sampled vectors.
	indices_of_houses_sampled_on_Date = new vector<int>[ Number_of_Days_Sampled ];

	//Allocate sampling results
	Number_of_Eggs_Sampled_on_Date = new vector<int>[ Number_of_Days_Sampled ];
	Number_of_Larvae_Sampled_on_Date = new vector<int>[ Number_of_Days_Sampled ];
	Number_of_Pupae_Sampled_on_Date = new vector<int>[ Number_of_Days_Sampled ];
	Number_of_Adults_Sampled_on_Date = new vector<int>[ Number_of_Days_Sampled ];
	}

SampledHouses::~SampledHouses()
	{
	delete[] indices_of_houses_sampled_on_Date;
	delete[] Number_of_Eggs_Sampled_on_Date;
	delete[] Number_of_Larvae_Sampled_on_Date;
	delete[] Number_of_Pupae_Sampled_on_Date;
	delete[] Number_of_Adults_Sampled_on_Date;
	}

/* Read a text file, SamplingRoutine.txt, that specifies the destructive sampling to be carried out */
void SampledHouses::ReadSamplingFile()
	{
	FILE *Sampling_Routine;
	char c=' ';
	int check;

	fprintf(stdout,"Reading sampling file...");

	// TODO: This requires traversing up two directories (from out/date to the same directory where the setup file is located. This is an inferior solution */
	Sampling_Routine = fopen("../../SamplingRoutine.txt", "r+");
	if (Sampling_Routine==NULL)
		{
			ErrorMessage(6);
		}
	else fprintf(stdout, "OK. \n");

	fseek(Sampling_Routine, 0L, SEEK_SET ); // go to the very beginning of the file

	char cc=' ', dd=' ';

	fprintf(stdout, "Sampling information : \n");
	do
	{
		check=fscanf(Sampling_Routine,"%c",&cc);
		check=fscanf(Sampling_Routine,"%c",&dd);
		fseek(Sampling_Routine, -1, SEEK_CUR); // go back one byte from the current position
	}
	while ((!((cc=='#')&&(dd=='#')))&&(check==EOF? ErrorMessage(44),0 : 1)); // find beginning of log in sampling routine file WHICH IS DELIMITED BY TWO HASH MARKS ("##")

	fseek(Sampling_Routine, 1, SEEK_CUR); // advance one byte

	// find end of log/commentary info in sampling routine file WHICH IS DELIMITED BY SINGLE HASH MARK ("#")
	do
	{
		check=fscanf(Sampling_Routine,"%c",&c);
		if (c!='#') fprintf(stdout, "%c", c);
	 }
	 while ((!(c=='#'))&&(check==EOF? ErrorMessage(44),0 : 1));

	// Check for first "=" sign, store everything up to that in "c", which is going to get you to the SAMPLING_DATES specification in SamplingRoutine.txt
	 do check=fscanf(Sampling_Routine,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(44),0 : 1)));

	for (int i=0; i < Number_of_Days_Sampled; i++)
				{
				int sampling_date;
				// Need to add something that says : "SamplingDates.txt doesn't have Number_of_Days_Sampled values ! "
				fscanf (Sampling_Routine, "%d ", &sampling_date);
				dates_houses_sampled[i] = sampling_date;
				}

	for (int i=0; i < Number_of_Days_Sampled; i++)
				{
				int sampling_date;

				do check=fscanf(Sampling_Routine,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(61),0 : 1))); // proceed to the next equals sign
				check=fscanf(Sampling_Routine,"%d\n",&sampling_date);

				// make sure you are on the correct sampling date
				if (sampling_date == dates_houses_sampled[i])
					{
					// proceed to the next equals sign, which should contain the number of houses to sample
					do check=fscanf(Sampling_Routine,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(61),0 : 1)));
									check=fscanf(Sampling_Routine,"%d\n",&number_of_houses_sampled_on_Date[i]);

					indices_of_houses_sampled_on_Date[i].resize( number_of_houses_sampled_on_Date[i] );

					// proceed to the next equals sign, which should contain the number of houses to sample
					do check=fscanf(Sampling_Routine,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(61),0 : 1)));

					for (int house_to_sample=0; house_to_sample < number_of_houses_sampled_on_Date[i]; house_to_sample++)
						{
						int house_subject_to_sampling;
						// store the index of house that will be sampled into house_subject_to_sampling
						check=fscanf(Sampling_Routine,"%d\n",&house_subject_to_sampling);
						indices_of_houses_sampled_on_Date[i][house_to_sample] = house_subject_to_sampling;
						}
					}
				else
					{
					ErrorMessage(61);
					}
				}
	}

void SampledHouses::SetupSamplingResults(int output_number)
	{
	for (int i=0; i < Number_of_Days_Sampled; i++)
		{
		Number_of_Eggs_Sampled_on_Date[i].resize(number_of_houses_sampled_on_Date[i]);
		Number_of_Larvae_Sampled_on_Date[i].resize(number_of_houses_sampled_on_Date[i]);
		Number_of_Pupae_Sampled_on_Date[i].resize(number_of_houses_sampled_on_Date[i]);
		Number_of_Adults_Sampled_on_Date[i].resize(number_of_houses_sampled_on_Date[i]);
		}
	stringstream filename;
	filename << "larval_sampling_outcome." << output_number << ".txt";
	larvae_sampling_file.open(filename.str().c_str() );
	filename.str("");

	filename << "pupae_sampling_outcome." << output_number << ".txt";
	pupae_sampling_file.open(filename.str().c_str() );
	filename.str("");

	filename << "adult_sampling_outcome." << output_number << ".txt";
	adult_sampling_file.open(filename.str().c_str() );
	filename.str("");
	}

/* Check to see if any houses are sampled on the given date, and return the index for that date. If no houses are being sampled on that date, return -1 */
int SampledHouses::CheckSamplingDate(int day)
	{
	int answer = -1;

	for (int i=0; i < Number_of_Days_Sampled; i++)
		{
		if (day == dates_houses_sampled[i])
			{
			answer = i;
			}
		}
	return answer;
	}

void SampledHouses::writeSamplingResults(int date_of_sampling)
	{
	if (DESTRUCTIVE_LARVAE_SAMPLING)
						{
						for (int house=0; house < Number_of_Larvae_Sampled_on_Date[date_of_sampling].size(); house++)
							larvae_sampling_file << date_of_sampling << " " << indices_of_houses_sampled_on_Date[date_of_sampling][house] << " " << Number_of_Larvae_Sampled_on_Date[date_of_sampling][house] << std::endl;
						}

	if (DESTRUCTIVE_PUPAE_SAMPLING)
						{
						for (int house=0; house < Number_of_Pupae_Sampled_on_Date[date_of_sampling].size(); house++)
							pupae_sampling_file << date_of_sampling << " " << indices_of_houses_sampled_on_Date[date_of_sampling][house] << " " << Number_of_Pupae_Sampled_on_Date[date_of_sampling][house] << std::endl;
						}

	if (DESTRUCTIVE_ADULT_SAMPLING)
						{
						for (int house=0; house < Number_of_Adults_Sampled_on_Date[date_of_sampling].size(); house++)
							adult_sampling_file << date_of_sampling << " " << indices_of_houses_sampled_on_Date[date_of_sampling][house] << " " << Number_of_Adults_Sampled_on_Date[date_of_sampling][house] << std::endl;
						}
	}

void SampledHouses::closeSamplingFiles()
	{
	larvae_sampling_file.close();
	pupae_sampling_file.close();
	adult_sampling_file.close();
	}
