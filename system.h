#ifndef SYSTEM_H
#define SYSTEM_H

#include <string>
#include <vector>
#include <map>

#include "process.h"
#include "processor.h"

struct ProcessHistory
{
  long active;
  long total;
};

class System {
 public:
  Processor& Cpu();                 
  std::vector<Process>& Processes();
  float MemoryUtilization();        
  long UpTime();                    
  int TotalProcesses();             
  int RunningProcesses();           
  std::string Kernel();             
  std::string OperatingSystem();    

 private:
  Processor cpu_ = {};
  std::vector<Process> processes_ = {};
  std::map<int, std::vector<ProcessHistory>> prior_ = {}; 
};


#endif