#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <thread>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <random>
#include <utility>

using namespace std;

#include "opae_svc_wrapper.h"
#include "csr_mgr.h"

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

typedef struct {
    int thread_id;
    int valor;
}result_t;



constexpr long size= 10000;   
constexpr long  firBound=  2500;
constexpr long  secBound=  5000;
constexpr long  thiBound=  7500;
constexpr long  fouBound= 10000;


std::mutex myMutex;

void sumUp(result_t somatorio, int *val, int beg, int end){
    for (int it= beg; it < end; it++){
        std::lock_guard<std::mutex> myLock(myMutex);
        *somatorio.valor+= val[it];
    }
}


int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    int samples = 1000;
    
    int *randValues = new int [size];


   for ( long  i=0 ; i< size ; ++i) randValues[i] = 10;   
    
    auto somatorio = (result *)fpga->allocBuffer(CL(1));
    *somatorio.valor = 0;
    *somatorio.thread_id = 0;
   
    assert(NULL != somatorio);
        
    auto samples_buffer = (samples_t *)fpga->allocBuffer(samples*sizeof(samples_t));
    assert(NULL != samples_buffer);
    
    auto status = (status_t *)fpga->allocBuffer(CL(1));
    assert(NULL != status);

    csrs.writeCSR(0, intptr_t(somatorio)/64);
    csrs.writeCSR(1, intptr_t(samples_buffer)/64);
    csrs.writeCSR(2, intptr_t(status)/64);
    csrs.writeCSR(3, samples);
    csrs.writeCSR(4, 200);
  
    
    std::thread t1(sumUp,somatorio,randValues,0,firBound);
    std::thread t2(sumUp,somatorio,randValues,firBound,secBound);
    std::thread t3(sumUp,somatorio,randValues,secBound,thiBound);
    std::thread t4(sumUp,somatorio,randValues,thiBound,fouBound); 
    
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();    
    
    
    
    
    struct timespec pause;
    pause.tv_sec = 0;//(fpga->hwIsSimulated() ? 1 : 0);
    pause.tv_nsec = 1;


    
    std::terminate;
    
    for(int i = 0; i < samples; i++){
        std::cout << i <<" Tempo: " << samples_buffer[i].time << " Old data: " << samples_buffer[i].oldData << " New Data: " << samples_buffer[i].newData  << std::endl;
        
    }
    
    while (status->done == 0 ){
        *somatorio.valor = *somatorio.valor +1;
        nanosleep(&pause, NULL);
    };
    
  
    delete fpga;
    
    return 0;

}
