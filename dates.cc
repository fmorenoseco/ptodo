
#include <ctime>
#include <sstream>

using namespace std;

//----------------------------------------------
// dates
//----------------------------------------------
string tmToString(struct tm * tms)
{
    stringstream ss;

    ss << (1900+tms->tm_year) 
       << '-' << (tms->tm_mon+1<10 ? "0" : "") << (tms->tm_mon+1)
       << '-' << (tms->tm_mday<10 ? "0" : "") << tms->tm_mday ;

    return ss.str();
}

string dateToString(int y,int m,int d)
{
    stringstream ss;

    ss << y << '-' << (m<10 ? "0":"") << m 
            << '-' << (d<10 ? "0":"") << d ;

    return ss.str();
}

// asumes dateformat is "YYYY-MM-DD";

void stringToDate(string date,int &y,int &m,int &d)
{
  unsigned i=0;
  
  y=0;
  while (i<4 && i<date.length())
  {
    y = y*10+date[i]-'0';
    i++;
  }
  i++; // -
  
  m=0;
  while (i<7 && i<date.length())
  {
    m = m*10+date[i]-'0';
    i++;
  }
  i++; // -

  d=0;
  while (i<date.length())
  {
    d = d*10+date[i]-'0';
    i++;
  }
}


// from: https://www.codespeedy.com/how-to-add-days-to-date-in-cpp/
bool isleapyear(int year)  
{  
    if (year % 400 == 0)  
        return true;  
  
    if (year % 100 == 0)  
        return false;  
  
    if (year % 4 == 0)  
        return true;  
    return false;  
}

void addDays(int &y,int &m,int &d,unsigned offset)
{
  int mdays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  
  while (offset)
  {
    d++;
    offset--;
    
    int md = mdays[m-1];
    if (isleapyear(y) && m==2) md++;
    
    if (d>md)
    {
      d=1;
      m++;
      if (m>12)
      {
        m=1;
        y++;
      }
    }
  }  
}

string dateToday()
{
    time_t t = time(0); 
    struct tm * timeStruct = localtime(&t);

    return tmToString(timeStruct);
}



void getToday(int &y,int &m,int &d)
{
    time_t t = time(0); 
    struct tm * tms = localtime(&t);
    
    y = 1900+tms->tm_year;
    m = 1+tms->tm_mon;
    d = tms->tm_mday;
}

string dateTomorrow()
{
    int y,m,d;
        
    getToday(y,m,d);
    addDays(y,m,d,1);
    return dateToString(y,m,d);
}
