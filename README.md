# _SHELL_

_Sai Teja Reddy Moolamalla_

_Siddhartha Laghuvarapu_

## Make instructions:

* From the directory where the Makefile is present, run:
	`make`
* Running the shell:
	`./a.out`
* To delete all the object files:
	`make clean`

## File breakdown:
* main.c: Contains functions for inputting and displaying the output. Also contains implementations for some commands like cd, pwd. Manges things like semi-colon splitting, background processes and functions for other system commands. 
* Makefile
* echo_implement.c: Consists of functions corresponding to the implementation of echo.
* echo_implement.h: Contains headers for echo_implement.c file.
* ls_implement.c: Consists of functions corresponding to the implementation of ls.
* ls_implement.h: Contains headers for ls_implement.c file.
* nightswatch_implement.c: Consists of functions corresponding to the implementation of nightswatch.
* nightswatch_implement.h: Contains headers for nightwatch.c file.
* pinfo_implement.c: Consists of functions corresponding to the implementation of pinfo.
* pinfo_implement.h: Contains headers for pinfo_implement.c file.
* jobs_ll.c: Consists of functions corresponding to the implementation of jobs builtin commands.
* jobs_ll.h:  Contains headers for jobs_ll.c file.
* Readme.txt : This file.

## Scope of Work

All the required functionality has been implemented, with extras included. 
Appropriate error codes have been implemented wherever deemed necessary.

## Functionality and Implementation details:

### Display:

* Once the code is executed, the shell prompt would be displayed in the format: 
 `<username@system_name:curr_dir>`
* Some color code has been used to display the shell prompt and some messages. 
* The functionality and the output/errors displayed for all the commands have been made to resemble bash.
* '~' corresponds to the present directory(i.e the directory from which the script is executed.)

### Processing the input:

* Getting line by line using, getline()
* Separating the line by semicolon(), to get each separate command.
* Processing each command using strtok(), with some DELIMITERS specified, and identifying the command, background or not etc. Also checking if two commands are separated by '&'. At this step, the command and the arguments, options are separated. 
* For each command identified, 
* Firstly, check if it is a shell builtin or not.
* If not, execute the command using execvc, otherwise send it to the appropriate function.

### cd:

* When the function corresponding to cd is invoked, the directory is changed using chdir.
* Cases like '~' etc. have been replaced with appropriate strings. 
* Appropriate error message will be printed if the operation fails using perror.

### ls :

* Various flags are identified, [al] in all possible combinations.
* In case of no flags, all files in the directory are printed(excluding the hidden ones), allincase of -a.
* In case of -l, file type,file permissions,file modified time,file size are fetched using stat. 
* When a directory is specified, files in that directory are specified.
* In case of symlinks, link is presented in ls -l (like in bash). 
* The printing format used is to emulate 'ls -l' in bash.
* Appropriate error messages will be printed for "file not found", wrong flags etc.

### pwd:

* Current working directory will be printed.(fetched using getcwd()).

### echo:

* The input is passed through a 'state-machine' type structure, so as to efficiently handle all the possible input types.
* All the cases when characters such as "", '', spacing, multiple occurrences of quotes etc. have been handled properly including cases when newline is printed after opening quotes. 
* Memory is dynamically allocated depending upon the size of the command.
* The format input and output would emulate bash. 

### pinfo:

* Necessary information is fetched from '/proc/pid/status' and executable path from 'proc/pid/exe'
* When no pid is specified, pid of the current process is brought from getpid().
* Appropriate error message printed when the pid could not be identified or false string has been specified.

### nightswatch:

* Options(-n) and commands are identified firstly, using getopt, and the corresponding value, if options are present.(line -n 2). 
* If time is specified, it is used. Default value is 2 s.
* The command output is printed and updated on a curses screen. (Emulating the watch command). The screen closes when 'q' is pressed.
* In  case of "Interrupt", appropriate information is fetched and updated from /proc/interrupts file.
* In case of "Dirty", the information is brought from /proc/meminfo file.
* Proper error messages have been printed in case the input is not in accord with the format. 

### Foreground/Background:

* Identified with '&' as described above. 
* fork() command is used and the process is run in background.
* Also, once the process has exited(using waitpid()), an exit message is printed after the execution of the next command or when a new line is printed(like in bash).
* SIGTTIN signal is used to ensure the bg processes do not take input from the terminal.

### Piping:

* ‘pipe’ command is used to create a read end and a write end.
* ‘dup2’ command is used to redirect stidin and stdout wherever necessary.
* Commands are seperated using ‘|’ symbol and the commands are written at one end of the pipe, read from the other.

### Redirection:

* File pointers are replaced with STDIN/STDOUT using dup2.
* When “<” is given, the input is read from the given filename.
* For “>” & “>>”  the files are opened appropriately to append/overwrite and 
to create one if it doesnt exist.
* Errors Handled: Wrong/no input files, no output files.  

### Redirection+Piping:

* No special implementation is required. It works fine with the above two implementations. 
* We had to combine the above two i.e, Redirection and Piping.

## Other Builtin commands:

### setenv: 

* setenv is used to set a new variable or redifie an existing one.
* If no value for variable given, sets to empty.
* Errors handled: Too few or high arguments, setenv returns an error.

### unsetenv: 

* unsetenv is used to unset an existing environment variable.
* Errors handled: Too few or high arguments, unsetenv fails.

### jobs:

* All the jobs present in the linked list of currently in background jobs maintained will be printed along with their running/stopped status. 

### kjob:

* Corresponding block in the linked list is identified to retreive the pid from the getjob function implemented..
* The signal is sent to the process with kill.
* Errors handled: Wrong pid, extra arguments, or no job id specified.

### fg:

* SIGCONT signal will be issued so that the process will be run if it is stopped and is sound to input and output.
* The shell control is directed to the process using waitpid.
* The process is removed from the list of backgound processes.
* Errors handled: Wrong pid, extra arguments, or no job id specified.

### bg:

* The process is continued with SIGCONT signal. 
* SIGTTIN signal is also been used to ensure no input is taken from terminal(as it is a background prcoess).
* The state of the process is updated to Running from Stopped. 
* Errors handled: Wrong pid, extra arguments, or no job id specified.

### overkill: 

* The function implemented for kjob is used.
* All the background processes will take the above function with SIGINT signal.

### quit: 

* The shell loop will be stopped. 
* When “quit” command is identified, a function “run_exit” is called which returns a zero value requesting shell loop to stop.

### CTRL+Z:

* SIGTSTP signal handler being used. 
* catchCTRL_Z  is called when the CTRL+Z is pressed.
* The current fg process is stopped and is added to the list of background processes.

### CTRL+C:

* SIGINT signal handler being used. 
* catchCTRL_C  is called when the CTRL+C is pressed. 
* The action taken is that the current fg process is killed. 