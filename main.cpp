#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.hpp"
#include "ImageCouleur.hpp"
#include "Patch.hpp"
#include "Rendu3D.hpp"

using namespace cv;
using namespace std;

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
        im.calculeOffsets(8, 3);
        //Rendu3D renduOffsets(&im, "Tous les offsets");
        im.selectionneOffsets(60);
        im.afficheOffsets();
        //im.calcule2(2, 0);
        //im.afficheOffsets();
        //im.afficheOffsetsPixels(2);
        im.completeKolmogorov();
        im.afficheResultat("resultat");
        // à mettre en dernier : affichage via GLUT bloque le reste de l'exécution
        Rendu3D renduOffsetsTries(&im, "K plus grands offsets");
    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
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

    testCouleur();

    waitKey();
    return 0;
}
