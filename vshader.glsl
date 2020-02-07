#version 150

in vec4 vPosition;
uniform mat4 ModelView, Projection;

flat out vec4 f_colour;

void main()
{
	if (gl_VertexID % 12 % 2 == 0 ) 
	{
		f_colour = vec4(1,1,1,1); // blue
	}
	else
	{
		f_colour = vec4(1,0,0,1); 
	}	



  gl_Position = Projection * ModelView * vPosition;
  
  // gl_Position = ModelView * vPosition;
  // gl_Position.xy += (gl_Position.z+ 2)/5;
  // gl_Position.z += 2;
}