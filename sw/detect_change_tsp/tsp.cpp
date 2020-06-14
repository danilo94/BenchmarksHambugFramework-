#include <bits/stdc++.h>
using namespace std;
#define vr 10
int TSP(int grph[][vr], int p){ // implement traveling Salesman Problem.
   vector<int> ver; //
   for (int i = 0; i < vr; i++)
      if (i != p)
         ver.push_back(i);
         int m_p = INT_MAX; // store minimum weight of a graph
   do {
      int cur_pth = 0;
      int k = p;
      for (int i = 0; i < ver.size(); i++) {
         cur_pth += grph[k][ver[i]];
         k = ver[i];
      }
      cur_pth += grph[k][p];
      m_p = min(m_p, cur_pth); // to update the value of minimum weight
   }
   while (next_permutation(ver.begin(), ver.end()));
   return m_p;
}
int main() {
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
   cout<< "\n The result is: "<< TSP(grph, p) << endl;
   return 0;
}
