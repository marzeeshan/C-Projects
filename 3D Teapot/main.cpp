#include <iostream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <string>

using namespace std;

int CurrentWidth = 800,
CurrentHeight = 600,
WindowHandle = 0;

GLuint VertexShaderId, FragmentShaderId, ProgramId;
GLuint MatrixID;
glm::mat4 mpv;
GLuint vertexbuffer;

GLuint s_vertexLoc, s_colorLoc ;

GLfloat Colors[10752];
GLfloat Vertices[8064];

const GLchar* VertexShader =
{
	"#version 120\n"
	"attribute vec3 in_vertex;"
	"attribute vec4 in_color;"

	"uniform mat4 MVP;"
	"varying vec4 intp_color; "

	"void main(void) {"
	"intp_color = in_color; "
	"gl_Position = MVP * vec4(in_vertex, 1.0);"

	"}"
};

const GLchar* FragmentShader =
{
	"#version 120\n"
	"varying vec4 intp_color; "

	"void main(void) {"
	"gl_FragColor = vec4(intp_color);"
	"}"
};



void keyboard( unsigned char key, int x, int y);


void ResizeFunction(int, int);
void RenderFunction(void);
void IdleFunction(void);
void CreateShaders(void);


int totalTriangles;

vector<GLfloat> Vertice;
GLfloat testVertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
};

vector<GLfloat> VerticeNormal;


vector<GLfloat> Color;
GLfloat testColors[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f,  0.0f, 1.0f, 0.0f
};



string lineFromFile;
vector<string> lines;
int numberOfLine = 0;
void ReadFromFile(){
    ifstream myfile("Tris1.txt");
    if (myfile.is_open())
    {
        int i = 0;
        while ( getline (myfile,lineFromFile) )
        {
            lines.push_back(lineFromFile);
            i++;
            numberOfLine = i+1;
        }
        myfile.close();
    }
    else cout << "Unable to open file";
}


void importData(){
    vector<string> words;
    string str;
    stringstream ss;
    ss << lines.at(0);
    ss >> str;
    totalTriangles = stoi(str);

    for (int i = 2; i < numberOfLine; i++){
        int posNo = 0;
        stringstream sst;
        sst << lines[i];
        while (sst >> str){
            if (posNo < 3){
                Vertice.push_back(stof(str));
                posNo = posNo + 1;
                
            }
			else if (posNo < 6 && posNo >= 3) {
				VerticeNormal.push_back(stof(str));
				posNo = posNo + 1;

			}
            else if (posNo >= 6){
                Color.push_back(stof(str));
                posNo = posNo + 1;
            }
        }
    }

}

glm::mat4 MVPComputing1(){

    glm::mat4 Projection = glm::ortho(-2.4f,2.4f,-1.8f,1.8f,1.0f,50.0f); 
    

    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(10,10,10), 
                                       glm::vec3(0,0,0), 
                                       glm::vec3(0,1,0)  
                                       );
    
    glm::mat4 Model      = glm::mat4(1.0f);
   
    glm::mat4 mvp        = Projection * View * Model;
	return mvp;
}

glm::mat4 MVPComputing2(){
    
	glm::mat4 Projection = glm::perspective(glm::radians(50.0f), 4.0f / 3.0f, 1.0f, 50.0f);
    
   
    glm::mat4 View       = glm::lookAt(
                                       glm::vec3(3,3,3), 
                                       glm::vec3(0,0,0), 
                                       glm::vec3(0,1,0)  
                                       );
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::rotate(Model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, -1.0f, 0.0f));
	
	glm::mat4 mvp = Projection * View * Model; 
	return mvp;
}

glm::mat4 MVPComputing3() {
	glm::mat4 Projection = glm::ortho(-2.4f, 2.4f, -1.8f, 1.8f, 1.0f, 50.0f);

																			 
	glm::mat4 View = glm::lookAt(
		glm::vec3(10, 10, 10), 
		glm::vec3(0, 0, 0), 
		glm::vec3(0, 1, 0)  
	);
	
	glm::mat4 Model = glm::mat4(1.0f);
	
	glm::mat4 mvp = Projection * View * Model; 
	return mvp;
}

glm::mat4 MVPComputing4() {
	
	glm::mat4 Projection = glm::perspective(glm::radians(50.0f), 4.0f / 3.0f, 1.0f, 50.0f);

	
	glm::mat4 View = glm::lookAt(
		glm::vec3(3, 3, 3), 
		glm::vec3(0, 0, 0), 
		glm::vec3(0, 1, 0)  
	);
	glm::mat4 Model = glm::mat4(1.0f);
	Model = glm::rotate(Model, glm::radians(45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	Model = glm::translate(Model, glm::vec3(0.0f, -1.0f, 0.0f));
	
	glm::mat4 mvp = Projection * View * Model; 
	return mvp;
}



int main(int argc, char* argv[])
{
    ReadFromFile();
    importData();
    
    for (int i = 0; i < 8064; i++){
        Vertices[i] = Vertice[i];
    }
    
    
    for (int i = 0; i < 10752; i++){
        Colors[i] = Color[i];
    }
    
    glutInit(&argc, argv);
    glutInitWindowSize(CurrentWidth, CurrentHeight);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    
    WindowHandle = glutCreateWindow("Mohammed_Zeeshan_HW4");
    
    glutReshapeFunc(ResizeFunction);
    glutDisplayFunc(RenderFunction);
    glutIdleFunc(IdleFunction);
    
   
    GLenum GlewInitResult;
    GlewInitResult = glewInit();
    
    if (GLEW_OK != GlewInitResult)
        exit(EXIT_FAILURE);
    
    
    CreateShaders();
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    glutMainLoop();
    
    
    exit(EXIT_SUCCESS);
}


void ResizeFunction(int Width, int Height)
{
    CurrentWidth = Width;
    CurrentHeight = Height;
    glViewport( 0, 0, CurrentWidth, CurrentHeight );
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
}

void RenderFunction(void)
{
    glUseProgram(ProgramId);
   
    GLuint MatrixID = glGetUniformLocation(ProgramId, "MVP");
    
    glm::mat4 MVP = mpv;
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable (GL_DEPTH_TEST);
    
    
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    
    glEnableVertexAttribArray(s_vertexLoc);
    glEnableVertexAttribArray(s_colorLoc);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    
    glVertexAttribPointer(s_vertexLoc, 3, GL_FLOAT, GL_FALSE, 0, Vertices);
    glVertexAttribPointer(s_colorLoc, 4, GL_FLOAT, GL_FALSE, 0, Colors);
    
    
    glDrawArrays(GL_TRIANGLES, 0, 2688);
    
    glDisableVertexAttribArray(s_vertexLoc);
    glDisableVertexAttribArray(s_colorLoc);
    
    glutSwapBuffers();
    glutKeyboardFunc(keyboard);
}

void IdleFunction(void)
{
    glutPostRedisplay();
}


void printLog(GLuint obj)
{
    int infologLength = 0;
    int maxLength;
    
    if( glIsShader( obj ) )
        glGetShaderiv( obj , GL_INFO_LOG_LENGTH , &maxLength );
    else
        glGetProgramiv( obj, GL_INFO_LOG_LENGTH, &maxLength);
    
    char infoLog[1255];
    
    if ( glIsShader(obj) )
        glGetShaderInfoLog( obj, maxLength, &infologLength, infoLog );
    else
        glGetProgramInfoLog( obj, maxLength, &infologLength, infoLog );
    
    if ( infologLength > 0 )
        printf( "\n Error detail: %s\n" , infoLog );
}


void CreateShaders(void)
{
    if( glCreateShader )
        printf(" ---- shader suppot ok ---");
    else
    {
        printf(" ---- no shader support ---");
        return ;
    }
   
    VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
    glCompileShader(VertexShaderId);
    printLog(VertexShaderId);
    

    FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
    glCompileShader(FragmentShaderId);
    printLog(FragmentShaderId);
    
   
    ProgramId = glCreateProgram();
    glAttachShader( ProgramId, VertexShaderId);
    glAttachShader( ProgramId, FragmentShaderId);
    glLinkProgram(ProgramId);
    printLog(ProgramId);
    
    glUseProgram(ProgramId);
    
    s_vertexLoc = glGetAttribLocation(ProgramId, "in_vertex");
    s_colorLoc = glGetAttribLocation(ProgramId, "in_color");
}

void keyboard( unsigned char key, int x, int y){
    if (key == '1'){
        mpv = MVPComputing1();
    }
    if (key == '2'){
        mpv = MVPComputing2();
    }
    if (key == '3'){
        mpv = MVPComputing3();
    }
    if (key == '4'){
        mpv = MVPComputing4();
    }
}
