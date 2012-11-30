#include <iostream>
#include <time.h>
#include <algorithm>
#include <limits>


#include "Image.hpp"
#include "Patch.hpp"
#include "GCoptimization.h"

using namespace std;
using namespace cv;

struct ForDataFn{
	int numLab;
	int *data;
};


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
                    if (y >=0 && y < tailleY && x >= 0 && x < tailleX) {
                        if (masque.ptr<uchar>(y)[x] > 126) {
                            valeur += pixels.ptr<uchar>(y)[x];
                            nombre++;
                        }
                    }
                }
                if (nombre > 0)
                    resultat.ptr<uchar>(i)[j] = (char) (valeur / nombre);
            }
        }
    }

    cout << "salut" << (int) resultat.ptr<uchar>(0)[0] << endl;

    return resultat;
}

void Image::afficheResultat(const string& nomFenetre) const {
    imshow(nomFenetre, resultat);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////Utilisation de Graph Cuts//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

int dataFn(int p, int l, void *data)
{
	ForDataFn *myData = (ForDataFn *) data;
	int numLab = myData->numLab;

	return( myData->data[p*numLab+l] );
}

//Définit la fonction Es de l'article
int Image::smoothFn(int p1, int p2, int l1, int l2)
{
    //Coordonnées des pixels 1 et 2 à partir de leur numéro
    int y1 = p1/tailleX;
    int x1 = p1-y1*tailleX;
    int y2 = p2/tailleX;
    int x2 = p2-y2*tailleX;

    //pixel1L1 = valeur du pixel situé en pixel1+(offset associé au Label 1)
    int pixel1L1 = pixels.ptr<uchar>(y1+offsets[l1].getY())[x1+offsets[l1].getX()];
    int pixel1L2 = pixels.ptr<uchar>(y1+offsets[l2].getY())[x1+offsets[l2].getX()];
    int pixel2L1 = pixels.ptr<uchar>(y2+offsets[l1].getY())[x2+offsets[l1].getX()];
    int pixel2L2 = pixels.ptr<uchar>(y2+offsets[l2].getY())[x2+offsets[l2].getX()];

    return ((pixel1L1-pixel1L2)*(pixel1L1-pixel1L2) + (pixel2L1-pixel2L2)*(pixel2L1-pixel2L2));
}


//Calcul l'optimisation de l'énergie en utilisant la fonction Ed et Es
void Image::GridGraph_DfnSfn(int width,int height,int num_pixels,int num_labels)
{
    int x = 0;
    int y = 0;
    int sup = 0;
	int *result = new int[num_pixels];   // stores result of optimization

    //Définit l'infini pour les entiers
    sup = numeric_limits<int>::max();

	//Définit un tableau nécessaire pour la fonction Ed des données
	//Pour chaque pixel p en (x,y) on a num_labels cases correspondant à la valeur de Ed  pour le pixel p et le label i 0<=i<num_labels.
	//Le tableau se parcours ainsi : [pixel1 label1 ; pixel1 Label2 ; ... ; pixel 1 Label60 ; pixel2 label2 ; .... ; pixel2 label 60 ; ...]
	int *data = new int[num_pixels*num_labels];

    //Défini la fonction Data (Ed de l'article)
	//y=numpixel/width
	//x=numpixel -y*width avec num_pixel commençant à 0
	for ( int i = 0; i < num_pixels; i++ )//Parcours de tous les pixels
	{
	    y = i/width;
	    x = i-y*width;
		for (int l = 0; l < num_labels; l++ )//Parcours des labels
        {
			if (this->estConnu(x+offsets[l].getX(),y+offsets[l].getY()))
			{
				data[i*num_labels+l] = 0;
			}
			else
			{
                data[i*num_labels+l] = sup;
			}

        }
	}

    //Effectue l'optimisation
	try{
		GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width,height,num_labels);

		// set up the needed data to pass to function for the data costs
		ForDataFn toFn;
		toFn.data = data;
		toFn.numLab = num_labels;

		gc->setDataCost(&dataFn,&toFn);

		// smoothness comes from function pointer
		gc->setSmoothCost(&smoothFn);

		printf("\nBefore optimization energy is %d",gc->compute_energy());
		gc->expansion(2);// run expansion for 2 iterations. For swap use gc->swap(num_iterations);
		printf("\nAfter optimization energy is %d",gc->compute_energy());

        //On récupère le résultat de l'optimisation
		for ( int  i = 0; i < num_pixels; i++ )
			result[i] = gc->whatLabel(i);   //Pour un pixel i de l'image (zone inconnue ?), on récupère le numéro de l'offset (whatLabel(i)) tel que pixel(i)=(i+offset)

        complete(result);

		delete gc;
	}
	catch (GCException e){
		e.Report();
	}

	delete [] result;
	delete [] data;

}

//Complète l'image avec le résultat
//WARNING : NE FONCTIONNE SUREMENT PAS !!!
//A VOIR : qu'est ce qui est stocké dans résult ? Toute l'image ou uniquement les pixels de la zone inconnue ?
Mat Image::completeKolmogorov(int *result) {
    resultat = pixels;

    int i = 0, j = 0;

    for (i = 0 ; i < tailleY ; i++) {
        for (j = 0 ; j < tailleX ; j++) {
                    resultat.ptr<uchar>(i)[j] = (char) (result[tailleX*j+i]);
            }
        }
    }
    return resultat;
}

//Retourne vrai si le pixel (x,y) est dans la zone connue
bool Image::estConnu(int x, int y) {
    return ((int) masque.ptr<uchar>(y)[x]) > 126;
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



