#include <iostream>
#include <time.h>
#include <algorithm>
#include "Image.hpp"
#include "Patch.hpp"
#include "gco-v3.0/GCoptimization.h"
#include "flann/flann.hpp"


#define INFINI  10000000

using namespace std;
using namespace cv;


Image::Image(const string& cheminFichier, const string& cheminMasque) {
    pixels = imread(cheminFichier, 0); // charge l'image en niveaux de gris
    if (pixels.empty())
        throw string("Image vide");

    tailleX = pixels.cols;
    tailleY = pixels.rows;

    masque = imread(cheminMasque, 0);
    if (masque.cols != tailleX || masque.rows != tailleY)
        throw string("Le masque n'est pas de la même taille que l'image");
}

/**
    Fonction de comparaison de deux offsets utilisée pour trier les offsets par nombre d'occurences.
    Renvoie true si o1 apparaît plus de fois que o2.
    Un tri par ordre croissant permet donc de trier les offsets du plus fréquent au moins fréquent.
*/
bool comparaisonOffsets(Offset o1, Offset o2) {
    return o1.getN() > o2.getN();
}

/**
    Ajoute un offset à l'image s'il n'existe pas, sinon incrémente son nombre d'occurences.
*/
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

/**
    Pour chaque pixel de l'image, détermine le vecteur tel que le patch de pixels dont le coin en haut à gauche
    est le pixel actuel et de taille taillePatch ressemble le plus à son translaté par le vecteur en question.
    Les vecteurs de norme inférieure ou égale à tau ne sont pas retenus.
*/
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
            if (!patch1Valide)
                continue;

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
        }
        std::cout << "Ligne " << i << std::endl;
    }
    endTime = clock();
    std::cout << "Time: " << (endTime - beginTime) / CLOCKS_PER_SEC << std::endl << std::endl;

    delete p;
    delete pp;
    delete p2;
    delete pp2;
}

void Image::calculeOffsetsKDTrees(int taillePatch, int tau) {
    int i, j, k, l;
    int nombreDonnees = 0, nombreDimensions = taillePatch*taillePatch;
    bool patchValide = true;

    //int requeteX = 4, requeteY = 1;

    offsets.clear();

    clock_t beginTime = clock();
    clock_t endTime;

    // On détermine le nombre de pixels de l'image pour lesquels il faut calculer les offsets
    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            if (masque.ptr<uchar>(i)[j] > 126) {
                patchValide = true;
                for (k = 0 ; k < taillePatch ; k++) {
                    for (l = 0 ; l < taillePatch ; l++) {
                        if (masque.ptr<uchar>(i+k)[j+l] < 126) {
                            patchValide = false;
                            break;
                        }
                    }
                    if (!patchValide)
                        break;
                }
                if (patchValide)
                    nombreDonnees++;
            }
        }
    }

    std::cout << "Nombre d'offsets a calculer : " << nombreDonnees << std::endl;


    float* data = new float[nombreDonnees*nombreDimensions];
    float* dataQuery = new float[nombreDonnees*nombreDimensions];
    int* posXPixelValide = new int[nombreDonnees];
    int* posYPixelValide = new int[nombreDonnees];

    // On remplit les données et les requêtes
    int pixelValide = 0;
    for (i = 0 ; i < tailleY - taillePatch + 1 ; i++) {
        for (j = 0 ; j < tailleX - taillePatch + 1 ; j++) {
            if (masque.ptr<uchar>(i)[j] > 126) {
                patchValide = true;
                for (k = 0 ; k < taillePatch ; k++) {
                    for (l = 0 ; l < taillePatch ; l++) {
                        if (masque.ptr<uchar>(i+k)[j+l] < 126) {
                            patchValide = false;
                            break;
                        }
                    }
                    if (!patchValide)
                        break;
                }
                if (patchValide) {
                    for (k = 0 ; k < taillePatch ; k++) {
                        for (l = 0 ; l < taillePatch ; l++) {
                            data[pixelValide*nombreDimensions+k*taillePatch+l] = pixels.ptr<uchar>(i+k)[j+l];
                            //if (i == requeteY && j == requeteX)
                            dataQuery[pixelValide*nombreDimensions+k*taillePatch+l] = pixels.ptr<uchar>(i+k)[j+l];
                        }
                    }
                    posXPixelValide[pixelValide] = j;
                    posYPixelValide[pixelValide] = i;
                    pixelValide++;
                }
            }
        }
    }

    int nn = 2; // nombre de nearest neighbor à rechercher
    flann::Matrix<float> dataset(data, nombreDonnees, nombreDimensions);
    flann::Matrix<float> query(dataQuery, nombreDonnees, nombreDimensions);
    flann::Matrix<int> indices(new int[query.rows*nn], query.rows, nn);
    flann::Matrix<float> dists(new float[query.rows*nn], query.rows, nn);
    // construct an randomized kd-tree index using 4 kd-trees
    flann::Index<flann::L2<float> > index(dataset, flann::KDTreeIndexParams(4));
    index.buildIndex();
    // do a knn search, using 25 checks
    index.knnSearch(query, indices, dists, nn, flann::SearchParams(25));

    for (i = 0 ; i < nombreDonnees ; i++) {
        //cout << "Pour (" << posXPixelValide[i] << "," << posYPixelValide[i] << ") : ";
        for (j = 0 ; j < nn ; j++) {
            int id = indices.ptr()[i*nn+j];

            int x = (posXPixelValide[id]-posXPixelValide[i]);
            int y = (posYPixelValide[id]-posYPixelValide[i]);

            long distCarre = x*x+y*y;
            if (distCarre > tau*tau) { // mauvais
                ajouterOffset(x, y);
                break;
            }

        }
    }

    cout << "Resultat : " << offsets.size() << " offsets uniques" << endl;

    endTime = clock();
    std::cout << "Time: " << (endTime - beginTime) / CLOCKS_PER_SEC << std::endl << std::endl;

    delete[] data;
    delete[] dataQuery;
    delete[] posXPixelValide;
    delete[] posYPixelValide;
}

/**
    Selectionne les K offsets les plus fréquents
*/
void Image::selectionneOffsets(int K) {
    std::cout << "Au total, il y a " << offsets.size() << " offsets." << std::endl;
    std::cout << "On ne conserve que les " << K << " plus frequents." << std::endl << std::endl;
    if (offsets.size() > K) {
        std::sort(offsets.begin(), offsets.end(), comparaisonOffsets);
        offsets.resize(K);
    }
}

/**
    Complète la zone inconnue de l'image en affectant à chaque pixel manquant une valeur
    égale à la moyenne des valeurs des K-translatés.
*/
void Image::completeMoyenne() {
    resultat = pixels;
    int i, j, x, y;
    int valeur, nombre;
    vector<Offset>::iterator k;

    std::cout << "On complete la zone inconnue avec la methode de la moyenne." << std::endl << std::endl;

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
}

/**
    Complète la zone inconnue de l'image en utilisant la méthode des graph-cuts.
    On utilise les graph-cuts de Kolmogorov.
*/
void Image::completeKolmogorov() {
    int i, j;
    int nombrePixelsMasque = 0;
    resultat = pixels;

    std::cout << "On complete la zone inconnue avec la methode des graph-cuts. On utilise les graph-cuts de Kolmogorov." << std::endl;

    for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126)
                nombrePixelsMasque++;
        }
    }
    int* result = optimisationChampsMarkov(nombrePixelsMasque);

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
                            //std::cout << result[k] << "," << aux.getX() << "," << aux.getY() << " ";
                        }
                        else
                            resultat.ptr<uchar>(i)[j] = 0;
                    }
                    else
                        resultat.ptr<uchar>(i)[j] = 0;
                }
                else {
                    resultat.ptr<uchar>(i)[j] = 0;
                    std::cout << "Label bizarre pour le pixel " << k << " : " << (int) result[k] << std::endl;
                }
                k++;
            }
        }
    }

    delete[] result;
}

/**
    Définition du terme a priori, Es.
*/
int Image::smoothFunction(int p1, int p2, int l1, int l2) {
    //Coordonnées des pixels 1 et 2 à partir de leur numéro
    int y1 = posYPixels[p1];
    int x1 = posXPixels[p1];
    int y2 = posYPixels[p2];
    int x2 = posXPixels[p2];

    int y1l1 = y1+offsets.at(l1).getY();
    int x1l1 = x1+offsets.at(l1).getX();
    int y1l2 = y1+offsets.at(l2).getY();
    int x1l2 = x1+offsets.at(l2).getX();
    int y2l1 = y2+offsets.at(l1).getY();
    int x2l1 = x2+offsets.at(l1).getX();
    int y2l2 = y2+offsets.at(l2).getY();
    int x2l2 = x2+offsets.at(l2).getX();

    if (y1l1 >= 0 && y1l1 < tailleY && y1l2 >= 0 && y1l2 < tailleY && y2l1 >= 0 && y2l1 < tailleY && y2l2 >= 0 && y2l2 < tailleY) {
        if (x1l1 >= 0 && x1l1 < tailleX && x1l2 >= 0 && x1l2 < tailleX && x2l1 >= 0 && x2l1 < tailleX && x2l2 >= 0 && x2l2 < tailleX) {
            //pixel1L1 = valeur du pixel situé en pixel1+(offset associé au Label 1)
            uchar pixel1L1 = pixels.ptr<uchar>(y1l1)[x1l1];
            uchar pixel1L2 = pixels.ptr<uchar>(y1l2)[x1l2];
            uchar pixel2L1 = pixels.ptr<uchar>(y2l1)[x2l1];
            uchar pixel2L2 = pixels.ptr<uchar>(y2l2)[x2l2];

            int res = (int) ((pixel1L1-pixel1L2)*(pixel1L1-pixel1L2) + (pixel2L1-pixel2L2)*(pixel2L1-pixel2L2));
            return res;
        }
        else
            return INFINI;
    }
    else
        return INFINI;
}

/**
    Optimise l'énergie associée aux champs de Markov mis en jeu.
    Renvoie une tableau indiquant le label affecté à chaque pixel.
*/
int* Image::optimisationChampsMarkov(int nombrePixels) {
    int nombreLabels = offsets.size();
	int *result = new int[nombrePixels];   // resultat de l'optimisation : result[i] = label à attribuer au pixel i
	int *data = new int[nombrePixels*nombreLabels];
	int i, j, x, y;
	vector<Offset>::iterator k;

    std::cout << "Affectation des labels aux pixels." << std::endl << "Nombre de pixels : " << nombrePixels << ", nombre de labels : " << nombreLabels << std::endl;

	//Définition de la fonction d'attache aux données Ed : data
	int pixel = 0;
	for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126) { //Si le pixel se situe dans la zone inconnue
                for (k = offsets.begin() ; k != offsets.end() ; ++k) { //On parstd::cout tous les Offsets
                    //Coordonnées du pixel+offset
                    x = j + k->getX();
                    y = i + k->getY();

                    if (y >= 0 && y < tailleY && x >= 0 && x < tailleX && masque.ptr<uchar>(y)[x] > 126)//Si pixel+offset se situe dans l'image ET dans la zone connue
                        data[pixel] = 0;
                    else
                        data[pixel] = INFINI;
                    pixel++;
                }
            }
        }
    }

    // On remplit deux tableaux pour représenter les coordonnées dans l'image des pixels manquants
    pixel = 0;
    posXPixels = new int[nombrePixels];
    posYPixels = new int[nombrePixels];
    for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
            if (masque.ptr<uchar>(i)[j] < 126) { //Si le pixel se situe dans la zone inconnue
                posXPixels[pixel] = j;
                posYPixels[pixel] = i;
                pixel++;
            }
        }
    }

	try {
		GCoptimizationGeneralGraph *gc = new GCoptimizationGeneralGraph(nombrePixels, nombreLabels);
		gc->setDataCost(data);
		gc->setSmoothCost(&Image::staticSmoothFunction, this);

		// On définit les voisins dans la zone inconnue (4-connexité)
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

        // On optimise
		printf("Valeur de l'energie avant optimisation : %lld \n", gc->compute_energy());
		gc->swap(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
        printf("Valeur de l'energie apres optimisation : %lld \n", gc->compute_energy());

        // On stocke le résultat
		for (i = 0; i < nombrePixels; i++)
			result[i] = gc->whatLabel(i);

		delete gc;
	}
	catch (GCException e) {
		e.Report();
	}
	delete[] data;
	delete[] posXPixels;
    delete[] posYPixels;

    return result;
}

void Image::sauvegarder(const string& nom) {
    imwrite(nom, resultat);
}

void Image::affichePixels() const {
    std::cout << "Taille Image : " << tailleX << "x" << tailleY << std::endl;
    std::cout << "Pixels = " << std::endl << pixels << std::endl << std::endl;
    std::cout << "Masque = " << std::endl << masque << std::endl << std::endl;
}

void Image::afficheImage(const std::string& nomFenetre) const {
    imshow(nomFenetre, pixels);
}

void Image::afficheMasque(const std::string& nomFenetre) const {
    imshow(nomFenetre, masque);
}

void Image::afficheOffsets() {
    std::cout << "Offsets uniques de l'image : " << offsets.size() << std::endl;
    vector<Offset>::iterator i;
    // On parcourt tout le tableau des offsets
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
       i->afficheTriplet();
    }
}

void Image::afficheResultat(const string& nomFenetre) const {
    imshow(nomFenetre, resultat);
}

const Mat& Image::getPixels() const {
    const Mat& ref = pixels;
    return ref;
}

const Mat& Image::getMasque() const {
    const Mat& ref = masque;
    return ref;
}

int Image::getTailleX() const {
    return tailleX;
}

int Image::getTailleY() const {
    return tailleY;
}

std::vector<Offset> Image::getOffsets() const {
    return offsets;
}











