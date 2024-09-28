#include "usb_camera.h"

usb_cam::usb_cam() : streaming(false), m_fd(-1)
{
}

usb_cam::~usb_cam()
{
}

std::vector<deviceData> usb_cam::find_device()
{
  std::vector<deviceData> devices;
  std::string videoPath = "/dev/";
  std::regex videoRegex("video[0-9]+");

  DIR* dir = opendir(videoPath.c_str());
  if (dir == nullptr)
  {
    std::cerr << "Failed to open /dev directory" << std::endl;
    return devices;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    std::string filename(entry->d_name);

    if (std::regex_match(filename, videoRegex))
    {
      std::string devicePath = videoPath + filename;

      int m_fd = open(devicePath.c_str(), O_RDWR | O_NONBLOCK);
      if (m_fd == -1)
      {
        std::cerr << "Failed to open device: " << devicePath << std::endl;
        continue;
      }

      struct v4l2_capability cap;
      if (ioctl(m_fd, VIDIOC_QUERYCAP, &cap) == -1)
      {
        std::cerr << "Failed to query device capabilities: " << devicePath << std::endl;
        close(m_fd);
        continue;
      }

      deviceData device;
      device.path = devicePath;
      device.device_name = std::string((char*)cap.card);

      devices.push_back(device);

      close(m_fd);
    }
  }

  closedir(dir);
  return devices;
}

int usb_cam::xioctl(int fd, int request, void* arg)
{
  int r;
  do
  {
    r = ioctl(fd, request, arg);
  } while (r == -1 && errno == EINTR);
  return r;
}

m_deviceInfo usb_cam::get_device_info(const std::string& devicePath)
{
  m_deviceInfo devInfo;

  int fd = open(devicePath.c_str(), O_RDWR);
  if (fd == -1)
  {
    std::cerr << "Failed to open device: " << devicePath << std::endl;
    return devInfo;
  }

  struct v4l2_capability cap;
  if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
  {
    std::cerr << "Failed to get device capabilities. Error: " << strerror(errno) << std::endl;
    close(fd);
    return devInfo;
  }

  devInfo.device_name = (char*)cap.card;
  devInfo.driver = (char*)cap.driver;
  devInfo.bus_info = (char*)cap.bus_info;

  struct v4l2_fmtdesc fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.index = 0;

  while (xioctl(fd, VIDIOC_ENUM_FMT, &fmt) != -1)
  {
    devInfo.formats.push_back((char*)fmt.description);

    struct v4l2_frmsizeenum frmsize;
    frmsize.pixel_format = fmt.pixelformat;
    frmsize.index = 0;

    while (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) != -1)
    {
      ResolutionInfo resInfo;
      if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
      {
        resInfo.resolution = std::make_pair(frmsize.discrete.width, frmsize.discrete.height);

        struct v4l2_frmivalenum frmival;
        frmival.pixel_format = fmt.pixelformat;
        frmival.width = frmsize.discrete.width;
        frmival.height = frmsize.discrete.height;
        frmival.index = 0;

        while (xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) != -1)
        {
          if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
          {
            float fps = (float)frmival.discrete.denominator / frmival.discrete.numerator;
            resInfo.fps.push_back(fps);
          }
          frmival.index++;
        }

        if (!resInfo.fps.empty())
        {
          devInfo.resolution_info.push_back(resInfo);
        }
      }
      frmsize.index++;
    }

    fmt.index++;
  }

  close(fd);
  return devInfo;
}

void usb_cam::start_stream(const m_deviceConfig& config)
{
  m_fd = open(config.path.c_str(), O_RDWR);
  if (m_fd == -1)
  {
    std::cerr << "Failed to open device: " << config.path << std::endl;
    return;
  }

  // Set video format
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = config.resolution.first;
  fmt.fmt.pix.height = config.resolution.second;
  if (config.format == "MJPEG" || config.format == "Motion-JPEG")
  {
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
  }
  else if (config.format == "YUYV")
  {
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  }
  else if (config.format == "H.264" || config.format == "H264")
  {
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_H264;
  }
  else
  {
    std::cerr << "Unsupported format: " << config.format << std::endl;
    close(m_fd);
    return;
  }

  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (xioctl(m_fd, VIDIOC_S_FMT, &fmt) == -1)
  {
    std::cerr << "Failed to set format" << std::endl;
    close(m_fd);
    return;
  }

  // Set frame rate
  struct v4l2_streamparm streamparm;
  memset(&streamparm, 0, sizeof(streamparm));
  streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  streamparm.parm.capture.timeperframe.numerator = 1;
  streamparm.parm.capture.timeperframe.denominator = static_cast<int>(config.fps);

  if (xioctl(m_fd, VIDIOC_S_PARM, &streamparm) == -1)
  {
    std::cerr << "Failed to set frame rate" << std::endl;
    close(m_fd);
    return;
  }

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (xioctl(m_fd, VIDIOC_REQBUFS, &req) == -1)
  {
    std::cerr << "Failed to request buffers" << std::endl;
    close(m_fd);
    return;
  }

  buffers.resize(req.count);
  buffer_lengths.resize(req.count);

  for (int i = 0; i < req.count; ++i)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (xioctl(m_fd, VIDIOC_QUERYBUF, &buf) == -1)
    {
      std::cerr << "Failed to query buffer" << std::endl;
      close(m_fd);
      return;
    }

    buffers[i] = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, buf.m.offset);
    buffer_lengths[i] = buf.length;

    if (buffers[i] == MAP_FAILED)
    {
      std::cerr << "Failed to map buffer" << std::endl;
      close(m_fd);
      return;
    }

    if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
    {
      std::cerr << "Failed to queue buffer" << std::endl;
      close(m_fd);
      return;
    }
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(m_fd, VIDIOC_STREAMON, &type) == -1)
  {
    std::cerr << "Failed to start streaming" << std::endl;
    close(m_fd);
    return;
  }

  streaming = true;

  stream_thread = std::thread([this, config]() {
    while (streaming)
    {
      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf));
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;

      if (xioctl(m_fd, VIDIOC_DQBUF, &buf) == -1)
      {
        std::cerr << "Failed to dequeue buffer" << std::endl;
        break;
      }

      cv::Mat img(cv::Size(config.resolution.first, config.resolution.second), CV_8UC3);
      img = cv::imdecode(cv::Mat(1, buf.bytesused, CV_8UC1, buffers[buf.index]), cv::IMREAD_COLOR);
      m_image = img.clone();

      if (xioctl(m_fd, VIDIOC_QBUF, &buf) == -1)
      {
        std::cerr << "Failed to queue buffer" << std::endl;
        break;
      }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(m_fd, VIDIOC_STREAMOFF, &type);
    close(m_fd);
  });
}

void usb_cam::stop_stream()
{
  if (!streaming)
  {
    return;
  }

  streaming = false;

  if (stream_thread.joinable())
  {
    stream_thread.join();
  }

  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(m_fd, VIDIOC_STREAMOFF, &type) == -1)
  {
    std::cerr << "Failed to stop streaming" << std::endl;
  }

  for (size_t i = 0; i < buffers.size(); ++i)
  {
    if (buffers[i] != MAP_FAILED)
    {
      munmap(buffers[i], buffer_lengths[i]);
    }
  }

  buffers.clear();
  buffer_lengths.clear();
  m_image.release();

  if (m_fd != -1)
  {
    close(m_fd);
    m_fd = -1;
  }
}

int usb_cam::set_control(int control_id, int value)
{
  struct v4l2_control control;
  control.id = control_id;
  control.value = value;

  std::string control_name = get_control_name(control_id);

  if (xioctl(m_fd, VIDIOC_S_CTRL, &control) == -1)
  {
    std::cerr << "Failed to set control (" << control_name << ", ID: " << control_id << "): " << strerror(errno)
              << std::endl;
    return -1;
  }

  return 0;
}

int usb_cam::get_control(int control_id)
{
  struct v4l2_control control;
  control.id = control_id;

  std::string control_name = get_control_name(control_id);

  if (xioctl(m_fd, VIDIOC_G_CTRL, &control) == -1)
  {
    std::cerr << "Failed to get control (" << control_name << ", ID: " << control_id << "): " << strerror(errno)
              << std::endl;
    return -1;
  }

  return control.value;
}

bool usb_cam::query_control(int control_id, v4l2_queryctrl& queryctrl)
{
  memset(&queryctrl, 0, sizeof(queryctrl));
  queryctrl.id = control_id;

  std::string control_name = get_control_name(control_id);

  if (ioctl(m_fd, VIDIOC_QUERYCTRL, &queryctrl) == -1)
  {
    if (errno != EINVAL)
    {
      std::cerr << "Failed to query control (" << control_name << ", ID: " << control_id << "): " << strerror(errno)
                << std::endl;
    }
    else
    {
      std::cerr << "Control (" << control_name << ", ID: " << control_id << ") is not supported." << std::endl;
    }
    queryctrl.flags |= V4L2_CTRL_FLAG_DISABLED;
    return false;
  }
  else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
  {
    std::cerr << "Control (" << control_name << ", ID: " << control_id << ") is disabled." << std::endl;
    return false;
  }
  return true;
}

void usb_cam::reset_controls_to_default()
{
  struct v4l2_queryctrl queryctrl;
  memset(&queryctrl, 0, sizeof(queryctrl));

  for (queryctrl.id = V4L2_CID_BASE; queryctrl.id < V4L2_CID_LASTP1; queryctrl.id++)
  {
    if (ioctl(m_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0)
    {
      if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
      {
        set_control(queryctrl.id, queryctrl.default_value);
      }
    }
  }

  for (queryctrl.id = V4L2_CID_PRIVATE_BASE; queryctrl.id < V4L2_CID_PRIVATE_BASE + 20; queryctrl.id++)
  {
    if (ioctl(m_fd, VIDIOC_QUERYCTRL, &queryctrl) == 0)
    {
      if (!(queryctrl.flags & V4L2_CTRL_FLAG_DISABLED))
      {
        set_control(queryctrl.id, queryctrl.default_value);
      }
    }
  }
}

std::string usb_cam::get_control_name(int control_id)
{
  switch (control_id)
  {
    case V4L2_CID_BRIGHTNESS:
      return "Brightness";
    case V4L2_CID_CONTRAST:
      return "Contrast";
    case V4L2_CID_SATURATION:
      return "Saturation";
    case V4L2_CID_HUE:
      return "Hue";
    case V4L2_CID_GAMMA:
      return "Gamma";
    case V4L2_CID_SHARPNESS:
      return "Sharpness";
    case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
      return "White Balance Temperature";
    case V4L2_CID_AUTO_WHITE_BALANCE:
      return "Auto White Balance";
    case V4L2_CID_EXPOSURE_ABSOLUTE:
      return "Exposure (Absolute)";
    case V4L2_CID_EXPOSURE_AUTO:
      return "Auto Exposure";
    case V4L2_CID_EXPOSURE_AUTO_PRIORITY:
      return "Auto Exposure Priority";
    case V4L2_CID_POWER_LINE_FREQUENCY:
      return "Power Line Frequency";
    case V4L2_CID_BACKLIGHT_COMPENSATION:
      return "Backlight Compensation";
    case V4L2_CID_FOCUS_ABSOLUTE:
      return "Focus (Absolute)";
    case V4L2_CID_FOCUS_AUTO:
      return "Auto Focus";
    case V4L2_CID_ZOOM_ABSOLUTE:
      return "Zoom (Absolute)";
    case V4L2_CID_PAN_ABSOLUTE:
      return "Pan (Absolute)";
    case V4L2_CID_TILT_ABSOLUTE:
      return "Tilt (Absolute)";
    case V4L2_CID_PRIVACY:
      return "Privacy";
    case V4L2_CID_ROTATE:
      return "Rotate";
    case V4L2_CID_HFLIP:
      return "Horizontal Flip";
    case V4L2_CID_VFLIP:
      return "Vertical Flip";
    case V4L2_CID_COLOR_KILLER:
      return "Color Killer";
    case V4L2_CID_COLORFX:
      return "Color Effects";
    case V4L2_CID_AUTOGAIN:
      return "Auto Gain";
    case V4L2_CID_GAIN:
      return "Gain";
    case V4L2_CID_HUE_AUTO:
      return "Auto Hue";
    case V4L2_CID_RED_BALANCE:
      return "Red Balance";
    case V4L2_CID_BLUE_BALANCE:
      return "Blue Balance";
    case V4L2_CID_DO_WHITE_BALANCE:
      return "Do White Balance";
    case V4L2_CID_AUTOBRIGHTNESS:
      return "Auto Brightness";
    case V4L2_CID_BAND_STOP_FILTER:
      return "Band Stop Filter";
    case V4L2_CID_ILLUMINATORS_1:
      return "Illuminators 1";
    case V4L2_CID_ILLUMINATORS_2:
      return "Illuminators 2";
    case V4L2_CID_ISO_SENSITIVITY:
      return "ISO Sensitivity";
    case V4L2_CID_ISO_SENSITIVITY_AUTO:
      return "Auto ISO Sensitivity";
    case V4L2_CID_EXPOSURE_METERING:
      return "Exposure Metering";
    case V4L2_CID_SCENE_MODE:
      return "Scene Mode";
    case V4L2_CID_3A_LOCK:
      return "3A Lock (Auto Exposure, White Balance, and Focus Lock)";
    case V4L2_CID_AUTO_FOCUS_START:
      return "Start Auto Focus";
    case V4L2_CID_AUTO_FOCUS_STOP:
      return "Stop Auto Focus";
    case V4L2_CID_AUTO_FOCUS_RANGE:
      return "Auto Focus Range";
    default:
      return "Unknown Control";
  }
}
