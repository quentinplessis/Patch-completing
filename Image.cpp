#include <iostream>
#include <time.h>
#include <algorithm>
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

bool comparaisonOffsets(Offset o1, Offset o2) {
    return o1.getN() > o2.getN();
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
    long diffMin, diff;
    int distCarre;
    int x = 0, y = 0; // composantes de l'offset à trouver pour chaque pixel
    int i, j, k, l, m, n;
    offsets.clear();

    uchar** p = new uchar*[taillePatch];    // contiendra les lignes du patch1
    uchar** pp = new uchar*[taillePatch];   // contiendra les lignes du masque du patch1
    uchar** p2 = new uchar*[taillePatch];
    uchar** pp2 = new uchar*[taillePatch];
    bool patch1Valide = true, patch2Valide = true;

    clock_t beginTime = clock();
    clock_t endTime;

    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        // On calcule les lignes du patch
        for (k = 0 ; k < taillePatch ; k++) {
            p[k] = pixels.ptr<uchar>(i+k);
            pp[k] = masque.ptr<uchar>(i+k);
        }
        for (j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            // (j, i) sont les coordonnées du patch 1
            patch1Valide = true;
            for (k = 0 ; k < taillePatch ; k++) {
                for (l = 0 ; l < taillePatch ; l++) {
                    if (pp[k][j+l] < 126) { // si un pixel du patch 1 est dans le masque
                        patch1Valide = false;
                        break;
                    }
                    if (!patch1Valide)
                        break;
                }
            }
            if (!patch1Valide) {
                tableauOffsetsPixels[i][j] = Offset();
                continue;
            }

            diffMin = -1;
            x = 0;
            y = 0;

            for (k = 0 ; k < tailleY - taillePatch + 1 ; k++) {
                for (m = 0 ; m < taillePatch ; m++) {
                    p2[m] = pixels.ptr<uchar>(k+m);
                    pp2[m] = masque.ptr<uchar>(k+m);
                }
                for (l = 0 ; l < tailleX - taillePatch + 1; l++) {
                    // (l, k) sont les coordonnées du patch 2
                    distCarre = (i-k)*(i-k)+(j-l)*(j-l);
                    if (distCarre > tau*tau) {
                        patch2Valide = true;
                        diff = 0;
                        for (m = 0 ; m < taillePatch ; m++) {
                            for (n = 0 ; n < taillePatch ; n++) {
                                if (pp2[m][l+n] < 126) {
                                    patch2Valide = false;
                                    break;
                                }
                                else
                                    diff += (p[m][j+n] - p2[m][l+n]) * (p[m][j+n] - p2[m][l+n]);
                            }
                            if (!patch2Valide)
                                break;
                        }
                        if (!patch2Valide)
                            continue;
                        if (diffMin == -1 || diff < diffMin) {
                            diffMin = diff;
                            y = k-i;
                            x = l-j;
                        }
                    }
                }
            }

            // (x,y) coordonnées de l'offset
            ajouterOffset(x, y);
            //tableauOffsetsPixels[i][j] = ajouterOffset(x, y);
        }
        cout << "Ligne " << i << endl;
    }
    endTime = clock();
    cout << "Time: " << (endTime - beginTime) / CLOCKS_PER_SEC << endl;

    delete p;
    delete pp;
    delete p2;
    delete pp2;
}

void Image::selectionneOffsets(int K) {
    cout << "Nombre d'offsets au total : " << offsets.size() << endl;
    cout << "On conserve les " << K << " plus importants" << endl;
    std::sort(offsets.begin(), offsets.end(), comparaisonOffsets);
    offsets.resize(K);
}

void Image::calcule2(int taillePatch, int tau) {
    Patch p1(taillePatch, pixels, masque), p2(taillePatch, pixels, masque);
    long diffMin, diff;
    int distCarre;
    int x = 0, y = 0; // composantes de l'offset à trouver pour chaque pixel
    int i, j, k, l, m, n;
    offsets.clear();

    clock_t beginTime;
    clock_t endTime;

    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        beginTime = clock();
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
        endTime = clock();
        cout << "Ligne " << i << " : " << (endTime - beginTime) / CLOCKS_PER_SEC << endl;
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
            //cout << "Pixel " << i << "," << j << " : ";
            tableauOffsetsPixels[i][j].affiche();
            cout << ", ";
        }
        cout << endl;
    }
}

int Image::getTaille() const {
    return tailleX;
}

std::vector<Offset> Image::getOffsets() const {
    return offsets;
}



