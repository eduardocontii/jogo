
#include "GameModel.h"
#include <QRandomGenerator>
#include <QDebug>

GameModel::GameModel(QObject *parent) : QObject(parent),
    m_currentLevel(1),
    m_gridSize(6),
    m_score(0),
    m_moves(0),
    m_maxDocks(5)
{
}

void GameModel::startLevel(int level, const QString& userName) {
    m_currentLevel = level;
    m_userName = userName;
    m_gridSize = (level == 1) ? 6 : (level == 2) ? 8 : 10;
    m_maxDocks = (level == 1) ? 5 : 6;
    m_score = 0;
    m_moves = 0;
    initializeLevel();
    emit scoreChanged(m_score);
}

void GameModel::initializeLevel() {
    m_buses.clear();
    m_passengerQueue.clear();
    m_dockedBuses.clear();
    m_undoStack.clear();

    if (m_currentLevel == 1) {
        m_buses.append(Bus(1, 4, 0, Qt::red, 0, 0, true, 2, true));
        m_buses.append(Bus(2, 6, 0, Qt::blue, 1, 0, true, 3, true));
        m_buses.append(Bus(3, 4, 0, Qt::green, 2, 0, true, 2, true));
        m_buses.append(Bus(4, 4, 0, Qt::yellow, 4, 0, true, 2, true));
        m_buses.append(Bus(5, 4, 0, Qt::magenta, 0, 4, false, 2, true));
    } else if (m_currentLevel == 2) {
        m_buses.append(Bus(1, 4, 0, Qt::red, 0, 0, true, 2, true));
        m_buses.append(Bus(2, 6, 0, Qt::blue, 1, 0, true, 3, true));
        m_buses.append(Bus(3, 8, 0, Qt::green, 2, 0, true, 4, true));
        m_buses.append(Bus(4, 4, 0, Qt::yellow, 4, 0, true, 2, true));
        m_buses.append(Bus(5, 6, 0, Qt::magenta, 5, 0, true, 3, true));
        m_buses.append(Bus(6, 4, 0, Qt::cyan, 0, 5, false, 2, true));
        m_buses.append(Bus(7, 6, 0, Qt::red, 0, 6, false, 3, true));
        m_buses.append(Bus(8, 8, 0, Qt::blue, 0, 7, false, 4, true));
    } else {
        m_buses.append(Bus(1, 4, 0, Qt::red, 0, 0, false, 2, false));
        m_buses.append(Bus(2, 6, 0, Qt::blue, 0, 2, false, 3, false));
        m_buses.append(Bus(3, 8, 0, Qt::green, 0, 4, false, 4, false));
        m_buses.append(Bus(4, 4, 0, Qt::yellow, 0, 6, false, 2, false));
        m_buses.append(Bus(5, 6, 0, Qt::magenta, 0, 8, false, 3, false));
        m_buses.append(Bus(6, 4, 0, Qt::cyan, 8, 1, false, 2, true));
        m_buses.append(Bus(7, 6, 0, Qt::red, 7, 3, false, 3, true));
        m_buses.append(Bus(8, 8, 0, Qt::blue, 6, 5, false, 4, true));
        m_buses.append(Bus(9, 4, 0, Qt::green, 8, 7, false, 2, true));
        m_buses.append(Bus(10, 6, 0, Qt::yellow, 7, 9, false, 3, true));
        m_buses.append(Bus(11, 4, 0, Qt::magenta, 2, 0, true, 2, false));
        m_buses.append(Bus(12, 6, 0, Qt::cyan, 4, 0, true, 3, false));
        m_buses.append(Bus(13, 8, 0, Qt::red, 6, 0, true, 4, false));
        m_buses.append(Bus(14, 4, 0, Qt::blue, 5, 2, true, 2, false));
        m_buses.append(Bus(15, 6, 0, Qt::green, 3, 1, true, 3, false));
        m_buses.append(Bus(16, 4, 0, Qt::yellow, 4, 8, true, 2, true));
        m_buses.append(Bus(17, 6, 0, Qt::magenta, 5, 7, true, 3, true));
        m_buses.append(Bus(18, 8, 0, Qt::cyan, 6, 6, true, 4, true));
        m_buses.append(Bus(19, 4, 0, Qt::red, 3, 8, true, 2, true));
        m_buses.append(Bus(20, 6, 0, Qt::blue, 4, 4, true, 3, true));
    }

    QVector<Passenger> allPassengers;
    for (const auto &bus : m_buses) {
        for (int i = 0; i < bus.capacity; i++) {
            allPassengers.append(Passenger(bus.color));
        }
    }
    for (int i = 0; i < allPassengers.size(); i++) {
        int j = QRandomGenerator::global()->bounded(allPassengers.size());
        allPassengers.swapItemsAt(i, j);
    }
    m_passengerQueue = allPassengers;

    saveState();
    emit busesChanged(m_buses);
    emit passengerQueueChanged(m_passengerQueue);
    emit dockedBusesChanged(m_dockedBuses);
}

void GameModel::saveState() {
    GameState s; 
    s.buses = m_buses; 
    s.queue = m_passengerQueue; 
    s.dockedBuses = m_dockedBuses; 
    s.score = m_score; 
    s.moves = m_moves;
    m_undoStack.push(s);
}

void GameModel::undoLastMove() {
    if (m_undoStack.size() > 1) {
        m_undoStack.pop(); 
        GameState s = m_undoStack.top();
        m_buses = s.buses; 
        m_passengerQueue = s.queue; 
        m_dockedBuses = s.dockedBuses; 
        m_score = s.score;
        m_moves = s.moves;
        emit busesChanged(m_buses);
        emit passengerQueueChanged(m_passengerQueue);
        emit dockedBusesChanged(m_dockedBuses);
        emit scoreChanged(m_score);
    }
}

void GameModel::tryBusExit(int busId) {
    for (int i = 0; i < m_buses.size(); i++) {
        if (m_buses[i].id == busId) {
            const Bus &bus = m_buses[i];
            int dr = 0, dc = 0;
            if (bus.horizontal) dc = bus.directionForward ? 1 : -1;
            else dr = bus.directionForward ? 1 : -1;

            bool canExit = true;
            int r = bus.row, c = bus.col;
            while (true) {
                r += dr; c += dc;
                if (r < 0 || r >= m_gridSize || c < 0 || c >= m_gridSize) break;
                for (int j = 0; j < m_buses.size(); j++) {
                    if (i == j) continue;
                    const Bus &other = m_buses[j];
                    int rStart = other.row;
                    int rEnd = other.row + (other.horizontal ? 0 : other.length - 1);
                    int cStart = other.col;
                    int cEnd = other.col + (other.horizontal ? other.length - 1 : 0);
                    if (r >= rStart && r <= rEnd && c >= cStart && c <= cEnd) {
                        canExit = false;
                        break;
                    }
                }
                if (!canExit) break;
            }

            if (canExit) {
                m_moves++; // Count move
                Bus exitedBus = bus;
                m_buses.removeAt(i);
                busExited(exitedBus);
                emit busesChanged(m_buses);
            }
            break;
        }
    }
}

void GameModel::busExited(const Bus &bus) {
    if (m_dockedBuses.size() < m_maxDocks) {
        Bus b = bus; 
        b.passengers = 0;
        m_dockedBuses.append(b);
        saveState();
        emit dockedBusesChanged(m_dockedBuses);
    } else {
        emit gameOver("Vagas lotadas!");
    }
}

void GameModel::processBoarding() {
    if (m_dockedBuses.isEmpty() || m_passengerQueue.isEmpty()) return;
    bool changed = false;
    for (int i = 0; i < m_dockedBuses.size(); i++) {
        Bus &bus = m_dockedBuses[i];
        if (bus.passengers < bus.capacity && m_passengerQueue.first().color == bus.color) {
            bus.passengers++;
            m_passengerQueue.removeFirst();
            m_score += 10;
            if (bus.passengers == bus.capacity) {
                m_dockedBuses.removeAt(i);
                i--;
            }
            changed = true; 
            break;
        }
    }
    if (changed) {
        saveState();
        emit passengerQueueChanged(m_passengerQueue);
        emit dockedBusesChanged(m_dockedBuses);
        emit scoreChanged(m_score);
        if (m_passengerQueue.isEmpty() && m_buses.isEmpty() && m_dockedBuses.isEmpty()) {
            // Efficiency bonus: Base score + (1000 / moves)
            int bonus = (m_moves > 0) ? (1000 / m_moves) : 1000;
            m_score += bonus;
            saveToRanking();
            emit levelCompleted(QString("Nível Concluído!\nSua pontuação: %1\nMovimentos: %2").arg(m_score).arg(m_moves));
        }
    }
}

bool GameModel::isNameAvailable(const QString& name) const {
    QJsonArray ranking;
    loadRankingFromFile(ranking);
    for (const QJsonValue& val : ranking) {
        if (val.toObject()["name"].toString() == name) return false;
    }
    return true;
}

void GameModel::saveToRanking() {
    QJsonArray ranking;
    loadRankingFromFile(ranking);
    
    bool updated = false;
    for (int i = 0; i < ranking.size(); ++i) {
        QJsonObject obj = ranking[i].toObject();
        // Se o mesmo jogador já tem recorde para ESTE nível
        if (obj["name"].toString() == m_userName && obj["level"].toInt() == m_currentLevel) {
            // Só atualiza se a nova pontuação for maior
            if (m_score > obj["score"].toInt()) {
                obj["score"] = m_score;
                obj["moves"] = m_moves;
                ranking[i] = obj;
            }
            updated = true;
            break;
        }
    }
    
    // Se é a primeira vez desse jogador neste nível, adiciona nova entrada
    if (!updated) {
        QJsonObject entry;
        entry["name"] = m_userName;
        entry["score"] = m_score;
        entry["moves"] = m_moves;
        entry["level"] = m_currentLevel;
        ranking.append(entry);
    }
    
    saveRankingToFile(ranking);
}

QJsonArray GameModel::getRanking() const {
    QJsonArray ranking;
    loadRankingFromFile(ranking);
    return ranking;
}

void GameModel::loadRankingFromFile(QJsonArray& ranking) const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");
    
    QFile file(path + "/ranking.json");
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        ranking = doc.array();
        file.close();
    }
}

void GameModel::saveRankingToFile(const QJsonArray& ranking) {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists()) dir.mkpath(".");
    
    QFile file(path + "/ranking.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(ranking);
        file.write(doc.toJson());
        file.close();
    }
}
