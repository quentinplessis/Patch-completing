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
        Image(const std::string& cheminFichier, const std::string& cheminMasque);
        ~Image();
        void affichePixels() const;
        void afficheImage(const std::string& nomFenetre) const;
        void afficheMasque(const std::string& nomFenetre) const;
        const cv::Mat& getPixels() const;
        const cv::Mat& getMasque() const;
        void setMasque(const std::string& cheminFichier);

        Offset ajouterOffset(int x, int y);
        void calculeOffsets(int taillePatch, int tau);
        void calcule2(int taillePatch, int tau);
        void afficheOffsets();
        void afficheOffsetsPixels(int taillePatch);
        std::vector<Offset> getOffsets() const;
        int getTaille() const;
        void selectionneOffsets(int K);
        cv::Mat complete();
        void afficheResultat(const std::string& nomFenetre) const;

    private:
        cv::Mat pixels;
        cv::Mat masque;
        cv::Mat resultat;
        bool possedeMasque;
        int tailleX, tailleY;
        std::vector<Offset> offsets;

        Offset** tableauOffsetsPixels;
};


#endif // IMAGE_HPP_INCLUDED
