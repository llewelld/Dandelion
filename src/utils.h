///////////////////////////////////////////////////////////////////
// Utils
// Generally useful definitions, structures, functions, etc.
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////

#ifndef UTILS_H
#define UTILS_H

///////////////////////////////////////////////////////////////////
// Includes

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_TWOPI
#define M_TWOPI (M_PI * 2.0)
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <GLee.h>
#include <glib.h>
#include <GL/glut.h>
#include <gtk/gtk.h>
#include <stdbool.h>

///////////////////////////////////////////////////////////////////
// Defines

#ifndef _WIN32
#define stricmp strcasecmp
#endif

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _Vector3 {
  GLfloat fX;
  GLfloat fY;
  GLfloat fZ;
} Vector3;

typedef struct _Matrix3 {
  GLfloat fA1;
  GLfloat fA2;
  GLfloat fA3;
  GLfloat fB1;
  GLfloat fB2;
  GLfloat fB3;
  GLfloat fC1;
  GLfloat fC2;
  GLfloat fC3;
} Matrix3;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

float absf (float fValue);
Vector3 Normal (Vector3 * v1, Vector3 * v2);
void Normalise (Vector3 * v1);
void Normalise3f (float * pfX, float * pfY, float * pfZ);
float Length (Vector3 * v1);
Matrix3 Invert (Matrix3 * m1);
float Determinant (Matrix3 * m1);
float DotProdAngle (float fX1, float fY1, float fX2, float fY2);
Vector3 MultMatrixVector (Matrix3 * m1, Vector3 * v1);
void PrintMatrix (Matrix3 * m1);
void PrintVector (Vector3 * v1);
void AddVectors (Vector3 * pvsAddition, Vector3 const * pvsV1, Vector3 const * pvsV2);
void ScaleVector (Vector3 * pvsScaled, Vector3 const * pvsV1, float fScale);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* UTILS_H */

