
#ifndef PASSENGER_H
#define PASSENGER_H

#include <QColor>

class Passenger {
public:
    QColor color;

    Passenger() : color(Qt::black) {}
    Passenger(const QColor& color);
};

#endif // PASSENGER_H
