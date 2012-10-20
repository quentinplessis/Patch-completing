#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "Image.hpp"
#include "Patch.hpp"

#include <iostream>

using namespace cv;
using namespace std;

int _brightness = 100;
int _contrast = 100;

Mat image;

/* brightness/contrast callback function */
void updateBrightnessContrast( int /*arg*/, void* )
{
    int histSize = 64;
    int brightness = _brightness - 100;
    int contrast = _contrast - 100;

    /*
     * The algorithm is by Werner D. Streidt
     * (http://visca.com/ffactory/archives/5-99/msg00021.html)
     */
    double a, b;
    if( contrast > 0 )
    {
        double delta = 127.*contrast/100;
        a = 255./(255. - delta*2);
        b = a*(brightness - delta);
    }
    else
    {
        double delta = -128.*contrast/100;
        a = (256.-delta*2)/255.;
        b = a*brightness + delta;
    }

    Mat dst, hist;
    image.convertTo(dst, CV_8U, a, b);
    imshow("image", dst);

    calcHist(&dst, 1, 0, Mat(), hist, 1, &histSize, 0);
    Mat histImage = Mat::ones(200, 320, CV_8U)*255;

    normalize(hist, hist, 0, histImage.rows, CV_MINMAX, CV_32F);

    histImage = Scalar::all(255);
    int binW = cvRound((double)histImage.cols/histSize);

    for( int i = 0; i < histSize; i++ )
        rectangle( histImage, Point(i*binW, histImage.rows),
                   Point((i+1)*binW, histImage.rows - cvRound(hist.at<float>(i))),
                   Scalar::all(0), -1, 8, 0 );
    imshow("histogram", histImage);
}
void help()
{
	cout << "\nThis program demonstrates the use of calcHist() -- histogram creation.\n"
			"Call:\n"
			"demhist [image_name -- Defaults to baboon.jpg]\n" << endl;
}

void testBlancNoirPetit() {
    // On définit une fenêtre
    namedWindow("image", 0);

    try {
        // Chargement et affichage de l'image ainsi que de ses pixels
        Image im("blancnoirpetit.jpg");
        im.affichePixels();
        im.afficheImage("image"); // on affiche l'image dans la fenêtre "image"

        // Définition d'un patch qu'on déplace sur l'image et dont on affiche les pixels
        Patch patch(2, im.getPixels());
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
        Patch patch2(2,im.getPixels());
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
        im.afficheOffsetsPixels(2);

    }
    catch (string const& e) {
        cout << "Exception : " << e << endl;
    }
}

int main( int argc, char** argv )
{
    // Load the source image. HighGUI use.
    /*image = imread( argc == 2 ? argv[1] : "photo.jpg", 0 );

    if( image.empty() )
    {
        help();
        return -1;
    }
    help();
    namedWindow("image", 0);
    namedWindow("histogram", 0);

    createTrackbar("brightness", "image", &_brightness, 200, updateBrightnessContrast);
    createTrackbar("contrast", "image", &_contrast, 200, updateBrightnessContrast);

    updateBrightnessContrast(0, 0);*/

    testBlancNoirPetit();

    waitKey();
    return 0;
}
