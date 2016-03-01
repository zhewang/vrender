#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "vgl.h"
#include "LoadShaders.h"
#include "loadObj.h"

using namespace std;
using namespace glm;


vector<GLuint> VAOs;
vector<GLuint> VAO_Sizes;
GLuint uniformShader;

typedef struct {
    std::string filepath = "null";
    double rx = 0.0, ry = 0.0, rz = 0.0;
    double s[3] = {0.0}, t[3] = {0.0};
} Task;

////////////////////////////////////////////////////////////////////
//  init
////////////////////////////////////////////////////////////////////

void init(void)
{
    float bounds[6] = {0};
    GLuint vao;
    GLuint vao_size;
    int status = loadObjFile("./objFiles/sphere.obj", bounds, &vao, &vao_size);
    if(status == 0) {
        return;
    }

    VAOs.push_back(vao);
    VAO_Sizes.push_back(vao_size);

    ////////////////////////////////////////////////////////////////////
    // Load shaders
    ////////////////////////////////////////////////////////////////////

	ShaderInfo  shaders[] = {
		{ GL_VERTEX_SHADER, "uniform.vert" },
		{ GL_FRAGMENT_SHADER, "uniform.frag" },
		{ GL_NONE, NULL }
	};

	uniformShader = LoadShaders( shaders );

    ////////////////////////////////
    glm::mat4 p = glm::perspective(glm::radians(45.0f), 1.0f , 0.5f, 100.0f);

    float ranges[3] = {
        bounds[1]-bounds[0],
        bounds[3]-bounds[2],
        bounds[5]-bounds[4]
    };

    float midpoints[3] = {
        (bounds[0]+bounds[1])/2,
        (bounds[2]+bounds[3])/2,
        (bounds[4]+bounds[5])/2
    };

    for(int i = 0; i < 6; i ++) {
        cout << bounds[i] << ", ";
    }
    cout << endl;

    glm::mat4 v = glm::lookAt(
            glm::vec3(3*bounds[1], 3*bounds[3], 3*bounds[5]), // eye location
            glm::vec3(midpoints[0], midpoints[1], midpoints[2]), // center
            glm::vec3(0, 0, 1)  // up
            );

    glm::mat4 m = glm::scale(
            glm::mat4(1.0f),
            glm::vec3(1.0f));

    ////////////////////////////////

    GLint viewLoc = glGetUniformLocation(uniformShader, "View");
    GLint modelLoc = glGetUniformLocation(uniformShader, "Model");
    GLint projectLoc = glGetUniformLocation(uniformShader, "Project");

    glProgramUniformMatrix4fv(uniformShader, viewLoc, 1, false,  glm::value_ptr(v));
    glProgramUniformMatrix4fv(uniformShader, modelLoc, 1, false,  glm::value_ptr(m));
    glProgramUniformMatrix4fv(uniformShader, projectLoc, 1, false,  glm::value_ptr(p));

    glUseProgram(uniformShader);
}

void renderDisplay()
{
	glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, VAO_Sizes[0]);

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

void keyboardEvent(unsigned char key, int x, int y)
{
    switch (key)
    {
        //case 'c': changeColor(); break;
        case 's': displaySolidSurface(); break;
        case 'w': displayWirefram(); break;
        //case 'g': initEllipse(); break;
        //case 'e': initEpitrochoid(); break;
        //case 'x': TWOTRI_ON = !TWOTRI_ON; break;
        //case 'y': COLORTRI_ON = !COLORTRI_ON; break;
        //case 'z': ELLIPSE_ON = !ELLIPSE_ON; break;
        //case 'a': EPITROCHOID_ON = !EPITROCHOID_ON; break;
        case 'q': exit(0); break;
        case 27: exit(0); break;
    }
    renderDisplay();
}

void loadConfig(const char* fileName, vector<Task> &tasks) {
    std::ifstream infile(fileName);
    if(!infile.is_open()){
        std::cerr << "Configure file not exits." << std::endl;
        exit(0);
    }
    string word;
    Task current, ZERO;
    bool first = true;
    while (infile >> word)
    {
        if(word == "obj") {
            if( first == false) tasks.push_back(current);
            current = ZERO;
            infile >> current.filepath;
            first = false;
        }
        else if(word == "rx") { infile >> current.rx; }
        else if(word == "ry") { infile >> current.ry; }
        else if(word == "rz") { infile >> current.rz; }
        else if(word == "s") { infile >> current.s[0] >> current.s[1] >> current.s[2];}
        else if(word == "t") { infile >> current.t[0] >> current.t[1] >> current.t[2];}
    }
    tasks.push_back(current);

}


int main(int argc, char* argv[])
{
    // Parse arguments
    if(argc != 3 || strcmp(argv[1], "-c")) {
        std::cerr << "Usage: " << argv[0] << " -c CONFIG_FILE" << std::endl;
        exit(0);
    }

    // Load configure file
    vector<Task> tasks;
    loadConfig(argv[2], tasks);

    //vector<Task>::iterator it = tasks.begin();
    //while(it != tasks.end()) {
        //cout << "obj " << it->filepath << endl;
        //cout << "rx " << it->rx << " ry " << it->ry << " rz " << it->rz << endl;
        //cout << "s " << it->s[0] << " " << it->s[1] << " " << it->s[2] << endl;
        //cout << "t " << it->t[0] << " " << it->t[1] << " " << it->t[2] << endl;
        //it ++;
    //}

    // Init OpenGL
    glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize( 512, 512 );

	glutCreateWindow( argv[0] );

	glewExperimental = GL_TRUE;	// added for glew to work!

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

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
