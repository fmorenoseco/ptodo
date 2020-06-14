
#ifndef _task
#define _task

#include <string>
#include <vector>

using namespace std;

struct Task {
  bool completed;
  string date_completed;
  string date_created;
  
  string priority;
  string text;
  
  vector<string> contexts;
  vector<string> projects;
  
  string date_due;
  string recurrence;
  string date_start;
};

#endif

