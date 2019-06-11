//
//  main.cpp
//  WAV
//
//  Created by Jakub Cichy on 10/06/2019.
//  Copyright © 2019 Jakub Cichy. All rights reserved.
//

#include <iostream>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <complex>
#include <valarray>
#include <fstream>
#include <string>

#define TESTWAV "test.wav"
#define ENCRYPTEDWAV "encrypted.wav"
#define DECRYPTEDWAV "decrypted.wav"
#define KEYFILE "key.txt"

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

int getFileSize(FILE *file) {
    int fileSize = 0;
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    return fileSize;
}

std::pair<long, int> readMetadata(const char* filePath) {
    wav_header wavHeader;
    int headerSize = sizeof(wav_header), filelength = 0;
    FILE *wavFile = fopen(filePath, "r");
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
            
        }
        delete [] buffer;
        buffer = nullptr;
        filelength = getFileSize(wavFile);
        
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
    }
    fclose(wavFile);
    return std::make_pair((long)wavHeader.chunkSize, headerSize);
}

class XOR {
    
    std::string key = "";
    
public:
    
    XOR() { };
    
    void generateKey(long lenght) {
        srand(time(NULL));
        std::ofstream output;
        output.open(KEYFILE, std::ofstream::out | std::ofstream::trunc);
        std::string k = "";
        for(int i = 0; i < lenght; i++) {
            k.append(std::to_string(rand() % 93 + 33));
        }
        output << k << std::endl;
        this->key = k;
    }

    void encryptWav(int headerSize) {
        FILE *inputFile = fopen(TESTWAV, "r");
        std::ofstream outputFile;
        outputFile.open(ENCRYPTEDWAV, std::ofstream::out | std::ofstream::trunc);
        char byte[1];
        
        for(int i = 0; i < headerSize; i++) {
            byte[0] = getc(inputFile);
            outputFile.put(byte[0]);
        }
        
        std::vector<char> toSwap;
        for(int i = 0; i < key.length(); i++) {
            int temp = getc(inputFile);
            temp = temp ^ key[i];
            toSwap.push_back((char)temp);
            if(toSwap.size() == 8) {
                std::reverse(toSwap.begin(), toSwap.end());
                for(int j = 0; j < toSwap.size(); j++) {
                    outputFile.put(toSwap[j]);
                }
                toSwap.clear();
            }
        }
        if(toSwap.size() != 0) {
            std::reverse(toSwap.begin(), toSwap.end());
            for(int i = 0; i < toSwap.size(); i++) {
                outputFile.put(toSwap[i]);
            }
            toSwap.clear();
        }
    
        fclose(inputFile);
        outputFile.close();
    }
    
    void decryptWav(int headerSize) {
        FILE *encryptedFile = fopen(ENCRYPTEDWAV, "r");
        std::ofstream outputFile;
        outputFile.open(DECRYPTEDWAV, std::ofstream::out | std::ofstream::trunc);
        char byte[1];
        
        for(int i = 0; i < headerSize; i++) {
            byte[0] = getc(encryptedFile);
            outputFile.put(byte[0]);
        }
        
        std::vector<char> toSwap;
        for(int i = 0; i < key.length(); i++) {
            int temp = getc(encryptedFile);
            toSwap.push_back((char)temp);
            if(toSwap.size() == 8) {
                std::reverse(toSwap.begin(), toSwap.end());
                for(int j = toSwap.size() - 1; j >= 0; j--) {
                    temp = toSwap[toSwap.size() - 1 - j] ^ key[i - j];
                    outputFile.put((char)temp);
                }
                toSwap.clear();
            }
        }
        if(toSwap.size() != 0) {
            std::reverse(toSwap.begin(), toSwap.end());
            for(int j = toSwap.size() - 1; j >= 0; j--) {
                int temp = toSwap[toSwap.size() - 1 - j] ^ key[key.length() - 1 - j];
                outputFile.put((char)temp);
            }
            toSwap.clear();
        }
        
        fclose(encryptedFile);
        outputFile.close();
    }
};

int main(int argc, const char * argv[]) {
    std::pair<long, int> lenghtAndHeaderSize = readMetadata(TESTWAV);
    XOR xor1;
    xor1.generateKey(lenghtAndHeaderSize.first);
    xor1.encryptWav(lenghtAndHeaderSize.second);
    xor1.decryptWav(lenghtAndHeaderSize.second);
}
