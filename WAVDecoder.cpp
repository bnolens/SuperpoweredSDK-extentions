//
//  WAVDecoder.cpp
//  WAVDecoder
//
//  Created by Benoit Nolens on 30/10/2015.
//  Copyright (c) 2015 True Story. All rights reserved.
//

#include <stdlib.h>
#include <fstream>
#include <assert.h>
#include <string.h>
#include "WAVDecoder.h"

WAVDecoder::WAVDecoder(const char* inFilePath) {
    
    // Open the file
    FILE* fd = NULL;
    fd = fopen(inFilePath,"rb");
    assert(fd);
    
    // Read header data
    this->readHeader(fd, &this->mSampleRate, &this->mBitsPerSample, &this->mNumberOfChannels, &this->mNumFrames);
    
    // Read data
    this->mFloatData = new float[mNumFrames*sizeof(float)];
    this->readData(fd, (float *)this->mFloatData, this->mSampleRate, this->mBitsPerSample, this->mNumFrames);
    
    // Close the file
    fclose(fd);
}

WAVDecoder::WAVDecoder() {
    
	delete this->mFloatData;
}

/**
 @brief Method to read all header data
 
 @param fd FILE object of the header we want to read
 @param sampRate Sample rate (reference)
 @param bitsPerSamp Bits per sample (reference)
 @param numSamp Number of samples (reference)
 */
void WAVDecoder::readHeader(FILE *fd, unsigned int *sampRate, unsigned int *bitsPerSamp, unsigned int *numOfChannels, unsigned int *numSamp) {
    
    unsigned char buf[5];
    
    /* ChunkID (RIFF for little-endian, RIFX for big-endian) */
    fread(buf, 1, 4, fd);
    buf[4] = '\0';
    assert(!strcmp((char*)buf, "RIFF") && "[WAVDecoder] ChunkID should be 'RIFF'");
    
    /* ChunkSize */
    fread(buf, 1, 4, fd);
    
    /* Format */
    fread(buf, 1, 4, fd);
    buf[4] = '\0';
    assert(!strcmp((char*)buf, "WAVE") && "[WAVDecoder] Format should be 'WAVE'");
    
    /* Subchunk1ID */
    fread(buf, 1, 4, fd);
    buf[4] = '\0';
    assert(!strcmp((char*)buf, "fmt ") && "[WAVDecoder] Subchunk1ID should be 'fmt'");
    
    /* Subchunk1Size (16 for PCM) */
    fread(buf, 1, 4, fd);
    assert((buf[0] == 16 && !buf[1] && !buf[2] && !buf[3]) && "[WAVDecoder] Bits per sample should be set to 16");
    
    /* AudioFormat (PCM = 1, other values indicate compression) */
    fread(buf, 1, 2, fd);
    assert(buf[0] == 1 && !buf[1] && "[WAVDecoder] AudioFormat should be PCM");
    
    /* NumChannels (Mono = 1, Stereo = 2, etc) */
    fread(buf, 1, 2, fd);
    unsigned int num_ch = buf[0] + (buf[1] << 8);
    assert(num_ch == 1 && "[WAVDecoder] Audio file should be mono");
    *numOfChannels = num_ch;
    
    /* SampleRate (8000, 44100, etc) */
    fread(buf, 1, 4, fd);
    *sampRate = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);
    
    /* ByteRate (SampleRate * NumChannels * BitsPerSample / 8) */
    fread(buf, 1, 4, fd);
    const unsigned int byte_rate = buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
    
    /* BlockAlign (NumChannels * BitsPerSample / 8) */
    fread(buf, 1, 2, fd);
    const unsigned int block_align = buf[0] + (buf[1] << 8);
    
    /* BitsPerSample */
    fread(buf, 1, 2, fd);
    *bitsPerSamp = buf[0] + (buf[1] << 8);
    
    assert(byte_rate == ((*sampRate * num_ch * *bitsPerSamp) >> 3) && "[WAVDecoder] Wrong bitrate");
    assert(block_align == ((num_ch * *bitsPerSamp) >> 3) && "[WAVDecoder] Wrong block align");
    
    /* Subchunk2ID */
    fread(buf, 1, 4, fd);
    assert(!strcmp((char*)buf, "data") && "[WAVDecoder] Unable to find the 'data' shunk");
    
    /* Subchunk2Size (NumSamples * NumChannels * BitsPerSample / 8) */
    fread(buf, 1, 4, fd);
    const unsigned int subchunk2_size = buf[0] + (buf[1] << 8) +
    (buf[2] << 16) + (buf[3] << 24);
    *numSamp = (subchunk2_size << 3) / (num_ch * *bitsPerSamp);
}

/**
 @brief Method to read all audio data and fill it in the data buffer (data is converted to floats)
 
 @param fd FILE object of the audio data we want to read
 @param data Data buffer we want to fill with the audio data (floats)
 @param sampRate Sample rate
 @param bitsPerSamp Bits per sample
 @param numSamp Number of samples
 */
void WAVDecoder::readData(FILE *fd, float *data, unsigned int sampRate, unsigned int bitsPerSamp, unsigned int numSamp) {
    
    unsigned char buf;
    unsigned int i, j;
    for (i=0; i < numSamp; ++i) {
        unsigned int tmp = 0;
        for (j=0; j != bitsPerSamp; j+=8) {
            // Read one sample
            fread(&buf, 1, 1, fd);
            tmp += buf << j;
        }
        
        // Fill the data buffer with the converted results
        // We also convert the result to floats
        data[i] = (float)convBitSize(tmp, bitsPerSamp)/0x8000;
    }
}

int WAVDecoder::convBitSize(unsigned int in, int bps) {
    
    const unsigned int max = (1 << (bps-1)) - 1;
    return in > max ? in - ((max<<1)+2) : in;
}
