#ifndef SCALABLE_LABEL_H
#define SCALABLE_LABEL_H

#include <QLabel>
#include <QObject>
#include <QWidget>

#include <QEvent>
#include <QMouseEvent>

#include <mutex>

class scalable_label : public QLabel {
  Q_OBJECT
public:
  explicit scalable_label(QWidget *parent = nullptr);

  volatile bool is_busy = false;

  std::mutex lock;

signals:
  void zoomed(const double r_relative_pos, const double c_relative_pos,
              const bool is_zooming_up);
  void moved(const double r_relative_pos, const double c_relative_pos);

protected:
  void wheelEvent(QWheelEvent *event) override;

  void mouseMoveEvent(QMouseEvent *event) override;
};

#endif // SCALABLE_LABEL_H
