#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.hpp"
#include "Patch.hpp"
#include "Rendu3D.hpp"

#include <iostream>

using namespace cv;
using namespace std;


void testBlancNoirPetit() {
    // On définit une fenêtre
    namedWindow("image", 0);
    namedWindow("masque", 0);

    try {
        // Chargement et affichage de l'image ainsi que de ses pixels
        Image im("blancnoirpetit.jpg", "masqueblancnoirpetit.jpg");
        //Image im("blancnoirpetit.jpg");
        im.affichePixels();
        im.afficheImage("image"); // on affiche l'image dans la fenêtre "image"
        im.afficheMasque("masque");

        // Définition d'un patch qu'on déplace sur l'image et dont on affiche les pixels
        Patch patch(2, im.getPixels(), im.getMasque());
        patch.affichePixels();
        patch.setPosition(3,0);
        patch.affichePixels();
        patch.setPosition(4,0);
        patch.affichePixels();
        patch.setPosition(4,4);
        patch.affichePixels();
        patch.setPosition(5,4);
        patch.affichePixels();
        cout << endl;

        // Définition d'un autre patch et calculs de différences
        cout << "Differences entre patchs : " << endl;
        Patch patch2(2,im.getPixels(), im.getMasque());
        patch.setPosition(0,0);

        // Dans la même zone de l'image
        patch2.setPosition(3,3);
        patch.affichePixels();
        patch2.affichePixels();
        cout << "diff = " << patch.difference(patch2) << endl << endl << endl;

        // Un dans la zone noire et un entre les deux
        patch2.setPosition(4,2);
        patch.affichePixels();
        patch2.affichePixels();
        cout << "diff = " << patch.difference(patch2) << endl << endl << endl;

        // Un dans la zone noire et un dans la zone blanche
        patch2.setPosition(6,2);
        patch.affichePixels();
        patch2.affichePixels();
        cout << "diff = " << patch.difference(patch2) << endl << endl << endl;

        // Calcul et affichage des offsets
        im.calculeOffsets(2, 0);
        im.afficheOffsets();
        //im.afficheOffsetsPixels(2);


        Rendu3D renduOffsets(&im, "Rendu");
    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
    }
}

void testBlancNoir() {
    // On définit une fenêtre
    namedWindow("image", 0);
    namedWindow("masque", 0);
    namedWindow("resultat", 0);

    try {
        // Chargement et affichage de l'image ainsi que de ses pixels
        Image im("colonnes.jpg", "masquecols.jpg");
        //Image im("blancnoirpetit.jpg");
        //im.affichePixels();
        im.afficheImage("image"); // on affiche l'image dans la fenêtre "image"
        im.afficheMasque("masque");

        // Calcul et affichage des offsets
        im.calculeOffsets(8, 0);
        //Rendu3D renduOffsets(&im, "Tous les offsets");
        im.selectionneOffsets(60);
        im.afficheOffsets();
        //im.calcule2(2, 0);
        //im.afficheOffsets();
        //im.afficheOffsetsPixels(2);
        im.completeKolmogorov();
        im.afficheResultat("resultat");
        Rendu3D renduOffsetsTries(&im, "K plus grands offsets");
    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
    }
}

int main( int argc, char** argv )
{
    glutInit(&argc, (char **)argv);

    testBlancNoir();

    waitKey();
    return 0;
}
