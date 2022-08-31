#include "zoomer.h"
#include "ui_zoomer.h"
#include <QByteArray>

#include <ctime>
#include <iostream>

using std::cout, std::endl;

zoomer::zoomer(QWidget *parent) : QWidget(parent), ui(new Ui::zoomer) {
  ui->setupUi(this);

  // this->minmin = -2 - 2j;
  this->minmin.fl[0] = -2;
  this->minmin.fl[1] = -2;
  // this->maxmax = 2 + 2j;
  this->maxmax.fl[0] = 2;
  this->maxmax.fl[1] = 2;

  connect(ui->image, &scalable_label::moved, this, &zoomer::mouse_move);

  connect(ui->image, &scalable_label::zoomed, this, &zoomer::update_scale);

  display_range();

  // repaint();
}

zoomer::~zoomer() { delete ui; }

void zoomer::mouse_move(const double r_relative_pos,
                        const double c_relative_pos) {
  cplx_union_d mouse = this->minmin;
  const bs_float r_span = this->maxmax.fl[1] - this->minmin.fl[1];
  const bs_float c_span = this->maxmax.fl[0] - this->minmin.fl[0];

  mouse.fl[1] += (1.0 - r_relative_pos) * r_span;
  mouse.fl[0] += c_relative_pos * c_span;

  QString str = "Mouse : ";
  str += QString::number((double)mouse.fl[0]);
  if (mouse.fl[1] >= 0)
    str += "+";

  str += QString::number((double)mouse.fl[1]) + 'i';

  ui->label_show_mouse->setText(str);
}

void zoomer::display_range() const {

  {
    QString str = QStringLiteral("Minmin : ");

    str += QString::number((double)this->minmin.fl[0]);
    if (this->minmin.fl[1] >= 0)
      str += "+";
    str += QString::number((double)this->minmin.fl[1]) + 'i';
    ui->label_show_minmin->setText(str);
  }

  {
    QString str = QStringLiteral("Maxmax : ");

    str += QString::number((double)this->maxmax.fl[0]);
    if (this->maxmax.fl[1] >= 0)
      str += "+";
    str += QString::number((double)this->maxmax.fl[1]) + 'i';
    ui->label_show_maxmax->setText(str);
  }

  const double r_span = this->maxmax.fl[1] - this->minmin.fl[1];

  ui->show_scale->setText(QString::number(r_span));

  cplx_union_d center;
  center.value = (this->maxmax.value + this->minmin.value) / 2;

  {
    QString str = QStringLiteral("Center : ");

    str += QString::number((double)center.fl[0]);

    if (center.fl[1] >= 0)
      str += "+";
    str += QString::number((double)center.fl[1]) + 'i';

    ui->label_show_center_dec->setText(str);
  }

  {
    QString str = QStringLiteral("0x");

    char *bin = (char *)(&center);

    str += QByteArray::fromRawData(bin, sizeof(center)).toHex();

    ui->show_center_hex->setText(str);
  }
}

void zoomer::repaint() {
  //

  this->setWindowTitle("Fractal zoomer (computing, please wait...)");

  ::mat_age *const mat = new mat_age;

  if (mat == NULL) {
    cout << "failed to allocate space for mat" << endl;
    return;
  }

  QImage img(burning_ship_cols, burning_ship_rows,
             QImage::Format::Format_Grayscale8);

  if (img.isNull()) {
    cout << "Failed to allocate space for image" << endl;
  }

  std::clock_t clk = std::clock();
  ::compute_frame(mat, this->minmin.value, this->maxmax.value,
                  ui->spin_max_iter->value());
  clk = std::clock() - clk;

  ::render(mat, img.scanLine(0), ui->spin_max_iter->value());

  delete mat;
  /*
    img = img.scaled(ui->image->width(), ui->image->height(),
                     Qt::AspectRatioMode::IgnoreAspectRatio,
                     Qt::TransformationMode::FastTransformation);
                     */

  ui->image->setPixmap(QPixmap::fromImage(img));

  this->setWindowTitle(
      QStringLiteral("Fractal zoomer. Computation finished in ") +
      QString::number(clk * 1000.0 / CLOCKS_PER_SEC) + " ms");
}

void zoomer::update_scale(const double r_relative_pos,
                          const double c_relative_pos,
                          const bool is_zooming_up) {

  const bs_float now_r_span = this->maxmax.fl[1] - this->minmin.fl[1];
  const bs_float now_c_span = this->maxmax.fl[0] - this->minmin.fl[0];

  const bs_float scale_speed = ui->spin_zoom_speed->value();
  const bs_float next_r_span =
      now_r_span * (is_zooming_up ? (1 / scale_speed) : scale_speed);
  const bs_float next_c_span = next_r_span * cols_div_rows;

  cplx_union_d new_center = this->minmin;
  new_center.fl[0] += c_relative_pos * now_c_span;
  new_center.fl[1] += (1 - r_relative_pos) * now_r_span;

  this->minmin = new_center;
  this->minmin.fl[0] -= next_c_span / 2;
  this->minmin.fl[1] -= next_r_span / 2;

  this->maxmax = new_center;
  this->maxmax.fl[0] += next_c_span / 2;
  this->maxmax.fl[1] += next_r_span / 2;

  display_range();

  repaint();

  // const cplx_d now_center = (this->maxmax.value + this->minmin.value) / 2;
}