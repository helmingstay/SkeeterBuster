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

// Container_type numbers ={'BATH':1, 'BOTTLE':2, 'CAN':3, 'COOK':4, 'DPLAS':5, 'LTANK':6, 'MISC':7, 'MSTOR':8, 'NATURAL':9, 'ODD':10, 'PET':11, 'TIRE':12, 'VASE':13, 'WELL':14}

#include "stdafx.h"
#include "Aedes.h"
#include "Aedes_class_defs.h"

extern vector<Building> House; 

void ReadHouseSetup()
{
	FILE *HouseSetup;
	char c=' ';
	int check;
	fprintf(stdout,"Opening SETUP file ...");
	HouseSetup=fopen("SkeeterBuster.setup","r+");
	if (HouseSetup==NULL)
	{
		ErrorMessage(4);
	}
	else fprintf(stdout, " OK.\n");

	fseek(HouseSetup, 0L, SEEK_SET );

	char cc=' ', dd=' ';
  
	fprintf(stdout, "Setup information : \n");
	do 
	{
		check=fscanf(HouseSetup,"%c",&cc); 
		check=fscanf(HouseSetup,"%c",&dd);
		fseek(HouseSetup, -1, SEEK_CUR);
	}
	while ((!((cc=='#')&&(dd=='#')))&&(check==EOF? ErrorMessage(44),0 : 1)); // find beginning of log in setup file, WHICH IS DELIMITED BY TWO HASH MARKS ("##")
    
	fseek(HouseSetup, 1, SEEK_CUR); // Move the comment along one
	
	do 
	{
		check=fscanf(HouseSetup,"%c",&c); 
		if (c!='#') fprintf(stdout, "%c", c);
	 }
	 while ((!(c=='#'))&&(check==EOF? ErrorMessage(44),0 : 1)); // find end of log in setup file, which is delimited by a single hash mark ("#")

	// TODO: pass this to sim_log writing somehow 
	 do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(44),0 : 1)));
		check=fscanf(HouseSetup,"%d\n",&SCENARIO);
	do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(44),0 : 1)));
	check=fscanf(HouseSetup,"%d\n",&NUMBER_OF_HOUSES);
	do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
	check=fscanf(HouseSetup,"%d\n",&NUMBER_OF_COLUMNS);
	do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
	check=fscanf(HouseSetup,"%d\n",&NUMBER_OF_ROWS); 
 
	cout << "Houses config : " << NUMBER_OF_HOUSES << " houses, " << NUMBER_OF_COLUMNS << " columns, " << NUMBER_OF_ROWS << " rows: Scenario " << SCENARIO << " \n" ;

	//if (SIMULATION_MODE2==DETERMINISTIC) NUMBER_OF_HOUSES=1; 
 
	//NUMBER_OF_COLUMNS=(int) pow((double) NUMBER_OF_HOUSES,0.5); 
	//NUMBER_OF_ROWS=NUMBER_OF_HOUSES/NUMBER_OF_COLUMNS;
	//printf("%d %d %d\n",NUMBER_OF_HOUSES,NUMBER_OF_ROWS,NUMBER_OF_COLUMNS); 
 
	fprintf(stdout, "\nReading SETUP file ... ");

	if (NUMBER_OF_HOUSES>0) 
	{ 
		int geiger=0 ;
		for (int i=0;i<NUMBER_OF_HOUSES;i++) House.push_back( Building() ); 
		vector<Building>::iterator itHouse; 
		for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ )  
		{ 
			 itHouse->Position = geiger;
			 
			do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1)); //this skips the House_Number line

			//do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1)); 
			//check=fscanf(HouseSetup,"%.5f\n",&itHouse->Attractiveness);
			itHouse->Attractiveness=1.;
			
			if (GORDONVALE_GOLF_COURSE)
				if (((geiger%NUMBER_OF_COLUMNS)<11.5) && ((geiger/NUMBER_OF_COLUMNS)>13.5) && ((geiger/NUMBER_OF_COLUMNS)<21.5))
					itHouse->Attractiveness=0.000001;

			//printf("HOUSE id: %d ", geiger);
			do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&((check==EOF? ErrorMessage(44),0 : 1)));
			check=fscanf(HouseSetup,"%d\n",&itHouse->Number_of_Containers);
			 
			if (itHouse->Number_of_Containers>0)
			{
				itHouse->Reserve_Containers.assign( itHouse->Number_of_Containers, Receptacle() ); 
			// printf("How many containers in house %d?: %d\n", itHouse->Position, itHouse->Number_of_Containers);
				
				itHouse->Number_of_Containers=0;
					//this will now be increased each day when new containers are pushed


				vector<Receptacle>::iterator itContainer;
				for (itContainer=itHouse->Reserve_Containers.begin(); itContainer != itHouse->Reserve_Containers.end(); itContainer++)
				{	 
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1)); //this skips the line with CONTAINER_NUMBER 
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Container_Type_Number);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Release_Date);
					itHouse->ContainerReleaseDates.push_back(itContainer->Release_Date);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Height);
				//	printf("%f\n", itContainer->Height);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Surface);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Covered);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Monthly_Turnover_Rate);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->SunExposure);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Daily_Food_Loss);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Daily_Food_Gain);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Cover_Reduction);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Filling_Method);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Draw_Down);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Watershed_Ratio);
				
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Water_Level);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itContainer->Initial_Food_Present);

					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Number_of_Release_Egg_Cohorts);
					
					if (itContainer->Number_of_Release_Egg_Cohorts>0)
					{
						itContainer->ReleaseEggs.assign( itContainer->Number_of_Release_Egg_Cohorts, Eggs_Cohort() );
						list<Eggs_Cohort>::iterator itReleaseEggs;
						for (itReleaseEggs= itContainer->ReleaseEggs.begin();itReleaseEggs != itContainer->ReleaseEggs.end(); itReleaseEggs++)
						{
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseEggs->Number);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseEggs->Level_laid_in_Container);					
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseEggs->Physiological_Development_Percent);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseEggs->Age);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseEggs->Mature);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseEggs->Release_Date);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseEggs->Genotype);	
							//if (itReleaseEggs->Genotype > pow((double)2,2*CHROM_NUMBER))
							//{
							//	fprintf(stdout, "\n*** Warning: Egg genotype %d found is not compatible with %d chromosomes ", itReleaseEggs->Genotype, CHROM_NUMBER);
							//	do CHROM_NUMBER++; while (itReleaseEggs->Genotype > pow((double)2,2*CHROM_NUMBER));
							//	fprintf(stdout, "\n*** The number of chromosomes has been adjusted to %d .. continuing ", CHROM_NUMBER);
							//}
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseEggs->Wolbachia);	
						}
					}				
					
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Number_of_Release_Larval_Cohorts);
					
					if (itContainer->Number_of_Release_Larval_Cohorts>0)
					{
						itContainer->ReleaseLarvae.assign( itContainer->Number_of_Release_Larval_Cohorts, Larvae_Cohort() );
						list<Larvae_Cohort>::iterator itReleaseLarvae;
						for (itReleaseLarvae= itContainer->ReleaseLarvae.begin();itReleaseLarvae != itContainer->ReleaseLarvae.end(); itReleaseLarvae++)
						{
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseLarvae->Number);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseLarvae->Physiological_Development_Percent);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseLarvae->Age);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseLarvae->Mature);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseLarvae->Release_Date);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleaseLarvae->Average_Larval_Weight);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseLarvae->Genotype);
			/*				if (itReleaseLarvae->Genotype > pow((double)2,2*CHROM_NUMBER))
							{
								fprintf(stdout, "\n*** Warning: Larval genotype %d found is not compatible with %d chromosomes ", itReleaseLarvae->Genotype, CHROM_NUMBER);
								do CHROM_NUMBER++; while (itReleaseLarvae->Genotype > pow((double)2,2*CHROM_NUMBER));
								fprintf(stdout, "\n*** The number of chromosomes has been adjusted to %d .. continuing ", CHROM_NUMBER);
							}
			*/				if (itReleaseLarvae->Genotype & 1) itReleaseLarvae->Sex=1; else itReleaseLarvae->Sex=0; 
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleaseLarvae->Wolbachia);

						}
					}
					
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itContainer->Number_of_Release_Pupal_Cohorts);		 
					 
					if (itContainer->Number_of_Release_Pupal_Cohorts>0) 
					{ 
						itContainer->ReleasePupae.assign( itContainer->Number_of_Release_Pupal_Cohorts, Pupae_Cohort() );
						list<Pupae_Cohort>::iterator itReleasePupae;
						for (itReleasePupae= itContainer->ReleasePupae.begin();itReleasePupae != itContainer->ReleasePupae.end(); itReleasePupae++)
						{
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleasePupae->Number);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleasePupae->Physiological_Development_Percent);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleasePupae->Age);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleasePupae->Average_Pupal_Weight);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%lg\n",&itReleasePupae->Average_Pupal_Lipid_Reserve);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleasePupae->Mature);
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleasePupae->Genotype);
			/*				if (itReleasePupae->Genotype > pow((double)2,2*CHROM_NUMBER))
							{
								fprintf(stdout, "\n*** Warning: Pupal genotype %d found is not compatible with %d chromosomes ", itReleasePupae->Genotype, CHROM_NUMBER);
								do CHROM_NUMBER++; while (itReleasePupae->Genotype > pow((double)2,2*CHROM_NUMBER));
								fprintf(stdout, "\n*** The number of chromosomes has been adjusted to %d .. continuing ", CHROM_NUMBER);
							}
			*/				do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleasePupae->Release_Date);
							if (itReleasePupae->Genotype & 1) itReleasePupae->Sex=1; else itReleasePupae->Sex=0; 
							do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
							check=fscanf(HouseSetup,"%d\n",&itReleasePupae->Wolbachia);
										
						} 
					}

				} 
			}

			if (GORDONVALE_REMOVE_173_195)
				if ((geiger==195)||(geiger==173)) itHouse->Reserve_Containers.clear();
			

			do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
			check=fscanf(HouseSetup,"%d\n",&itHouse->Number_of_Release_Female_Adult_Cohorts); 

			if (itHouse->Number_of_Release_Female_Adult_Cohorts>0) 
			{
				int total_number=0;
			 	for (int cohort=0 ; cohort<itHouse->Number_of_Release_Female_Adult_Cohorts ; cohort++)
				{
					int number_in_cohort=0;
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&number_in_cohort);

					//tmp
					total_number+=number_in_cohort;

					Female_Adult typical(0,0,0,0,0,0,0,0,0,0,0,0,0);

					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&typical.Physiological_Development_Percent);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Age);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&typical.Average_Adult_Weight);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Mature);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Nulliparous);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Genotype);
	/*				if (typical.Genotype > pow((double)2,2*CHROM_NUMBER))
					{
						fprintf(stdout, "\n*** Warning: Female adult genotype %d found is not compatible with %d chromosomes ", typical.Genotype, CHROM_NUMBER);
						do CHROM_NUMBER++; while (typical.Genotype > pow((double)2,2*CHROM_NUMBER));
						fprintf(stdout, "\n*** The number of chromosomes has been adjusted to %d .. continuing ", CHROM_NUMBER);
					}
	*/				do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Wolbachia);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Release_Date);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Mated);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&typical.Male_Genotype);
					for (int i=0 ; i<number_in_cohort ; i++)
						itHouse->Release_Female_Adult.push_back(typical);
					

				}
				//tmp
				if (itHouse->Release_Female_Adult.size() != total_number) 
				{
					ErrorMessage(41);
				}
			}

				
			
 
			do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
			check=fscanf(HouseSetup,"%d\n",&itHouse->Number_of_Release_Male_Adult_Cohorts);

			if (itHouse->Number_of_Release_Male_Adult_Cohorts>0) 
			{ 
				itHouse->Release_Male_Adult.assign( itHouse->Number_of_Release_Male_Adult_Cohorts, Male_Adult_Cohort() );
				list<Male_Adult_Cohort>::iterator itRelease_Male_Adult;
				for (itRelease_Male_Adult= itHouse->Release_Male_Adult.begin();itRelease_Male_Adult != itHouse->Release_Male_Adult.end(); itRelease_Male_Adult++)
				{
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itRelease_Male_Adult->Number);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itRelease_Male_Adult->Physiological_Development_Percent);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itRelease_Male_Adult->Age);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%lg\n",&itRelease_Male_Adult->Average_Adult_Weight);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itRelease_Male_Adult->Mature);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itRelease_Male_Adult->Genotype);
					//if (itRelease_Male_Adult->Genotype > pow((double)2,2*CHROM_NUMBER))
					//{
					//	fprintf(stdout, "\n*** Warning: Male adult genotype %d found is not compatible with %d chromosomes ", itRelease_Male_Adult->Genotype, CHROM_NUMBER);
					//	do CHROM_NUMBER++; while (itRelease_Male_Adult->Genotype > pow((double)2,2*CHROM_NUMBER));
					//	fprintf(stdout, "\n*** The number of chromosomes has been adjusted to %d .. continuing ", CHROM_NUMBER);
					//}
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itRelease_Male_Adult->Wolbachia);
					do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check==EOF? ErrorMessage(44),0 : 1));
					check=fscanf(HouseSetup,"%d\n",&itRelease_Male_Adult->Release_Date);
					
				} 
			} 
			itHouse->Maximum_Daily_Air_Temperature.assign(NUMBER_OF_DAYS,0); 
			itHouse->Minimum_Daily_Air_Temperature.assign(NUMBER_OF_DAYS,0); 
			itHouse->Average_Daily_Air_Temperature.assign(NUMBER_OF_DAYS,0); 
			itHouse->Precipitation.assign(NUMBER_OF_DAYS,0); 
			itHouse->Relative_Humidity.assign(NUMBER_OF_DAYS,0); 
			itHouse->Saturation_Deficit.assign(NUMBER_OF_DAYS,0);      

			geiger++;
			}
		}
	
		// file reading should be finished at this point
		// controls:
		// - if the file is missing info, EOF will be caught by one of the fscanf
		// - we check here if the file has extra info that should NOT be here

		do check=fscanf(HouseSetup,"%c",&c); while ((c!='=')&&(check!=EOF));
		// this line SHOULD catch EOF
		if (check!=EOF)
			ErrorMessage(45);


	fclose(HouseSetup);

	fprintf(stdout, "OK.\n");
}


void ReadConfigFile() 
{ 
	fprintf(stdout, "Opening CONF file ...");
	
	ConfigFile config("SkeeterBuster.conf"); 

	fprintf(stdout, " OK.\n");
	fprintf(stdout, "Reading CONF file ...");

	config.readInto(SIMULATION_MODE1,"SIMULATION_MODE1");  
	config.readInto(SIMULATION_MODE2,"SIMULATION_MODE2"); 
	fprintf(stdout, "%d", SIMULATION_MODE2);
	if (SIMULATION_MODE1==CIMSIM)
		SIMULATION_MODE2=DETERMINISTIC;
	config.readInto(REPRODUCTION_MODE,"REPRODUCTION_MODE"); 
	config.readInto(DEVELOPMENT_MODE,"DEVELOPMENT_MODE"); 
	config.readInto(DISPERSAL_MODE,"DISPERSAL_MODE"); 
	config.readInto(IS_IQUITOS,"IS_IQUITOS");
	config.readInto(OVIPOSIT_EGGS_INDEPENDENTLY, "OVIPOSIT_EGGS_INDEPENDENTLY");
	config.readInto(PROBABILITY_CLUTCH_PARTITIONED, "PROBABILITY_CLUTCH_PARTITIONED");
	config.readInto(MAXIMUM_NUMBER_OF_EGG_PARTITIONS, "MAXIMUM_NUMBER_OF_EGG_PARTITIONS");
	config.readInto(PROPORTION_IN_PARTITION_0, "PROPORTION_IN_PARTITION_0");
	config.readInto(PROPORTION_IN_PARTITION_1, "PROPORTION_IN_PARTITION_1");
	config.readInto(PROPORTION_IN_PARTITION_2, "PROPORTION_IN_PARTITION_2");
	config.readInto(PROPORTION_IN_PARTITION_3, "PROPORTION_IN_PARTITION_3");
	config.readInto(PROPORTION_IN_PARTITION_4, "PROPORTION_IN_PARTITION_4");
	config.readInto(PROPORTION_IN_PARTITION_5, "PROPORTION_IN_PARTITION_5");
	config.readInto(PROPORTION_IN_PARTITION_6, "PROPORTION_IN_PARTITION_6");
	config.readInto(PROPORTION_IN_PARTITION_7, "PROPORTION_IN_PARTITION_7");
	config.readInto(PROPORTION_IN_PARTITION_8, "PROPORTION_IN_PARTITION_8");
	config.readInto(PROPORTION_IN_PARTITION_9, "PROPORTION_IN_PARTITION_9");

	config.readInto(LARVAL_EFFECT_ON_OVIPOSITION_PROBABILITY, "LARVAL_EFFECT_ON_OVIPOSITION_PROBABILITY");

	config.readInto(CONTAINER_MULTIPLIER_FACTOR,"CONTAINER_MULTIPLIER_FACTOR");

	config.readInto(INCREASED_CONTAINER_VARIANCE,"INCREASED_CONTAINER_VARIANCE");
	config.readInto(INCREASED_VARIANCE_EVERY_N_HOUSES,"INCREASED_VARIANCE_EVERY_N_HOUSES");

 
	config.readInto(NUMBER_OF_DAYS,"NUMBER_OF_DAYS"); 
 
	config.readInto(NOPUPATION,"NOPUPATION"); 
	config.readInto(MARK_RELEASE_RECAPTURE,"MARK_RELEASE_RECAPTURE"); 
 
	config.readInto(DATED_OUTPUT,"DATED_OUTPUT"); 
	config.readInto(LOG_PER_HOUSE,"LOG_PER_HOUSE"); 
	config.readInto(OUTPUT_FOR_SBEED,"OUTPUT_FOR_SBEED"); 
	config.readInto(OUTPUT_PER_CONTAINER_TYPE,"OUTPUT_PER_CONTAINER_TYPE");
	config.readInto(OUTPUT_PER_CONTAINER,"OUTPUT_PER_CONTAINER");
	config.readInto(OUTPUT_WATER_LEVEL,"OUTPUT_WATER_LEVEL");
	config.readInto(OUTPUT_PER_CONTAINER_START_DATE,"OUTPUT_PER_CONTAINER_START_DATE");
	config.readInto(DETAILED_LOG,"DETAILED_LOG"); 
	if (DETAILED_LOG) 
	{ 
		config.readInto(DETAILED_LOG_START,"DETAILED_LOG_START"); 
		config.readInto(DETAILED_LOG_END,"DETAILED_LOG_END"); 
	} 
 
	config.readInto(EPS_OUTPUT,"EPS_OUTPUT"); 
	if (EPS_OUTPUT) config.readInto(EPS_OUTPUT_FREQ,"EPS_OUTPUT_FREQ"); 
	config.readInto(EPS_WOLBACHIA_OUTPUT,"EPS_WOLBACHIA_OUTPUT"); 
	if (EPS_WOLBACHIA_OUTPUT) config.readInto(EPS_WOLBACHIA_OUTPUT_FREQ,"EPS_WOLBACHIA_OUTPUT_FREQ"); 
	config.readInto(EPS_DENSITY_OUTPUT,"EPS_DENSITY_OUTPUT"); 
	if (EPS_DENSITY_OUTPUT) config.readInto(EPS_DENSITY_OUTPUT_FREQ,"EPS_DENSITY_OUTPUT_FREQ"); 
	config.readInto(EPS_XT_OUTPUT,"EPS_XT_OUTPUT"); 
	config.readInto(EPS_YT_OUTPUT,"EPS_YT_OUTPUT"); 
	config.readInto(SPATIAL_STATS,"SPATIAL_STATS"); 
	config.readInto(FSTATS,"FSTATS"); 
	config.readInto(TEST_1,"TEST_1");
	config.readInto(TEST_2,"TEST_2");
	config.readInto(GENTIME,"GENTIME");
	config.readInto(LARVTIME,"LARVTIME");
	
	config.readInto(HTML_OUTPUT,"HTML_OUTPUT"); 
	config.readInto(DONT_POPUP,"DONT_POPUP"); 
 
	config.readInto(DESTINATION,"DESTINATION"); 
	if (DESTINATION==CUSTOM_DESTINATION) 
	{ 
		config.readInto(RESULTS_DIR,"RESULTS_DIR"); 
		config.readInto(GNUPLOT_DIR,"GNUPLOT_DIR"); 
	} 
 
	config.readInto(SAVE_FINAL_STATE_AS_SETUP_FILE,"SAVE_FINAL_STATE_AS_SETUP_FILE");
	config.readInto(ZIPPED_OUTPUT,"ZIPPED_OUTPUT");

	config.readInto(CHROM_NUMBER,"CHROM_NUMBER"); 
	config.readInto(GENETIC_CONTROL_METHOD,"GENETIC_CONTROL_METHOD"); 

	config.readInto(TRADITIONAL_CONTROL_METHOD,"TRADITIONAL_CONTROL_METHOD");

	config.readInto(ADULTICIDE_BEGIN_DATE,"ADULTICIDE_BEGIN_DATE");
	config.readInto(ADULTICIDE_END_DATE,"ADULTICIDE_END_DATE");
	config.readInto(ADULTICIDE_FREQUENCY,"ADULTICIDE_FREQUENCY");
	config.readInto(ADULTICIDE_TYPE,"ADULTICIDE_TYPE");
	config.readInto(ADULTICIDE_RESIDUAL_LENGTH,"ADULTICIDE_RESIDUAL_LENGTH");
	config.readInto(ADULTICIDE_REGION,"ADULTICIDE_REGION");
	config.readInto(ADULTICIDE_SIZE,"ADULTICIDE_SIZE");
	config.readInto(ADULTICIDE_REGION_2_XMIN,"ADULTICIDE_REGION_2_XMIN");
	config.readInto(ADULTICIDE_REGION_2_XMAX,"ADULTICIDE_REGION_2_XMAX");
	config.readInto(ADULTICIDE_REGION_2_YMIN,"ADULTICIDE_REGION_2_YMIN");
	config.readInto(ADULTICIDE_REGION_2_YMAX,"ADULTICIDE_REGION_2_YMAX");
	config.readInto(ADULTICIDE_EFFICACY,"ADULTICIDE_EFFICACY");
	config.readInto(ADULTICIDE_EFFICACY_DECREASE,"ADULTICIDE_EFFICACY_DECREASE");
	config.readInto(ADULTICIDE_COMPLIANCE,"ADULTICIDE_COMPLIANCE");

	config.readInto(LARVICIDE_BEGIN_DATE,"LARVICIDE_BEGIN_DATE");
	config.readInto(LARVICIDE_END_DATE,"LARVICIDE_END_DATE");
	config.readInto(LARVICIDE_FREQUENCY,"LARVICIDE_FREQUENCY");
	config.readInto(LARVICIDE_TYPE,"LARVICIDE_TYPE");
	config.readInto(LARVICIDE_REGION,"LARVICIDE_REGION");
	config.readInto(LARVICIDE_SIZE,"LARVICIDE_SIZE");
	config.readInto(LARVICIDE_REGION_2_XMIN,"LARVICIDE_REGION_2_XMIN");
	config.readInto(LARVICIDE_REGION_2_XMAX,"LARVICIDE_REGION_2_XMAX");
	config.readInto(LARVICIDE_REGION_2_YMIN,"LARVICIDE_REGION_2_YMIN");
	config.readInto(LARVICIDE_REGION_2_YMAX,"LARVICIDE_REGION_2_YMAX");
	config.readInto(LARVICIDE_CONTAINER_PROBABILITY,"LARVICIDE_CONTAINER_PROBABILITY");
	config.readInto(LARVICIDE_EFFICACY_ON_EGGS,"LARVICIDE_EFFICACY_ON_EGGS");
	config.readInto(LARVICIDE_EFFICACY_ON_LARVAE,"LARVICIDE_EFFICACY_ON_LARVAE");
	config.readInto(LARVICIDE_EFFICACY_ON_PUPAE,"LARVICIDE_EFFICACY_ON_PUPAE");
	config.readInto(LARVICIDE_COMPLIANCE,"LARVICIDE_COMPLIANCE");

	config.readInto(SOURCE_REMOVAL_BEGIN_DATE,"SOURCE_REMOVAL_BEGIN_DATE");
	config.readInto(SOURCE_REMOVAL_TYPE,"SOURCE_REMOVAL_TYPE");
	config.readInto(SOURCE_REMOVAL_REGION,"SOURCE_REMOVAL_REGION");
	config.readInto(SOURCE_REMOVAL_SIZE,"SOURCE_REMOVAL_SIZE");
	config.readInto(SOURCE_REMOVAL_REGION_2_XMIN,"SOURCE_REMOVAL_REGION_2_XMIN");
	config.readInto(SOURCE_REMOVAL_REGION_2_XMAX,"SOURCE_REMOVAL_REGION_2_XMAX");
	config.readInto(SOURCE_REMOVAL_REGION_2_YMIN,"SOURCE_REMOVAL_REGION_2_YMIN");
	config.readInto(SOURCE_REMOVAL_REGION_2_YMAX,"SOURCE_REMOVAL_REGION_2_YMAX");
	config.readInto(SOURCE_REMOVAL_PROPORTION,"SOURCE_REMOVAL_PROPORTION");
	config.readInto(SOURCE_REMOVAL_COMPLIANCE,"SOURCE_REMOVAL_COMPLIANCE");

	config.readInto(CONDUCT_DESTRUCTIVE_SAMPLING, "CONDUCT_DESTRUCTIVE_SAMPLING");
	config.readInto(DESTRUCTIVE_LARVAE_SAMPLING, "DESTRUCTIVE_LARVAE_SAMPLING");
	config.readInto(DESTRUCTIVE_ADULT_SAMPLING, "DESTRUCTIVE_ADULT_SAMPLING");
	config.readInto(DESTRUCTIVE_PUPAE_SAMPLING, "DESTRUCTIVE_PUPAE_SAMPLING");

	config.readInto(NUMBER_OF_DAYS_SAMPLED, "NUMBER_OF_DAYS_SAMPLED");
	config.readInto(SAMPLE_CONTAINER_PROBABILITY, "CONDUCT_DESTRUCTIVE_SAMPLING");
	config.readInto(PROPORTION_OF_EGGS_SAMPLED, "PROPORTION_OF_EGGS_SAMPLED");
	config.readInto(PROPORTION_OF_LARVAE_SAMPLED, "PROPORTION_OF_LARVAE_SAMPLED");
	config.readInto(PROPORTION_OF_ADULT_FEMALES_SAMPLED, "PROPORTION_OF_ADULT_FEMALES_SAMPLED");
	config.readInto(PROPORTION_OF_ADULT_MALES_SAMPLED, "PROPORTION_OF_ADULT_MALES_SAMPLED");
	config.readInto(PROPORTION_OF_PUPAE_SAMPLED, "PROPORTION_OF_PUPAE_SAMPLED");

	config.readInto(OVITRAP_FEMALE_KILLING_EFFICIENCY,"OVITRAP_FEMALE_KILLING_EFFICIENCY");

	config.readInto(FITNESS_COST_STAGE,"FITNESS_COST_STAGE");
	config.readInto(FITNESS_COST_STAGE_RR,"FITNESS_COST_STAGE_RR");
	config.readInto(TETRACYCLINE, "TETRACYCLINE");
	config.readInto(FITNESS_COST_STAGE_SPECIFIC_AGE,"FITNESS_COST_STAGE_SPECIFIC_AGE");
	config.readInto(EMBRYONIC_COST_SELECTION_TYPE,"EMBRYONIC_COST_SELECTION_TYPE");
	config.readInto(REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS,"REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS");
	config.readInto(REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS_DOMINANCE,"REDUCTION_IN_TRANSGENIC_MALES_MATING_COMPETITIVENESS_DOMINANCE");
 
	if (GENETIC_CONTROL_METHOD==ENGINEERED_UNDERDOMINANCE) 
	{ 
		config.readInto(EU_TYPE,"EU_TYPE"); 
		config.readInto(FITNESS_COST_CONSTRUCT_ALPHA,"FITNESS_COST_CONSTRUCT_ALPHA"); 
		config.readInto(FITNESS_COST_CONSTRUCT_BETA,"FITNESS_COST_CONSTRUCT_BETA"); 
		config.readInto(FITNESS_COST_CONSTRUCT_GAMMA,"FITNESS_COST_CONSTRUCT_GAMMA"); 
		config.readInto(FITNESS_COST_CONSTRUCT_DELTA,"FITNESS_COST_CONSTRUCT_DELTA"); 
		config.readInto(FITNESS_COST_DOMINANCE,"FITNESS_COST_DOMINANCE"); 
	} 

	config.readInto(NUMBER_OF_INCOMPATIBLE_WOLBACHIA,"NUMBER_OF_INCOMPATIBLE_WOLBACHIA");
	config.readInto(WOLBACHIA_INFECTED_FEMALE_FECUNDITY_LOSS,"WOLBACHIA_INFECTED_FEMALE_FECUNDITY_LOSS");
	config.readInto(WOLBACHIA_LEVEL_OF_INCOMPATIBILITY,"WOLBACHIA_LEVEL_OF_INCOMPATIBILITY");
	config.readInto(WOLBACHIA_MATERNAL_TRANSMISSION,"WOLBACHIA_MATERNAL_TRANSMISSION");
	config.readInto(WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_ONSET_AGE,"WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_ONSET_AGE");
	config.readInto(WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_FACTOR, "WOLBACHIA_SURVIVAL_REDUCTION_FEMALE_FACTOR");
	config.readInto(WOLBACHIA_SURVIVAL_REDUCTION_MALE_ONSET_AGE, "WOLBACHIA_SURVIVAL_REDUCTION_MALE_ONSET_AGE");
	config.readInto(WOLBACHIA_SURVIVAL_REDUCTION_MALE_FACTOR, "WOLBACHIA_SURVIVAL_REDUCTION_MALE_FACTOR");
	config.readInto(WOLBACHIA_EGG_SURVIVAL_REDUCTION_ONSET_AGE, "WOLBACHIA_EGG_SURVIVAL_REDUCTION_ONSET_AGE");
	config.readInto(WOLBACHIA_EGG_SURVIVAL_REDUCTION_FACTOR, "WOLBACHIA_EGG_SURVIVAL_REDUCTION_FACTOR");


	if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE) 
	{ 
		config.readInto(MD_TYPE,"MD_TYPE"); 
		config.readInto(FITNESS_COST_INSENSITIVE_TRANSGENE,"FITNESS_COST_INSENSITIVE_TRANSGENE"); 
		config.readInto(FITNESS_COST_INSENSITIVE_NATURAL,"FITNESS_COST_INSENSITIVE_NATURAL"); 
		config.readInto(FITNESS_COST_DRIVE_GENE,"FITNESS_COST_DRIVE_GENE"); 
		config.readInto(DRIVE_STRENGTH_SN,"DRIVE_STRENGTH_SN"); 
		config.readInto(DRIVE_STRENGTH_IT,"DRIVE_STRENGTH_IT"); 
		config.readInto(DRIVE_STRENGTH_IN,"DRIVE_STRENGTH_IN"); 
		config.readInto(DRIVE_STRENGTH_MN,"DRIVE_STRENGTH_MN"); 
		config.readInto(MD_FITNESS_COST_DOMINANCE,"MD_FITNESS_COST_DOMINANCE"); 
	} 
	
	if (GENETIC_CONTROL_METHOD==MEDEA)
	{
		config.readInto(MEDEA_UNITS, "MEDEA_UNITS");
		config.readInto(MEDEA_CROSS_RESCUE, "MEDEA_CROSS_RESCUE");
		config.readInto(MEDEA_MATERNAL_LETHALITY, "MEDEA_MATERNAL_LETHALITY");
		config.readInto(MEDEA_MATERNAL_FECUNDITY_LOSS, "MEDEA_MATERNAL_FECUNDITY_LOSS");
		config.readInto(MEDEA_MATERNAL_FECUNDITY_LOSS_DOMINANCE, "MEDEA_MATERNAL_FECUNDITY_LOSS_DOMINANCE");
		config.readInto(MEDEA_FITNESS_COST_PER_CONSTRUCT, "MEDEA_FITNESS_COST_PER_CONSTRUCT");
		config.readInto(MEDEA_FITNESS_COST_DOMINANCE, "MEDEA_FITNESS_COST_DOMINANCE");
		config.readInto(MEDEA_FEMALE_SPECIFIC_FITNESS_COST, "MEDEA_FEMALE_SPECIFIC_FITNESS_COST");

	}

	if (GENETIC_CONTROL_METHOD==FKR)
	{
		config.readInto(IS_RESCUE,"IS_RESCUE");
		config.readInto(NUMBER_OF_FEMALE_KILLING_LOCI,"NUMBER_OF_FEMALE_KILLING_LOCI");
		config.readInto(FEMALE_KILLING_EFFICIENCY,"FEMALE_KILLING_EFFICIENCY");
		config.readInto(FKR_IS_EMBRYO_KILLING,"FKR_IS_EMBRYO_KILLING");
		config.readInto(KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST,"KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST");
		config.readInto(KILLING_ALLELE_FITNESS_COST_DOMINANCE,"KILLING_ALLELE_FITNESS_COST_DOMINANCE");
		config.readInto(RESCUE_ALLELE_HOMOZYGOUS_FITNESS_COST,"RESCUE_ALLELE_HOMOZYGOUS_FITNESS_COST");
		config.readInto(RESCUE_ALLELE_FITNESS_COST_DOMINANCE,"RESCUE_ALLELE_FITNESS_COST_DOMINANCE");
	}

if (GENETIC_CONTROL_METHOD==Male_Killing)
	{
	config.readInto(MALE_KILLING_EFFICIENCY, "MALE_KILLING_EFFICIENCY");
	config.readInto(NUMBER_OF_MALE_KILLING_LOCI, "NUMBER_OF_MALE_KILLING_LOCI");
	}

if (GENETIC_CONTROL_METHOD==ReduceReplace)
	{
		config.readInto(NUMBER_OF_ADDITIONAL_LOCI,"NUMBER_OF_ADDITIONAL_LOCI");
		config.readInto(NUMBER_OF_FEMALE_KILLING_LOCI,"NUMBER_OF_FEMALE_KILLING_LOCI");
		config.readInto(FEMALE_KILLING_EFFICIENCY,"FEMALE_KILLING_EFFICIENCY");
		config.readInto(FKR_IS_EMBRYO_KILLING,"FKR_IS_EMBRYO_KILLING");
		config.readInto(KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST,"KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST");
		config.readInto(KILLING_ALLELE_FITNESS_COST_DOMINANCE,"KILLING_ALLELE_FITNESS_COST_DOMINANCE");
		config.readInto(TRANSGENE_ALLELE_HOMOZYGOUS_FITNESS_COST,"TRANSGENE_ALLELE_HOMOZYGOUS_FITNESS_COST");
		config.readInto(TRANSGENE_ALLELE_FITNESS_COST_DOMINANCE,"TRANSGENE_ALLELE_FITNESS_COST_DOMINANCE");
	}

	if (GENETIC_CONTROL_METHOD==SELECTION)
	{
		config.readInto(SELECTIVE_FITNESS_COST,"SELECTIVE_FITNESS_COST");
		config.readInto(SELECTIVE_FITNESS_COST_DOMINANCE,"SELECTIVE_FITNESS_COST_DOMINANCE");
	}

	config.readInto(SAD_INTRODUCTION,"SAD_INTRODUCTION");
	config.readInto(SAD_INTRODUCTION_DATE,"SAD_INTRODUCTION_DATE");
	config.readInto(SAD_INTRODUCTION_REGION,"SAD_INTRODUCTION_REGION");
	config.readInto(SAD_INTRODUCTION_X_MIN,"SAD_INTRODUCTION_X_MIN");
	config.readInto(SAD_INTRODUCTION_X_MAX,"SAD_INTRODUCTION_X_MAX");
	config.readInto(SAD_INTRODUCTION_Y_MIN,"SAD_INTRODUCTION_Y_MIN");
	config.readInto(SAD_INTRODUCTION_Y_MAX,"SAD_INTRODUCTION_Y_MAX");
	config.readInto(SAD_INTRODUCTION_FREQUENCY,"SAD_INTRODUCTION_FREQUENCY");
	config.readInto(SAD_INTRODUCTION_FEMALE_GENOTYPE,"SAD_INTRODUCTION_FEMALE_GENOTYPE");


	config.readInto(R,"R"); 
 
	config.readInto(RO25_EMBRYONATION,"RO25_EMBRYONATION"); 
	config.readInto(DHA_EMBRYONATION,"DHA_EMBRYONATION"); 
	config.readInto(DHH_EMBRYONATION,"DHH_EMBRYONATION"); 
	config.readInto(THALF_EMBRYONATION,"THALF_EMBRYONATION"); 
 
	config.readInto(RO25_LARVAL_DEVELOPMENT,"RO25_LARVAL_DEVELOPMENT"); 
	config.readInto(DHA_LARVAL_DEVELOPMENT,"DHA_LARVAL_DEVELOPMENT"); 
	config.readInto(DHH_LARVAL_DEVELOPMENT,"DHH_LARVAL_DEVELOPMENT"); 
	config.readInto(THALF_LARVAL_DEVELOPMENT,"THALF_LARVAL_DEVELOPMENT"); 
 
	config.readInto(RO25_PUPAL_DEVELOPMENT,"RO25_PUPAL_DEVELOPMENT"); 
	config.readInto(DHA_PUPAL_DEVELOPMENT,"DHA_PUPAL_DEVELOPMENT"); 
	config.readInto(DHH_PUPAL_DEVELOPMENT,"DHH_PUPAL_DEVELOPMENT"); 
	config.readInto(THALF_PUPAL_DEVELOPMENT,"THALF_PUPAL_DEVELOPMENT"); 
 
	config.readInto(RO25_GONOTROPHIC_CYCLE,"RO25_GONOTROPHIC_CYCLE"); 
	config.readInto(DHA_GONOTROPHIC_CYCLE,"DHA_GONOTROPHIC_CYCLE"); 
	config.readInto(DHH_GONOTROPHIC_CYCLE,"DHH_GONOTROPHIC_CYCLE"); 
	config.readInto(THALF_GONOTROPHIC_CYCLE,"THALF_GONOTROPHIC_CYCLE"); 
 
	config.readInto(PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMBRYONATION,"PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMBRYONATION"); 
	config.readInto(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_PUPATION,"SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_PUPATION"); 
	config.readInto(CIMSIM_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_PUPATION,"CIMSIM_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_PUPATION"); 
	config.readInto(PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMERGENCE,"PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_EMERGENCE"); 
	config.readInto(PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST_GONOTROPHIC_CYCLE,"PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST_GONOTROPHIC_CYCLE"); 
	config.readInto(PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LATER_GONOTROPHIC_CYCLES,"PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LATER_GONOTROPHIC_CYCLES"); 
 
	config.readInto(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST,"SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_FIRST"); 
	config.readInto(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST,"SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_LAST"); 
	config.readInto(SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_SHAPE,"SKEETER_BUSTER_PHYSIOLOGICAL_DEVELOPMENT_THRESHOLD_SHAPE"); 
 
	config.readInto(NOMINAL_DAILY_SURVIVAL_EGGS,"NOMINAL_DAILY_SURVIVAL_EGGS"); 
	config.readInto(NOMINAL_DAILY_SURVIVAL_FEMALE_LARVAE,"NOMINAL_DAILY_SURVIVAL_FEMALE_LARVAE"); 
	config.readInto(NOMINAL_DAILY_SURVIVAL_MALE_LARVAE,"NOMINAL_DAILY_SURVIVAL_MALE_LARVAE"); 
	config.readInto(NOMINAL_DAILY_SURVIVAL_FEMALE_PUPAE,"NOMINAL_DAILY_SURVIVAL_FEMALE_PUPAE"); 
	config.readInto(NOMINAL_DAILY_SURVIVAL_MALE_PUPAE,"NOMINAL_DAILY_SURVIVAL_MALE_PUPAE"); 
	config.readInto(CIMSIM_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS,"CIMSIM_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS"); 
	config.readInto(CIMSIM_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS,"CIMSIM_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS"); 
	config.readInto(SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS,"SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS"); 
	config.readInto(SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS,"SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS"); 
 
	config.readInto(FEMALE_AGE_DEPENDENT_SURVIVAL,"FEMALE_AGE_DEPENDENT_SURVIVAL") ; 
	config.readInto(FEMALE_MAXIMUM_AGE_1, "FEMALE_MAXIMUM_AGE_1") ; 
	config.readInto(FEMALE_START_SENESCENCE_2, "FEMALE_START_SENESCENCE_2") ; 
	config.readInto(FEMALE_MAXIMUM_AGE_2, "FEMALE_MAXIMUM_AGE_2") ; 
 
	config.readInto(MALE_AGE_DEPENDENT_SURVIVAL, "MALE_AGE_DEPENDENT_SURVIVAL") ; 
	config.readInto(MALE_MAXIMUM_AGE_1, "MALE_MAXIMUM_AGE_1") ; 
	config.readInto(MALE_START_SENESCENCE_2, "MALE_START_SENESCENCE_2") ; 
	config.readInto(MALE_MAXIMUM_AGE_2, "MALE_MAXIMUM_AGE_2") ; 
 
	config.readInto(EGG_DAILY_SURVIVAL_LOW_TEMPERATURE,"EGG_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"EGG_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"EGG_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE,"EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"EGG_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE,"FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"FEMALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"FEMALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE,"FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"FEMALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE,"MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"MALE_LARVAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"MALE_LARVAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE,"MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"MALE_LARVAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE,"FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"FEMALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"FEMALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE,"FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"FEMALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE,"MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"MALE_PUPAE_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"MALE_PUPAE_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE,"MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"MALE_PUPAE_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE,"FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE,"FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE,"MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_LOWER_LIMIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_NORMAL_TEMPERATURE_UPPER_LIMIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE,"MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_HIGH_TEMPERATURE_LIMIT"); 
 
	config.readInto(EGG_DAILY_SURVIVAL_WET_CONTAINER,"EGG_DAILY_SURVIVAL_WET_CONTAINER"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_SUNEXPOSURE_LIMIT,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_SUNEXPOSURE_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT_LIMIT,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_LOW_SATURATIONDEFICIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_LOW_SUNEXPOSURE_HIGH_SATURATIONDEFICIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_DRY_CONTAINER_HIGH_SUNEXPOSURE,"EGG_DAILY_SURVIVAL_DRY_CONTAINER_HIGH_SUNEXPOSURE"); 
 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER,"FEMALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER"); 
 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER,"MALE_LARVAE_DAILY_SURVIVAL_DRY_CONTAINER"); 
 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT,"FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT,"FEMALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT"); 
	config.readInto(FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT,"FEMALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT"); 
 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT,"MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT_LIMIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT,"MALE_ADULT_DAILY_SURVIVAL_LOW_SATURATIONDEFICIT"); 
	config.readInto(MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT,"MALE_ADULT_DAILY_SURVIVAL_HIGH_SATURATIONDEFICIT"); 
 
	config.readInto(EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT,"EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT,"EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE_LIMIT"); 
	config.readInto(EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE,"EGG_DAILY_SURVIVAL_TOPREDATION_LOW_TEMPERATURE"); 
	config.readInto(EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE,"EGG_DAILY_SURVIVAL_TOPREDATION_HIGH_TEMPERATURE"); 
 
	config.readInto(MINIMUM_TEMPERATURE_FOR_EGGHATCH,"MINIMUM_TEMPERATURE_FOR_EGGHATCH"); 
	config.readInto(RATIO_OF_EGGS_HATCHING_WITHOUT_FLOODING,"RATIO_OF_EGGS_HATCHING_WITHOUT_FLOODING"); 
	config.readInto(DAILY_RATIO_OF_EMBRYONATED_EGGS_THAT_HATCH_IF_SUBMERGED,"DAILY_RATIO_OF_EMBRYONATED_EGGS_THAT_HATCH_IF_SUBMERGED"); 
	config.readInto(EGGS_SEX_PERCENTAGE_FEMALE,"EGGS_SEX_PERCENTAGE_FEMALE"); 
 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_WEIGHT_AT_HATCH,"SKEETER_BUSTER_FEMALE_LARVAE_WEIGHT_AT_HATCH"); 
	config.readInto(CIMSIM_FEMALE_LARVAE_WEIGHT_AT_HATCH,"CIMSIM_FEMALE_LARVAE_WEIGHT_AT_HATCH"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE,"FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE"); 
	config.readInto(FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE,"FEMALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE"); 
	config.readInto(FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL,"FEMALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL,"SKEETER_BUSTER_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL"); 
	config.readInto(CIMSIM_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL,"CIMSIM_FEMALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL"); 
	config.readInto(FEMALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS,"FEMALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS"); 
	config.readInto(FEMALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION,"FEMALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION"); 
	config.readInto(FEMALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES,"FEMALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES"); 
	config.readInto(FEMALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE,"FEMALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE"); 
	config.readInto(FEMALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE,"FEMALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE"); 
	config.readInto(FEMALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C,"FEMALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C"); 
	config.readInto(FEMALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE,"FEMALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE"); 
	config.readInto(FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE,"FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE"); 
	config.readInto(FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO,"FEMALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO"); 
	config.readInto(FEMALE_LARVAE_CONVERSION_RATE_OF_CADAVERS_TO_FOOD,"FEMALE_LARVAE_CONVERSION_RATE_OF_CADAVERS_TO_FOOD"); 
	config.readInto(FEMALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS,"FEMALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS"); 
 
	config.readInto(LARVAE_NUMBER_OF_EULER_STEPS,"LARVAE_NUMBER_OF_EULER_STEPS"); 
 
 
	//SKEETER BUSTER 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION,"SKEETER_BUSTER_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"SKEETER_BUSTER_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25,"SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50,"SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75,"SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100,"SKEETER_BUSTER_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100"); 
	config.readInto(SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN,"SKEETER_BUSTER_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN"); 
 
 
 
	//CIMSIM 
	config.readInto(CIMSIM_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION,"CIMSIM_FEMALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION"); 
	config.readInto(CIMSIM_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"CIMSIM_FEMALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(CIMSIM_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"CIMSIM_FEMALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(CIMSIM_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN,"CIMSIM_FEMALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN"); 
 
 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_WEIGHT_AT_HATCH,"SKEETER_BUSTER_MALE_LARVAE_WEIGHT_AT_HATCH"); 
	config.readInto(CIMSIM_MALE_LARVAE_WEIGHT_AT_HATCH,"CIMSIM_MALE_LARVAE_WEIGHT_AT_HATCH"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE,"MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITH_LIPID_RESERVE"); 
	config.readInto(MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE,"MALE_LARVAE_DAILY_SURVIVAL_FASTING_WITHOUT_LIPID_RESERVE"); 
	config.readInto(MALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL,"MALE_LARVAE_SMALLEST_WEIGHT_FOR_SURVIVAL"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL,"SKEETER_BUSTER_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL"); 
	config.readInto(CIMSIM_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL,"CIMSIM_MALE_LARVAE_MAXIMAL_PHYSIOLOGICAL_DEVELOPMENTAL_TIME_FOR_SURVIVAL"); 
	config.readInto(MALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS,"MALE_LARVAE_CONVERSION_RATE_OF_CONSUMED_FOOD_TO_BIOMASS"); 
	config.readInto(MALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION,"MALE_LARVAE_INCREMENT_OF_INCREASE_OF_RATE_OF_FOOD_EXPLOITATION"); 
	config.readInto(MALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES,"MALE_LARVAE_ASYMPTOTIC_RATE_OF_FOOD_EXPLOITATION_AT_HIGH_FOOD_DENSITIES"); 
	config.readInto(MALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE,"MALE_LARVAE_RATE_OF_METABOLIC_WEIGHT_LOSS_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE"); 
	config.readInto(MALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE,"MALE_LARVAE_EXPONENT_OF_METABOLIC_REQUIREMENTS_OF_LARVAE_AS_A_FUNCTION_OF_DRY_WEIGHT_OF_LARVAE"); 
	config.readInto(MALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C,"MALE_LARVAE_CHRONOLOGICAL_BASIS_AT_26C"); 
	config.readInto(MALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE,"MALE_LARVAE_MINIMUM_LIPID_RATIO_OF_LIVING_LARVAE"); 
	config.readInto(MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE,"MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_ONE"); 
	config.readInto(MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO,"MALE_LARVAE_CONVERSION_OF_DRY_BODY_WEIGHT_TO_LIPID_RESERVE_PARAMETER_TWO"); 
	config.readInto(MALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS,"MALE_LARVAE_SURVIVAL_AT_PUPATION_DUE_TO_BIRTH_DEFECTS"); 
 
 
	//SKEETER BUSTER 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION,"SKEETER_BUSTER_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"SKEETER_BUSTER_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25,"SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_25"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50,"SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_50"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75,"SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_75"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100,"SKEETER_BUSTER_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T_100"); 
	config.readInto(SKEETER_BUSTER_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN,"SKEETER_BUSTER_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN"); 
 
	//CIMSIM 
	config.readInto(CIMSIM_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION,"CIMSIM_MALE_LARVAE_ABSOLUTE_MINIMUM_WEIGHT_FOR_PUPATION"); 
	config.readInto(CIMSIM_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"CIMSIM_MALE_LARVAE_SLOPE_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(CIMSIM_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T,"CIMSIM_MALE_LARVAE_INTERCEPT_OF_MINIMAL_LARVAL_WEIGHT_FOR_PUPATION_AS_FUNCTION_OF_T"); 
	config.readInto(CIMSIM_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN,"CIMSIM_MALE_LARVAE_PHYSIOLOGICAL_DEVELOPMENTAL_PERCENT_FOR_WMIN"); 
 
 
	config.readInto(FEMALE_PUPAE_SURVIVAL_AT_EMERGENCE,"FEMALE_PUPAE_SURVIVAL_AT_EMERGENCE"); 
	config.readInto(FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT,"FEMALE_ADULT_CONVERSION_FACTOR_OF_DRY_WEIGHT_OF_LARVAE_TO_WET_WEIGHT_OF_ADULT"); 
	config.readInto(FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT,"FEMALE_ADULT_DAILY_FECUNDITY_FACTOR_FOR_WET_WEIGHT"); 
	config.readInto(SKEETER_BUSTER_FECUNDITY_DROP_BY_AGE,"SKEETER_BUSTER_FECUNDITY_DROP_BY_AGE"); 
	config.readInto(SKEETER_BUSTER_FECUNDITY_RATIO_STANDARD_DEVIATION_MEAN,"SKEETER_BUSTER_FECUNDITY_RATIO_STANDARD_DEVIATION_MEAN"); 
	config.readInto(FEMALE_ADULT_MINIMUM_TEMPERATURE_FOR_OVIPOSITION,"FEMALE_ADULT_MINIMUM_TEMPERATURE_FOR_OVIPOSITION"); 
 
 
	config.readInto(MALE_PUPAE_SURVIVAL_AT_EMERGENCE,"MALE_PUPAE_SURVIVAL_AT_EMERGENCE"); 
 
	config.readInto(NULLIPAROUS_FEMALE_ADULT_DISPERSAL,"NULLIPAROUS_FEMALE_ADULT_DISPERSAL"); 
	config.readInto(PAROUS_FEMALE_ADULT_DISPERSAL,"PAROUS_FEMALE_ADULT_DISPERSAL"); 
	config.readInto(MALE_ADULT_DISPERSAL,"MALE_ADULT_DISPERSAL"); 
	config.readInto(DISPERSAL_DIRECTION_BIAS,"DISPERSAL_DIRECTION_BIAS");
 
	config.readInto(NULLIPAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE,"NULLIPAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE"); 
	config.readInto(PAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE,"PAROUS_FEMALE_ADULT_DISPERSAL_FROM_EMPTY_HOUSE"); 
	config.readInto(MALE_ADULT_DISPERSAL_WHEN_NO_FEMALE,"MALE_ADULT_DISPERSAL_WHEN_NO_FEMALE"); 
	config.readInto(MAX_NUMBER_OF_DISPERSAL_STEPS,"MAX_NUMBER_OF_DISPERSAL_STEPS");

	config.readInto(NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL,"NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL"); 
	config.readInto(PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL,"PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL"); 
	config.readInto(MALE_ADULT_LONG_RANGE_DISPERSAL,"MALE_ADULT_LONG_RANGE_DISPERSAL"); 
	config.readInto(LR_DISPERSAL_MAX_DISTANCE,"LR_DISPERSAL_MAX_DISTANCE"); 
 
	config.readInto(CONTAINER_MOVEMENT_PROBABILITY,"CONTAINER_MOVEMENT_PROBABILITY"); 
	config.readInto(PROBABILITY_OF_EMPTYING_MANUALLY_FILLED_CONTAINERS, "PROBABILITY_OF_EMPTYING_MANUALLY_FILLED_CONTAINERS");
	config.readInto(OVIPOSITION_REDUCTION_IN_COVERED_CONTAINERS, "OVIPOSITION_REDUCTION_IN_COVERED_CONTAINERS");
 
	config.readInto(FEMALE_DAILY_RECAPTURE_RATE,"FEMALE_DAILY_RECAPTURE_RATE"); 
	config.readInto(MALE_DAILY_RECAPTURE_RATE,"MALE_DAILY_RECAPTURE_RATE"); 

	config.readInto(DAILY_REMATING_PROBABILITY_PAROUS_FEMALES,"DAILY_REMATING_PROBABILITY_PAROUS_FEMALES");
 
	//degree-day parameters 
	config.readInto(MINIMUM_DEVELOPMENTAL_TEMPERATURE,"MINIMUM_DEVELOPMENTAL_TEMPERATURE"); 
	config.readInto(DEGREE_HOURS_TO_BE_REACHED,"DEGREE_HOURS_TO_BE_REACHED"); 
 
	//weather files 
	config.readInto(WEATHER_YEAR_ONE,"WEATHER_YEAR_ONE"); 
	if (NUMBER_OF_DAYS>365) config.readInto(WEATHER_YEAR_TWO,"WEATHER_YEAR_TWO"); 
	if (NUMBER_OF_DAYS>730) config.readInto(WEATHER_YEAR_THREE,"WEATHER_YEAR_THREE"); 
	if (NUMBER_OF_DAYS>1095) config.readInto(WEATHER_YEAR_FOUR,"WEATHER_YEAR_FOUR"); 
	if (NUMBER_OF_DAYS>1460) config.readInto(WEATHER_YEAR_FIVE,"WEATHER_YEAR_FIVE"); 
	if (NUMBER_OF_DAYS>1825) config.readInto(WEATHER_YEAR_SIX,"WEATHER_YEAR_SIX"); 
	if (NUMBER_OF_DAYS>2190) config.readInto(WEATHER_YEAR_SEVEN,"WEATHER_YEAR_SEVEN"); 
	if (NUMBER_OF_DAYS>2555) config.readInto(WEATHER_YEAR_EIGHT,"WEATHER_YEAR_EIGHT"); 

	// location-specific tweaks

	config.readInto(GORDONVALE_REMOVE_173_195,"GORDONVALE_REMOVE_173_195");
	config.readInto(GORDONVALE_GOLF_COURSE,"GORDONVALE_GOLF_COURSE");

	fprintf(stdout, " OK.\n");
	 
} 
 
void WeatherInput() 
{ 
  char Akarmi[80]; 
  float Daily_Maximum_Air_Temperature; 
  float Daily_Minimum_Air_Temperature; 
  float Daily_Average_Air_Temperature; 
  float Saturation_Deficit; 
  float Rain; 
  float Relative_Humidity; 
  int Day; 
   
  FILE *WeatherInput; //input weather file 
 
  //this part reads in the weather file, and loads the values into the weather variables 
  int Remaining_Days=NUMBER_OF_DAYS; 
  int Year=1; 
  int Limit=0; 
 
  fprintf(stdout, "Importing weather files ...");

	do 
	{ 
		if (Year==1) WeatherInput=fopen(WEATHER_YEAR_ONE.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_ONE.c_str());
			ErrorMessage(5);
		}
		if (Year==2) WeatherInput=fopen(WEATHER_YEAR_TWO.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_TWO.c_str());
			ErrorMessage(5);
		}
		if (Year==3) WeatherInput=fopen(WEATHER_YEAR_THREE.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_THREE.c_str());
			ErrorMessage(5);
		}
		if (Year==4) WeatherInput=fopen(WEATHER_YEAR_FOUR.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_FOUR.c_str());
			ErrorMessage(5);
		}
		if (Year==5) WeatherInput=fopen(WEATHER_YEAR_FIVE.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_FIVE.c_str());
			ErrorMessage(5);
		}
		if (Year==6) WeatherInput=fopen(WEATHER_YEAR_SIX.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_SIX.c_str());
			ErrorMessage(5);
		}
		if (Year==7) WeatherInput=fopen(WEATHER_YEAR_SEVEN.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_SEVEN.c_str());
			ErrorMessage(5);
		}
		if (Year==8) WeatherInput=fopen(WEATHER_YEAR_EIGHT.c_str(),"r"); 
		if (WeatherInput==NULL) 
		{
			fprintf(stdout, "\n*** Error: Unable to open weather file %s\n", WEATHER_YEAR_EIGHT.c_str());
			ErrorMessage(5);
		}
		fseek(WeatherInput, 0L, SEEK_SET ); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		//fscanf(WeatherInput,"%s\n",Akarmi); 
		if (Remaining_Days>365) Limit=365; else Limit=Remaining_Days; 
		for (int Date=0;Date<Limit;Date++) 
			{ 
			int check = fscanf(WeatherInput,"%d %g %g %g %g %g %g %s\n",&Day,&Daily_Maximum_Air_Temperature,&Daily_Minimum_Air_Temperature,&Daily_Average_Air_Temperature,&Saturation_Deficit,&Rain,&Relative_Humidity,Akarmi); 
			if (check!=8)
			{
				fprintf(stdout, "\n*** Error: Missing data in weather file number %d, day %d\n", Year, Date%365);
				ErrorMessage(51);
			}
			Day--; 
			Day=Day+(Year-1)*365; 
			vector<Building>::iterator itHouse; 
 			for ( itHouse = House.begin(); itHouse != House.end(); itHouse++ ) 
				{ 
				itHouse->Maximum_Daily_Air_Temperature[Day]=Daily_Maximum_Air_Temperature; 
				itHouse->Minimum_Daily_Air_Temperature[Day]=Daily_Minimum_Air_Temperature; 
				itHouse->Average_Daily_Air_Temperature[Day]=Daily_Average_Air_Temperature; 
				//itHouse->Average_Daily_Air_Temperature[Day]=FixedAirAverageTemperature; 
				itHouse->Saturation_Deficit[Day]=Saturation_Deficit; 
				itHouse->Relative_Humidity[Day]=Relative_Humidity; 
				itHouse->Precipitation[Day]=Rain; 
				} 
			} 
		fclose(WeatherInput);   
		Remaining_Days-=Limit; 
		Year++; 
	} 
	while (Remaining_Days>0);

	fprintf(stdout, " OK.\n");

}


void CheckParameters()
// this function to list parameters check and return errors 
// if some parameters values are non realistic or incompatible
{
	fprintf(stdout, "Checking parameters ...");

	// calculation mode required for soft selection with embryonic costs
	if ((FITNESS_COST_STAGE==0)&&(EMBRYONIC_COST_SELECTION_TYPE==SOFT)&&(REPRODUCTION_MODE==SAMPLING))
	{
		fprintf(stdout, "\n*** Error: Embryonic costs with soft selection require CALCULATION");
		fprintf(stdout, "\n*** Please make sure CALCULATION mode is selected, or ");
		fprintf(stdout, "\n*** set REPRODUCTION_MODE to 0 in the CONF file\n");
		ErrorMessage(1);
	}

	// costs have to be costs (ie. 0 < values < 1) with certain selection types

	if ((FITNESS_COST_STAGE==1) || (FITNESS_COST_STAGE==2) || (FITNESS_COST_STAGE==3) || ((FITNESS_COST_STAGE==0)&&(EMBRYONIC_COST_SELECTION_TYPE==HARD)))
		if ((
			((GENETIC_CONTROL_METHOD==1) && 
			(!CheckProba(FITNESS_COST_CONSTRUCT_ALPHA) 
				|| !CheckProba(FITNESS_COST_CONSTRUCT_BETA) 
				|| !CheckProba(FITNESS_COST_CONSTRUCT_GAMMA) 
				|| !CheckProba(FITNESS_COST_CONSTRUCT_DELTA)))
			||
			((GENETIC_CONTROL_METHOD==3) &&
			(!CheckProba(FITNESS_COST_INSENSITIVE_TRANSGENE)
				|| !CheckProba(FITNESS_COST_INSENSITIVE_NATURAL)
				|| !CheckProba(FITNESS_COST_DRIVE_GENE)))
			||
			((GENETIC_CONTROL_METHOD==4) &&
			(!CheckProba(MEDEA_FITNESS_COST_PER_CONSTRUCT)))
			||
			((GENETIC_CONTROL_METHOD==6) &&
			(!CheckProba(KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST)
				|| !CheckProba(RESCUE_ALLELE_HOMOZYGOUS_FITNESS_COST)))
			||
			((GENETIC_CONTROL_METHOD==7) &&
			(!CheckProba(SELECTIVE_FITNESS_COST)))
			))
	{
		fprintf(stdout, "\n*** Error: Fitness costs at stage %d", FITNESS_COST_STAGE);
		if (FITNESS_COST_STAGE==0)
			fprintf(stdout, " with hard selection");
		fprintf(stdout, "\n*** require costs to be strictly between 0 and 1.");
		fprintf(stdout, "\n*** Please check the fitness costs for your selected control mechanism\n");
		ErrorMessage(1);
	}

	// MEDEA is incompatible with fitness costs othe than embryonic
	if (GENETIC_CONTROL_METHOD==MEDEA)
	  if (FITNESS_COST_STAGE!=0)
		{
			fprintf(stdout, "\n*** Error: fitness costs other than embryonic are incompatible with MEDEA genetic control method\n");
			ErrorMessage(1);
		}


	// defining region for introduction at stable age distribution
	if ((SAD_INTRODUCTION)&&(SAD_INTRODUCTION_REGION==1))
	{
		if ((SAD_INTRODUCTION_X_MIN < 0) || (SAD_INTRODUCTION_X_MIN > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Minimum X coordinate (SAD_INTRODUCTION_X_MIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if ((SAD_INTRODUCTION_X_MAX < 0) || (SAD_INTRODUCTION_X_MAX > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum X coordinate (SAD_INTRODUCTION_X_MAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if (SAD_INTRODUCTION_X_MAX < SAD_INTRODUCTION_X_MIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum X coordinate (SAD_INTRODUCTION_X_MAX)");
			fprintf(stdout, "\n*** must be higher than minimum (SAD_INTRODUCTION_X_MIN)\n");
			ErrorMessage(1);
		}
		if ((SAD_INTRODUCTION_Y_MIN < 0) || (SAD_INTRODUCTION_Y_MIN > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Minimum Y coordinate (SAD_INTRODUCTION_Y_MIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if ((SAD_INTRODUCTION_Y_MAX < 0) || (SAD_INTRODUCTION_Y_MAX > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum Y coordinate (SAD_INTRODUCTION_Y_MAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if (SAD_INTRODUCTION_Y_MAX < SAD_INTRODUCTION_Y_MIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for SAD release");
			fprintf(stdout, "\n*** Maximum Y coordinate (SAD_INTRODUCTION_Y_MAX)");
			fprintf(stdout, "\n*** must be higher than minimum (SAD_INTRODUCTION_Y_MIN)\n");
			ErrorMessage(1);
		}

	}

	// larvicidal control -- check region size

	if ((TRADITIONAL_CONTROL_METHOD%2)&&(LARVICIDE_REGION==1)&&(!(LARVICIDE_SIZE>0)))
	{
		fprintf(stdout, "\n*** Error: Immature control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of LARVICIDE_SIZE\n");
		ErrorMessage(1);
	}

	if ((TRADITIONAL_CONTROL_METHOD%2)&&(LARVICIDE_REGION==2))
	{
		if ((LARVICIDE_REGION_2_XMIN < 0) || (LARVICIDE_REGION_2_XMIN > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Minimum X coordinate (LARVICIDE_REGION_2_XMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if ((LARVICIDE_REGION_2_XMAX < 0) || (LARVICIDE_REGION_2_XMAX > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum X coordinate (LARVICIDE_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if (LARVICIDE_REGION_2_XMAX < LARVICIDE_REGION_2_XMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum X coordinate (LARVICIDE_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (LARVICIDE_REGION_2_XMIN)\n");
			ErrorMessage(1);
		}
		if ((LARVICIDE_REGION_2_YMIN < 0) || (LARVICIDE_REGION_2_YMIN > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Minimum Y coordinate (LARVICIDE_REGION_2_YMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if ((LARVICIDE_REGION_2_YMAX < 0) || (LARVICIDE_REGION_2_YMAX > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum Y coordinate (LARVICIDE_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if (LARVICIDE_REGION_2_YMAX < LARVICIDE_REGION_2_YMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for immature control");
			fprintf(stdout, "\n*** Maximum Y coordinate (LARVICIDE_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (LARVICIDE_REGION_2_YMIN)\n");
			ErrorMessage(1);
		}

	}

	// adulticidal control -- check region size

	if (((TRADITIONAL_CONTROL_METHOD>>1)%2)&&(ADULTICIDE_REGION==1)&&(!(ADULTICIDE_SIZE>0)))
	{
		fprintf(stdout, "\n*** Error: Adulticidal control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of ADULTICIDE_SIZE\n");
		ErrorMessage(1);
	}

	if (((TRADITIONAL_CONTROL_METHOD>>1)%2)&&(ADULTICIDE_REGION==2))
	{
		if ((ADULTICIDE_REGION_2_XMIN < 0) || (ADULTICIDE_REGION_2_XMIN > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Minimum X coordinate (ADULTICIDE_REGION_2_XMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if ((ADULTICIDE_REGION_2_XMAX < 0) || (ADULTICIDE_REGION_2_XMAX > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum X coordinate (ADULTICIDE_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if (ADULTICIDE_REGION_2_XMAX < ADULTICIDE_REGION_2_XMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum X coordinate (ADULTICIDE_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (ADULTICIDE_REGION_2_XMIN)\n");
			ErrorMessage(1);
		}
		if ((ADULTICIDE_REGION_2_YMIN < 0) || (ADULTICIDE_REGION_2_YMIN > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Minimum Y coordinate (ADULTICIDE_REGION_2_YMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if ((ADULTICIDE_REGION_2_YMAX < 0) || (ADULTICIDE_REGION_2_YMAX > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (ADULTICIDE_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if (ADULTICIDE_REGION_2_YMAX < ADULTICIDE_REGION_2_YMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for adulticidal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (ADULTICIDE_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (ADULTICIDE_REGION_2_YMIN)\n");
			ErrorMessage(1);
		}

	}

	// source removal control -- check region size

	if (((TRADITIONAL_CONTROL_METHOD>>2)%2)&&(SOURCE_REMOVAL_REGION==1)&&(!(SOURCE_REMOVAL_SIZE>0)))
	{
		fprintf(stdout, "\n*** Error: Source removal control in a region of size 0 (or negative)");
		fprintf(stdout, "\n*** Check the value of SOURCE_REMOVAL_SIZE\n");
		ErrorMessage(1);
	}

	if (((TRADITIONAL_CONTROL_METHOD>>2)%2)&&(SOURCE_REMOVAL_REGION==2))
	{
		if ((SOURCE_REMOVAL_REGION_2_XMIN < 0) || (SOURCE_REMOVAL_REGION_2_XMIN > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Minimum X coordinate (SOURCE_REMOVAL_REGION_2_XMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if ((SOURCE_REMOVAL_REGION_2_XMAX < 0) || (SOURCE_REMOVAL_REGION_2_XMAX > NUMBER_OF_COLUMNS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum X coordinate (SOURCE_REMOVAL_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_COLUMNS-1);
			ErrorMessage(1);
		}
		if (SOURCE_REMOVAL_REGION_2_XMAX < SOURCE_REMOVAL_REGION_2_XMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum X coordinate (SOURCE_REMOVAL_REGION_2_XMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (SOURCE_REMOVAL_REGION_2_XMIN)\n");
			ErrorMessage(1);
		}
		if ((SOURCE_REMOVAL_REGION_2_YMIN < 0) || (SOURCE_REMOVAL_REGION_2_YMIN > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Minimum Y coordinate (SOURCE_REMOVAL_REGION_2_YMIN)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if ((SOURCE_REMOVAL_REGION_2_YMAX < 0) || (SOURCE_REMOVAL_REGION_2_YMAX > NUMBER_OF_ROWS-1))
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (SOURCE_REMOVAL_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be between 0 and %d\n", NUMBER_OF_ROWS-1);
			ErrorMessage(1);
		}
		if (SOURCE_REMOVAL_REGION_2_YMAX < SOURCE_REMOVAL_REGION_2_YMIN)
		{
			fprintf(stdout, "\n*** Error: Definition of region for source removal control");
			fprintf(stdout, "\n*** Maximum Y coordinate (SOURCE_REMOVAL_REGION_2_YMAX)");
			fprintf(stdout, "\n*** must be higher than minimum (SOURCE_REMOVAL_REGION_2_YMIN)\n");
			ErrorMessage(1);
		}

	}


	// if everything is passed

	fprintf(stdout, " OK.\n");
				
}

void ErrorMessage (int n)
{
	fprintf(stdout, "\n*** Error (%d)", n);
	
	// see ErrorList.txt file for a list of error codes
	
	switch (n)
	{
	case 1: // incompatible parameters (general catch -- see CheckParameters function for details)
		fprintf(stdout, "\n*** Incompatible parameters");
		fprintf(stdout, "\n*** Exiting\n");
		exit(1);

	case 2: // file SkeeterBuster.conf not found
		fprintf(stdout, "\n*** Unable to find the file SkeeterBuster.conf\n");
		exit(2);

	case 3: // key in Skeeterbuster.conf not found 
		fprintf(stdout, "\n*** Key not found -- Exiting\n");
		exit(3);

	case 4: // file SkeeterBuster.setup not found
		fprintf(stdout, "\n*** Error: Unable to find the file SkeeterBuster.setup\n");
		exit(4);

	case 41: // problem with adult females count in setup file
		fprintf(stdout, "\n*** Error: Inconsistent number of female adults in setup");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(41);

	case 44: // EOF found while reading SETUP file (ie. SETUP file too short)
		fprintf(stdout, "\n*** Error: EOF found while reading SETUP file");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** The SETUP file does not contain all the information");
		fprintf(stdout, "\n*** that was expected by the program");
		fprintf(stdout, "\n*** Please check the composition of the SETUP file\n");
		exit(44);

	case 45: // EOF not found when reading SETUP file should be finished (ie. SETUP file too long)
		fprintf(stdout, "\n*** Error: Expected EOF at the end of SETUP file not found");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** The SETUP file contains additional information");
		fprintf(stdout, "\n*** that was not expected by the program.");
		fprintf(stdout, "\n*** It is likely that the information previously read");
		fprintf(stdout, "\n*** was misinterpreted by the program.");
		fprintf(stdout, "\n*** Please check the composition of the SETUP file\n");
		exit(45);

	case 5: // weather file not found
		fprintf(stdout, "\n*** File not found -- Exiting\n");
		exit(5);

	case 51: // missing info in weather file
		fprintf(stdout, "\n*** Please check the format of this weather file and");
		fprintf(stdout, "\n*** make sure that all the required information is present\n");
		exit(51);

	case 6: // Sampling Routine file not found
		fprintf(stdout, "\n*** Error: Unable to find the file SamplingRoutine.txt \n");
		exit(6);
	case 61: // Sampling Routine file has errors
		fprintf(stdout, "\n*** Error: SamplingRoutine.txt contains error and is not usable \n");
		exit(61);


	case 9: // libraries not found
		fprintf(stdout, "\n*** Skeeter Buster is not compatible with this system\n");
		exit(9);

	case 101: // caught proba not between 0 and 1 for binomial calculations
		fprintf(stdout, "\n*** Probability value found not between 0 and 1");
		fprintf(stdout, "\n*** Check parameter values, particularly costs\n");
		exit(101);

	case 102: // oviposition into a 102 container 
		fprintf(stdout, "\n*** Warning: oviposition into a 102 container\n");
		break;
	
	case 801: // larvicide control in incorrect region
		fprintf(stdout, "\n*** Unexpected error - Larvicidal control");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(801);

	case 802: // adulticide control in incorrect region
		fprintf(stdout, "\n*** Unexpected error - Adulticidal control");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(802);

	case 803: // source removal control in incorrect region
		fprintf(stdout, "\n*** Unexpected error - Source removal control");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(803);

	case 833: // MEDEA with non-embryonic fitness costs
		fprintf(stdout, "\n*** Unexpected error - MEDEA fitness calculation");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(833);

	case 909:
		fprintf(stdout, "\n*** Warning: Unable to compress output");
		fprintf(stdout, "\n*** Zip utility not found or not compatible");
		fprintf(stdout, "\n*** Continuing ...\n");
		break;

	default: // non handled cases
		fprintf(stdout, "\n*** Unknown error");
		fprintf(stdout, "\n***");
		fprintf(stdout, "\n*** Please signal this error to info@skeeterbuster.net");
		fprintf(stdout, "\n*** referencing error type %d\n", n);
		exit(1000);

	}
}
