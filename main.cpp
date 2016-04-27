#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <SOIL.h>

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

glm::mat4 v; // View matrix
glm::mat4 p; // Projection matrix

glm::vec3 eye, center, up;
glm::vec3 eye_default, center_default, up_default;

void loadTexture(char fullPath[128]) {
    if(strcmp(fullPath, "") != 0) {

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        int width, height, channels;
        unsigned char* image = SOIL_load_image(fullPath, &width, &height, &channels, SOIL_LOAD_AUTO);
        if(image == NULL) {
            cout << fullPath << endl;
            cout << "Load Texture Image Error.\n";
            exit(0);
        }

        //flip
        for (int j = 0; j * 2 < height; ++j)
        {
            int index1 = j * width * channels;
            int index2 = (height - 1 - j) * width * channels;
            for (int i = width * channels; i > 0; --i)
            {
                unsigned char temp = image[index1];
                image[index1] = image[index2];
                image[index2] = temp;
                ++index1;
                ++index2;
            }
        }

        if(channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                    GL_UNSIGNED_BYTE, image);
        } else if (channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                    GL_UNSIGNED_BYTE, image);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
        //tempObjList[i].diffuseTexMapID = tex;
        SOIL_free_image_data(image);

        glBindTexture(GL_TEXTURE_2D, tex);
        
    }
}

void initSlice(GLfloat z)
{
    ////////////////////////////////////////////////////////////////////
    // load each obj and calculate model matrix
    ////////////////////////////////////////////////////////////////////
    GLuint vao;
    GLuint vBuffer, vtBuffer;
    GLuint vao_size;

	GLfloat vertices[6][3] = {
		{-1.0f, -1.0f, z},
		{ 1.0f, -1.0f, z},
		{ 1.0f,  1.0f, z},
		{-1.0f,  1.0f, z},
		{-1.0f, -1.0f, z},
		{ 1.0f,  1.0f, z},
	};

    GLfloat vt_z = (z+1.0)/2.0;
	GLfloat vtexture[6][3] = {
		{0.0f, 0.0f, vt_z},
		{1.0f, 0.0f, vt_z},
		{1.0f, 1.0f, vt_z},
		{0.0f, 1.0f, vt_z},
		{0.0f, 0.0f, vt_z},
		{1.0f, 1.0f, vt_z},
	};
    vao_size = 6;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vBuffer);
    glGenBuffers(1, &vtBuffer);

	glBindVertexArray(vao);

	glBindBuffer( GL_ARRAY_BUFFER, vBuffer);
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindBuffer( GL_ARRAY_BUFFER, vtBuffer);
	glBufferData( GL_ARRAY_BUFFER, sizeof(vtexture), vtexture, GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

    VAOs.push_back(vao);
    VAO_Sizes.push_back(vao_size);

    // Apply transformation
    glm::mat4 m = glm::mat4(1.0f);
    Models.push_back(m);
}


void init() {
    ////////////////////////////////////////////////////////////////////
    // Generate Slices
    ////////////////////////////////////////////////////////////////////
    int sliceCount = 100;
    float sliceStep = 2.0f/sliceCount;
    for(int i = 0; i < sliceCount; i ++) {
        initSlice(-1.0+i*sliceStep); // [-1,1]
    }

    ////////////////////////////////////////////////////////////////////
    // Load Texture
    ////////////////////////////////////////////////////////////////////
    loadTexture("./mandrill.png");

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

void moveCamera(char cmd)
{
    glm::vec3 right = glm::normalize(glm::cross(eye-center, up));
    float speed = 0.1f;
    switch(cmd)
    {
        case 'w':
            {
                vec3 change = (center-eye)*0.01f;
                eye += change;
                center += change;
                break;
            }
        case 's':
            {
                vec3 change = (center-eye)*0.01f;
                eye -= change;
                center -= change;
                break;
            }
        case 'a':
            {
                float l = glm::length(center-eye);
                l = l/10;
                eye += right*l*speed;
                center += right*l*speed;
                break;
            }
        case 'd':
            {
                float l = glm::length(center-eye);
                l = l/10;
                eye -= right*l*speed;
                center -= right*l*speed;
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
                gaze = glm::rotate(gaze, glm::radians(-1.0f), right);
                //center = gaze + eye;
                eye = center - gaze;
                break;
            }
        case 'e':
            {
                vec3 gaze = center - eye;
                gaze = glm::rotate(gaze, glm::radians(1.0f), right);
                //center = gaze + eye;
                eye = center - gaze;
                break;
            }
        case 'z':
            {
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(-1.0f), up);
                //eye = center - gaze;
                eye = glm::rotate(eye, glm::radians(1.0f), up);
                center = glm::rotate(center, glm::radians(1.0f), up);
                break;
            }
        case 'x':
            {
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(1.0f), up);
                //eye = center - gaze;
                eye = glm::rotate(eye, glm::radians(-1.0f), up);
                center = glm::rotate(center, glm::radians(-1.0f), up);
                break;
            }
        case 'c':
            {
                up = glm::rotate(up, glm::radians(1.0f), glm::normalize(center-eye));
                break;
            }
        case 'v':
            {
                up = glm::rotate(up, glm::radians(-1.0f), glm::normalize(center-eye));
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
