#include <iostream>
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.hpp"
#include "ImageCouleur.hpp"
#include "Rendu3D.hpp"

using namespace cv;
using namespace std;

void testBlancNoir() {
    // On définit les fenêtres
    namedWindow("image", 0);
    namedWindow("masque", 0);
    namedWindow("resultat", 0);

    try {
        // Chargement et affichage de l'image ainsi que de ses pixels
        Image im("colonnes.jpg", "masquecols.jpg");
        im.afficheImage("image"); // on affiche l'image dans la fenêtre "image"
        im.afficheMasque("masque");

        // Calcul et affichage des offsets
        im.calculeOffsets(8, 3);

        im.selectionneOffsets(60);
        im.afficheOffsets();

        im.completeKolmogorov();
        im.afficheResultat("resultat");
        // à mettre en dernier : affichage via GLUT bloque le reste de l'exécution
        Rendu3D renduOffsetsTries(&im, "K plus grands offsets");
    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
    }
}

void testFLANN(string image, string masque, int patch, int tau) {
    namedWindow("image", 0);
    namedWindow("masque", 0);
    namedWindow("resultat", 0);

    size_t pos;
    ostringstream oss;

    ImageCouleur im(image, masque);
    im.afficheImage("image");
    im.afficheMasque("masque");

    im.complete(patch, tau);

    pos = image.find_last_of("/\\");
    image = image.substr(pos);
    oss << "rendus/\\" << image << "-p" << patch << "-t" << tau << ".jpg";
    im.sauvegarder(oss.str());

    im.afficheResultat("resultat");
    Rendu3D renduOffsetsTries(im.getImageCalculs(), "K plus grands offsets");
}

void boucle() {
    size_t pos;
    ostringstream oss;
    ImageCouleur* im;
    string image;

    int patchs[8] = {2, 4, 8, 10, 16, 20, 24, 32};
    int taus[9] = {0, 4, 8, 10, 13, 17, 23, 27, 32};
    string images[11] = {"inputs/colonnes.png", "inputs/cat.jpg", "inputs/escargot.jpg", "inputs/escargot-trait.jpg", "inputs/gate_input.jpg", "inputs/house_input.jpg", "inputs/pano.jpg", "inputs/panorama300.jpg", "inputs/plage.jpg", "inputs/pumpkin_input.jpg", "inputs/soldier_input.jpg"};
    string masques[11] = {"inputs/masquecols2.jpg", "inputs/catM.jpg", "inputs/escargotM.jpg", "inputs/escargotM.jpg", "inputs/gate_inputM.jpg", "inputs/house_inputM.jpg", "inputs/panomasque.jpg", "inputs/panoramaM.jpg", "inputs/plageM.jpg", "inputs/pumpkin_inputM.jpg", "inputs/soldier_inputM.jpg"};

    for (int i = 5 ; i < 10 ; i++) {
        for (int j = 0 ; j < 8 ; j++) {
            for (int k = 0 ; k < 9 ; k++) {
                im = new ImageCouleur(images[i], masques[i]);
                im->complete(patchs[j], taus[k]);
                pos = images[i].find_last_of("/\\");
                image = images[i].substr(pos);
                ostringstream oss;
                oss << "rendus/\\" << image << "-p" << patchs[j] << "-t" << taus[k] << ".jpg";
                im->sauvegarder(oss.str());
                delete im;
            }
        }
    }
}

void testCouleur() {
    // On définit une fenêtre
    namedWindow("image", 0);
    namedWindow("masque", 0);
    namedWindow("resultat", 0);

    try {
        // Chargement et affichage de l'image ainsi que de ses pixels
        ImageCouleur im("colonnes.jpg", "masquecols.jpg");

        im.afficheImage("image"); // on affiche l'image dans la fenêtre "image"
        im.afficheMasque("masque");

        im.complete(4, 3);
        im.afficheResultat("resultat");

        Rendu3D renduOffsetsTries(im.getImageCalculs(), "K plus grands offsets");
    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
    }
}

int main( int argc, char** argv )
{
    glutInit(&argc, (char **)argv);

    if (argc == 2)
        boucle();
    else if (argc == 3)
        testFLANN(argv[1], argv[2], 4, 3);
    else if (argc == 4)
        testFLANN(argv[1], argv[2], atoi(argv[3]), 3);
    else if (argc == 5)
        testFLANN(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
    else
        testFLANN("inputs/colonnes.jpg", "inputs/masquecols.jpg", 4, 3);

    waitKey();
    return 0;
}
