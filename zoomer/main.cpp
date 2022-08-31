#include "zoomer.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {

  if (!::check_sizes()) {
    return 1;
  }

  QApplication a(argc, argv);

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
