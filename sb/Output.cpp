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


extern char date[12];
extern char *hour;
extern int Date,ref;

extern vector<Building> House;

char path_global_html[300]="", path_adults_html[300]="", path_allelefreq_html[300]="", path_eps_output[300]="";
char path_output[200]="", path_output_per_house[200]="", path_output_per_container_type[200]="", path_output_water_level[200]="", path_short_output[200]="", path_fstbydist[200]="";
char path_eps_output_by_day[100]="",path_eps_wolbachia_output_by_day[100]="",path_eps_density_output_by_day[100]="",path_eps_xt_output[100]=""  ;

FILE *HTML_Global_Output, *HTML_Adults_Output, *HTML_AlleleFreq_Output, *GNUplot_command_file ;
FILE *sim_log ;
extern FILE *EPS_Output, *EPS_Wolbachia_Output, *EPS_Density_Output, *EPS_XT_Output, *Output, *Short_Output, *Output_per_House, *Output_per_Container_Type, *Output_Water_Level, *fstbydist;


void Get_Date (void) 
{
  time_t dat ;
  char * str ;
  char *day, *month, *num, *year ;

  //formatted_date = "" ;
  //fprintf(stdout, "before time and ctime ...") ;


  dat = time(NULL) ;
  str = ctime(&dat) ;
  //fprintf(stdout, "\n%s\n", str) ;
  

  day = strtok(str, " ") ;
  //fprintf(stdout, "day %s\n", day) ;
  month = strtok(NULL, " ") ;
  //fprintf(stdout, "month %s\n", month) ;
  num = strtok(NULL, " ") ;
  //fprintf(stdout, "num %s\n", num) ;
  hour = strtok(NULL, " ") ;
  //fprintf(stdout, "hour %s\n", hour) ;
  year = strtok(NULL, "\n") ;
  //fprintf(stdout, "year %s\n", year) ;

  strcat(date, year) ;
  //fprintf(stdout, "f %s\n", date) ;
  strcat(date, ".") ;
  strcat(date, month) ;
  strcat(date, ".") ;
  if (strlen(num) == 1) strcat(date, "0") ;
  strcat(date, num) ;

  //fprintf(stdout, "\n%s\n",date) ;

}


void Write_HTML(double Img)
{
  int kmax=(int)pow((double)2, CHROM_NUMBER-1) ;
 
  fprintf(HTML_Global_Output, "<HTML>\n");
  fprintf(HTML_Global_Output, "<HEAD>\n");
  fprintf(HTML_Global_Output, "<TITLE>Output -- Simulation #%d on %s</TITLE>\n", ref, __DATE__);
  fprintf(HTML_Global_Output, "</HEAD>\n");
  fprintf(HTML_Global_Output, "<BODY>\n");
  fprintf(HTML_Global_Output, "<a name=\"top\">\n") ;
  fprintf(HTML_Global_Output, "<h1>Output -- Simulation #%d</h1>\n", ref);
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><b>Top</b> -<font size=-1> <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> - <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<b>Simulation description :</b><br>\n") ;
  fprintf(HTML_Global_Output, "%s %s<br>\n", __DATE__, __TIME__);
  fprintf(HTML_Global_Output, "Type : <b>%s</b><br>\n", (SIMULATION_MODE2 ?  "stochastic" : "deterministic")) ;
  fprintf(HTML_Global_Output, "Method : <b>%s</b><br>\n", (REPRODUCTION_MODE ? "sampling" : "calculation")) ;  
  fprintf(HTML_Global_Output, "Number of days : <b>%d</b><br>\n", NUMBER_OF_DAYS) ;
  fprintf(HTML_Global_Output, "Number of chromosomes : <b>%d</b><br>\n", CHROM_NUMBER) ;
  fprintf(HTML_Global_Output, "Number of houses : <b>%d</b><br>\n", NUMBER_OF_HOUSES) ;
  fprintf(HTML_Global_Output, "Dispersal : ") ;
  if (NULLIPAROUS_FEMALE_ADULT_DISPERSAL || PAROUS_FEMALE_ADULT_DISPERSAL || MALE_ADULT_DISPERSAL) 
    {
	  fprintf(HTML_Global_Output, "<br><b>SHORT RANGE</b> : ");
      fprintf(HTML_Global_Output, "<font size=\"-1\">\n") ;
      fprintf(HTML_Global_Output, "nulliparous females <b>%.3lf</b> - ", NULLIPAROUS_FEMALE_ADULT_DISPERSAL) ;
      fprintf(HTML_Global_Output, "parous females <b>%.3lf</b> - ", PAROUS_FEMALE_ADULT_DISPERSAL) ;
      fprintf(HTML_Global_Output, "males <b>%.3lf</b>\n", MALE_ADULT_DISPERSAL) ;
      fprintf(HTML_Global_Output, "</font>\n");
    }
  else fprintf(HTML_Global_Output, "<b>*NO SHORT RANGE* </b>\n") ;
  if (NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL || PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL || MALE_ADULT_LONG_RANGE_DISPERSAL) 
    {
      fprintf(HTML_Global_Output, "<br><b>LONG RANGE</b> (max dist = %d) : ", LR_DISPERSAL_MAX_DISTANCE);
      fprintf(HTML_Global_Output, "<font size=\"-1\">\n") ;
      fprintf(HTML_Global_Output, "nulliparous females <b>%.3lf</b> - ", NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL) ;
      fprintf(HTML_Global_Output, "parous females <b>%.3lf</b> - ", PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL) ;
      fprintf(HTML_Global_Output, "males <b>%.3lf</b><br />\n", MALE_ADULT_LONG_RANGE_DISPERSAL) ;
      fprintf(HTML_Global_Output, "</font>\n");
    }
  else fprintf(HTML_Global_Output, "<b>*NO LONG RANGE*</b><br>\n") ;

  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"eggs\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> -</font> <b>Eggs</B> <font size=-1>- <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> - <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/eggsref%d.png\"></CENTER><br clear=\"left\">\n", ref);
  fprintf(HTML_Global_Output, "\n");
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"larvae\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> -</font> <b>Larvae</b> <font size=-1>- <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> - <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/larvaeref%d.png\"></CENTER><br clear=\"left\">\n", ref);
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"pupae\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> -</font> <b>Pupae</b> <font size=-1>- <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> - <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/pupaeref%d.png\"></CENTER><br clear=\"left\">\n", ref);
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"adults\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> -</font> <b>Adults</b> <font size=-1>- <a href=\"#allele_freq\">Allele frequencies</a> - <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<font size=-1><br><CENTER><a href=\"adults.ref%d.html#111\">Edit graphics</a></CENTER></font><br>\n", ref); 
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/adultsref%d.png\"></CENTER><br clear=\"left\">\n", ref);
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"allele_freq\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> -</font> <b>Allele frequencies</b> <font size=-1>- <a href=\"#stats\">Spatial statistics</a> - <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE && MD_TYPE==1)
		fprintf(HTML_Global_Output, "<CENTER><img src=\"images/md_gamete_freq.ref%d.png\"></CENTER><br clear=\"left\">\n",ref);
  else
  {
  fprintf(HTML_Global_Output, "<font size=-1><br><CENTER><a href=\"allelefreq.ref%d.html#%d\">Edit graphics</a></CENTER></font><br>\n", ref, kmax-1); 
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/allele_freqref%d.png\"></CENTER><br clear=\"left\">\n",ref);
  }
  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/sex_allele_freqref%d.png\"></CENTER><br clear=\"left\">\n",ref);
  if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA) fprintf(HTML_Global_Output, "<CENTER><img src=\"images/wolbachia.ref%d.png\"></CENTER><br clear=\"left\">\n",ref);
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"stats\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> -</font> <b>Spatial statistics</b> <font size=-1>- <a href=\"#fstats\">F-stats</a></font></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  if (SPATIAL_STATS)
  {
	  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/kstats.ref%d.png\"></CENTER><br clear=\"left\">\n", ref); 
	  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/kstats-diff.ref%d.png\"></CENTER><br clear=\"left\">\n", ref);
	  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/moransi.ref%d.png\"></CENTER><br clear=\"left\">\n", ref);
	  fprintf(HTML_Global_Output, "<CENTER><b>Global Moran's I = %.5f</b></CENTER><br clear=\"left\">\n", Img);

  }
  else
  {
	  fprintf(HTML_Global_Output, "<br />\n<CENTER>Option turned off</CENTER>\n<br />\n");
  }
  fprintf(HTML_Global_Output, "<p>\n");
  fprintf(HTML_Global_Output, "<a name=\"fstats\">\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  fprintf(HTML_Global_Output, "<CENTER><font size=-1><a href=\"#top\">Top</a> - <a href=\"#eggs\">Eggs</a> - <a href=\"#larvae\">Larvae</a> - <a href=\"#pupae\">Pupae</a> - <a href=\"#adults\">Adults</a> - <a href=\"#allele_freq\">Allele frequencies</a> -<a href=\"#eggs\">Spatial statistics</a> -</font> <b>F-stats</b></CENTER>\n");
  fprintf(HTML_Global_Output, "<hr width=70%%>\n");
  if (FSTATS)
  {
	  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/fst.ref%d.png\"></CENTER><br clear=\"left\">\n", ref); 
	  fprintf(HTML_Global_Output, "<CENTER><img src=\"images/fstbydist.ref%d.png\"></CENTER><br clear=\"left\">\n", ref);
	  FILE *fitparam = fopen("fit_values.txt", "r");
	  double a,b,a_err,b_err ;
	  char c;
	  do fscanf(fitparam, "%c", &c); while (c!='=');
	  fscanf(fitparam, "%lf", &a);
	  do fscanf(fitparam, "%c", &c); while (c!='=');
	  fscanf(fitparam, "%lf", &b);
	  do fscanf(fitparam, "%c", &c); while (c!='=');
	  fscanf(fitparam, "%lf", &a_err);
	  do fscanf(fitparam, "%c", &c); while (c!='=');
	  fscanf(fitparam, "%lf", &b_err);
	  fprintf(HTML_Global_Output, "<CENTER><b>\n");
	  fprintf(HTML_Global_Output, "Fit : f(x) = a*x+b<br /></b><p>\n");
	  fprintf(HTML_Global_Output, "a = %.3g &#177 %.3g (%3.1f %%)<br />\n", a, a_err, 100*a_err/a);
	  fprintf(HTML_Global_Output, "b = %.3g &#177 %.3g (%3.1f %%)<br />\n", b, b_err, 100*b_err/b);
	  fprintf(HTML_Global_Output, "</CENTER>\n");
	  fprintf(HTML_Global_Output, "<p><p>\n");
	  fprintf(HTML_Global_Output, "<CENTER><b>Mantel test results :</b><br />\n");
	  fprintf(HTML_Global_Output, "<pre>\n");
	  FILE *ztresults=fopen("zt_results.txt", "r");
	  char line[200];
	  while(!feof(ztresults))
	  {
		fgets(line, 200, ztresults);
		fputs(line, HTML_Global_Output);
	  }
	  fclose(ztresults);
	  fprintf(HTML_Global_Output, "</pre>\n");
  }
  else
  {
	  fprintf(HTML_Global_Output, "<br />\n<CENTER>Option turned off</CENTER>\n<br />\n");
  }
  
  fprintf(HTML_Global_Output, "</BODY>\n");
  fprintf(HTML_Global_Output, "</HTML>  \n");
  
  // _____________________________
  //|                             |
  //| editing graphics for adults |
  //|_____________________________|


  fprintf(HTML_Adults_Output, "<HTML>\n") ;
  fprintf(HTML_Adults_Output, "<HEAD>\n") ;
  fprintf(HTML_Adults_Output, "</HEAD>\n") ;
  fprintf(HTML_Adults_Output, "<BODY>\n") ;
  fprintf(HTML_Adults_Output, "<h2>Editing adults time series</h2>\n") ;
  fprintf(HTML_Adults_Output, "<a href=\"#111\">back to graphics</a>\n") ;
  fprintf(HTML_Adults_Output, "<p><a name=111>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><b>On</b>/<a href=\"#011\">Off</a> : Nul. females<br><b>On</b>/<a href=\"#101\">Off</a> : Par. females<br><b>On</b>/<a href=\"#110\">Off</a> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=011>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsmpref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><a href=\"#111\">On</a>/<b>Off</b> : Nul. females<br><b>On</b>/<a href=\"#001\">Off</a> : Par. females<br><b>On</b>/<a href=\"#010\">Off</a> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=101>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsnmref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><b>On</b>/<a href=\"#001\">Off</a> : Nul. females<br><a href=\"#111\">On</a>/<b>Off</b> : Par. females<br><b>On</b>/<a href=\"#100\">Off</a> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=110>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsnpref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><b>On</b>/<a href=\"#010\">Off</a> : Nul. females<br><b>On</b>/<a href=\"#100\">Off</a> : Par. females<br><a href=\"#111\">On</a>/<b>Off</b> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=100>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsnref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><b>On</b>/<i>Off</i> : Nul. females<br><a href=\"#110\">On</a>/<b>Off</b> : Par. females<br><a href=\"#101\">On</a>/<b>Off</b> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=010>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultspref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><a href=\"#110\">On</a>/<b>Off</b> : Nul. females<br><b>On</b>/<i>Off</i> : Par. females<br><a href=\"#011\">On</a>/<b>Off</b> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<a name=001>\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsmref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "<font size=-1><br>Edit graphics :<br><a href=\"#101\">On</a>/<b>Off</b> : Nul. females<br><a href=\"#011\">On</a>/<b>Off</b> : Par. females<br><b>On</b>/<i>Off</i> : Males<p><a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
  fprintf(HTML_Adults_Output, "<p>\n") ;
  fprintf(HTML_Adults_Output, "<!-- Just to fill in the bottom of the page - Find another way -->\n") ;
  fprintf(HTML_Adults_Output, "<CENTER><img alt=\"Time series for adults\" align=\"left\" src=\"images/adultsref%d.png\"></CENTER>\n", ref) ;
  fprintf(HTML_Adults_Output, "</BODY>\n") ;
  fprintf(HTML_Adults_Output, "</HTML>\n") ;

  // _________________________________________
  //|                                         |
  //| editing graphics for allele frequencies |
  //|_________________________________________|

  fprintf(HTML_AlleleFreq_Output, "<HTML>\n") ;
  fprintf(HTML_AlleleFreq_Output, "<HEAD>\n") ;
  fprintf(HTML_AlleleFreq_Output, "</HEAD>\n") ;
  fprintf(HTML_AlleleFreq_Output, "<BODY>\n") ;
  fprintf(HTML_AlleleFreq_Output, "<h2>Editing adults time series</h2>\n") ;
  fprintf(HTML_AlleleFreq_Output, "<a href=\"#%d\">back to graphics</a>\n", kmax-1) ;
  
  for (int k=kmax-1 ; k>=0 ; k--)
    {
      fprintf(HTML_AlleleFreq_Output, "<p>\n") ;
      fprintf(HTML_AlleleFreq_Output, "<a name=\"%d\">\n", k) ;
      fprintf(HTML_AlleleFreq_Output, "<CENTER><img align=\"left\" src=\"images/allele_freq.%d.ref%d.png\"></CENTER>\n", k, ref) ;
      fprintf(HTML_AlleleFreq_Output, "<font size=-1><br>Edit graphics :<br>") ;
      for (int chrom=0 ; chrom<CHROM_NUMBER-1 ; chrom++)
	{
	  int p=(int)pow((double)2, chrom) ;
	  if ((k&p)/p)
	    {
	      int offtarget = k-p ;
	      if (offtarget)
		fprintf(HTML_AlleleFreq_Output, "<b>On</b>/<a href=\"#%d\">Off</a> : chromosome %d<br>\n", offtarget, chrom) ;
	      else
		fprintf(HTML_AlleleFreq_Output, "<b>On</b>/<i>Off</i> : chromosome %d<br>\n", chrom) ;
	    }
	  else
	    fprintf(HTML_AlleleFreq_Output, "<a href=\"#%d\">On</a>/<b>Off</b> : chromosome %d<br>\n", k+p, chrom) ;
	}
      fprintf(HTML_AlleleFreq_Output, "<a href=\"global.ref%d.html\">Back to main output</a><br clear=\"left\">\n", ref) ;
    }
  fprintf(HTML_AlleleFreq_Output, "<p>\n") ;
  fprintf(HTML_AlleleFreq_Output, "<!-- Just to fill in the bottom of the page - Find another way -->\n") ;
  fprintf(HTML_AlleleFreq_Output, "<CENTER><img align=\"left\" src=\"images/allele_freq.%d.ref%d.png\"></CENTER>\n", kmax-1, ref) ;
  fprintf(HTML_AlleleFreq_Output, "</BODY>\n") ;
  fprintf(HTML_AlleleFreq_Output, "</HTML>\n") ;
}

void Write_GNUplot(void)
{
  int pos ;

  
  fprintf(GNUplot_command_file, "set terminal png\n");
  fprintf(GNUplot_command_file, "set xlabel \"Day\"\n");
  fprintf(GNUplot_command_file, "set output \"images/eggsref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:2 w linespoints t \"Eggs\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/larvaeref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:3 w linespoints t \"Female Larvae\", \'\' u 1:4 w linespoints lt 3 pt 3 t \"Male Larvae\", \'\' u 1:($3+$4) w linespoints lt -1 pt 13 t \"Total Larvae\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/pupaeref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:5 w linespoints t \"Female Pupae\", \'\' u 1:6 w linespoints lt 3 pt 3 t \"Male Pupae\", \'\' u 1:($5+$6) w linespoints lt -1 pt 13 t \"Total Pupae\"\n", ref);
  fprintf(GNUplot_command_file, "set yrange [] writeback\n") ;
  fprintf(GNUplot_command_file, "set output \"images/adultsref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:7 w lp lt 1 pt 1 t \"Nulliparous females\", '' u 1:8 w lp lt 2 pt 2 t \"Parous females\", '' u 1:9 w lp lt 3 pt 3 t \"Males\"\n", ref);
  fprintf(GNUplot_command_file, "set yrange restore\n") ;
  fprintf(GNUplot_command_file, "set output \"images/adultsnref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:7 w lp lt 1 pt 1 t \"Nulliparous females\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/adultspref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:8 w lp lt 2 pt 2 t \"Parous females\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/adultsmref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:9 w lp lt 3 pt 3 t \"Males\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/adultsnpref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:7 w lp lt 1 pt 1 t \"Nulliparous females\", '' u 1:8 w lp lt 2 pt 2 t \"Parous females\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/adultsnmref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:7 w lp lt 1 pt 1 t \"Nulliparous females\", '' u 1:9 w lp lt 3 pt 3 t \"Males\"\n", ref);
  fprintf(GNUplot_command_file, "set output \"images/adultsmpref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:8 w lp lt 2 pt 2 t \"Parous females\", '' u 1:9 w lp lt 3 pt 3 t \"Males\"\n", ref);
  fprintf(GNUplot_command_file, "set autoscale y\n") ;
 
  // calculating the position of allelic frequencies in output file
  pos = (int)pow((double)2, 2*CHROM_NUMBER) ; // total genotypes
  pos *=2 ; // total adult genotypes
  pos += 9 ; // 9 fixed values at the beginning of the file
  
  fprintf(GNUplot_command_file, "set yrange [] writeback\n") ;
  fprintf(GNUplot_command_file, "set output \"images/allele_freqref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:%d w linespoints t \"Allele 0 Chromosome 0\"",ref,pos+1);
  for (int i=1;i<CHROM_NUMBER-1;i++) 
    fprintf(GNUplot_command_file, ",'' u 1:%d w linespoints t \"Allele 0 Chromosome %d\"", pos+2*i+1,i); 
  fprintf(GNUplot_command_file, "\n");
  fprintf(GNUplot_command_file, "set yrange restore\n") ;
    
  int kmax=(int)pow((double)2, CHROM_NUMBER-1) ;
  
  for (int k=0 ; k<kmax ; k++)
    {
      bool first=true ;
      fprintf(GNUplot_command_file, "set output \"images/allele_freq.%d.ref%d.png\n", k, ref) ;
      fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" ", ref);
      for (int chrom=0; chrom<CHROM_NUMBER-1; chrom++)
	{
	  int p=(int)pow((double)2, chrom) ;
	  if ((k&p)/p)
	    {
	      if (first) first=false ;
	      else fprintf(GNUplot_command_file, ", '' ") ;

	      fprintf(GNUplot_command_file, "u 1:%d w lp lt %d pt %d t \"Allele 0 Chromosome %d\"", pos+2*chrom+1, chrom+1, chrom+1, chrom) ;
	    }
	}
      fprintf(GNUplot_command_file, "\n") ;
    }
  
  fprintf(GNUplot_command_file, "set autoscale y\n") ;
  fprintf(GNUplot_command_file, "set output \"images/sex_allele_freqref%d.png\"\n", ref);
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:(2*$%d) w linespoints t \"Proportion of males\"\n",ref,pos+2*CHROM_NUMBER);
  fprintf(GNUplot_command_file, "set output \"images/wtf%d.png\"\n", ref) ;
  fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" u 1:(($10+$11)/($2+$3+$4+$5+$6+$7+$8+$9)) w linespoints t \"Wild-type genotype frequency\"\n",ref);
 
  if (GENETIC_CONTROL_METHOD==MEIOTIC_DRIVE && MD_TYPE==1)
  // not working
    /*{
      int nb = (int)pow(2,6) ;
      int *YD= new int [nb];
	  int *Yd= new int [nb];
	  int *Xit= new int [nb];
	  int *Xsn= new int [nb];
      int i_YD=0, i_Yd=0, i_Xit=0, i_Xsn=0  ;
      for (int g=0 ; g<nb ; g++)
	{
	  /*	  if ((g&1)&&(!(g&2)))
	    {
	      if (!(g&12))
		Yd[i_Yd++]=g ; 
	      if ((g&4)&&(!(g&8)))  
		YD[i_YD]=g ;
	      if (!(g&48))
		Xsn[i_Xsn++]=g ;
	      if ((g&16)&&(!(g&32)))
		Xit[i_Xit++]=g ;
	    }
	  if (!(g&3))
	    {
	      if (!(g&48))
		Xsn[i_Xsn++]=g ;
	      if ((g&16)&&(!(g&32)))
		Xit[i_Xit++]=g ;
	      if (!(g&12))
		Xsn[i_Xsn++]=g ;
	      if ((g&4)&&(!(g&8)))  
		Xit[i_Xit++]=g ;
		}*//*

	  if (!(g&48))
	    Xsn[i_Xsn++]=g ;
	  if ((g&16)&&(!(g&32)))
	    Xit[i_Xit++]=g ;
	  // if ((g&16)&&(g&32))
	  //Xin[i_Xin++]=g ;
	  
	  if ((!(g&12)) && (!(g&3)))
	    Xsn[i_Xsn++]=g ;
	  if (((g&4)&&(!(g&8))) && (!(g&3)))
	    Xit[i_Xit++]=g ;
	  //	  if (((g&8)&&(g&4))&&(!(g&3)))
	  //Xin[i_Xin++]=g ;
	  if ((!(g&12))&&((g&1)&&(!(g&2))))
	    Yd[i_Yd++]=g ;
	  if (((g&4)&&!((g&8)))&&((g&1)&&(!(g&2))))
	    YD[i_YD++]=g ;
 
	}*/
    {
      fprintf(GNUplot_command_file, "set output \"images/md_gamete_freq.ref%d.png\"\n", ref) ;
      fprintf(GNUplot_command_file, "plot \"Short_Output.ref%d.txt\" ", ref) ;
      // YD
      fprintf(GNUplot_command_file, "u (($15+$31+$63)/($11+$15+$27+$31+$59+$63)) w lp t \"YD\"") ;
      /*fprintf(GNUplot_command_file, "$%d", YD[0]) ;
      for (int i=1 ; i<i_YD ; i++)
	fprintf(GNUplot_command_file, "+$%d", YD[i]) ;
      fprintf(GNUplot_command_file, ")/($%d", YD[0]) ;
      for (int i=1 ; i<i_YD ; i++)
	fprintf(GNUplot_command_file, "+$%d", YD[i]) ;
      for (int i=0 ; i<i_Yd ; i++)
	fprintf(GNUplot_command_file, "+$%d", Yd[i]) ;
      fprintf(GNUplot_command_file, ")) w lp t \"YD\"") ;*/

      // Xit
	  fprintf(GNUplot_command_file, ", '' u (($14+$26+$27+$30+$30+$31+$38+$62)/($10+$10+$11+$14+$14+$15+$22+$22+$26+$26+$27+$30+$30+$31+$38+$58+$58+$59+$62+$62+$63+$70)) w lp t \"Xit\"\n") ;
	  /*fprintf(GNUplot_command_file, ", '' u ((") ;
      fprintf(GNUplot_command_file, "$%d", Xit[0]) ;
      for (int i=1 ; i<i_Xit ; i++)
	fprintf(GNUplot_command_file, "+$%d", Xit[i]) ;
      fprintf(GNUplot_command_file, ")/($%d", Xit[0]) ;
      for (int i=1 ; i<i_Xit ; i++)
	fprintf(GNUplot_command_file, "+$%d", Xit[i]) ;
      for (int i=0 ; i<i_Xsn ; i++)
	fprintf(GNUplot_command_file, "+$%d", Xsn[i]) ;
      fprintf(GNUplot_command_file, ")) w lp t \"Xit\"\n") ;
	  */

	  /*delete [] YD;
	  delete [] Yd;
	  delete [] Xit;
	  delete [] Xsn;*/
    }

	// process wolbachia frequencies (if any)

	if (NUMBER_OF_INCOMPATIBLE_WOLBACHIA>0)
	{
		fprintf(GNUplot_command_file, "set output \"images/wolbachia.ref%d.png\"\n", ref);
		fprintf(GNUplot_command_file, "plot [][0:] \"Short_Output.ref%d.txt\" u 1:%d w lp t \"Wolbachia #1\"", ref, pos+2*CHROM_NUMBER+1);
		for (int ww=1 ; ww<NUMBER_OF_INCOMPATIBLE_WOLBACHIA ; ww++)
		{
			fprintf(GNUplot_command_file, ", \"\" u 1:%d w lp t \"Wolbachia #%d\"", pos+2*CHROM_NUMBER+1+ww, 1+ww);
		}
		fprintf(GNUplot_command_file, "\n");
	}

	// process spatial statistics
	if (SPATIAL_STATS)
	{
		fprintf(GNUplot_command_file, "set xlabel \"Distance\"\n") ;
		fprintf(GNUplot_command_file, "set output \"images/kstats.ref%d.png\"\n", ref) ;
		fprintf(GNUplot_command_file, "plot \"kstats.ref%d.txt\" u 1:2 w lp t \"L(d)\", \"\" u 1:3 w lp t \"L*(d)\", \"\" u 1:4 w lp t \"Lw(d)\", \"\" u 1:5 w lp t \"Lw*(d)\", x lt 0 t \"d\"\n", ref) ;
		fprintf(GNUplot_command_file, "set output \"images/kstats-diff.ref%d.png\"\n", ref) ;
		fprintf(GNUplot_command_file, "plot \"kstats.ref%d.txt\" u 1:6 w lp lt 3 pt 3 t \"dLw-dL\", \"\" u 1:7 w lp lt 4 pt 4 t \"dLw*-dL*\", 0 w l lt 0\n", ref) ;
		fprintf(GNUplot_command_file, "set output \"images/moransi.ref%d.png\"\n", ref);
		fprintf(GNUplot_command_file, "set ylabel \"Moran I\"\n");
		fprintf(GNUplot_command_file, "plot \"moransi.ref%d.txt\" u 1:2 w lp notitle, 0 w l lt 0 notitle\n", ref);
		fprintf(GNUplot_command_file, "set ylabel\n");
		fprintf(GNUplot_command_file, "set output\n");
	}

	if (FSTATS)
	{
		fprintf(GNUplot_command_file, "set xlabel \"Day\"\n");
		fprintf(GNUplot_command_file, "set output \"images/fst.ref%d.png\"\n", ref) ;
		fprintf(GNUplot_command_file, "plot [][0:] \"Short_Output.ref%d.txt\" u 1:%d w linespoints t \"Fst\"\n",ref,pos+2*CHROM_NUMBER+2+NUMBER_OF_INCOMPATIBLE_WOLBACHIA);
		fprintf(GNUplot_command_file, "set print \"fit_initial.txt\"\n");
		fprintf(GNUplot_command_file, "a=1 ; b=1\n");
		fprintf(GNUplot_command_file, "print \"a=\",a\n");
		fprintf(GNUplot_command_file, "print \"b=\",b\n");
		fprintf(GNUplot_command_file, "set print\n");
		fprintf(GNUplot_command_file, "f(x)=a*x+b\n");
		fprintf(GNUplot_command_file, "set fit errorvariables\n");
		fprintf(GNUplot_command_file, "fit f(x) \"fstbydist.ref%d.txt\" u 1:2 via \"fit_initial.txt\"\n", ref);
		fprintf(GNUplot_command_file, "update \"fit_initial.txt\" \"fit_values.txt\"\n");
		fprintf(GNUplot_command_file, "set print \"fit_values.txt\" append\n");
		fprintf(GNUplot_command_file, "print \"a_err=\",a_err\n");
		fprintf(GNUplot_command_file, "print \"b_err=\",b_err\n");		
		fprintf(GNUplot_command_file, "set print\n");
		fprintf(GNUplot_command_file, "set xlabel \"ln(Distance)\"\n");
		fprintf(GNUplot_command_file, "set ylabel \"Fst / (1-Fst)\"\n");		
		fprintf(GNUplot_command_file, "set output \"images/fstbydist.ref%d.png\"\n", ref);
		fprintf(GNUplot_command_file, "plot \"fstbydist.ref%d.txt\" u 1:2 w p notitle, f(x) w l lt -1 notitle\n", ref);
		
	}

}


int Get_Number(void)
{
  int reff, i=1, eof ;
  char c=' ';
 
 
  sim_log = fopen("simulations.log", "a+") ;
  fseek(sim_log, 0, SEEK_END) ;
  eof = ftell(sim_log) ;
  fseek(sim_log, -1, SEEK_END) ;
  while (++i, c!='#' && ftell(sim_log) && !feof(sim_log))
    {
      fseek(sim_log, -i, SEEK_END) ;
      fscanf(sim_log, "%c", &c) ;
     }
  
  if (c!='#') reff=0 ;
  else fscanf(sim_log, "%d", &reff) ;
  reff++ ;
  
  fseek(sim_log, 0L, SEEK_SET) ; 

  // logging the simulation parameters description

  fprintf(sim_log, "\n===================================================================================\n") ;
  fprintf(sim_log, "Simulation #%3d :\n", reff);
  fprintf(sim_log, "-----------------\n\n") ;
  fprintf(sim_log, "%s %s\n", date, hour);
  fprintf(sim_log, "Based on SkeeterBuster with Compile Date = %s\n", __DATE__);
  fprintf(sim_log, "Based on SkeeterBuster with Compile Time = %s\n", __TIME__);
  fprintf(sim_log, "%d days, %d houses\n", NUMBER_OF_DAYS, NUMBER_OF_HOUSES) ;
  fprintf(sim_log, "%d chromosomes\n", CHROM_NUMBER) ;
  fprintf(sim_log, "Genetic control method : ");
  switch(GENETIC_CONTROL_METHOD)
  {
  case 0:
	  fprintf(sim_log, "NEUTRAL\n");
	  break;

  case 1:
	  fprintf(sim_log, "ENGINEERED UNDERDOMINANCE -- TYPE ") ;
	  if (EU_TYPE >=1 && EU_TYPE<=4) fprintf(sim_log, "%d\n", EU_TYPE) ;
	  else fprintf(sim_log, "unknown\n") ;
	  fprintf(sim_log, "\tFitness cost construct A: %.3f\n", FITNESS_COST_CONSTRUCT_ALPHA);
	  fprintf(sim_log, "\tFitness cost construct B: %.3f\n", FITNESS_COST_CONSTRUCT_BETA);
	  if (EU_TYPE>=2) fprintf(sim_log, "\tFitness cost construct C: %.3f\n", FITNESS_COST_CONSTRUCT_GAMMA);
	  if (EU_TYPE>=3) fprintf(sim_log, "\tFitness cost construct D: %.3f\n", FITNESS_COST_CONSTRUCT_DELTA);
	  fprintf(sim_log, "\tDominance of fitness costs: %.3f\n", FITNESS_COST_DOMINANCE);
	  
	  break;

  case 2:
	  fprintf(sim_log, "WOLBACHIA\n") ;
	  break;

  case 3:
	  fprintf(sim_log, "MEIOTIC DRIVE -- SCENARIO ") ;
	  if (MD_TYPE >=1 && MD_TYPE<=3) fprintf(sim_log, "%d\n", MD_TYPE) ;
	  else fprintf(sim_log, "unknown\n") ;
	  fprintf(sim_log, "\tFitness cost Xit: %.3f\n", FITNESS_COST_INSENSITIVE_TRANSGENE );
	  if (MD_TYPE>=2) fprintf(sim_log, "\tFitness cost Xin: %.3f\n", FITNESS_COST_INSENSITIVE_NATURAL );
	  fprintf(sim_log, "\tFitness cost YD: %.3f\n", FITNESS_COST_DRIVE_GENE );
	  fprintf(sim_log, "\tDominance of fitness costs: %.3f\n", MD_FITNESS_COST_DOMINANCE);
	  fprintf(sim_log, "\tDrive strength on Xsn: %.3f\n", DRIVE_STRENGTH_SN );
	  fprintf(sim_log, "\tDrive strength on Xit: %.3f\n", DRIVE_STRENGTH_IT );
	  if (MD_TYPE>=2) fprintf(sim_log, "\tDrive strength on Xin: %.3f\n", DRIVE_STRENGTH_IN );
	  if (MD_TYPE>=3) fprintf(sim_log, "\tDrive strength on Xsn+modifier: %.3f\n", DRIVE_STRENGTH_MN );

	  break;

  case 4:
	  fprintf(sim_log, "MEDEA\n") ;
	  fprintf(sim_log, "\tNumber of MEDEA constructs: %d\n", MEDEA_UNITS);
	  fprintf(sim_log, "\tCross rescuing elements: %s\n", MEDEA_CROSS_RESCUE?"YES":"NO");
	  fprintf(sim_log, "\tMaternal lethality: %.3f\n", MEDEA_MATERNAL_LETHALITY);
	  fprintf(sim_log, "\tFitness cost per construct: %.3f\n", MEDEA_FITNESS_COST_PER_CONSTRUCT);
	  fprintf(sim_log, "\tFitness cost dominance: %.3f\n", MEDEA_FITNESS_COST_DOMINANCE); 
	  
	  break;

  
  case 6:
	  if (IS_RESCUE) fprintf(sim_log, "KILLING+RESCUE");
	  else fprintf(sim_log, "KILLING");
	  fprintf(sim_log, "\tNumber of killing loci: %d\n", NUMBER_OF_FEMALE_KILLING_LOCI);
	  fprintf(sim_log, "\tFemale-killing only: %s\n", FKR_IS_EMBRYO_KILLING?"NO":"YES");
	  fprintf(sim_log, "\tKilling efficiency: %.3f\n", FEMALE_KILLING_EFFICIENCY);
	  fprintf(sim_log, "\tKilling alleles homozygous fitness cost: %.3f\n", KILLING_ALLELE_HOMOZYGOUS_FITNESS_COST);
	  fprintf(sim_log, "\tKilling alleles fitness cost dominance: %.3f\n", KILLING_ALLELE_FITNESS_COST_DOMINANCE);
	  if (IS_RESCUE)
	  {
		  fprintf(sim_log, "\tRescue alleles homozygous fitness cost: %.3f\n", RESCUE_ALLELE_HOMOZYGOUS_FITNESS_COST);
		  fprintf(sim_log, "\tRescue alleles fitness cost dominance: %.3f\n", RESCUE_ALLELE_FITNESS_COST_DOMINANCE);
	  }

	  break;

  case 8:
	  fprintf(sim_log, "REDUCE_REPLACE\n") ;
	  fprintf(sim_log, "\tScenario: %d\n", SCENARIO );
	  fprintf(sim_log, "Construct gene fitness cost applied at stage: %d\n", FITNESS_COST_STAGE_RR);
	  break;

  default:
	  fprintf(sim_log, "unknown\n") ;
	  break;
  }

  fprintf(sim_log, "Fitness costs are applied at stage %d at age %d\n", FITNESS_COST_STAGE, FITNESS_COST_STAGE_SPECIFIC_AGE);

  fprintf(sim_log, "Border condition type : ") ;
  switch(DISPERSAL_MODE)
  {
  case 0:
	  fprintf(sim_log, "STICKY BORDERS\n") ;
	  break;
  case 1:
	  fprintf(sim_log, "BOUNCING BORDERS\n") ;
	  break;
  case 2:
	  fprintf(sim_log, "TORE\n") ;
	  break;
  case 3:
	  fprintf(sim_log, "RANDOM EDGES\n") ;
	  break;
  default:
	  fprintf(sim_log, "unknown\n") ;
  }
  
  fprintf(sim_log, "Dispersal rates :\n") ;
  fprintf(sim_log, "\t- short range : ") ;
  if (NULLIPAROUS_FEMALE_ADULT_DISPERSAL || PAROUS_FEMALE_ADULT_DISPERSAL || MALE_ADULT_DISPERSAL)
  {
  fprintf(sim_log, "\n") ;
  fprintf(sim_log, "\t    n = %.3lf\n", NULLIPAROUS_FEMALE_ADULT_DISPERSAL) ;
  fprintf(sim_log, "\t    p = %.3lf\n", PAROUS_FEMALE_ADULT_DISPERSAL) ;
  fprintf(sim_log, "\t    m = %.3lf\n", MALE_ADULT_DISPERSAL) ;
  }
  else fprintf(sim_log, "NONE\n") ;
  fprintf(sim_log, "\t- long range : ") ;
  if (NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL || PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL || MALE_ADULT_LONG_RANGE_DISPERSAL)
  {
  fprintf(sim_log, "\n") ;
  fprintf(sim_log, "\t    n = %.3lf\n", NULLIPAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL) ;
  fprintf(sim_log, "\t    p = %.3lf\n", PAROUS_FEMALE_ADULT_LONG_RANGE_DISPERSAL) ;
  fprintf(sim_log, "\t    m = %.3lf\n", MALE_ADULT_LONG_RANGE_DISPERSAL) ;
  fprintf(sim_log, "\t    maxdist = %d\n", LR_DISPERSAL_MAX_DISTANCE) ;
  }
  else fprintf(sim_log, "NONE\n") ;

  fprintf(sim_log, "Container properties:\n");
  fprintf(sim_log, "\t- Ct turnover rate: %.5f\n", CONTAINER_MOVEMENT_PROBABILITY);
  fprintf(sim_log, "\t- emptying rate of manually filled Ct: %.3f\n", PROBABILITY_OF_EMPTYING_MANUALLY_FILLED_CONTAINERS);
  fprintf(sim_log, "\t- oviposition reduction in covered Ct: %.3f\n", OVIPOSITION_REDUCTION_IN_COVERED_CONTAINERS);

  fprintf(sim_log, "Female adult survival type : ") ;
  switch(FEMALE_AGE_DEPENDENT_SURVIVAL)
  {
  case 0:
	  fprintf(sim_log, "CONSTANT at %.3lf\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS);
	  break;
  case 1:
	  fprintf(sim_log, "LINEAR DECREASE from %.3lf at age 0 to 0 at age %d\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS, FEMALE_MAXIMUM_AGE_1) ;
	  break ;
  case 2:
	  fprintf(sim_log, "CONSTANT at %.3lf from age 0 to age %d THEN LINEAR DECREASE to 0 at age %d\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_FEMALE_ADULTS, FEMALE_START_SENESCENCE_2, FEMALE_MAXIMUM_AGE_2) ;
	  break ;
  }
  fprintf(sim_log, "Male adult survival type : ") ;
  switch(MALE_AGE_DEPENDENT_SURVIVAL)
  {
  case 0:
	  fprintf(sim_log, "CONSTANT at %.3lf\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS);
	  break;
  case 1:
	  fprintf(sim_log, "LINEAR DECREASE from %.3lf at age 0 to 0.000 at age %d\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS, MALE_MAXIMUM_AGE_1) ;
	  break ;
  case 2:
	  fprintf(sim_log, "CONSTANT at %.3lf from age 0 to age %d THEN LINEAR DECREASE to 0.000 at age %d\n", SKEETER_BUSTER_NOMINAL_DAILY_SURVIVAL_MALE_ADULTS, MALE_START_SENESCENCE_2, MALE_MAXIMUM_AGE_2) ;
	  break ;
  }

  
  
  
  fclose(sim_log) ; 
  return reff;
}

void EPS_Header (int width, int length)
{
  int newwid, newlen, xorigin, yorigin;
  double	  rat ;
  char sref[10]="" ;
	int iloum;

	int oddwidth = (width%2) ; // correction for display bug when column number is odd

	   strcpy(path_eps_output_by_day, "eps/") ;
		#ifndef REF_EPS_OUTPUT
			#ifdef _MSC_VER
				#define REF_EPS_OUTPUT done
				iloum = sprintf(sref, "ref%d/", ref) ;
				_chdir("./eps/"); _mkdir(sref); _chdir("../");// current directory is out/$date/
				strcat(path_eps_output_by_day, sref) ;

			#else
				#define REF_EPS_OUTPUT done
				iloum = sprintf(sref, "ref%d/", ref) ;
				chdir("./eps/"); mkdir(sref, 00755); chdir("../");// current directory is out/$date/
				strcat(path_eps_output_by_day, sref) ;
			#endif
		#endif 
			
	   strcat(path_eps_output_by_day, "image.ref") ;
	   iloum = sprintf(sref, "%d", ref) ;
       strcat(path_eps_output_by_day, sref) ;
       strcat(path_eps_output_by_day, ".day.") ;
       iloum = sprintf(sref, "%04d", Date) ;
       strcat(path_eps_output_by_day, sref) ;
       strcat(path_eps_output_by_day, ".eps") ;
       
       EPS_Output = fopen(path_eps_output_by_day, "w") ;
      

  newwid = width ; newlen = length ;
  rat = 500./newwid ; newwid = 500. ; newlen = rat*newlen ;
  if (newlen > 700.)
    {
      rat = 700./newlen ; newlen = 700. ; newwid = rat*newwid ;
    }
  
  xorigin = (int) (612-newwid)/2 ;
  yorigin = (int) (792-newlen)/2 ;

  fprintf(EPS_Output, "%%!PS-Adobe-2.0 EPSF-2.0\n") ;
  fprintf(EPS_Output, "%%%%BoundingBox: %d %d %d %d\n", xorigin, yorigin, xorigin+newwid+3, yorigin+newlen+23) ;
  fprintf(EPS_Output, "gsave\n") ;
  fprintf(EPS_Output, "/Arial findfont\n") ;
  fprintf(EPS_Output, "14 scalefont\nsetfont\n") ;
  fprintf(EPS_Output, "%d %d moveto\n(%04d) show\n\n", xorigin + (newwid/2), yorigin+6, Date) ; 
  
  // skipping linedrawing because of oddwidth adjustments
  // to be corrected

  //fprintf(EPS_Output, "%d %d moveto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "stroke \n\n") ; 

  fprintf(EPS_Output, "/bufstr %d string def\n\n", (oddwidth ? width+1 : width)) ;
  fprintf(EPS_Output, "%d %d translate\n", xorigin+2, yorigin+22) ;
  fprintf(EPS_Output, "%d %d scale\n\n", newwid, newlen) ;
  fprintf(EPS_Output, "%d %d 4\n", (oddwidth ? width+1 : width), length) ;
  fprintf(EPS_Output, "[%d 0 0 %d 0 %d]\n", (oddwidth ? width+1 : width), -length, length) ;
  fprintf(EPS_Output, "{currentfile bufstr readhexstring pop} bind false 3 colorimage\n");
}

void EPS_Wolbachia_Header (int width, int length)
{
  int newwid, newlen, xorigin, yorigin;
  double	  rat ;
  char sref[10]="" ;
	int iloum;

	int oddwidth = (width%2) ; // correction for display bug when column number is odd

	   strcpy(path_eps_wolbachia_output_by_day, "eps/") ;
		#ifndef REF_EPS3_OUTPUT
			#ifdef _MSC_VER
				#define REF_EPS3_OUTPUT done
				iloum = sprintf(sref, "ref%d/", ref) ;
				_chdir("./eps/"); _mkdir(sref); _chdir("../");// current directory is out/$date/
				strcat(path_eps_wolbachia_output_by_day, sref) ;
			#else
				#define REF_EPS3_OUTPUT done
				iloum = sprintf(sref, "ref%d/", ref) ;
				chdir("./eps/"); mkdir(sref, 00755); chdir("../");// current directory is out/$date/
				strcat(path_eps_wolbachia_output_by_day, sref) ;
			#endif
		#endif 
			
	   strcat(path_eps_wolbachia_output_by_day, "wolbachia.ref") ;
	   iloum = sprintf(sref, "%d", ref) ;
       strcat(path_eps_wolbachia_output_by_day, sref) ;
       strcat(path_eps_wolbachia_output_by_day, ".day.") ;
       iloum = sprintf(sref, "%04d", Date) ;
       strcat(path_eps_wolbachia_output_by_day, sref) ;
       strcat(path_eps_wolbachia_output_by_day, ".eps") ;
       
       EPS_Wolbachia_Output = fopen(path_eps_wolbachia_output_by_day, "w") ;
      

  newwid = width ; newlen = length ;
  rat = 500./newwid ; newwid = 500. ; newlen = rat*newlen ;
  if (newlen > 700.)
    {
      rat = 700./newlen ; newlen = 700. ; newwid = rat*newwid ;
    }
  
  xorigin = (int) (612-newwid)/2 ;
  yorigin = (int) (792-newlen)/2 ;

  fprintf(EPS_Wolbachia_Output, "%%!PS-Adobe-2.0 EPSF-2.0\n") ;
  fprintf(EPS_Wolbachia_Output, "%%%%BoundingBox: %d %d %d %d\n", xorigin, yorigin, xorigin+newwid+3, yorigin+newlen+23) ;
  fprintf(EPS_Wolbachia_Output, "gsave\n") ;
  fprintf(EPS_Wolbachia_Output, "/Arial findfont\n") ;
  fprintf(EPS_Wolbachia_Output, "14 scalefont\nsetfont\n") ;
  fprintf(EPS_Wolbachia_Output, "%d %d moveto\n(%04d) show\n\n", xorigin + (newwid/2), yorigin+6, Date) ; 
  
  // skipping linedrawing because of oddwidth adjustments
  // to be corrected

  //fprintf(EPS_Output, "%d %d moveto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "stroke \n\n") ; 

  fprintf(EPS_Wolbachia_Output, "/bufstr %d string def\n\n", (oddwidth ? width+1 : width)) ;
  fprintf(EPS_Wolbachia_Output, "%d %d translate\n", xorigin+2, yorigin+22) ;
  fprintf(EPS_Wolbachia_Output, "%d %d scale\n\n", newwid, newlen) ;
  fprintf(EPS_Wolbachia_Output, "%d %d 4\n", (oddwidth ? width+1 : width), length) ;
  fprintf(EPS_Wolbachia_Output, "[%d 0 0 %d 0 %d]\n", (oddwidth ? width+1 : width), -length, length) ;
  fprintf(EPS_Wolbachia_Output, "{currentfile bufstr readhexstring pop} bind false 3 colorimage\n");
}

void EPS_Density_Header (int width, int length)
{
  int newwid, newlen, xorigin, yorigin;
 double	  rat ;
char sref[10]="" ;
	int iloum;

	int oddwidth = (width%2) ; // correction for display bug when column number is odd

	   strcpy(path_eps_density_output_by_day, "eps/") ;
		#ifndef REF_EPS2_OUTPUT
			#ifdef _MSC_VER
				#define REF_EPS2_OUTPUT done
		
				iloum = sprintf(sref, "ref%d/", ref) ;
				_chdir("./eps/"); _mkdir(sref); chdir("../"); // current directory is out/$date/
				strcat(path_eps_density_output_by_day, sref) ;
			#else
				#define REF_EPS2_OUTPUT done

				iloum = sprintf(sref, "ref%d/", ref) ;
				chdir("./eps/"); mkdir(sref, 00755); chdir("../"); // current directory is out/$date/
				strcat(path_eps_density_output_by_day, sref) ;
				
			#endif	
		#endif 
			
       strcat(path_eps_density_output_by_day, "density.ref") ;
       iloum = sprintf(sref, "%d", ref) ;
       strcat(path_eps_density_output_by_day, sref) ;
       strcat(path_eps_density_output_by_day, ".day.") ;
       iloum = sprintf(sref, "%04d", Date) ;
       strcat(path_eps_density_output_by_day, sref) ;
       strcat(path_eps_density_output_by_day, ".eps") ;

	   EPS_Density_Output = fopen(path_eps_density_output_by_day, "w") ;
      
  newwid = width ; newlen = length ;
  rat = 500./newwid ; newwid = 500. ; newlen = rat*newlen ;
  if (newlen > 700.)
    {
      rat = 700./newlen ; newlen = 700. ; newwid = rat*newwid ;
    }
  
  xorigin = (int) (612-newwid)/2 ;
  yorigin = (int) (792-newlen)/2 ;

  fprintf(EPS_Density_Output, "%%!PS-Adobe-2.0 EPSF-2.0\n") ;
  fprintf(EPS_Density_Output, "%%%%BoundingBox: %d %d %d %d\n", xorigin, yorigin, xorigin+newwid+3, yorigin+newlen+23) ;
  fprintf(EPS_Density_Output, "gsave\n") ;
  fprintf(EPS_Density_Output, "/Arial findfont\n") ;
  fprintf(EPS_Density_Output, "14 scalefont\nsetfont\n") ;
  fprintf(EPS_Density_Output, "%d %d moveto\n(%04d) show\n\n", xorigin + (newwid/2), yorigin+6, Date) ; 
  
  // skipping linedrawing because of oddwidth adjustments
  // to be corrected

  //fprintf(EPS_Output, "%d %d moveto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "stroke \n\n") ; 

  fprintf(EPS_Density_Output, "/bufstr %d string def\n\n", (oddwidth ? width+1 : width)) ;
  fprintf(EPS_Density_Output, "%d %d translate\n", xorigin+2, yorigin+22) ;
  fprintf(EPS_Density_Output, "%d %d scale\n\n", newwid, newlen) ;
  fprintf(EPS_Density_Output, "%d %d 4\n", (oddwidth ? width+1 : width), length) ;
  fprintf(EPS_Density_Output, "[%d 0 0 %d 0 %d]\n", (oddwidth ? width+1 : width), -length, length) ;
  fprintf(EPS_Density_Output, "{currentfile bufstr readhexstring pop} bind false 3 colorimage\n");
}

void EPS_XT_Header (int width, int length)
{
  int newwid, newlen, xorigin, yorigin;
 double	  rat ;
char sref[10]="" ;
	int iloum;

	int oddwidth = (width%2) ; // correction for display bug when column number is odd

	   strcpy(path_eps_xt_output, "eps/xt.ref") ;
       iloum = sprintf(sref, "%d", ref) ;
       strcat(path_eps_xt_output, sref) ;
       strcat(path_eps_xt_output, ".eps") ;
       
       EPS_XT_Output = fopen(path_eps_xt_output, "w") ;
      

  newwid = width ; newlen = length ;
  rat = 300./newwid ; newwid = 300. ; newlen = rat*newlen ;
  /*if (newlen > 700.)
    {
      rat = 700./newlen ; newlen = 700. ; newwid = rat*newwid ;
    }
  */
  //xorigin = (int) (612-newwid)/2 ;
  //yorigin = (int) (792-newlen)/2 ;
	xorigin = 10 ;
	yorigin = 10 ;

  fprintf(EPS_XT_Output, "%%!PS-Adobe-2.0 EPSF-2.0\n") ;
  fprintf(EPS_XT_Output, "%%%%BoundingBox: %d %d %d %d\n", xorigin, yorigin, xorigin+newwid+3, yorigin+newlen+23) ;
  fprintf(EPS_XT_Output, "gsave\n") ;
  fprintf(EPS_XT_Output, "/Arial findfont\n") ;
  fprintf(EPS_XT_Output, "14 scalefont\nsetfont\n") ;
  //fprintf(EPS_XT_Output, "%d %d moveto\n(%04d) show\n\n", xorigin + (newwid/2), yorigin+6, Date) ; 
  
  // skipping linedrawing because of oddwidth adjustments
  // to be corrected

  //fprintf(EPS_Output, "%d %d moveto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+newlen+22) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+newwid+2, yorigin+21) ;
  //fprintf(EPS_Output, "%d %d lineto\n", xorigin+1, yorigin+21) ;
  //fprintf(EPS_Output, "stroke \n\n") ; 

  fprintf(EPS_XT_Output, "/bufstr %d string def\n\n", (oddwidth ? width+1 : width)) ;
  fprintf(EPS_XT_Output, "%d %d translate\n", xorigin+2, yorigin+22) ;
  fprintf(EPS_XT_Output, "%d %d scale\n\n", newwid, newlen) ;
  fprintf(EPS_XT_Output, "%d %d 4\n", (oddwidth ? width+1 : width), length) ;
  fprintf(EPS_XT_Output, "[%d 0 0 %d 0 %d]\n", (oddwidth ? width+1 : width), -length, length) ;
  fprintf(EPS_XT_Output, "{currentfile bufstr readhexstring pop} bind false 3 colorimage\n");
}

void EPS_Trailer (void)
{
  fprintf(EPS_Output, "\n") ;
  fprintf(EPS_Output, "grestore\n") ;
  fprintf(EPS_Output, "showpage\n") ;
}

void EPS_Wolbachia_Trailer (void)
{
  fprintf(EPS_Wolbachia_Output, "\n") ;
  fprintf(EPS_Wolbachia_Output, "grestore\n") ;
  fprintf(EPS_Wolbachia_Output, "showpage\n") ;
}

void EPS_Density_Trailer (void)
{
  fprintf(EPS_Density_Output, "\n") ;
  fprintf(EPS_Density_Output, "grestore\n") ;
  fprintf(EPS_Density_Output, "showpage\n") ;
}

void EPS_XT_Trailer (void)
{
  fprintf(EPS_XT_Output, "\n") ;
  fprintf(EPS_XT_Output, "grestore\n") ;
  fprintf(EPS_XT_Output, "showpage\n") ;
}

void PrintOutput(double Img)
{
	int err=0, iloum;
	char curdir[200];
	char sref[10]="" ;


	 GETCWD(curdir,200);

	// calling zt for Mantel test
	 if (FSTATS)
	 {
		 fprintf(stdout, "Population genetics calculations ...");
		 err=system("..\\..\\zt.exe -s ./zt_genetic.dat ./zt_dist.dat 50000 > ./zt_results.txt");
		 if (err) fprintf(stdout, " *** FAILED  - Error %d ***\n", err);
		 else fprintf(stdout, " done.\n");
	 }

	// writing the GNUplot command file
    fprintf(stdout, "Editing the graphics commands ... ") ;
    GNUplot_command_file=fopen("input.gp", "w+") ;
    Write_GNUplot() ;
    fclose(GNUplot_command_file) ;
    fprintf(stdout, " done.\nProcessing the graphics ... ");
   
    // calling gnuplot
	#ifdef _MSC_VER
		if (DESTINATION==DEFAULT_DESTINATION) err=system("..\\..\\wgnuplot.exe input.gp") ;
		else 
		{
			char Gnuplot_dir[100]="";
			strcpy(Gnuplot_dir,GNUPLOT_DIR.c_str());
			strcat(Gnuplot_dir,"wgnuplot.exe input.gp");
			err=system(Gnuplot_dir);
		}
	#else
	if (DESTINATION==CUSTOM_DESTINATION) {GETCWD(curdir,200); chdir(GNUPLOT_DIR.c_str());}
		err=system("gnuplot input.gp") ;
		if (DESTINATION==CUSTOM_DESTINATION) chdir(curdir);
	#endif

    if (err) fprintf(stdout, " *** FAILED - Error %d ***", err) ; else fprintf(stdout, " done.") ;
    fprintf(stdout, "\nEditing the output ... ") ;
    
	if (HTML_OUTPUT)
	{
		// writing the HTML output
		strcat(path_global_html,curdir);
		strcat(path_global_html, "/global.ref") ;
		iloum = sprintf(sref, "%d", ref) ;
		strcat(path_global_html, sref) ;
		strcat(path_global_html, ".html") ;
		HTML_Global_Output = fopen(path_global_html, "w+") ;
		//HTML_Global_Output = fopen("test.html","w+");
		//printf("%s\n",path_global_html);
		
		strcat(path_adults_html,curdir);
		strcat(path_adults_html, "/adults.ref") ;
		iloum = sprintf(sref, "%d", ref) ;
		strcat(path_adults_html, sref) ;
		strcat(path_adults_html, ".html") ;
		HTML_Adults_Output = fopen(path_adults_html, "w+") ;

		strcat(path_allelefreq_html,curdir);
		strcat(path_allelefreq_html, "/allelefreq.ref") ;
		iloum = sprintf(sref, "%d", ref) ;
		strcat(path_allelefreq_html, sref) ;
		strcat(path_allelefreq_html, ".html") ;
		HTML_AlleleFreq_Output = fopen(path_allelefreq_html, "w+") ;


		Write_HTML(Img) ;
		fclose(HTML_Global_Output) ;
		fclose(HTML_Adults_Output) ;
		fclose(HTML_AlleleFreq_Output) ;
	
		

		#ifdef _MSC_VER
			char system_launch_browser[100]="c:\\progra~1\\intern~1\\iexplore.exe";
		#else
			char system_launch_browser[100]="firefox " ;
		#endif	

		// trying to pop up FF from here
		if (!DONT_POPUP) 
		{

			fprintf(stdout, " done.\nLaunching browser ... ") ;

			char path_html_init[400]="";
			strcpy(path_html_init,"file://");
			strcat(path_html_init,path_global_html);
			//printf("%s\n",path_html_init);
			#ifdef _MSC_VER
				//System::Diagnostics::Process::Start(system_launch_browser,path_global_html);
				System::Diagnostics::Process::Start(path_html_init);
			#else
				strcat(system_launch_browser,path_html_init);
				strcat(system_launch_browser, " &") ;
				err=system(system_launch_browser);
			#endif
		}
	    
		
		if (err) fprintf(stdout, " *** FAILED - Error %d ***", err) ; else fprintf(stdout, " done.") ;
		fprintf(stdout, "\n") ;
	}
}

void CreateDirectories()
{
		#ifdef _MSC_VER
			if (DESTINATION==DEFAULT_DESTINATION) { _mkdir("./out/"); _chdir("./out/");}
			if (DESTINATION==CUSTOM_DESTINATION) _chdir(RESULTS_DIR.c_str());
			if (DATED_OUTPUT)  {_mkdir(date); _chdir(date) ;}
			_mkdir("images") ;
			_mkdir("eps") ;
		#else
			#ifdef _SYS_STAT_H
				#ifdef _UNISTD_H
					if (DESTINATION==DEFAULT_DESTINATION)	{mkdir("./out/", 00755); chdir("./out/");}
					if (DESTINATION==CUSTOM_DESTINATION) chdir(RESULTS_DIR.c_str());
					if (DATED_OUTPUT)	{ mkdir(date, 00755); chdir(date) ;}
					mkdir("images", 00755) ;
					mkdir("eps", 00755) ;
				#else 
					fprintf(stdout, "Missing library : unistd.h\n") ;
					ErrorMessage(9) ;
				#endif
			#else
				fprintf(stdout, "Missing library : sys/stat.h\n") ;
				ErrorMessage(9) ;
			#endif
		#endif
}

void OpenLogFiles()
{
  char sref[10]="" ;
	int iloum;
 

  //opening of log files
  
  strcat(path_output, "Output.ref") ;
  iloum = sprintf(sref, "%d", ref) ;
  strcat(path_output, sref) ;
  strcat(path_output, ".txt") ;
  if (DETAILED_LOG) Output=fopen(path_output,"w"); //this opens the detailed log file

  if (OUTPUT_PER_CONTAINER_TYPE)
  {
	strcat(path_output_per_container_type, "Output_per_Container_Type.ref") ;
	iloum = sprintf(sref, "%d", ref) ;
	strcat(path_output_per_container_type, sref) ;
	strcat(path_output_per_container_type, ".txt") ;
	Output_per_Container_Type=fopen(path_output_per_container_type,"w"); // this opens the output by container type
  }

  if (OUTPUT_WATER_LEVEL)
  {
	strcat(path_output_water_level, "Output_Water_Level.ref") ;
	iloum = sprintf(sref, "%d", ref) ;
	strcat(path_output_water_level, sref) ;
	strcat(path_output_water_level, ".txt") ;
	Output_Water_Level=fopen(path_output_water_level,"w"); // this opens the output for container average water level
  }

  if (LOG_PER_HOUSE) 
    {
     strcat(path_output_per_house, "Output_per_House.ref") ;
      iloum = sprintf(sref, "%d", ref) ;
      strcat(path_output_per_house, sref) ;
      strcat(path_output_per_house, ".txt") ;
      Output_per_House=fopen(path_output_per_house,"w"); 
      //this opens log file per house      
     }

  strcat(path_short_output, "Short_Output.ref") ;
  iloum = sprintf(sref, "%d", ref) ;
  strcat(path_short_output, sref) ;
  strcat(path_short_output, ".txt") ;

  Short_Output=fopen(path_short_output,"w+"); //this opens the short log file

  if (FSTATS)
  {
	strcat(path_fstbydist, "fstbydist.ref") ;
	iloum = sprintf(sref, "%d", ref) ;
	strcat(path_fstbydist, sref) ;
	strcat(path_fstbydist, ".txt") ;

	fstbydist=fopen(path_fstbydist,"w+");
  }

}

void SaveHouseSetupOut()
{
	// saves the final state of the population into a SETUP file for future simulations
	//
	// note that some information is not carried over
	// e.g. mated status of females, and their mate's genotype
	//
	// passing those would require a change in the SETUP file structure

	fprintf(stdout, "Creating final state SETUP file ... ");

	FILE *HouseSetupOut;
	char path_setup_output[100]="";
	char sref[10]="";
	int iloum;
	vector<Building>::iterator itHouse;

	strcat(path_setup_output, "SkeeterBuster.final.ref") ;
	iloum = sprintf(sref, "%d", ref) ;
	strcat(path_setup_output, sref) ;
	strcat(path_setup_output, ".setup") ;
	
	HouseSetupOut=fopen(path_setup_output,"w+");
	fprintf(HouseSetupOut,"# SKEETER BUSTER house setup file\n");
	fprintf(HouseSetupOut,"\n\n##\n");
	fprintf(HouseSetupOut,"Initialized by simulation %d on %s\n",ref,date);
	fprintf(HouseSetupOut,"##\n\n");
	fprintf(HouseSetupOut,"NUMBER_OF_HOUSES = %d\n",NUMBER_OF_HOUSES);
	fprintf(HouseSetupOut,"NUMBER_OF_COLUMNS = %d\n",NUMBER_OF_COLUMNS);
	fprintf(HouseSetupOut,"NUMBER_OF_ROWS = %d\n",NUMBER_OF_ROWS);
	fprintf(HouseSetupOut,"\n\n\n");		
	int House_Number=0;
	for (itHouse=House.begin();itHouse!=House.end();itHouse++)
	{
		fprintf(HouseSetupOut,"# HOUSE %d\n",House_Number);
		fprintf(HouseSetupOut,"\n\n\n");
		fprintf(HouseSetupOut,"HOUSE_NUMBER = %d\n\n",House_Number);
		fprintf(HouseSetupOut,"NUMBER_OF_CONTAINERS = %d\n",itHouse->Number_of_Containers);
		fprintf(HouseSetupOut,"\n\n\n");
		vector<Receptacle>::iterator itContainer;
		int Container_Number=0;
		for (itContainer=itHouse->Container.begin();itContainer!=itHouse->Container.end();itContainer++)
		{
			fprintf(HouseSetupOut,"# CONTAINER %d\n",Container_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"CONTAINER_NUMBER = %d\n\n",Container_Number);
			fprintf(HouseSetupOut,"CONTAINER_TYPE = %d\n\n",itContainer->Container_Type_Number);
			fprintf(HouseSetupOut,"RELEASE_DATE = %d\n",itContainer->Release_Date);
			fprintf(HouseSetupOut,"\n\n");
			fprintf(HouseSetupOut,"HEIGHT = %lf\n\n",itContainer->Height);
			fprintf(HouseSetupOut,"SURFACE = %lf\n\n",itContainer->Surface);
			fprintf(HouseSetupOut,"COVERED = %d\n\n",itContainer->Covered);
			fprintf(HouseSetupOut,"MONTHLY_TURNOVER = %lf\n\n",itContainer->Monthly_Turnover_Rate);
			fprintf(HouseSetupOut,"SUNEXPOSURE = %lf\n\n",itContainer->SunExposure);
			fprintf(HouseSetupOut,"DAILY_FOOD_LOSS = %lf\n\n",itContainer->Daily_Food_Loss);
			fprintf(HouseSetupOut,"DAILY_FOOD_GAIN = %lf\n\n",itContainer->Daily_Food_Gain);
			fprintf(HouseSetupOut,"COVER_REDUCTION = %lf\n\n",itContainer->Cover_Reduction);
			fprintf(HouseSetupOut,"FILLING_METHOD = %d\n\n",itContainer->Filling_Method);
			fprintf(HouseSetupOut,"DRAW_DOWN = %lf\n\n",itContainer->Draw_Down);
			fprintf(HouseSetupOut,"WATERSHED_RATIO = %lf\n\n",itContainer->Watershed_Ratio);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"INITIAL_WATER_LEVEL = %lf\n\n",itContainer->Water_Level);
			fprintf(HouseSetupOut,"INITIAL_LARVAL_FOOD = %lf\n\n",itContainer->Food);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_EGG_COHORTS = %d\n\n",itContainer->Eggs.size());		
			fprintf(HouseSetupOut,"\n\n\n");
			list<Eggs_Cohort>::iterator itEggs;
			int EggCohort_Number=0;
			for (itEggs=itContainer->Eggs.begin();itEggs!=itContainer->Eggs.end();itEggs++)
			{
				fprintf(HouseSetupOut,"# EGG COHORT %d\n",EggCohort_Number);
				fprintf(HouseSetupOut,"\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_EGGS = %lf\n\n",itEggs->Number);
				fprintf(HouseSetupOut,"LEVEL_LAID = %lf\n\n",itEggs->Level_laid_in_Container);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",itEggs->Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %d\n\n",itEggs->Age);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",itEggs->Mature);
				fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",itEggs->Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",itEggs->Wolbachia);
				fprintf(HouseSetupOut,"\n\n");
				fprintf(HouseSetupOut,"# end of EGG COHORT %d\n",EggCohort_Number);
				fprintf(HouseSetupOut,"\n\n");
				EggCohort_Number++;
			}
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_LARVAL_COHORTS = %d\n",itContainer->Larvae.size());
			fprintf(HouseSetupOut,"\n\n\n");
			list<Larvae_Cohort>::iterator itLarvae;
			int LarvaeCohort_Number=0;
			for (itLarvae=itContainer->Larvae.begin();itLarvae!=itContainer->Larvae.end();itLarvae++)
			{
				fprintf(HouseSetupOut,"# LARVAE COHORT %d\n",LarvaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_LARVAE = %lf\n\n",itLarvae->Number);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",itLarvae->Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %d\n\n",itLarvae->Age);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",itLarvae->Mature);
				fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
				fprintf(HouseSetupOut,"AVERAGE_LARVAL_WEIGHT = %lf\n\n",itLarvae->Average_Larval_Weight);
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",itLarvae->Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",itLarvae->Wolbachia);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"# end of LARVAE COHORT %d\n",LarvaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				LarvaeCohort_Number++;
			}
			fprintf(HouseSetupOut,"NUMBER_OF_INITIAL_PUPAL_COHORTS = %d\n",itContainer->Pupae.size());
			fprintf(HouseSetupOut,"\n\n\n");
			list<Pupae_Cohort>::iterator itPupae;
			int PupaeCohort_Number=0;
			for (itPupae=itContainer->Pupae.begin();itPupae!=itContainer->Pupae.end();itPupae++)
			{
				fprintf(HouseSetupOut,"# PUPAE COHORT %d\n",PupaeCohort_Number);
				fprintf(HouseSetupOut,"\n\n\n");
				fprintf(HouseSetupOut,"NUMBER_OF_PUPAE = %lf\n\n",itPupae->Number);
				fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",itPupae->Physiological_Development_Percent);
				fprintf(HouseSetupOut,"AGE = %d\n\n",itPupae->Age);
				fprintf(HouseSetupOut,"AVERAGE_PUPAL_WEIGHT = %lf\n\n",itPupae->Average_Pupal_Weight);
				fprintf(HouseSetupOut,"AVERAGE_LIPID_RESERVE = %lf\n\n",itPupae->Average_Pupal_Lipid_Reserve);
				fprintf(HouseSetupOut,"MATURE = %d\n\n",itPupae->Mature);
				fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",itPupae->Genotype);
				fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",itPupae->Wolbachia);
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
		fprintf(HouseSetupOut,"NUMBER_OF_FEMALE_ADULT_COHORTS = %d\n",itHouse->Female_Adults.size());
		fprintf(HouseSetupOut,"\n\n\n");
		list<Female_Adult>::iterator itFemale_Adult;
		int FemaleAdult_Number=0;
		for (itFemale_Adult=itHouse->Female_Adults.begin();itFemale_Adult!=itHouse->Female_Adults.end();itFemale_Adult++)
		{
			fprintf(HouseSetupOut,"# FEMALE ADULT %d\n",FemaleAdult_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_FEMALE_ADULTS = 1\n\n");
			fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",itFemale_Adult->Physiological_Development_Percent);
			fprintf(HouseSetupOut,"AGE = %d\n\n",itFemale_Adult->Age);
			fprintf(HouseSetupOut,"AVERAGE_ADULT_WEIGHT = %lf\n\n",itFemale_Adult->Average_Adult_Weight);
			fprintf(HouseSetupOut,"MATURE = %d\n\n",itFemale_Adult->Mature);
			fprintf(HouseSetupOut,"NULLIPAROUS = %d\n\n",itFemale_Adult->Nulliparous);
			fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",itFemale_Adult->Genotype);
			fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",itFemale_Adult->Wolbachia);
			fprintf(HouseSetupOut,"RELEASE_DATE = 0\n\n");
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"# end of FEMALE ADULT COHORT %d\n",FemaleAdult_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			FemaleAdult_Number++;
		}
		fprintf(HouseSetupOut,"NUMBER_OF_MALE_ADULT_COHORTS = %d\n",itHouse->Male_Adult.size());
		fprintf(HouseSetupOut,"\n\n\n");
		list<Male_Adult_Cohort>::iterator itMale_Adult;
		int MaleAdultCohort_Number=0;
		for (itMale_Adult=itHouse->Male_Adult.begin();itMale_Adult!=itHouse->Male_Adult.end();itMale_Adult++)
		{
			fprintf(HouseSetupOut,"# MALE ADULT COHORT %d\n",MaleAdultCohort_Number);
			fprintf(HouseSetupOut,"\n\n\n");
			fprintf(HouseSetupOut,"NUMBER_OF_MALE_ADULTS = %lf\n\n",itMale_Adult->Number);
			fprintf(HouseSetupOut,"PHYS_DEV = %lf\n\n",itMale_Adult->Physiological_Development_Percent);
			fprintf(HouseSetupOut,"AGE = %d\n\n",itMale_Adult->Age);
			fprintf(HouseSetupOut,"AVERAGE_ADULT_WEIGHT = %lf\n\n",itMale_Adult->Average_Adult_Weight);
			fprintf(HouseSetupOut,"MATURE = %d\n\n",itMale_Adult->Mature);
			fprintf(HouseSetupOut,"GENOTYPE = %d\n\n",itMale_Adult->Genotype);
			fprintf(HouseSetupOut,"WOLBACHIA = %d\n\n",itMale_Adult->Wolbachia);
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

void CompressOutputs (void)
{

	fprintf(stdout, "Compressing outputs ... ");

	char path_archive[200]="out.ref";
	char sref[10]="";
	int iloum;
	iloum = sprintf(sref, "%d", ref) ;
	strcat(path_archive,sref);
	strcat(path_archive,".zip");

	char zip_cmd[200]="zip -m -q -9 ";
	// define archive name
	strcat(zip_cmd,"output.files.ref");
	strcat(zip_cmd,sref);
	strcat(zip_cmd,".zip");
	// include text files
	strcat(zip_cmd," *.ref");
	strcat(zip_cmd,sref);
	strcat(zip_cmd,".txt");
	// include saved setup file
	if (SAVE_FINAL_STATE_AS_SETUP_FILE)
	{
		strcat(zip_cmd," *.ref");
		strcat(zip_cmd,sref);
		strcat(zip_cmd,".setup");
	}

	int err=system(zip_cmd);
	if (err) ErrorMessage(909);
	else fprintf(stdout, "done.\n");

}
