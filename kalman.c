/* code for "kalman" pd class.  This takes two messages: floating-point
   numbers, and "rats", and just prints something out for each message. */

#define MAX_ITERATIONS 100
#define DEFAULT_ITERATIONS 30
#define DEFAULT_INIT_VALUE 0
#define DEFAULT_NOISE_COVARIANCE 0.5

#include <stdlib.h>
#include <math.h>
#include "m_pd.h"


/* the data structure for each copy of "kalman".  In this case we
   only need pd's obligatory header (of type t_object). */
typedef struct kalman
{
  t_object x_ob;
  t_float noise_covariance;
  t_float init_value;
  t_int iterations;
  t_float *history;
  t_int index;
  t_int toggle_analyze;
  // analysis variables
  t_int count;
  double sum, sumsquares;
  double analyze_mean, analyze_sd;
} t_kalman;

void analyze (t_kalman *x, t_float f);

/* this is called back when kalman gets a "float" message (i.e., a
   number.) */
void kalman_float(t_kalman *x, t_floatarg f)
{
  if (x->toggle_analyze == 0)
    {
      x->history[x->index] = f; // store new value
      x->index = (x->index + 1) % x->iterations; // increment index

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

      outlet_float (x->x_ob.ob_outlet, xk);
      x=NULL; /* don't warn about unused variables */
    }
  else analyze (x,f);
}

void analyze (t_kalman *x, t_float f)
{
  x->count++;
  x->sum += f;
  x->sumsquares += (f*f);
  x->analyze_mean = x->sum / x->count;
  if (x->count==1) x->analyze_sd = 1;
  else x->analyze_sd = sqrt (x->sumsquares - (x->sum * x->sum)/(x->count - 1));
}

/* this is called when kalman gets the message, "noise". */
void kalman_setnoise(t_kalman *x, t_float f)
{
  post("kalman: noise set to %f", f);
  x->noise_covariance = f;
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "init". */
void kalman_setinit(t_kalman *x, t_float f)
{
  post("kalman: init val set to %f", f);
  x->init_value = f;
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "iterations". */
void kalman_setiterations(t_kalman *x, t_float f)
{
  post("kalman: iteration val set to %f", f);
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
      x->noise_covariance = (t_float)(x->analyze_mean / x->analyze_sd);
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
  outlet_new(&x->x_ob, &s_float);

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
  class_addmethod(kalman_class, (t_method)kalman_setanalyze, gensym("analyze"), A_FLOAT, 0);
  class_addfloat(kalman_class, kalman_float);
}
