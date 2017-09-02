Make instructions:
From the directory where the Makefile is present, run:

make 

Running the shell:
./a.out 


To delete all the object files:
make clean

File breakdown:
main.c: Contains functions for inputting and displaying the output. Also contains implementations for some commands like cd, pwd. Manges things like semi-colon splitting, background processes and functions for other system commands. 
Makefile
echo_implement.c: Consists of functions corresponding to the implementation of echo.
echo_implement.h: Contains headers for echo_implement.c file.
ls_implement.c: Consists of functions corresponding to the implementation of ls.
ls_implement.h: Contains headers for ls_implement.c file.
nightswatch_implement.c: Consists of functions corresponding to the implementation of nightswatch.
nightswatch_implement.h: Contains headers for nightwatch.c file.
pinfo_implement.c: Consists of functions corresponding to the implementation of pinfo.
pinfo_implement.h: Contains headers for pinfo_implement.c file.
Readme.txt : This file.

Scope of Work : All the required functionality has been implemented, all bonuses included. Appropriate error codes have been implmented wherever deemed necessary.

Implementation and functionality details:

Display:
-Once the code is executed, the shell prompt would be displayed in the format:
<username@system_name:curr_dir>
- Some color code has been used to display the shell prompt and some messages. 
- The functionality and the output/errors displayed for all the commands have been made to resemble bash.
- '~' corresonds to the present directory(i.e the directory from which the script is executed.)

Processing the input:
-Getting line by line using, getline()
-Seperating the line by semicolon(), to get each seperate command.
-Processing each command using strtok(), with some DELIMITERS specified, and identifying the command, background or not etc. Also checking if two commands are seperated by '&'. At this step, the command and the arguments, options are seperated. 
For each command identified, 
-Firstly, check if it is a shell builtin or not.
-If not, execute the command using execvc, otherwise send it to the appropriate function.

cd:
-When the fucnction corresponding to cd is invoked, the directory is changed using chdir.
-Cases like '~' etc. have been replaced with appropriate strings. 
-Appropriate error message will be printed if the operation fails using perror.

ls :
-Various flags are identifed, [al] in all possible combinations.
-In case of no flags, all files in the directory are printed(excluding the hidden ones), all incase of -a.
-In case of -l, file type,file permissions,file modified time,file size are fetched using stat. 
-When a directory is spcified, files in that directory are specified.
-In case of symlinks, link is presented in ls -l (like in bash). 
-The printing format used is to emulate 'ls -l' in bash.
-Appropriate error messages will be printed for "file not found", wrong flags etc.


pwd:
Current working directory will be printed.(fetched using getcwd()).

echo:
-The input is passed through a 'state-machine' type structure, so as to efficiently handle all the possible input types.
-All the cases when characters such as "", '', spacing, multiple occurances of quotes etc. have been handled properly including cases when newline is printed after opening quotes. 
-Memory is dynamically allocated depending upon the size of the command.
-The format input and output would emulate bash. 

pinfo:
-Necessary information is fetched from '/proc/pid/status' and executable path from 'proc/pid/exe'
-When no pid is specified, pid of the current process is brought from getpid().
-Appropriate error message printed when the pid could not be identified or false string has been specified.


nightswatch:
-Options(-n) and commands are idntified firstly, using getopt, and the corresponding value, if options are present.(line -n 2). 
-If time is specified, it is used. Defualt value is 2 s.
-The command output is printed and updated on a curses screen. (Emulating the watch command). The screen closes when 'q' is pressed.
-In  case of "Interrupt", appropriate information is fetched and updated from /proc/interrupts file.
-In case of "Dirty", the information is brought from /proc/meminfo file.
-Proper error messages have been printed in case the input is not in accord with the format. 

background/foreground:
- Identified with '&' as described above. 
- fork() command is used and the process is run in background.	
- Also, once the process has exited(using waitpid()), an exit message is printed after the execution of the next command or when a new line is printed(like in bash).