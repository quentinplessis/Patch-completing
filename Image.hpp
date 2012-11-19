#ifndef IMAGE_HPP_INCLUDED
#define IMAGE_HPP_INCLUDED

#include <string>
#include <vector>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "Offset.hpp"

class Image {
    public:
        Image(const std::string& cheminFichier, const std::string& cheminMasque);

        void affichePixels() const;
        void afficheImage(const std::string& nomFenetre) const;
        void afficheMasque(const std::string& nomFenetre) const;
        void afficheOffsets();
        void afficheOffsetsPixels(int taillePatch);
        void afficheResultat(const std::string& nomFenetre) const;
        void sauvegarder(const std::string& nom);

        const cv::Mat& getPixels() const;
        const cv::Mat& getMasque() const;
        int getTailleX() const;
        int getTailleY() const;
        std::vector<Offset> getOffsets() const;

        Offset ajouterOffset(int x, int y);
        void calculeOffsets(int taillePatch, int tau);
        void calculeOffsetsKDTrees(int taillePatch, int tau);
        void selectionneOffsets(int K);

        void completeMoyenne();
        void completeKolmogorov();

        int* optimisationChampsMarkov(int nombrePixels);
        int smoothFunction(int p1, int p2, int l1, int l2);

        static int staticSmoothFunction(int p1, int p2, int l1, int l2, void *param) {
            Image* obj = reinterpret_cast<Image*>(param);
            return obj->smoothFunction(p1, p2, l1, l2);
        }

    private:
        cv::Mat pixels;
        cv::Mat masque;
        cv::Mat resultat;
        int tailleX, tailleY;
        std::vector<Offset> offsets;
        int *posXPixels, *posYPixels;
};


#endif // IMAGE_HPP_INCLUDED
