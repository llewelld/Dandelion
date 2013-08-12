/*******************************************************************
 * Shader.c
 * Manage the OpenGL GLSL shaders
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * December 2011
 *******************************************************************
*/

#ifndef SHADER_H
#define SHADER_H

/* Includes */

#include "utils.h"

/* Defines */

/* Enums */

/* Structures */
typedef struct _ShaderPersist ShaderPersist;

/* Function prototypes */
ShaderPersist * NewShaderPersist ();
void DeleteShaderPersist (ShaderPersist * psShaderData);
void ActivateShader (ShaderPersist * psShaderData);
void DeactivateShader (ShaderPersist * psShaderData);
void SetShaderPosition (Vector3 const * const pvPosition, ShaderPersist * psShaderData);
void SetShaderScale (Vector3 const * const pvScale, ShaderPersist * psShaderData);
void SetShaderTime (float const fTime, ShaderPersist * psShaderData);
void ShaderRegenerateVertex (char const * const szShader, ShaderPersist * psShaderData);
void ShaderRegenerateFragment (char const * const szShader, ShaderPersist * psShaderData);
void SetShaderActive (bool const boActive, ShaderPersist * psShaderData);
bool GetShaderActive (ShaderPersist * psShaderData);
char * LoadShaderFile (char const * const szFilename);
char * ReplaceTextCopy (char const * const szText, char const * const szToken, char const * const szReplace);
char * ReplaceTextMove (char * szText, char const * const szToken, char const * const szReplace);
GLuint GetShaderProgram (ShaderPersist * psShaderData);

#endif /* SHADER_H */

