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

    //offsets.clear();

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

void Image::affichePixels() const {
    cout << "Taille Image : " << tailleX << "x" << tailleY << endl;
    cout << "Pixels = " << endl << pixels << endl << endl;
}

void Image::afficheImage(const std::string& nomFenetre) const {
    imshow(nomFenetre, pixels);
}

const Mat& Image::getPixels() const {
    const Mat& ref = pixels;
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
    Patch p1(taillePatch, pixels), p2(taillePatch, pixels);
    long diffMin, diff;
    int distCarre;
    int x = 0, y = 0; // composantes de l'offset à trouver pour chaque pixel
    int i, j, k, l;
    offsets.clear();

    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            p1.setPosition(j, i);
            diffMin = -1;
            x = 0;
            y = 0;

            for (k = 0 ; k < tailleY - taillePatch + 1 ; k++) {
                for (l = 0 ; l < tailleX - taillePatch + 1; l++) {
                    distCarre = (i-k)*(i-k)+(j-l)*(j-l);
                    if (distCarre > tau*tau) {
                        p2.setPosition(l, k);
                        diff = p1.difference(p2);
                        if (diffMin == -1 || diff < diffMin) {
                            diffMin = diff;
                            y = k-i;
                            x = l-j;
                        }
                    }
                }
            }
            tableauOffsetsPixels[i][j] = ajouterOffset(x, y);
        }
    }
}

void Image::afficheOffsets() {
    cout << "Offsets uniques de l'image : " << offsets.size() << endl;
    vector<Offset>::iterator i;
    // On parcourt tout le tableau des offsets
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
        i->affiche();
    }
}

void Image::afficheOffsetsPixels(int taillePatch) {
    cout << "Offsets de l'image par pixel :" << endl;

    for (int i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (int j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            cout << "Pixel " << i << "," << j << " : ";
            tableauOffsetsPixels[i][j].affiche();
        }
    }
}



