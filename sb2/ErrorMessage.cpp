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
#include<cstdio>
#include<cstdlib>

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
		fprintf(stdout, "\n*** Unable to find conf file \n");
		exit(2);

	case 3: // key in Skeeterbuster.conf not found 
		fprintf(stdout, "\n*** Key not found -- Exiting\n");
		exit(3);

	case 4: // setup file SkeeterBuster.setup not found
		fprintf(stdout, "\n*** Error: Unable to find setup file \n");
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
		fprintf(stdout, "\n*** Error: Unable to find the sampling routine file \n");
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
