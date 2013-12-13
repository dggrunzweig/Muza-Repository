//-----------------------------------------------------------------------------
// name: raka-audio.cpp
// desc: audio stuff for bokeh
//
// author: Ge Wang (ge@ccrma.stanford.edu)
//   date: 2013
//-----------------------------------------------------------------------------
#include "raka-audio.h"
#include "muza-globals.h"
#include "Thread.h"
#include "mfluidsynth.h"
#include "x-audio.h"
#include <iostream>
using namespace std;



// globals
mFluidSynth * g_synth;
//mFluidSynth * g_synth2;
double g_now;
double g_nextTime;
int g_prog = 0;




// basic note struct
struct Note
{
    int channel;
    float pitch;
    float velocity;
    float duration; // in seconds
    // add more stuff?

    // constructor
    Note( int c, float p, float v, float d )
    {
        channel = c;
        pitch = p;
        velocity = v;
        duration = d;
    }
};

int majorPentatonic[] = {-12, -10, -8, -5, -3, 0, 2, 4, 7, 9, 12, 14, 16, 19, 24};
int minorPentatonic[] = {-12, -9, -7, -5, -2, 0, 3, 5, 7, 10, 12, 15, 17, 19, 22, 24};
int ionian[] = {-12, -10, -8, -7, -5, -3, -1, 0, 2, 4, 5, 7, 9, 11, 12};
int dorian[] = {-12, -10, -9, -7, -5, -3, -2, 0, 2, 3, 5, 7, 9, 10, 12};
int phrygian[] = {-12, -11, -9, -7, -5, -4, -2, 0, 1, 3, 5, 7, 8, 10, 12};
int lydian[] = {-12, -10, -8, -6, -5, -3, -1, 0, 2, 4, 6, 7, 9, 11, 12};
int mixolydian[] = {-12, -10, -8, -7, -5, -3, -2, 0, 2, 4, 5, 7, 9, 10, 12};
int aeolian[] = {-12, -10, -9, -7, -4, -2, 0, 2,3,5,7,8,10,12};
int locrian[] = {-12, -11, -9, -7, -6, -4, -2, 0,1,3,5,6,8,10,12};

// HACK: vector of notes
vector<Note> g_notes;
int g_noteIndex = 0;
Mutex g_mutex;

// play some notes
void raka_playNotes( float pitch, float velocity, int chordType )
{
    // lock
    g_mutex.lock();
    // clear notes
    g_notes.clear();
    switch (chordType) {
        case 2://ionian
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = ionian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
        case 1: //dorian
            pitch = pitch + 2; //second
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = dorian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
        case 6: //phrygian
            pitch = pitch + 4; //phrygian
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = phrygian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
//        case : //lydian
//            pitch = pitch + 6;
//            g_notes.push_back(Note(1, pitch, .75, 1));
//            for (int i = 0; i < 50; i++){
//                int index = rand() % 15;
//                int nextNote = lydian[index];
//                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
//            }
//            g_notes.push_back(Note(1, pitch, .75, 1));
//            break;
        case 3: //mixolydian
            pitch = pitch + 7;
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = mixolydian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
        case 4: //aeolian
            pitch = pitch + 9; // aoelian
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = aeolian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
        case 5: //locrian
            pitch = pitch - 1; //locrian
            g_notes.push_back(Note(1, pitch, .75, 1));
            for (int i = 0; i < 50; i++){
                int index = rand() % 15;
                int nextNote = locrian[index];
                g_notes.push_back( Note( 1, pitch+nextNote, .75, .5+rand()/RAND_MAX) );
            }
            g_notes.push_back(Note(1, pitch, .75, 1));
            break;
    }
    
    // unlock
    g_mutex.unlock();

    // reset the index
    g_noteIndex = 0;
    
    // play now!
    g_nextTime = g_now;
}




//-----------------------------------------------------------------------------
// name: audio_callback
// desc: audio callback
//-----------------------------------------------------------------------------
static void audio_callback( SAMPLE * buffer, unsigned int numFrames, void * userData )
{
    // keep track of current time in samples
    g_now += numFrames;
    
    // HACK: rough time keeping for next notes - this logic really should be
    // somewhere else: e.g., in its own class and not directly in the audio callback!
    if( g_now > g_nextTime )
    {
        // lock (to protect vector)
        g_mutex.lock();
        // move down the vector
        if( g_noteIndex < g_notes.size() )
        {
            // temporary note pointer
            Note * n = &g_notes[g_noteIndex];
            // note on!
            g_synth->noteOn( n->channel, n->pitch, n->velocity * 120 );
            // HACK: with a major 3rd above!
            //g_synth->noteOn( n->channel, n->pitch + 4, n->velocity * 80 );
            // check to see next time
            g_nextTime += n->duration * SRATE;
            // move to next note for next time
            g_noteIndex++;
        }
        // release lock
        g_mutex.unlock();
    }
    
    // sum
    SAMPLE sum = 0;
    // num channels
    unsigned int channels = Globals::lastAudioBufferChannels;
    
    // zero out
    memset( Globals::lastAudioBuffer, 0,
           sizeof(SAMPLE)*Globals::lastAudioBufferFrames*channels );
    memset( Globals::lastAudioBufferMono, 0,
           sizeof(SAMPLE)*Globals::lastAudioBufferFrames );
    
    // copy to global buffer
    memcpy( Globals::lastAudioBuffer, buffer,
           sizeof(SAMPLE)*numFrames*channels );
    
    // copy to mono buffer
    for( int i = 0; i < numFrames; i++ )
    {
        // zero out
        sum = 0;
        // loop over channels
        for( int j = 0; j < channels; j++ )
        {
            // sum
            sum += buffer[i*channels + j];
        }
        // set
        Globals::lastAudioBufferMono[i] = sum / channels;
    }
    
    // window it for taper in visuals
    for( int i = 0; i < numFrames; i++ )
    {
        // multiply
        Globals::lastAudioBufferMono[i] *= Globals::audioBufferWindow[i];
    }

    // synthesize it
    g_synth->synthesize2( buffer, numFrames );

}




//-----------------------------------------------------------------------------
// name: raka_audio_init()
// desc: initialize audio system
//-----------------------------------------------------------------------------
bool raka_audio_init( unsigned int srate, unsigned int frameSize, unsigned channels )
{
    // initialize
    if( !XAudioIO::init( 0, 0, srate, frameSize, channels, audio_callback, NULL ) )
    {
        // done
        return false;
    }
    
    // instantiate
    g_synth = new mFluidSynth();
    // init
    g_synth->init( srate, 32 );
    
    // load the soundfont
    g_synth->load( "/Users/dggrunzweig/Developer/Muza/rtanpad1/rtanpad1.SF2", "" );
    // map program changes
    g_synth->programChange( 0, 0 );
    g_synth->programChange( 1, 79 );
    g_synth->programChange( 2, 4 );
    g_synth->programChange( 3, 10 );
    g_synth->programChange( 4, 13 );


//    // make a note
//    g_note = makeNote( 0, 60, .9, .5, 0 );
    
    // allocate
    Globals::lastAudioBuffer = new SAMPLE[frameSize*channels];
    // allocate mono buffer
    Globals::lastAudioBufferMono = new SAMPLE[frameSize];
    // allocate window buffer
    Globals::audioBufferWindow = new SAMPLE[frameSize];
    // set frame size (could have changed in XAudioIO::init())
    Globals::lastAudioBufferFrames = frameSize;
    // set num channels
    Globals::lastAudioBufferChannels = channels;
    
    
    return true;
}




//-----------------------------------------------------------------------------
// name: vq_audio_start()
// desc: start audio system
//-----------------------------------------------------------------------------
bool raka_audio_start()
{
    // start the audio
    if( !XAudioIO::start() )
    {
        // done
        return false;
    }
    
    return true;
}

void play_muzaSong(){
    
    // lock
    g_mutex.lock();
    // clear notes
    g_notes.clear();
    
    int root = 70;
    
    g_notes.push_back(Note(1, root, .75, 1));
    for (int i = 0; i < 25; i++){
        int index = rand() % 15;
        int nextNote = majorPentatonic[index];
        g_notes.push_back( Note( 1, root+nextNote, .75, .25+rand()/RAND_MAX) );
    }
    for (int i = 0; i < 25; i++){
        int index = rand() % 15;
        int nextNote = minorPentatonic[index];
        g_notes.push_back( Note( 1, root+nextNote, .75, .25+rand()/RAND_MAX) );
    }
    g_notes.push_back(Note(1, root, .75, 1));
    
    
    // unlock
    g_mutex.unlock();
    
    // reset the index
    g_noteIndex = 0;
    
    // play now!
    g_nextTime = g_now;
    
}
