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

#define SIZE 1000


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

void geometricMean(int arr[], int n, volatile float *result) 
{ 
    // declare product variable and 
    // initialize it to 1. 
    float product = 1; 
  
    // Compute the product of all the 
    // elements in the array. 
    for (int i = 0; i < n; i++) 
        product = product * arr[i]; 
  
    // compute geometric mean through formula 
    // pow(product, 1/n) and return the value 
    // to main function. 
    float result[0] = pow(product, (float)1 / n); 
} 




int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    int samples = 1000;
   
    auto monitorada_handle = fpga->allocBuffer(CL(1));
    auto monitorada= reinterpret_cast<volatile float*>(monitorada_handle->c_type());
    
    auto samples_buffer_handle = fpga->allocBuffer(samples*sizeof(samples_t));
    auto samples_buffer = reinterpret_cast<volatile samples_t *>(samples_buffer_handle->c_type());
 
    auto status_handle = fpga->allocBuffer(CL(1));
    auto status = reinterpret_cast<volatile status_t *>(status_handle->c_type());

    csrs.writeCSR(0, intptr_t(pi)/64);
    csrs.writeCSR(1, intptr_t(samples_buffer)/64);
    csrs.writeCSR(2, intptr_t(status)/64);
    csrs.writeCSR(3, samples);
    csrs.writeCSR(4, 200);
  
    
    
    int *valores = new int[SIZE];
    
    //volatile float monitorada[1];
    monitorada[0]=0;
    //srand (time(NULL));
    
    
    for (int i=0; i<SIZE; i++){
        valores[i] = rand() % SIZE + 1;
    }
    
    std::thread t1(geometricMean,valores,SIZE,monitorada);
    
    
    t1.join();
    
    struct timespec pause;
    pause.tv_sec = 0;//(fpga->hwIsSimulated() ? 1 : 0);
    pause.tv_nsec = 1;

    while (status->done == 0 ){
        nanosleep(&pause, NULL);
    };
    
    
    std::terminate;
    
   for(int i = 0; i < samples; i++){
        std::cout << i <<" Tempo: " << samples_buffer[i].time << " Old data: " << (double) samples_buffer[i].oldData << " New Data: " << (double) samples_buffer[i].newData  << std::endl;
        
    }
    
    delete fpga;
    
    return 0;

}
