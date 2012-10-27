#include <iostream>
#include "Patch.hpp"

using namespace std;
using namespace cv;

Patch::Patch(int posX, int posY, int tailleX) {
    this->posX = posX;
    this->posY = posY;
    this->tailleX = tailleX;
    this->tailleY = tailleX;
}

Patch::Patch(int posX, int posY, int tailleX, int tailleY) {
    this->posX = posX;
    this->posY = posY;
    this->tailleX = tailleX;
    this->tailleY = tailleY;
}

Patch::Patch(int taille, const Mat& pixels, const Mat& masque) {
    posX = 0;
    posY = 0;
    this->tailleX = taille;
    this->tailleY = taille;
    this->pixels = pixels;
    this->masque = masque;
}

void Patch::setPosition(int posX, int posY) {
    this->posX = posX;
    this->posY = posY;
}

// Affiche les pixels de l'image contenus dans ce patch
void Patch::affichePixels() const {
    const uchar* p;
    cout << "Patch " << tailleX << "x" << tailleY << " (" << posX << "," << posY << ") = " << endl;

    for (int i = 0 ; i < tailleY ; i++) {
        p = pixels.ptr<uchar>(posY + i); // on récupère la ligne posY+i de l'image

        for (int j = 0 ; j < tailleX ; j++) {
            cout << (int) p[posX + j] << " , ";
        }
        cout << endl;
    }
    cout << endl;
}

int Patch::getTailleX() const {
    return tailleX;
}

int Patch::getTailleY() const {
    return tailleY;
}

int Patch::getPosX() const {
    return posX;
}

int Patch::getPosY() const {
    return posY;
}

// Calcule la distance (au carré) entre les deux patchs pour la norme 2,
// pour des patchs de même taille (et on suppose qu'ils sont sur la même image)
long Patch::difference(const Patch patch) const {
    if (tailleX != patch.getTailleX() || tailleY != patch.getTailleY())
        throw string("Les patchs n'ont pas la même taille");

    long res = 0;
    const uchar *p1, *p2;
    int pixel1, pixel2;

    for (int i = 0 ; i < tailleY ; i++) {
        p1 = pixels.ptr<uchar>(posY + i);
        p2 = pixels.ptr<uchar>(patch.getPosY() + i);

        for (int j = 0 ; j < tailleX ; j++) {
            pixel1 = (int) p1[posX + j];
            pixel2 = (int) p2[patch.getPosX() + j];
            res += (pixel1 - pixel2) * (pixel1 - pixel2);
        }
    }

    return res;
}

// Renvoie true ssi le pixel (x,y) est dans la zone connue de l'image
bool Patch::estConnu(int x, int y) {
    return ((int) masque.ptr<uchar>(y)[x]) > 126;
}

bool Patch::estValide() {
    for (int i = 0 ; i < tailleY ; i++) {
        for (int j = 0 ; j < tailleX ; j++) {
            if (!estConnu(posY + j, posX + i))
                return false;
        }
    }
    return true;
}




