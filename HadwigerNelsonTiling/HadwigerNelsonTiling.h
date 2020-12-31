#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_HadwigerNelsonTiling.h"

class HadwigerNelsonTiling : public QMainWindow
{
   Q_OBJECT

public:
   HadwigerNelsonTiling( QWidget *parent = Q_NULLPTR );

private:
   Ui::HadwigerNelsonTilingClass ui;
};
