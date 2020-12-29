#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { 
    if (Total() == 0)
        return 0.0;
    else
    {
        return static_cast<float>(Active())/Total();
    }
}

string Process::Command(int width) { return LinuxParser::Command(Pid(), width); }

// Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid()); }

// Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const { 
    return this->CpuUtilization() < a.CpuUtilization(); 
}

int Process::Active() const { 
    return active_; 
}

int Process::Total() const { 
    return total_; 
}
