/* code for "kalman" pd class.  This takes two messages: floating-point
   numbers, and "rats", and just prints something out for each message. */

#include "m_pd.h"

/* the data structure for each copy of "kalman".  In this case we
   only need pd's obligatory header (of type t_object). */
typedef struct kalman
{
  t_object x_ob;
} t_kalman;

/* this is called back when kalman gets a "float" message (i.e., a
   number.) */
void kalman_float(t_kalman *x, t_floatarg f)
{
  post("kalman: %f", f);
  outlet_float (x->x_ob.ob_outlet, f);
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "noise". */
void kalman_setnoise(t_kalman *x, t_float f)
{
  post("kalman: noise set to %f", f);
  x=NULL; /* don't warn about unused variables */
}

/* this is called when kalman gets the message, "init". */
void kalman_setinit(t_kalman *x, t_float f)
{
  post("kalman: init val set to %f", f);
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
  post("kalman_new");
  // initialize variables here
  return (void *)x;
}

void kalman_free(t_kalman *x)
{
  // free array memory if malloc'ed
}

/* this is called once at setup time, when this code is loaded into Pd. */
void kalman_setup(void)
{
  post("kalman_setup");
  kalman_class = class_new(gensym("kalman"), (t_newmethod)kalman_new, kalman_free,
                           sizeof(t_kalman), 0, A_GIMME, 0);
  class_addmethod(kalman_class, (t_method)kalman_setnoise, gensym("noise"), A_FLOAT, 0);
  class_addmethod(kalman_class, (t_method)kalman_setinit, gensym("init"), A_FLOAT, 0);
  class_addfloat(kalman_class, kalman_float);
}
