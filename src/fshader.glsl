#version 150

flat in vec4 f_colour;
out vec4 out_colour;

// lighting...
in vec3 N, L, E;
in vec3 N2, L2, E2;
flat in vec4 AmbientMaterial, DiffuseMaterial, SpecularMaterial;
uniform vec4 AmbientLight, DiffuseLight, SpecularLight;
flat in float Shininess;

uniform bool UseLighting;

void main() 
{
	if (!UseLighting) {
		out_colour = f_colour;
	} else {
		vec3 H = normalize( L + E );
		vec3 H2 = normalize( L2 + E2 );

		vec4 ambient = AmbientLight*AmbientMaterial;

		vec4 DiffuseProduct= DiffuseLight*DiffuseMaterial;
		vec4 SpecularProduct= SpecularLight*SpecularMaterial;
		
		float Kd = max( dot(L, N), 0.0 );
		vec4  diffuse = Kd * DiffuseProduct;

		float Kd2 = max( dot(L2, N2), 0.0 );
		vec4  diffuse2 = Kd2 * DiffuseProduct;
		
		float Ks = pow( max(dot(N, H), 0.0), Shininess );
		vec4  specular = Ks * SpecularProduct;

		float Ks2 = pow( max(dot(N2, H2), 0.0), Shininess );
		vec4  specular2 = Ks2 * SpecularProduct;

		if ( dot(L, N) < 0.0 ) {
			specular = vec4(0.0, 0.0, 0.0, 1.0);
		}

		if ( dot(L2, N2) < 0.0 ) {
			specular2 = vec4(0.0, 0.0, 0.0, 1.0);
		}

		out_colour = ambient + diffuse + diffuse2 + specular + specular2;
		out_colour.a = 1.0;
	}
}
