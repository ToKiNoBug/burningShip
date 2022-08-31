#ifndef ZOOMER_H
#define ZOOMER_H

#include <QWidget>

#include "burning_ship.h"

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
};

#endif // ZOOMER_H
