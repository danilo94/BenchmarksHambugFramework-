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


std::mutex myMutex;


void isPrime(int value,int begin,int end,int *totalPrimes){
    for (int i=begin; i<=end; i++){
        if (value%i==0){
             std::lock_guard<std::mutex> myLock(myMutex);
            *totalPrimes = *totalPrimes + 1;
        }
    }
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
    
    int valor = 10000000;
    
    std::thread t1(isPrime,valor,1,2500000,monitorada);
    std::thread t2(isPrime,valor,2500000,5000000,monitorada);
    std::thread t3(isPrime,valor,5000000,750000,monitorada);
    std::thread t4(isPrime,valor,7500000,valor,monitorada);

    

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    

    
    

   
   
    for(int i = 0; i < samples; i++){
        std::cout << i <<" Tempo: " << samples_buffer[i].time << " Old data: " << samples_buffer[i].oldData << " New Data: " << samples_buffer[i].newData  << std::endl;
    }
  
    if (*monitorada>2)
        std::cout << "Não Primo" << std::endl;
  
    else
        std::cout << "Primo" << std::endl;
    
    
    delete fpga;
    
    return 0;

}
