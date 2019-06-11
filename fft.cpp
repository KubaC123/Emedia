//
//  main.cpp
//  WAV
//
//  Created by Jakub Cichy on 02/06/2019.
//  Copyright © 2019 Jakub Cichy. All rights reserved.
//

#include <iostream>
#include <cstdint>
#include <vector>
#include <complex>
#include <valarray>
#include <fstream>

typedef struct WAV_HEADER {
    uint8_t RIFF[4];
    uint32_t chunkSize;
    uint8_t WAVE[4];
    uint8_t fmt[4];
    uint8_t subChunk1Size;
    uint16_t audioFormat;
    uint16_t numOfChan;
    uint32_t samplesPerSec;
    uint32_t bytesPerSec;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    uint8_t subchunk2Id[4];
    uint32_t subchunk2Size;
} wav_header;

typedef std::complex<double> Complex; // liczba zespolona
typedef std::valarray<Complex> ComplexArray;

const double PI = 3.141592653589793238460;

int getFileSize(FILE *file) {
    int fileSize = 0;
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    return fileSize;
}

ComplexArray readMetadataAndGet1000Samples(const char* filePath) {
    wav_header wavHeader;
    int headerSize = sizeof(wav_header), filelength = 0;
    FILE *wavFile = fopen(filePath, "r");
    Complex samples[1000];
    int i = 0;
    if(wavFile == nullptr) {
        fprintf(stderr, "Unable to open given file.");
        exit(1);
    }
    size_t bytes = fread(&wavHeader, 1, headerSize, wavFile);
    std::cout << "Bytes: " << bytes << std::endl;
    if(bytes > 0) {
        uint16_t bytesPerSample = wavHeader.bitsPerSample / 8;
        uint64_t numSamples = wavHeader.chunkSize / bytesPerSample;
        static const uint16_t BUFFER_SIZE = 4096;
        int8_t* buffer = new int8_t[BUFFER_SIZE];
        while((bytes = fread(buffer, sizeof(buffer[0]), BUFFER_SIZE / sizeof(buffer[0]), wavFile)) > 0) {
            // READ 1000 SAMPLES
            for(int i = 0; i < 1000; i++) {
                samples[i] = buffer[i];
            }
            break;
        }
        delete [] buffer;
        buffer = nullptr;
        filelength = getFileSize(wavFile);
        
        // PRINT WAV HEADER
        std::cout << "WAV FILE HEADER: " << std::endl;
        std::cout << "File has: " << filelength << " bytes." << std::endl;
        std::cout << "RIFF header: " << wavHeader.RIFF[0] << wavHeader.RIFF[1] << wavHeader.RIFF[2] << wavHeader.RIFF[3] << std::endl;
        std::cout << "WAVE header: " << wavHeader.WAVE[0] << wavHeader.WAVE[1] << wavHeader.WAVE[2] << wavHeader.WAVE[3] << std::endl;
        std::cout << "FMT :" << wavHeader.fmt[0] << wavHeader.fmt[1] << wavHeader.fmt[2] << wavHeader.fmt[3] << std::endl;
        std::cout << "Data size : " << wavHeader.chunkSize << std::endl;
        std::cout << "Sampling rate: " << wavHeader.samplesPerSec << std::endl;
        std::cout << "Number of bits used: " << wavHeader.bitsPerSample << std::endl;
        std::cout << "Number of channels: " << wavHeader.numOfChan << std::endl;
        std::cout << "Number of bytes per second: " << wavHeader.bytesPerSec << std::endl;
        std::cout << "Data length: " << wavHeader.subchunk2Size << std::endl;
        std::cout << "Audio format: " << wavHeader.audioFormat << std::endl;
        std::cout << "Block align: " << wavHeader.blockAlign << std::endl;
        std::cout << "Data string: " << wavHeader.subchunk2Id[0] << wavHeader.subchunk2Id[1] << wavHeader.subchunk2Id[2] << wavHeader.subchunk2Id[3] << std::endl;
        std::cout << "VECTOR:" << std::endl;
        // PRINT SAMPLES
        for(int i = 0; i < 1000; i++) {
            std::cout << samples[i] << std::endl;
        }
    }
    fclose(wavFile);
    ComplexArray samplesArray(samples, 1000);
    return samplesArray;
}

void FFT(ComplexArray &samples) {
    const size_t N = samples.size();
    if(N <= 1) return;
    ComplexArray even = samples[std::slice(0, N/2, 2)];
    ComplexArray odd = samples[std::slice(1, N/2, 2)];
    
    FFT(even);
    FFT(odd);
    
    // combine
    for(size_t i = 0; i < N/2; ++i) {
        Complex temp = std::polar(1.0, -2 * PI * i / N) * odd[i];
        samples[i] = even[i] + temp;
        samples[i + N/2] = even[i] - temp;
    }
}

void IFFT(ComplexArray &samples) {
    // std::conj licze sprzężenie zespolone
    samples = samples.apply(std::conj);
    
    FFT(samples);
    
    samples = samples.apply(std::conj);
    
    samples /= samples.size();
}

int main(int argc, const char * argv[]) {
    ComplexArray samples;
    samples = readMetadataAndGet1000Samples("test.wav");
    
    FFT(samples);
    std::cout << "FFT perform" << std::endl;
    for(int i = 0; i < 1000; ++i) {
        std::cout << samples[i] << std::endl;
    }
    
    std::ofstream output;
    output.open("fftdata.txt", std::ofstream::out | std::ofstream::trunc);
    
    for(int i=0; i < 1000; ++i) {
        output << samples[i].real() << ", " << samples[i].imag() << std::endl;
    }
    output.close();
    
    // PLOT USING GNUPLOT
    system("gnuplot fftplot");
}
