
#include <iostream>

#include "task.h"
#include "filterDPCP.h"
#include "parse-todo.h"
#include "listing.h"


LST_TYPE lstType=COMPACT;
extern string today,tomorrow;

//----------------------------------------------
//  listing functions
//----------------------------------------------
void showTaskDBG(const Task &t)
{
    cout << "Task " << ":" 
         << t.text << endl
         << " [completed=" << t.completed 
         << "][completion date=" << t.date_completed
         << "][creation date=" << t.date_created
         << "][priority=" << t.priority <<  "]" << endl;

    cout << " Contexts: ";
    for (unsigned j=0;j<t.contexts.size();j++)
      cout << t.contexts[j] << " ";
    cout << endl;

    cout << " Projects: ";
    for (unsigned j=0;j<t.projects.size();j++)
      cout << t.projects[j] << " ";
    cout << endl;
    
    cout << " [due date=" << t.date_due
         << "][start date=" << t.date_start
         << "][recurrence=" << t.recurrence << "]" << endl;
}

void showTaskCOMPACT(const Task &t)
{
    if (t.completed) cout << "x ";
    if (t.date_due!="")
    {
      if (t.date_due < today) cout << "-";
      else if (t.date_due == today) cout << "*";
      else if (t.date_due == tomorrow) cout << "+";
      else cout << " ";
      cout << "[" << t.date_due << "] ";
    }
    else
      cout << " [          ] ";
      
    string pri=(t.priority == "" ? "   " : t.priority);
    cout << pri << " "
         << t.text << endl;
}


void showTaskORIG(ostream &os, const Task &t)
{
    if (t.completed) os << "x ";
    if (t.priority != "") 
      os << t.priority << " ";
      
    if (t.completed) os << t.date_completed << " ";
    
    if (t.date_created != "")
      os << t.date_created << " ";

    os << t.text << endl;
}



void showTask(const Task &t)
{
  switch (lstType) {
      case DBG:
         showTaskDBG(t);
         break;
      case COMPACT:
         showTaskCOMPACT(t);
         break;
      case ORIG:
         showTaskORIG(cout,t);
         break;
  }
}

void showAllTasks(vector<Task *> vt)
{
  for (unsigned i=0;i<vt.size();i++)
    showTask(*vt[i]);
}

void showTasksFilter(vector<Task *> vt,bool (*fltTsk)(const Task &))
{
  for (unsigned i=0;i<vt.size();i++)
    if (fltTsk(*vt[i]))
      showTask(*vt[i]);
  
}

vector<unsigned> nTasks;

void showTasksFilterNumbered(vector<Task *> vt,bool (*fltTsk)(const Task &))
{
  int count=1;
  nTasks.clear();
  for (unsigned i=0;i<vt.size();i++)
    if (fltTsk(*vt[i]))
    {
      if (count<10) cout << "{ " << count << "}";
      else          cout << "{" << count << "}";
      showTask(*vt[i]);
      nTasks.push_back(i);
      count++;
    }
}

