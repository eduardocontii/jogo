
#include "Bus.h"

Bus::Bus(int id, int capacity, int passengers, const QColor& color, int row, int col, bool horizontal, int length, bool directionForward)
    : id(id), capacity(capacity), passengers(passengers), color(color), row(row), col(col), horizontal(horizontal), length(length), directionForward(directionForward) {}
