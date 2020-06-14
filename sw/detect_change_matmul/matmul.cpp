    #include <iostream>
    using namespace std;
    #define MATSIZE 800
    
        void matmul (int a[][MATSIZE], int b[][MATSIZE], int c[][MATSIZE]){
            for(int i = 0; i < MATSIZE; ++i){
                for(int j = 0; j < MATSIZE; ++j){
                    for(int k = 0; k < MATSIZE; ++k){
                        c[i][j] += a[i][k] * b[k][j];
                    }
                }
            }
        }
    
    
    int main()
    {
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
            matmul(a,b,mult);
        return 0;
    }
    
    
