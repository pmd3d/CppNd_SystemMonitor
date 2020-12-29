#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include <map>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using std::map;

// Return the system's CPU
Processor& System::Cpu() { return cpu_; }

vector<Process>& System::Processes() {

    processes_.clear();

    for (auto pid : LinuxParser::Pids()) {
        auto found = prior_.find(pid);

        ProcessHistory current { active: LinuxParser::ActiveJiffies(pid), 
                                 total: LinuxParser::Jiffies() };

        if (found == prior_.end()) {
            // initialize history...
            Process process(pid, 0, 0);
            vector<ProcessHistory> history { current };

            prior_.emplace(pid, history);
            processes_.emplace_back(process);
        }
        else {
            // -- clean out old ones...
            while (found->second.size() > 20)
            {
                found->second.erase(found->second.begin());
            }

            // -- add new one...
            found->second.emplace_back(current);      

            // average out prior deltas...
            vector<ProcessHistory> history = found->second;

            int totalPriorActive = 0;
            int totalPriorTotal = 0;
            for (uint32_t i = 1; i < history.size(); i++) {
                totalPriorActive += history.at(i).active - history.at(i - 1).active; 
                totalPriorTotal += history.at(i).total - history.at(i - 1).total;
            }
            float avgActive = totalPriorActive / history.size();
            int avgTotal = totalPriorTotal / history.size();

            Process process(pid, avgActive, avgTotal);
            processes_.emplace_back(process);
        }
    }

    std::sort(processes_.begin(), processes_.end());
    std::reverse(processes_.begin(), processes_.end());

    return processes_;
}

std::string System::Kernel() { return LinuxParser::Kernel(); }

float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

long int System::UpTime() { return LinuxParser::UpTime(); }