#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>

#define PI 3.141592
#define ERROR   randfrom(-MAX_NOISE, MAX_NOISE)
#define MAX_NOISE  5.0
//#define SAMPLING_FREQUENCY 20

typedef struct {

double amplitude;
double frequency;
double phase;
} sinusoid;


double randfrom(double min,double max)
{
	double range = max-min;
	double div = RAND_MAX/range;
	return min+(rand()/div);
}

double quantize_signal(double y,int x)
{

  return((int)(y/x) * x);

}


int main(){

   
   srand(time(NULL));
   sinusoid first,second,third,fourth,fifth,final;
   double maxfrequency = 0.0;
   double max_amplitude =0.0;
   double sum = 0.0;
   int number_of_values = 0;
   double mean_square_error;
   double actual_value;
   int no_of_quantization_levels=0;
   int quantization_unit;

   int counter = 0;
   double errors[100];
   double sampling[100];
   int e,r,j;


   FILE* f1 = fopen("f1.txt","w");
   FILE* F = fopen("output.dat","w"); 
   FILE* G = fopen("output1.dat","w");  
   /*SET AMPLITUDES */

   first.amplitude = randfrom(5.0,50.0);
   max_amplitude+=first.amplitude;
   second.amplitude = randfrom(5.0,50.0);
   max_amplitude+=second.amplitude;
   third.amplitude = randfrom(5.0,50.0);
   max_amplitude+=third.amplitude;
   fourth.amplitude = randfrom(5.0,50.0);
   max_amplitude+=fourth.amplitude;
   fifth.amplitude = randfrom(5.0,50.0);
   max_amplitude+=fifth.amplitude;
   



   /*SET FREQUENCIES */
   
   first.frequency = randfrom(5.0,50.0);
   if(first.frequency > maxfrequency) maxfrequency = first.frequency;
   second.frequency = randfrom(5.0,50.0);
   if(second.frequency > maxfrequency) maxfrequency = second.frequency;
   third.frequency = randfrom(5.0,50.0);
   if(third.frequency > maxfrequency) maxfrequency = third.frequency;
   fourth.frequency = randfrom(5.0,50.0);
   if(fourth.frequency > maxfrequency) maxfrequency = fourth.frequency;
   fifth.frequency = randfrom(5.0,50.0);
   if(fifth.frequency > maxfrequency) maxfrequency = fifth.frequency;

   /*SET PHASES */
   maxfrequency = (double)maxfrequency/(2*PI) ;
   fprintf(f1, "Max frequency - %g\n",maxfrequency);
   first.phase =  randfrom(0.0,2.0*PI);
   second.phase =  randfrom(0.0,2.0*PI);
   third.phase =  randfrom(0.0,2.0*PI);
   fourth.phase =  randfrom(0.0,2.0*PI);
   fifth.phase =  randfrom(0.0,2.0*PI);
   
   /*amplitude based approach*/
  no_of_quantization_levels= 2*(int)sqrt(1+((max_amplitude*max_amplitude)/(MAX_NOISE*MAX_NOISE))) + 1;
  
/*POWER BASED APPROACH */
  // no_of_quantization_levels=1+(int)((max_amplitude*max_amplitude)/(MAX_NOISE * MAX_NOISE));

quantization_unit=(int)2*max_amplitude/(no_of_quantization_levels-1);
   //quantization_unit=(int)2*max_amplitude/(no_of_quantization_levels-1);
   


   fprintf(f1,"%lf %lf %lf\n",first.amplitude,first.frequency,first.phase);
   fprintf(f1,"%lf %lf %lf\n",second.amplitude,second.frequency,second.phase);
   fprintf(f1,"%lf %lf %lf\n",third.amplitude,third.frequency,third.phase);
   fprintf(f1,"%lf %lf %lf\n",fourth.amplitude,fourth.frequency,fourth.phase);
   fprintf(f1,"%lf %lf %lf\n",fifth.amplitude,fifth.frequency,fifth.phase);
   fprintf(f1, "\n\n\n" );
   int i;
  double xi, yi, x[500], y[500];

  for(j = 3; j < (int)12*maxfrequency ; j++){

 // printf ("#m=0,S=2\n");

  for (i = 0; i < 2*j; i++)
    {
      x[i] = (double)i/j;
    
      y[i] = first.amplitude * sin(first.frequency*x[i] + first.phase) +
             second.amplitude * sin(second.frequency*x[i] + second.phase) +
             third.amplitude * sin(third.frequency*x[i] + third.phase) +
             fourth.amplitude * sin(fourth.frequency*x[i] + fourth.phase) +
             fifth.amplitude * sin(fifth.frequency*x[i] + fifth.phase);

      y[i] = quantize_signal(y[i],quantization_unit);       
      y[i] =y[i]+ERROR;
    
     // printf("%g %g\n", x[i], y[i]);

      //fprintf(f1,"%g\n",ERROR );
    }

 // printf ("#m=2,S=0\n");

  
    gsl_interp_accel *acc1 
      = gsl_interp_accel_alloc ();
    gsl_spline *spline1 
      = gsl_spline_alloc (gsl_interp_cspline,2*j);

   gsl_spline_init (spline1, x, y, 2*j);

    for (xi = x[0]; xi < x[2*j-1]; xi += 0.01)
      {
        
        yi = gsl_spline_eval (spline1, xi, acc1);
        actual_value = first.amplitude * sin(first.frequency*xi + first.phase) +
             second.amplitude * sin(second.frequency*xi + second.phase) +
             third.amplitude * sin(third.frequency*xi + third.phase) +
             fourth.amplitude * sin(fourth.frequency*xi + fourth.phase) +
             fifth.amplitude * sin(fifth.frequency*xi + fifth.phase);
        sum += (yi- actual_value)*(yi - actual_value);
        number_of_values++;
        
     //   printf ("%g %g\n", xi, yi);
      }
      mean_square_error = sqrt((double)sum/number_of_values);
      errors[counter] = mean_square_error;
      sampling[counter] = j;
      counter++;
      fprintf(f1,"\nThe mean_square_error is %g for sampling frequency %d\n",mean_square_error,j);
    gsl_spline_free (spline1);
    gsl_interp_accel_free (acc1);
}


fprintf(f1, "The number of samples are %d\n",counter );

fprintf (F,"#m=0,S=2\n");

for (i = 0; i < counter; i++)
    {
      x[i] = (double)sampling[i];
    
      y[i] = errors[i];
     // quantize_signal();       
     // y[i] =y[i]+ERROR;
      fprintf(F,"%g %g\n", x[i], y[i]);

     // fprintf(f1,"%g\n",ERROR );
    }

 fprintf (F,"#m=1,S=0\n");
gsl_interp_accel *acc2
      = gsl_interp_accel_alloc ();
    gsl_spline *spline2 
      = gsl_spline_alloc (gsl_interp_cspline,counter);

   gsl_spline_init (spline2, x, y, counter);

   for (xi = x[0]; xi < x[counter-1]; xi += 0.01)
      {
        yi = gsl_spline_eval (spline2, xi, acc2);
        fprintf (F,"%g %g\n", xi, yi);
      }
    gsl_spline_free (spline2);
    gsl_interp_accel_free (acc2);


    maxfrequency = (int) maxfrequency;

fprintf (G,"#m=0,S=2\n");

  for (i = 0; i < 4*maxfrequency; i++)
    {
      x[i] = (double)i/2*maxfrequency;
    
      y[i] = first.amplitude * sin(first.frequency*x[i] + first.phase) +
             second.amplitude * sin(second.frequency*x[i] + second.phase) +
             third.amplitude * sin(third.frequency*x[i] + third.phase) +
             fourth.amplitude * sin(fourth.frequency*x[i] + fourth.phase) +
             fifth.amplitude * sin(fifth.frequency*x[i] + fifth.phase);

    //  y[i] = quantize_signal(y[i],quantization_unit);       
    //  y[i] =y[i]+ERROR;
    
     fprintf(G,"%g %g\n", x[i], y[i]);

      //fprintf(f1,"%g\n",ERROR );
    }

  fprintf (G,"#m=1,S=0\n");

  
    gsl_interp_accel *acc4
      = gsl_interp_accel_alloc ();
    gsl_spline *spline4 
      = gsl_spline_alloc (gsl_interp_cspline,4*maxfrequency);

   gsl_spline_init (spline4, x, y, 4*maxfrequency);


    for (xi = x[0]; xi < x[4*(int)maxfrequency-1]; xi += 0.01)
      {
        
        yi = gsl_spline_eval (spline4, xi, acc4);
       // actual_value = first.amplitude * sin(first.frequency*xi + first.phase) +
        //     second.amplitude * sin(second.frequency*xi + second.phase) +
        //     third.amplitude * sin(third.frequency*xi + third.phase) +
        //     fourth.amplitude * sin(fourth.frequency*xi + fourth.phase) +
        //     fifth.amplitude * sin(fifth.frequency*xi + fifth.phase);
       // sum += (yi- actual_value)*(yi - actual_value);
       // number_of_values++;
        
        fprintf (G,"%g %g\n", xi, yi);
      }
    gsl_spline_free (spline4);
    gsl_interp_accel_free (acc4);








     fprintf (G,"#m=0,S=2\n");

  for (i = 0; i < 4*maxfrequency; i++)
    {
      x[i] = (double)i/2*maxfrequency;
    
      y[i] = first.amplitude * sin(first.frequency*x[i] + first.phase) +
             second.amplitude * sin(second.frequency*x[i] + second.phase) +
             third.amplitude * sin(third.frequency*x[i] + third.phase) +
             fourth.amplitude * sin(fourth.frequency*x[i] + fourth.phase) +
             fifth.amplitude * sin(fifth.frequency*x[i] + fifth.phase);

      y[i] = quantize_signal(y[i],quantization_unit);       
      y[i] =y[i]+ERROR;
    
     fprintf(G,"%g %g\n", x[i], y[i]);

      //fprintf(f1,"%g\n",ERROR );
    }

  fprintf (G,"#m=2,S=0\n");

  
    gsl_interp_accel *acc3
      = gsl_interp_accel_alloc ();
    gsl_spline *spline3 
      = gsl_spline_alloc (gsl_interp_cspline,4*maxfrequency);

   gsl_spline_init (spline3, x, y, 4*maxfrequency);


    for (xi = x[0]; xi < x[4*(int)maxfrequency-1]; xi += 0.01)
      {
        
        yi = gsl_spline_eval (spline3, xi, acc3);
        actual_value = first.amplitude * sin(first.frequency*xi + first.phase) +
             second.amplitude * sin(second.frequency*xi + second.phase) +
             third.amplitude * sin(third.frequency*xi + third.phase) +
             fourth.amplitude * sin(fourth.frequency*xi + fourth.phase) +
             fifth.amplitude * sin(fifth.frequency*xi + fifth.phase);
        sum += (yi- actual_value)*(yi - actual_value);
        number_of_values++;
        
        fprintf (G,"%g %g\n", xi, yi);
      }
    gsl_spline_free (spline3);
    gsl_interp_accel_free (acc3);



  

  system("graph -T ps -C output.dat > MSEandSamplingFrequency.ps");
  system("graph -T ps -C output1.dat > Input0utput.ps");





   return 0;
}

