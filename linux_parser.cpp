#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  string key, value, line;
  long total, available;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "MemTotal:")
      {
        total = std::stol(value);
      }
      else if (key == "MemAvailable:")
      {
          available = std::stol(value);

          if (total != 0)
            return static_cast<float>(total - available) / total;
      }
    }
  }

  return 0.0; 
}

long LinuxParser::UpTime() 
{
  string uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  return stoi(uptime);
}

enum { UserTime = 0, Nice, System, Idle, Iowait, Irq, Softirq, Steal, Guest, GuestNice };

vector<int> LinuxParser::ReadJiffies()
{
  string cpu;
  int value;
  vector<int> result;  
  string line;

  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line))
    {
      std::istringstream linestream(line);
      linestream >> cpu;
      if (cpu == "cpu")
      {
        for (int i = 0; i < 10; i++) {
          linestream >> value;
          result.emplace_back(value);
        }
      }
    }
  }
  return result;
}

vector<string> LinuxParser::ReadJiffies(int pid)
{
  string cpu;
  string value;
  vector<string> result;  
  string line;

  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    if (std::getline(stream, line))
    {
      std::istringstream linestream(line);
      for (int i = 0; i < 52; i++) {
          linestream >> value;
          result.emplace_back(value);
        }
    }
  }
  return result;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 

  return ActiveJiffies() + IdleJiffies(); 
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  enum { utime = 14-1, stime = 15-1 };

  auto value = ReadJiffies(pid);

  return std::stol(value.at(utime)) + std::stol(value.at(stime)); 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto value = ReadJiffies();

  auto user = value.at(UserTime) + value.at(Nice);
  auto system = value.at(System) + value.at(Irq) + value.at(Softirq) + value.at(Steal);

  return user + system;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { 
  auto value = ReadJiffies();

  return  value.at(Idle) + value.at(Iowait); 
}

// Read and return CPU utilization
// Trying to follow: https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
vector<string> LinuxParser::CpuUtilization() { 
  vector<int> pids = LinuxParser::Pids(); 
  vector<string> result;

  long total = Jiffies();
  result.emplace_back(std::to_string(static_cast<float>(ActiveJiffies()) / total));

  for (auto pid : pids) {
    float percent = static_cast<float>(ActiveJiffies(pid)) / total; 
    result.emplace_back(std::to_string(percent));
  }

  return result; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { return Pids().size(); }

// Read and return the number of running processes
int LinuxParser::RunningProcesses() { 
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line))
    {
      std::istringstream linestream(line);
      linestream >> key >> value;
      if (key == "procs_running") {
        return std::stoi(value);
      }
    }
  }
  return 0; 
}

string LinuxParser::Command(int pid, int width) { 
  string cmd;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cmd;
  }

  // -- fill out width of display with spaces to clean up prior text...
  string cleanup = "                                               ";

  return cmd.substr(0, cmd.length() - 1) + cleanup.substr(0, width - cmd.length() - 1);
}

string LinuxParser::StatusHelperParseValue(int pid, string keyParam)
{
  string key, value;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key >> value;    
      if (key == keyParam)
      {
        return value;
      }
    }
  }
  return string();
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  return StatusHelperParseValue(pid, "VmSize:");
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  return StatusHelperParseValue(pid, "Uid:");
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line;
  string name, passwd, uidPasswd;

  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> passwd >> uidPasswd;
      if (uid == uidPasswd)
      {
        return name;      
      }
    }
  }
  
  return string();
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string value;
  string line;
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  long uptime = LinuxParser::UpTime();

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    value = "";
    for (int i = 1; i <= 22; i++)
      linestream >> value;
    if (value != "")
    { 
      return uptime - std::stol(value) / sysconf(_SC_CLK_TCK);
    }
  }

  return 0;
}