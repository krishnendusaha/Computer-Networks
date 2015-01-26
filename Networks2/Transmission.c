#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <complex.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_spline.h>


#define PI 3.141592
#define ERROR   randfrom(-MAX_NOISE, MAX_NOISE)
#define MAX_AMP    50
#define MIN_AMP    5
#define MAX_FREQ   4000
#define MIN_FREQ   20
#define SAMPLING_FREQ  8000
#define F1  43008
#define F2  61568
#define MAX_NOISE  0.05
#define QUANTIZATON  0.05
#define J   _Complex_I
typedef struct {

float amplitude;
float frequency;
float phase;
}Sinusoid;

typedef struct{
Sinusoid  s[5];
}Voice_Signal;


typedef struct 
{
  float r;
  float i; 
}Complex;

float randfrom(float min,float max)
{
	float range = max-min;
	float div = RAND_MAX/range;
	return min+(rand()/div);
}

float  Add_and_Sample(Voice_Signal V,float sample_time_diff,int i)
{
	int k;
	float ret_val=0;
	float t=i*sample_time_diff;
	for(k=0;k<5;k++)		
	{
      ret_val+=V.s[k].amplitude*sin(V.s[k].frequency*t+V.s[k].phase);
      
	}	
	return ret_val;
}




double complex* FFT(int size, complex double *samples) 
{ 
  complex double* f = (complex double*)malloc(size*sizeof(complex double)); 
  if(size == 1) { f[0] = samples[0]; return f; } 
  complex double *samples1, *samples2; 
  complex double *E, *D;

  samples1= (complex double*)malloc((size-size/2)*sizeof(complex double));
  samples2= (complex double*)malloc((size/2)*sizeof(complex double)); 
  
  int i; 

  for(i = 0; i < size; i++) 
  { 
    if(i%2 == 0) samples1[i/2] = samples[i]; 
    else samples2[i/2] = samples[i]; 
  } 
  
  E = (complex double*)malloc((size-size/2)*sizeof(complex double));
  D = (complex double*)malloc((size/2)*sizeof(complex double)); 
  
  E = FFT(size-size/2, samples1); 
  D = FFT(size/2, samples2);
double complex omega_i ; 
  for(i = 0; i < size-size/2; i++)
  { 
    omega_i = cos((double)2*PI*i/size) + sin((double)2*PI*i/size)*_Complex_I;
    f[i] = E[i] + omega_i*D[i];
    if(i + size-size/2 >= size) break;
    f[i + size-size/2] = E[i] - omega_i*D[i]; 
  } 
    return f; 
}



double complex* IFFT(int size, complex double *samples) 
{ 
  int i;
 
  complex double* f = (complex double*)malloc(size*sizeof(complex double)); 
  if(size == 1) { f[0] = samples[0]; return f; } 
  complex double *samples1, *samples2; 
  complex double *E, *D;

  samples1= (complex double*)malloc((size-size/2)*sizeof(complex double));
  samples2= (complex double*)malloc((size/2)*sizeof(complex double)); 
  

  for(i = 0; i < size; i++) 
  { 
    if(i%2 == 0) samples1[i/2] = samples[i]; 
    else samples2[i/2] = samples[i]; 
  } 
  
  E = (complex double*)malloc((size-size/2)*sizeof(complex double));
  D = (complex double*)malloc((size/2)*sizeof(complex double)); 
  
  E =IFFT(size-size/2, samples1); 
  D =IFFT(size/2, samples2); 
  double complex omega_i;
  for(i = 0; i < size-size/2; i++)
  { 
    omega_i = cos((double)2*PI*i/size) - sin((double)2*PI*i/size)*_Complex_I; 
    f[i] =( E[i] + omega_i*D[i]);
    if(i + size-size/2 >= size) break;
    f[i + size-size/2] =(E[i] - omega_i*D[i]); 
  } 
    return f; 
}





int main(int argc, char const *argv[])
{
   int i,j;
   Voice_Signal  V[2];

   FILE* F = fopen("T.dat","w"); 
   FILE* G = fopen("R.dat","w");  
  
  //1:Make Two Voice Signals with 5 sinusoids
   for(j=0;j<2;j++)
   {
     for(i=0;i<5;i++)
     {
      V[j].s[i].amplitude=randfrom(MIN_AMP,MAX_AMP);
      V[j].s[i].frequency=randfrom(MIN_FREQ,MAX_FREQ);
      V[j].s[i].phase=randfrom(0.0,2.0*PI);
     
     }
   }
  
  //2: Sampling the Signals
   complex double*  samples1=(complex double*)malloc(sizeof(complex double)*4096);
   complex double*  samples2=(complex double*)malloc(sizeof(complex double)*4096);
   float sample_time_diff=1/(float)SAMPLING_FREQ;

   FILE * f5=fopen("f.txt","w"); 
   for(i=0;i<4096;i++)
   {
     	samples1[i]=Add_and_Sample(V[0],sample_time_diff,i)+0*_Complex_I;
      samples2[i]=Add_and_Sample(V[1],sample_time_diff,i)+0*_Complex_I;
      fprintf(f5, "%f+%fi\t %f+%fi\n",creal(samples1[i]),cimag(samples1[i]),creal(samples2[i]),cimag(samples2[i]) ); 
   }
   
   //*******************************************************************
     double xi, yi, x[4096], y[4096],y0[4096];

     fprintf (F,"#m=0,S=2\n");

     for (i = 0; i < 4096; i++)
     {
      x[i] = i*sample_time_diff;
      y[i] = creal(samples1[i]);//+creal(samples2[i]);
      fprintf (F,"%g %g\n", x[i], y[i]);
     }

     fprintf (F,"#m=1,S=0\n");

    {
       gsl_interp_accel *acc 
         = gsl_interp_accel_alloc ();
       gsl_spline *spline 
         = gsl_spline_alloc (gsl_interp_cspline, 4096);

       gsl_spline_init (spline, x, y, 4096);

    for (xi = x[0]; xi < x[4095]; xi += 0.001)
      {
        yi = gsl_spline_eval (spline, xi, acc);
        fprintf (F,"%g %g\n", xi, yi);
      }
    gsl_spline_free (spline);
    gsl_interp_accel_free (acc);
    }
     
   //*******************************************************************

   complex double *array=(complex double*)malloc(sizeof(complex double)*65536);
   complex double *y1=FFT(4096,samples1);
   complex double *y2=FFT(4096,samples2); 
   

   //FILE * f3=fopen("c.txt","w");
   //for ( i = 0; i < 4096 ; ++i)
   //{
   //  fprintf(f3, "%f+%fi\t %f+%fi\n",creal(y1[i]),cimag(y1[i]),creal(y2[i]),cimag(y2[i]) );
   //}
 
    // FILE * f1=fopen("a.txt","w");
    int k1=0,k2=0;
    for(i=0;i<65536;i++)
    {
      if(i>=F1-2048 && i<F1+2048)
      {
        array[i]=y1[k1++];  //fprintf(f1,"%d :%f+%fi \n   ",i,creal(array[i]), cimag(array[i]));
      }
      else if(i>=F2-2048 && i<F2+2048)
      {
        array[i]=y2[k2++];  //fprintf(f1,"%d :%f+%fi \n   ",i,creal(array[i]), cimag(array[i]));
      }
      else
      {
         array[i]=0+0*J;    //fprintf(f1,"%d :%f+%fi \n   ",i,creal(array[i]), cimag(array[i]));
      }  
    }  
  printf("***********************************************\n");
    
    complex double *z;
    z=IFFT(65536,array);
    for(i=0;i<65536;i++)
    { 
      z[i]=creal(z[i])/65536+cimag(z[i])/65536*J;
     // printf("%f+%fi\n",creal(z[i]),cimag(z[i]));
    }
 

   //  double *transmission_vals=(double*)malloc(sizeof(double)*65536); 
  
   
   // //3. Quantizaton and Error  Addition:  
   // for(i=0;i<65536;i++)
   // { 
   //   transmission_vals[i]=creal(z[i]);
   //   transmission_vals[i]=(int)(transmission_vals[i]/QUANTIZATON)*QUANTIZATON;
   //   transmission_vals[i]+=ERROR;
   //   //printf("%f \n",transmission_vals[i] );
   // } 



   // //4:Received values at the receiver end

   // double *received_vals=(double*)malloc(sizeof(double)*65536);  
   // for(i=0;i<65536;i++)
   // { 
   //   received_vals[i]=transmission_vals[i];
   //   received_vals[i]=(int)(received_vals[i]/QUANTIZATON)*QUANTIZATON;
   //   //printf("%f \n",received_vals[i] );
  
   // }

   // free(transmission_vals);

   //  for(i=0;i<65536;i++)
   // {
   //   array[i]=received_vals[i]+0*_Complex_I;
   // }
   complex  double * z2;
   z2=FFT(65536,z);
   //FILE * f2=fopen("b.txt","w");
   
   // for(i=0;i<65536;i++)
   // {
   //     fprintf(f2,"%d :%f+%fi\n",i,creal(z2[i]),cimag(z2[i]));
   // }    


    //5: Seperation  of components:
    k1=0,k2=0;
     for(i=0;i<65536;i++)
    {
      if(i>=F1-2048 && i<F1+2048)
      {
        samples1[k1++]=z2[i];  // printf("%d :%f+%fi \n   ",i,creal(array[i]), cimag(array[i]));
      }
      else if(i>=F2-2048 && i<F2+2048)
      {
        samples2[k2++]=z2[i]; // printf("%d :%f+%fi \n   ",i,creal(array[i]), cimag(array[i]));
      }
      else
      {
           
      }  
    }  
   // FILE * f4=fopen("d.txt","w");
   // for ( i = 0; i < 4096 ; ++i)
   // {
   //   fprintf(f4, "%f+%fi\t %f+%fi\n",creal(samples1[i]),cimag(samples1[i]),creal(samples2[i]),cimag(samples2[i]) );
   // }

    y1=IFFT(4096,samples1);
    y2=IFFT(4096,samples2);

  
    for(i=0;i<4096;i++)
    {
       printf("%d :%f+%fi   %f+%fi\n   " ,i,creal(y1[i])/4096, cimag(y1[i])/4096,creal(y2[i])/4096, cimag(y2[i])/4096);
    }
    


  //*****************************************************************************
    fprintf (G,"#m=0,S=2\n");

     for (i = 0; i < 4096; i++)
     {
      x[i] = i*sample_time_diff;
      y0[i] = creal(y1[i])/4096+creal(y2[i])/4096;
      fprintf (G,"%g %g\n", x[i], y[i]);
     }

     fprintf (G,"#m=1,S=0\n");

    {
       gsl_interp_accel *acc1 
         = gsl_interp_accel_alloc ();
       gsl_spline *spline1 
         = gsl_spline_alloc (gsl_interp_cspline, 4096);

       gsl_spline_init (spline1, x, y, 4096);

    for (xi = x[0]; xi < x[4095]; xi += 0.001)
      {
        yi = gsl_spline_eval (spline1, xi, acc1);
        fprintf (G,"%g %g\n", xi, yi);
      }
    gsl_spline_free (spline1);
    gsl_interp_accel_free (acc1);
    }
   


  //*****************************************************************************
   
  double error=0;
  for (i = 0; i <4096; ++i)
    {
      error+=pow(y0[i]-y[i],2);
    }
    error=sqrt(error);

    printf("%f\n",error );

    system("graph -T ps < T.dat > T.ps");
    system("graph -T ps < R.dat > R.ps");

	return 0;
}


