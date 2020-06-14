#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <thread>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <cmath>
#include <vector>

#include<fstream>
#include "opae_svc_wrapper.h"
#include "csr_mgr.h"

using namespace std;


 typedef struct samples_t{
     union{
        int data[16];
      struct{
        uint32_t newData;
        uint32_t oldData;
        uint64_t time;
      };
     };
     
}samples_t;


typedef struct status_t{
    
    union{
    int  data[16];
    struct{
       char done:1;
     };
    };
          
}status_t;


std::mutex myMutex;


void encrypty(uint32_t *v0, uint32_t *v1, const uint32_t *k,int begin, int end, int *done) {
        for (int i = begin; i < end; i++) {
            uint32_t sum = 0, delta = 0x9e3779b9;
            for (int j = 0; j < 32; j++) {
                std::lock_guard<std::mutex> myLock(myMutex);
                sum += delta;
                v0[i] += ((v1[i] << 4) + k[0]) ^ (v1[i] + sum) ^ ((v1[i] >> 5) + k[1]);
                v1[i] += ((v0[i] << 4) + k[2]) ^ (v0[i] + sum) ^ ((v0[i] >> 5) + k[3]);
            }
        }
        *done= *done+1;
}


int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    long samples = 1000;
    
    auto monitorada = (int *)fpga->allocBuffer(CL(1));
    
    *monitorada = 0;

    assert(NULL != monitorada);
        
    auto samples_buffer = (samples_t *)fpga->allocBuffer(samples*sizeof(samples_t));
    assert(NULL != samples_buffer);
    
    auto status = (status_t *)fpga->allocBuffer(CL(1));
    assert(NULL != status);

    csrs.writeCSR(0, intptr_t(monitorada)/64);
    csrs.writeCSR(1, intptr_t(samples_buffer)/64);
    csrs.writeCSR(2, intptr_t(status)/64);
    csrs.writeCSR(3, samples);
    csrs.writeCSR(4, 200);

    uint32_t *key = new uint32_t[4];
    
    key[0] = 0x9474B8E8;
    key[1] = 0xC73BCA7D;
    key[2] = 0x53239142;
    key[3] = 0xf3c3121a;


    std::vector<std::string> strVec;    
    
    
    ifstream myReadFile;
    myReadFile.open("t8.shakespeare.txt");
    string buffer="";
    if (myReadFile.is_open()) {
        while (!myReadFile.eof()) {
            myReadFile >> buffer;
            strVec.push_back(buffer);
        }
    }
    myReadFile.close();
    
    std::vector <char> charsVector;
    
        for (int i=0; i<strVec.size(); i++){
            for (int j=0; j<strVec[i].length(); j++){
                charsVector.push_back((char) strVec[i][j]);
            }
        }
        
    uint32_t *chunk1 = new uint32_t[charsVector.size()/2];
    uint32_t *chunk2 = new uint32_t[charsVector.size()/2];

    for (int i=0; i<charsVector.size()/2; i++){
        chunk1[i] = (uint32_t) charsVector[i];
    }
    int j=0;
      for (int i=charsVector.size()/2; i<charsVector.size(); i++){
        chunk2[j] = (uint32_t) charsVector[i];
        j++;
    }
    
    thread t1(encrypty,chunk1,chunk2,key,0,503843,monitorada);
    thread t2(encrypty,chunk1,chunk2,key,503843,1007686,monitorada);
    thread t3(encrypty,chunk1,chunk2,key,1007686,1511529,monitorada);
    thread t4(encrypty,chunk1,chunk2,key,1511529,2015372,monitorada);

    t1.join();
    t2.join();
    t3.join();
    t4.join();

    
    for (int i=0; i<samples; i++){
      std::cout << "Tempo: " << samples_buffer[i].time <<"Dado antigo: "<<samples_buffer[i].oldData << "  Dado novo:  "<< samples_buffer[i].newData << std::endl;
    }
    
    
    delete fpga;
    
    return 0;
}
