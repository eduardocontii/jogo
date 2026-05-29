
#ifndef BUS_H
#define BUS_H

#include <QColor>

class Bus {
public:
    int id;
    int capacity;
    int passengers;
    QColor color;
    int row, col;
    bool horizontal;
    int length;
    bool directionForward;

    Bus() : id(0), capacity(0), passengers(0), color(Qt::black), row(0), col(0), horizontal(true), length(0), directionForward(true) {}
    Bus(int id, int capacity, int passengers, const QColor& color, int row, int col, bool horizontal, int length, bool directionForward);
};

#endif // BUS_H
