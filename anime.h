#ifndef ANIME_H
#define ANIME_H

#include <QString>
#include <QStringList>

class Anime
{
public:
  Anime();
  QString getID() { return id; }
  void addSynonym(QString syn) { synonyms.append(syn); }
  QStringList getSynonyms() { return synonyms; }
  void setID(QString id) { this->id = id; }

private:
  QString id;
  QStringList synonyms;
};

#endif // ANIME_H
