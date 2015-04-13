/* Copyright 2015 Kazakuri */

#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include <QPixmap>

namespace Ui {
class About;
}

class About : public QDialog
{
    Q_OBJECT

public:
    explicit About(QWidget *parent = 0);
    ~About();

protected:
    void paintEvent(QPaintEvent *);

private:
    Ui::About *ui;
    QPixmap mascot;
};

#endif // ABOUT_H
