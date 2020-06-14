#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <thread>
#include <iostream>
#include <string>
#include <atomic>

using namespace std;

#include "opae_svc_wrapper.h"
#include "csr_mgr.h"

 typedef struct samples_t{
     
     union{
        int data[16];

      struct{
        uint64_t newData;
        uint64_t oldData;
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

void produtor(uint64_t *buffer){
    struct timespec pause;
    pause.tv_sec = 4;//(fpga->hwIsSimulated() ? 1 : 0);
    
    int counter =0;
    while(1){
     int aux = (buffer[0] & 1);
     uint64_t finalData = std::hash<std::thread::id>{}(std::this_thread::get_id());
     finalData = finalData << 4;
     finalData = finalData | aux;
     if (aux <=0){
      buffer[0] = buffer[0] + 1;
      std::cout << "CHANGE" << std::endl;
     }
    nanosleep(&pause, NULL);
    if (counter==10){
        std::cout << "FIM" << std::endl;
        return;
    }
    counter++;
    }
}

void consumidor(uint64_t *buffer){
    struct timespec pause;
    pause.tv_sec = 4;//(fpga->hwIsSimulated() ? 1 : 0);
    int counter =0;
    while (1){
     int aux = (buffer[0] & 1);
     uint64_t finalData = std::hash<std::thread::id>{}(std::this_thread::get_id());
     finalData = finalData << 4;
     finalData = finalData | aux;
     
     if (aux>0) {
       finalData = finalData-1;
       buffer[0] = finalData;
       cout << "CHANGE" << std::endl;
     }
     
    nanosleep(&pause, NULL);
    if (counter==10){
        std::cout << "FIM" << std::endl;
        return;
    }
    counter++;
    }
    return;
}

void threadManager (uint64_t *buffer){
    
    std::thread p (produtor,buffer);
    std::thread c (consumidor,buffer);
    
    p.join();
    c.join();    
    
}



int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    long samples = 10;
    

    auto monitorada = (uint64_t *)fpga->allocBuffer(CL(1));
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
    csrs.writeCSR(4, 100);
    
    std::thread p (produtor,monitorada);
    std::thread c (consumidor,monitorada);
    
    p.join();
    c.join(); 
    
    std::terminate;
    
    for(int i = 0; i < samples; i++){
        std::cout << i <<" Tempo: " << samples_buffer[i].time << std::endl;
        std::cout << "Valor Antigo: " << (samples_buffer[i].oldData&1) << " Thread: "<< std::hex << (samples_buffer[i].oldData >> 4) << std::endl;
        std::cout << "Valor Novo: " << (samples_buffer[i].newData&1) << " Thread: "<< std::hex << (samples_buffer[i].newData >> 4) << std::endl;
    }
  
    delete fpga;
    
    return 0;

}
