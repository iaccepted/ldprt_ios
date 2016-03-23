attribute vec3 position;
attribute vec3 color;

uniform mat4 MVP;

varying highp vec3 fcolor;

void main()
{
	gl_Position = MVP * vec4(position, 1.0);
	fcolor = color;
}