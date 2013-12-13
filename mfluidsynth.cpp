/*----------------------------------------------------------------------------
  MCD-Y: higher-level objects for audio/graphics/interaction programming
         (sibling of MCD-X API)

  Copyright (c) 2013 Ge Wang
    All rights reserved.
    http://ccrma.stanford.edu/~ge/

  Music, Computing, Design Group @ CCRMA, Stanford University
    http://ccrma.stanford.edu/groups/mcd/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  U.S.A.
-----------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// name: y-fluidsynth.cpp
// desc: fluidsynth software synthesizer wrapper
//
// authors: Ge Wang (ge@ccrma.stanford.edu)
//    date: Winter 2010
//    version: 1.0
//-----------------------------------------------------------------------------
#include "mfluidsynth.h"
#include <iostream>
using namespace std;




//-----------------------------------------------------------------------------
// name: YFluidSynth()
// desc: constructor
//-----------------------------------------------------------------------------
mFluidSynth::mFluidSynth()
    : m_settings(NULL), m_synth(NULL)
{ }




//-----------------------------------------------------------------------------
// name: ~YFluidSynth()
// desc: destructor
//-----------------------------------------------------------------------------
mFluidSynth::~mFluidSynth()
{
    // lock
    m_mutex.lock();

    // clean up
    if( m_synth ) delete_fluid_synth( m_synth );
    if( m_settings ) delete_fluid_settings( m_settings );
    m_synth = NULL;
    m_settings = NULL;
    
    // unlock
    m_mutex.unlock();
}




//-----------------------------------------------------------------------------
// name: init()
// desc: init the synth
//-----------------------------------------------------------------------------
bool mFluidSynth::init( int srate, int polyphony )
{
    if( m_synth != NULL )
    {
        std::cerr << "synth already initialized..." << std::endl;
        return false;
    }
    
    // lock
    m_mutex.lock();
    
    // log
    cerr << "[y-fluidsynth]: initializing synth..." << endl;
    // instantiate settings
    m_settings = new_fluid_settings();
    // set sample rate
    fluid_settings_setnum( m_settings, (char *)"synth.sample-rate", (double)srate );
    // set polyphony
    if( polyphony <= 0 ) polyphony = 1;
    else if( polyphony > 256 ) polyphony = 256;
    fluid_settings_setint( m_settings, (char *)"synth.polyphony", polyphony );
    // instantiate the synth
    m_synth = new_fluid_synth( m_settings );
    
    // unlock
    m_mutex.unlock();
    
    return m_synth != NULL;
}
    



//-----------------------------------------------------------------------------
// name: load()
// desc: load a font
//-----------------------------------------------------------------------------
bool mFluidSynth::load( const char * filename, const char * extension )
{
    if( m_synth == NULL ) return false;

    // lock
    m_mutex.lock();

    // the pathc
    std::string path = filename;
    
    // log
    // NSLog( @"loading font file: %s.%s...", filename, extension );
    
    // load
    if( fluid_synth_sfload( m_synth, path.c_str(), true ) == -1 )
    {
        // error
        std::cerr << "cannot load font file: " << filename << "." << extension << std::endl;
        // unlock
        m_mutex.unlock();

        return false;
    }
    
    // unlock
    m_mutex.unlock();
    
    return true;
}




//-----------------------------------------------------------------------------
// name: programChange()
// desc: apply program change
//-----------------------------------------------------------------------------
void mFluidSynth::programChange( int channel, int program )
{
    if( m_synth == NULL ) return;
    if( program < 0 || program > 127 ) return;
    m_mutex.lock();
    fluid_synth_program_change( m_synth, channel, program );
    m_mutex.unlock();
}
    



//-----------------------------------------------------------------------------
// name: controlChange()
// desc: control change
//-----------------------------------------------------------------------------
void mFluidSynth::controlChange( int channel, int data2, int data3 )
{
    if( m_synth == NULL ) return;
    if( data2 < 0 || data2 > 127 ) return;
    m_mutex.lock();
    fluid_synth_cc( m_synth, channel, data2, data3 );
    m_mutex.unlock();
}




//-----------------------------------------------------------------------------
// name: noteOn()
// desc: send a note on message
//-----------------------------------------------------------------------------
void mFluidSynth::noteOn( int channel, float pitch, int velocity )
{
    // sanity check
    if( m_synth == NULL ) return;
    // integer pitch
    int pitch_i = (int)(pitch + .5f);
    // difference
    float diff = pitch - pitch_i;
    // lock
    m_mutex.lock();
    // if needed
    if( diff != 0 )
    {
        // pitch bend
        fluid_synth_pitch_bend( m_synth, channel, (int)(8192 + diff * 8191) );
    }
    // sound note
    fluid_synth_noteon( m_synth, channel, pitch, velocity );
    // unlock
    m_mutex.unlock();
}




//-----------------------------------------------------------------------------
// name: pitchBend()
// desc: send a pitchBend on message
//-----------------------------------------------------------------------------
void mFluidSynth::pitchBend( int channel, float pitchDiff )
{
    // sanity check
    if( m_synth == NULL ) return;
    // lock
    m_mutex.lock();
    // pitch bend
    fluid_synth_pitch_bend( m_synth, channel, (int)(8192 + pitchDiff * 8191) );
    // unlock
    m_mutex.unlock();
}




//-----------------------------------------------------------------------------
// name: noteOff()
// desc: send a note off message
//-----------------------------------------------------------------------------
void mFluidSynth::noteOff( int channel, int pitch )
{
    if( m_synth == NULL ) return;
    m_mutex.lock();
    fluid_synth_noteoff( m_synth, channel, pitch );
    m_mutex.unlock();
}




//-----------------------------------------------------------------------------
// name: allNotesOff()
// desc: send all notes off message
//-----------------------------------------------------------------------------
void mFluidSynth::allNotesOff( int channel )
{
    // send all notes off control message
    controlChange( channel, 120, 0x7B );
}



//-----------------------------------------------------------------------------
// name: synthesize2()
// desc: synthesize stereo output (interleaved)
//-----------------------------------------------------------------------------
bool mFluidSynth::synthesize2( double * buffer, unsigned int numFrames )
{
    if( m_synth == NULL ) return false;
    m_mutex.lock();
    // get it from fluidsynth
    int retval = fluid_synth_write_float( m_synth, numFrames, (float *)buffer, 0, 2, buffer, 1, 2 );
    m_mutex.unlock();
    
    // return
    return retval == 0;
}
