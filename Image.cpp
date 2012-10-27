#include <iostream>
#include "Image.hpp"
#include "Patch.hpp"

using namespace std;
using namespace cv;


Image::Image(const string& cheminFichier) {
    pixels = imread(cheminFichier, 0); // charge l'image en niveaux de gris
    if (pixels.empty())
        throw string("Image vide");

    tailleX = pixels.cols;
    tailleY = pixels.rows;

    possedeMasque = false;

    tableauOffsetsPixels = new Offset*[tailleY];
    for (int i = 0 ; i < tailleY ; i++) {
        tableauOffsetsPixels[i] = new Offset[tailleX];
    }
}

Image::Image(const string& cheminFichier, const string& cheminMasque) {
    pixels = imread(cheminFichier, 0); // charge l'image en niveaux de gris
    if (pixels.empty())
        throw string("Image vide");

    tailleX = pixels.cols;
    tailleY = pixels.rows;

    masque = imread(cheminMasque, 0);
    if (masque.cols != tailleX || masque.rows != tailleY)
        throw string("Le masque n'est pas de la même taille que l'image");
    possedeMasque = true;

    tableauOffsetsPixels = new Offset*[tailleY];
    for (int i = 0 ; i < tailleY ; i++) {
        tableauOffsetsPixels[i] = new Offset[tailleX];
    }
}


Image::~Image() {
    for (int i = 0 ; i < tailleY ; i++) {
        delete(tableauOffsetsPixels[i]);
    }
    delete(tableauOffsetsPixels);
}

void Image::setMasque(const string& cheminFichier) {
    masque = imread(cheminFichier, 0);
    if (masque.cols != tailleX || masque.rows != tailleY)
        throw string("Le masque n'est pas de la même taille que l'image");
    possedeMasque = true;
}

void Image::affichePixels() const {
    cout << "Taille Image : " << tailleX << "x" << tailleY << endl;
    cout << "Pixels = " << endl << pixels << endl << endl;
    if (possedeMasque)
        cout << "Masque = " << endl << masque << endl << endl;
}

void Image::afficheImage(const std::string& nomFenetre) const {
    imshow(nomFenetre, pixels);
}

void Image::afficheMasque(const std::string& nomFenetre) const {
    imshow(nomFenetre, masque);
}

const Mat& Image::getPixels() const {
    const Mat& ref = pixels;
    return ref;
}

const Mat& Image::getMasque() const {
    const Mat& ref = masque;
    return ref;
}

// Ajoute un offset à l'image s'il n'existe pas,
// sinon incrémente son nombre d'occurences
Offset Image::ajouterOffset(int x, int y) {
    vector<Offset>::iterator i;
    // On parcourt tout le tableau des offsets
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
        if ((*i).augmenteSiEgal(x, y)) // l'offset existe déjà et on vient d'incrémenter son nombre d'occurences
            return *i;
    }

    // L'offset n'existe pas encore
    Offset nouvelOffset(x, y);
    offsets.push_back(nouvelOffset);

    return offsets.back();
}


void Image::calculeOffsets(int taillePatch, int tau) {
    Patch p1(taillePatch, pixels, masque), p2(taillePatch, pixels, masque);
    long diffMin, diff;
    int distCarre;
    int x = 0, y = 0; // composantes de l'offset à trouver pour chaque pixel
    int i, j, k, l;
    offsets.clear();

    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            p1.setPosition(j, i);
            if (!possedeMasque || p1.estValide()) { // le patch p1 est entièrement contenu dans la zone connue de l'image
                diffMin = -1;
                x = 0;
                y = 0;

                for (k = 0 ; k < tailleY - taillePatch + 1 ; k++) {
                    for (l = 0 ; l < tailleX - taillePatch + 1; l++) {
                        p2.setPosition(l, k);
                        if (!possedeMasque || p2.estValide()) {
                            distCarre = (i-k)*(i-k)+(j-l)*(j-l);
                            if (distCarre > tau*tau) {
                                diff = p1.difference(p2);
                                if (diffMin == -1 || diff < diffMin) {
                                    diffMin = diff;
                                    y = k-i;
                                    x = l-j;
                                }
                            }
                        }
                    }
                }
                tableauOffsetsPixels[i][j] = ajouterOffset(x, y);
            }
            else {
                tableauOffsetsPixels[i][j] = Offset();
            }
        }
    }
}

void Image::afficheOffsets() {
    cout << "Offsets uniques de l'image : " << offsets.size() << endl;
    vector<Offset>::iterator i;
    // On parcourt tout le tableau des offsets
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
        i->afficheTriplet();
    }
}

void Image::afficheOffsetsPixels(int taillePatch) {
    cout << "Offsets de l'image par pixel (ne pas tenir compte des 'fois') :" << endl;

    for (int i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (int j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            cout << "Pixel " << i << "," << j << " : ";
            tableauOffsetsPixels[i][j].affiche();
        }
    }
}

int Image::getTaille() const {
    return tailleX;
}

std::vector<Offset> Image::getOffsets() const {
    return offsets;
}



