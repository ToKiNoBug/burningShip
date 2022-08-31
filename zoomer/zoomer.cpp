#include "zoomer.h"
#include "ui_zoomer.h"
#include <QByteArray>

using namespace std::complex_literals;

zoomer::zoomer(QWidget *parent) : QWidget(parent), ui(new Ui::zoomer) {
  ui->setupUi(this);

  // this->minmin = -2 - 2j;
  this->minmin.fl64[0] = -2;
  this->minmin.fl64[1] = -2;
  // this->maxmax = 2 + 2j;
  this->maxmax.fl64[0] = 2;
  this->maxmax.fl64[1] = 2;

  connect(ui->image, &scalable_label::moved, this, &zoomer::mouse_move);

  connect(ui->image, &scalable_label::zoomed, this, &zoomer::update_scale);

  display_range();
}

zoomer::~zoomer() { delete ui; }

void zoomer::mouse_move(const double r_relative_pos,
                        const double c_relative_pos) {
  cplx_union_d mouse = this->minmin;
  const double r_span =
      ::cimag(this->maxmax.value) - ::cimag(this->minmin.value);
  const double c_span =
      ::creal(this->maxmax.value) - ::creal(this->minmin.value);

  mouse.fl64[1] += (1.0 - r_relative_pos) * r_span;
  mouse.fl64[0] += c_relative_pos * c_span;

  QString str = "Mouse : ";
  str += QString::number(::creal(mouse.value));
  if (::cimag(mouse.value) >= 0)
    str += '+';

  str += QString::number(::cimag(mouse.value)) + 'i';

  ui->label_show_mouse->setText(str);
}

void zoomer::display_range() const {

  {
    QString str = QStringLiteral("Minmin : ");

    str += QString::number(this->minmin.fl64[0]);
    if (this->minmin.fl64[1] >= 0)
      str += '+';
    str += QString::number(this->minmin.fl64[1]) + 'i';
    ui->label_show_minmin->setText(str);
  }

  {
    QString str = QStringLiteral("Maxmax : ");

    str += QString::number(this->maxmax.fl64[0]);
    if (this->maxmax.fl64[1] >= 0)
      str += '+';
    str += QString::number(this->maxmax.fl64[1]) + 'i';
    ui->label_show_maxmax->setText(str);
  }

  const double r_span =
      ::cimag(this->maxmax.value) - ::cimag(this->minmin.value);

  ui->label_show_scale->setText(QStringLiteral("Scale by height : ") +
                                QString::number(r_span));

  cplx_union_d center;
  center.value = (this->maxmax.value + this->minmin.value) / 2;

  {
    QString str = QStringLiteral("Center : ");

    str += QString::number(center.fl64[0]);

    if (center.fl64[1] >= 0)
      str += '+';
    str += QString::number(center.fl64[1]) + 'i';

    ui->label_show_center_dec->setText(str);
  }

  {
    QString str = QStringLiteral("Center in hex : 0x");

    char *bin = (char *)(&center);

    str += QByteArray::fromRawData(bin, sizeof(center)).toHex();

    ui->show_center_hex->setText(str);
  }
}

void zoomer::repaint() const {
  //
}

void zoomer::update_scale(const double r_relative_pos,
                          const double c_relative_pos,
                          const bool is_zooming_up) {

  const double now_r_span =
      ::cimag(this->maxmax.value) - ::cimag(this->minmin.value);
  const double now_c_span =
      ::creal(this->maxmax.value) - ::creal(this->minmin.value);

  const double scale_speed = ui->spin_zoom_speed->value();
  const double next_r_span =
      now_r_span * (is_zooming_up ? (1 / scale_speed) : scale_speed);
  const double next_c_span = next_r_span * cols_div_rows;

  cplx_union_d new_center = this->minmin;
  new_center.fl64[0] += c_relative_pos * now_c_span;
  new_center.fl64[1] += (1 - r_relative_pos) * now_r_span;

  this->minmin = new_center;
  this->minmin.fl64[0] -= next_c_span / 2;
  this->minmin.fl64[1] -= next_r_span / 2;

  this->maxmax = new_center;
  this->maxmax.fl64[0] += next_c_span / 2;
  this->maxmax.fl64[1] += next_r_span / 2;

  display_range();

  ui->image->setEnabled(false);

  repaint();

  ui->image->setEnabled(true);

  // const cplx_d now_center = (this->maxmax.value + this->minmin.value) / 2;
}