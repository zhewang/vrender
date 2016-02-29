#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "vgl.h"
#include "LoadShaders.h"

using namespace std;


vector<GLuint> VAOs;
GLuint uniformShader;

// Default Color
float customColor[3] = {0.0f, 0.0f, 1.0f};

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
    GLuint vao;
    GLuint buf;
	glGenVertexArrays(1, &vao);
    VAOs.push_back(vao);
	glGenBuffers(1, &buf);

    ////////////////////////////////////////////////////////////////////
    // Two Triangles
    ////////////////////////////////////////////////////////////////////
	GLfloat vertices[6][2] = {
		{ -0.90f, -0.9f },	// Triangle 1
		{  0.85f, -0.9f },
		{ -0.90f,  0.85f },
		{ 0.90f, -0.85f },	// Triangle 2
		{ 0.90f,  0.90f },
		{ -0.85f, 0.90f },
	};

	glBindVertexArray(vao);

	glBindBuffer( GL_ARRAY_BUFFER, buf);
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray( 0 );

    ////////////////////////////////////////////////////////////////////
    // Load shaders
    ////////////////////////////////////////////////////////////////////

	ShaderInfo  shaders[] = {
		{ GL_VERTEX_SHADER, "uniform.vert" },
		{ GL_FRAGMENT_SHADER, "uniform.frag" },
		{ GL_NONE, NULL }
	};

	uniformShader = LoadShaders( shaders );

    GLint colorLoc = glGetUniformLocation(uniformShader, "vColor");
    glProgramUniform3fv(uniformShader, colorLoc, 1, customColor);

    glUseProgram(uniformShader);
}

void renderDisplay()
{
	glClear( GL_COLOR_BUFFER_BIT );

    glBindVertexArray(VAOs[0]);
    glDrawArrays(GL_TRIANGLES, 0, 6);

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

    vector<Task>::iterator it = tasks.begin();
    while(it != tasks.end()) {
        cout << "obj " << it->filepath << endl;
        cout << "rx " << it->rx << " ry " << it->ry << " rz " << it->rz << endl;
        cout << "s " << it->s[0] << " " << it->s[1] << " " << it->s[2] << endl;
        cout << "t " << it->t[0] << " " << it->t[1] << " " << it->t[2] << endl;
        it ++;
    }

    // Init OpenGL
    glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA );
	glutInitWindowSize( 512, 512 );

	glutCreateWindow( argv[0] );

	glewExperimental = GL_TRUE;	// added for glew to work!

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