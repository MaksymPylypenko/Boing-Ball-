#version 150

in vec4 vPosition;

uniform mat4 ViewSphere;
uniform mat4 ViewSphereInvTra;
uniform mat4 ViewGround;
uniform mat4 ViewGroundInvTra;
uniform mat4 Projection; 
uniform mat4 ViewCamera;

uniform mat4 ViewWallShadow;
uniform mat4 ViewGroundShadow;

uniform int groundIndex;
uniform int wallIndex;
uniform int sphereIndex;
uniform int groundShadowIndex;

flat out vec4 f_colour;

// for lighting

uniform bool UseLighting;
uniform vec4 lightPositionTop;
uniform vec4 lightPositionNear;

in vec4 vNormal;
out vec3 N, L, E;
out vec3 N2, L2, E2;

flat out vec4 AmbientMaterial;
flat out vec4 DiffuseMaterial;
flat out vec4 SpecularMaterial;
flat out float Shininess;


void set(mat4 ViewModel, mat4 ViewModelInvTra){
	if (UseLighting) {
		/*** Blinn-Phong shader: ***/

		vec3 pos = (ViewModel * vPosition).xyz;

		L = lightPositionTop.xyz - pos;
		E = -pos;
		N = ( ViewModelInvTra*vec4(vNormal.x,vNormal.y,vNormal.z,0) ).xyz;

		
		L2 = lightPositionNear.xyz - pos;
		E2 = -pos;
		N2 = ( ViewModelInvTra*vec4(vNormal.x,vNormal.y,vNormal.z,0) ).xyz;
	}
	gl_Position = Projection * ViewCamera * ViewModel * vPosition;
}


// Materials from http://devernay.free.fr/cours/opengl/materials.html
void red_rubber(){
	AmbientMaterial = vec4(		0.05,	0.0,	0.0,	1.0);
	DiffuseMaterial = vec4(		0.8,	0.1,	0.1,	1.0);
	SpecularMaterial = vec4(	0.7,	0.04,	0.04,	1.0);
	Shininess = 0.078125;
	f_colour = vec4(			0.05,	0.0,	0.0,	1); 
}

void white_rubber(){
	AmbientMaterial = vec4(		0.05,	0.05,	0.05,	1.0);
	DiffuseMaterial = vec4(		0.8,	0.8,	0.7,	1.0);
	SpecularMaterial = vec4(	0.7,	0.7,	0.7,	1.0);
	Shininess = 0.078125;
	f_colour = vec4(			0.05,	0.05,	0.05,	1); 
}

void black_rubber(){
	AmbientMaterial = vec4(		0.02,	0.02,	0.02,	1.0);
	DiffuseMaterial = vec4(		0.11,	0.11,	0.31,	1.0);
	SpecularMaterial = vec4(	0.4,	0.4,	0.4,	1.0);
	Shininess = 0.078125;
	f_colour = vec4(			0.02,	0.02,	0.02,	1); 
}

void main()
{
	if(gl_VertexID < groundIndex)
	{ 
		set(ViewGround,ViewGroundInvTra);
	
		if (gl_VertexID % 2 == 0 ) 
		{ 
			black_rubber( ); 
		} else 
		{ 
			white_rubber(); 
		}
	} 
	else if(gl_VertexID < wallIndex)
	{ 
		set(ViewGround,ViewGroundInvTra);		
		
		if (gl_VertexID % 2 == 0 ) 
		{ 
			white_rubber(); 
		} else 
		{ 
			black_rubber(); 
		}
	}
	else if(gl_VertexID < sphereIndex) 
	{
		set(ViewSphere,ViewSphereInvTra);

		if (gl_VertexID % 2 == 0 ) 
		{ 
			white_rubber(); 
		} else 
		{ 
			red_rubber(); 
		}			
	}
	else if(gl_VertexID < groundShadowIndex) 
	{
		set(ViewGroundShadow, ViewGroundInvTra);
		black_rubber();
	}
	else // if(gl_VertexID < wallShadowIndex)
	{
		set(ViewWallShadow, ViewGroundInvTra);
		black_rubber();
	}

}