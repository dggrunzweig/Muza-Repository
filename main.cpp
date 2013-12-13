//
//  main.cpp
//  Muza
//
//  Created by David Grunzweig on 11/7/13.
//
//
/*
 When using homebrew installed libraries:
 add "usr/local/lib" to library search paths and "usr/local/include" to header search paths
 */


//basic c stuff
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
//speech
#include <ApplicationServices/ApplicationServices.h>
//open gl and glut
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glutbitmap.h>
#include <GLUT/glutstroke.h>
//fluid synth
#include <fluidsynth.h>
//files
#include "RtAudio.h"
#include "chuck_fft.h"
#include "Thread.h"
#include "Stk.h"
#include "mfluidsynth.h"
#include "muza-globals.h"
#include "x-audio.h"
#include "raka-audio.h"
#include "muza-personality.h"




void initGfx();
void idleFunc();
void displayFunc();
void reshapeFunc( GLsizei width, GLsizei height );
void keyboardFunc( unsigned char, int, int );
void mouseFunc( int button, int state, int x, int y );


/*------------------------------------------
 global variables right hurr dawg
 -------------------------------------------*/
Globals Globals;
std::string inputText;
std::string outputText;
bool impatient = false;
int currentMood = 1;

// width and height
int g_width = 1024;
int g_height = 720;


float radiusWave;
float waveSpeed = .005;
float nextWaveSpeed = 0;
float diff = 0;

int cycleCounter = 0;
int lastInteractionTime = 0;
int numThoughts = 25;

/**
 * The <code>Muza</code> class
 * searches a line of input from a user for a term and then returns a response that matches the content of the term.
 * @author David Grunzweig
 * @version 1 11/6/2013
 */

struct thoughtDot{
    float x;
    float y;
    int xDirection;
    int yDirection;
    float deltaX;
    float deltaY;
    float r;
    float g;
    float b;
    float steps;
    float maxSteps;
    thoughtDot(int direction){
        if (direction == -1){
            xDirection = -1;
            x = 7 + 2*(double)rand()/RAND_MAX;
        } else {
            xDirection = 1;
            x = -7 - 2*(double)rand()/RAND_MAX;
        }
        y = -4 + 8*(double)(rand() % GLUT_WINDOW_HEIGHT)/(GLUT_WINDOW_HEIGHT);
        deltaX = .002;
        deltaY = .001;
        yDirection = -xDirection;
        r = (float)rand()/RAND_MAX;
        g = (float)rand()/RAND_MAX;
        b = (float)rand()/RAND_MAX;
        steps = 0;
        maxSteps = rand()%14000;
    };
    void move(int mood){
        updateColor(mood);
        x += (xDirection*deltaX);
        y += (yDirection*deltaY);
        glBegin(GL_TRIANGLES);
        glColor3f(r,g,b);
        glVertex3f(x, y, 0);
        glVertex3f(x+.1, y, 0);
        glVertex3f(x, y+.1, 0);
        glEnd();
        steps++;
        if (steps == maxSteps){
            yDirection = -yDirection;
        }
        if (x < -7){
            x = 7 + 2*(double)rand()/RAND_MAX;
            y = -4 + 8*(double)(rand() % GLUT_WINDOW_HEIGHT)/(GLUT_WINDOW_HEIGHT);
            steps = 0;
        } else if (x > 7){
            x = -7 - 2*(double)rand()/RAND_MAX;
            y = -4 + 8*(double)(rand() % GLUT_WINDOW_HEIGHT)/(GLUT_WINDOW_HEIGHT);
            steps = 0;
        }
    }
    void updateColor(int mood){
        switch (mood){
            case 6:
                r = 1;
                g = 1;
                b = 1;
                break;
            default:
                r = (float)mood/6;
                b = (float)(6-mood)/6;
                break;
        }
    }
    
};

struct mainFace{
    float x;
    float y;
    float r;
    float g;
    float b;
    float a;
    float ainc;
    mainFace(){
        x = 0;
        y = 0;
        r = .75;
        g = .1;
        b = 0;
        a = .1;
        ainc = .0005;
    }
    void renderFace(){
        a+=ainc;
        float x2,y2;
        float angle;
        double radius=1.5;
        glColor4f(r, g, b, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x,y);
        
        for (angle=1.0f;angle<361.0f;angle+=0.2)
        {
            x2 = x+sin(angle)*radius;
            y2 = y+cos(angle)*radius;
            glVertex2f(x2,y2);
        }
        
        glEnd();
        
//        float DEG2RAD = 3.14159/180;
//        
//        glLineWidth(1.5f);
//        glColor3f(0.0f, 0.0f, 0.0f);
//        glBegin(GL_LINE_LOOP);
//        for (float r = 0; r < radius; r+=(float)radius/5){
//            for (int i=0; i < 360; i++)
//            {
//                float degInRad = i*DEG2RAD;
//                glVertex2f(cos(degInRad)*r,sin(degInRad)*r);
//            }
//        }
//        
//        glEnd();
        
        if (a > 1){
            ainc = -ainc;
        } else if (a < 0.1){
            ainc = -ainc;
        }
        
    }
};

//create a vector of thoughtdots
vector<thoughtDot> thoughtDots;
mainFace face;

//-----------------------------------------------------------------------------
// Name: initGfx( )
// Desc: initiate the graphics
//-----------------------------------------------------------------------------

void initGfx()
{
    // double buffer, use rgb color, enable depth buffer
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    // initialize the window size
    g_width = glutGet(GLUT_WINDOW_WIDTH);
    g_height = glutGet(GLUT_WINDOW_HEIGHT);
    
    // set the window postion
    glutInitWindowPosition( 0, 0 );
    // create the window
    glutCreateWindow( "MUZA" );
    glutFullScreen();
    // set the idle function - called when idle
    glutIdleFunc( idleFunc );
    // set the display function - called when redrawing
    glutDisplayFunc( displayFunc );
    // set the reshape function - called when client area changes
    glutReshapeFunc( reshapeFunc );
    // set the keyboard function - called on keyboard events
    glutKeyboardFunc( keyboardFunc );
    // set the mouse function - called on mouse stuff
    glutMouseFunc( mouseFunc );
    
    // set clear color
    glClearColor( 0, 0, 0, 0 );
    // enable color material
    glEnable( GL_COLOR_MATERIAL );
    // enable depth test
    glEnable( GL_DEPTH_TEST );
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glDisable( GL_LIGHTING);

    
}

int main(int argc, char ** argv)
{
    
    // initialize GLUT
    glutInit(&argc, argv);
    // init gfx
    initGfx();
    
    for (int i = 0; i < 150; i++){
        thoughtDots.push_back(thoughtDot(0 - i%2));
    }
    
    // start real-time audio
    if( !raka_audio_init( SRATE, FRAMESIZE, NUMCHANNELS ) )
    {
        // error message
        cerr << "[MUZA]: cannot initialize real-time audio I/O..." << endl;
        return -1;
    }
    
    // start audio
    if( !raka_audio_start() )
    {
        // error message
        cerr << "[MUZA]: cannot start real-time audio I/O..." << endl;
        return -1;
    }
    
    //initialize the terms array, prompts array, and defaults array
    if ( !muzaInit()){
        cerr << "[MUZA]: cannot initialize the muza object..." << endl;
        return -1;
    }
    outputText = welcomeMessage();
    glutMainLoop();
    
    
    return 0;
}


//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc( GLsizei w, GLsizei h )
{
    // save the new window size
    g_width = w; g_height = h;
    // map the view port to the client area
    glViewport( 0, 0, w, h );
    // set the matrix mode to project
    glMatrixMode( GL_PROJECTION );
    // load the identity matrix
    glLoadIdentity( );
    // create the viewing frustum
    gluPerspective( 45.0, (GLfloat) w / (GLfloat) h, 1.0, 300.0 );
    // set the matrix mode to modelview
    glMatrixMode( GL_MODELVIEW );
    // load the identity matrix
    glLoadIdentity( );
    // position the view point
    gluLookAt( 0.0f, 0.0f, 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f );
    
}


//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc( )
{
    // render the scene
    glutPostRedisplay( );
}


void drawCircle(float radius)
{
    const float DEG2RAD = 3.14159/180;
    
    glBegin(GL_LINE_LOOP);
    
    for (int i=0; i < 360; i++)
    {
        float degInRad = i*DEG2RAD;
        glVertex2f(cos(degInRad)*radius,sin(degInRad)*radius);
    }
    
    glEnd();
}

void drawMuza(){
    glLineWidth(1.5);
    glPushMatrix();
    for (int i = 0; i < numThoughts; i++){
        
        thoughtDots.at(i).move(currentMood);
        
    }
    glPopMatrix();
}

void writeOutputToScreen(std::string text)
{
    glColor3f( 0.0f, 1.0f, 0.0f );
    glPushMatrix();
    glRasterPos2f(-5.0f, 3.0f);
    for (const char * p = text.c_str(); *p; p++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glPopMatrix();
    
}
void writeMoodToScreen()
{
    glColor3f( 0.0f, 1.0f, 0.0f );
    glPushMatrix();
    glRasterPos2f(4.0f, -3.0f);
    std::string text = "Current Mood: ";
    text.append(getMood());
    for (const char * p = text.c_str(); *p; p++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glPopMatrix();
    
}

void writeInputToScreen(std::string text)
{
    glColor3f( 0.0f, 1.0f, 0.0f );
    glPushMatrix();
    glRasterPos2f(-5.0f, 2.8f);
    for (int i = 0; i< text.length(); i++){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text.at(i));
    }
    glPopMatrix();
}


void incrementRadiusWave(){
    if (diff < 0 && waveSpeed > nextWaveSpeed){
        waveSpeed += diff;
    } else if (diff > 0 && waveSpeed < nextWaveSpeed){
        waveSpeed += diff;
    } else {
        diff = 0;
    }
    radiusWave += waveSpeed;
    
}

//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc( )
{
    
    
    // clear the color and depth buffers
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   
    //float radiusMod = sin(2*PI*radiusWave*PI/180);
    //float startAngle = 360* (1/sin(2*PI*radiusWave*PI/180));
    drawMuza();
    writeOutputToScreen(outputText);
    writeInputToScreen(inputText);
    writeMoodToScreen();
    face.renderFace();
    
    //incrementRadiusWave();
    cycleCounter++;
    if(cycleCounter==2500){
        cycleCounter = 0;
        lastInteractionTime++;
    }
    if (lastInteractionTime == 5){
        lastInteractionTime = 0;
        currentMood = 6;
        drawMuza();
        outputText = "I wrote you this song... what do you think?";
        impatience();
        
    }
    
    // flush!
    glFlush( );
    // swap the double buffer
    glutSwapBuffers();
}

//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc( int button, int state, int x, int y )
{

}




//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc( unsigned char key, int x, int y )
{
    switch( key )
    {
        case 'a':
        case 'A':
            inputText.append("A");
            //reshapeFunc(g_width, g_height);
            break;
        case 'b':
        case 'B':
            inputText.append("B");
            //reshapeFunc(g_width, g_height);
            break;
        case 'c':
        case 'C':
            inputText.append("C");
            //reshapeFunc(g_width, g_height);
            
            break;
        case 'd':
        case 'D':
            inputText.append("D");
            //reshapeFunc(g_width, g_height);
            break;
        case 'e':
        case 'E':
            inputText.append("E");
            //reshapeFunc(g_width, g_height);
            break;
        case 'f':
        case 'F':
            inputText.append("F");
            //reshapeFunc(g_width, g_height);
            break;
        case 'g':
        case 'G':
            inputText.append("G");
            //reshapeFunc(g_width, g_height);
            break;
        case 'h':
        case 'H':
            inputText.append("H");
            //reshapeFunc(g_width, g_height);
            break;
        case 'i':
        case 'I':
            inputText.append("I");
            //reshapeFunc(g_width, g_height);
            break;
        case 'j':
        case 'J':
            inputText.append("J");
            //reshapeFunc(g_width, g_height);
            break;
        case 'k':
        case 'K':
            inputText.append("K");
            //reshapeFunc(g_width, g_height);
            break;
        case 'l':
        case 'L':
            inputText.append("L");
            //reshapeFunc(g_width, g_height);
            break;
        case 'm':
        case 'M':
            inputText.append("M");
            //reshapeFunc(g_width, g_height);
            break;
        case 'n':
        case 'N':
            inputText.append("N");
            //reshapeFunc(g_width, g_height);
            break;
        case 'o':
        case 'O':
            inputText.append("O");
            //reshapeFunc(g_width, g_height);
            break;
        case 'p':
        case 'P':
            inputText.append("P");
            //reshapeFunc(g_width, g_height);
            break;
        case 'q':
        case 'Q':
            inputText.append("Q");
            //reshapeFunc(g_width, g_height);
            break;
        case 'r':
        case 'R':
            inputText.append("R");
            //reshapeFunc(g_width, g_height);

            break;
        case 's':
        case 'S':
            inputText.append("S");
            //reshapeFunc(g_width, g_height);
            break;
        case 't':
        case 'T':
            inputText.append("T");
            //reshapeFunc(g_width, g_height);
            break;
        case 'u':
        case 'U':
            inputText.append("U");
            //reshapeFunc(g_width, g_height);
            break;
        case 'v':
        case 'V':
            inputText.append("V");
            //reshapeFunc(g_width, g_height);
            break;
        case 'w':
        case 'W':
            inputText.append("W");
            //reshapeFunc(g_width, g_height);
            break;
        case 'x':
        case 'X':
            inputText.append("X");
            //reshapeFunc(g_width, g_height);
            break;
        case 'y':
        case 'Y':
            inputText.append("Y");
            //reshapeFunc(g_width, g_height);
            break;
        case 'z':
        case 'Z':
            inputText.append("Z");
            //reshapeFunc(g_width, g_height);
            break;
        case '1':
            inputText.append("1");
            //reshapeFunc(g_width, g_height);
            break;
        case '2':
            inputText.append("2");
            //reshapeFunc(g_width, g_height);
            break;
        case '3':
            inputText.append("3");
            //reshapeFunc(g_width, g_height);
            break;
        case '4':
            inputText.append("4");
            //reshapeFunc(g_width, g_height);
            break;
        case '5':
            inputText.append("5");
            //reshapeFunc(g_width, g_height);

            break;
        case '6':
            inputText.append("6");
            //reshapeFunc(g_width, g_height);
            break;
        case '7':
            inputText.append("7");
            //reshapeFunc(g_width, g_height);
            break;
        case '8':
            inputText.append("8");
            //reshapeFunc(g_width, g_height);
            break;
        case '9':
            inputText.append("9");
            //reshapeFunc(g_width, g_height);
            break;
        case '0':
            inputText.append("0");
            //reshapeFunc(g_width, g_height);
            break;
        case 39: //apostrophe
            inputText.append("'");
            //reshapeFunc(g_width, g_height);
            break;
        case 32: //space
            inputText.append(" ");
            //reshapeFunc(g_width, g_height);
            break;
        case 46: //period
            inputText.append(".");
            break;
        case 44: //comma
            inputText.append(",");
            break;
        case 33: // !
            inputText.append("!");
            break;
        case 63: // ?
            inputText.append("?");
            break;
        case 13: //return
            
            std::transform(inputText.begin(), inputText.end(), inputText.begin(), ::tolower);
            if (inputText != "quit" ) {
                outputText = "";
                outputText = getResponse(inputText);
                writeOutputToScreen(outputText);
                inputText = "";
                lastInteractionTime = 0;
                currentMood = getMoodIndex();
                switch (currentMood) {
                    case 1:
                        numThoughts = 50;
                        break;
                    case 2:
                        numThoughts = 30;
                        break;
                    case 3:
                        numThoughts = 40;
                        break;
                    case 4:
                        numThoughts = 70;
                        break;
                    case 5:
                        numThoughts = 150;
                        break;
                    case 6:
                        numThoughts = 10;
                        break;
                }
            } else {
                exit(1);
            };
            
            //reshapeFunc(g_width, g_height);
            break;
            
        case 8: //backspace
        case 127:
            inputText = inputText.substr(0, inputText.size()-1);
            //reshapeFunc(g_width, g_height);
            break;
        case 27: // escape
            exit(1);
            break;
    }
    
    glutPostRedisplay( );
}





