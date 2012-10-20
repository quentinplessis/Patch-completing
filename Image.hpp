#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include <string>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Offset.hpp"

class Image {
    public:
        Image(const std::string& cheminFichier);
        ~Image();
        void affichePixels() const;
        void afficheImage(const std::string& nomFenetre) const;
        const cv::Mat& getPixels() const;

        Offset ajouterOffset(int x, int y);
        void calculeOffsets(int taillePatch, int tau);
        void afficheOffsets();
        void afficheOffsetsPixels(int taillePatch);

    private:
        cv::Mat pixels;
        int tailleX, tailleY;
        std::vector<Offset> offsets;

        Offset** tableauOffsetsPixels;
};

#endif // IMAGE_HPP_INCLUDED
