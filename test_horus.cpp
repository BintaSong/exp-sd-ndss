#include "horus/Horus.h"
#include "utils/Utilities.h"
using namespace std;

int main(int argc, char** argv) {
    Horus horus(false, 1100);

 /*   horus.insert("test", 5);
    horus.insert("test", 6);
    horus.insert("test", 7);
    horus.insert("test", 8);
    horus.remove("test", 7);
    vector<int> res = horus.search("test");

    for (auto item : res) {
        cout << item << endl;
    }
*/
    for (int i = 0; i < 1; i++) {
        for (int j = 0; j < 1000; j++)
        horus.insert(std::to_string(i), j);
    }

     cout << "setup done." << endl;
     vector<int> res = horus.search("0");
    
    return 0;
}
