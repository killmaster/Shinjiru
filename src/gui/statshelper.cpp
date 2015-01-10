#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::updateStatistics() {
  progress_bar->setFormat("Updating statistics");
  progress_bar->setValue(0);
  QList<Anime*> animes = this->user->getAnimeList();

  progress_bar->setMaximum(animes.count());

  int   episodes_watched = 0;

  int   watching         = 0;
  int   completed        = 0;
  int   on_hold          = 0;
  int   dropped          = 0;
  int   plan_to_watch    = 0;

  QList<int> scores;

  for(Anime *anime: animes) {
    progress_bar->setValue(progress_bar->value() + 1);
    episodes_watched += anime->getMyProgress();

    QString status = anime->getMyStatus();
    QString s_score = anime->getMyScore();

    if(user->scoreType() != 3) {
      int score = 0;
      s_score = s_score.replace(" Star", "");
      s_score = s_score.replace("-", "0");
      score = s_score.toInt();

      if(score > 0) {
        scores.append(score);
      }
    }

    if(status == "watching")
      watching++;
    else if(status == "completed")
      completed++;
    else if(status == "on-hold")
      on_hold++;
    else if(status == "dropped")
      dropped++;
    else if(status == "plan to watch")
      plan_to_watch++;
    else
        qDebug() << status;
  }

  if(scores.count() > 0) {
    //
  }

  int total = watching + completed + on_hold + dropped + plan_to_watch;

  int minutes_watched = user->animeTime();
  int hours_watched = (minutes_watched / 60) % 24;
  int days_watched = (minutes_watched / 60.0) / 24;
  minutes_watched %= 60;


  QString time_watched = QString::number(days_watched)    + " days, " +
                         QString::number(hours_watched)   + " hours, and " +
                         QString::number(minutes_watched) + " minutes.";


  ui->lblWatching       ->setText(QString::number(watching));
  ui->lblOnHold         ->setText(QString::number(on_hold));
  ui->lblCompleted      ->setText(QString::number(completed));
  ui->lblDropped        ->setText(QString::number(dropped));
  ui->lblPlanToWatch    ->setText(QString::number(plan_to_watch));
  ui->lblTotalEntries   ->setText(QString::number(total));
  ui->lblDaysWatched    ->setText(time_watched);
  ui->lblEpisodesWatched->setText(QString::number(episodes_watched));

  progress_bar->setFormat("");
  progress_bar->setValue(0);
}
