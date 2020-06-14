
#ifndef _dates_
#define _dates_

#include <string>

using namespace std;

//----------------------------------------------
// dates
//----------------------------------------------
string tmToString(struct tm * tms);
string dateToString(int y,int m,int d);
void stringToDate(string date,int &y,int &m,int &d);
void addDays(int &y,int &m,int &d,unsigned offset);
string dateToday();
void getToday(int &y,int &m,int &d);
string dateTomorrow();

#endif
