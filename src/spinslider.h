///////////////////////////////////////////////////////////////////
// SpinSlider
// Utility functions for creating GTK2+ spin/slider combinations
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summer 2013
///////////////////////////////////////////////////////////////////

#ifndef SPINSLIDER_H
#define SPINSLIDER_H

///////////////////////////////////////////////////////////////////
// Includes

#include <glib.h>
#include <gtk/gtk.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _SpinSliderPersist SpinSliderPersist;

typedef struct _SpinSliderLink SpinSliderLink;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

SpinSliderPersist * NewSpinSliderPersist (void);
void DeleteSpinSliderPersist (SpinSliderPersist * psSpinSliderData);
SpinSliderLink * CreateSpinSlider (char const * szTitle, float * pfVariable, float fMin, float fMax, float fStepInc, float fPageInc, int nDigits, GtkTable * psParent, SpinSliderPersist * psSpinSliderData);
void SpinSliderUpdate (SpinSliderLink * psSpinSliderLink);
void SpinSliderSetValue (float fValue, SpinSliderLink * psSpinSliderLink);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* SPINSLIDER_H */

