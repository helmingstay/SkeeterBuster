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


extern long ra[M+1], nd;

/* The following routine is taken from the RANLIB random number generator
library, , which has been published by ACM Tran. Math. Software.   

General permission  to copy and  distribute the algorithm without fee is 
granted provided that the copies  are not made  or distributed for  
direct  commercial  advantage.
 */

#define ABS(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))



int ignbin(int n,double pp)
/*
 **********************************************************************
     int ignbin(int n,double pp)
                    GENerate BINomial random deviate
                              Function
     Generates a single random deviate from a binomial
     distribution whose number of trials is N and whose
     probability of an event in each trial is P.
                              Arguments
     n  --> The number of trials in the binomial distribution
            from which a random deviate is to be generated.
     p  --> The probability of an event in each trial of the
            binomial distribution from which a random deviate
            is to be generated.
     ignbin <-- A random deviate yielding the number of events
                from N independent trials, each of which has
                a probability of event P.
                              Method
     This is algorithm BTPE from:
         Kachitvichyanukul, V. and Schmeiser, B. W.
         Binomial Random Variate Generation.
         Communications of the ACM, 31, 2
         (February, 1988) 216.
 **********************************************************************
     SUBROUTINE BTPEC(N,PP,ISEED,JX)
     BINOMIAL RANDOM VARIATE GENERATOR
     MEAN .LT. 30 -- INVERSE CDF
       MEAN .GE. 30 -- ALGORITHM BTPE:  ACCEPTANCE-REJECTION VIA
       FOUR REGION COMPOSITION.  THE FOUR REGIONS ARE A TRIANGLE
       (SYMMETRIC IN THE CENTER), A PAIR OF PARALLELOGRAMS (ABOVE
       THE TRIANGLE), AND EXPONENTIAL LEFT AND RIGHT TAILS.
     BTPE REFERS TO BINOMIAL-TRIANGLE-PARALLELOGRAM-EXPONENTIAL.
     BTPEC REFERS TO BTPE AND "COMBINED."  THUS BTPE IS THE
       RESEARCH AND BTPEC IS THE IMPLEMENTATION OF A COMPLETE
       USABLE ALGORITHM.
     REFERENCE:  VORATAS KACHITVICHYANUKUL AND BRUCE SCHMEISER,
       "BINOMIAL RANDOM VARIATE GENERATION,"
       COMMUNICATIONS OF THE ACM, FORTHCOMING
     WRITTEN:  SEPTEMBER 1980.
       LAST REVISED:  MAY 1985, JULY 1987
     REQUIRED SUBPROGRAM:  RAND() -- A UNIFORM (0,1) RANDOM NUMBER
                           GENERATOR
     ARGUMENTS
       N : NUMBER OF BERNOULLI TRIALS            (INPUT)
       PP : PROBABILITY OF SUCCESS IN EACH TRIAL (INPUT)
       ISEED:  RANDOM NUMBER SEED                (INPUT AND OUTPUT)
       JX:  RANDOMLY GENERATED OBSERVATION       (OUTPUT)
     VARIABLES
       PSAVE: VALUE OF PP FROM THE LAST CALL TO BTPEC
       NSAVE: VALUE OF N FROM THE LAST CALL TO BTPEC
       XNP:  VALUE OF THE MEAN FROM THE LAST CALL TO BTPEC
       P: PROBABILITY USED IN THE GENERATION PHASE OF BTPEC
       FFM: TEMPORARY VARIABLE EQUAL TO XNP + P
       M:  INTEGER VALUE OF THE CURRENT MODE
       FM:  DOUBLEING POINT VALUE OF THE CURRENT MODE
       XNPQ: TEMPORARY VARIABLE USED IN SETUP AND SQUEEZING STEPS
       P1:  AREA OF THE TRIANGLE
       C:  HEIGHT OF THE PARALLELOGRAMS
       XM:  CENTER OF THE TRIANGLE
       XL:  LEFT END OF THE TRIANGLE
       XR:  RIGHT END OF THE TRIANGLE
       AL:  TEMPORARY VARIABLE
       XLL:  RATE FOR THE LEFT EXPONENTIAL TAIL
       XLR:  RATE FOR THE RIGHT EXPONENTIAL TAIL
       P2:  AREA OF THE PARALLELOGRAMS
       P3:  AREA OF THE LEFT EXPONENTIAL TAIL
       P4:  AREA OF THE RIGHT EXPONENTIAL TAIL
       U:  A U(0,P4) RANDOM VARIATE USED FIRST TO SELECT ONE OF THE
           FOUR REGIONS AND THEN CONDITIONALLY TO GENERATE A VALUE
           FROM THE REGION
       V:  A U(0,1) RANDOM NUMBER USED TO GENERATE THE RANDOM VALUE
           (REGION 1) OR TRANSFORMED INTO THE VARIATE TO ACCEPT OR
           REJECT THE CANDIDATE VALUE
       IX:  INTEGER CANDIDATE VALUE
       X:  PRELIMINARY CONTINUOUS CANDIDATE VALUE IN REGION 2 LOGIC
           AND A DOUBLEING POINT IX IN THE ACCEPT/REJECT LOGIC
       K:  ABSOLUTE VALUE OF (IX-M)
       F:  THE HEIGHT OF THE SCALED DENSITY FUNCTION USED IN THE
           ACCEPT/REJECT DECISION WHEN BOTH M AND IX ARE SMALL
           ALSO USED IN THE INVERSE TRANSFORMATION
       R: THE RATIO P/Q
       G: CONSTANT USED IN CALCULATION OF PROBABILITY
       MP:  MODE PLUS ONE, THE LOWER INDEX FOR EXPLICIT CALCULATION
            OF F WHEN IX IS GREATER THAN M
       IX1:  CANDIDATE VALUE PLUS ONE, THE LOWER INDEX FOR EXPLICIT
             CALCULATION OF F WHEN IX IS LESS THAN M
       I:  INDEX FOR EXPLICIT CALCULATION OF F FOR BTPE
       AMAXP: MAXIMUM ERROR OF THE LOGARITHM OF NORMAL BOUND
       YNORM: LOGARITHM OF NORMAL BOUND
       ALV:  NATURAL LOGARITHM OF THE ACCEPT/REJECT VARIATE V
       X1,F1,Z,W,Z2,X2,F2, AND W2 ARE TEMPORARY VARIABLES TO BE
       USED IN THE FINAL ACCEPT/REJECT TEST
       QN: PROBABILITY OF NO SUCCESS IN N TRIALS
     REMARK
       IX AND JX COULD LOGICALLY BE THE SAME VARIABLE, WHICH WOULD
       SAVE A MEMORY POSITION AND A LINE OF CODE.  HOWEVER, SOME
       COMPILERS (E.G.,CDC MNF) OPTIMIZE BETTER WHEN THE ARGUMENTS
       ARE NOT INVOLVED.
     ISEED NEEDS TO BE DOUBLE PRECISION IF THE IMSL ROUTINE
     GGUBFS IS USED TO GENERATE UNIFORM RANDOM NUMBER, OTHERWISE
     TYPE OF ISEED SHOULD BE DICTATED BY THE UNIFORM GENERATOR
 **********************************************************************
 *****DETERMINE APPROPRIATE ALGORITHM AND WHETHER SETUP IS NECESSARY
 */
{
	static double psave = -1.0;
	static int nsave = -1;
	static int ignbin_var=0,i=0,ix=0,ix1=0,k=0,m=0,mp=0,T1=0;
	static double al=0,alv=0,amaxp=0,c=0,f=0,f1=0,f2=0,ffm=0,fm=0,g=0,p=0,p1=0,p2=0,p3=0,p4=0,q=0,qn=0,r=0,u=0,v=0,w=0,w2=0,x=0,x1=0,
	x2=0,xl=0,xll=0,xlr=0,xm=0,xnp=0,xnpq=0,xr=0,ynorm=0,z=0,z2=0;

	if(pp != psave) goto S10;
	if(n != nsave) goto S20;
	if(xnp < 30.0) goto S150;
	goto S30;
	S10:
	/*
	 *****SETUP, PERFORM ONLY WHEN PARAMETERS CHANGE
	 */
	psave = pp;
	p = min(psave,1.0-psave);
	q = 1.0-p;
	S20:
	xnp = n*p;
	nsave = n;
	if(xnp < 30.0) goto S140;
	ffm = xnp+p;
	m = (int) ffm;
	fm = m;
	xnpq = xnp*q;
	p1 = (int) (2.195*sqrt(xnpq)-4.6*q)+0.5;
	xm = fm+0.5;
	xl = xm-p1;
	xr = xm+p1;
	c = 0.134+20.5/(15.3+fm);
	al = (ffm-xl)/(ffm-xl*p);
	xll = al*(1.0+0.5*al);
	al = (xr-ffm)/(xr*q);
	xlr = al*(1.0+0.5*al);
	p2 = p1*(1.0+c+c);
	p3 = p2+c/xll;
	p4 = p3+c/xlr;
	S30:
	/*
	 *****GENERATE VARIATE
	 */
	 u = ((double)RANDOM_INTEGER/(RIMAX+1.0))*p4;
	 v = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	 /*
     TRIANGULAR REGION
	  */
	 if(u > p1) goto S40;
	 ix = (int) (xm-p1*v+u);
	 goto S170;
	 S40:
	 /*
     PARALLELOGRAM REGION
	  */
	 if(u > p2) goto S50;
	 x = xl+(u-p1)/c;
	 v = v*c+1.0-ABS(xm-x)/p1;
	 if(v > 1.0 || v <= 0.0) goto S30;
	 ix = (int) x;
	 goto S70;
	 S50:
	 /*
     LEFT TAIL
	  */
	 if(u > p3) goto S60;
	 ix = (int) ( xl+log(v)/xll );
	 if(ix < 0) goto S30;
	 v *= ((u-p2)*xll);
	 goto S70;
	 S60:
	 /*
     RIGHT TAIL
	  */
	 ix = (int) ( xr-log(v)/xlr );
	 if(ix > n) goto S30;
	 v *= ((u-p3)*xlr);
	 S70:
	 /*
	  *****DETERMINE APPROPRIATE WAY TO PERFORM ACCEPT/REJECT TEST
	  */
	 k = ABS(ix-m);
	 if(k > 20 && k < xnpq/2-1) goto S130;
	 /*
     EXPLICIT EVALUATION
	  */
	 f = 1.0;
	 r = p/q;
	 g = (n+1)*r;
	 T1 = m-ix;
	 if(T1 < 0) goto S80;
	 else if(T1 == 0) goto S120;
	 else  goto S100;
	 S80:
	 mp = m+1;
	 for(i=mp; i<=ix; i++) f *= (g/i-r);
	 goto S120;
	 S100:
	 ix1 = ix+1;
	 for(i=ix1; i<=m; i++) f /= (g/i-r);
	 S120:
	 if(v <= f) goto S170;
	 goto S30;
	 S130:
	 /*
     SQUEEZING USING UPPER AND LOWER BOUNDS ON ALOG(F(X))
	  */
	 amaxp = k/xnpq*((k*(k/3.0+0.625)+0.1666666666666)/xnpq+0.5);
	 ynorm = -(k*k/(2.0*xnpq));
	 alv = log(v);
	 if(alv < ynorm-amaxp) goto S170;
	 if(alv > ynorm+amaxp) goto S30;
	 /*
     STIRLING'S FORMULA TO MACHINE ACCURACY FOR
     THE FINAL ACCEPTANCE/REJECTION TEST
	  */
	 x1 = ix+1.0;
	 f1 = fm+1.0;
	 z = n+1.0-fm;
	 w = n-ix+1.0;
	 z2 = z*z;
	 x2 = x1*x1;
	 f2 = f1*f1;
	 w2 = w*w;
	 if(alv <= xm*log(f1/x1)+(n-m+0.5)*log(z/w)+(ix-m)*log(w*p/(x1*q))+(13860.0-
			 (462.0-(132.0-(99.0-140.0/f2)/f2)/f2)/f2)/f1/166320.0+(13860.0-(462.0-
					 (132.0-(99.0-140.0/z2)/z2)/z2)/z2)/z/166320.0+(13860.0-(462.0-(132.0-
							 (99.0-140.0/x2)/x2)/x2)/x2)/x1/166320.0+(13860.0-(462.0-(132.0-(99.0
									 -140.0/w2)/w2)/w2)/w2)/w/166320.0) goto S170;
	 goto S30;
	 S140:
	 /*
     INVERSE CDF LOGIC FOR MEAN LESS THAN 30
	  */
	 qn = pow(q,(double)n);
	 r = p/q;
	 g = r*(n+1);
	 S150:
	 ix = 0;
	 f = qn;
	 u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	 S160:
	 if(u < f) goto S170;
	 if(ix > 110) goto S150;
	 u -= f;
	 ix += 1;
	 f *= (g/ix-r);
	 goto S160;
	 S170:
	 if(psave > 0.5) ix = n-ix;
	 ignbin_var = ix;
	 if(ignbin_var < 0)
	 {
		 fprintf(stdout, "*** ignbin_var = %d ***\n",ignbin_var);
	 }
	 return ignbin_var;
}


void genmul(int n,double *p,int ncat,int *ix)
/*
 **********************************************************************

     void genmul(int n,double *p,int ncat,int *ix)
     GENerate an observation from the MULtinomial distribution
                              Arguments
     N --> Number of events that will be classified into one of
           the categories 1..NCAT
     P --> Vector of probabilities.  P(i) is the probability that
           an event will be classified into category i.  Thus, P(i)
           must be [0,1]. Only the first NCAT-1 P(i) must be defined
           since P(NCAT) is 1.0 minus the sum of the first
           NCAT-1 P(i).
     NCAT --> Number of categories.  Length of P and IX.
     IX <-- Observation from multinomial distribution.  All IX(i)
            will be nonnegative and their sum will be N.
                              Method
     Algorithm from page 559 of

     Devroye, Luc

     Non-Uniform Random Variate Generation.  Springer-Verlag,
     New York, 1986.

 **********************************************************************
 */
{
	static double prob,ptot,sum;
	static int i,icat,ntot;
	//    if(n < 0) ftnstop("N < 0 in GENMUL");
	//    if(ncat <= 1) ftnstop("NCAT <= 1 in GENMUL");
	// ptot = 0.0;
	//for(i=0; i<ncat-1; i++) {
	//        if(*(p+i) < 0.0F) ftnstop("Some P(i) < 0 in GENMUL");
	//  if(*(p+i) > 1.0F) ftnstop("Some P(i) > 1 in GENMUL");
	//      ptot += *(p+i);
	//  }
	// if(ptot > 0.99999F) ftnstop("Sum of P(i) > 1 in GENMUL");
	/*
     Initialize variables
	 */
	ntot = n;
	sum = 1.0;
	for(i=0; i<ncat; i++) ix[i] = 0;
	/*
     Generate the observation
	 */
	for(icat=0; icat<ncat-1; icat++) {
		prob = *(p+icat)/sum;
		*(ix+icat) = ignbin(ntot,prob);
		ntot -= *(ix+icat);
		if(ntot <= 0) return;
		sum -= *(p+icat);
	}
	*(ix+ncat-1) = ntot;
	/*
     Finished
	 */
	return;
}

double snorm(void)
/*
 **********************************************************************


     (STANDARD-)  N O R M A L  DISTRIBUTION                           


 **********************************************************************
 **********************************************************************

     FOR DETAILS SEE:                                                 

               AHRENS, J.H. AND DIETER, U.                            
               EXTENSIONS OF FORSYTHE'S METHOD FOR RANDOM             
               SAMPLING FROM THE NORMAL DISTRIBUTION.                 
               MATH. COMPUT., 27,124 (OCT. 1973), 927 - 937.          

     ALL STATEMENT NUMBERS CORRESPOND TO THE STEPS OF ALGORITHM 'FL'  
     (M=5) IN THE ABOVE PAPER     (SLIGHTLY MODIFIED IMPLEMENTATION)  

     Modified by Barry W. Brown, Feb 3, 1988 to use RANF instead of   
     SUNIF.  The argument IR thus goes away.                          

 **********************************************************************
     THE DEFINITIONS OF THE CONSTANTS A(K), D(K), T(K) AND
     H(K) ARE ACCORDING TO THE ABOVEMENTIONED ARTICLE
 */
{
	static double a[32] = {
			0.0,3.917609E-2,7.841241E-2,0.11777,0.1573107,0.1970991,0.2372021,0.2776904,
			0.3186394,0.36013,0.4022501,0.4450965,0.4887764,0.5334097,0.5791322,
			0.626099,0.6744898,0.7245144,0.7764218,0.8305109,0.8871466,0.9467818,
			1.00999,1.077516,1.150349,1.229859,1.318011,1.417797,1.534121,1.67594,
			1.862732,2.153875
	};
	static double d[31] = {
			0.0,0.0,0.0,0.0,0.0,0.2636843,0.2425085,0.2255674,0.2116342,0.1999243,
			0.1899108,0.1812252,0.1736014,0.1668419,0.1607967,0.1553497,0.1504094,
			0.1459026,0.14177,0.1379632,0.1344418,0.1311722,0.128126,0.1252791,
			0.1226109,0.1201036,0.1177417,0.1155119,0.1134023,0.1114027,0.1095039
	};
	static double t[31] = {
			7.673828E-4,2.30687E-3,3.860618E-3,5.438454E-3,7.0507E-3,8.708396E-3,
			1.042357E-2,1.220953E-2,1.408125E-2,1.605579E-2,1.81529E-2,2.039573E-2,
			2.281177E-2,2.543407E-2,2.830296E-2,3.146822E-2,3.499233E-2,3.895483E-2,
			4.345878E-2,4.864035E-2,5.468334E-2,6.184222E-2,7.047983E-2,8.113195E-2,
			9.462444E-2,0.1123001,0.136498,0.1716886,0.2276241,0.330498,0.5847031
	};
	static double h[31] = {
			3.920617E-2,3.932705E-2,3.951E-2,3.975703E-2,4.007093E-2,4.045533E-2,
			4.091481E-2,4.145507E-2,4.208311E-2,4.280748E-2,4.363863E-2,4.458932E-2,
			4.567523E-2,4.691571E-2,4.833487E-2,4.996298E-2,5.183859E-2,5.401138E-2,
			5.654656E-2,5.95313E-2,6.308489E-2,6.737503E-2,7.264544E-2,7.926471E-2,
			8.781922E-2,9.930398E-2,0.11556,0.1404344,0.1836142,0.2790016,0.7010474
	};
	static long i;
	static double snorm,u,s,ustar,aa,w,y,tt;
	u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	s = 0.0;
	if(u > 0.5) s = 1.0;
	u += (u-s);
	u = 32.0*u;
	i = (long) (u);
	if(i == 32) i = 31;
	if(i == 0) goto S100;
	/*
                                START CENTER
	 */
	ustar = u-(double)i;
	aa = *(a+i-1);
	S40:
	if(ustar <= *(t+i-1)) goto S60;
	w = (ustar-*(t+i-1))**(h+i-1);
	S50:
	/*
                                EXIT   (BOTH CASES)
	 */
	y = aa+w;
	snorm = y;
	if(s == 1.0) snorm = -y;
	return snorm;
	S60:
	/*
                                CENTER CONTINUED
	 */
	u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	w = u*(*(a+i)-aa);
	tt = (0.5*w+aa)*w;
	goto S80;
	S70:
	tt = u;
	ustar = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	S80:
	if(ustar > tt) goto S50;
	u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	if(ustar >= u) goto S70;
	ustar = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	goto S40;
	S100:
	/*
                                START TAIL
	 */
	i = 6;
	aa = *(a+31);
	goto S120;
	S110:
	aa += *(d+i-1);
	i += 1;
	S120:
	u += u;
	if(u < 1.0) goto S110;
	u -= 1.0;
	S140:
	w = u**(d+i-1);
	tt = (0.5*w+aa)*w;
	goto S160;
	S150:
	tt = u;
	S160:
	ustar = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	if(ustar > tt) goto S50;
	u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	if(ustar >= u) goto S150;
	u = ((double)RANDOM_INTEGER/(RIMAX+1.0));
	goto S140;
}




// temporary:

double Binomial_Deviate(double n, double pp)
{
	try
	{
		if (!CheckProba(pp))
			throw 101;
		return (double)ignbin( (int) n, pp);
		//return n*pp;
	}
	catch (int n)
	{
		ErrorMessage(n);
	}
}

/* GSL - The poisson distribution has the form

   p(n) = (mu^n / n!) exp(-mu)

   for n = 0, 1, 2, ... . The method used here is the one from Knuth. */



void Multinomial_Deviate(double n, double *p, int ncat, double *ix)
{
	for (int i=0;i<ncat;i++)
		*(ix+i)=n* *(p+i);
}

double Normal_Deviate(double av, double sd)
{
	double Normal_value=sd*snorm()+av;
	return(Normal_value);
}

double Random_Number()
{
	return ((double)RANDOM_INTEGER/(RIMAX+1.0));
}


/* Also from GSL */

double gamma_large(const double a)
{
  /* Works only if a > 1, and is most efficient if a is large

     This algorithm, reported in Knuth, is attributed to Ahrens.  A
     faster one, we are told, can be found in: J. H. Ahrens and
     U. Dieter, Computing 12 (1974) 223-246.  */

  double sqa, x, y, v;
  sqa = sqrt (2 * a - 1);
  do
    {
      do
        {
          y = tan (3.14159265 * Random_Number());
          x = sqa * y + a - 1;
        }
      while (x <= 0);
      v = Random_Number();
    }
  while (v > (1 + y * y) * exp ((a - 1) * log (x / (a - 1)) - sqa * y));

  return x;
}


double gen_gamma(const unsigned int a)
{
  if (a < 12)
    {
      unsigned int i;
      double prod = 1;
      double random_value;

      for (i = 0; i < a; i++)
        {
    	  random_value = Random_Number();
    	  if (random_value==1)
			  {
    		  random_value = 1 - pow(2.0,-52); // if the random value is precisely equal to one, subtract machine epsilon
			  }
    	  if (random_value==0)
    	  	  {
    	      random_value = 1 + pow(2.0,-52); // if the random value is precisely equal to zero, add machine epsilon
    	  	  }
          prod *= random_value;
        }

      /* Note: for 12 iterations we are safe against underflow, since
         the smallest positive random number is O(2^-32). This means
         the smallest possible product is 2^(-12*32) = 10^-116 which
         is within the range of double precision. */

      return -log (prod);
    }
  else
    {
      return gamma_large((double) a);
    }
}


int gen_poisson (double mu)
{
  double emu;
  double prod = 1.0;
  unsigned int k = 0;

  while (mu > 10)
    {
      unsigned int m = mu * (7.0 / 8.0);

      double X = gen_gamma (m);

      if (X >= mu)
        {
          return k + Binomial_Deviate( m - 1, mu / X);
        }
      else
        {
          k += m;
          mu -= X;
        }
    }

  /* This following method works well when mu is small */

  emu = exp (-mu);

  do
    {
      prod *= Random_Number();
      k++;
    }
  while (prod > emu);

  return k - 1;

}
