
#ifndef _listing_
#define _listing_

#include "task.h"

enum LST_TYPE {
  DBG, ORIG, COMPACT
};

extern LST_TYPE lstType;  // =COMPACT

void showTaskORIG(ostream &os, const Task &t);
void showAllTasks(vector<Task *> vt);
void showTasksFilter(vector<Task *> vt,bool (*fltTsk)(const Task &));
void showTasksFilterNumbered(vector<Task *> vt,bool (*fltTsk)(const Task &));


#endif
