#include "./ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow), m_camera(new usb_cam)
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
}

MainWindow::~MainWindow()
{
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

void MainWindow::read_device_value()
{
  if (m_camera->streaming)
  {
    int br, ct, sat, hue, wb_temp, gamma, sharpness, backlight, auto_white_balance;

    // Query ranges and set slider ranges for each control
    v4l2_queryctrl br_ctrl = m_camera->query_control(V4L2_CID_BRIGHTNESS);
    v4l2_queryctrl ct_ctrl = m_camera->query_control(V4L2_CID_CONTRAST);
    v4l2_queryctrl sat_ctrl = m_camera->query_control(V4L2_CID_SATURATION);
    v4l2_queryctrl hue_ctrl = m_camera->query_control(V4L2_CID_HUE);
    v4l2_queryctrl wb_ctrl = m_camera->query_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
    v4l2_queryctrl gamma_ctrl = m_camera->query_control(V4L2_CID_GAMMA);
    v4l2_queryctrl sharp_ctrl = m_camera->query_control(V4L2_CID_SHARPNESS);
    v4l2_queryctrl backlight_ctrl = m_camera->query_control(V4L2_CID_BACKLIGHT_COMPENSATION);
    v4l2_queryctrl auto_wb_ctrl = m_camera->query_control(V4L2_CID_AUTO_WHITE_BALANCE);

    // Set ranges and values or disable unsupported controls

    // Brightness
    if (br_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->brightnessSlider->setDisabled(true);
      ui->brightness->setText("NON");
    }
    else
    {
      ui->brightnessSlider->setRange(br_ctrl.minimum, br_ctrl.maximum);
      br = m_camera->get_control(V4L2_CID_BRIGHTNESS);
      ui->brightnessSlider->setValue(br);
      ui->brightness->setText(QString::number(br));
      ui->brightnessSlider->setEnabled(true);
    }

    // Contrast
    if (ct_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->contrastSlider->setDisabled(true);
      ui->contrast->setText("NON");
    }
    else
    {
      ui->contrastSlider->setRange(ct_ctrl.minimum, ct_ctrl.maximum);
      ct = m_camera->get_control(V4L2_CID_CONTRAST);
      ui->contrastSlider->setValue(ct);
      ui->contrast->setText(QString::number(ct));
      ui->contrastSlider->setEnabled(true);
    }

    // Saturation
    if (sat_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->saturationSlider->setDisabled(true);
      ui->saturation->setText("NON");
    }
    else
    {
      ui->saturationSlider->setRange(sat_ctrl.minimum, sat_ctrl.maximum);
      sat = m_camera->get_control(V4L2_CID_SATURATION);
      ui->saturationSlider->setValue(sat);
      ui->saturation->setText(QString::number(sat));
      ui->saturationSlider->setEnabled(true);
    }

    // Hue
    if (hue_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->hueSlider->setDisabled(true);
      ui->hue->setText("NON");
    }
    else
    {
      ui->hueSlider->setRange(hue_ctrl.minimum, hue_ctrl.maximum);
      hue = m_camera->get_control(V4L2_CID_HUE);
      ui->hueSlider->setValue(hue);
      ui->hue->setText(QString::number(hue));
      ui->hueSlider->setEnabled(true);
    }

    // White Balance Auto Check
    auto_white_balance = m_camera->get_control(V4L2_CID_AUTO_WHITE_BALANCE);
    if (auto_white_balance == 1)
    {
      // Auto white balance enabled, check the box and disable manual white balance
      ui->whiteBalanceAuto->setChecked(true);
      ui->whiteBalanceSlider->setDisabled(true);
      ui->whiteBalance->setText("AUTO");
    }
    else
    {
      // Auto white balance disabled, uncheck the box and enable manual white balance
      ui->whiteBalanceAuto->setChecked(false);
      if (wb_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
      {
        ui->whiteBalanceSlider->setDisabled(true);
        ui->whiteBalance->setText("NON");
      }
      else
      {
        ui->whiteBalanceSlider->setRange(wb_ctrl.minimum, wb_ctrl.maximum);
        wb_temp = m_camera->get_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE);
        ui->whiteBalanceSlider->setValue(wb_temp);
        ui->whiteBalance->setText(QString::number(wb_temp));
        ui->whiteBalanceSlider->setEnabled(true);
      }
    }

    // Gamma
    if (gamma_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->gammaSlider->setDisabled(true);
      ui->gamma->setText("NON");
    }
    else
    {
      ui->gammaSlider->setRange(gamma_ctrl.minimum, gamma_ctrl.maximum);
      gamma = m_camera->get_control(V4L2_CID_GAMMA);
      ui->gammaSlider->setValue(gamma);
      ui->gamma->setText(QString::number(gamma));
      ui->gammaSlider->setEnabled(true);
    }

    // Sharpness
    if (sharp_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->sharpnessSlider->setDisabled(true);
      ui->sharpness->setText("NON");
    }
    else
    {
      ui->sharpnessSlider->setRange(sharp_ctrl.minimum, sharp_ctrl.maximum);
      sharpness = m_camera->get_control(V4L2_CID_SHARPNESS);
      ui->sharpnessSlider->setValue(sharpness);
      ui->sharpness->setText(QString::number(sharpness));
      ui->sharpnessSlider->setEnabled(true);
    }

    // Backlight Compensation
    if (backlight_ctrl.flags & V4L2_CTRL_FLAG_DISABLED)
    {
      ui->backlightSlider->setDisabled(true);
      ui->backlight->setText("NON");
    }
    else
    {
      ui->backlightSlider->setRange(backlight_ctrl.minimum, backlight_ctrl.maximum);
      backlight = m_camera->get_control(V4L2_CID_BACKLIGHT_COMPENSATION);
      ui->backlightSlider->setValue(backlight);
      ui->backlight->setText(QString::number(backlight));
      ui->backlightSlider->setEnabled(true);
    }
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
    streamTimer->start(30);
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
  // Disable auto white balance if it's enabled
  int result = m_camera->set_control(V4L2_CID_AUTO_WHITE_BALANCE, 0);
  if (result != 0)
  {
    std::cerr << "Failed to disable auto WB" << std::endl;
    ui->whiteBalance->setText("Failed to disable auto WB");
    return;
  }

  // Set the white balance temperature
  result = m_camera->set_control(V4L2_CID_WHITE_BALANCE_TEMPERATURE, value);

  // Check if the control was successfully set
  if (result == 0)
  {
    // Update the label if successful
    ui->whiteBalance->setText(QString::number(value));
  }
  else
  {
    // Error handling if it fails to set the control
    ui->whiteBalance->setText("Failed to set WB");
  }
}

void MainWindow::on_whiteBalanceAuto_stateChanged(int state)
{
  if (state == Qt::Checked)
  {
    m_camera->set_control(V4L2_CID_AUTO_WHITE_BALANCE, 1);
    ui->whiteBalanceSlider->setDisabled(true);
    ui->whiteBalance->setText("AUTO");
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

void MainWindow::on_backlightSlider_valueChanged(int value)
{
  m_camera->set_control(V4L2_CID_BACKLIGHT_COMPENSATION, value);
  ui->backlight->setText(QString::number(value));
}
