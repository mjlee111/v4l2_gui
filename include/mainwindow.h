#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define DEBUG

#include <QMainWindow>
#include <QTimer>
#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <iostream>
#include "usb_camera.h"
#include "joystick.h"

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
  void on_gammaSlider_valueChanged(int value);
  void on_sharpnessSlider_valueChanged(int value);
  void on_exposureSlider_valueChanged(int value);
  void on_exposureAuto_stateChanged(int arg1);
  void on_gainSlider_valueChanged(int value);

  void on_panSlider_valueChanged(int value);
  void on_tiltSlider_valueChanged(int value);
  void on_backlightSlider_valueChanged(int value);
  void on_powerLineSlider_valueChanged(int value);
  void on_zoomSlider_valueChanged(int value);
  void on_focusSlider_valueChanged(int value);
  void on_focusAuto_stateChanged(int arg1);

private slots:
  void update_frame();

private:
  Ui::MainWindow* ui;
  usb_cam* m_camera;
  Joystick* m_joystick;
  QTimer* streamTimer;

  std::vector<deviceData> devices;
  m_deviceInfo device_info;

  void read_device_value();
  void set_qslider_from_query(QSlider* slider, QLabel* label, int control_id);
  void set_qslider_from_query(QSlider* slider, QLabel* label, QCheckBox* check, int control_id_auto, int control_id);
};
#endif  // MAINWINDOW_H
