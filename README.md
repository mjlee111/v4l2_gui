# v4l2_gui

`v4l2_gui` is a graphical user interface (GUI) application that enables users to control video devices using the V4L2 (Video4Linux2) API. It allows for streaming video, adjusting image quality parameters such as brightness, contrast, saturation, hue, white balance, and more. The application also supports joystick input for pan and tilt control, as well as the ability to reset camera controls to default settings.

![sample](docs/run.gif)

## Features

- **Device Selection**: Automatically detects available video devices and allows users to select a device for streaming.
- **Stream Control**: Start and stop video streams with configurable resolution, FPS, and format settings.
- **Real-Time Image Display**: Shows the live video stream in a resizable window with aspect ratio preservation.
- **Camera Control**: Adjust camera parameters like brightness, contrast, saturation, white balance, and more through sliders.
- **Pan and Tilt Control**: Supports pan and tilt control through V4L2 controls, with joystick input support.
- **Auto vs. Manual Modes**: Toggle between automatic and manual modes for exposure, white balance, and focus.
- **Reset Controls**: Reset all camera parameters to their default values.

## Requirements

- **Qt 5 or higher**: For the GUI components.
- **OpenCV 4.5 or higher**: For handling image processing and displaying video frames.
- **V4L2 (Video4Linux2)**: To interface with video capture devices.
- **Joystick support (Optional)**: Requires `/dev/input/js0` device for joystick control.

## Installation

### Prerequisites

Make sure to install the following dependencies:

```bash
sudo apt-get install qt5-default libopencv-dev v4l-utils libudev-dev
```

### Building the Project

1. Clone the repository:
   ```bash
   git clone https://github.com/mjlee111/v4l2_gui.git
   cd v4l2_gui
   ```

2. Build the project using `cmake`:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

## Usage

1. Launch the application:
   ```bash
   ./v4l2_gui
   ```

2. Select a video device from the dropdown menu.

3. Adjust the resolution, FPS, and format from the available options.

4. Click the `STREAM` button to start or stop the video stream.

5. Use the sliders to adjust image properties such as brightness, contrast, and white balance.

6. Joystick control is available for supported devices to manage pan and tilt functions.

## Controls

- **Brightness**: Adjusts image brightness.
- **Contrast**: Adjusts image contrast.
- **Saturation**: Adjusts the saturation of the image.
- **Hue**: Changes the hue of the image.
- **White Balance**: Manually adjust white balance or enable auto white balance.
- **Exposure**: Set exposure manually or enable automatic exposure control.
- **Gamma**: Adjusts gamma correction.
- **Sharpness**: Sets the sharpness level.
- **Pan & Tilt**: Use sliders or a joystick to control the pan and tilt of the camera.

## Known Issues

- Some camera controls may not be supported on all devices. If a control is unsupported, it will be disabled and marked as "NA".
- Joystick control is only available if `/dev/input/js0` is detected. If the joystick is not connected, pan and tilt will have to be adjusted via sliders.

## Future Improvements

- **Multi-Device Support**: Allow simultaneous streaming from multiple cameras.
- **Preset Configurations**: Save and load camera settings for different environments.
- **Advanced Controls**: Add more controls for professional video tuning, such as exposure priority and gain.
- **Audio Streaming**: Capture and stream audio along with video.

## Contributing
I welcome all contributions! Whether it's bug reports, feature suggestions, or pull requests, your input helps us improve. If you're interested in contributing, please check out our contributing guidelines or submit an issue.

## License
This project is licensed under the [Apache 2.0 License](LICENSE). Feel free to use and distribute it according to the terms of the license.

## Contact
If you have any questions or feedback, don't hesitate to reach out! You can contact us at [menggu1234@naver.com][email].

[email]: mailto:menggu1234@naver.com
