#ifndef VECTOR2D_H
#define VECTOR2D_H

struct Vector2D{

    double x;
    double y;

    // Constructor con Lista de inicializacion
    Vector2D(double xValue=0.0, double yValue=0.0): x(xValue), y(yValue){} // Lo que va fuera del parantesis siempre es el atributo y lo que va adentro es el parametro.


    // Sobrecarga de Operadores
    Vector2D operator+(const Vector2D &other) const;

    Vector2D operator-(const Vector2D &other) const;

    Vector2D operator*(double scalar) const;

    Vector2D operator/(double scalar) const;


    // Metodo para magnitud del vector
    double length() const; // √(x² + y²)


};

#endif // VECTOR2D_H
