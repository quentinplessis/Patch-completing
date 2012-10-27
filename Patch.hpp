#ifndef PATCH_HPP_INCLUDED
#define PATCH_HPP_INCLUDED

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

class Patch {
    public:
        Patch(int posX, int posY, int tailleX);
        Patch(int posX, int posY, int tailleX, int tailleY);
        Patch(int taille, const cv::Mat& pixels, const cv::Mat& masque);

        void setPosition(int posX, int posY);
        void affichePixels() const;
        int getTailleX() const;
        int getTailleY() const;
        int getPosX() const;
        int getPosY() const;

        long difference(const Patch patch) const;
        bool estValide();

    private:
        int tailleX, tailleY;
        int posX, posY;
        cv::Mat pixels;
        cv::Mat masque;

        bool estConnu(int x, int y);
};


#endif // PATCH_HPP_INCLUDED
