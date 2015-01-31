#ifndef SEARCHPANEL_H
#define SEARCHPANEL_H

#include <QDialog>

namespace Ui {
class SearchPanel;
}

class SearchPanel : public QDialog
{
  Q_OBJECT

public:
  explicit SearchPanel(QWidget *parent = 0);
  ~SearchPanel();

private:
  Ui::SearchPanel *ui;
};

#endif // SEARCHPANEL_H
