#ifndef ZOOMER_H
#define ZOOMER_H

#include <QWidget>

#include <burning_ship.h>
#include <renders.h>

#include <mutex>

#include <QKeyEvent>

#include <list>

constexpr double cols_div_rows = double(burning_ship_cols) / burning_ship_rows;

namespace Ui {
class zoomer;
}

class zoomer : public QWidget {
  Q_OBJECT

public:
  explicit zoomer(QWidget *parent = nullptr);
  ~zoomer();

private:
  Ui::zoomer *ui;

  mat_age *const mat;
  norm2_matc1 *const norm2;

  mat_age_f32 *const mat_f32;

  cplx_union_d minmin;
  cplx_union_d maxmax;

  render_by_q_options opt;

  std::mutex lock;

  std::list<std::pair<cplx_d, bs_float>> scale_record;

  void display_range() const;

public slots:

  void repaint();

  void mouse_move(const double r_relative_pos, const double c_relative_pos);

  void update_scale(const double r_relative_pos, const double c_relative_pos,
                    const bool is_zooming_up);

private slots:
  void update_center_and_scale();
  void update_center_and_scale(const cplx_union_d center,
                               const bs_float scale_by_height);

  void on_btn_repaint_clicked() { update_center_and_scale(); }

  void on_btn_revert_clicked();

  void on_btn_save_image_clicked();

  void on_btn_save_frame_clicked();
};

#endif // ZOOMER_H
