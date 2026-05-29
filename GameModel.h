
#ifndef GAMEMODEL_H
#define GAMEMODEL_H

#include <QObject>
#include <QVector>
#include <QStack>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include "Bus.h"
#include "Passenger.h"

struct GameState {
    QVector<Bus> buses;
    QVector<Passenger> queue;
    QVector<Bus> dockedBuses;
    int score;
    int moves;
};

struct RankingEntry {
    QString name;
    int score;
    int moves;
    int level;
};

class GameModel : public QObject {
    Q_OBJECT
public:
    explicit GameModel(QObject *parent = nullptr);

    void startLevel(int level, const QString& userName);
    void processBoarding();
    void undoLastMove();
    void tryBusExit(int busId);
    void busExited(const Bus &bus);

    int getGridSize() const { return m_gridSize; }
    int getCurrentLevel() const { return m_currentLevel; }
    const QVector<Bus>& getBuses() const { return m_buses; }
    const QVector<Passenger>& getPassengerQueue() const { return m_passengerQueue; }
    const QVector<Bus>& getDockedBuses() const { return m_dockedBuses; }
    int getScore() const { return m_score; }
    int getMaxDocks() const { return m_maxDocks; }
    int getMoves() const { return m_moves; }
    QString getUserName() const { return m_userName; }

    bool isNameAvailable(const QString& name) const;
    void saveToRanking();
    QJsonArray getRanking() const;

signals:
    void busesChanged(const QVector<Bus>& buses);
    void passengerQueueChanged(const QVector<Passenger>& queue);
    void dockedBusesChanged(const QVector<Bus>& dockedBuses);
    void scoreChanged(int score);
    void gameOver(const QString& message);
    void levelCompleted(const QString& message);

private:
    int m_currentLevel;
    int m_gridSize;
    int m_score;
    int m_moves;
    QString m_userName;
    QVector<Bus> m_buses;
    QVector<Passenger> m_passengerQueue;
    QVector<Bus> m_dockedBuses;
    int m_maxDocks;
    QStack<GameState> m_undoStack;

    void initializeLevel();
    void saveState();
    void loadRankingFromFile(QJsonArray& ranking) const;
    void saveRankingToFile(const QJsonArray& ranking);
};

#endif // GAMEMODEL_H
