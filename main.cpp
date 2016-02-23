#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "vgl.h"
#include "LoadShaders.h"

using namespace std;

enum VAO_IDs {TwoTriangles, ColorTriangle, Ellipse, Epitrochoid, NumVAOs};
enum Buffer_IDs {TwoTriBuf, ColorTriBuf_Loc, ColorTriBuf_Color, EllipseBuf, EpiBuf, NumBufs};
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBufs];

GLuint uniformShader, colorShader;

typedef struct {
    std::string filepath = "null";
    double rx = 0.0, ry = 0.0, rz = 0.0;
    double s[3] = {0.0}, t[3] = {0.0};
} Task;

////////////////////////////////////////////////////////////////////
//  init
////////////////////////////////////////////////////////////////////

void init (void )
{

    ////////////////////////////////////////////////////////////////////
    // Global initial
    ////////////////////////////////////////////////////////////////////
	glGenVertexArrays( NumVAOs, VAOs );
	glGenBuffers( NumBufs, Buffers);

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

	glBindVertexArray( VAOs[TwoTriangles] );

	glBindBuffer( GL_ARRAY_BUFFER, Buffers[TwoTriBuf]);
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
}

void renderDisplay()
{
	glClear( GL_COLOR_BUFFER_BIT );

    GLint colorLoc = glGetUniformLocation(uniformShader, "vColor");

    //if(TWOTRI_ON == true) drawTwoTriangles();
    //if(COLORTRI_ON == true) drawColorTriangle();
    //if(ELLIPSE_ON == true) drawEllipse();
    //if(EPITROCHOID_ON == true) drawEpitrochoid();

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
        //case 's': displaySolidSurface(); break;
        //case 'w': displayWirefram(); break;
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

    vector<Task> tasks;
    loadConfig("config.txt", tasks);

    vector<Task>::iterator it = tasks.begin();
    while(it != tasks.end()) {
        cout << "obj " << it->filepath << endl;
        cout << "rx " << it->rx << " ry " << it->ry << " rz " << it->rz << endl;
        cout << "s " << it->s[0] << " " << it->s[1] << " " << it->s[2] << endl;
        cout << "t " << it->t[0] << " " << it->t[1] << " " << it->t[2] << endl;
        it ++;
    }


    //init();
    //glutDisplayFunc(renderDisplay);
    //glutKeyboardFunc(keyboardEvent);

	//glutMainLoop();

	return 0;
}
