#include "joystick.h"

Joystick::Joystick(std::string device) : device_path(device), joystick_fd(-1), running(false)
{
  joystick_fd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
  if (joystick_fd == -1)
  {
    CERR_ENDL("Failed to open joystick: " << device);
  }
  else
  {
    COUT("Joystick connected: ");
    COUT_ENDL(device);
    initialize();
  }
}

Joystick::~Joystick()
{
  stopEventThread();
  if (joystick_fd != -1)
  {
    close(joystick_fd);
    COUT_ENDL("Joystick disconnected.");
  }
}

bool Joystick::isConnected() const
{
  return joystick_fd != -1;
}

void Joystick::startEventThread()
{
  if (!running && joystick_fd != -1)
  {
    running = true;
    event_thread = std::thread(&Joystick::readEvent, this);
  }
}

void Joystick::stopEventThread()
{
  if (running)
  {
    running = false;
    if (event_thread.joinable())
    {
      event_thread.join();
    }
  }
}

void Joystick::initialize()
{
  if (joystick_fd == -1)
  {
    CERR_ENDL("Joystick not connected!");
    return;
  }

  // Get the number of axes
  __u8 axes_count;
  if (ioctl(joystick_fd, JSIOCGAXES, &axes_count) == -1)
  {
    CERR_ENDL("Failed to get axes count.");
    axes_count = 0;
  }

  // Get the number of buttons
  __u8 buttons_count;
  if (ioctl(joystick_fd, JSIOCGBUTTONS, &buttons_count) == -1)
  {
    CERR_ENDL("Failed to get buttons count.");
    buttons_count = 0;
  }

  // Resize the axis and button vectors
  axis.resize(axes_count, 0);
  button.resize(buttons_count, 0);

  COUT_ENDL("Joystick initialized with " << (int)axes_count << " axes and " << (int)buttons_count << " buttons.");
}

void Joystick::readEvent()
{
  while (running)
  {
    struct js_event event;
    ssize_t bytes = read(joystick_fd, &event, sizeof(event));

    if (bytes == sizeof(event))
    {
      switch (event.type & ~JS_EVENT_INIT)
      {
        case JS_EVENT_AXIS:
          if (event.number < axis.size())
          {
            axis[event.number] = event.value;
            COUT_ENDL("Axis event: Axis " << (int)event.number << " Value " << event.value);
          }
          break;
        case JS_EVENT_BUTTON:
          if (event.number < button.size())
          {
            button[event.number] = event.value;
            COUT_ENDL("Button event: Button " << (int)event.number << " Value " << event.value);
          }
          break;
        default:
          break;
      }
    }
  }
}
