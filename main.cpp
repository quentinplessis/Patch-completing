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

void testFLANN() {
    namedWindow("image", 0);
    namedWindow("masque", 0);
    namedWindow("resultat", 0);

    int patchs[5] = {2, 4, 8, 16, 32};
    int taus[9] = {0, 3, 6, 10, 13, 17, 23, 27, 32};
    string image = "colonnes.jpg";
    string masque = "masquecols.jpg";

    ImageCouleur im(image, masque);
    im.afficheImage("image");
    im.afficheMasque("masque");

    im.complete(4, 3);
    string nom = "rendus/";
    nom.append("test.jpg");
    im.sauvegarder(nom);

   /* for (int i = 0 ; i < 5 ; i++) {
        for (int j = 0 ; j < 9 ; j++) {
            im.complete(patchs[i], taus[j]);
            std::ostringstream oss;
            // écrire un nombre dans le flux
            oss << "rendus/" << image << "-p" << patchs[i] << "-t" << taus[j] << ".jpg";
            // récupérer une chaîne de caractères
            std::string result = oss.str();

            im.sauvegarder(result);
        }
    }*/




    //im.calculeOffsetsKDTrees(4, 3);
    //im.selectionneOffsets(60);
    //im.completeKolmogorov();
    im.afficheResultat("resultat");
    Rendu3D renduOffsetsTries(im.getImageCalculs(), "K plus grands offsets");


    /*Mat img_1 = imread( "colonnes.jpg", CV_LOAD_IMAGE_GRAYSCALE );
  Mat img_2 = imread( "colonnes.png", CV_LOAD_IMAGE_GRAYSCALE );

  if( !img_1.data || !img_2.data )
  { cout<< " --(!) Error reading images " << std::endl; }

  //-- Step 1: Detect the keypoints using SURF Detector
  int minHessian = 400;

  SurfFeatureDetector detector( minHessian );

  std::vector<KeyPoint> keypoints_1, keypoints_2;

  detector.detect( img_1, keypoints_1 );
  detector.detect( img_2, keypoints_2 );

  //-- Step 2: Calculate descriptors (feature vectors)
  SurfDescriptorExtractor extractor;

  Mat descriptors_1, descriptors_2;

  extractor.compute( img_1, keypoints_1, descriptors_1 );
  extractor.compute( img_2, keypoints_2, descriptors_2 );

  //-- Step 3: Matching descriptor vectors using FLANN matcher
  FlannBasedMatcher matcher;
  std::vector< DMatch > matches;
  matcher.match( descriptors_1, descriptors_2, matches );

  double max_dist = 0; double min_dist = 100;

  //-- Quick calculation of max and min distances between keypoints
  for( int i = 0; i < descriptors_1.rows; i++ )
  { double dist = matches[i].distance;
    if( dist < min_dist ) min_dist = dist;
    if( dist > max_dist ) max_dist = dist;
  }

  printf("-- Max dist : %f \n", max_dist );
  printf("-- Min dist : %f \n", min_dist );

  //-- Draw only "good" matches (i.e. whose distance is less than 2*min_dist )
  //-- PS.- radiusMatch can also be used here.
  std::vector< DMatch > good_matches;

  for( int i = 0; i < descriptors_1.rows; i++ )
  { if( matches[i].distance < 2*min_dist )
    { good_matches.push_back( matches[i]); }
  }

  //-- Draw only "good" matches
  Mat img_matches;
  drawMatches( img_1, keypoints_1, img_2, keypoints_2,
               good_matches, img_matches, Scalar::all(-1), Scalar::all(-1),
               vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

  //-- Show detected matches
  imshow( "Good Matches", img_matches );

  for( int i = 0; i < good_matches.size(); i++ )
   printf( "-- Good Match [%d] Keypoint 1: %d  -- Keypoint 2: %d  \n", i, good_matches[i].queryIdx, good_matches[i].trainIdx );
*/
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

    testFLANN();
    //testCouleur();

    waitKey();
    return 0;
}
