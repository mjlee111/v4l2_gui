#ifndef USB_CAMERA_H
#define USB_CAMERA_H

#define DEBUG

#include <iostream>
#include <vector>
#include <string>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <regex>
#include <dirent.h>
#include <thread>
#include <sys/mman.h>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "debug.h"

struct deviceData
{
  std::string path;
  std::string device_name;
};

struct ResolutionInfo
{
  std::pair<int, int> resolution;
  std::vector<float> fps;
};

struct m_deviceInfo
{
  std::string device_name;
  std::string driver;
  std::string bus_info;

  std::vector<std::string> formats;
  std::vector<ResolutionInfo> resolution_info;
};

struct m_deviceConfig
{
  std::string path;
  std::string device_name;
  std::string format;
  std::pair<int, int> resolution;
  float fps;
};

class usb_cam
{
public:
  usb_cam();
  ~usb_cam();

  std::vector<deviceData> find_device();
  m_deviceInfo get_device_info(const std::string& devicePath);
  void start_stream(const m_deviceConfig& config);
  void stop_stream();

  int set_control(int control_id, int value);
  int get_control(int control_id);
  bool query_control(int control_id, v4l2_queryctrl& queryctl);
  void reset_controls_to_default();

  cv::Mat m_image;
  std::atomic<bool> streaming;

private:
  std::vector<void*> buffers;
  std::vector<size_t> buffer_lengths;
  std::thread stream_thread;
  int m_fd;

  int xioctl(int fd, int request, void* arg);
  std::string get_control_name(int control_id);
};

#endif
