
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <sys/stat.h>

using namespace std;

#include "dates.h"
#include "parse-todo.h"
#include "filterDPCP.h"
#include "listing.h"

// ------------- Global variables -------------------------
string version="1.0 2020-06-14";
string today,tomorrow;
FilterDPCP fdpcp;

string filename;
struct stat infoFile;
bool createBackup;

int nline=1;

enum SortType { NONE, DATE, ALPHA };

SortType cst;
vector<Task *> ordTasks;


// listing.cc
extern vector<unsigned> nTasks;
extern LST_TYPE lstType;  // =COMPACT

// parse-todo.cc
extern vector<Task> allTasks;


// --------------------------------------------------------
void usage(char *prog)
{
  cerr << "Usage:\n interactive mode: " << prog << " <todo.txt file>" << endl;
  cerr << " non-interactive: " << prog << " [OPTIONS] <todo.txt file>" << endl
       << "   -a 'new task'   - backup + add task + write\n"
       << "                     (not compatible with -f)\n"
       << "   -s [dan]        - sort by (d)ue date, (a)lphabetically or (n)one [default]\n"
       << "                     if used...\n"
       << "                         .. with -a : backup + sort + add task + write\n"
       << "                         .. with -f : sort + filter (no backup, no write)\n"
       << "                         .. alone :   backup + sort + write\n"
       << "   -f 'filter'       [actived by default: week]\n"
       << "                     (not compatible with -a)\n"
       << "   -o [oc]         - output format (o)riginal or (c)ompact [default]\n"
       << "                     only with -f, otherwise it's ignored\n"
       << "   (at least one of -a, -f, -s)\n"
       << endl
       ;
}

void error(string msg)
{
  cerr << "Error in line " << nline << ": " << msg << endl;
}
//-----------------------------------------------------------------------
void helpFilter()
{
  cout << 
   " filter options: \n"
   "    t/today             \n"
   "    w/week [default]  \n"
   "    m/month (30 days)  \n"
   "    d/days nn  \n"
   "    a/all (incompleted)  \n"
   "    A/ALL (all)  \n"
   "      \n"
   "    (A)     (removes -(P))  \n"
   "    -(A)    (removes (P))  \n"
   "    #()     (removes (P))  \n"
   "    #-()    (removes -(P))  \n"
   "      \n"
   "    @Context    \n"
   "    -@Context  \n"
   "    #@Context  (removes @Context)  \n"
   "    #-@Context (removes -@Context)  \n"
   "    #@         (removes all @Context)  \n"
   "    #-@        (removes all -@Context)  \n"
   "      \n"
   "    +Project  \n"
   "    -+Project  \n"
   "    #+Project   (removes +Project)  \n"
   "    #-+Project  (removes -+Project)  \n"
   "    #+          (removes all +Project)  \n"
   "    #-+         (removes all -+Project)  \n"
   "      \n"
   "    'text'  (task.text contains text)  \n"   
   "    #''     (removes text)  \n"   
   ;
}

//-------------------------------------------------

void errOrder(string order)
{
  cerr << "Unknown order <" << order << ">" << endl;
}


void help(string o)
{
  if (o == "h f")
    helpFilter();
  else
  {
    cout << "Help:\n" 
            " h <topic>\n"
            " f <filter>\n"
            " s [adn] - sort (a)lphabetically, by (d)ue date [default] or (n)one\n"
            " A [ra] - archive completed (r)ecurrent/(a)ll\n"
            " e nn due:(yyyy-mm-dd|today|tomorrow)\n"
            " e nn rec:...\n"
            " e nn @Context\n"
            " e nn -@Context\n"
            " e nn +Project\n"
            " e nn -+Project\n"
            " e nn -()  - remove priority\n"
            " e nn (P)  - sets priority\n"
            " c nn - complete task nn \n"
            " u nn - uncomplete task nn \n"
            " a new task ... \n"
            " q  - exit\n";
  }
}


//-----------------------------------------------------
// sort tasks by due date + priority, or alphabetically
//-----------------------------------------------------
bool cmpTasks(const Task *t1,const Task *t2)
{
  // completed tasks go to the end
  if (!t1->completed || !t2->completed)
  {
    if (t2->completed) return true;
    if (t1->completed) return false;
  }
  else {
     if (t1->date_completed == t2->date_completed)
       return t1->text < t2->text;
     else
       return t1->date_completed < t2->date_completed;
  }

//  if (debug) cout << "both incomplete, t1=<"<< t1->text << ">, t2=<" << t2->text << ">" << endl;
  if (t1->date_due!="" || t2->date_due!="") {
    // tasks with no due date go to the end
    if (t2->date_due=="") return true;
    if (t1->date_due=="") return false;

    if (t1->date_due!="" && t2->date_due!="") {
      if (t1->date_due<t2->date_due) return true;
      else if (t1->date_due>t2->date_due) return false;
    }
  }
  
  // same date or no due date both
  if (t1->priority == "" && t2->priority == "") 
      return t1->text < t2->text;
  if (t2->priority == "") return true;
  if (t1->priority == "") return false;
  return t1->priority < t2->priority;
}

bool cmpTasksAlpha(const Task *t1,const Task *t2)
{
  // completed tasks go to the end
  if (!t1->completed || !t2->completed)
  {
    if (t2->completed) return true;
    if (t1->completed) return false;
  }
  else {
     if (t1->date_completed == t2->date_completed)
       return t1->text < t2->text;
     else
       return t1->date_completed < t2->date_completed;
  }

  // compare priority + text
  if (t1->priority == "" && t2->priority == "") 
      return t1->text < t2->text;
  if (t2->priority == "") return true;
  if (t1->priority == "") return false;
  return t1->priority < t2->priority;
}


void sortTasks(vector<Task *> &vt, SortType st)
{
  vt.clear();
  
  for (unsigned i=0;i<allTasks.size();i++)
    vt.push_back(&allTasks[i]);

  switch (st) {
   case DATE:
     sort(vt.begin(),vt.end(),cmpTasks);
     break;
   case ALPHA:
     sort(vt.begin(),vt.end(),cmpTasksAlpha);
     break;    
   case NONE:
     // do nothing
     break;
  }
}

//-----------------------------------------------------------------------
// Filter
//-----------------------------------------------------------------------
void computeDate(FilterDPCP &f)
{
  // compute date
  int y,m,d;
    
  getToday(y,m,d);
  switch (f.tm) {
      case TODAY:
        f.date = today;
        break;
      case WEEK:
        addDays(y,m,d,7);
        f.date = dateToString(y,m,d);
        break;
      case MONTH:
        addDays(y,m,d,30);
        f.date = dateToString(y,m,d);
        break;
      case DAYS:
        addDays(y,m,d,f.days);
        f.date = dateToString(y,m,d);
        break;
      case ALL_INCOMPLETED:
      case ALL:
        f.date="";
        break;
  }
}

bool isPriority(string p,FilterDPCP &f)
{
  if (p.length()==3 && p[0]=='(' && p[2]==')' && isupper(p[1]))
  {
    f.plusPri = p;
    f.minusPri = "";
    return true;
  }
  else if (p == "#()")
  {
    f.plusPri = "";
    return true;
  }
  else if (p.length()==4 && p[0]=='-' && p[1]=='(' && p[3]==')' && isupper(p[2]))
  {
    f.minusPri = "(";
    f.minusPri += ((char)p[2]);
    f.minusPri += ")";
    f.plusPri = "";
    return true;
  }
  else if (p == "#-()")
  {
    f.minusPri = "";
    return true;
  }
  return false;
}

void addToVec(vector<string> &v,string s)
{
  for (unsigned i=0;i<v.size();i++)
    if (v[i]==s) return;
  v.push_back(s);
}

void erase(vector<string> &v,string s)
{
  for (unsigned i=0;i<v.size();i++)
    if (v[i]==s) 
    {
      v.erase(v.begin()+i);
      return;
    }
}

bool isContext(string p,FilterDPCP &f)
{
  if (p == "#@")
  {
    f.plusCtx.clear();
    return true;
  }
  else if (p == "#-@")
  {
    f.minusCtx.clear();
    return true;
  }
  else if (p.length()>=2 && p[0]=='@')
  {
    addToVec(f.plusCtx,p);
    return true;
  }
  else if (p.length()>=3 && p[0]=='#' && p[1]=='@')
  {
    erase(f.plusCtx,p.substr(1));
    return true;
  }
  else if (p.length()>=3 && p[0]=='-' && p[1]=='@')
  {
    addToVec(f.minusCtx,p.substr(1));
    return true;
  }
  else if (p.length()>=4 && p[0]=='#' && p[1]=='-' && p[2]=='@')
  {
    erase(f.minusCtx,p.substr(2));
    return true;
  }
  return false;
}

bool isProject(string p,FilterDPCP &f)
{
  if (p == "#+")
  {
    f.plusPrj.clear();
    return true;
  }
  else if (p == "#-+")
  {
    f.minusPrj.clear();
    return true;
  }
  else if (p.length()>=2 && p[0]=='+')
  {
    addToVec(f.plusPrj,p);
    return true;
  }
  else if (p.length()>=3 && p[0]=='#' && p[1]=='+')
  {
    erase(f.plusPrj,p.substr(1));
    return true;
  }
  else if (p.length()>=3 && p[0]=='-' && p[1]=='+')
  {
    addToVec(f.minusPrj,p.substr(1));
    return true;
  }
  else if (p.length()>=4 && p[0]=='#' && p[1]=='-' && p[2]=='+')
  {
    erase(f.minusPrj,p.substr(2));
    return true;
  }
  return false;
}

bool isText(string p,FilterDPCP &f)
{
  if (p == "#\'\'")
  {
    f.text="";
    return true;
  }
  else if (p[0]=='\'' && p[p.length()-1]=='\'')
  {
    f.text=p.substr(1,p.length()-2);
    return true;
  }
  return false;
}

bool nextWord(string l,unsigned &i,string &w)
{
  while (i<l.length() && l[i]==' ') i++;
  
  w="";
  if (i<l.length() && l[i]=='\'')
  {
    w += l[i];
    i++;
    while (i<l.length() && l[i]!='\'') {
      w += l[i];
      i++;
    }
    w += l[i];
    i++;
  }
  else
  {
    while (i<l.length() && l[i]!=' ') {
      w += l[i];
      i++;
    }
  }
  return w!="";
}

bool processFilter(string fline,FilterDPCP &f)
{
  bool opPeriod=false;
  bool ok=true;
  unsigned il=0;
  string op;

  f.days = 0;
     
  while (nextWord(fline,il,op)) 
  {
    if (op == "t" || op == "today")
    {
      if (!opPeriod)
      {
        f.tm = TODAY;
        opPeriod = true;
      }
      else
        ok=false;
    }
    else if (op == "w" || op == "week")
    {
      if (!opPeriod)
      {
        f.tm = WEEK;
        opPeriod = true;
      }
      else
        ok=false;
    }
    else if (op == "m" || op == "month")
    {
      if (!opPeriod)
      {
        f.tm = MONTH;
        opPeriod = true;
      }
      else
        ok=false;
    }
    else if (op == "a" || op == "all")
    {
      if (!opPeriod)
      {
        f.tm = ALL_INCOMPLETED;
        opPeriod = true;
      }
      else
        ok=false;
    }
    else if (op == "A" || op == "ALL")
    {
      if (!opPeriod)
      {
        f.tm = ALL;
        opPeriod = true;
      }
      else
        ok=false;
    }
    else if (op == "d" || op == "days")
    {
      if (!opPeriod)
      {
        string ndays;
        
        nextWord(fline,il,ndays);
        if (ndays.length()>0 && isdigit(ndays[0]))
        {
          f.tm = DAYS;
          f.days = stoi(ndays);
        }
        else
          ok=false;
      }
      else
        ok=false;
    }
    else if (isPriority(op,f))
    {
    }
    else if (isContext(op,f))
    {
    }
    else if (isProject(op,f))
    {
    }
    else if (isText(op,f))
    {
    }
    else
      ok=false;
  }
  if (ok)
    computeDate(f);
  else
    cerr << "Wrong filter <" << fline << ">" << endl;
  
  return ok;
}

//-----------------------------------------------------------------------
// Files
//-----------------------------------------------------------------------


void getFileInfo(struct stat &fi)
{
  stat(filename.c_str(),&fi);
}

string getFileMTime()
{
  struct tm * timeStruct = localtime(&infoFile.st_mtime);
  char h[10];
  sprintf(h,"%02d:%02d:%02d",timeStruct->tm_hour,timeStruct->tm_min,timeStruct->tm_sec);

  return tmToString(timeStruct) + " " + h ;
}

bool checkFileNotModified()
{
  struct stat f;
  
  getFileInfo(f);
  return f.st_mtime == infoFile.st_mtime;
}

// writes all tasks to file 'fn'
void writeFile(string fn)
{
  ofstream f;

  f.open(fn.c_str(),ios::out);
  if (f.is_open())
  {
    for (unsigned i=0;i<allTasks.size();i++)
    {
       showTaskORIG(f,allTasks[i]);
    }
    
    f.close();
  }
  if (fn == filename)
    getFileInfo(infoFile);   
}

// writes tasks in 'vt' to file 'fn'
void writeFile(vector<Task *> vt,string fn)
{
  ofstream f;

  f.open(fn.c_str(),ios::out);
  if (f.is_open())
  {
    for (unsigned i=0;i<vt.size();i++)
    {
       showTaskORIG(f,*vt[i]);
    }
    
    f.close();
  }
}

void reReadFile()
{
  allTasks.clear();
  readTodoFile(filename);
  getFileInfo(infoFile);
  sortTasks(ordTasks,cst);
}

bool cfnm(string op)
{
  if (!checkFileNotModified())
  {
    cerr << "File has changed, abort " << op << " and rereading..." << endl;
    reReadFile();
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------
// Backup
//-----------------------------------------------------------------------


void backup(string filename)
{
  time_t t = time(0); 
  struct tm * timeStruct = localtime(&t);
  char h[10];
  sprintf(h,"%02d-%02d-%02d",timeStruct->tm_hour,timeStruct->tm_min,timeStruct->tm_sec);
  string fnb = filename + ".backup-" + tmToString(timeStruct) + "-" + h ;

  writeFile(fnb);  
}


//-----------------------------------------------------------------------
// Edit
//-----------------------------------------------------------------------

bool extractDate(string l,unsigned i,string &date);

string removeText(string s,string r)
{
  unsigned i,j;
  string n;
  bool removed=false;
  
  i=0;
  while (i<s.length())
  {
    if (s[i]==r[0] && !removed)
    {
      j=0;
      while (j<r.length() && i+j<s.length() && s[i+j]==r[j])
        j++;
      
      if (j==r.length())
      {
        if ((i+j<s.length() && s[i+j]==' ') || i+j==s.length()) 
        {
          removed=true;
          i += j-1;     
        }
        else
          n += s[i];
      }
      else
        n += s[i];
    }
    else
      n += s[i];
    i++;
  }
  return n;
}

void updateDue(string &text,string ddate)
{
  size_t pos=text.find("due:");
  if (pos != string::npos)
  {
    pos += 4;
    for (unsigned j=0;j<ddate.length();j++)
      text[pos+j]=ddate[j];
  }
  else
    text += " due:" + ddate;
}

void edit(string o)
{
  unsigned i=2;
  string nt,eord;
  bool ok=false;
  
  if (!cfnm("editing")) return;

  while (i<o.length() && o[i]==' ') i++;
  while (i<o.length() && isdigit(o[i]))
  {
    nt += o[i];
    i++;
  }
  
  unsigned numt = stoi(nt);
  if (numt>0 && numt<=nTasks.size())
  {
    Task *pt=ordTasks[nTasks[numt-1]];
  
    while (i<o.length() && o[i]==' ') i++;
    
    nextWord(o,i,eord);
    if (eord.substr(0,4)=="due:") 
    {
      // change due_date
      string ddate = eord.substr(4);
      bool dtok=false;
      if (ddate=="today")
      {
        ddate=today;
        dtok=true;
      }
      else if (ddate=="tomorrow")
      {
        ddate=tomorrow;
        dtok=true;
      }
      else
        dtok=extractDate(eord,4,ddate);
      if (dtok && ddate != pt->date_due)
      {
        updateDue(pt->text,ddate);
        pt->date_due=ddate;
        ok=true;
      }
    }
    else if (eord.substr(0,4)=="rec:") 
    {
      // change recurrence
      string nrec=eord.substr(4);
      string ntext;
      if (nrec != pt->recurrence)
      {
        size_t posrec=pt->text.find("rec:");
        if (posrec != string::npos)
        {
          for (unsigned j=0;j<pt->text.length() && j<posrec;j++)
            ntext += pt->text[j];
          ntext += eord;
          while (posrec<pt->text.length() && pt->text[posrec]!=' ')
            posrec++;
          for (unsigned j=posrec;j<pt->text.length();j++)
            ntext += pt->text[j];
        
          pt->text=ntext;
          pt->recurrence=nrec;
          ok=true;
        }
        else
        {
          pt->text+=" "+eord;
          pt->recurrence=nrec;
          ok=true;
        }
      }
    }
    else if (eord == "-()")
    {
      if (pt->priority != "")
      {
        pt->priority="";
        ok=true;
      }
    }
    else if (eord.length()==3 && eord[0]=='(' && isupper(eord[1]) && eord[2]==')')
    {
      pt->priority=eord;
      ok=true;
    }
    else if (eord.length()>1 && eord[0]=='@')
    {
      // add context (if absent)
      if (!containsString(pt->contexts,eord))
      {
        pt->contexts.push_back(eord);
        pt->text+=" "+eord;
        ok=true;
      }
    }
    else if (eord.length()>2 && eord[0]=='-' && eord[1]=='@')
    {
      // remove context
      string ctx=eord.substr(1);
      if (containsString(pt->contexts,ctx))
      {
        for (unsigned j=0;j<pt->contexts.size();j++)
          if (pt->contexts[j] == ctx)
          {
            pt->contexts.erase(pt->contexts.begin()+i);
            break;
          }
        pt->text=removeText(pt->text,ctx);
        ok=true;
      }
    }
    else if (eord.length()>1 && eord[0]=='+')
    {
      // add project (if absent)
      if (!containsString(pt->projects,eord))
      {
        pt->projects.push_back(eord);
        pt->text+=" "+eord;
        ok=true;
      }
    }
    else if (eord.length()>2 && eord[0]=='-' && eord[1]=='+')
    {
      // remove project
      string prj=eord.substr(1);
      if (containsString(pt->projects,prj))
      {
        for (unsigned j=0;j<pt->projects.size();j++)
          if (pt->projects[j] == prj)
          {
            pt->projects.erase(pt->projects.begin()+i);
            break;
          }
        pt->text=removeText(pt->text,prj);
        ok=true;
      }
    }
    
    if (ok)
    {
      sortTasks(ordTasks,cst);
      writeFile(filename);
    }
    else
      cerr << "Wrong edit order <" << o << ">" << endl;
  }
  else
    cerr << "Wrong task number" << endl;
}

//-----------------------------------------------------------------------
// Complete/uncomplete
//-----------------------------------------------------------------------

string computeNewDate(string date,string rec)
{
  int y,m,d;
  unsigned nmb=0;
  bool isleapyear(int year);
  int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

  
  stringToDate(date,y,m,d);
      
  unsigned i=0;
  while (i<rec.length() && isdigit(rec[i]))
  {
    nmb = 10*nmb+rec[i]-'0';
    i++;
  }
  
  switch (rec[i]) {
    case 'd': addDays(y,m,d,nmb); 
      break; 
    case 'w': addDays(y,m,d,nmb*7);
      break;
    case 'm': 
             while (nmb) 
             {
              m++;
              if (m>12) 
              {
                m=1;
                y++;
              }
              if (d > mdays[m-1])  // adjust to last day in that month
                d = mdays[m-1];
              nmb--;
            }
      break;
    case 'y': 
              y+=nmb;
              if (m == 2 && d == 29 && !isleapyear(y))  // adjust february
                d = 28;
      break;
  }
  return dateToString(y,m,d);
}

void complete(string o)
{
  unsigned i=2;
  string nt;
  

  while (i<o.length() && o[i]==' ') i++;
  while (i<o.length() && isdigit(o[i]))
  {
    nt += o[i];
    i++;
  }
  
  unsigned numt = stoi(nt);
  if (numt>0 && numt<=nTasks.size())
  {
    Task *pt=ordTasks[nTasks[numt-1]];
  
    if (pt->completed)
      cerr << "Error, task is completed" << endl;
    else 
    { 

      if (!cfnm("completing")) return;
      
      pt->completed=true;
      pt->date_completed=today;
      
      if (pt->recurrence != "")
      {
         string ndd;
         
         if (pt->recurrence[0]=='+' && pt->date_due!="")
           ndd = computeNewDate(pt->date_due,pt->recurrence.substr(1));
         else
           ndd = computeNewDate(today,pt->recurrence);

         string text=pt->text;
         if (pt->priority != "") text=pt->priority+" "+text;

         updateDue(text,ndd);
         
         createTaskFromLine(text);         
      }
      
      sortTasks(ordTasks,cst);
      writeFile(filename);
    }
  }
  else
    cerr << "Wrong task number" << endl;
}

// ----------------------------------------------
void uncomplete(string o)
{
  unsigned i=2;
  string nt;
  

  while (i<o.length() && o[i]==' ') i++;
  while (i<o.length() && isdigit(o[i]))
  {
    nt += o[i];
    i++;
  }
  
  unsigned numt = stoi(nt);
  if (numt>0 && numt<=nTasks.size())
  {
    Task *pt=ordTasks[nTasks[numt-1]];
  
    if (!pt->completed)
      cerr << "Error, task is incomplete" << endl;
    else 
    { 
      if (pt->recurrence != "")
      {
        cerr << "Warning: the task has recurrence, uncompleting it may yield duplicate tasks. Confirm? (Y/N):";
        string cnf;
        getline(cin,cnf);
        if (cnf != "Y" && cnf != "y") return;
      }

      if (!cfnm("uncompleting")) return;
      
      pt->completed=false;
      pt->date_completed="";
      
      sortTasks(ordTasks,cst);
      writeFile(filename);
    }
  }
  else
    cerr << "Wrong task number" << endl;
}


//-----------------------------------------------------------------------
// Archive
//-----------------------------------------------------------------------
void archiveTasks(bool onlyrec)
{
  string fn=filename+".done";
  
  ofstream f;
  
  if (!cfnm("archive")) return;
  
  f.open(fn.c_str(),ios::app);
  if (f.is_open())
  {
    for (unsigned i=0;i<allTasks.size();i++)
    {
      if (allTasks[i].completed)
      {
        if (!onlyrec || allTasks[i].recurrence != "")
        {
          showTaskORIG(f,allTasks[i]);
          
          allTasks.erase(allTasks.begin()+i);
          i--;
        }
      }
    }
    
    f.close();
  }
  
  sortTasks(ordTasks,cst);
  writeFile(filename);
}

//-----------------------------------------------------------------------
// Add task
//-----------------------------------------------------------------------
void addTask(string o)
{
  if (!cfnm("adding")) return;

  string line = o.substr(2);
  
  size_t pos=line.find("due:today");
  if (pos != string::npos)
  {
    string ddate="due:"+today;
    
    line.replace(pos,9,ddate);
  }
  createTaskFromLine(line);
  sortTasks(ordTasks,cst);
  writeFile(filename);
}

//-----------------------------------------------------------------------
// Arguments
//-----------------------------------------------------------------------

bool processArguments(int argc,char *argv[],bool &interactive)
{
  bool ok=false,fn=false;
  LST_TYPE lt=COMPACT;
  string arg;
  SortType st=NONE;
  bool filter=false,add=false,sort=false,output=false;

  if (argc == 2)
  {
    filename=argv[1];
    ok=true;
    fn=true;
    interactive=true;
  }
  else if (argc > 2)
  {
    
    ok=true;
    for (int i=1;i<argc && ok;i++)
    {
      if (!strcmp(argv[i],"-a") && i+1<argc && !add && !filter)
      {
        arg = argv[i+1];
        i++;
        add=true;
      }
      else if (!strcmp(argv[i],"-f") && i+1<argc && !add && !filter)
      {
        arg = argv[i+1];
        i++;
        filter=true;
      }
      else if (!strcmp(argv[i],"-s") && i+1<argc && !sort)
      {
        if (!strcmp(argv[i+1],"d")) st=DATE;
        else if (!strcmp(argv[i+1],"a")) st=ALPHA;
        else if (!strcmp(argv[i+1],"n")) st=NONE;
        else ok=false;
        i++;
        sort=true;
      }
      else if (!strcmp(argv[i],"-o") && i+1<argc && !output)
      {
        if (!strcmp(argv[i+1],"o")) lt=ORIG;
        else if (!strcmp(argv[i+1],"c")) lt=COMPACT;
        else ok=false;
        i++;
        output=true;
      }
      else if (!fn)
      {
        filename=argv[i];
        fn=true;
      }
      else
        ok=false;     
    }
  
    if (!fn || (!add && !filter && !sort)) ok=false;
  
    if (!ok) 
      usage(argv[0]);
    else
    {
      if (readTodoFile(filename))
      {
        if (filter)
        {
          if (output) lstType = lt;

          sortTasks(ordTasks,st);
        
          processFilter(arg,fdpcp);
          showTasksFilter(ordTasks,filterTaskDPCP);
        }
        else if (add)
        {
          backup(filename);
          createTaskFromLine(arg);

          sortTasks(ordTasks,st);

          writeFile(ordTasks,filename);
        }
        else if (sort)
        {
          backup(filename);

          sortTasks(ordTasks,st);
        
          writeFile(ordTasks,filename);
        }
      }
    }
  }
  
  return ok;
}



// ----------------------------------------------

void interactive()
{
  string order;

  backup(filename);
  getFileInfo(infoFile);
  sortTasks(ordTasks,cst);

  do {
    string filter,line;

    cout << "version: " << version << "   ";
    cout << "(" << allTasks.size() << ")  [" << filename << " " << getFileMTime() << "]" << endl;
    cout << "# "; showFilter(fdpcp);
    showTasksFilterNumbered(ordTasks,filterTaskDPCP);
    cout << "> ";
    getline(cin,order);
        
    if (order == "q" || order == "h" || (order.length()>2 && order[1] == ' '))
        
      switch (order[0]) {
        
        case 'h':  help(order);
          break;
        case 'u':  uncomplete(order);
          break;
        case 'c':  complete(order);
          break;
        case 'e':  edit(order);
          break;
        case 'a':  addTask(order);
          break;
        case 'f':  processFilter(order.substr(2),fdpcp); 
          break;
        case 'A':
              switch (order[2]) {
                case 'r':
                  archiveTasks(true);
                  break;
                case 'a':
                  archiveTasks(false);
                  break;
              }
          break;
        case 's':
              switch (order[2]) {
                case 'd':
                  if (cst != DATE)
                  {
                    cst = DATE;
                    sortTasks(ordTasks,cst);
                  }
                  break;
                case 'a':
                  if (cst != ALPHA)
                  {
                    cst = ALPHA;
                    sortTasks(ordTasks,cst);
                  }
                  break;
                case 'n':
                  if (cst != NONE)
                  {
                    cst = NONE;
                    sortTasks(ordTasks,cst);
                  }
                  break;
                default:
                  errOrder(order);
              }
          break;
        case 'q':
          break;
        default:
          errOrder(order);
      }
    else
      errOrder(order);
        
  } while (order != "q");
}


// ----------------------------------------------

/* TODO

     
 - config file
     defaultFilter week
     defaultOrder d         # due date
     backup yes             # make a backup at start interactive
     backupFile "%s.backup-%date-%hour"
     archiveFile "%s.done"

*/


int main(int argc,char *argv[])
{
  bool inter=false;

  // initialize global variables
  today=dateToday();
  tomorrow=dateTomorrow();

  // .. with default values
  fdpcp.tm = WEEK;
  computeDate(fdpcp);
  createBackup=true;
  cst = DATE;
  lstType = COMPACT;

  
  if (processArguments(argc,argv,inter))
  { 
    if (inter && readTodoFile(filename))
    {
      interactive();
    }
  }
  return 0;
}
