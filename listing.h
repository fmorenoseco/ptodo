
#ifndef _listing_
#define _listing_

#include "task.h"

/*
 Listing type:
  DBG : debug 
  ORIG : todo.txt format
  COMPACT : 
    [-*+] - past due, * due today, + due tomorrow
    [due date]
    task text (without creation date nor completion date)
    
    Example:  
    
    *[2020-06-14]    Add documentation to listing.h due:2020-06-14 @ptodo
    +[2020-06-15]    Commit changes @ptodo due:2020-06-15
    
*/

enum LST_TYPE {
  DBG, ORIG, COMPACT
};

extern LST_TYPE lstType;  // =COMPACT  (default)

// prints a task (in original format) to an output stream
void showTaskORIG(ostream &os, const Task &t);

// 
void showAllTasks(vector<Task *> vt);

// given a filter function, show tasks (w/o number) (yeah, a pointer to a function!)
void showTasksFilter(vector<Task *> vt,bool (*fltTsk)(const Task &));
void showTasksFilterNumbered(vector<Task *> vt,bool (*fltTsk)(const Task &));


#endif
