This repository has code for the programs that I have wrote in  C language for Systems Programming class.

### [echo-server](https://github.com/hikarimn/systems-programming/tree/master/echo-server)
The project is set up to read every line of text that you type in the client terminal, send it to the server, and have the server echo that text back to you. Start the server project first, and then the client. 

### [folder-watcher](https://github.com/hikarimn/systems-programming/tree/master/folder-watcher)
This program implements a folder watching system that watches a directory on a computer for files that have been updated and keeps a log of which files have been updated at what times.

The two programs consists of a folder watcher control program, `fw`, and the folder watcher daemon, `fwd.`

`fw` will take commands from the command line to tell it to start or stop the `fwd` program. To start `fwd` you will invoke `fw` from the command line using a command

<pre>./fw start <path>
</pre>

where `<path>` is the path to a directory that you want `fwd` to watch for changes. The `fw` program will start the `fwd` program as a background process, save the process id of that background process to a text file, and then exit.

To stop the `fwd` program you will run `fw` from the command line with the command

<pre>./fw stop
</pre>

When `fw` runs with this option, it will open the text file to read the process id of the `fwd` process, send the `SIGTERM` signal to that process, and then exit.

The `fwd` program will start by reading the path to the folder it needs to watch from the command line. The `fwd` program will open a log file for writing and drop into an infinite loop. On each iteration through the loop `fwd` will note the current time, sleep for a period of five minutes, and then scan the directory it is watching for files that have changed. If it finds any files in the directory with modification times after the time it went to sleep, it will write the names of those files along with their modification times to the log file. `fwd` should only watch the files located in the target directory. It does not have to watch any files located in subdirectories of the target directory.

When the `fwd` program receives the `SIGTERM` signal from `fw`, it will close the log file and exit.

### [folders](https://github.com/hikarimn/systems-programming/tree/master/folders)
A C program that reads the names from the names.txt file and makes a folder for each of the names. I assume that I have already made a directory named Folders in my project directory. 

### [gsl](https://github.com/hikarimn/systems-programming/tree/master/gsl) 
The program sets up a simple system of linear equations and uses functions from the GSL library to solve the system.

### [jello](https://github.com/hikarimn/systems-programming/tree/master/jello)
Given an input file hello.txt that consists of the string “Hello, world!\n”, this program that uses mmap to change the contents of hello.txt to “Jello, world!\n”.

### [mkdir](https://github.com/hikarimn/systems-programming/tree/master/mkdir)
Updated [folders program](https://github.com/hikarimn/systems-programming/tree/master/folders) with two changes. The first change is that I use mkdir() in place of the clunky execve() method. The second change is that in this version this program creates a directory only if the corresponding directory does not already exist.

### [mm](https://github.com/hikarimn/systems-programming/tree/master/mm)
Programming exercise 9.18 from "Computer Systems: 
A Programmer’s Perspective"

### [tfgets](https://github.com/hikarimn/systems-programming/tree/master/tfgets)
A program uses the tfgets function to prompt the user to type their name to stdin. If the user types their name, print a message that greets them by name. If the user does not type their name in five seconds, my program prints the message "Time's up!" instead.
