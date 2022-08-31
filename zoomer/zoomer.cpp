#include "zoomer.h"
#include "ui_zoomer.h"

zoomer::zoomer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::zoomer)
{
    ui->setupUi(this);
}

zoomer::~zoomer()
{
    delete ui;
}
