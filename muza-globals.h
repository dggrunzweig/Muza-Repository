//-----------------------------------------------------------------------------
// name: raka-globals.h
// desc: global stuff for bokeh visualization
//
// author: Ge Wang (ge@ccrma.stanford.edu)
//   date: 2013
//-----------------------------------------------------------------------------
#ifndef __MUZA_GLOBALS_H__
#define __MUZA_GLOBALS_H__

//open gl and glut
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glutbitmap.h>
#include <GLUT/glutstroke.h>

// c++
#include <string>
#include <map>
#include <vector>
#include <utility>



// defines
//#define SRATE        44100
#define FRAMESIZE    1024
#define NUMCHANNELS  2
#define MAX_TEXTURES 32
#define SAMPLE double





// forward reference
//class RAKASim;




//-----------------------------------------------------------------------------
// name: class Globals
// desc: the global class
//-----------------------------------------------------------------------------
class Globals
{
public:
    // top level root simulation
    //static RAKASim * sim;
    
    // path
    static std::string path;
    // path to datapath
    static std::string relpath;
    // datapath
    static std::string datapath;
    // version
    static std::string version;
    
    // last audio buffer
    static SAMPLE * lastAudioBuffer;
    static SAMPLE * lastAudioBufferMono;
    static SAMPLE * audioBufferWindow;
    static unsigned int lastAudioBufferFrames;
    static unsigned int lastAudioBufferChannels;
    
    // width and height of the window
    static GLsizei windowWidth;
    static GLsizei windowHeight;
    static GLsizei lastWindowWidth;
    static GLsizei lastWindowHeight;
    

    
    // waveform
    //static YWaveform * waveform;
    
};




#endif
