#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <iostream>
#include "usb_camera.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots:
  void on_stream_clicked();
  void on_reset_clicked();
  void on_quality_currentIndexChanged(int index);
  void on_devices_currentIndexChanged(int index);

  void on_brightnessSlider_valueChanged(int value);
  void on_contrastSlider_valueChanged(int value);
  void on_saturationSlider_valueChanged(int value);
  void on_hueSlider_valueChanged(int value);
  void on_whiteBalanceSlider_valueChanged(int value);
  void on_whiteBalanceAuto_stateChanged(int arg1);
  void on_backlightSlider_valueChanged(int value);
  void on_sharpnessSlider_valueChanged(int value);
  void on_gammaSlider_valueChanged(int value);

private slots:
  void update_frame();

private:
  Ui::MainWindow* ui;
  usb_cam* m_camera;
  QTimer* streamTimer;

  std::vector<deviceData> devices;
  deviceInfo device_info;

  void read_device_value();
};
#endif  // MAINWINDOW_H
