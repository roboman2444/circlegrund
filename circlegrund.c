#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <math.h>

#define TRUE 1
#define FALSE 0


const char * circleshadervert = "\
	attribute vec2 tcattrib;\n\
	attribute vec2 posattrib;\n\
	attribute float colattrib;\n\
	varying float col;\n\
	varying vec2 tc;\n\
	void main(){\n\
		col = colattrib;\n\
		tc = tcattrib;\n\
		gl_Position = vec4(posattrib, 0.0, 1.0);\n\
	}\n\
";
//uses iq's pallette code
const char * circleshaderfrag = "\
	varying vec2 tc;\n\
	varying float col;\n\
	vec3 palette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d){\n\
		return a + b*cos( 6.28318*(c*t+d) );\n\
	}\n\
	void main(){\n\
		float dist = length(tc);\n\
		if(dist >=1.0) discard;\n\
		float xout = clamp((smoothstep(0.9, 1.0, dist) - max(0.0, dist - 0.9)) * 0.5, 0.0, 1.0);\n\
		//float xin = clamp(0.3 / (dist * dist + 0.1), 0.0, 1.0);\n\
		float xin = 1.0 - dist*dist ;\n\
		gl_FragColor.rgb = clamp(palette(col * 20.0+dist/5.0, vec3(0.5), vec3(0.5), vec3(1.0), vec3(0.0, 0.2, 0.4)) * xin, vec3(0.0), vec3(1.0));\n\
		gl_FragColor.r *= 1.0 - xout;\n\
		gl_FragColor.a = 0.0;\n\
	}\n\
";
const char * postshadervert = "\
	attribute vec2 tcattrib;\n\
	attribute vec2 posattrib;\n\
	varying vec2 tc;\n\
	void main(){\n\
		tc = tcattrib;\n\
		gl_Position = vec4(posattrib, 0.0, 1.0);\n\
	}\n\
";
const char * postshaderfrag = "\
	varying vec2 tc;\n\
	void main(){\n\
		float dist = length(tc);\n\
		if(dist >=1.0) discard;\n\
		gl_FragColor.r = 1.0 - max(0.0, dist - 0.5);\n\
		gl_FragColor.g = 0.5;\n\
		gl_FragColor.b = 0.0;\n\
		gl_FragColor.a = 0.0;\n\
	}\n\
";

int shader_printProgramLogStatus(const int id){
	GLint blen = 0;
	glGetProgramiv(id, GL_INFO_LOG_LENGTH, &blen);
	if(blen > 1){
		GLchar * log = (GLchar *) malloc(blen);
		glGetProgramInfoLog(id, blen, 0, log);
		printf("program log: \n%s\n", log);
		free(log);
	}
	return blen;
}
int shader_printShaderLogStatus(const int id){
	GLint blen = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &blen);
	if(blen > 1){
		GLchar * log = (GLchar *) malloc(blen);
		glGetShaderInfoLog(id, blen, 0, log);
		printf("shader log: \n%s\n", log);
		free(log);
	}
	return blen;
}

#define COLATTRIBLOC 2
#define POSATTRIBLOC 0
#define TCATTRIBLOC 1

int main(void){
	GLFWwindow * window;
	if(!glfwInit()) return -1;
	window 	= glfwCreateWindow(1024, 768, "circlegrund", NULL, NULL);
	if (!window){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	GLenum glewError = glewInit();
	if(glewError != GLEW_OK){
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		return FALSE;
	}
	//gl and glew are good 2 go

	GLuint vertid = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragid = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertid, 1, (const GLchar **)&circleshadervert, 0);
	glShaderSource(fragid, 1, (const GLchar **)&circleshaderfrag, 0);

	glCompileShader(vertid);
	shader_printShaderLogStatus(vertid);
	glCompileShader(fragid);
	shader_printShaderLogStatus(fragid);


	GLuint shaderid = glCreateProgram();
	if(!shaderid) printf("unable to greate program\n");
	glAttachShader(shaderid, vertid);
	glAttachShader(shaderid, fragid);

	glBindAttribLocation(shaderid, COLATTRIBLOC, "colattrib");
	glBindAttribLocation(shaderid, POSATTRIBLOC, "posattrib");
	glBindAttribLocation(shaderid, TCATTRIBLOC, "tcattrib");

	glLinkProgram(shaderid);
	glDeleteShader(vertid);
	glDeleteShader(fragid);

	shader_printProgramLogStatus(shaderid);
	glUseProgram(shaderid);

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);

	int numcircles = 256;


	GLfloat * vertices = malloc(numcircles * 8 * sizeof(GLfloat));
	GLfloat * texcoords = malloc(numcircles * 8 * sizeof(GLfloat));
	GLfloat * colors = malloc(numcircles * 4 * sizeof(GLfloat));
	GLuint * indices = malloc(numcircles * 6 * sizeof(GLuint));
	float * pos = malloc(numcircles * 2 * sizeof(float));
	float * speed = malloc(numcircles * 2 * sizeof(float)); //direction in radians and a speed
	float * sizes = malloc(numcircles * sizeof(float));
	int i;
	for(i = 0; i < numcircles; i++){
		sizes[i] = (double)rand() / (double)RAND_MAX * 0.1;
		texcoords[i * 8 +0] = -1.0;
		texcoords[i * 8 +1] = -1.0;

		texcoords[i * 8 +2] = 1.0;
		texcoords[i * 8 +3] = -1.0;

		texcoords[i * 8 +4] = 1.0;
		texcoords[i * 8 +5] = 1.0;

		texcoords[i * 8 +6] = -1.0;
		texcoords[i * 8 +7] = 1.0;

		indices[i *6 +0] = i * 4 + 0;
		indices[i *6 +1] = i * 4 + 1;
		indices[i *6 +2] = i * 4 + 2;

		indices[i *6 +3] = i * 4 + 0;
		indices[i *6 +4] = i * 4 + 2;
		indices[i *6 +5] = i * 4 + 3;
	}
	glEnableVertexAttribArray(POSATTRIBLOC);
	glVertexAttribPointer(POSATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertices);
	glEnableVertexAttribArray(COLATTRIBLOC);
	glVertexAttribPointer(COLATTRIBLOC, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), colors);
	glEnableVertexAttribArray(TCATTRIBLOC);
	glVertexAttribPointer(TCATTRIBLOC, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texcoords);

	while(!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT);
		double seconds = glfwGetTime() * 0.5;

		int i;
		for(i = 0; i <numcircles; i++){
//			float circleradius = 0.001f / (abs(sin(seconds * 0.01 + sizes[i] * 1000.0)));
			float circleradius = sin(seconds * 0.1 + sizes[i] * 1000.0);
			circleradius *= circleradius * 0.05;
			if(circleradius < 0.01) circleradius = 0.01;
			float y	= sin((seconds * 1.0 + (float)i) + 1.0) * circleradius * 16.0;
			float x	= cos((seconds * 0.05 - (float)i * 1.5) + 2.0) + ((float)i*2.0)/numcircles - 1.0;
			colors[i * 4 + 0] = circleradius;
			colors[i * 4 + 1] = circleradius;
			colors[i * 4 + 2] = circleradius;
			colors[i * 4 + 3] = circleradius;

			vertices[i * 8 +0] = x - circleradius;
			vertices[i * 8 +1] = y - circleradius;
			vertices[i * 8 +2] = x + circleradius;
			vertices[i * 8 +3] = y - circleradius;
			vertices[i * 8 +4] = x + circleradius;
			vertices[i * 8 +5] = y + circleradius;
			vertices[i * 8 +6] = x - circleradius;
			vertices[i * 8 +7] = y + circleradius;
		}
//		glDrawArrays(GL_POINTS, 0, numcircles *6);
		glDrawElements(GL_TRIANGLES, numcircles * 6, GL_UNSIGNED_INT, indices);
		glfwSwapBuffers(window);
	}
}
