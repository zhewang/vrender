#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "vgl.h"
#include "LoadShaders.h"
#include "loadObj.h"

#define GLM_FORCE_RADIANS

using namespace std;
using namespace glm;


vector<GLuint> VAOs;
vector<GLuint> VAO_Sizes;
vector<glm::mat4> Models;
GLuint uniformShader;

float SceneBounds[6];

glm::mat4 v; // View matrix
glm::mat4 p; // Projection matrix

glm::vec3 eye, center, up;
glm::vec3 eye_default, center_default, up_default;

bool SOLID = true;

typedef struct {
    char op; // r:(x, y, z), s, t
    float x;
    float y;
    float z;
} Operation;

typedef struct {
    std::string filepath;
    std::vector<Operation> operations;
} Task;

void printMat4(mat4 m) {
    for(int i = 0; i < 4; i ++){
        for(int j = 0; j < 4; j ++) {
            cout << m[i][j] << " ";
        }
        cout << endl;
    }
}


void initSlice(GLfloat z)
{
    ////////////////////////////////////////////////////////////////////
    // load each obj and calculate model matrix
    ////////////////////////////////////////////////////////////////////
    GLuint vao;
    GLuint buffer;
    GLuint vao_size;

	GLfloat vertices[6][3] = {
		{ -1.0f, -1.0f, z},
		{  1.0f, -1.0f, z},
		{  1.0f,  1.0f, z},
		{ -1.0f,  1.0f, z},
		{ -1.0f, -1.0f, z},
		{  1.0f,  1.0f, z},
	};

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &buffer);
    vao_size = 6;

	glBindVertexArray(vao);
	glBindBuffer( GL_ARRAY_BUFFER, buffer);

	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray( 0 );

    VAOs.push_back(vao);
    VAO_Sizes.push_back(vao_size);

    // Apply transformation
    glm::mat4 m = glm::mat4(1.0f);
    Models.push_back(m);
}


void init() {
    int sliceCount = 10;
    float sliceStep = 2.0f/sliceCount;
    for(int i = 0; i < sliceCount; i ++) {
        initSlice(-1.0+i*sliceStep); // [-1,1]
    }

    ////////////////////////////////////////////////////////////////////
    // Calculate initial projection and view
    ////////////////////////////////////////////////////////////////////
    p = glm::perspective(glm::radians(45.0f), 1.0f, 0.5f, 100.0f);

    eye_default = eye = glm::vec3(0,0,5);
    center_default = center = glm::vec3(0, 0, 0);
    up_default = up = glm::vec3(0, 1, 0);

    v = glm::lookAt(eye, center, up);

    ////////////////////////////////////////////////////////////////////
    // Load shaders
    ////////////////////////////////////////////////////////////////////

    ShaderInfo  shaders[] = {
        { GL_VERTEX_SHADER, "uniform.vert" },
        { GL_FRAGMENT_SHADER, "uniform.frag" },
        { GL_NONE, NULL }
    };

    uniformShader = LoadShaders( shaders );

    glUseProgram(uniformShader);
}


void renderDisplay()
{
    glClearColor(0, 43/255.0, 54/255.0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    GLint modelLoc = glGetUniformLocation(uniformShader, "Model");
    GLint viewLoc = glGetUniformLocation(uniformShader, "View");
    GLint projectLoc = glGetUniformLocation(uniformShader, "Project");

    // Send View and Projection matices to shader
    v = glm::lookAt(eye, center, up);
    glProgramUniformMatrix4fv(uniformShader, viewLoc, 1, false,  glm::value_ptr(v));
    glProgramUniformMatrix4fv(uniformShader, projectLoc, 1, false,  glm::value_ptr(p));

    for(int i = 0; i < VAOs.size(); i ++) {
        glProgramUniformMatrix4fv(uniformShader, modelLoc, 1, false,  glm::value_ptr(Models[i]));

        glBindVertexArray(VAOs[i]);
        glDrawArrays(GL_TRIANGLES, 0, VAO_Sizes[i]);
    }

    glFlush();
}

////////////////////////////////////////////////////////////////////////
//  main
////////////////////////////////////////////////////////////////////////

void displaySolidSurface()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glutPostRedisplay();
}

void displayWirefram()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(5.0);
    glutPostRedisplay();
}

void moveCamera(char cmd)
{
    glm::vec3 right = glm::normalize(glm::cross(eye-center, up));
    float speed = 0.3f;
    switch(cmd)
    {
        case 'w':
            {
                vec3 change = glm::normalize(center-eye)*speed;
                eye += change;
                center += change;
                break;
            }
        case 's':
            {
                vec3 change = glm::normalize(center-eye)*speed;
                eye -= change;
                center -= change;
                break;
            }
        case 'a':
            {
                eye += right*speed;
                center += right*speed;
                break;
            }
        case 'd':
            {
                eye -= right*speed;
                center -= right*speed;
                break;
            }
        case 'r':
            {
                eye += glm::normalize(up)*speed;
                center += glm::normalize(up)*speed;
                break;
            }
        case 't':
            {
                eye -= glm::normalize(up)*speed;
                center -= glm::normalize(up)*speed;
                break;
            }
        case 'q':
            {
                vec3 gaze = center - eye;
                gaze = glm::rotate(gaze, glm::radians(1.0f), right);
                center = gaze + eye;
                break;
            }
        case 'e':
            {
                vec3 gaze = center - eye;
                gaze = glm::rotate(gaze, glm::radians(-1.0f), right);
                center = gaze + eye;
                break;
            }
        case 'z':
            {
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(-1.0f), up);
                //eye = center - gaze;
                eye = glm::rotate(eye, glm::radians(-1.0f), up);
                center = glm::rotate(center, glm::radians(-1.0f), up);
                break;
            }
        case 'x':
            {
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(1.0f), up);
                //eye = center - gaze;
                eye = glm::rotate(eye, glm::radians(1.0f), up);
                center = glm::rotate(center, glm::radians(1.0f), up);
                break;
            }
        case 'c':
            {
                up = glm::rotate(up, glm::radians(-1.0f), glm::normalize(center-eye));
                break;
            }
        case 'v':
            {
                up = glm::rotate(up, glm::radians(1.0f), glm::normalize(center-eye));
                break;
            }
        case 'j':
            {
                cout << "Please enter x, y, z for camera position:" << endl;
                float x, y, z;
                cin >> x >> y >> z;
                eye = vec3(x,y,z);
                break;
            }
        case 'k':
            {
                cout << "Please enter x, y, z for focal point:" << endl;
                float x, y, z;
                cin >> x >> y >> z;
                center = vec3(x,y,z);
                break;
            }
        case 'l':
            {
                cout << "Please enter x, y, z for view up vector:" << endl;
                float x, y, z;
                cin >> x >> y >> z;
                up = glm::normalize(vec3(x,y,z));
                break;
            }
    }
}

void keyboardEvent(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'p':
            eye = eye_default;
            center = center_default;
            up = up_default;
            break;
        case 'o':
            if(SOLID) {
                displayWirefram();
                SOLID = !SOLID;
            } else {
                displaySolidSurface();
                SOLID = !SOLID;
            }
            break;
        case 27: exit(0); break;
        default:
                 moveCamera(key); break;
    }
    glutPostRedisplay();
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    p = glm::perspective(glm::radians(45.0f), (float)w*1.0f/h, 0.5f, 100.0f);
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    // Parse arguments
    //if(argc != 3 || strcmp(argv[1], "-c")) {
        //std::cerr << "Usage: " << argv[0] << " -c CONFIG_FILE" << std::endl;
        //exit(0);
    //}

    // Load configure file
    //vector<Task> tasks;
    //loadConfig(argv[2], tasks);

    // Init OpenGL
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize( 512, 512 );

    glutCreateWindow( argv[0] );

    glewExperimental = GL_TRUE;	// added for glew to work!

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glutReshapeFunc(Reshape);

    if ( glewInit() )
    {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit (EXIT_FAILURE );
    }

    init();
    glutDisplayFunc(renderDisplay);
    glutKeyboardFunc(keyboardEvent);

    glutMainLoop();

    return 0;
}
