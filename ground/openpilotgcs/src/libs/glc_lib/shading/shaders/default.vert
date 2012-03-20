uniform mat4 modelview_matrix;
uniform mat4 mvp_matrix;

attribute vec4 a_position;
attribute vec4 a_textcoord0;
attribute vec4 a_color;
attribute vec3 a_normal;

void main()
{
	gl_Position= mvp_matrix * a_position;	
}