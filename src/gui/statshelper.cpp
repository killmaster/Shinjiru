/* Copyright 2015 Kazakuri */

#include "./mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::updateStatistics() {
  progress_bar->setFormat(tr("Updating statistics"));
  progress_bar->reset();
  QList<Anime*> animes = User::sharedUser()->getAnimeList();

  progress_bar->setMaximum(animes.count());

  int    episodes_watched = 0;

  int    watching         = 0;
  int    completed        = 0;
  int    on_hold          = 0;
  int    dropped          = 0;
  int    plan_to_watch    = 0;

  double mean             = 0;
  double median           = 0;
  double mode             = 0;
  double stddev           = 0.0;
  double sum              = 0.0;

  QList<double> scores;

  for (Anime *anime : animes) {
    progress_bar->setValue(progress_bar->value() + 1);
    if (anime->getMyRewatch() > 0) {
      episodes_watched += anime->getMyProgress() * anime->getMyRewatch();
    }
    episodes_watched += anime->getMyProgress();

    QString status = anime->getMyStatus();
    QString s_score = anime->getMyScore();

    if (User::sharedUser()->scoreType() != 4) {
      int score = 0;
      s_score = s_score.replace(" Star", "");
      s_score = s_score.replace("-", "0");
      s_score = s_score.replace(":(", "33");
      s_score = s_score.replace(":|", "66");
      s_score = s_score.replace(":)", "99");
      score = s_score.toInt();

      if (score > 0) {
        scores.append(score);
        sum += score;
      }
    } else {
      double score = 0.0;
      score = s_score.toDouble();

      if (score > 0.0) {
        scores.append(score);
        sum += score;
      }
    }

    if (status == "watching")
      watching++;
    else if (status == "completed")
      completed++;
    else if (status == "on-hold")
      on_hold++;
    else if (status == "dropped")
      dropped++;
    else if (status == "plan to watch")
      plan_to_watch++;
  }

  if (scores.count() > 0) {
    mean = sum / static_cast<double>(scores.count());
    median = scores.at(scores.count() / 2);
    qSort(scores);

    int number = scores.at(0);
    mode = number;
    int count = 1;
    int countMode = 1;

    for (int i = 1; i < scores.count(); i++) {
      stddev += scores.at(i) - mean;
      if (scores.at(i) == number) {
        count++;
      } else {
        if (count > countMode) {
          countMode = count;
          mode = number;
        }
        count = 1;
        number = scores.at(i);
      }
    }
  }

  stddev = qSqrt(stddev / (scores.count() - 1));

  int total = watching + completed + on_hold + dropped + plan_to_watch;

  int minutes_watched = User::sharedUser()->animeTime();
  int hours_watched = (minutes_watched / 60) % 24;
  int days_watched = (minutes_watched / 60.0) / 24;
  minutes_watched %= 60;

  QString time_watched = tr("%1 days, %2 hours, and %3 minutes")
                         .arg(days_watched).arg(hours_watched)
                         .arg(minutes_watched);


  ui->lblWatching       ->setText(QString::number(watching));
  ui->lblOnHold         ->setText(QString::number(on_hold));
  ui->lblCompleted      ->setText(QString::number(completed));
  ui->lblDropped        ->setText(QString::number(dropped));
  ui->lblPlanToWatch    ->setText(QString::number(plan_to_watch));
  ui->lblTotalEntries   ->setText(QString::number(total));
  ui->lblDaysWatched    ->setText(time_watched);
  ui->lblEpisodesWatched->setText(QString::number(episodes_watched));
  ui->lblMean           ->setText(QString::number(mean, 'g', 2));
  ui->lblMedian         ->setText(QString::number(median));
  ui->lblMode           ->setText(QString::number(mode));
  ui->lblScoreDeviation ->setText(QString::number(stddev, 'g', 2));

  progress_bar->setFormat("");
  progress_bar->reset();
  progress_bar->setMaximum(100);

  reloadSmartTitles();
}
