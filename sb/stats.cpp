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

double SpatialStats(void)
{

		FILE *stats;
		char sref[10]="" ;
		char path_stats[200]="" ;
		int iloum ;
		
		int number_of_distances = 50;
		double l[50], ls[50], lw[50], lsw[50] ;
		int dist_increment = 1;
		int kdmax = (NUMBER_OF_COLUMNS>NUMBER_OF_ROWS?NUMBER_OF_ROWS:NUMBER_OF_COLUMNS) ;
		kdmax = ((dist_increment*number_of_distances)>kdmax?kdmax:(dist_increment*number_of_distances)) ;
		int area = (NUMBER_OF_ROWS-1)*(NUMBER_OF_COLUMNS-1) ;
		double pi = 3.1415926536 ;
		int count_d=0 ;

		/** Preliminary calculations **/

		int sum1=0, sum2=0 ;
		int pupae_in_house=0;
		double barx, varx ;
		for (int i=0 ; i<NUMBER_OF_HOUSES; i++)
		{
			pupae_in_house = (int)(House[i].TotalFemalePupae+House[i].TotalMalePupae) ;
			sum1+=pupae_in_house ;
			sum2+=(pupae_in_house*pupae_in_house) ;
		}
		barx = ((double)sum1)/((double)NUMBER_OF_HOUSES) ;
		varx = /*sqrt*/((((double)sum2)/((double)NUMBER_OF_HOUSES)) - (barx*barx)) ;
		
		#if 0
		/** Getis K-statistic **/
		
		strcat(path_stats, "kstats.ref") ;
		iloum = sprintf(sref, "%d", ref) ;
		strcat(path_stats, sref) ;
		strcat(path_stats, ".txt") ; 
		stats=fopen(path_stats, "w") ;
		
		
		fprintf(stats, "d\tL(d)\tL*(d)\tLw(d)\tLw*(d)\t\tdLw-dL\t\tdL*w-dL*\n") ;
		fprintf(stats, "-\t----\t-----\t-----\t------\t\t------\t\t--------\n") ;
			


		for (int d=dist_increment ; d<kdmax+1 ; d+=dist_increment)
		{
			double l_num=0., l_denom=0., ls_num=0., ls_denom=0., lw_num=0., lw_denom=0., lsw_num=0., lsw_denom=0. ;
			int pupae_in_i, pupae_in_j ;

			for (int i=0 ; i<NUMBER_OF_HOUSES ; i++)
			{
				pupae_in_i = (int)(House[i].TotalFemalePupae+House[i].TotalMalePupae) ;
				for (int dx=-d ; dx<d+1 ; dx++)
					for(int dy=-d ; dy<d+1 ; dy++)
						if (abs(dx)+abs(dy)<(d+1)) // houses within distance d (ie. k(i,j)!=0 )
						{
							int j = ((((i/NUMBER_OF_COLUMNS)+dy+10*NUMBER_OF_ROWS)%NUMBER_OF_ROWS)*NUMBER_OF_COLUMNS) + (((i%NUMBER_OF_COLUMNS)+dx+10*NUMBER_OF_COLUMNS)%NUMBER_OF_COLUMNS) ;
							
							pupae_in_j = (int)(House[j].TotalFemalePupae+House[j].TotalMalePupae) ;

							if (i!=j) l_num+=1 ;
							ls_num+=1 ;
							if (i!=j) lw_num+=(pupae_in_i*pupae_in_j);
							lsw_num+=(pupae_in_i*pupae_in_j);
						}
			}

			l_num*=area ; ls_num*=area ; lw_num*=area ; lsw_num*=area ;
			l_denom = NUMBER_OF_HOUSES*(NUMBER_OF_HOUSES-1) ; l_denom*=pi ;
			ls_denom = NUMBER_OF_HOUSES*NUMBER_OF_HOUSES ; ls_denom*=pi ;
			lw_denom = (sum1*sum1)-sum2 ; lw_denom*=pi ;
			lsw_denom = (sum1*sum1) ; lsw_denom*=pi ;

			l[count_d] = sqrt(l_num/l_denom) ;
			ls[count_d] = sqrt(ls_num/ls_denom) ;
			lw[count_d] = sqrt(lw_num/lw_denom) ;
			lsw[count_d] = sqrt(lsw_num/lsw_denom) ;

			fprintf(stats, "%d\t%.3f\t%.3f\t%.3f\t%.3f", d, l[count_d], ls[count_d], lw[count_d], lsw[count_d]) ;
			if(count_d)
				fprintf(stats, "\t\t%.4f\t\t%.4f", (lw[count_d]-lw[count_d-1])-(l[count_d]-l[count_d-1]), (lsw[count_d]-lsw[count_d-1])-(ls[count_d]-ls[count_d-1])) ;
			fprintf(stats, "\n") ;

			count_d++ ;

		}
		fclose(stats) ;

		/** Getis' Gi-statistics **/
		
		FILE *localstats;
		char path_localstats[200]="" ;
		double barxi=0., varxi=0. ;

		strcat(path_localstats, "localstats.ref") ;
		strcat(path_localstats, sref) ;
		strcat(path_localstats, ".txt") ; 
		localstats=fopen(path_localstats, "w") ;

		fprintf(localstats, "i\tx\ty") ;
		for (int d=dist_increment ; d<(number_of_distances+1)*dist_increment ; d+=dist_increment)
			fprintf(localstats, "\td=%d", d) ;
		fprintf(localstats, "\n") ;
		fprintf(localstats, "-\t-\t-") ;
		for (int d=dist_increment ; d<(number_of_distances+1)*dist_increment ; d+=dist_increment)
			fprintf(localstats, "\t----") ;
		fprintf(localstats, "\n") ;

		for (int i=0 ; i<NUMBER_OF_HOUSES ; i++)
		{
			int pupae_in_i=0, pupae_in_j=0 ;
			fprintf(localstats, "%d\t%d\t%d", i, i/NUMBER_OF_COLUMNS, i%NUMBER_OF_COLUMNS) ;

			pupae_in_i = (int)(House[i].TotalFemalePupae+House[i].TotalMalePupae) ;
			barxi = ((double)(sum1-pupae_in_i))/((double)(NUMBER_OF_HOUSES-1));
			varxi = /*sqrt*/((((double)(sum2-(pupae_in_i*pupae_in_i)))/((double)(NUMBER_OF_HOUSES-1))) - (barxi*barxi)) ;

			for (int d=dist_increment ; d<(number_of_distances+1)*dist_increment ; d+=dist_increment)
			{
				int Wid=0, Wisd=0 ;
				//int S1id=0, S1isd=0 ; // here S1id = Wid
				int sumid=0, sumisd=0 ;
				double Gid=0., Gisd=0. ;

				for (int dx=-d ; dx<d+1 ; dx++)
					for(int dy=-d ; dy<d+1 ; dy++)
						if (abs(dx)+abs(dy)<(d+1)) // houses within distance d (ie. k(i,j)!=0 )
						{
							int j = ((((i/NUMBER_OF_COLUMNS)+dy+10*NUMBER_OF_ROWS)%NUMBER_OF_ROWS)*NUMBER_OF_COLUMNS) + (((i%NUMBER_OF_COLUMNS)+dx+10*NUMBER_OF_COLUMNS)%NUMBER_OF_COLUMNS) ;
							// change that 10x ...
							pupae_in_j = (int)(House[j].TotalFemalePupae+House[j].TotalMalePupae) ;

							if (i!=j) Wid+=1 ;
							Wisd+=1 ;
							if (i!=j) sumid+=pupae_in_j;
							sumisd+=pupae_in_j;
						}

				if (Wisd<NUMBER_OF_HOUSES)
				{
					Gid = ((double)(sumid-(barxi*Wid)))/(varxi*sqrt((double)(((NUMBER_OF_HOUSES-1)*Wid)-(Wid*Wid))/(double)(NUMBER_OF_HOUSES-2)));
					Gisd = ((double)(sumisd-(barx*Wisd)))/(varx*sqrt((double)((NUMBER_OF_HOUSES*Wisd)-(Wisd*Wisd))/(double)(NUMBER_OF_HOUSES-1)));
				
					fprintf(localstats, "\t%.3f", Gisd) ;
				}
				else
					fprintf(localstats, "\t<--") ;
				
			}

			fprintf(localstats, "\n") ;
			if (!((i+1)%NUMBER_OF_COLUMNS)) fprintf(localstats, "\n");
		}


		fclose(localstats) ;
		#endif

		/** Morisita index **/

		FILE * morisita ;
		char path_morisita[200]="" ;
		
		int nreps = 20 ; // number of replicates

		int house_number, p, countcells ;
		unsigned long int Id_num, tot ;
		double Id ;
		int dmax = (NUMBER_OF_COLUMNS>NUMBER_OF_ROWS ? NUMBER_OF_COLUMNS : NUMBER_OF_ROWS) ;
		int hmax = dmax/2 ; // at least 2 cells
		if (hmax<2) fprintf(stdout, "Grid is too small to compute Morisita's index\n");
		else
		{
			strcat(path_morisita, "morisita.ref") ;
			strcat(path_morisita, sref) ;
			strcat(path_morisita, ".txt") ; 
			morisita= fopen(path_morisita, "w") ;
			
			fprintf(morisita, "d\tId : %d reps-->\n", nreps) ;
			fprintf(morisita, "-\t--\n") ;

			for (int h=2 ; h<=hmax ; h++) // calculate index for increasing cell size
			{
				fprintf(morisita, "%d", h) ;
				for (int rep=0 ; rep<nreps ; rep++)
				{

				int i0=(int)(Random_Number()*NUMBER_OF_COLUMNS) ; 
				int j0=(int)(Random_Number()*NUMBER_OF_ROWS) ; // cells start from a random point on each replicate

				countcells=0 ;
				Id_num=0 ; tot=0 ;
				Id=0.;
				for (int i=0 ; i<=NUMBER_OF_COLUMNS-h ; i+=h)
					for (int j=0 ; j<=NUMBER_OF_ROWS-h ; j+=h) // going through cells 
					{
						p=0 ; 
						countcells++ ;
						for (int ii=i ; ii<i+h ; ii++)
							for (int jj=j ; jj<j+h ; jj++) // going through houses within a cell
							{
								house_number = (((j0+jj)%NUMBER_OF_ROWS)*NUMBER_OF_COLUMNS)+((i0+ii)%NUMBER_OF_COLUMNS) ;
								p += (int)(House[house_number].TotalFemalePupae+House[house_number].TotalMalePupae) ;
							}
						Id_num += p*(p-1) ;
						tot += p ;	// since cell tiling might not cover the entire grid
									// we have to compute the "total" number of pupae in this tiling
					}
				Id = ((double)(countcells*Id_num))/((double)(tot*(tot-1))) ;

				fprintf(morisita, "\t%.3f", Id) ;
				}
				fprintf(morisita, "\n") ; 
			}
			fclose(morisita) ;
		}
		
		/** Moran's I by distance **/

		FILE * moransi;
		char path_moransi[200]="" ;

		if (hmax<2) fprintf(stdout, "Grid is too small to compute Moran's I\n");
		else
		{
			strcat(path_moransi, "moransi.ref") ;
			strcat(path_moransi, sref) ;
			strcat(path_moransi, ".txt") ; 
			moransi= fopen(path_moransi, "w") ;
			
			fprintf(moransi, " d\tIm\n") ;
			fprintf(moransi, "--\t--\n") ;

			for (int h=1 ; h<=hmax ; h++) // calculate index for increasing cell size
			{
				double Im=0.;
				double Im_num=0. , Im_denom=0.;
				int pupae_in_i, pupae_in_j;
				int count_pairs=0;

				for (int i=0 ; i<NUMBER_OF_HOUSES ; i++)
				{
					pupae_in_i = (int)(House[i].TotalFemalePupae + House[i].TotalMalePupae);
					for (int dx=-h ; dx<h+1 ; dx++)
						for (int dy=-h ; dy<h+1 ; dy++)
							if (abs(dx)+abs(dy)==h) // houses at distance exactly h
							{
								int j = ((((i/NUMBER_OF_COLUMNS)+dy+10*NUMBER_OF_ROWS)%NUMBER_OF_ROWS)*NUMBER_OF_COLUMNS) + (((i%NUMBER_OF_COLUMNS)+dx+10*NUMBER_OF_COLUMNS)%NUMBER_OF_COLUMNS) ;

								pupae_in_j = (int)(House[j].TotalFemalePupae + House[j].TotalMalePupae);
								Im_num += (pupae_in_i - barx)*(pupae_in_j-barx);
								count_pairs++;
							}
					Im_denom += (pupae_in_i - barx)*(pupae_in_i - barx);
				}

				Im_denom = sqrt(Im_denom);
				Im_denom = Im_denom / NUMBER_OF_HOUSES;

				Im = Im_num/Im_denom ;
				Im = Im / count_pairs;

				fprintf(moransi, "%d %.5f\n", h, Im);

			}
		fclose(moransi);

		}

		/** Moran's I global **/

		double count_weights=0., weightij=0.;
		double Img=0., Img_num=0., Img_denom=0.;
		int pupae_in_i, pupae_in_j;
				

		for (int i=0 ; i<NUMBER_OF_HOUSES ; i++)
		{
			pupae_in_i = (int)(House[i].TotalFemalePupae + House[i].TotalMalePupae);
			for (int dx=-hmax ; dx<hmax+1 ; dx++)
				for (int dy=-hmax ; dy<hmax+1 ; dy++)
					if (dx||dy) // excluding i=j
					{
						int j = ((((i/NUMBER_OF_COLUMNS)+dy+10*NUMBER_OF_ROWS)%NUMBER_OF_ROWS)*NUMBER_OF_COLUMNS) + (((i%NUMBER_OF_COLUMNS)+dx+10*NUMBER_OF_COLUMNS)%NUMBER_OF_COLUMNS);
						pupae_in_j = (int)(House[j].TotalFemalePupae + House[j].TotalMalePupae);
						weightij = 1./(abs(dx)+abs(dy));
						Img_num += weightij*(pupae_in_i - barx)*(pupae_in_j-barx);
						count_weights+=weightij;
					}
			Img_denom += (pupae_in_i - barx)*(pupae_in_i - barx);
		}

		Img_num *= NUMBER_OF_HOUSES ;
		Img_denom  *= count_weights ;

		Img = Img_num/Img_denom ;

		//fprintf(stdout, "Morans I is %.3f\n", Img);
		return Img;
	

}
