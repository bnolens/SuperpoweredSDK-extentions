//
//  SuperpoweredMetronomePlayer.c
//  BandGenius
//
//  Created by Benoit Nolens on 10/27/15.
//  Copyright © 2015 The Hobby. All rights reserved.
//

#include "SuperpoweredMetronomePlayer.h"
#include <string.h>

SuperpoweredMetronomePlayer::SuperpoweredMetronomePlayer(unsigned int samplerate, SuperpoweredMetronomePlayerSignature signature) {
    
    this->samplerate = samplerate;
    this->signature = signature;
    this->beatIndex = 0;
    this->positionMs = 0;
    this->totalFrames = 0;
    this->clickFramesToPlay = 0;
    this->clickFramesPlayed = 0;
}

SuperpoweredMetronomePlayer::~SuperpoweredMetronomePlayer(void) {
    this->clear();
}

void SuperpoweredMetronomePlayer::loadDefaultSound(const char *path) {
    this->playing = false;
    this->mDefaultSource = new WAVDecoder(path);
}

void SuperpoweredMetronomePlayer::loadSecondarySound(const char *path) {
    this->playing = false;
    this->mSecondarySource = new WAVDecoder(path);
}

void SuperpoweredMetronomePlayer::clear() {
    delete this->mDefaultSource;
    delete this->mSecondarySource;
}

void SuperpoweredMetronomePlayer::setFirstBeatMs(const float firstBeat){
    this->firstBeat = firstBeat;
}

void SuperpoweredMetronomePlayer::resetPosition(bool andStop){
    this->positionMs = 0;
    
    this->totalFrames = positionMs * samplerate * 1000.f;
    
    if (!andStop) {
        this->play();
    } else {
        this->playing = false;
    }
}

void SuperpoweredMetronomePlayer::setTempo(const float tempo) {
    this->tempo = tempo;
}

void SuperpoweredMetronomePlayer::play() {
    this->playing = true;
}

void SuperpoweredMetronomePlayer::pause() {
    this->playing = false;
}

void SuperpoweredMetronomePlayer::stop() {
    this->playing = false;
    this->resetPosition(true);
}

bool SuperpoweredMetronomePlayer::process(float *buffer, bool bufferAdd, unsigned int numberOfSamples, float volume, double masterBpm, double masterMsElapsedSinceLastBeat, float beatIndex) {
    
    // Only process when it's playing and the volume is > 0
    if (this->playing && volume > 0) {
        
        // FIXME: Stop using the beatIndex parameter since it requires another player in the stack
        // or require to be calculated elsewhere. Instead it shoud be calculated here.
        this->beatIndex = beatIndex;
        
        // Calculate the current position in ms.
        // TODO: Test this! Not sure it is still right after tempo has been changed.
        this->positionMs += (numberOfSamples / (float)this->samplerate) * 1000.;
    
        // Only play a metronome click on the beat
        if (this->clickFramesToPlay > 0 || ((int)this->beatIndex != (int)this->previousBeatIndex && (int)this->beatIndex != 0)) {
            
            this->previousBeatIndex = this->beatIndex;
            
            // Get the correct decoded audio source
            // The first beat get another sound
            WAVDecoder *source;
            if (((int)this->beatIndex == 0 || (int)this->beatIndex == SuperpoweredMetronomePlayerSignature4By4) && getSecondarySource()) {
                source = getSecondarySource();
            } else {
                source = getDefaultSource();
            }
            
            // No source? Nothing to play than..
            if (!source) { return bufferAdd; }
            
            // Initiate empty buffers for new click sound
            float leftClickBuffer[numberOfSamples], rightClickBuffer[numberOfSamples];
            memset(leftClickBuffer, 0, numberOfSamples * sizeof(float));
            memset(rightClickBuffer, 0, numberOfSamples * sizeof(float));
            float *clickBuffers[2] = {leftClickBuffer, rightClickBuffer};
            
            // Process metronome click frames
            processClick(clickBuffers, numberOfSamples, source);
            
            // Fill audio buffer with metronome sound and apply the volume on it
            for (int i = 0; i < numberOfSamples; i++) {
                buffer[i*2] += (short)(leftClickBuffer[i] * 0x8000) * 0.0001 * volume;
                buffer[i*2+1] += (short)(rightClickBuffer[i] * 0x8000) * 0.0001 * volume;
            }
            
            // Reset clickFramesToPlay & clickFramesPlayed when one click is completely played
            if (this->clickFramesToPlay == 0) {
                this->clickFramesToPlay = (int)source->getNumFrames() / numberOfSamples;
                this->clickFramesPlayed = 0;
            }
            
            this->totalFrames += numberOfSamples;
            this->clickFramesToPlay--;
        }
    }
    
    return bufferAdd;
}

void SuperpoweredMetronomePlayer::processClick(float** const buffers, const int frameCount, WAVDecoder *source) {
    
    if ( !this->playing ) return;
    
    int64_t numFramesLeft = source->getNumFrames() - this->clickFramesPlayed;
    if (numFramesLeft < 0) {
        numFramesLeft = 0;
        this->clickFramesPlayed = 0;
    }
    int64_t numFramesToRead = (numFramesLeft < frameCount) ? numFramesLeft : frameCount;
    unsigned long index = this->clickFramesPlayed;
    
    memcpy(buffers[0], source->getFloatData()+index, numFramesToRead * sizeof(float));
    memcpy(buffers[1], source->getFloatData()+index, numFramesToRead * sizeof(float));

    this->clickFramesPlayed += numFramesToRead;
}