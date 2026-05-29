#include "busstopwindows.h"
#include "ui_busstopwindows.h"

#include <QApplication>
#include <QFont>
#include <QMessageBox>
#include <QResizeEvent>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QRandomGenerator>
#include <QPainter>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QLinearGradient>
#include <QTableWidget>
#include <QHeaderView>

// ═══════════════════════════════════════════════════════════════
//  PASSENGER QUEUE WIDGET
// ═══════════════════════════════════════════════════════════════
PassengerQueueWidget::PassengerQueueWidget(QWidget *parent) : QWidget(parent) {
    setFixedHeight(60);
}

void PassengerQueueWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int radius = 15;
    int spacing = 5;
    for (int i = 0; i < qMin(m_queue.size(), 30); i++) {
        painter.setBrush(m_queue[i].color);
        painter.setPen(QPen(Qt::black, 1));
        painter.drawEllipse(10 + i * (radius * 2 + spacing), 15, radius * 2, radius * 2);
    }
}

// ═══════════════════════════════════════════════════════════════
//  DOCK WIDGET
// ═══════════════════════════════════════════════════════════════
DockWidget::DockWidget(QWidget *parent) : QWidget(parent) {
    setMinimumWidth(80);
    setMaximumWidth(120);
}

void DockWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    int dockW = width() - 10;
    int spacing = 8;
    int dockH = (height() - (m_max + 1) * spacing) / m_max;
    dockH = qMin(50, dockH);
    for (int i = 0; i < m_max; i++) {
        QRect dockRect(10, spacing + i * (dockH + spacing), dockW, dockH);
        painter.setBrush(QColor(200, 200, 200, 100));
        painter.setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter.drawRoundedRect(dockRect, 8, 8);
        if (i < m_docks.size()) {
            const Bus &bus = m_docks[i];
            painter.setBrush(bus.color);
            painter.setPen(QPen(Qt::black, 2));
            painter.drawRoundedRect(dockRect, 8, 8);
            painter.setPen(Qt::white);
            painter.drawText(dockRect, Qt::AlignCenter, QString("%1/%2").arg(bus.passengers).arg(bus.capacity));
        }
    }
}

// ═══════════════════════════════════════════════════════════════
//  GAMEBOARD
// ═══════════════════════════════════════════════════════════════
GameBoard::GameBoard(QWidget *parent) : QWidget(parent) {
    setMouseTracking(true);
}

void GameBoard::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(245, 240, 225));
    m_cellSize = qMin(width(), height()) / m_gridSize;
    int boardWidth = m_gridSize * m_cellSize;
    int boardHeight = m_gridSize * m_cellSize;
    int offsetX = (width() - boardWidth) / 2;
    int offsetY = (height() - boardHeight) / 2;
    painter.setPen(QPen(QColor(220, 215, 200), 1));
    for (int i = 0; i <= m_gridSize; i++) {
        painter.drawLine(offsetX + i * m_cellSize, offsetY, offsetX + i * m_cellSize, offsetY + boardHeight);
        painter.drawLine(offsetX, offsetY + i * m_cellSize, offsetX + boardWidth, offsetY + i * m_cellSize);
    }
    for (const auto &bus : m_buses) {
        QRect rect = getBusRect(bus).translated(offsetX, offsetY);
        drawModernBus(painter, bus, rect);
    }
}

void GameBoard::drawModernBus(QPainter &painter, const Bus &bus, const QRect &rect) {
    painter.setBrush(bus.color);
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRoundedRect(rect, 6, 6);
    painter.setBrush(Qt::white);
    QPolygonF arrow;
    int s = m_cellSize / 6;
    if (bus.horizontal) {
        if (bus.directionForward) arrow << QPointF(rect.right()-3, rect.center().y()) << QPointF(rect.right()-3-s, rect.center().y()-s) << QPointF(rect.right()-3-s, rect.center().y()+s);
        else arrow << QPointF(rect.left()+3, rect.center().y()) << QPointF(rect.left()+3+s, rect.center().y()-s) << QPointF(rect.left()+3+s, rect.center().y()+s);
    } else {
        if (bus.directionForward) arrow << QPointF(rect.center().x(), rect.bottom()-3) << QPointF(rect.center().x()-s, rect.bottom()-3-s) << QPointF(rect.center().x()+s, rect.bottom()-3-s);
        else arrow << QPointF(rect.center().x(), rect.top()+3) << QPointF(rect.center().x()-s, rect.top()+3+s) << QPointF(rect.center().x()+s, rect.top()+3+s);
    }
    painter.drawPolygon(arrow);
    painter.setPen(Qt::white);
    painter.drawText(rect, Qt::AlignCenter, QString::number(bus.capacity));
}

void GameBoard::mousePressEvent(QMouseEvent *event) {
    int boardWidth = m_gridSize * m_cellSize;
    int boardHeight = m_gridSize * m_cellSize;
    int offsetX = (width() - boardWidth) / 2;
    int offsetY = (height() - boardHeight) / 2;
    QPoint pos = event->pos() - QPoint(offsetX, offsetY);
    for (const auto &bus : m_buses) {
        if (getBusRect(bus).contains(pos)) {
            emit busClicked(bus.id);
            break;
        }
    }
}

QRect GameBoard::getBusRect(const Bus &bus) const {
    int x = bus.col * m_cellSize + 2;
    int y = bus.row * m_cellSize + 2;
    int w = (bus.horizontal ? bus.length : 1) * m_cellSize - 4;
    int h = (bus.horizontal ? 1 : bus.length) * m_cellSize - 4;
    return QRect(x, y, w, h);
}

// ═══════════════════════════════════════════════════════════════
//  BUSSTOPWINDOWS — CLASSE PRINCIPAL
// ═══════════════════════════════════════════════════════════════
BusStopWindows::BusStopWindows(QWidget *parent)
    : QWidget(parent), ui(new Ui::BusStopWindows), m_currentScreen(GameScreen::Home), m_gameModel(new GameModel(this)) {
    ui->setupUi(this);
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &BusStopWindows::processBoarding);
    m_gameTimer->start(150);
    buildLevelScreen();
    buildGameScreen();
    buildRankingScreen();
    connect(m_gameModel, &GameModel::busesChanged, this, &BusStopWindows::updateBuses);
    connect(m_gameModel, &GameModel::passengerQueueChanged, this, &BusStopWindows::updatePassengerQueue);
    connect(m_gameModel, &GameModel::dockedBusesChanged, this, &BusStopWindows::updateDockedBuses);
    connect(m_gameModel, &GameModel::scoreChanged, this, &BusStopWindows::updateScore);
    connect(m_gameModel, &GameModel::gameOver, this, &BusStopWindows::onGameOver);
    connect(m_gameModel, &GameModel::levelCompleted, this, &BusStopWindows::onLevelCompleted);
    connect(ui->startButton, &QPushButton::clicked, this, &BusStopWindows::onStartButtonClicked);
    showScreen(GameScreen::Home);
}

BusStopWindows::~BusStopWindows() {
    delete ui;
}

void BusStopWindows::buildLevelScreen() {
    m_levelScreen = new QWidget(this);
    m_levelScreen->hide();
    m_levelScreen->setStyleSheet("background-color: #2c3e50;");
    
    QVBoxLayout *vlay = new QVBoxLayout(m_levelScreen);
    vlay->setContentsMargins(15, 15, 15, 15);
    vlay->setSpacing(8);

    QLabel *title = new QLabel("SELECIONE O NÍVEL", m_levelScreen);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #ecf0f1; font-size: 28px; font-weight: bold; margin-bottom: 5px;");
    
    m_btnLevel1 = new QPushButton("NÍVEL 1 (Fácil)", m_levelScreen);
    m_btnLevel2 = new QPushButton("NÍVEL 2 (Médio)", m_levelScreen);
    m_btnLevel3 = new QPushButton("NÍVEL 3 (Difícil)", m_levelScreen);
    m_btnShowRanking = new QPushButton("VER RANKING", m_levelScreen);
    m_btnBackToHome = new QPushButton("VOLTAR", m_levelScreen);

    // Ajuste dinâmico: botões expandem para ocupar o espaço disponível igualmente
    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_btnLevel1->setSizePolicy(sp);
    m_btnLevel2->setSizePolicy(sp);
    m_btnLevel3->setSizePolicy(sp);
    m_btnShowRanking->setSizePolicy(sp);
    m_btnBackToHome->setSizePolicy(sp);

    QString baseStyle = "QPushButton { color: white; border-radius: 12px; font-size: 18px; font-weight: bold; border: 2px solid rgba(255,255,255,0.2); } QPushButton:hover { border: 2px solid white; }";
    m_btnLevel1->setStyleSheet(baseStyle + "QPushButton { background-color: #27ae60; }");
    m_btnLevel2->setStyleSheet(baseStyle + "QPushButton { background-color: #2980b9; }");
    m_btnLevel3->setStyleSheet(baseStyle + "QPushButton { background-color: #e67e22; }");
    m_btnShowRanking->setStyleSheet(baseStyle + "QPushButton { background-color: #9b59b6; }");
    m_btnBackToHome->setStyleSheet(baseStyle + "QPushButton { background-color: #e74c3c; font-size: 16px; }");

    vlay->addWidget(title);
    vlay->addWidget(m_btnLevel1);
    vlay->addWidget(m_btnLevel2);
    vlay->addWidget(m_btnLevel3);
    vlay->addWidget(m_btnShowRanking);
    vlay->addWidget(m_btnBackToHome);

    connect(m_btnLevel1, &QPushButton::clicked, this, &BusStopWindows::onLevel1Clicked);
    connect(m_btnLevel2, &QPushButton::clicked, this, &BusStopWindows::onLevel2Clicked);
    connect(m_btnLevel3, &QPushButton::clicked, this, &BusStopWindows::onLevel3Clicked);
    connect(m_btnShowRanking, &QPushButton::clicked, this, &BusStopWindows::onShowRankingClicked);
    connect(m_btnBackToHome, &QPushButton::clicked, this, &BusStopWindows::onBackToHomeClicked);
}

void BusStopWindows::buildGameScreen() {
    m_gameScreen = new QWidget(this);
    m_gameScreen->hide();
    QVBoxLayout *mainV = new QVBoxLayout(m_gameScreen);
    mainV->setContentsMargins(5, 5, 5, 5);
    mainV->setSpacing(3);
    m_queueWidget = new PassengerQueueWidget(m_gameScreen);
    mainV->addWidget(m_queueWidget);
    QHBoxLayout *midH = new QHBoxLayout();
    midH->setSpacing(5);
    midH->setContentsMargins(0, 0, 0, 0);
    m_gameBoard = new GameBoard(m_gameScreen);
    midH->addWidget(m_gameBoard, 5); // Aumenta a proporção do tabuleiro
    m_dockWidget = new DockWidget(m_gameScreen);
    midH->addWidget(m_dockWidget, 1); // Diminui a proporção das paragens
    mainV->addLayout(midH, 1);

    // Linha separadora para garantir que as paragens não fiquem coladas na barra
    QFrame *line = new QFrame(m_gameScreen);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("background-color: rgba(255, 255, 255, 0.1);");
    mainV->addWidget(line);

    QHBoxLayout *bottomH = new QHBoxLayout();
    bottomH->setContentsMargins(10, 5, 10, 5);
    bottomH->setSpacing(5);
    
    m_scoreDisplay = new QLabel("0", m_gameScreen);
    m_scoreDisplay->setStyleSheet("font-size: 18px; font-weight: bold; color: #e67e22;");
    m_scoreDisplay->setMaximumHeight(30);
    bottomH->addWidget(m_scoreDisplay);
    
    // Spacer para empurrar os botões para o centro
    bottomH->addStretch(1);
    
    QHBoxLayout *buttonContainer = new QHBoxLayout();
    buttonContainer->setSpacing(8);
    buttonContainer->setContentsMargins(0, 0, 0, 0);
    
    m_btnRestart = new QPushButton("Reiniciar", m_gameScreen);
    m_btnUndo = new QPushButton("Desfazer", m_gameScreen);
    m_btnBack = new QPushButton("Menu", m_gameScreen);
    
    m_btnRestart->setMaximumHeight(30);
    m_btnUndo->setMaximumHeight(30);
    m_btnBack->setMaximumHeight(30);
    
    QString ctrlStyle = "QPushButton { background-color: #34495e; color: white; border-radius: 6px; padding: 6px 12px; font-weight: bold; font-size: 12px; min-width: 70px; }";
    m_btnRestart->setStyleSheet(ctrlStyle); m_btnUndo->setStyleSheet(ctrlStyle); m_btnBack->setStyleSheet(ctrlStyle);
    
    buttonContainer->addWidget(m_btnRestart);
    buttonContainer->addWidget(m_btnUndo);
    buttonContainer->addWidget(m_btnBack);
    
    bottomH->addLayout(buttonContainer);
      // Spacer final para garantir que nada fique na extrema direita (onde estão as paragens)
    bottomH->addStretch(1);

    mainV->addLayout(bottomH, 0);
    connect(m_btnRestart, &QPushButton::clicked, this, &BusStopWindows::onRestartClicked);
    connect(m_btnUndo, &QPushButton::clicked, this, &BusStopWindows::onUndoClicked);
    connect(m_btnBack, &QPushButton::clicked, this, &BusStopWindows::onBackToMenuClicked);
    connect(m_gameBoard, &GameBoard::busClicked, this, &BusStopWindows::onBusClicked);
}

void BusStopWindows::buildRankingScreen() {
    m_rankingScreen = new QWidget(this);
    m_rankingScreen->hide();
    m_rankingScreen->setStyleSheet("background-color: #2c3e50;");
    QVBoxLayout *vlay = new QVBoxLayout(m_rankingScreen);
    vlay->setContentsMargins(10, 10, 10, 10);

    QLabel *title = new QLabel("RANKING", m_rankingScreen);
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #ecf0f1; font-size: 24px; font-weight: bold; margin: 5px;");
    
    QTableWidget *table = new QTableWidget(0, 4, m_rankingScreen);
    table->setHorizontalHeaderLabels({"Nome", "Nív", "Pts", "Jog"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setStyleSheet("QTableWidget { background-color: #34495e; color: white; font-size: 13px; border: none; } QHeaderView::section { background-color: #2c3e50; color: white; font-weight: bold; padding: 2px; }");
    table->setObjectName("rankingTable");
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->verticalHeader()->setVisible(false);

    QHBoxLayout *btnLay = new QHBoxLayout();
    QPushButton *btnBackMenu = new QPushButton("MENU", m_rankingScreen);
    QPushButton *btnBackHome = new QPushButton("INÍCIO", m_rankingScreen);
    QString rnkStyle = "QPushButton { color: white; border-radius: 10px; padding: 8px; font-weight: bold; font-size: 15px; min-height: 40px; }";
    btnBackMenu->setStyleSheet(rnkStyle + "QPushButton { background-color: #3498db; }");
    btnBackHome->setStyleSheet(rnkStyle + "QPushButton { background-color: #e74c3c; }");
    btnLay->addWidget(btnBackHome); btnLay->addWidget(btnBackMenu);

    vlay->addWidget(title);
    vlay->addWidget(table);
    vlay->addLayout(btnLay);
    
    connect(btnBackMenu, &QPushButton::clicked, this, &BusStopWindows::onBackToMenuClicked);
    connect(btnBackHome, &QPushButton::clicked, this, &BusStopWindows::onBackToHomeClicked);
}

void BusStopWindows::showScreen(GameScreen screen) {
    ui->titleLabel->setVisible(false); ui->nameLabel->setVisible(false); ui->nameInput->setVisible(false); ui->startButton->setVisible(false);
    if (m_levelScreen) m_levelScreen->hide(); if (m_gameScreen) m_gameScreen->hide(); if (m_rankingScreen) m_rankingScreen->hide();
    m_currentScreen = screen;
    if (screen == GameScreen::Home) { ui->titleLabel->setVisible(true); ui->nameLabel->setVisible(true); ui->nameInput->setVisible(true); ui->startButton->setVisible(true); }
    else if (screen == GameScreen::LevelSelect) { m_levelScreen->setGeometry(this->rect()); m_levelScreen->show(); }
    else if (screen == GameScreen::Game) { m_gameScreen->setGeometry(this->rect()); m_gameScreen->show(); }
    else if (screen == GameScreen::Ranking) {
        QTableWidget *table = m_rankingScreen->findChild<QTableWidget*>("rankingTable");
        if (table) {
            QJsonArray ranking = m_gameModel->getRanking();
            table->setRowCount(0);
            for (const QJsonValue& val : ranking) {
                QJsonObject obj = val.toObject();
                int row = table->rowCount();
                table->insertRow(row);
                table->setItem(row, 0, new QTableWidgetItem(obj["name"].toString()));
                table->setItem(row, 1, new QTableWidgetItem(QString::number(obj["level"].toInt())));
                table->setItem(row, 2, new QTableWidgetItem(QString::number(obj["score"].toInt())));
                table->setItem(row, 3, new QTableWidgetItem(QString::number(obj["moves"].toInt())));
            }
            table->sortItems(2, Qt::DescendingOrder);
        }
        m_rankingScreen->setGeometry(this->rect()); m_rankingScreen->show();
    }
}

void BusStopWindows::onStartButtonClicked() {
    QString name = ui->nameInput->text().trimmed();
    if (name.isEmpty()) { QMessageBox::warning(this, "Aviso", "Por favor, insira seu nome!"); return; }
    showScreen(GameScreen::LevelSelect);
}

void BusStopWindows::onLevel1Clicked() { startLevel(1); }
void BusStopWindows::onLevel2Clicked() { startLevel(2); }
void BusStopWindows::onLevel3Clicked() { startLevel(3); }

void BusStopWindows::startLevel(int level) {
    m_gameModel->startLevel(level, ui->nameInput->text().trimmed());
    m_gameBoard->setGridSize(m_gameModel->getGridSize());
    m_dockWidget->setDocks(m_gameModel->getDockedBuses(), m_gameModel->getMaxDocks());
    showScreen(GameScreen::Game);
}

void BusStopWindows::onRestartClicked() {
    m_gameModel->startLevel(m_gameModel->getCurrentLevel(), m_gameModel->getUserName());
    m_dockWidget->setDocks(m_gameModel->getDockedBuses(), m_gameModel->getMaxDocks());
}

void BusStopWindows::onUndoClicked() { m_gameModel->undoLastMove(); }
void BusStopWindows::onBackToMenuClicked() { showScreen(GameScreen::LevelSelect); }
void BusStopWindows::onShowRankingClicked() { showScreen(GameScreen::Ranking); }
void BusStopWindows::onBackToHomeClicked() { ui->nameInput->clear(); showScreen(GameScreen::Home); }
void BusStopWindows::onBusClicked(int busId) { m_gameModel->tryBusExit(busId); }
void BusStopWindows::processBoarding() { m_gameModel->processBoarding(); }
void BusStopWindows::updateBuses(const QVector<Bus>& buses) { m_gameBoard->setBuses(buses); }
void BusStopWindows::updatePassengerQueue(const QVector<Passenger>& queue) { m_queueWidget->setQueue(queue); }
void BusStopWindows::updateDockedBuses(const QVector<Bus>& dockedBuses) { m_dockWidget->setDocks(dockedBuses, m_gameModel->getMaxDocks()); }
void BusStopWindows::updateScore(int score) { m_scoreDisplay->setText(QString::number(score)); }
void BusStopWindows::onGameOver(const QString& message) { QMessageBox::critical(this, "Game Over", message); showScreen(GameScreen::LevelSelect); }
void BusStopWindows::onLevelCompleted(const QString& message) { QMessageBox::information(this, "Vitória", message); showScreen(GameScreen::LevelSelect); }

void BusStopWindows::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_levelScreen) m_levelScreen->setGeometry(this->rect());
    if (m_gameScreen) m_gameScreen->setGeometry(this->rect());
    if (m_rankingScreen) m_rankingScreen->setGeometry(this->rect());
}
