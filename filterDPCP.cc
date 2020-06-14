
#include <vector>
#include <iostream>

using namespace std;

#include "filterDPCP.h"

// -------------------------------------------------------
// filter tasks
// -------------------------------------------------------

void showFilter(const FilterDPCP &f)
{
  switch (f.tm) {
    case TODAY: cout << "today ";
      break;
    case WEEK: cout << "week ";
      break;
    case MONTH: cout << "month ";
      break;
    case DAYS: cout << "days " << f.days << " ";
      break;
    case ALL_INCOMPLETED: cout << "all ";
      break;
    case ALL: cout << "ALL ";
      break;
  }
  
  if (f.plusPri != "") cout << f.plusPri << " ";
  if (f.minusPri != "") cout << "-" << f.plusPri << " ";
  
  for (unsigned i=0;i<f.plusCtx.size();i++)
    cout << f.plusCtx[i] << " ";
  for (unsigned i=0;i<f.minusCtx.size();i++)
    cout << "-" << f.minusCtx[i] << " ";
  for (unsigned i=0;i<f.plusPrj.size();i++)
    cout << f.plusPrj[i] << " ";
  for (unsigned i=0;i<f.minusPrj.size();i++)
    cout << "-" << f.minusPrj[i] << " ";

  if (f.text != "") cout << "text=" << f.text << " ";
  
  cout << endl;
}


bool containsString(vector<string> v,string s)
{
  for (unsigned i=0;i<v.size();i++)
    if (v[i] == s) return true;
  return false;
}

bool filterTaskDPCP(const Task &t)
{
  extern FilterDPCP fdpcp;

  if (t.completed && fdpcp.tm != ALL) 
    return false;
    
  // filter date
  if (t.date_due == "" && (fdpcp.tm != ALL && fdpcp.tm != ALL_INCOMPLETED))
    return false;
    
  if (t.date_due != "" && (fdpcp.tm != ALL && fdpcp.tm != ALL_INCOMPLETED) && t.date_due > fdpcp.date)
    return false;
    
    
  // filter priority
  if (fdpcp.plusPri != "" || fdpcp.minusPri != "")
  {
    if (t.priority != "")
    { 
      if (fdpcp.plusPri != ""  && t.priority != fdpcp.plusPri) return false;
      if (fdpcp.minusPri != "" && t.priority == fdpcp.minusPri) return false;
    }
    else 
    { 
      if (fdpcp.plusPri != "") return false;
    }
  }
  
  // filter contexts
  if (fdpcp.minusCtx.size()>0)
    for (unsigned i=0;i<t.contexts.size();i++)
      if (containsString(fdpcp.minusCtx,t.contexts[i])) return false;
    
  if (fdpcp.plusCtx.size()>0)
  {
    bool ct=false;
    for (unsigned i=0;i<t.contexts.size() && !ct;i++)
      ct = containsString(fdpcp.plusCtx,t.contexts[i]);
    if (!ct) return false;
  }
  
  // filter projects
  if (fdpcp.minusPrj.size()>0)
    for (unsigned i=0;i<t.projects.size();i++)
      if (containsString(fdpcp.minusPrj,t.projects[i])) return false;
    
  if (fdpcp.plusPrj.size()>0)
  {
    bool ct=false;
    for (unsigned i=0;i<t.projects.size() && !ct;i++)
      ct = containsString(fdpcp.plusPrj,t.projects[i]);
    if (!ct) return false;
  }
  
  // filter text
  if (fdpcp.text != "" && t.text.find(fdpcp.text) == string::npos)
    return false;
  
  return true;  
}

