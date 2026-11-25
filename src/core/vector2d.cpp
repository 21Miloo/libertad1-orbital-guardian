#include "vector2d.h"
#include <cmath>
#include <stdexcept>


// Operador de Suma
Vector2D Vector2D::operator+(const Vector2D &other) const {

    Vector2D result(x + other.x, y + other.y);
    return result;
}

// Operador de resta
Vector2D Vector2D::operator-(const Vector2D &other) const {

    Vector2D result(x - other.x, y - other.y);
    return result;
}

// Operador de multiplicacion (Alargar el vector)
Vector2D Vector2D::operator*(double scalar) const {

    Vector2D result(x * scalar, y * scalar);
    return result;
}

// Operador de division (acortar el vector)
Vector2D Vector2D::operator/(double scalar) const {

    if (scalar == 0.0){
        throw std::invalid_argument("Vector2D: division por cero");
    }

    Vector2D result(x / scalar, y / scalar);
    return result;
}

// conocer magnitud del vector
double Vector2D::length() const {

   return std::sqrt( (x*x) + (y * y) );

}