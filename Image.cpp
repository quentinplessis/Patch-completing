#include <iostream>
#include <time.h>
#include <algorithm>
#include "Image.hpp"
#include "Patch.hpp"
#include "gco-v3.0/GCoptimization.h"

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
               // tableauOffsetsPixels[i][j] = Offset();
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

Mat Image::complete() {
    resultat = pixels;

    int i, j, x, y;
    int valeur, nombre;
    vector<Offset>::iterator k;
    char aux;

    for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126) {
                valeur = 0;
                nombre = 0;
                // On parcourt tout le tableau des offsets
                for (k = offsets.begin() ; k != offsets.end() ; ++k) {
                    x = j + k->getX();
                    y = i + k->getY();
                    if (y >= 0 && y < tailleY && x >= 0 && x < tailleX) {
                        if (masque.ptr<uchar>(y)[x] > 126) {
                            valeur += pixels.ptr<uchar>(y)[x];
                            nombre++;
                        }
                    }
                }
                if (nombre > 0)
                    resultat.ptr<uchar>(i)[j] = (char) (valeur / nombre);
                else
                    resultat.ptr<uchar>(i)[j] = 0;
            }
        }
    }

    cout << "salut" << (int) resultat.ptr<uchar>(0)[0] << endl;

    return resultat;
}

void Image::completeKolmogorov() {
    int i, j;
    int nombrePixelsMasque = 0;
    resultat = pixels;

    for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126)
                nombrePixelsMasque++;
        }
    }
    int* result = GeneralGraph_DArraySArraySpatVarying(nombrePixelsMasque);
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
                        if (masque.ptr<uchar>(y)[x] > 126) {
                            resultat.ptr<uchar>(i)[j] = pixels.ptr<uchar>(y)[x];
                        }
                        else
                            resultat.ptr<uchar>(i)[j] = 0;
                    }
                    else
                        resultat.ptr<uchar>(i)[j] = 0;
                }
                else {
                    resultat.ptr<uchar>(i)[j] = 0;
                    cout << "choix " << k << " : " << (int) result[k] << endl;
                }
                k++;
            }
        }
    }
}

int* Image::GeneralGraph_DArraySArraySpatVarying(int nombrePixels) {
    int nombreLabels = offsets.size();
	int *result = new int[nombrePixels];   // stores result of optimization
	int *traduction = new int[nombrePixels];

	int i, j, x, y;
	vector<Offset>::iterator k;

	// first set up the array for data costs
	int *data = new int[nombrePixels*nombreLabels];
	int pixel = 0;
	for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126) {
                for (k = offsets.begin() ; k != offsets.end() ; ++k) {
                    x = j + k->getX();
                    y = i + k->getY();
                    if (y >= 0 && y < tailleY && x >= 0 && x < tailleX && masque.ptr<uchar>(y)[x] > 126)
                        data[pixel] = 0;
                    else
                        data[pixel] = 1000;
                    pixel++;
                }
            }
        }
    }
	/*for ( int i = 0; i < num_pixels; i++ )
		for (int l = 0; l < num_labels; l++ )
			if (i < 25 ){
				if(  l == 0 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}
			else {
				if(  l == 5 ) data[i*num_labels+l] = 0;
				else data[i*num_labels+l] = 10;
			}*/

	// next set up the array for smooth costs
	int *smooth = new int[nombreLabels*nombreLabels];
	for ( int l1 = 0; l1 < nombreLabels; l1++ )
		for (int l2 = 0; l2 < nombreLabels; l2++ )
			smooth[l1+l2*nombreLabels] = (l1-l2)*(l1-l2) <= 4  ? (l1-l2)*(l1-l2):4;


	try{
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(nombrePixels, nombreLabels);
		gc->setDataCost(data);
		gc->setSmoothCost(smooth);

		// now set up a grid neighborhood system
		pixel = 0;
		int *voisins = new int[tailleX];
		bool precedentEstVoisin = false;
        for (i = 0 ; i < tailleY ; i++) {
		    precedentEstVoisin = false;
            for (j = 0 ; j < tailleX ; j++) {
                if (masque.ptr<uchar>(i)[j] < 126) {
                    if (precedentEstVoisin)
                        gc->setNeighbors(pixel-1, pixel, 1);
                    precedentEstVoisin = true;

                    if (i != 0) {
                        if (voisins[j] != -1)
                        gc->setNeighbors(voisins[j], pixel, 1);
                    }
                    voisins[j] = pixel;
                    pixel++;
                }
                else {
                    precedentEstVoisin = false;
                    voisins[j] = -1;
                }
            }
		}
		delete[] voisins;

		// first set up horizontal neighbors
		/*for (int y = 0; y < height; y++ )
			for (int  x = 1; x < width; x++ ){
				int p1 = x-1+y*width;
				int p2 =x+y*width;
				gc->setNeighbors(p1,p2,p1+p2);
			}

		// next set up vertical neighbors
		for (int y = 1; y < height; y++ )
			for (int  x = 0; x < width; x++ ){
				int p1 = x+(y-1)*width;
				int p2 =x+y*width;
				gc->setNeighbors(p1,p2,p1*p2);
			}*/

        /*for (i = 0; i < nombrePixels; i++ )
			traduction[i] = gc->whatLabel(i);*/

			for (i = 0; i < 50; i++ )
			cout << "i " << i << " : " << gc->whatLabel(i) << endl;

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

		/*for (i = 0; i < nombrePixels; i++ ) {
		    int label = gc->whatLabel(i);
		    result[i] = 0;
            for (j = 0 ; j < nombrePixels ; j++) {
                if (traduction[j] == label) {
                    result[i] = j;
                    break;
                }
            }
		}*/

		for (i = 0; i < nombrePixels; i++ )
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] smooth;
	delete [] data;

    return result;

}

void Image::afficheResultat(const string& nomFenetre) const {
    imshow(nomFenetre, resultat);
}

void Image::calcule2(int taillePatch, int tau) {
    Patch p1(taillePatch, pixels, masque), p2(taillePatch, pixels, masque);
    long diffMin, diff;
    int distCarre;
    int x = 0, y = 0; // composantes de l'offset à trouver pour chaque pixel
    int i, j, k, l;
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
                //tableauOffsetsPixels[i][j] = ajouterOffset(x, y);
                ajouterOffset(x, y);
            }
            else {
                //tableauOffsetsPixels[i][j] = Offset();
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



