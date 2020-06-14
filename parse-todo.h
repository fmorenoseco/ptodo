
#ifndef _parse_todo
#define _parse_todo

#include <vector>
#include <string>

using namespace std;

#include "task.h"
extern vector<Task> allTasks;

void createTaskFromLine(string l);
bool readTodoFile(string filename);

#endif
