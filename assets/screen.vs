#version 120

uniform vec3 vPosition, vScale;
uniform float fTime;

void main() {
	// Specify position of vertex
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}

