
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

#include "parse-todo.h"

/*
vector<string> allContexts;
vector<string> allProjects;
*/

vector<Task> allTasks;

extern int nline;
extern void error(string msg);

/* (comented out for future uses)
// -------------------------------------------------------
// manage contexts and projects
// -------------------------------------------------------
void newContext(string c)
{
  for (unsigned i=0;i<allContexts.size();i++)
    if (allContexts[i]==c) return;
    
  allContexts.push_back(c);
}

void newProject(string c)
{
  for (unsigned i=0;i<allProjects.size();i++)
    if (allProjects[i]==c) return;
    
  allProjects.push_back(c);
}
*/

//------------------------------------------
//  read and process file 
//------------------------------------------
bool startsWith(string l,unsigned i,string st)
{
  unsigned j=i;
  
  while (j<l.length() && j-i<st.length())
    if (l[j] != st[j-i]) return false;
    else j++;
  
  if (j-i<st.length())
    return false;
  
  return true;
}

bool extractDate(string l,unsigned i,string &date)
{
  unsigned j=i;
  string dateformat="DDDD-DD-DD";
  string dat;
  
  while (j<l.length() && j-i<dateformat.length())
    if (dateformat[j-i]=='D') {
      if (!isdigit(l[j])) return false;
      else {
        dat += l[j];
        j++;
      }
    }
    else if (l[j] != dateformat[j-i]) return false;
    else {
      dat += l[j];
      j++;
    }
  
  if (j-i<dateformat.length())
    return false;
  
  date = dat;
  return true;
}

bool extractPriority(string l,unsigned i,string &priority)
{
  unsigned j=i;
  string prioformat="(L)";
  string prio;
  
  while (j<l.length() && j-i<prioformat.length())
    if (prioformat[j-i]=='L') {
      if (!isupper(l[j])) return false;
      else {
        prio += l[j];
        j++;
      }
    }
    else if (l[j] != prioformat[j-i]) return false;
    else {
      prio += l[j];
      j++;
    }
  
  if (j-i<prioformat.length())
    return false;
  
  priority = prio;
  return true;
}

string eraseToken(string l,string &token)
{
  unsigned i=0;
  
  token="";
  while (i<l.length() && l[i]!=' ')
  {
    token += l[i];
    i++;
  }
  
  while (i<l.length() && l[i]==' ') i++;  // ignore blanks
  return l.substr(i);
}

string eraseToken(string l)
{
  unsigned i=0;
  
  while (i<l.length() && l[i]!=' ')
  {
    i++;
  }
  
  while (i<l.length() && l[i]==' ') i++;  // ignore blanks
  return l.substr(i);
}

void extractCPK(Task &t,string l)
{
  stringstream ss;
  string word;
  
  ss << l;
  while (ss >> word)
  {
    string d;
    if (word.length()>1 && word[0]=='@')
    {
      //newContext(word);
      t.contexts.push_back(word);
    }
    else if (word.length()>1 && word[0]=='+')
    {
      //newProject(word);
      t.projects.push_back(word);
    }
    else if (word.length()>1 && startsWith(word,0,"due:"))
    {
      if (!extractDate(word,4,t.date_due))
         error("wrong due date");
    }
    else if (word.length()>1 && startsWith(word,0,"t:"))
    {
      if (!extractDate(word,2,t.date_start))
        error("wrong start/threshold date");
    }
    else if (word.length()>1 && startsWith(word,0,"rec:"))
    {
      t.recurrence = word.substr(4);
    }
  }
  
}

string cleanCR(string l)
{
  string s;
  
  for (unsigned i=0;i<l.length();i++)
    if (l[i]!='\r')
      s += l[i];
  return s;
}

void createTaskFromLine(string l)
{
  Task t;
  string s;
  
  t.completed = false;
  if (startsWith(l,0,"x "))
  {
    t.completed = true;
    l = eraseToken(l);
  }
  
  if (extractPriority(l,0,t.priority))
    l = eraseToken(l);

  if (t.completed)
  {
    if (extractDate(l,0,t.date_completed))
        l = eraseToken(l);
  }
        
  if (extractDate(l,0,t.date_created))
     l = eraseToken(l);

  // text  @context +project key:value
  t.text = l; //cleanCR(l);
  extractCPK(t,l);
  allTasks.push_back(t);
}

bool readTodoFile(string filename)
{
  bool ok=false;
  ifstream f;
    
  f.open(filename,ios::in);
  if (f.is_open())
  {
    string line;
      
    while (getline(f,line))
    {
      createTaskFromLine(line);
      nline++;
    }
    f.close();
    ok = true;   
  }
  else
     cerr << "Error, can't open file '" << filename << "'" << endl;
    
  return ok;
}



