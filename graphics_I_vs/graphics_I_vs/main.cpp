/*  by Alun Evans 2016 LaSalle (alunthomasevasevans@gmail.com)

MAIN:
+ This file creates the window and uses a simple render loop to paint a coloured quad
+ GLUT is a simple toolkit for create OpenGL applications. 
  To download glut go to http://alunevans.info/resources/freeglut.zip
  To install GLUT, unzip the copy the contents of the 'include', 'bin' and 'lib' directories to any valid 
  for your project e.g. default places for VS2015: 
    C:\Archivos de programa (x86)\Microsoft Visual Studio 14.0\VC\include
	C:\Archivos de programa (x86)\Microsoft Visual Studio 14.0\VC\bin
	C:\Archivos de programa (x86)\Microsoft Visual Studio 14.0\VC\lib
+ GLEW is a library that loads some useful OpenGL extensions.
  To download GLEW go to http://alunevans.info/resources/glew.zip  
  To install GLEW, unzip and copy the contents the same as GLUT
+ GLM is a maths library that provides us useful C++ classes for managing vectors and matrices
  To download GLM go here http://alunevans.info/resources/glm.zip 
  To install, unzip and copy the contents of the include directory to the include path. GLM
  is a 'header only' library which means it does not have any pre-compiled libraries
*/

//include some standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <cmath> 
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <math.h>
//include GLUT, GLEW and GLM - see below
#include <GL/glew.h>
#include <GL/glut.h>
#include <glm/glm.hpp>
#include <string> 
#include <vector> 
#define TINYOBJLOADER_IMPLEMENTATION 
#include "tiny_obj_loader.h"
#include "imageloader.h"
#define M_PI 3.1416

//include our custom shader Class
#include "Shader.h"

//namespaces in C++ are declarative regions that permit scope. For example, the GLM library
//declares the glm:: namespace. So, normally, to use anything inside the library we must
//prefix it with glm:: e.g. to create a 3-component vector we type glm::vec3(0.0,1.0,0.0);

//the 'using namespace' declaration tells the compiler that, for this file, we dispense with 
//the need to prefix glm:: to all of GLMs components e.g. we only have to type vec3 instead of glm::vec3 
using namespace glm;

// ------------------------------------------------------------------------------------------
// Global Variables
// ------------------------------------------------------------------------------------------

// Default window size, in pixels
int g_ViewportWidth = 1024;
int g_ViewportHeight = 1020;
float g_moon = 0.0f, g_earth = 0.0f, g_plane= 0, distancePlane = 0.01f;
double  planeX = 0.0f, planeY = 0.0f;
float g_angle = 90.0f, g_camera = 45;
bool isMousePressed = true;
int i = 0;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

// background colour - a GLM 3-component vector
const vec3 g_ColorFons(0.2f, 0.2f, 0.2f);
const float ROTATIONAL_SPEED = 0.5f;
glm::vec2 oldMousePosition;

// shader program identifier - this is literally just an integer, which is assigned automatically
// later on
GLuint g_ShaderProgram = 0;
GLuint g_ShaderProgram_stars = 0, g_ShaderProgram_spec = 0;
GLuint texture_id[5];
GLuint texture_earth_normal, texture_earth_specular;
vec3 g_light_dir(100, 100, 100), light_pos, translateTierra(20.0f * cos(g_earth), 0.0f, 20.0f * sin(g_earth)),sunTranslate (0.0f, 0.0f, -50.0f);
vec3 moontranslate(3.0f * cos(g_moon), 0.0f, 3.0f * sin(g_moon));
//  Vertex Array Object (VAO) identifier which contains the geometry. Again this is an
// automatically assigned integer
GLuint g_VaoArray[5];
std::string Imageinputfile[7];
Image *image[5], *normalEarth[2], *specularEarth[2];
// Numbre of triangles we are painting. We edit this when creating geometry
GLuint g_NumTriangles = 0;
mat4 rotate_matrix, sun, model, scaled, tierra, moon, univers, plane,rotateMatrix;

bool lock = false, front = false, back = false;

void cargaEsfera(std::vector<tinyobj::shape_t> shapes, std::vector<tinyobj::material_t> materials)
{
	GLuint buffer;

	for (i = 0; i < 4; i++) {
		//Creamos Vertex Array Object y lo bindeamos para poder usarlo
		glGenVertexArrays(1, &g_VaoArray[i]);
		glBindVertexArray(g_VaoArray[i]);
		//creamos el handler del VBO ( Vertex Buffer object) y lo bindeamos para activarlo

		/*
		target
		Specifies the target to which the buffer object is bound for glBufferData
		size
		Specifies the size in bytes of the buffer object's new data store.
		data
		Specifies a pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.
		usage
		Specifies the expected usage pattern of the data store.
		*/
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, shapes[0].mesh.texcoords.size() * sizeof(float), &(shapes[0].mesh.texcoords[0]), GL_STATIC_DRAW);

		GLuint uvLoc = glGetAttribLocation(g_ShaderProgram, "uvs");
		glEnableVertexAttribArray(uvLoc);
		glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		// Crea VBO per vèrtexs
		glGenBuffers(1, &buffer); //glGenBuffers requires a reference
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, shapes[0].mesh.positions.size() * sizeof(float), &(shapes[0].mesh.positions[0]), GL_STATIC_DRAW); //fill it with out vertices

																																		// Activate shader attribute for this buffer
		GLuint vertexLoc = glGetAttribLocation(g_ShaderProgram, "vertexPos"); //find attribute vertexPos in shader
		glEnableVertexAttribArray(vertexLoc); //enable it
		glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0); //specify location and data format

																	   // Create VBO por normales
		glGenBuffers(1, &buffer); //create more new buffers
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[0].mesh.normals.size() * sizeof(unsigned), &(shapes[0].mesh.normals[0]), GL_STATIC_DRAW); //fill buffer with normal data

		GLuint normals = glGetAttribLocation(g_ShaderProgram, "a_normal"); //find attribute vertexPos in shader
		glEnableVertexAttribArray(normals); //enable it
		glVertexAttribPointer(normals, 3, GL_FLOAT, GL_FALSE, 0, 0); //specify location and data format

																	 // Create VBO por indices
		glGenBuffers(1, &buffer); //create more new buffers
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[0].mesh.indices.size() * sizeof(unsigned), &(shapes[0].mesh.indices[0]), GL_STATIC_DRAW); //fill buffer with index data


		g_NumTriangles = shapes[0].mesh.indices.size() / 3;

		glGenTextures(1, &texture_id[i]);
		glBindTexture(GL_TEXTURE_2D, texture_id[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, //target
			0, //level, = 0 base, no mipmap
			GL_RGB, //the way opengl stores the data
			image[i]->width, //width
			image[i]->height, //height
			0, //border, must be 0!
			GL_RGB, //the format of the original data
			GL_UNSIGNED_BYTE, //type of data
			image[i]->pixels); // pointer to start of data

		if (i == 1) {
			glGenTextures(1, &texture_earth_normal);
			glBindTexture(GL_TEXTURE_2D, texture_earth_normal);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, //target
				0, //level, = 0 base, no mipmap
				GL_RGB, //the way opengl stores the data
				normalEarth[0]->width, //width
				normalEarth[0]->height, //height
				0, //border, must be 0!
				GL_RGB, //the format of the original data
				GL_UNSIGNED_BYTE, //type of data
				normalEarth[0]->pixels); // pointer to start of data

			glGenTextures(1, &texture_earth_specular);
			glBindTexture(GL_TEXTURE_2D, texture_earth_specular);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, //target
				0, //level, = 0 base, no mipmap
				GL_RGB, //the way opengl stores the data
				specularEarth[0]->width, //width
				specularEarth[0]->height, //height
				0, //border, must be 0!
				GL_RGB, //the format of the original data
				GL_UNSIGNED_BYTE, //type of data
				specularEarth[0]->pixels); // pointer to start of data
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}

void cargaAvion(std::vector<tinyobj::shape_t> shape, std::vector<tinyobj::material_t> materials) 
{
	GLuint buffer;

	//Creamos Vertex Array Object y lo bindeamos para poder usarlo
	glGenVertexArrays(1, &g_VaoArray[4]);
	glBindVertexArray(g_VaoArray[4]);
	//creamos el handler del VBO ( Vertex Buffer object) y lo bindeamos para activarlo

	/*
	target
	Specifies the target to which the buffer object is bound for glBufferData
	size
	Specifies the size in bytes of the buffer object's new data store.
	data
	Specifies a pointer to data that will be copied into the data store for initialization, or NULL if no data is to be copied.
	usage
	Specifies the expected usage pattern of the data store.
	*/
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, shape[0].mesh.texcoords.size() * sizeof(float), &(shape[0].mesh.texcoords[0]), GL_STATIC_DRAW);

	GLuint uvLoc = glGetAttribLocation(g_ShaderProgram, "uvs");
	glEnableVertexAttribArray(uvLoc);
	glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Crea VBO per vèrtexs
	glGenBuffers(1, &buffer); //glGenBuffers requires a reference
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, shape[0].mesh.positions.size() * sizeof(float), &(shape[0].mesh.positions[0]), GL_STATIC_DRAW); //fill it with out vertices

																																	// Activate shader attribute for this buffer
	GLuint vertexLoc = glGetAttribLocation(g_ShaderProgram, "vertexPos"); //find attribute vertexPos in shader
	glEnableVertexAttribArray(vertexLoc); //enable it
	glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, 0); //specify location and data format

																	// Create VBO por normales
	glGenBuffers(1, &buffer); //create more new buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape[0].mesh.normals.size() * sizeof(unsigned), &(shape[0].mesh.normals[0]), GL_STATIC_DRAW); //fill buffer with normal data

	GLuint normals = glGetAttribLocation(g_ShaderProgram, "a_normal"); //find attribute vertexPos in shader
	glEnableVertexAttribArray(normals); //enable it
	glVertexAttribPointer(normals, 3, GL_FLOAT, GL_FALSE, 0, 0); //specify location and data format

																	// Create VBO por indices
	glGenBuffers(1, &buffer); //create more new buffers
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer); //bind them as element array
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, shape[0].mesh.indices.size() * sizeof(unsigned), &(shape[0].mesh.indices[0]), GL_STATIC_DRAW); //fill buffer with index data


	g_NumTriangles = shape[0].mesh.indices.size() / 3;

	glGenTextures(1, &texture_id[4]);
	glBindTexture(GL_TEXTURE_2D, texture_id[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, //target
		0, //level, = 0 base, no mipmap
		GL_RGB, //the way opengl stores the data
		image[4]->width, //width
		image[4]->height, //height
		0, //border, must be 0!
		GL_RGB, //the format of the original data
		GL_UNSIGNED_BYTE, //type of data
		image[4]->pixels); // pointer to start of data

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
}

void creaGeometria()
{
	//String basica para saber donde esta el proyecto
	std::string basepath = ""; 
	//path para añadir el objeto esfera
	std::string inputfile = "sphere.obj";
	std::string inputPlane = "main.obj";


	std::vector<tinyobj::shape_t> shapes,shapePlane; 
	std::vector<tinyobj::material_t> materials,materialsPlane;

	std::string err; 
	//cargamos la esfera
	bool ret = tinyobj::LoadObj(shapes, materials, err, inputfile.c_str(), basepath.c_str());

	if (!err.empty()) { 
		std::cerr << err << std::endl; 
	}
	//Comprovamos cuantos objetos estamos cargando
	//std::cout << "# of shapes : " << shapes.size() << std::endl;
	//Cargamos la textura que le ponemos al la esfera
	Imageinputfile[0] = "milkyway.bmp";
	image[0] = loadBMP(Imageinputfile[0].c_str());
	Imageinputfile[1] = "earthmap1k.bmp";
	image[1] = loadBMP(Imageinputfile[1].c_str());
	Imageinputfile[2] = "sunmap.bmp";
	image[2] = loadBMP(Imageinputfile[2].c_str());
	Imageinputfile[3] = "moon.bmp";
	image[3] = loadBMP(Imageinputfile[3].c_str());
	Imageinputfile[4] = "hover_texture.bmp";
	image[4] = loadBMP(Imageinputfile[4].c_str());

	Imageinputfile[5] = ("earthnormal.bmp");
	normalEarth[0] = loadBMP(Imageinputfile[5].c_str());
	Imageinputfile[6] = ("earthspec.bmp");
	specularEarth[0] = loadBMP(Imageinputfile[6].c_str());

	cargaEsfera(shapes, materials);
	ret = tinyobj::LoadObj(shapePlane, materialsPlane, err, inputPlane.c_str(), basepath.c_str());
	if (!err.empty()) {
		std::cerr << err << std::endl;
	}
	cargaAvion(shapePlane, materialsPlane);

}



// ------------------------------------------------------------------------------------------
// Load all resources (shaders and geometry in this case)
// ------------------------------------------------------------------------------------------
void loadResources()
{
	// Carrega els shaders i crea el programa de shaders
	Shader simpleShader("shader.vert", "shader.frag");
	g_ShaderProgram = simpleShader.program;

	Shader texShader("shader_tex.vert","shader_tex.frag");
	g_ShaderProgram_stars = texShader.program;

	Shader specShader("shader_spec.vert", "shader_spec.frag");
	g_ShaderProgram_spec = specShader.program;
	// Crea la geomtria i desa-la al VAO
	creaGeometria();
}


// --------------------------------------------------------------
// GLUT callback detects window resize
// --------------------------------------------------------------
void onReshape(int w, int h)
{
	g_ViewportWidth = w;
	g_ViewportHeight = h;

	glViewport(0, 0, g_ViewportWidth, g_ViewportHeight);
}

void animation() {
	g_angle --;
	g_moon += 0.005f;
	g_earth += 0.001f;
	if (lock) {
		cameraPos = vec3(planeX, -2.0f, planeY);
	}

	if (front) {
		planeX -= distancePlane * sin(g_plane * M_PI / 180);
		planeY -= distancePlane * cos(g_plane * M_PI / 180);
	}
	else if (back) {
		planeX += distancePlane * sin(g_plane * M_PI / 180);
		planeY += distancePlane * cos(g_plane * M_PI / 180);
	}
	
	glutPostRedisplay();
}

void drawEarth()
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(g_ShaderProgram_spec);

	GLuint colorLoc = glGetUniformLocation(g_ShaderProgram_spec, "u_color");
	GLuint u_model = glGetUniformLocation(g_ShaderProgram_spec, "u_model");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram_spec, "u_projection");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram_spec, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_ShaderProgram_spec, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_ShaderProgram_spec, "u_texture");
	GLuint u_light_dir = glGetUniformLocation(g_ShaderProgram_spec, "u_light_dir");
	GLuint u_cam_pos = glGetUniformLocation(g_ShaderProgram_spec, "u_cam_pos");
	GLuint u_shininess = glGetUniformLocation(g_ShaderProgram_spec, "u_shininess");
	GLuint u_ambient = glGetUniformLocation(g_ShaderProgram_spec, "u_ambient");
	GLuint u_light_color = glGetUniformLocation(g_ShaderProgram_spec, "u_light_color");
	GLuint u_texture_normal = glGetUniformLocation(g_ShaderProgram_spec, "u_texture_normal");
	GLuint u_texture_spec = glGetUniformLocation(g_ShaderProgram_spec, "u_texture_spec");

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection_matrix = glm::perspective(
		60.0f, // Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f //far plane (distance from camera)
	);

	translateTierra = vec3(20 * cos(g_earth), 0, 20 * sin(g_earth));

	tierra = translate(sun, translateTierra);//::translate(sun, vec3(20.0f * cos(g_earth), 0.0f, 20.0f * sin(g_earth)));
	rotate_matrix = glm::rotate(mat4(), g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	model = tierra *rotate_matrix;

	mat3 normal_matrix = inverseTranspose((mat3(model)));

	light_pos = (translateTierra-sunTranslate);

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1i(u_texture, 0);
	glUniform1i(u_texture_normal, 1);
	glUniform1i(u_texture_spec, 2);
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(u_light_dir,-light_pos.x, -light_pos.y, -light_pos.z);
	glUniform3f(u_light_color, 1.0, 1.0, 1.0);
	glUniform3f(u_cam_pos, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(u_shininess, 50.0);
	glUniform1f(u_ambient, 0.1f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[1]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_earth_normal);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_earth_specular);

	glBindVertexArray(g_VaoArray[1]);
	glDrawElements(GL_TRIANGLES, g_NumTriangles * 3, GL_UNSIGNED_INT, 0);
}

void drawUnivers() 
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(g_ShaderProgram_stars);

	GLuint colorLoc = glGetUniformLocation(g_ShaderProgram_stars, "u_color");
	GLuint u_model = glGetUniformLocation(g_ShaderProgram_stars, "u_model");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram_stars, "u_projection");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram_stars, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_ShaderProgram_stars, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_ShaderProgram_stars, "u_texture");
	GLuint u_transparency = glGetUniformLocation(g_ShaderProgram_stars, "u_transparency");


	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection_matrix = glm::perspective(
		60.0f, // Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f //far plane (distance from camera)
	);

	scaled = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f));
	univers = glm::translate(mat4(), cameraPos);
	model = univers * scaled;

	mat3 normal_matrix = inverseTranspose((mat3(model)));

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1i(u_texture, 0);
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(u_transparency, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[0]);

	glCullFace(GL_FRONT);
	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(g_VaoArray[0]);
	glDrawElements(GL_TRIANGLES, g_NumTriangles * 3, GL_UNSIGNED_INT, 0);

	glEnable(GL_DEPTH_TEST);

}

void drawSun()
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glUseProgram(g_ShaderProgram_stars);

	GLuint colorLoc = glGetUniformLocation(g_ShaderProgram_stars, "u_color");
	GLuint u_model = glGetUniformLocation(g_ShaderProgram_stars, "u_model");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram_stars, "u_projection");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram_stars, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_ShaderProgram_stars, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_ShaderProgram_stars, "u_texture");
	GLuint u_transparency = glGetUniformLocation(g_ShaderProgram_stars, "u_transparency");


	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection_matrix = glm::perspective(
		60.0f, // Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f //far plane (distance from camera)
	);

	sun = glm::translate(mat4(1.0f), sunTranslate);
	scaled = glm::scale(glm::mat4(1.0f), glm::vec3(3.0f));
	model = sun * scaled;

	mat3 normal_matrix = inverseTranspose((mat3(model)));

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1i(u_texture, 0);
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(u_transparency, 1.0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[2]);

	glBindVertexArray(g_VaoArray[2]);
	glDrawElements(GL_TRIANGLES, g_NumTriangles * 3, GL_UNSIGNED_INT, 0);
}

void drawMoon()
{
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	glUseProgram(g_ShaderProgram);

	GLuint colorLoc = glGetUniformLocation(g_ShaderProgram, "u_color");
	GLuint u_model = glGetUniformLocation(g_ShaderProgram, "u_model");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram, "u_projection");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_ShaderProgram, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_ShaderProgram, "u_texture");
	GLuint u_light_dir = glGetUniformLocation(g_ShaderProgram, "u_light_dir");
	GLuint u_cam_pos = glGetUniformLocation(g_ShaderProgram, "u_cam_pos");
	GLuint u_shininess = glGetUniformLocation(g_ShaderProgram, "u_shininess");
	GLuint u_ambient = glGetUniformLocation(g_ShaderProgram, "u_ambient");
	GLuint u_light_color = glGetUniformLocation(g_ShaderProgram, "u_light_color");

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection_matrix = glm::perspective(
		60.0f, // Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f //far plane (distance from camera)
	);
	moontranslate = vec3(3.0f * cos(g_moon), 0.0f, 3.0f * sin(g_moon));
	rotate_matrix = glm::rotate(mat4(), g_angle, glm::vec3(0.0f, 1.0f, 0.0f));
	moon = glm::translate(tierra,moontranslate);
	scaled = glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
	model = moon * scaled * rotate_matrix;

	mat3 normal_matrix = inverseTranspose((mat3(model)));

	light_pos = moontranslate-sunTranslate;

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1i(u_texture, 0);
	glUniform3f(u_light_dir, 100, 100, 100);
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform3f(u_light_dir, light_pos.x, light_pos.y, -light_pos.z);
	glUniform3f(u_light_color, 1.0, 1.0, 1.0);
	glUniform3f(u_cam_pos, cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform1f(u_shininess, 50.0);
	glUniform1f(u_ambient, 0.1f);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[3]);

	glBindVertexArray(g_VaoArray[3]);
	glDrawElements(GL_TRIANGLES, g_NumTriangles * 3, GL_UNSIGNED_INT, 0);
}

void drawPlane() {
	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(g_ShaderProgram_stars);

	GLuint colorLoc = glGetUniformLocation(g_ShaderProgram_stars, "u_color");
	GLuint u_model = glGetUniformLocation(g_ShaderProgram_stars, "u_model");
	GLuint u_projection = glGetUniformLocation(g_ShaderProgram_stars, "u_projection");
	GLuint u_view = glGetUniformLocation(g_ShaderProgram_stars, "u_view");
	GLuint u_normal_matrix = glGetUniformLocation(g_ShaderProgram_stars, "u_normal_matrix");
	GLuint u_texture = glGetUniformLocation(g_ShaderProgram_stars, "u_texture");
	GLuint u_transparency = glGetUniformLocation(g_ShaderProgram_stars, "u_transparency");

	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glm::mat4 projection_matrix = glm::perspective(
		60.0f, // Field of view
		4.0f / 3.0f, // Aspect ratio
		0.1f, // near plane (distance from camera)
		100.0f //far plane (distance from camera)
	);

	plane = glm::translate(mat4(1.0f),vec3(planeX,-3.0f,planeY));
	scaled = glm::scale(mat4(1.0f), glm::vec3(0.05f));
	rotate_matrix = glm::rotate(mat4(), g_plane, vec3(0.0f, 1.0f, 0.0f));
	model = plane * scaled * rotate_matrix;

	mat3 normal_matrix = inverseTranspose((mat3(model)));

	glUniformMatrix4fv(u_projection, 1, GL_FALSE, glm::value_ptr(projection_matrix));
	glUniformMatrix4fv(u_view, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1i(u_texture, 0);
	glUniformMatrix3fv(u_normal_matrix, 1, GL_FALSE, glm::value_ptr(normal_matrix));
	glUniformMatrix4fv(u_model, 1, GL_FALSE, glm::value_ptr(model));
	glUniform1f(u_transparency, 0.5);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id[4]);

	glBindVertexArray(g_VaoArray[4]);
	glDrawElements(GL_TRIANGLES, g_NumTriangles * 3, GL_UNSIGNED_INT, 0);
}

void onDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);

	drawUnivers();
	drawEarth();
	drawSun();
	drawMoon();
	drawPlane();
	//clear everything
	glBindVertexArray(0);
	glUseProgram(0);

	// Swap the buffers so back buffer is on screen 
	glutSwapBuffers(); 
	glutPostRedisplay();
}

void eventKey(unsigned char key, int xx, int yy)
{
	GLfloat cameraSpeed = 1.0f;
	switch (key)
	{
		case 'i':
		{
			cameraPos += cameraSpeed * cameraFront;
			break;
		}

		case 'k':
		{
			cameraPos -= cameraSpeed * cameraFront;
			break;
		}
		case 'j':
		{
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break;
		}
		case 'l':
		{
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
			break; 
		}
		case 32: {
			glutWarpPointer(g_ViewportWidth / 2, g_ViewportHeight / 2);
			break;
		}
		case 'p':
		{
			if (lock){
				lock = false;
			}
			else {
				lock = true;
			}
		}
		case 'w':
			planeX -= distancePlane * sin(g_plane * M_PI / 180);
			planeY -= distancePlane * cos(g_plane * M_PI / 180);
			if (back && !front) {
				front = false;
				back = false;
			}
			else {
				front = true;
				back = false;
			}

			break;
		case 's':
			planeX += distancePlane * sin(g_plane * M_PI / 180);
			planeY += distancePlane * cos(g_plane * M_PI / 180);
			if (front) {
				front = false;
			}
			else {
				back = true;
			}
			break;
		case 'a':
			g_plane++;
			break;
		case 'd':
			g_plane--;
			break;
	}
}

void mouseUpdate(int x, int y) {
	glm::vec2 Mouse = glm::vec2(x, y);
	glm::vec2 mouseDelta = Mouse - oldMousePosition;
	if (glm::length(mouseDelta) > 10.0f) {
		oldMousePosition = Mouse;
		return;
	}
	else {
		glm::vec3 toRotateArund = glm::cross(cameraFront, cameraUp);

		rotateMatrix =
			glm::rotate(-mouseDelta.x * ROTATIONAL_SPEED, cameraUp) *
			glm::rotate(-mouseDelta.y * ROTATIONAL_SPEED, toRotateArund);

		cameraFront = glm::mat3(rotateMatrix) * cameraFront;
		oldMousePosition = Mouse;
		onDisplay();
	}
}


int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGBA  | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(g_ViewportWidth, g_ViewportHeight);
	glutCreateWindow("Univers!");

	GLenum glew_status = glewInit();
	if (glew_status != GLEW_OK){
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
		return EXIT_FAILURE;
	}
	
	loadResources();

	glClearColor(g_ColorFons.x, g_ColorFons.y, g_ColorFons.z, 1.0f );
	
	//this line is very important, as it tell GLUT what function to execute
	//every frame (which should be our draw function)
	glutDisplayFunc(onDisplay);
	glutIdleFunc(animation); 
	//tell GLUT about the keyboard and window resizing
	glutKeyboardFunc(eventKey);
	glutPassiveMotionFunc(mouseUpdate);
	glutReshapeFunc(onReshape);
	//start everything up
	glutMainLoop();

	return EXIT_SUCCESS;
}