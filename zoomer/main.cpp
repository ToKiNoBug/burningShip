#include "zoomer.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <omp.h>
#include <thread>

int main(int argc, char *argv[]) {

  if (!::check_sizes()) {
    return 1;
  }

  QApplication a(argc, argv);

  omp_set_num_threads(std::thread::hardware_concurrency());

  /*
  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
      const QString baseName = "MapViewer_" + QLocale(locale).name();
      if (translator.load(":/i18n/" + baseName)) {
          a.installTranslator(&translator);
          break;
      }
  }
  */
  zoomer w;
  w.show();

  w.repaint();

  return a.exec();
}
