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
#define vr 10
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

  
int TSP(int grph[][vr],int *monitorada){ // implement traveling Salesman Problem.
    
        struct timespec pause;
    pause.tv_sec = 0;//(fpga->hwIsSimulated() ? 1 : 0);
    pause.tv_nsec = 2500;
    
    vector<int> ver; //
   for (int i = 0; i < vr; i++)
      if (i != 0)
         ver.push_back(i);
         monitorada[0] = INT_MAX; // store minimum weight of a graph
   do {
      int cur_pth = 0;
      int k = 0;
      for (int i = 0; i < ver.size(); i++) {
         cur_pth += grph[k][ver[i]];
         k = ver[i];
      }
      nanosleep(&pause, NULL);
      cur_pth += grph[k][0];
      monitorada[0] = min(monitorada[0], cur_pth); // to update the value of minimum weight
   }
   while (next_permutation(ver.begin(), ver.end()));
   std::cout << monitorada[0]<<std::endl;
   return monitorada[0];
}


int main(int argc, char *argv[]){

    OPAE_SVC_WRAPPER *fpga = new  OPAE_SVC_WRAPPER("9f81ba12-1d38-4cc7-953a-dafeef45065b");
    assert(fpga->isOk());
    CSR_MGR csrs(*fpga);
    
    long samples = 100;
    
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

   int grph[][vr] = { 
      { 10,15,0,0,12,0,2,4,7,2}, //values of a graph in a form of matrix
      { 1,0,10,0,16,0,2,4,7,2},
      { 0,12,0,15,0,0,1,2,5,1},
      { 3,0,17,0,1,1,6,7,9,1},
      { 0,13,0,1,0,0,12,3,6,6},
      { 0,2,0,18,0,0,2,5,8,2},
      { 5,7,13,21,33,0,2,5,7,2},
      { 1,3,5,6,7,0,2,4,7,2},
      { 1,2,6,8,6,4,5,7,9,0},
      {1,5,6,77,3,322,5,3,22,21}
   };
    int p =0 ;
    std::thread t1(TSP,grph,monitorada);


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
