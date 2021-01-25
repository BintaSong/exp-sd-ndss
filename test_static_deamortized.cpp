#include "static/DeAmortized.h"
using namespace std;

int main(int argc, char** argv) {

    DeAmortized client(false, 100, 11000);

    cout << "Update begins. " << endl;
    
    int len = 10000;
    Utilities::startTimer(len);
    //client.update(OP::INS, "test", 5, true);
    for (int i = 0; i < len; i++)
    {
        client.update(OP::INS, std::to_string(len), i, false);
        if (i % 10 == 0){
            client.update(OP::INS, std::to_string(len/10), i, false);
        }

        if (i % 100 == 0){
            client.update(OP::INS, std::to_string(len/100), i, false);
        }

        if (i % 1000 == 0){
            client.update(OP::INS, std::to_string(len/1000), i, false);
        }
    }
    cout<< "Com size: "<< client.getTotalUpdateCommSize() << " bytes" <<endl;

    double ut = Utilities::stopTimer(len);
    cout<< "Len: "<< len << ", Update Time per update: "<< ut/len << " us" <<endl;

    Utilities::startTimer(len);
    vector<int>  res = client.search(std::to_string(len));
    double st = Utilities::stopTimer(len);
    cout<< "Len: "<< res.size() << ", Search Time per id: "<< st/res.size()/1000.0 << " ms" <<endl;

     Utilities::startTimer(len);
    res = client.search(std::to_string(len/10));
    st = Utilities::stopTimer(len);
    cout<< "Len: "<< res.size() << ", Search Time per id: "<< st/res.size()/1000.0 << " ms" <<endl;

    Utilities::startTimer(len);
    res = client.search(std::to_string(len/100));
    st = Utilities::stopTimer(len);
    cout<< "Len: "<< res.size() << ", Search Time per id: "<< st/res.size()/1000.0 << " ms" <<endl;

    Utilities::startTimer(len);
    res = client.search(std::to_string(len/1000));
    st = Utilities::stopTimer(len);
    cout<< "Len: "<< res.size() << ", Search Time per id: "<< st/res.size()/1000.0 << " ms" <<endl;
    
    //cout<< sizeof(double)<<endl; 
    return 0;
}
