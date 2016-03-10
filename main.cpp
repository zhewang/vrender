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


void initObjModel(Task t, float bounds[6])
{
    ////////////////////////////////////////////////////////////////////
    // load each obj and calculate model matrix
    ////////////////////////////////////////////////////////////////////
    GLuint vao;
    GLuint vao_size;

    char f_cstr[1000];
    strcpy(f_cstr, t.filepath.c_str());
    float tempbounds[6];

    int status = loadObjFile(f_cstr, tempbounds, &vao, &vao_size);
    if(status == 0) {
        return;
    }

    VAOs.push_back(vao);
    VAO_Sizes.push_back(vao_size);

    // Apply transformation
    glm::mat4 m = glm::mat4(1.0f);
    for(int i = t.operations.size()-1; i >= 0; i --) {
        Operation o = t.operations[i];
        if(o.op == 's') {
            m = glm::scale(m, glm::vec3(o.x, o.y, o.z));
        } else if(o.op == 't') {
            m = glm::translate(m, glm::vec3(o.x, o.y, o.z));
        } else if(o.op == 'x') {
            m = glm::rotate(m, glm::radians(o.x), glm::vec3(1.0f, 0.0f, 0.0f));
        } else if(o.op == 'y') {
            m = glm::rotate(m, glm::radians(o.y), glm::vec3(0.0f, 1.0f, 0.0f));
        } else if(o.op == 'z') {
            m = glm::rotate(m, glm::radians(o.z), glm::vec3(0.0f, 0.0f, 1.0f));
        }
    }

    Models.push_back(m);

    // Update bounds
    glm::vec4 min = glm::vec4(tempbounds[0], tempbounds[2], tempbounds[4], 1);
    glm::vec4 max = glm::vec4(tempbounds[1], tempbounds[3], tempbounds[5], 1);
    min = m*min;
    max = m*max;
    bounds[0] = min[0] < bounds[0] ? min[0]:bounds[0];
    bounds[2] = min[1] < bounds[2] ? min[1]:bounds[2];
    bounds[4] = min[2] < bounds[4] ? min[2]:bounds[4];
    bounds[1] = max[0] > bounds[1] ? max[0]:bounds[1];
    bounds[3] = max[1] > bounds[3] ? max[1]:bounds[3];
    bounds[5] = max[2] > bounds[5] ? max[2]:bounds[5];
}


void init(std::vector<Task> tasks) {
    float MIN = -999999999, MAX = 999999999;
    SceneBounds[0] = SceneBounds[2] = SceneBounds[4] = MAX;
    SceneBounds[1] = SceneBounds[3] = SceneBounds[5] = MIN;
    vector<Task>::iterator it = tasks.begin();
    while(it != tasks.end()) {
        initObjModel(*it, SceneBounds);
        it ++;
    }

    ////////////////////////////////////////////////////////////////////
    // Calculate initial projection and view
    ////////////////////////////////////////////////////////////////////
    p = glm::perspective(glm::radians(45.0f), 1.0f, 0.5f, 100.0f);

    float midpoints[3] = {
        (SceneBounds[0]+SceneBounds[1])/2,
        (SceneBounds[2]+SceneBounds[3])/2,
        (SceneBounds[4]+SceneBounds[5])/2
    };

    eye_default = eye = glm::vec3(3*SceneBounds[1], 3*SceneBounds[3], SceneBounds[5]);
    center_default = center = glm::vec3(midpoints[0], midpoints[1], midpoints[2]);
    up_default = up = glm::vec3(0, 0, 1);

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

void loadConfig(const char* fileName, vector<Task> &tasks) {
    std::ifstream infile(fileName);
    if(!infile.is_open()){
        std::cerr << "Configure file not exits." << std::endl;
        exit(0);
    }
    string word;
    bool first = true;
    while (infile >> word)
    {
        if(word == "obj") {
            tasks.push_back(Task());
            infile >> tasks.back().filepath;
        }
        else {
            Operation o;
            if(word == "rx") {
                o.op = 'x';
                infile >> o.x;
            }
            else if(word == "ry") {
                o.op = 'y';
                infile >> o.y;
            }
            else if(word == "rz") {
                o.op = 'z';
                infile >> o.z;
            }
            else if(word == "s") {
                o.op = 's';
                infile >> o.x >> o.y >> o.z;
            }
            else if(word == "t") {
                o.op = 't';
                infile >> o.x >> o.y >> o.z;
            }
            tasks.back().operations.push_back(o);
        }
    }
}

void Reshape(int w, int h) {
    glViewport(0, 0, w, h);
    p = glm::perspective(glm::radians(45.0f), (float)w*1.0f/h, 0.5f, 100.0f);
    glutPostRedisplay();
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

    init(tasks);
    glutDisplayFunc(renderDisplay);
    glutKeyboardFunc(keyboardEvent);

    glutMainLoop();

    return 0;
}
