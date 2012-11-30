#include <iostream>
#include "ImageCouleur.hpp"

using namespace std;
using namespace cv;

ImageCouleur::ImageCouleur(const string& cheminFichier, const string& cheminMasque) {
    pixels = imread(cheminFichier, CV_LOAD_IMAGE_COLOR); // charge l'image en RGB
    if (pixels.empty())
        throw string("Image vide");

    image = new Image(cheminFichier, cheminMasque);
    masque = image->getMasque();
    tailleX = pixels.cols;
    tailleY = pixels.rows;
}

void ImageCouleur::complete(int taillePatch, int tau) {
        // Calcul des offsets
        //image->calculeOffsets(taillePatch, tau);
        image->calculeOffsetsKDTrees(taillePatch, tau);
        image->selectionneOffsets(60);
        offsets = image->getOffsets();

        // Complétion de l'image via Kolmogorov
        int i, j, nombrePixelsMasque = 0;
        resultat = pixels;

        for (i = 0 ; i < tailleY ; i++) {
            for (j = 0 ; j < tailleX ; j++) {
                if (masque.ptr<uchar>(i)[j] < 126)
                    nombrePixelsMasque++;
            }
        }
        int* result = image->optimisationChampsMarkov(nombrePixelsMasque);

        // On renconstitue l'image
        int x = 0, y = 0, k = 0;
        Offset aux;
        for (i = 0 ; i < tailleY ; i++) {
            for (j = 0 ; j < tailleX ; j++) {
                if (masque.ptr<uchar>(i)[j] < 126) {
                    if (result[k] >= 0 && result[k] < offsets.size()) {
                        aux = offsets.at(result[k]);
                        x = j + aux.getX();
                        y = i + aux.getY();

                        if (y >= 0 && y < tailleY && x >= 0 && x < tailleX) {
                            if (masque.ptr<uchar>(y)[x] > 126)
                                resultat.ptr<Vec3b>(i)[j] = pixels.ptr<Vec3b>(y)[x];
                            else
                                cout << "bad, ";
                        }
                        else
                            cout << "bad, ";
                    }
                    else
                        cout << "choix bizarre pour le pixel " << k << " : " << (int) result[k] << endl;
                    k++;
            }
        }
    }

    delete[] result;
}

void ImageCouleur::sauvegarder(const string& nom) {
    imwrite(nom, resultat);
}

void ImageCouleur::afficheImage(const string& fenetre) const {
    imshow(fenetre, pixels);
}

void ImageCouleur::afficheMasque(const string& fenetre) const {
    imshow(fenetre, masque);
}

void ImageCouleur::afficheOffsets() const {
    image->afficheOffsets();
}

void ImageCouleur::afficheResultat(const string& fenetre) const {
    imshow(fenetre, resultat);
}

Image* ImageCouleur::getImageCalculs() const {
    return image;
}

ImageCouleur::~ImageCouleur() {
    delete image;
}

