#ifndef TEXTURES_H
#define TEXTURES_H

typedef struct _TexPersist TexPersist;

typedef enum _TEXNAME {
  TEXNAME_INVALID = -1,
  
  TEXNAME_NOTES,
  
  TEXNAME_NUM
} TEXNAME;

TexPersist * NewTexPersist (int nMaxTextures);
void DeleteTexPersist (TexPersist * psTexData);
void LoadTextures (TexPersist * psTexData);
GLuint GetTexture (TEXNAME eTexture, GLfloat afTexCoords[], TexPersist * psTexData);
GLuint GetTexturePortion (TEXNAME eTexture, GLfloat afPortion[], GLfloat afTexCoords[], TexPersist * psTexData);
GLuint LoadTextureRaw (char const * const szFilename, int const nWidth, int const nHeight, bool boAlpha, TexPersist * psTexData);
inline void GetTextureCoord (TEXNAME eTexture, GLfloat fXIn, GLfloat fYIn, GLfloat * pfXOut, GLfloat * pfYOut, TexPersist * psTexData);

#endif /* TEXTURES_H */

