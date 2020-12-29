#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  int Pid() const;                             
  std::string User();                    
  std::string Command(int width);                 
  float CpuUtilization() const;                
  std::string Ram();                     
  long int UpTime();                     
  bool operator<(Process const& a) const;

  Process(int pid, int active, int total) 
  : pid_(pid), active_(active), total_(total)
  { };

  int Active() const;
  int Total() const;
  
 private:
  int pid_;
  int active_;
  int total_;
};

#endif