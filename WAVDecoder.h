//
//  WAVDecoder.cpp
//  WAVDecoder
//
//  Created by Benoit Nolens on 30/10/2015.
//  Copyright (c) 2015 True Story. All rights reserved.
//

#ifndef __Buffer__
#define __Buffer__

#include <stdint.h>

/**
 @brief This class helps you decode WAV files. It has only been tested with WAV files with one specific encoding. This is how the WAV file should be encoded:
 
 Format: WAV,
 AudioFormat: PCM,
 Bits per sample: 16 bit,
 Number of channels: 1 (mono),
 Sample rate: 44100
 
 TIP: Export your wav file with Audacity with the 'WAV 16 bit PCM' preset and it should work.
 */
class WAVDecoder {
public:
	
    /**
     @brief Constructor
     @param inFilePath Local path to the file to load in memory
     */
	WAVDecoder(const char* inFilePath);
    
    /**
     @brief Deconstructor
     */
	WAVDecoder();
	
    /**
     @brief Number of channels of this wav file
     */
    const int getNumChannels() { return mNumberOfChannels; };
	
    /**
     @brief Number of frames of this wav file
     */
    const int64_t getNumFrames() { return mNumFrames; }
	
    /**
     @brief Sample rate of this wav file
     */
    const double getSampleRate() { return mSampleRate; }
	
    /**
     @brief Bits per sample of this wav file
     */
    const double getBitsPerSample() { return mBitsPerSample; }
    
    /**
     @brief Data of this wav file
     */
	const float* const &getFloatData() { return mFloatData; }
	
private:
	
    unsigned int mNumberOfChannels;
    unsigned int mNumFrames;
    unsigned int mSampleRate;
    unsigned int mBitsPerSample;
    const float *mFloatData;
    
    /**
     @brief Method to read all header data
     
     @param fd FILE object of the header we want to read
     @param sampRate Sample rate (reference)
     @param bitsPerSamp Bits per sample (reference)
     @param numSamp Number of samples (reference)
     */
    void readHeader(FILE *fd, unsigned int *sampRate, unsigned int *bitsPerSamp, unsigned int *numberOfChannels, unsigned int *numSamp);
    
    /**
     @brief Method to read and decode all audio data and fill it in the data buffer (data is converted to floats)
     
     @param fd FILE object of the audio data we want to read
     @param data Data buffer we want to fill with the audio data (floats)
     @param sampRate Sample rate
     @param bitsPerSamp Bits per sample
     @param numSamp Number of samples
     */
    void readData(FILE *fd, float *data, unsigned int sampRate, unsigned int bitsPerSamp, unsigned int numSamp);
    
    int convBitSize(unsigned int in, int bps);
};

#endif /* defined(__Buffer__) */
