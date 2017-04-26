#include <iostream>
#include <glog/logging.h>

using namespace google;
using namespace std;

int main() {

    InitGoogleLogging("testglog.exe");
    SetLogDestination(GLOG_INFO, "./info");
    LOG(INFO) << "abc" << endl;

    return 0;
}