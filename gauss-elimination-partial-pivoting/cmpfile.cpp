#include <fstream>
#include <sstream>
#include <iostream>
#include <math.h>

#define EPSILON 0.0000001

using namespace std;


int main(int argc, char** argv) {
    if(argc != 3) {
        cout << "input two file names as argument" << endl;
        return -1;
    }

    const char *filename1 = argv[1];
    const char *filename2 = argv[2];
    
    ifstream fin1(filename1);
    ifstream fin2(filename2);

    int r1, c1, M1, N1, n1, r2, c2, M2, N2, n2;
    double val1, val2;

    int line_n = 1;

    fin1 >> M1 >> N1 >> n1;
    fin2 >> M2 >> N2 >> n2;
    if(M1 != M2 || N1 != N2 || n1 != n2) {
        cout << "different at line " << line_n << endl;
        return 0;
    }
    
    line_n++;

    while(1) {
        fin1 >> r1 >> c1 >> val1;
        fin2 >> r2 >> c2 >> val2;

        if(r1 != r2 || c1 != c2 || fabs(val1 - val2) > EPSILON) {
            cout << "different at line " << line_n << endl;
            return 0;
        }
        
        if(r1 == 0) {
            cout << "identical files" << endl;
            return 0;
        }

        line_n++;
    }

    return 0;
}


