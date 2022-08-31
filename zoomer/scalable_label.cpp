#include "scalable_label.h"

#include <iostream>

using std::cout, std::endl;

scalable_label::scalable_label(QWidget *parent) : QLabel(parent) {
  this->setMouseTracking(true);
}

void scalable_label::wheelEvent(QWheelEvent *event) {

  if (!this->lock.try_lock()) {
    return;
  }

  const QPointF point = event->position();

  // bool is_inverted = event->isInverted();
  // inverted() and isInverted don't work on windows10
  const QPointF angle_delta = event->angleDelta();

  bool is_scaling_up;

  if (angle_delta.y() != 0) {
    is_scaling_up = (angle_delta.y() > 0);
  } else {
    is_scaling_up = (angle_delta.x() > 0);
  }

  event->accept();

  emit zoomed(point.y() / (this->height() - 1), point.x() / (this->width() - 1),
              is_scaling_up);
  /*
    cout << "position : " << point.x() << ", " << point.y();
    cout << ", angle_delta = " << angle_delta.x() << ", " << angle_delta.y()
         << endl;
         */
  //<< ", inverted = " << (is_inverted ? "true" : "false") << endl;

  this->lock.unlock();
}

void scalable_label::mouseMoveEvent(QMouseEvent *event) {

  const QPointF point = event->position();

  emit moved(point.y() / (this->height() - 1), point.x() / (this->width() - 1));

  event->accept();
}