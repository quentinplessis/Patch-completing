#include <iostream>
#include "Offset.hpp"

using namespace std;

Offset::Offset() {
    x = 0;
    y = 0;
    nombre = 0;
}

Offset::Offset(int x, int y) {
    this->x = x;
    this->y = y;
    nombre = 1;
}

// Si l'offset actuel a pour composantes (x,y),
// incrémente son nombre d'occurences et renvoie true
// sinon renvoie false
bool Offset::augmenteSiEgal(int x, int y) {
    if (this->x == x && this->y == y) {
        nombre++;
        return true;
    }
    return false;
}

void Offset::affiche() const {
    cout << "(" << x << "," << y << ") : " << nombre << " fois" << endl;
}

void Offset::afficheTriplet() const {
    cout << "" << x << "\t" << y << "\t" << nombre << "" << endl;
}

int Offset::getX() const {
    return x;
}

int Offset::getY() const {
    return y;
}

int Offset::getN() const {
    return nombre;
}

