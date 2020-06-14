
# ptodo - parse/edit todo.txt  v1.0

A program to manage a todo.txt file from command line, or 
interactively 

(more info on todo.txt: 
  http://todotxt.org/
  https://github.com/todotxt/todo.txt
)


(C) 2020 fmorenoseco@gmail.com
(except isleapyear from: https://www.codespeedy.com/how-to-add-days-to-date-in-cpp/) 

License: GPLv3
Warranty: there is no warranty (make backups!), it has not been exhaustively
tested, although there are no known bugs


ToDo
----

 - add config file for default values, archive file format, backup file
   format, ...


Command-line mode
-----------------

ptodo [OPTIONS] <todo.txt file>

   -a 'new task'   - backup + add task + write
                     (not compatible with -f)
                   example: -a 'Buy a new keyboard @Home due:2020-06-01'
                   (you need the ' ' to pass the whole task as only one argument)

   -s [dan]        - sort by (d)ue date, (a)lphabetically or (n)one [default]
                     if used...
                         .. with -a : backup + sort + add task + write
                         .. with -f : sort + filter (no backup, no write)
                         .. alone :   backup + sort + write

   -f 'filter'     - lists tasks after filtering
                     [actived by default: week]
                     (not compatible with -a)
                   example: -f 'month @Work +dev'

   -o [oc]         - output format (o)riginal or (c)ompact [default]
                     only with -f, otherwise it's ignored and defaults to
                     original
   (at least one of -a, -f, -s)


 Options -f and -a are mutually exclusive, while -s can appear with both and
also alone. Both -a and -s (alone) write in original format. In -f option
tasks appear in compact format (starts with due date) by default, but you
can change the format with -o option.


Interactive mode
----------------

In this mode, a task listing shows and then it reads and executes an order.
If an order modifies the task list, it first checks file modification time
(and, if modified, rereads it and aborts order), then executes the order, 
and finally writes the file.

Orders are:

h                    - help
h f                  - help on filters
f <filter>           - adds a filter
s [adn]              - sort (a)lphabetically, by (d)ue date [default] or (n)one
A [ra]               - archive completed (r)ecurrent/(a)ll
e nn due:yyyy-mm-dd  - edit task nn from listing and changes/sets due date
e nn due:today       - edit task nn from listing and changes/sets due date to today
e nn due:tomorrow    - edit task nn from listing and changes/sets due date to tomorrow
e nn rec:...         - edit recurrence
e nn @Context        - adds @Context to task nn (if it wasn't yet)
e nn -@Context       - removes @Context (if present)
e nn +Project        - adds +Project
e nn -+Project       - removes +Project
e nn -()             - removes priority (if present)
e nn (P)             - sets priority
c nn                 - complete task nn 
u nn                 - uncomplete task nn (warns if recurrent)
a 'new task ...'     - adds a task 
q                    - exit
 


Recurrence
----------

 Recurrence types managed are 'd','w','m' and 'y', and also recurrence from due date
(rec:+...) and from today (rec:...)



Filter
------

Filters are cumulative, you can add more filters to previous filter (shown
in the listing). Filter options are:

    t/today             
    w/week [default]  
    m/month (30 days)  
    d/days nn  
    a/all (incompleted with or without due date)  
    A/ALL (all)  
    (only one of these is active)


    
    (A)     (show tasks with (A), removes -(P) if present)  
    -(A)    (show tasks without (A), removes (P) if present)  
    #()     (removes (P) filter)  
    #-()    (removes -(P) filter)  


    @Context   (show task with @Context)
    -@Context  (show task without @Context)
    #@Context  (removes @Context from filter)  
    #-@Context (removes -@Context from filter)  
    #@         (removes all @Context filters)  
    #-@        (removes all -@Context filters)  


    +Project   (same as with @Context)
    -+Project  
    #+Project   
    #-+Project  
    #+          
    #-+         


    'text'  (task.text contains text)
    #''     (removes text filter)


Examples

 w @Work -+dev    : tasks due this week from @Work context, without +dev
 m 'rewrite'      : tasks due in 30 days with 'rewrite' in its text



Sort
----

 These are the sorting options:
  (n)one : preserve original file order
  (a)lphabetically : order alphabeticallly 
  (d)ue date : order by due date 

 The sorting only affects to the listing, the todo file will preserve it's
original order (you may change file order with command-line mode option '-s')


Archive
-------

 When an archive order is executed, the program will open for appending the
file

  <todo-file>.done

and it will add completed tasks (with 'a' option) or only completed
recurrent tasks (with 'r'). These tasks will be removed from the task list
(and the file will be updated).


Backup
------

 In command-line mode, if it's not a filter operation, it will make a backup
of the todo.txt file. Also, when entering interactive mode a backup will be made
(yes, even if you don't modify anything). The backup file will be:

  <todo-file>.backup-YYYY-MM-DD-hh-mm-ss

You can deactivate this option by changing

  createBackup=true;

for

  createBackup=false;

in the 'main' function. This is one of the things to be soon included in a
config file (see ToDo)




