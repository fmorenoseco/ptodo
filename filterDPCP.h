
#ifndef _filter_
#define _filter_

#include <string>
#include <vector>

using namespace std;

#include "task.h"

/* options:
    t (today)
    w (week [default])
    m (month (30 days))
    d days
    a (all (incompleted))
    A (all)
    
    (A)
    -(A)
    
    @Context  
    -@Context
    
    +Project
    -+Project
    
    'text'
*/

enum TimePeriod {
  TODAY,
  WEEK,
  MONTH,
  DAYS,
  ALL_INCOMPLETED,
  ALL
};

struct FilterDPCP {

  TimePeriod tm;
  int days;       // for -d
  string date;    // computed date, or "" for ALL*

  string plusPri;
  string minusPri;
  
  vector<string> plusCtx;
  vector<string> minusCtx;

  vector<string> plusPrj;
  vector<string> minusPrj;
  
  string text;   // task.text contains
};

bool containsString(vector<string> v,string s);
bool filterTaskDPCP(const Task &t);
void showFilter(const FilterDPCP &f);

#endif
