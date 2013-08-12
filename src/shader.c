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

/* Includes */
#include "shader.h"
#include "vis.h"
#include "string.h"

/* Defines */

/* Enums */

/* Structures */

struct _ShaderPersist {
	GLuint uVertex;
	GLuint uFragment;
	GLuint uProgram;
	Vector3 vPosition;
	Vector3 vScale;
	float fTime;
  bool boActive;
};

/* Function prototypes */

void LinkShader (GLuint uShader, ShaderPersist * psShaderData);
GLuint CompileVertexShader (char const * szShader, ShaderPersist * psShaderData);
GLuint CompileFragmentShader (char const * szShader, ShaderPersist * psShaderData);

/* Function defininitions */
ShaderPersist * NewShaderPersist () {
	ShaderPersist * psShaderData;
	GLenum eError;

	psShaderData = (ShaderPersist *)calloc (1, sizeof (ShaderPersist));
	psShaderData->uProgram = glCreateProgram ();
	if (psShaderData->uProgram == 0) {
		eError = glGetError ();
		if (eError != 0) {
			fprintf (stderr, "OpenGL program creation failed. Error %d.\n", eError);
		}
		else {
			fprintf (stderr, "OpenGL program creation failed.\n");
		}
	}

	psShaderData->uVertex = 0u;
	psShaderData->uFragment = 0u;
	psShaderData->vPosition.fX = 0.0f;
	psShaderData->vPosition.fY = 0.0f;
	psShaderData->vPosition.fZ = 0.0f;
	psShaderData->vScale.fX = 1.0f;
	psShaderData->vScale.fY = 1.0f;
	psShaderData->vScale.fZ = 1.0f;
	psShaderData->fTime = 0.0f;
  psShaderData->boActive = TRUE;

	return psShaderData;
}

void DeleteShaderPersist (ShaderPersist * psShaderData) {
	if (psShaderData) {
		if (psShaderData->uVertex != 0) {
			glDeleteShader (psShaderData->uVertex);
			if (psShaderData->uProgram != 0u) {
				glDetachShader (psShaderData->uProgram, psShaderData->uVertex);
			}
		}

		if (psShaderData->uFragment != 0) {
			glDeleteShader (psShaderData->uFragment);
			if (psShaderData->uProgram != 0u) {
				glDetachShader (psShaderData->uProgram, psShaderData->uFragment);
			}
		}

		if (psShaderData->uProgram != 0u) {
			glDeleteProgram (psShaderData->uProgram);
		}

		free (psShaderData);
		psShaderData = NULL;
	}
}

char * LoadShaderFile (char const * const szFilename) {
	FILE * hFile;
	char * szMemory;
	int nLength;
	int nRead;

	szMemory = NULL;	
	hFile = fopen (szFilename, "r");
	if (hFile) {
		fseek (hFile, 0, SEEK_END);
		nLength = ftell (hFile);
		szMemory = g_new (char, nLength + 1);
		szMemory[nLength] = 0;
		fseek (hFile, 0, SEEK_SET);
		nRead = fread (szMemory, 1, nLength, hFile);
		if (nRead < nLength) {
			fprintf (stderr, "Shader file incompletely read.\n");
		}
		fclose (hFile);
	}
	
	return szMemory;
}

GLuint CompileVertexShader (char const * szShader, ShaderPersist * psShaderData) {
	GLuint uVertex;
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	uVertex = 0u;
	if (szShader) {
		uVertex = glCreateShader (GL_VERTEX_SHADER);
		glShaderSource (uVertex, 1, & szShader, NULL);
		glCompileShader (uVertex);
		glGetShaderiv (uVertex, GL_COMPILE_STATUS, & nStatus);
		if (nStatus != GL_TRUE) {
			eError = glGetError ();
			fprintf (stderr, "Vertex shader failed to compile. Error %d.\n", eError);
		}
		glGetShaderiv (uVertex, GL_INFO_LOG_LENGTH, & nLogsize);
		if (nLogsize > 1) {
			szLog = (char *)malloc (nLogsize + 1);
			glGetShaderInfoLog (uVertex, nLogsize + 1, NULL, szLog);
			szLog[nLogsize] = 0;
			printf ("%s\n", szLog);
			free (szLog);
			szLog = NULL;
		}
	}
	
	return uVertex;
}

GLuint CompileFragmentShader (char const * szShader, ShaderPersist * psShaderData) {
	GLuint uFragment;
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	uFragment = 0u;
	if (szShader) {
		uFragment = glCreateShader (GL_FRAGMENT_SHADER);
		glShaderSource (uFragment, 1, & szShader, NULL);
		glCompileShader (uFragment);
		glGetShaderiv (uFragment, GL_COMPILE_STATUS, & nStatus);
		if (nStatus != GL_TRUE) {
			eError = glGetError ();
			fprintf (stderr, "Fragment shader failed to compile. Error %d.\n", eError);
		}
		glGetShaderiv (uFragment, GL_INFO_LOG_LENGTH, & nLogsize);
		if (nLogsize > 1) {
			szLog = (char *)malloc (nLogsize + 1);
			glGetShaderInfoLog (uFragment, nLogsize + 1, NULL, szLog);
			szLog[nLogsize] = 0;
			printf ("%s\n", szLog);
			free (szLog);
			szLog = NULL;
		}
	}
	
	return uFragment;
}

void LinkShader (GLuint uShader, ShaderPersist * psShaderData) {
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	glAttachShader (psShaderData->uProgram, uShader);
	
	glLinkProgram (psShaderData->uProgram);
	glGetProgramiv (psShaderData->uProgram, GL_LINK_STATUS, & nStatus);
	if (nStatus != GL_TRUE) {
		eError = glGetError ();
		fprintf (stderr, "Shader failed to link. Error %d.\n", eError);
	}
	glGetProgramiv (psShaderData->uProgram, GL_INFO_LOG_LENGTH, & nLogsize);
	if (nLogsize > 1) {
		szLog = (char *)malloc (nLogsize + 1);
		glGetProgramInfoLog (psShaderData->uProgram, nLogsize + 1, NULL, szLog);
		szLog[nLogsize] = 0;
		printf ("%s\n", szLog);
		free (szLog);
		szLog = NULL;
	}
}

void ActivateShader (ShaderPersist * psShaderData) {
	GLint nPosition;
	GLint nScale;
	GLint nTime;

	if ((psShaderData->boActive == TRUE) && (psShaderData->uProgram != 0u)) {
		if ((psShaderData->uVertex != 0) || (psShaderData->uFragment != 0)) {
			glUseProgram (psShaderData->uProgram);

			nPosition = glGetUniformLocation (psShaderData->uProgram, "vPosition");
			nScale = glGetUniformLocation (psShaderData->uProgram, "vScale");
			nTime = glGetUniformLocation (psShaderData->uProgram, "fTime");
			glUniform3fv (nPosition, 1, (GLfloat *)(& psShaderData->vPosition));
			glUniform3fv (nScale, 1, (GLfloat *)(& psShaderData->vScale));
			glUniform1fv (nTime, 1, (GLfloat *)(& psShaderData->fTime));
		}
	}
}

void DeactivateShader (ShaderPersist * psShaderData) {
	glUseProgram (0u);
}

void SetShaderPosition (Vector3 const * const pvPosition, ShaderPersist * psShaderData) {
	psShaderData->vPosition =  *pvPosition;
}

void SetShaderScale (Vector3 const * const pvScale, ShaderPersist * psShaderData) {
	psShaderData->vScale = *pvScale;
}

void SetShaderTime (float const fTime, ShaderPersist * psShaderData) {
	psShaderData->fTime = fTime;
}

void SetShaderActive (bool const boActive, ShaderPersist * psShaderData) {
	psShaderData->boActive = boActive;
}

bool GetShaderActive (ShaderPersist * psShaderData) {
	return psShaderData->boActive;
}

void ShaderRegenerateVertex (char const * const szShader, ShaderPersist * psShaderData) {
	// Free up any previously compiled shader
	if (psShaderData->uVertex != 0) {
		glDeleteShader (psShaderData->uVertex);
		if (psShaderData->uProgram != 0u) {
			glDetachShader (psShaderData->uProgram, psShaderData->uVertex);
		}
	}

	// Compile a new shader
	if (szShader) {
		psShaderData->uVertex = CompileVertexShader (szShader, psShaderData);
		LinkShader (psShaderData->uVertex, psShaderData);
	}
	else {
		fprintf (stderr, "Vertex shader file could not be read.\n");
	}
}

void ShaderRegenerateFragment (char const * const szShader, ShaderPersist * psShaderData) {
	// Free up any previously compiled shader
	if (psShaderData->uFragment != 0) {
		glDeleteShader (psShaderData->uFragment);
		if (psShaderData->uProgram != 0u) {
			glDetachShader (psShaderData->uProgram, psShaderData->uFragment);
		}
	}

	// Compile a new shader
	if (szShader) {
		psShaderData->uFragment = CompileFragmentShader (szShader, psShaderData);
		LinkShader (psShaderData->uFragment, psShaderData);
	}
	else {
		fprintf (stderr, "Vertex shader file could not be read.\n");
	}
}

char * ReplaceTextCopy (char const * const szText, char const * const szToken, char const * const szReplace) {
	// Find the string "/*REPLACE:<szToken>*/<text>/*END*/
	// and replace it with <szReplace>
	int nPosition;
	int nEnd;
	int nTokenLen;
	int nFindLen;
	int nReplaceLen;
	int nEndLen;
	int nTextLen;
	int nNewLen;
	int nFound;
	int nFoundEnd;
	int nGapLen;
	char * szNewCopy;
	char * szFind;
	
	nTokenLen = strlen (szToken);
	nReplaceLen = strlen (szReplace);
	nEndLen = sizeof ("/*END*/") - 1;

	nFindLen = sizeof ("/*REPLACE:*/") - 1 + nTokenLen;
	szFind = malloc (nFindLen + 1);
	strncpy (szFind, "/*REPLACE:", sizeof ("/*REPLACE:"));
	strncpy (szFind + sizeof ("/*REPLACE:") - 1, szToken, nTokenLen);
	strncpy (szFind + sizeof ("/*REPLACE:") - 1 + nTokenLen, "*/", sizeof ("*/"));
	szFind[nFindLen] = '\0';

	nTextLen = 0;
	while (szText[nTextLen] != '\0') {
		nTextLen++;
	}

	nPosition = 0;
	nFound = -1;
	nFoundEnd = -1;
	while ((nFound < 0) && (nPosition < nTextLen)) {
		if (strncmp (szText + nPosition, szFind, nFindLen) == 0) {
			nEnd = nPosition + nFindLen;
			nFoundEnd = -1;
			while ((nFoundEnd < 0) && (szText[nEnd] != 0)) {
				if (strncmp (szText + nEnd, "/*END*/", nEndLen) == 0) {
					nFoundEnd = nEnd;
					nFound = nPosition;
				}
				nEnd++;
			}
		}
		nPosition++;
	}
	
	if (nFound >= 0) {
		nGapLen = nFoundEnd - nFound + nEndLen;
		nNewLen = nTextLen - nGapLen + nReplaceLen;
		szNewCopy = (char *)malloc (nNewLen + 1);
		strncpy (szNewCopy, szText, nFound);
		strncpy (szNewCopy + nFound, szReplace, nReplaceLen);
		strncpy (szNewCopy + nFound + nReplaceLen, szText + nFound + nGapLen, nTextLen - nFound - nGapLen);
		szNewCopy[nNewLen] = '\0';
	}
	else {
		nNewLen = nTextLen;
		szNewCopy = (char *)malloc (nNewLen + 1);
		strncpy (szNewCopy, szText, nTextLen);
		szNewCopy[nNewLen] = '\0';
	}

	return szNewCopy;
}

char * ReplaceTextMove (char * szText, char const * const szToken, char const * const szReplace) {
	char * szNewCopy;

	szNewCopy = ReplaceTextCopy (szText, szToken, szReplace);
	g_free (szText);
	
	return szNewCopy;
}

GLuint GetShaderProgram (ShaderPersist * psShaderData) {
	return psShaderData->uProgram;
}

