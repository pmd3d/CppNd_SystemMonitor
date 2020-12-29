#include <string>
#include <sstream>
#include <iomanip>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) { 
    int hours = seconds / (60 * 60);
    int remainder = seconds % (60 * 60);
    int minutes = remainder / 60; 
    remainder = remainder % 60; 

    std::ostringstream ss;
    
    ss << std::setfill('0') 
       << std::setw(2) << hours << ":" 
       << std::setw(2) << minutes << ":" 
       << std::setw(2) << remainder;
    
    return ss.str();
}