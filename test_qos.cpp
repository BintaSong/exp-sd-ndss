#include "qos/Client.h"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {

    Server server(false, 120000);
    Client client(&server, 120000, 200);

    // client.insert("test", 5, false);
    // client.insert("test", 6, false);
    // client.insert("test", 7, false);
    // client.remove("test", 6, false);
    
    // Utilities::startTimer(123);
    // for (int i = 0; i < 5; i++) {
    //     for (int j = 0; j < 10; j++)
    //     client.insert(std::to_string(i), j, false);
    // }
    // double t1 = Utilities::stopTimer(123);
    // cout<< t1/50 << " us" <<endl;
    // vector<int> res = client.search("0");

    // // for (auto item : res) {
    // //     cout << item << endl;
    // // }
    // cout << "done" << endl;

    // cout << "Test begins. " << endl;
    // unordered_map<std::string, std::string> testmap;
    // for (int i = 0; i < 1000; i++)
    // {
    //     testmap[std::to_string(i)] = "test";
    // }

    // Utilities::startTimer(110);
    // for (int i = 0; i < 1000; i++)
    // {
    //     std::string a = testmap[std::to_string(i)];
    // }
    // double at = Utilities::stopTimer(110);
    // cout<< "Access Time per update: "<< at/1000 << " us" <<endl;

    int len = 10000;
    Utilities::startTimer(len);
    //client.update(OP::INS, "test", 5, true);
    for (int i = 0; i < len; i++)
    {
        client.insert(std::to_string(len), i, true);
        if (i % 10 == 0){
            client.insert(std::to_string(len/10), i, true);
        }

        if (i % 100 == 0){
            client.insert(std::to_string(len/100), i, true);
        }

        if (i % 1000 == 0){
            client.insert(std::to_string(len/1000), i, true);
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
