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



constexpr long size= 10000;   

constexpr long  firBound=  2500;
constexpr long  secBound=  5000;
constexpr long  thiBound=  7500;
constexpr long  fouBound= 10000;


std::mutex myMutex;

void monteCarlo(volatile double *pi, int interval){
    int  i; 
    double rand_x, rand_y, origin_dist; 
    int circle_points = 0, square_points = 0; 
  
    // Initializing rand() 
    srand(time(NULL)); 
  
    // Total Random numbers generated = possible x 
    // values * possible y values 
    for (i = 0; i < (interval * interval); i++) { 
  
        // Randomly generated x and y values 
        rand_x = double(rand() % (interval + 1)) / interval; 
        rand_y = double(rand() % (interval + 1)) / interval; 
  
        // Distance between (x, y) from the origin 
        origin_dist = rand_x * rand_x + rand_y * rand_y; 
  
        // Checking if (x, y) lies inside the define 
        // circle with R=1 
        if (origin_dist <= 1) 
            circle_points++; 
  
        // Total number of points generated 
        square_points++; 
  
        // estimated pi after this iteration 
       // std::lock_guard<std::mutex> myLock(myMutex);
        *pi = double(4 * circle_points) / square_points;         
    }

}






int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    int samples = 1000;
    
    int *randValues = new int [size];


   for ( long  i=0 ; i< size ; ++i) randValues[i] = 10;   
    
   
    auto monitorada_handle = fpga->allocBuffer(CL(1));
    auto pi= reinterpret_cast<volatile double*>(monitorada_handle->c_type());
    
    assert (NULL != pi);
//    *pi = 0;
    auto samples_buffer_handle = fpga->allocBuffer(samples*sizeof(samples_t));
    auto samples_buffer = reinterpret_cast<volatile samples_t *>(samples_buffer_handle->c_type());
 
    auto status_handle = fpga->allocBuffer(CL(1));
    auto status = reinterpret_cast<volatile status_t *>(status_handle->c_type());

    assert(NULL != status);

    csrs.writeCSR(0, intptr_t(pi)/64);
    csrs.writeCSR(1, intptr_t(samples_buffer)/64);
    csrs.writeCSR(2, intptr_t(status)/64);
    csrs.writeCSR(3, samples);
    csrs.writeCSR(4, 200);
  
    
    std::thread t1(monteCarlo,pi,100);
    //std::thread t2(monteCarlo,pi,1000);
    //std::thread t3(monteCarlo,pi,1000);
    //std::thread t4(monteCarlo,pi,1000); 
    
    
    t1.join();
    //t2.join();
    //t3.join();
    //t4.join();    
    //status->done = 1;
    
    
    /*
    struct timespec pause;
    pause.tv_sec = 0;//(fpga->hwIsSimulated() ? 1 : 0);
    pause.tv_nsec = 1;

    while (status->done == 0 ){
        nanosleep(&pause, NULL);
    };*/
    
    
    std::terminate;
    
  // for(int i = 0; i < samples; i++){
  //      std::cout << i <<" Tempo: " << samples_buffer[i].time << " Old data: " << (double) samples_buffer[i].oldData << " New Data: " << (double) samples_buffer[i].newData  << std::endl;
        
  //  }
    
    delete fpga;
    
    return 0;

}
