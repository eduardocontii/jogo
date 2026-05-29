#ifndef BUSSTOPWINDOWS_H
#define BUSSTOPWINDOWS_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QTimer>
#include <QResizeEvent>

#include "Bus.h"
#include "Passenger.h"
#include "GameModel.h"

enum class GameScreen { Home, LevelSelect, Game, Ranking };

QT_BEGIN_NAMESPACE
namespace Ui { class BusStopWindows; }
QT_END_NAMESPACE

class PassengerQueueWidget : public QWidget {
    Q_OBJECT
public:
    explicit PassengerQueueWidget(QWidget *parent = nullptr);
    void setQueue(const QVector<Passenger> &queue) { m_queue = queue; update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<Passenger> m_queue;
};

class DockWidget : public QWidget {
    Q_OBJECT
public:
    explicit DockWidget(QWidget *parent = nullptr);
    void setDocks(const QVector<Bus> &docks, int max) { m_docks = docks; m_max = max; update(); }
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    QVector<Bus> m_docks;
    int m_max = 5;
};

class GameBoard : public QWidget {
    Q_OBJECT
public:
    explicit GameBoard(QWidget *parent = nullptr);
    void setGridSize(int size) { m_gridSize = size; update(); }
    void setBuses(const QVector<Bus> &buses) { m_buses = buses; update(); }
    QVector<Bus> getBuses() const { return m_buses; }
signals:
    void busClicked(int busId);
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
private:
    int m_gridSize = 6;
    QVector<Bus> m_buses;
    int m_cellSize = 60;
    QRect getBusRect(const Bus &bus) const;
    void drawModernBus(QPainter &painter, const Bus &bus, const QRect &rect);
};

class BusStopWindows : public QWidget {
    Q_OBJECT
public:
    explicit BusStopWindows(QWidget *parent = nullptr);
    ~BusStopWindows() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    // UI Controls
    void onStartButtonClicked();
    void onLevel1Clicked();
    void onLevel2Clicked();
    void onLevel3Clicked();
    void onRestartClicked();
    void onUndoClicked();
    void onBackToMenuClicked();
    void onShowRankingClicked();
    void onBackToHomeClicked();

    // Game Logic
    void onBusClicked(int busId);
    void processBoarding();

    // Model Updates
    void updateBuses(const QVector<Bus>& buses);
    void updatePassengerQueue(const QVector<Passenger>& queue);
    void updateDockedBuses(const QVector<Bus>& dockedBuses);
    void updateScore(int score);
    void onGameOver(const QString& message);
    void onLevelCompleted(const QString& message);

private:
    Ui::BusStopWindows *ui;
    GameScreen m_currentScreen;
    GameModel *m_gameModel;

    // Screens
    QWidget *m_levelScreen = nullptr;
    QWidget *m_gameScreen = nullptr;
    QWidget *m_rankingScreen = nullptr;

    // Game Widgets
    GameBoard *m_gameBoard = nullptr;
    PassengerQueueWidget *m_queueWidget = nullptr;
    DockWidget *m_dockWidget = nullptr;
    QLabel *m_scoreDisplay = nullptr;

    // Buttons
    QPushButton *m_btnRestart = nullptr, *m_btnUndo = nullptr, *m_btnBack = nullptr;
    QPushButton *m_btnLevel1 = nullptr, *m_btnLevel2 = nullptr, *m_btnLevel3 = nullptr;
    QPushButton *m_btnShowRanking = nullptr;
    QPushButton *m_btnBackToHome = nullptr;

    QTimer *m_gameTimer;

    void buildLevelScreen();
    void buildGameScreen();
    void buildRankingScreen();
    void showScreen(GameScreen screen);
    void startLevel(int level);
};

#endif // BUSSTOPWINDOWS_H
