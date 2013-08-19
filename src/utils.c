///////////////////////////////////////////////////////////////////
// Utils
// Generally useful definitions, structures, functions, etc.
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

static Matrix3 mId0 = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

///////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////
// Function definitions

float absf (float fValue) {
  if (fValue < 0.0f) fValue = -fValue;
  return fValue;
}

Vector3 Normal (Vector3 * v1, Vector3 * v2) {
  Vector3 vReturn;

  vReturn.fX = (v1->fY * v2->fZ) - (v1->fZ * v2->fY);
  vReturn.fY = (v1->fZ * v2->fX) - (v1->fX * v2->fZ);
  vReturn.fZ = (v1->fX * v2->fY) - (v1->fY * v2->fX);

  Normalise (& vReturn);

  return vReturn;
}

void Normalise (Vector3 * v1) {
  float fLength;

  fLength = sqrt ((v1->fX * v1->fX) + (v1->fY * v1->fY) + (v1->fZ * v1->fZ));

  v1->fX /= fLength;
  v1->fY /= fLength;
  v1->fZ /= fLength;
}

void Normalise3f (float * pfX, float * pfY, float * pfZ) {
  float fLength;

  fLength = sqrt (((*pfX) * (*pfX)) + ((*pfY) * (*pfY)) + ((*pfZ) * (*pfZ)));

  *pfX /= fLength;
  *pfY /= fLength;
  *pfZ /= fLength;
}

float Length (Vector3 * v1) {
  return sqrt ((v1->fX * v1->fX) + (v1->fY * v1->fY) + (v1->fZ * v1->fZ));
}

Matrix3 Invert (Matrix3 * m1) {
  Matrix3 vReturn;
  float fDet;

  fDet = Determinant (m1);
  if (fDet != 0.0f) {
    fDet = 1 / fDet;

    vReturn.fA1 =   fDet * ((m1->fB2 * m1->fC3) - (m1->fC2 * m1->fB3));
    vReturn.fA2 = - fDet * ((m1->fA2 * m1->fC3) - (m1->fC2 * m1->fA3));
    vReturn.fA3 =   fDet * ((m1->fA2 * m1->fB3) - (m1->fB2 * m1->fA3));

    vReturn.fB1 = - fDet * ((m1->fB1 * m1->fC3) - (m1->fC1 * m1->fB3));
    vReturn.fB2 =   fDet * ((m1->fA1 * m1->fC3) - (m1->fC1 * m1->fA3));
    vReturn.fB3 = - fDet * ((m1->fA1 * m1->fB3) - (m1->fB1 * m1->fA3));

    vReturn.fC1 =   fDet * ((m1->fB1 * m1->fC2) - (m1->fC1 * m1->fB2));
    vReturn.fC2 = - fDet * ((m1->fA1 * m1->fC2) - (m1->fC1 * m1->fA2));
    vReturn.fC3 =   fDet * ((m1->fA1 * m1->fB2) - (m1->fB1 * m1->fA2));
  }
  else {
    vReturn = mId0;
  }

  return vReturn;  
}

float Determinant (Matrix3 * m1) {
  return (m1->fA1 * ((m1->fB2 * m1->fC3) - (m1->fB3 * m1->fC2)))
    - (m1->fA2 * ((m1->fB1 * m1->fC3) - (m1->fB3 * m1->fC1)))
    + (m1->fA3 * ((m1->fB1 * m1->fC2) - (m1->fB2 * m1->fC1)));
}

float DotProdAngle (float fX1, float fY1, float fX2, float fY2) {
  float fAngle;
  float fScaler;

  float fY;
  float fRot;

  fRot = atan2 (fY1, fX1);
  fY = - (fX2 * sin (fRot)) + (fY2 * cos (fRot));

  fScaler = sqrt ((fX1 * fX1) + (fY1 * fY1)) * sqrt ((fX2 * fX2) + (fY2 * fY2));
  fAngle = acos (((fX1 * fX2) + (fY1 * fY2)) / fScaler);

  if (fY < 0) fAngle = -fAngle;

  return fAngle;
}

// Useful for calculating the result of simultaneous equations
// and therefore where the normal to a plane passes through a point
// [ a1 b1 c1 ] [ v1 ]   [ r1 ]   [ (a1*v1) + (b1*v2) + (c1*v3) ]
// [ a2 b2 c2 ] [ v2 ] = [ r3 ] = [ (a2*v1) + (b2*v2) + (c2*v3) ]
// [ a3 b3 c3 ] [ v3 ]   [ r3 ]   [ (a3*v1) + (b3*v2) + (c3*v3) ]
Vector3 MultMatrixVector (Matrix3 * m1, Vector3 * v1) {
  Vector3 vReturn;
  vReturn.fX = (m1->fA1 * v1->fX) + (m1->fB1 * v1->fY) + (m1->fC1 * v1->fZ);
  vReturn.fY = (m1->fA2 * v1->fX) + (m1->fB2 * v1->fY) + (m1->fC2 * v1->fZ);
  vReturn.fZ = (m1->fA3 * v1->fX) + (m1->fB3 * v1->fY) + (m1->fC3 * v1->fZ);

  return vReturn;
}

void PrintMatrix (Matrix3 * m1) {
  printf ("[ %f, \t%f, \t%f \t]\n", m1->fA1, m1->fB1, m1->fC1);
  printf ("[ %f, \t%f, \t%f \t]\n", m1->fA2, m1->fB2, m1->fC2);
  printf ("[ %f, \t%f, \t%f \t]\n", m1->fA3, m1->fB3, m1->fC3);
}

void PrintVector (Vector3 * v1) {
  printf ("[ %f, \t%f, \t%f \t]\n", v1->fX, v1->fY, v1->fZ);
}

void AddVectors (Vector3 * pvsAddition, Vector3 const * pvsV1, Vector3 const * pvsV2) {
	if (pvsAddition) {
		pvsAddition->fX = pvsV1->fX + pvsV2->fX;
		pvsAddition->fY = pvsV1->fY + pvsV2->fY;
		pvsAddition->fZ = pvsV1->fZ + pvsV2->fZ;
	}
}

void ScaleVector (Vector3 * pvsScaled, Vector3 const * pvsV1, float fScale) {
	if (pvsScaled) {
		pvsScaled->fX = pvsV1->fX * fScale;
		pvsScaled->fY = pvsV1->fY * fScale;
		pvsScaled->fZ = pvsV1->fZ * fScale;
	}
}


