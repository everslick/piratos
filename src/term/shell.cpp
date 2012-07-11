#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stropts.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "shell.h"

Shell::Shell(const char *ttl, const char *cmd, char * const argv[]) {
	master_pty = -1;
	child_pid = 0;
	running = false;

	title = strdup(ttl);

	Open(cmd, argv);
}

Shell::~Shell(void) {
	Close();
	CleanUp();

	free(title);
}

int
Shell::DebugOut(const char *fmt, ...) {
	int n = 0;

#if 0
	va_list ap;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);
#endif

	return (n);
}

void
Shell::CloseAll(int fd) {
	int fdlimit = 100; // sysconf(SC_OPEN_MAX);

	// close all FDs >= a specified value
	while (fd < fdlimit) close(fd++);
}

bool
Shell::Open(const char *command, char * const argv[]) {
	// let's watch if the slave is already running
	if (!running) {
		DebugOut("opening master\n");

		// open a master
		if ((master_pty = open("/dev/ptmx", O_RDWR | O_NONBLOCK)) < 0) {
			DebugOut("open master pty failed: %s\n", strerror(errno));

			return (false);
		}

		DebugOut("master opened %i\n", master_pty);

		// change permission of slave
		if (grantpt(master_pty) < 0) {
			DebugOut("could not change permission of slave: %s\n", strerror(errno));

			return (false);
		}

		// unlock slave
		if (unlockpt(master_pty) < 0) {
			DebugOut("could not unlock slave: %s", strerror(errno));

			return (false);
		}

		char *slavename = ptsname(master_pty);

		// get name of slave
		if (!slavename) {
			DebugOut("could not get a slave name\n");

			return (false);
		}

		DebugOut("name of slave device is %s\n", slavename);

		int pid = fork();

		if (pid < 0) {
			close(master_pty);
			master_pty = 0;
			DebugOut("fork failed\n");

			return (false);
		}

		if (pid == 0) {
			DebugOut("slave: reached\n");

			// we are in the child process
			CloseAll(0);

			// we need to make this process a session group leader, because
			// it is on a new PTY, and things like job control simply will
			// not work correctly unless there is a session group leader
			// and process group leader (which a session group leader
			// automatically is). this also disassociates us from our old
			// controlling tty.

			if (setsid() < 0) {
				DebugOut("could not set session leader for %s\n", slavename);
			}

			DebugOut("slave: setsid ok\n");

			// open slave
			int slave = open(slavename, O_RDWR);

			if (slave < 0) {
				DebugOut("open slave pty failed: %s\n", strerror(errno));
				exit(1);
			}

			DebugOut("slave: pts id is %i\n", slave);

			ioctl(slave, I_PUSH, "ptem");   // push ptem
			ioctl(slave, I_PUSH, "ldterm"); // push ldterm

			// tie us to our new controlling tty
			if (ioctl(slave, TIOCSCTTY, NULL)) {
				DebugOut("could not set new controlling tty for %s\n", slavename);
			}

			DebugOut("slave: ioctl ok\n");

			// make slave pty be standard in, out, and error
			dup2(slave,  STDIN_FILENO);
			dup2(slave, STDOUT_FILENO);
			dup2(slave, STDERR_FILENO);

			// at this point the slave pty should be standard input
			if (slave > 2) close(slave);

			// tell the executing program which protocol we are using
			putenv((char *)"TERM=linux"); // linux

			// start the child process
			execv(command, argv);

			// exec has failed
			_exit(1);
		}

		// save the child id
		child_pid = pid;
		running = true;

		// so the terminal emulation can respond to requests from the shell
		vt102.SetOutputFD(master_pty);

		DebugOut("master: reached\n");
		DebugOut("started: %s pid=%i pts=%i\n", title, pid, master_pty);

		// we are in the master process
	}

	return (true);
}

bool
Shell::ShellWaitPid(int pid, int timeout) {
	if (pid < 0) return (true);

	for (int i=timeout/100; i>0; i--) {
		if (waitpid(pid, NULL, WNOHANG) == pid) return (true);

		HandleOutput();

		usleep(100 * 1000);
	}

	// timeout
	return (false);
}

bool
Shell::Close(void) {
	// give the process the ability to quit
	if (running && (child_pid >= 0)) {
		kill(child_pid, SIGTERM);
		kill(child_pid, SIGHUP);

		if (!ShellWaitPid(child_pid, 500)) {
			DebugOut("killing non interuptable child: pid=%i\n", child_pid);
			kill(child_pid, SIGKILL);

			if (!ShellWaitPid(child_pid, 500)) {
				DebugOut("timeout on killing child: pid=%i\n", child_pid);

				return (false);
			} else {
				child_pid = -1;
			}
		} else {
			child_pid = -1;
		}
	}

	return (true);
}

void
Shell::CleanUp(void) {
	if (running) {
		if (child_pid >= 0) {
			// avoid zombies
			waitpid(child_pid, NULL, WNOHANG);

			DebugOut("child terminated pid=%i\n", child_pid);

			child_pid = -1;
		}

		if (master_pty >= 0) {
			if (close(master_pty) < 0) {
				DebugOut("could not close pts: pts=%i\n", master_pty);
			} else {
				master_pty = -1;
			}
		}

		running = false;
	}
}

void
Shell::SetTerminalSize(int char_w, int char_h, int pixel_w, int pixel_h) {
	vt102.SetSize(char_w, char_h);

	if (running) {
		winsize ws;

		ws.ws_col = char_w;
		ws.ws_row = char_h;
		ws.ws_xpixel = pixel_w;
		ws.ws_ypixel = pixel_h;

		// try to set window size; failure isn't critical
		if (ioctl(master_pty, TIOCSWINSZ, &ws) < 0) {
			DebugOut("could not set pts window size: pts=%i\n", master_pty);
		}
	}
}

void
Shell::Write(const unsigned char *data, int len) {
	if (running) {
		int i = write(master_pty, (char *)data, len);

		if (i < 0 && errno != EINTR) CleanUp();
	}
}

bool
Shell::HandleOutput(void) {
	if (master_pty >= 0) {
		int i = 0;

		while (1) {
			i = read(master_pty, buffer, INPUT_BUFSIZE-1);

			if (i > 0) {
				buffer[i] = '\0';    // terminate string
				vt102.Write(buffer); // print on screen
			} else if (i == 0 || (i < 0 && errno != EAGAIN && errno != EINTR)) {
				// slave has terminated, so free the pty ...
				CleanUp();

				// ... and signal the user interface (with an empty string)
				vt102.Write((unsigned char*)"");

				// brings the shells to remove me from its polling list
				return (false);
			}

			if (i < INPUT_BUFSIZE) break;

			// if the buffer is full then do a new cycle to ensure
			// that all available data were read
		}
	}

	return (true);
}
