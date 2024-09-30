#include "./ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::MainWindow), m_camera(new usb_cam), m_joystick(new Joystick("/dev/input/js0"))
{
  ui->setupUi(this);
  QIcon icon(":/image/images/icon.png");
  setWindowIcon(icon);

  devices = m_camera->find_device();
  for (const auto& item : devices)
  {
    ui->devices->addItem(QString::fromStdString(item.path + " - " + item.device_name));
  }
  device_info = m_camera->get_device_info(devices[0].path);

  ui->quality->clear();
  for (const auto& resInfo : device_info.resolution_info)
  {
    QString resolution = QString::number(resInfo.resolution.first) + "x" + QString::number(resInfo.resolution.second);
    ui->quality->addItem(resolution);
  }

  ui->format->clear();
  for (const auto& format : device_info.formats)
  {
    ui->format->addItem(QString::fromStdString(format));
  }

  if (m_joystick->isConnected())
  {
    m_joystick->startEventThread();
  }
}

MainWindow::~MainWindow()
{
  m_joystick->stopEventThread();
  delete m_camera;
  delete m_joystick;
  delete ui;
}

void MainWindow::update_frame()
{
  if (!m_camera->m_image.empty())
  {
    cv::Mat rgbFrame;
    cv::cvtColor(m_camera->m_image, rgbFrame, cv::COLOR_BGR2RGB);

    QImage qimg(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, rgbFrame.step, QImage::Format_RGB888);

    ui->img->setPixmap(QPixmap::fromImage(qimg).scaled(ui->img->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  }
}

void MainWindow::on_devices_currentIndexChanged(int index)
{
  device_info = m_camera->get_device_info(devices[index].path);

  ui->quality->clear();
  for (const auto& resInfo : device_info.resolution_info)
  {
    QString resolution = QString::number(resInfo.resolution.first) + "x" + QString::number(resInfo.resolution.second);
    ui->quality->addItem(resolution);
  }

  ui->format->clear();
  for (const auto& format : device_info.formats)
  {
    ui->format->addItem(QString::fromStdString(format));
  }
}

void MainWindow::on_quality_currentIndexChanged(int index)
{
  ui->fps->clear();

  if (index >= 0 && index < device_info.resolution_info.size())
  {
    const auto& selectedResolutionInfo = device_info.resolution_info[index];
    for (const auto& fps : selectedResolutionInfo.fps)
    {
      ui->fps->addItem(QString::number(fps) + " fps");
    }
  }
}

void MainWindow::on_stream_clicked()
{
  if (m_camera->streaming)
  {
    if (streamTimer)
    {
      streamTimer->stop();
      delete streamTimer;
      streamTimer = nullptr;
    }

    m_camera->stop_stream();
    ui->img->clear();
    ui->stream->setStyleSheet("color: red;");
    ui->stream->setText("STREAM");
    ui->devices->setEnabled(true);
  }
  else
  {
    int deviceIndex = ui->devices->currentIndex();
    int qualityIndex = ui->quality->currentIndex();
    int fpsIndex = ui->fps->currentIndex();
    int formatIndex = ui->format->currentIndex();

    if (deviceIndex < 0 || qualityIndex < 0 || fpsIndex < 0 || formatIndex < 0)
    {
      std::cerr << "Invalid selection" << std::endl;
      return;
    }

    m_deviceConfig config;
    config.path = devices[deviceIndex].path;
    config.device_name = devices[deviceIndex].device_name;
    config.resolution = device_info.resolution_info[qualityIndex].resolution;
    config.fps = device_info.resolution_info[qualityIndex].fps[fpsIndex];
    config.format = ui->format->itemText(formatIndex).toStdString();

    m_camera->start_stream(config);
    read_device_value();

    streamTimer = new QTimer(this);
    connect(streamTimer, &QTimer::timeout, this, &MainWindow::update_frame);
    streamTimer->start(10);
    ui->stream->setStyleSheet("color: green;");
    ui->stream->setText("STOP");
    ui->devices->setEnabled(false);
  }
}

void MainWindow::on_reset_clicked()
{
  if (m_camera->streaming)
  {
    m_camera->reset_controls_to_default();
    read_device_value();
  }
}

void MainWindow::on_brightnessSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_BRIGHTNESS, value);
  ui->brightness->setText(QString::number(value));
}

void MainWindow::on_contrastSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_CONTRAST, value);
  ui->contrast->setText(QString::number(value));
}

void MainWindow::on_saturationSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_SATURATION, value);
  ui->saturation->setText(QString::number(value));
}

void MainWindow::on_hueSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_HUE, value);
  ui->hue->setText(QString::number(value));
}

void MainWindow::on_whiteBalanceSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE, value);
  ui->whiteBalance->setText(QString::number(value));
}

void MainWindow::on_whiteBalanceAuto_stateChanged(int state)
{
  if (state == Qt::Checked)
  {
    m_camera->set_control(V4L2_CID_AUTO_WHITE_BALANCE, 1);
    ui->whiteBalanceSlider->setDisabled(true);
  }
  else
  {
    m_camera->set_control(V4L2_CID_AUTO_WHITE_BALANCE, 0);
    ui->whiteBalanceSlider->setEnabled(true);

    int wb_temp = m_camera->get_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
    ui->whiteBalanceSlider->setValue(wb_temp);
    ui->whiteBalance->setText(QString::number(wb_temp));
  }
}

void MainWindow::on_gammaSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_GAMMA, value);
  ui->gamma->setText(QString::number(value));
}

void MainWindow::on_sharpnessSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_SHARPNESS, value);
  ui->sharpness->setText(QString::number(value));
}

void MainWindow::on_exposureSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_EXPOSURE, value);
  ui->exposure->setText(QString::number(value));
}

void MainWindow::on_exposureAuto_stateChanged(int state)
{
  if (state == Qt::Checked)
  {
    m_camera->set_control(V4L2_CID_EXPOSURE_AUTO, 1);
    ui->exposureSlider->setDisabled(true);
  }
  else
  {
    m_camera->set_control(V4L2_CID_EXPOSURE_AUTO, 0);
    ui->exposureSlider->setEnabled(true);

    int exposure = m_camera->get_control(V4L2_CID_EXPOSURE);
    ui->exposureSlider->setValue(exposure);
    ui->exposure->setText(QString::number(exposure));
  }
}

void MainWindow::on_gainSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_GAIN, value);
  ui->gain->setText(QString::number(value));
}

void MainWindow::on_panSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_PAN_ABSOLUTE, value);
  ui->pan->setText(QString::number(value));
}

void MainWindow::on_tiltSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_TILT_ABSOLUTE, value);
  ui->tilt->setText(QString::number(value));
}

void MainWindow::on_backlightSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_BACKLIGHT_COMPENSATION, value);
  ui->backlight->setText(QString::number(value));
}

void MainWindow::on_powerLineSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_POWER_LINE_FREQUENCY, value);
  ui->powerLine->setText(QString::number(value));
}

void MainWindow::on_zoomSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_ZOOM_ABSOLUTE, value);
  ui->zoom->setText(QString::number(value));
}

void MainWindow::on_focusSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_FOCUS_ABSOLUTE, value);
  ui->focus->setText(QString::number(value));
}

void MainWindow::on_focusAuto_stateChanged(int state)
{
  if (state == Qt::Checked)
  {
    m_camera->set_control(V4L2_CID_FOCUS_AUTO, 1);
    ui->focusSlider->setDisabled(true);
  }
  else
  {
    m_camera->set_control(V4L2_CID_FOCUS_AUTO, 0);
    ui->focusSlider->setEnabled(true);

    int focus = m_camera->get_control(V4L2_CID_FOCUS_AUTO);
    ui->focusSlider->setValue(focus);
    ui->focus->setText(QString::number(focus));
  }
}

void MainWindow::read_device_value()
{
  if (m_camera->streaming)
  {
    set_qslider_from_query(ui->brightnessSlider, ui->brightness, V4L2_CID_BRIGHTNESS);
    set_qslider_from_query(ui->contrastSlider, ui->contrast, V4L2_CID_CONTRAST);
    set_qslider_from_query(ui->saturationSlider, ui->saturation, V4L2_CID_SATURATION);
    set_qslider_from_query(ui->hueSlider, ui->hue, V4L2_CID_HUE);
    set_qslider_from_query(ui->whiteBalanceSlider, ui->whiteBalance, ui->whiteBalanceAuto, V4L2_CID_AUTO_WHITE_BALANCE,
                           V4L2_CID_WHITE_BALANCE_TEMPERATURE);
    set_qslider_from_query(ui->gammaSlider, ui->gamma, V4L2_CID_GAMMA);
    set_qslider_from_query(ui->sharpnessSlider, ui->sharpness, V4L2_CID_SHARPNESS);
    set_qslider_from_query(ui->exposureSlider, ui->exposure, ui->exposureAuto, V4L2_CID_EXPOSURE_AUTO,
                           V4L2_CID_EXPOSURE_ABSOLUTE);
    set_qslider_from_query(ui->gainSlider, ui->gain, V4L2_CID_GAIN);
    set_qslider_from_query(ui->panSlider, ui->pan, V4L2_CID_PAN_ABSOLUTE);
    set_qslider_from_query(ui->tiltSlider, ui->tilt, V4L2_CID_TILT_ABSOLUTE);
    set_qslider_from_query(ui->backlightSlider, ui->backlight, V4L2_CID_BACKLIGHT_COMPENSATION);
    set_qslider_from_query(ui->powerLineSlider, ui->powerLine, V4L2_CID_POWER_LINE_FREQUENCY);
    set_qslider_from_query(ui->zoomSlider, ui->zoom, V4L2_CID_ZOOM_ABSOLUTE);
    set_qslider_from_query(ui->focusSlider, ui->focus, ui->focusAuto, V4L2_CID_FOCUS_AUTO, V4L2_CID_FOCUS_ABSOLUTE);
  }
}

void MainWindow::set_qslider_from_query(QSlider* slider, QLabel* label, int control_id)
{
  v4l2_queryctrl queryctrl;
  int value;

  if (m_camera->query_control(control_id, queryctrl))
  {
    slider->setEnabled(true);
    value = m_camera->get_control(control_id);
    slider->setRange(queryctrl.minimum, queryctrl.maximum);
    slider->setValue(value);
    label->setText(QString::number(value));
  }
  else
  {
    slider->setDisabled(true);
    label->setText("NA");
  }
}

void MainWindow::set_qslider_from_query(QSlider* slider, QLabel* label, QCheckBox* check, int control_id_auto,
                                        int control_id)
{
  v4l2_queryctrl queryctrl;
  int auto_value = m_camera->get_control(control_id_auto);
  int value;

  if (auto_value == 1)
  {
    check->setChecked(true);
    slider->setDisabled(true);
    label->setText("AUTO");
  }
  else
  {
    check->setChecked(false);
    m_camera->query_control(control_id, queryctrl);
    if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      slider->setDisabled(true);
      label->setText("NA");
    }
    else
    {
      slider->setEnabled(true);
      value = m_camera->get_control(control_id);
      slider->setRange(queryctrl.minimum, queryctrl.maximum);
      slider->setValue(value);
      label->setText(QString::number(value));
    }
  }
}
