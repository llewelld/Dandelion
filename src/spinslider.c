///////////////////////////////////////////////////////////////////
// SpinSlider
// Utility functions for creating GTK2+ spin/slider combinations
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summer 2013
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "spinslider.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _SpinSliderPersist {
	GPtrArray * apsSpinSliderLinks;
};

struct _SpinSliderLink {
	float * pfVariable;
	GtkSpinButton * psSpin;
	GtkScale * psSlider;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

static gboolean SliderChanged (GtkWidget * psWidget, gpointer psData);
static gboolean SpinChanged (GtkWidget * psWidget, gpointer psData);

///////////////////////////////////////////////////////////////////
// Function definitions

SpinSliderPersist * NewSpinSliderPersist (void) {
	SpinSliderPersist * psSpinSliderData;

	psSpinSliderData = g_new0 (SpinSliderPersist, 1);

	psSpinSliderData->apsSpinSliderLinks = g_ptr_array_new ();
	g_ptr_array_set_free_func (psSpinSliderData->apsSpinSliderLinks, g_free);

	return psSpinSliderData;
}

void DeleteSpinSliderPersist (SpinSliderPersist * psSpinSliderData) {
	if (psSpinSliderData) {
	
		if (psSpinSliderData->apsSpinSliderLinks) {
			g_ptr_array_free (psSpinSliderData->apsSpinSliderLinks, TRUE);
			psSpinSliderData->apsSpinSliderLinks = NULL;
		}

		g_free (psSpinSliderData);
	}
}

SpinSliderLink * CreateSpinSlider (char const * szTitle, float * pfVariable, float fMin, float fMax, float fStepInc, float fPageInc, int nDigits, GtkTable * psParent, SpinSliderPersist * psSpinSliderData) {
	guint uRows;
	guint uColumns;
	GtkWidget * psLabel;
	GtkWidget * psSpin;
	GtkWidget * psSlider;
	SpinSliderLink * psSpinSliderLink;

	// Add an extra row to the table
	gtk_table_get_size (psParent, & uRows, & uColumns);
	
	if (uColumns < 3) {
		uColumns = 3;
	}
	
	uRows++;
	gtk_table_resize  (psParent, uRows, uColumns);

	// Create the label widget
	psLabel = gtk_label_new (szTitle);
	
	// Create the spin button widget
	psSpin = gtk_spin_button_new_with_range (fMin, fMax, fStepInc);
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON (psSpin), nDigits);
	gtk_spin_button_set_increments (GTK_SPIN_BUTTON (psSpin), fStepInc, fPageInc);
	
	// Create the horizontal slider widget
	psSlider = gtk_hscale_new_with_range (fMin, fMax, fStepInc);
	gtk_scale_set_digits (GTK_SCALE (psSlider), nDigits);
	gtk_scale_set_draw_value (GTK_SCALE (psSlider), FALSE);

	if (pfVariable) {
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (psSpin), * pfVariable);
	  gtk_range_set_value (GTK_RANGE (psSlider), * pfVariable);
	}

	// Add the widgets to the table
	gtk_table_attach (psParent, psLabel, 0, 1, uRows - 1, uRows, (GTK_FILL), (GTK_EXPAND | GTK_FILL), 0, 0);
	gtk_table_attach (psParent, psSpin, 1, 2, uRows - 1, uRows, (GTK_FILL), (GTK_EXPAND | GTK_FILL), 0, 2);
	gtk_table_attach (psParent, psSlider, 2, 3, uRows - 1, uRows, (GTK_EXPAND | GTK_FILL), (GTK_EXPAND | GTK_FILL), 0, 0);

  gtk_widget_show (psLabel);
  gtk_widget_show (psSpin);
  gtk_widget_show (psSlider);

	psSpinSliderLink = g_new (SpinSliderLink, 1);
	psSpinSliderLink->psSpin = GTK_SPIN_BUTTON (psSpin);
	psSpinSliderLink->psSlider = GTK_SCALE (psSlider);
	psSpinSliderLink->pfVariable = pfVariable;

  g_signal_connect (psSpin, "value_changed", G_CALLBACK (SpinChanged), (gpointer)psSpinSliderLink);
  g_signal_connect (psSlider, "value_changed", G_CALLBACK (SliderChanged), (gpointer)psSpinSliderLink);

	g_ptr_array_add (psSpinSliderData->apsSpinSliderLinks, psSpinSliderLink);
  
  return psSpinSliderLink;
}

static gboolean SpinChanged (GtkWidget * psWidget, gpointer psData) {
  SpinSliderLink * psSpinSliderLink = (SpinSliderLink * )psData;
  float fValue;

	g_assert (GTK_SPIN_BUTTON (psWidget) == psSpinSliderLink->psSpin);

  // Transfer value to slider
  fValue = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidget));

  gtk_range_set_value (GTK_RANGE (psSpinSliderLink->psSlider), fValue);

  // Update the values
  if (psSpinSliderLink->pfVariable) {
  	* psSpinSliderLink->pfVariable = fValue;
  }

  return TRUE;
}

static gboolean SliderChanged (GtkWidget * psWidget, gpointer psData) {
  SpinSliderLink * psSpinSliderLink = (SpinSliderLink * )psData;
  float fValue;

	g_assert (GTK_SCALE (psWidget) == psSpinSliderLink->psSlider);

  fValue = gtk_range_get_value (GTK_RANGE (psWidget));
  gtk_spin_button_set_value (GTK_SPIN_BUTTON (psSpinSliderLink->psSpin), fValue);

  // The values will be updated if the Spin Button value changes

  return TRUE;
}

void SpinSliderUpdate (SpinSliderLink * psSpinSliderLink) {
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psSpinSliderLink->psSpin), * psSpinSliderLink->pfVariable);
}

void SpinSliderSetValue (float fValue, SpinSliderLink * psSpinSliderLink) {
	if (psSpinSliderLink->pfVariable) {
		* psSpinSliderLink->pfVariable = fValue;
			SpinSliderUpdate (psSpinSliderLink);
	}
}


