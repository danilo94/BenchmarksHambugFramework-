#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <bits/stdc++.h>
#include <thread>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <cmath>
#include <vector>
#include <algorithm>
#include<fstream>
#include "opae_svc_wrapper.h"
#include "csr_mgr.h"
#define MATSIZE 800

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
    
void matmul (int a[][MATSIZE], int b[][MATSIZE], int c[][MATSIZE], volatile int *monitorada){
  for(int i = 0; i < MATSIZE; ++i){
   for(int j = 0; j < MATSIZE; ++j){
    for(int k = 0; k < MATSIZE; ++k){
      monitorada[0] += a[i][k] * b[k][j];
    }
    c[i][j] = monitorada[0];
    monitorada[0]=0;
   }
  }
}

int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    long samples = 1000;
    
    auto monitorada = (volatile int *)fpga->allocBuffer(CL(1));
    
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

          srand (time(NULL));
        int a[MATSIZE][MATSIZE], b[MATSIZE][MATSIZE], mult[MATSIZE][MATSIZE];
        for(int i = 0; i < MATSIZE; ++i)
            for(int j = 0; j < MATSIZE; ++j)
            {
                a[i][j] = rand() % 357 + 1;
                b[i][j] = rand() % 2000 + 1;
            }

        for(int i = 0; i < MATSIZE; ++i)
            for(int j = 0; j < MATSIZE; ++j)
            {
                mult[i][j]=0;
            }
            
            
            
            
    std::thread t1(matmul,a,b,mult,monitorada);


    t1.join();




    for (int i=0; i<samples; i++){
        if (samples_buffer[i].oldData != 0)
      std::cout << "Tempo: " << samples_buffer[i].time <<" Dado antigo: "<<samples_buffer[i].oldData << "  Dado novo:  "<< samples_buffer[i].newData << std::endl;
    }
    
        struct timespec pause;
    pause.tv_sec = 0;//(fpga->hwIsSimulated() ? 1 : 0);
    pause.tv_nsec = 1;

    
        while (status->done == 0 ){
        *monitorada = *monitorada + 9999;
        nanosleep(&pause, NULL);
    };
    delete fpga;
    
    return 0;
}
