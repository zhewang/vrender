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
#include <GL/glui.h>

#define GLM_FORCE_RADIANS

using namespace std;
using namespace glm;


typedef struct {
    char op; // r:(x, y, z), s, t
    float x;
    float y;
    float z;
} Operation;

typedef struct {
    std::string filepath;
    std::string transfuncpath;
    std::vector<Operation> operations;
    int volume_x, volume_y, volume_z, pixel_bytes;
    int tff_size;
    bool hasView;
    float camPosition[3];
    float focalPoint[3];
    float viewUp[3];
} SceneConfig;

//GLUI const
const char* TransFuncNames[7] = {"bone.dat", "colorcube.dat",
                                 "copper.dat", "hsv.dat",
                                 "jet.dat", "tff.dat",
                                 "vga.dat"};
const GLuint GLUI_UPDATE_VIEW = 1;
const GLuint GLUI_UPDATE_COLORMAP = 2;
const GLuint GLUI_UPDATE_THRESH = 3;
const GLuint GLUI_UPDATE_SLICE = 4;

// Global Variables
SceneConfig config;

vector<GLuint> VAOs;
vector<GLuint> VAO_Sizes;
vector<glm::mat4> Models;

GLuint uniformShader;

glm::mat4 v; // View matrix
glm::mat4 p; // Projection matrix

glm::vec3 eye, center, up;
glm::vec3 eye_default, center_default, up_default;

float zx_rotate = 0;
float qe_rotate = 0;
float cv_rotation = 0;

GLuint tffTexObj;
GLuint volTexObj;

GLfloat thresh = 0.1f;

//GLUI variables
int main_window;
int gluiSliceCount = 1000;
vec3 gluiEye;
vec3 gluiRotation;
int lastColormapId;
int gluiColormapId = 0;
float gluiThreshold = 0.1f;

void myGlutIdle( void )
{
      if ( glutGetWindow() != main_window )
            glutSetWindow(main_window);
      glutPostRedisplay();
}

GLuint initTFF1DTex(const std::string filename, int tff_size)
{
    // read in the user defined data of transfer function
    ifstream inFile(filename, ifstream::in);
    if (!inFile)
    {
        cerr << "Error openning file: " << filename << endl;
        exit(EXIT_FAILURE);
    }
    
    const int MAX_CNT = tff_size*4+10; //4 channels: RGBA
    GLubyte *tff = (GLubyte *) calloc(MAX_CNT, sizeof(GLubyte));
    inFile.read(reinterpret_cast<char *>(tff), MAX_CNT);
    if (inFile.eof())
    {
        size_t bytecnt = inFile.gcount();
        *(tff + bytecnt) = '\0';
        //cout << "bytecnt " << bytecnt << endl;
    }
    else if(inFile.fail())
    {
        cout << filename << "read failed " << endl;
    }
    else
    {
        cout << filename << "is too large" << endl;
    }    
    GLuint tff1DTex;
    glGenTextures(1, &tff1DTex);
    glBindTexture(GL_TEXTURE_1D, tff1DTex);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, tff_size, 0, GL_RGBA, GL_UNSIGNED_BYTE, tff);
    free(tff);    
    return tff1DTex;
}

void normValue(GLubyte *data, GLuint pixelCount, GLuint pixelBits) {
    unsigned int min = 65536;
    unsigned int max = 0;
    unsigned char* ptr_char = data;
    unsigned short* ptr_short = (unsigned short *)data;
    for(int i = 0; i < pixelCount; i ++) {
        unsigned int temp;
        if(pixelBits == 8) temp = ptr_char[i];
        if(pixelBits == 16) temp = ptr_short[i];
        if(temp < min) min = temp;
        if(temp > max) max = temp;
    }
    for(int i = 0; i < pixelCount; i ++) {
        unsigned int temp;
        if(pixelBits == 8) temp = ptr_char[i];
        if(pixelBits == 16) temp = ptr_short[i];
        if(pixelBits == 8) {
            ptr_char[i] = (temp-min)*1.0/(max-min)*255;
        } else if(pixelBits == 16) {
            ptr_short[i] = (temp-min)*1.0/(max-min)*65535;
        }
    }
}

GLuint loadTexture(const std::string filename, GLuint w, GLuint h, GLuint d, GLuint b)
{
    GLuint tex;
    FILE *fp;
    size_t size = w * h * d * (b/8);
    GLubyte *data = new GLubyte[size];
    if (!(fp = fopen(filename.c_str(), "rb")))
    {
        cout << "Error: opening .raw file failed" << endl;
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << "OK: open .raw file successed" << endl;
    }
    if ( fread(data, sizeof(char), size, fp)!= size) 
    {
        cout << "Error: read .raw file failed" << endl;
        exit(1);
    }
    else
    {
        cout << "OK: read .raw file successed" << endl;
    }
    fclose(fp);

    glGenTextures(1, &tex);
    // bind 3D texture target
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    // pixel transfer happens here from client to OpenGL server
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    if(b == 8) {
        normValue(data, w*h*d, 8);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, w, h, d, 0, GL_RED, GL_UNSIGNED_BYTE,data);
    } else if (b == 16){
        normValue(data, w*h*d, 16);
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R16, w, h, d, 0, GL_RED, GL_UNSIGNED_SHORT,data);
    } else {
        cout << "Unsupported pixel bytes." << endl;
        exit(1);
    }

    delete []data;
    return tex;
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
    for(int i = config.operations.size()-1; i >= 0; i --) {
        Operation o = config.operations[i];
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
}


void init() {
    ////////////////////////////////////////////////////////////////////
    // Generate Slices
    ////////////////////////////////////////////////////////////////////
    int sliceCount = 1000;
    float sliceStep = 2.0f/(sliceCount-1);
    for(int i = 0; i < sliceCount; i ++) {
        initSlice(-1.0+i*sliceStep); // [-1,1]
    }

    ////////////////////////////////////////////////////////////////////
    // Load Texture
    ////////////////////////////////////////////////////////////////////
    tffTexObj = initTFF1DTex(config.transfuncpath, config.tff_size);
    volTexObj = loadTexture(config.filepath, 
                            config.volume_x, config.volume_y, config.volume_z,
                            config.pixel_bytes);

    //cout << tffTexObj << ", " << volTexObj << endl;


    ////////////////////////////////////////////////////////////////////
    // Calculate initial projection and view
    ////////////////////////////////////////////////////////////////////
    p = glm::perspective(glm::radians(45.0f), 1.0f, 0.5f, 100.0f);

    if(config.hasView == true) {
        eye_default = eye = vec3(config.camPosition[0], config.camPosition[1], config.camPosition[2]);
        center_default = center = vec3(config.focalPoint[0], config.focalPoint[1], config.focalPoint[2]);
        up_default = up = vec3(config.viewUp[0], config.viewUp[1], config.viewUp[2]);
    } else {
        eye_default = eye = glm::vec3(0,0,5);
        center_default = center = glm::vec3(0, 0, 0);
        up_default = up = glm::vec3(0, 1, 0);
    }

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

    GLint tffLoc = glGetUniformLocation(uniformShader, "TransferTex");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, tffTexObj);
    glUniform1i(tffLoc, 0);

    GLint volumeLoc = glGetUniformLocation(uniformShader, "VolumeTex");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, volTexObj);
    glUniform1i(volumeLoc, 1);

    GLint modelLoc = glGetUniformLocation(uniformShader, "Model");
    GLint viewLoc = glGetUniformLocation(uniformShader, "View");
    GLint projectLoc = glGetUniformLocation(uniformShader, "Project");
    GLint texRotateLoc = glGetUniformLocation(uniformShader, "texRotate");

    // Send View and Projection matices to shader
    v = glm::lookAt(eye, center, up);
    glProgramUniformMatrix4fv(uniformShader, viewLoc, 1, false,  glm::value_ptr(v));
    glProgramUniformMatrix4fv(uniformShader, projectLoc, 1, false,  glm::value_ptr(p));

    GLint threshLoc = glGetUniformLocation(uniformShader, "threshold");
    glUniform1f(threshLoc, thresh);

    glm::vec3 right = glm::normalize(glm::cross(eye-center, up));
    v = glm::rotate(glm::mat4(1.0f), glm::radians(zx_rotate), up);
    v = glm::rotate(v, glm::radians(qe_rotate), right);
    v = glm::rotate(v, glm::radians(cv_rotation), glm::normalize(center-eye));
    glProgramUniformMatrix4fv(uniformShader, texRotateLoc, 1, false,  glm::value_ptr(v));

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
    float rotate_step = 2.0;
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
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(-1.0f), right);
                //eye = center - gaze;
                qe_rotate -= rotate_step;
                break;
            }
        case 'e':
            {
                //vec3 gaze = center - eye;
                //gaze = glm::rotate(gaze, glm::radians(1.0f), right);
                //eye = center - gaze;
                qe_rotate += rotate_step;
                break;
            }
        case 'z':
            {
                //eye = glm::rotate(eye, glm::radians(1.0f), up);
                //center = glm::rotate(center, glm::radians(1.0f), up);
                zx_rotate -= rotate_step;
                break;
            }
        case 'x':
            {
                //eye = glm::rotate(eye, glm::radians(-1.0f), up);
                //center = glm::rotate(center, glm::radians(-1.0f), up);
                zx_rotate += rotate_step;
                break;
            }
        case 'c':
            {
                //up = glm::rotate(up, glm::radians(1.0f), glm::normalize(center-eye));
                cv_rotation += rotate_step;
                break;
            }
        case 'v':
            {
                //up = glm::rotate(up, glm::radians(-1.0f), glm::normalize(center-eye));
                cv_rotation -= rotate_step;
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
            zx_rotate = 0;
            qe_rotate = 0;
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

void loadConfig(const char* fileName, SceneConfig &config) {
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
            infile >> config.filepath;
        } else if (word == "view") {
            config.hasView = true;
            float cam[3], focal[3], up[3];
            infile >> word >> config.camPosition[0] >> config.camPosition[1] >> config.camPosition[2]
                   >> word >> config.focalPoint[0] >> config.focalPoint[1] >> config.focalPoint[2]
                   >> word >> config.viewUp[0] >> config.viewUp[1] >> config.viewUp[2];
        } else if (word == "transfunc") {
            infile >> config.transfuncpath;
        } else if(word == "volume_size") {
            infile >> config.volume_x >> config.volume_y >> config.volume_z >> config.pixel_bytes;
        } else if(word == "transfertex_size") {
            infile >> config.tff_size;
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
            config.operations.push_back(o);
        }
    }
}

void glui_cb(int control) {
    switch(control)
    {
        case GLUI_UPDATE_VIEW:
        {
            glm::mat4 m = glm::mat4(1.0f);
            eye = gluiEye;
            m = glm::rotate(m, glm::radians(gluiRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            m = glm::rotate(m, glm::radians(gluiRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            m = glm::rotate(m, glm::radians(gluiRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            for(int i =0; i < Models.size(); i++) {
                Models[i] = m;
            }
            break;
        }
        case GLUI_UPDATE_COLORMAP:
        {
            if( config.transfuncpath.find(TransFuncNames[lastColormapId]) != std::string::npos) {
                 int pos = config.transfuncpath.find(TransFuncNames[lastColormapId]);
                 config.transfuncpath = config.transfuncpath.replace(pos, config.transfuncpath.length() - pos, 
                                                                     TransFuncNames[gluiColormapId]);
                 tffTexObj = initTFF1DTex(config.transfuncpath, config.tff_size);
                 lastColormapId = gluiColormapId;

            } else  {
                cerr << "error occurred when updating colormap" << "\n";
            }
            break;
        }
        case GLUI_UPDATE_THRESH:
        {
            thresh = gluiThreshold;
            break;
        }
        case GLUI_UPDATE_SLICE:
        {
            VAOs.clear();
            VAO_Sizes.clear();
            float sliceStep = 2.0f/(gluiSliceCount - 1);
            for(int i = 0; i < gluiSliceCount; i ++) {
                initSlice(-1.0+i*sliceStep); // [-1,1]
            }
            break;
        }
    }
    glutPostRedisplay();
}

void setupGlui() {
    // GLUI code
    GLUI *glui = GLUI_Master.create_glui( "GLUI", 0, 512, 0);

    glui->add_statictext("View Control");
    glui->add_separator();

    //camera ui
    gluiEye = eye;
    GLUI_Panel *camera_panel = glui->add_panel( "Camera" );
    GLUI_EditText *edittext1 = glui->add_edittext_to_panel(camera_panel, "x:", GLUI_EDITTEXT_FLOAT, &gluiEye.x );
    GLUI_EditText *edittext2 = glui->add_edittext_to_panel(camera_panel, "y:", GLUI_EDITTEXT_FLOAT, &gluiEye.y );
    GLUI_EditText *edittext3 = glui->add_edittext_to_panel(camera_panel, "z:", GLUI_EDITTEXT_FLOAT, &gluiEye.z );
    edittext1->set_float_limits(-1000.f, 1000.0f );
    edittext2->set_float_limits(-1000.f, 1000.0f );
    edittext3->set_float_limits(-1000.f, 1000.0f );

    //rotation ui
    for(int i = config.operations.size()-1; i >= 0; i --) {
        Operation o = config.operations[i];
        if(o.op == 'x') {
            gluiRotation.x = o.x;
        } else if(o.op == 'y') {
            gluiRotation.x = o.y;
        } else if(o.op == 'z') {
            gluiRotation.z = o.z;
        }
    }
    GLUI_Panel *rotation_panel = glui->add_panel( "Rotation" );
    GLUI_EditText *edittext4 = glui->add_edittext_to_panel(rotation_panel, "x:", GLUI_EDITTEXT_FLOAT, &gluiRotation.x );
    GLUI_EditText *edittext5 = glui->add_edittext_to_panel(rotation_panel, "y:", GLUI_EDITTEXT_FLOAT, &gluiRotation.y );
    GLUI_EditText *edittext6 = glui->add_edittext_to_panel(rotation_panel, "z:", GLUI_EDITTEXT_FLOAT, &gluiRotation.z );
    edittext4->set_float_limits(0.0f, 360.0f );
    edittext5->set_float_limits(0.0f, 360.0f );
    edittext6->set_float_limits(0.0f, 360.0f );

    glui->add_button( "OK", GLUI_UPDATE_VIEW, (GLUI_Update_CB)glui_cb);

    glui->add_column(true);

    glui->add_statictext("Render Control");
    glui->add_separator();

    //colormap ui
    GLUI_Listbox *listbox = glui->add_listbox("Colormap: ", &gluiColormapId, GLUI_UPDATE_COLORMAP, (GLUI_Update_CB)glui_cb);
    for(int i = 0; i < sizeof(TransFuncNames)/sizeof(*TransFuncNames); i ++) {
        listbox->add_item(i, TransFuncNames[i]);
        if(config.transfuncpath.find(TransFuncNames[i])  != std::string::npos) {
            lastColormapId = i;
        }
    }
    listbox->set_int_val(lastColormapId);// set default colormap

    // threshold ui
    GLUI_Spinner *threshold_spinner =
    glui->add_spinner( "Threshold: ", GLUI_SPINNER_FLOAT, &gluiThreshold, GLUI_UPDATE_THRESH, (GLUI_Update_CB)glui_cb);
    threshold_spinner->set_speed(2);
    threshold_spinner->set_float_limits(0.0f, 1.0f);

    // slice ui
    GLUI_EditText *etSlice = glui->add_edittext("# of slice: ", GLUI_EDITTEXT_INT,
                                                &gluiSliceCount, GLUI_UPDATE_SLICE,
                                                (GLUI_Update_CB)glui_cb);
    glui->add_statictext( "   " );
    glui->add_statictext( "   " );
    glui->add_statictext( "   " );
    glui->add_button( "Quit", 0,(GLUI_Update_CB)exit );

    glui->set_main_gfx_window( main_window );
    GLUI_Master.set_glutIdleFunc( myGlutIdle );
}

int main(int argc, char* argv[])
{
    // Parse arguments
    if(argc != 3 || strcmp(argv[1], "-c")) {
        std::cerr << "Usage: " << argv[0] << " -c CONFIG_FILE" << std::endl;
        exit(0);
    }

    // Load configure file
    loadConfig(argv[2], config);

    // Init OpenGL
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize( 512, 512 );

    main_window =  glutCreateWindow( argv[0] );

    glewExperimental = GL_TRUE;	// added for glew to work!

    //glEnable(GL_DEPTH_TEST);
    //glDepthMask(GL_TRUE);

    glEnable( GL_ALPHA_TEST );
    glAlphaFunc( GL_GREATER, 0.3f );

    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glutReshapeFunc(Reshape);

    if ( glewInit() )
    {
        cerr << "Unable to initialize GLEW ... exiting" << endl;
        exit (EXIT_FAILURE );
    }

    init();
    glutDisplayFunc(renderDisplay);
    glutKeyboardFunc(keyboardEvent);
    setupGlui();

    glutMainLoop();

    return 0;
}
