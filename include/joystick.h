#ifndef JOYSTICK_H
#define JOYSTICK_H

#define NON_DEBUG

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <sys/ioctl.h>

#include "debug.h"

class Joystick
{
public:
  Joystick(std::string device);
  ~Joystick();

  bool isConnected() const;
  void startEventThread();
  void stopEventThread();

  std::vector<int> axis;
  std::vector<int> button;

private:
  void readEvent();
  void initialize();

  int joystick_fd;
  std::string device_path;

  std::atomic<bool> running;
  std::thread event_thread;
};

#endif
