/* -----------------------------------------------------------------------
   SIMPLE 1D KALMAN FILTER
   Joel Matthys
   joel at matthys music daht cahm
*/

#define MAX_ITERATIONS 100
#define DEFAULT_ITERATIONS 30
#define DEFAULT_INIT_VALUE 0
#define DEFAULT_NOISE_COVARIANCE 0.5

#include <stdlib.h>
#include <math.h>
#include "m_pd.h"

typedef struct kalman
{
  t_object x_obj;
  t_outlet *filter_out, *accuracy_out;
  t_float noise_covariance;
  t_float init_value;
  t_int iterations;
  t_float *history;
  t_int index;
  t_int toggle_analyze;
  // analysis variables
  t_int count;
  double sum, sumsquares, sumvariance;
  t_float previous;
  double analyze_mean, analyze_sd, analyze_variance;
} t_kalman;

// prototype for method to determine mean and standard deviation of input
void analyze (t_kalman *x, t_float f);

void kalman_float(t_kalman *x, t_floatarg f)
{
  // analyze input for mean and standard deviation
  if ((int)x->toggle_analyze != 0) analyze (x,f);

  // perform kalman filtering
  x->history[x->index] = f; // store new value
  x->index = (x->index + 1) % x->iterations; // increment index

  // the calculations for the kalman filtering
  double Pk = 1;
  double xk = (double)x->init_value;
  int i;

  for (i = 0; i < x->iterations; i++)
    {
      double Zk = (double)x->history [(i + x->index) % x->iterations];
      double Kk = Pk / (Pk + (double)x->noise_covariance);
      xk = xk + Kk * (Zk - xk);
      Pk = (1 - Kk) * Pk;
    }

  outlet_float (x->filter_out, xk);
  outlet_float (x->accuracy_out, xk);
  x=NULL; /* don't warn about unused variables */
}

// internal method called by kalman_float
void analyze (t_kalman *x, t_float f)
{
  x->count++;
  x->sum += f;
  x->sumsquares += (f*f);
  x->sumvariance += fabs(f - x->previous);
  x->previous = f;
  x->analyze_mean = x->sum / x->count;
  x->analyze_variance = x-> sumvariance / x->count;
  if (x->count==1) x->analyze_sd = 1;
  else x->analyze_sd = sqrt (x->sumsquares - (x->sum * x->sum)/(x->count - 1));
}

/* this is called when kalman gets the message, "noise". */
void kalman_setnoise(t_kalman *x, t_float f)
{
  if ((float) f <= 0)
    {
      error ("kalman: noise cannot be 0 or less");
    }
  else
    {
      post("kalman: noise covariance set to %f", f);
      x->noise_covariance = f;
    }
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "init". */
void kalman_setinit(t_kalman *x, t_float f)
{
  post("kalman: initial val set to %f", f);
  x->init_value = f;
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "iterations". */
void kalman_setiterations(t_kalman *x, t_float f)
{
  short iter = (short) f;
  if (iter < 1)
    {
      iter = 1;
      error ("kalman: minimum is 1 iteration");
    }
  if (iter > MAX_ITERATIONS)
    {
      iter = MAX_ITERATIONS;
      error ("kalman: exceeded maximum of %d iterations", MAX_ITERATIONS);
    }
  post("kalman: number of iterations set to %d", iter);
  x->iterations = f;
  x=NULL; /* don't warn about unused variables */
}

void kalman_setanalyze(t_kalman *x, t_float f)
{
  if (f>0)
    {
      x->toggle_analyze = 1;
      x->count = 0;
      x->sum = 0;
      x->sumsquares = 0;
      post("kalman: analyzing input for optimal coefficients");

    }
  else
    {
      x->toggle_analyze = 0;
      x->init_value = (t_float)x->analyze_mean;
      x->noise_covariance = (t_float)x->analyze_variance;
      int i;
      for (i=0; i<MAX_ITERATIONS; i++) x->history[i] = x->init_value;
      post("kalman: analyze mode off.\nmean: %f, standard deviation: %f, noise covariance: %f", x->init_value, x-> analyze_sd, x->noise_covariance);
    }

  x=NULL; /* don't warn about unused variables */
}

/* this is a pointer to the class for "kalman", which is created in the
   "setup" routine below and used to create new ones in the "new" routine. */
t_class *kalman_class;

/* this is called when a new "kalman" object is created. */
void *kalman_new(t_symbol *s, int argc, t_atom *argv)
{
  t_kalman *x = (t_kalman *)pd_new(kalman_class);
  x->filter_out = outlet_new(&x->x_obj, &s_float);
  x->accuracy_out = outlet_new(&x->x_obj, &s_float);

  x->iterations = DEFAULT_ITERATIONS;
  x->noise_covariance = DEFAULT_NOISE_COVARIANCE;
  x->init_value = DEFAULT_INIT_VALUE;
  x->index = 0;
  x->toggle_analyze = 0;

  switch (argc)
    {
    case 3:
      x->init_value = atom_getfloat(argv+2);
    case 2:
      x->noise_covariance = atom_getfloat(argv+1);
    case 1:
      x->iterations = atom_getfloat(argv);
    }

  post("kalman filter initialized with %d iterations, %f noise covariance, and an init value of %f", x->iterations, x->noise_covariance, x->init_value);

  // initialize variables here
  x->history = malloc (MAX_ITERATIONS * sizeof(t_float));

  int i;
  for (i = 0; i< MAX_ITERATIONS; i++)
    {
      x->history[i] = x->init_value;
    }

  return (void *)x;
}

void kalman_free(t_kalman *x)
{
  // free array memory if malloc'ed
  free (x->history);
}

/* this is called once at setup time, when this code is loaded into Pd. */
void kalman_setup(void)
{
  post("kalman_setup");
kalman_class = class_new(gensym("kalman"), (t_newmethod)kalman_new, (t_method)kalman_free,
                           sizeof(t_kalman), 0, A_GIMME, 0);
  class_addmethod(kalman_class, (t_method)kalman_setnoise, gensym("noise"), A_FLOAT, 0);
  class_addmethod(kalman_class, (t_method)kalman_setiterations, gensym("iterations"), A_FLOAT, 0);
  class_addmethod(kalman_class, (t_method)kalman_setinit, gensym("init"), A_FLOAT, 0);
  class_addmethod(kalman_class, (t_method)kalman_setinit, gensym("mean"), A_FLOAT, 0);
  class_addmethod(kalman_class, (t_method)kalman_setanalyze, gensym("analyze"), A_FLOAT, 0);
  class_addfloat(kalman_class, kalman_float);
}
