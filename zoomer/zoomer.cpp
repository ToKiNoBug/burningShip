#include "zoomer.h"
#include "ui_zoomer.h"
#include <QByteArray>
#include <burning_ship.h>

#include <ctime>
#include <iostream>

#include <QWidget>

#include <QPushButton>

#include <QFileDialog>

using std::cout, std::endl;

zoomer::zoomer(QWidget *parent)
    : QWidget(parent), ui(new Ui::zoomer), mat(new mat_age),
      mat_f32(new mat_age_f32), norm2(new norm2_matc1) {
  ui->setupUi(this);

  // this->minmin = -2 - 2j;
  this->minmin.fl[0] = -2;
  this->minmin.fl[1] = -2;
  // this->maxmax = 2 + 2j;
  this->maxmax.fl[0] = 2;
  this->maxmax.fl[1] = 2;

  connect(ui->image, &scalable_label::moved, this, &zoomer::mouse_move);

  connect(ui->image, &scalable_label::zoomed, this, &zoomer::update_scale);

  // void (zoomer::*ptr)() = &zoomer::update_center_and_scale;
  // connect(ui->btn_repaint, &QPushButton::click, this, ptr);

  display_range();

  this->opt.err_tolerence = 1e-6;
  opt.f_buffer = new double[32768];
  opt.hist_skip_cols = 0;
  opt.hist_skip_rows = 0;
  opt.L_mean_div_L_max = 0.2;
  opt.newton_max_it = 5000;
  opt.q_guess = 0.6;

  // repaint();
}

zoomer::~zoomer() {
  delete ui;
  delete mat;
  delete mat_f32;
  delete norm2;
  delete opt.f_buffer;
}

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

  if (!this->lock.try_lock()) {
    return;
  }

  if (this->scale_record.empty()) {
    const bs_float scale = this->maxmax.fl[0] - this->minmin.fl[0];

    const cplx_d center = (this->maxmax.value + this->minmin.value) / 2;
    this->scale_record.emplace_back(std::make_pair(center, scale));
  }

  this->setWindowTitle("Fractal zoomer (computing, please wait...)");

  if (this->mat == NULL) {
    cout << "failed to allocate space for this->mat" << endl;
    return;
  }

  QImage img(burning_ship_cols, burning_ship_rows,
             QImage::Format::Format_RGB888);

  if (img.isNull()) {
    cout << "Failed to allocate space for image" << endl;
  }

  ::bs_range_wind rwind;
  rwind.minmin = this->minmin.value;
  rwind.maxmax = this->maxmax.value;

  std::clock_t clk = std::clock();
  /*
  ::compute_frame(this->mat, this->minmin.value, this->maxmax.value,
                  ui->spin_max_iter->value());
                  */
  ::compute_frame_norm2c1_range(this->mat, rwind, ui->spin_max_iter->value(),
                                this->norm2);
  clk = std::clock() - clk;

  //::render_u8c1(this->mat, img.scanLine(0), ui->spin_max_iter->value());
  ::smooth_by_norm2(this->mat, this->norm2, this->mat_f32);

  {
    double q;
    double L_mean;
    ::smooth_age_by_q(this->mat, this->mat_f32, ui->spin_max_iter->value(),
                      &this->opt, this->mat_f32, &q, &L_mean);
    if (std::isnan(q)) {
      cout << "Error! q is nan." << endl;
      exit(0);
    }

    cout << "q = " << q << ", L_mean = " << L_mean << endl;

    this->opt.q_guess = q;

    if (q <= 0) {
      cout << "f = [";
      for (int idx = 0; idx <= ui->spin_max_iter->value(); idx++) {
        cout << this->opt.f_buffer[idx] << ", ";
      }
      cout << "];" << endl;
    }
  }
  /*
  bool ok = coloring_by_f32_u8c3_more(this->mat, this->mat_f32, img.scanLine(0),
                                      NAN, NAN);*/
  ::coloring_by_f32_u8c3(this->mat, this->mat_f32, img.bits());

  /*
    img = img.scaled(ui->image->width(), ui->image->height(),
                     Qt::AspectRatioMode::IgnoreAspectRatio,
                     Qt::TransformationMode::FastTransformation);
                     */

  ui->image->setPixmap(QPixmap::fromImage(img));

  this->setWindowTitle(
      QStringLiteral("Fractal zoomer. Computation finished in ") +
      QString::number(clk * 1000.0 / CLOCKS_PER_SEC) + " ms");

  this->lock.unlock();
}

void zoomer::update_scale(const double r_relative_pos,
                          const double c_relative_pos,
                          const bool is_zooming_up) {

  const bs_float prev_r_span = this->maxmax.fl[1] - this->minmin.fl[1];
  const bs_float prev_c_span = this->maxmax.fl[0] - this->minmin.fl[0];

  const bs_float scale_speed = ui->spin_zoom_speed->value();
  const bs_float next_r_span =
      prev_r_span * (is_zooming_up ? (1 / scale_speed) : scale_speed);
  const bs_float next_c_span = next_r_span * cols_div_rows;

  cplx_union_d next_center = this->minmin;
  next_center.fl[0] += c_relative_pos * prev_c_span;
  next_center.fl[1] += (1 - r_relative_pos) * prev_r_span;

  this->scale_record.emplace_back(
      std::make_pair(next_center.value, next_r_span));

  this->minmin = next_center;
  this->minmin.fl[0] -= next_c_span / 2;
  this->minmin.fl[1] -= next_r_span / 2;

  this->maxmax = next_center;
  this->maxmax.fl[0] += next_c_span / 2;
  this->maxmax.fl[1] += next_r_span / 2;

  display_range();

  repaint();

  // const cplx_d prev_center = (this->maxmax.value + this->minmin.value) / 2;
}

void zoomer::update_center_and_scale() {
  cplx_union_d next_center;
  bool ok = true;
  const double scale_by_height = ui->show_scale->text().toDouble(&ok);
  if (!ok) {
    return;
  }
  QString str = ui->show_center_hex->text();
  if (!str.startsWith(QStringLiteral("0x"))) {
    return;
  }

  str = str.right(str.length() - 2);

  QByteArray data = QByteArray::fromHex(str.toUtf8());

  memcpy(next_center.bytes, data.data(), sizeof(next_center));

  update_center_and_scale(next_center, scale_by_height);

  this->scale_record.emplace_back(
      std::make_pair(next_center.value, bs_float(scale_by_height)));
}

void zoomer::update_center_and_scale(const cplx_union_d center,
                                     const bs_float scale_r) {
  const bs_float scale_c = scale_r * cols_div_rows;

  this->minmin = center;
  this->minmin.fl[0] -= scale_c / 2;
  this->minmin.fl[1] -= scale_r / 2;
  this->maxmax = center;
  this->maxmax.fl[0] += scale_c / 2;
  this->maxmax.fl[1] += scale_r / 2;

  this->display_range();

  this->repaint();
}

void zoomer::on_btn_revert_clicked() {
  if (this->scale_record.size() > 1) {

    this->scale_record.pop_back();
  }

  if (this->scale_record.empty()) {
    cplx_union_d v;
    v.fl[0] = 0;
    v.fl[1] = 0;
    this->scale_record.emplace_back(std::make_pair(v.value, bs_float(4)));
  }

  this->update_center_and_scale((cplx_union_d &)this->scale_record.back().first,
                                this->scale_record.back().second);
}

void zoomer::on_btn_save_image_clicked() {

  QImage img = ui->image->pixmap().toImage();

  if (img.isNull()) {
    return;
  }

  const QString path =
      QFileDialog::getSaveFileName(this, "Save current preview", "", "*.png");

  if (path.isEmpty()) {
    return;
  }

  img.save(path);
}

void zoomer::on_btn_save_frame_clicked() {
  const QString filename = QFileDialog::getSaveFileName(
      this, "Save current frame", "", "*.bs_frame;;*.gz");

  if (filename.isEmpty()) {
    return;
  }

  if ((!filename.endsWith(QStringLiteral(".gz"))) &&
      (!filename.endsWith(QStringLiteral(".bs_frame")))) {
    return;
  }

  if (filename.endsWith(QStringLiteral(".gz"))) {
    ::write_compressed(this->mat, filename.toLocal8Bit().data());
  } else {
    ::write_uncompressed(this->mat, filename.toLocal8Bit().data());
  }

  return;
}
