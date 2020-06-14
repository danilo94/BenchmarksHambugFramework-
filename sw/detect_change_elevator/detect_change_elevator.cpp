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


int size = 20000; 
int disk_size = 200000; 
  
void CSCAN(int arr[], int *head) 
{ 
    int seek_count = 0; 
    int distance, cur_track; 
    vector<int> left, right; 
    vector<int> seek_sequence; 
  
    // appending end values 
    // which has to be visited 
    // before reversing the direction 
    left.push_back(0); 
    right.push_back(disk_size - 1); 
  
    // tracks on the left of the 
    // head will be serviced when 
    // once the head comes back 
    // to the beggining (left end). 
    for (int i = 0; i < size; i++) { 
        if (arr[i] < head) 
            left.push_back(arr[i]); 
        if (arr[i] > head) 
            right.push_back(arr[i]); 
    } 
  
    // sorting left and right vectors 
    std::sort(left.begin(), left.end()); 
    std::sort(right.begin(), right.end()); 
  
    // first service the requests 
    // on the right side of the 
    // head. 
    for (int i = 0; i < right.size(); i++) { 
        cur_track = right[i]; 
        // appending current track to seek sequence 
        seek_sequence.push_back(cur_track); 
  
        // calculate absolute distance 
        distance = abs(cur_track - head); 
  
        // increase the total count 
        seek_count += distance; 
  
        // accessed track is now new head 
        head = cur_track; 
    } 
  
    // once reached the right end 
    // jump to the beggining. 
    head = 0; 
  
    // Now service the requests again 
    // which are left. 
    for (int i = 0; i < left.size(); i++) { 
        cur_track = left[i]; 
  
        // appending current track to seek sequence 
        seek_sequence.push_back(cur_track); 
  
        // calculate absolute distance 
        distance = abs(cur_track - head); 
  
        // increase the total count 
        seek_count += distance; 
  
        // accessed track is now the new head 
        head = cur_track; 
    } 
  
    cout << "Total number of seek operations = " 
         << seek_count << endl; 
  
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

    srand(time(NULL));
    // request array 
    int arr[size];

    for (int i=0; i<size; i++){
        arr[i] = rand()%200;
    }

    
    thread t1(CSCAN,arr,monitorada);


    t1.join();

    
    for (int i=0; i<samples; i++){
      std::cout << "Tempo: " << samples_buffer[i].time <<"Dado antigo: "<<samples_buffer[i].oldData << "  Dado novo:  "<< samples_buffer[i].newData << std::endl;
    }
    
    
    delete fpga;
    
    return 0;
}
