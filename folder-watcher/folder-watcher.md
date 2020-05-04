<div class="container">

### A folder watcher system

In this exam problem you will write a pair of C programs. These programs will implement a folder watching system that watches a directory on a computer for files that have been updated and keeps a log of which files have been updated at what times.

The two programs you will write are a folder watcher control program, `fw`, and the folder watcher daemon, `fwd.`

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

### How to read options from the command line

The main function in a C program has the following structure.

<pre>int main(int argc, char **argv)
{
}
</pre>

The `argc` parameter is the number of options on the command line when the program was invoked, and `argv` is a pointer to an array of argc pointers to strings containing the various command line options.

For example, if you invoke `fw` from a terminal with the command line

<pre>./fw start /home/joe/project
</pre>

the `argc` parameter to `main()` in `fw` will be 3, and `argv` will point to an array containing the three strings `"./fw"`, `"start"`, and `"/home/joe/project"`.

### Further resources

For this exam you will also need to make use of the C string and time functions. Documentation on the functions in string.h is available at [http://www.cplusplus.com/reference/cstring/](http://www.cplusplus.com/reference/cstring/). Documentation on the functions in time.h is available at [http://www.cplusplus.com/reference/ctime/](http://www.cplusplus.com/reference/ctime/).

You can confirm that `fwd` is running by using the `ps` command in a terminal to list all running processes.

### Possible questions

Here are answers to a couple of questions you may have.

_What exactly do_ `fw` _and_ `fwd` _do?_

The sole responsibility of `fw` is starting and stopping `fwd`. `fwd` does most of the actual work of the system: it watches the folder for changes and logs those changes to a log file.

_How do I store the modification times in the log file?_

You will get information about files in the watched folder by calling `stat()`. You can store the value of the `st_mtime` field of the `stat` struct in the file. If you want to get fancy, you are also welcome to convert that time value to a human-readable form.

### How to turn in your work

Send me your fw.c and fwd.c source code files as an attachment to an email message.

</div>
