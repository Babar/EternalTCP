#include "ProcessHelper.hpp"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

using namespace et;

#if __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#include <sys/event.h>
#else
#endif

#if __APPLE__
void noteProcDeath(
  CFFileDescriptorRef fdref,
  CFOptionFlags callBackTypes,
  void* info)
{
  // LOG_DEBUG(@"noteProcDeath... ");

  struct kevent kev;
  int fd = CFFileDescriptorGetNativeDescriptor(fdref);
  kevent(fd, NULL, 0, &kev, 1, NULL);
  // take action on death of process here
  unsigned int dead_pid = (unsigned int)kev.ident;

  CFFileDescriptorInvalidate(fdref);
  CFRelease(fdref); // the CFFileDescriptorRef is no longer of any use in this example

  int our_pid = getpid();
  // when our parent dies we die as well..
  LOG(INFO) << "exit! parent process (pid " << dead_pid << ") died. no need for us (pid " << our_pid << ") to stick around";
  exit(EXIT_SUCCESS);
}
#else
thread *parentDeathThread;
void monitorParentDeath() {
  int parent_pid = getppid();
  while(true) {
    if (parent_pid != getppid()) {
      // parent pid has changed, parent must have died.
      LOG(INFO) << "exit! parent process (pid " << parent_pid << ") died. no need for us (pid " << getppid() << ") to stick around";
      break;
    }
    sleep(1);
  }
  exit(EXIT_SUCCESS);
}
#endif

void ProcessHelper::initChildProcess() {
#if __APPLE__
  int parent_pid = getppid();
  int our_pid = getpid();
  LOG(ERROR) << "suicide_if_we_become_a_zombie(). parent process (pid " << parent_pid << ") that we monitor. our pid " << our_pid;

  int fd = kqueue();
  struct kevent kev;
  EV_SET(&kev, parent_pid, EVFILT_PROC, EV_ADD|EV_ENABLE, NOTE_EXIT, 0, NULL);
  kevent(fd, &kev, 1, NULL, 0, NULL);
  CFFileDescriptorRef fdref = CFFileDescriptorCreate(kCFAllocatorDefault, fd, true, noteProcDeath, NULL);
  CFFileDescriptorEnableCallBacks(fdref, kCFFileDescriptorReadCallBack);
  CFRunLoopSourceRef source = CFFileDescriptorCreateRunLoopSource(kCFAllocatorDefault, fdref, 0);
  CFRunLoopAddSource(CFRunLoopGetMain(), source, kCFRunLoopDefaultMode);
  CFRelease(source);
#else
  parentDeathThread = new thread(monitorParentDeath);
#endif
}

void ProcessHelper::daemonize() {
  pid_t pid;

  /* Fork off the parent process */
  pid = fork();

  /* An error occurred */
  if (pid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (pid > 0)
    exit(EXIT_SUCCESS);

  /* On success: The child process becomes session leader */
  if (setsid() < 0)
    exit(EXIT_FAILURE);

  /* Catch, ignore and handle signals */
  //TODO: Implement a working signal handler */
  signal(SIGCHLD, SIG_IGN);
  signal(SIGHUP, SIG_IGN);

  /* Fork off for the second time*/
  pid = fork();

  /* An error occurred */
  if (pid < 0)
    exit(EXIT_FAILURE);

  /* Success: Let the parent terminate */
  if (pid > 0)
    exit(EXIT_SUCCESS);

  /* Set new file permissions */
  umask(0);

  /* Close all open file descriptors */
  fclose(stderr);
  fclose(stdin);
  fclose(stdout);

  /* Open the log file */
  openlog ("ETServer", LOG_PID, LOG_DAEMON);
}
