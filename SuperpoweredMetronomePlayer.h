//
//  SuperpoweredMetronomePlayer.h
//  BandGenius
//
//  Created by Benoit Nolens on 10/27/15.
//  Copyright © 2015 The Hobby. All rights reserved.
//

#ifndef SuperpoweredMetronomePlayer_h
#define SuperpoweredMetronomePlayer_h

enum SuperpoweredMetronomePlayerSignature {
    SuperpoweredMetronomePlayerSignature4By4 = 4
};

#import <stdio.h>
#include "WAVDecoder.h"

/**
 @brief High performance metronome player
 
 @param firstBeatMs Tells where the first beat (the beatgrid) begins. Must be correct for syncing.
 @param positionMs The current position. Always accurate, no matter of time-stretching and other transformations. Read only.
 @param playing Indicates if the player is playing or paused. Read only.
 @param tempo The current tempo. Read only.
 @param beatIndex Which beat has just happened (1 [1.0f-1.999f], 2 [2.0f-2.999f], 3 [3.0f-3.99f], 4 [4.0f-4.99f]). A value of 0 means "don't know". Read only.
 
 */
class SuperpoweredMetronomePlayer {
public:
// READ ONLY parameters, don't set them directly, use the methods below.
    float firstBeat;
    float positionMs;
    bool playing;
    float tempo;
    float beatIndex;
    SuperpoweredMetronomePlayerSignature signature;
    
    /**
     @brief Create a metronome player instance with the current sample rate value.
     
     Example: SuperpoweredMetronomePlayer player = new SuperpoweredMetronomePlayer(44100, SuperpoweredMetronomePlayerSignature4x4);
     
     @param samplerate The current samplerate.
     @param signature The signature of the track (example 4/4)
     */
    SuperpoweredMetronomePlayer(unsigned int samplerate, SuperpoweredMetronomePlayerSignature signature);
    ~SuperpoweredMetronomePlayer();
    
    /**
     @brief Start playback
     */
    void play();
    
    /**
     @brief Pause playback
     */
    void pause();
    
    /**
     @brief Stop playback
     */
    void stop();
    
    /**
     @brief Process the metronome audio. This process method should be executed in the audio processing callback function. This method fills the buffer with metronome click samples.
     
     @return Put something into output or not.

     @param buffer 32-bit interleaved stereo input/output buffer. Should be numberOfSamples * 8 + 64 bytes big.
     @param bufferAdd If true, the contents of buffer will be preserved and audio will be added to them. If false, buffer is completely overwritten.
     @param numberOfSamples The number of samples to provide.
     @param volume 0.0f is silence, 1.0f is "original volume". Changes are automatically smoothed between consecutive processes.
     @param masterBpm A bpm value to sync with. Use 0.0f for no syncing.
     @param masterMsElapsedSinceLastBeat How many milliseconds elapsed since the last beat on the other stuff we are syncing to. Use -1.0 to ignore.
     @param beatIndex A beatIndex value to sync with.
     */
    bool process(float *buffer, bool bufferAdd, unsigned int numberOfSamples, float volume = 1.0f, double masterBpm = 0.0f, double masterMsElapsedSinceLastBeat = -1.0, float beatIndex = 0.0f);
    
    /**
     @brief Values used to calculate when to play the beats. (IMPORTANT!) Currently unused since we use the beatIndex value passed by the process method
     
     Should be called after a successful open().
     */
    void setFirstBeatMs(const float firstBeat);
    
    /**
     @brief Reset the player postion to 0. 
     
     @param andStop If true, stops playback.
     */
    void resetPosition(bool andStop);
    
    /**
     @brief Sets the relative tempo of the playback.
     
     @param tempo 1.0f is "original speed".
     */
    void setTempo(const float tempo);

    /**
     @brief Opens a file for the default click sound, with playback paused.
     
    @param path The full file system path of the file.
     */
    void loadDefaultSound(const char *path);
    
    /**
     @brief (optional) Opens a file for the secondary click sound, with playback paused. This sound is played on every first beat.
     
     @param path The full file system path of the file.
     */
    void loadSecondarySound(const char *path);
    
    /**
     @brief Returns the decoded default click sound
     */
    inline WAVDecoder* const &getDefaultSource() { return mDefaultSource; }
    
    /**
     @brief Returns the decoded secondary click sound
     */
    inline WAVDecoder* const &getSecondarySource() { return mSecondarySource; }
    
    void clear();
    
private:
    unsigned int samplerate;
    float previousBeatIndex;
    float totalFrames;
    int clickFramesToPlay;
    int64_t clickFramesPlayed;
    WAVDecoder *mDefaultSource;
    WAVDecoder *mSecondarySource;
    void processClick(float** const buffers, const int frameCount, WAVDecoder *source);
};

#endif
