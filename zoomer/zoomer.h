#ifndef ZOOMER_H
#define ZOOMER_H

#include <QWidget>

#include "burning_ship.h"

constexpr double cols_div_rows = 1920.0 / 1080;

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

  cplx_union_d minmin;
  cplx_union_d maxmax;

  void display_range() const;

  void repaint() const;

public slots:

  void mouse_move(const double r_relative_pos, const double c_relative_pos);

  void update_scale(const double r_relative_pos, const double c_relative_pos,
                    const bool is_zooming_up);
};

#endif // ZOOMER_H
