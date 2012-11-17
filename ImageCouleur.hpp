#ifndef IMAGECOULEUR_HPP_INCLUDED
#define IMAGECOULEUR_HPP_INCLUDED

#include "Image.hpp"

class ImageCouleur {
    public:
        ImageCouleur(const std::string& cheminFichier, const std::string& cheminMasque);
        ~ImageCouleur();
        void complete(int taillePatch, int tau);
        void afficheImage(const std::string& fenetre) const ;
        void afficheMasque(const std::string& fenetre) const ;
        void afficheOffsets() const ;
        void afficheResultat(const std::string& fenetre) const ;
        Image* getImageCalculs() const;

    private:
        cv::Mat pixels;
        cv::Mat masque;
        cv::Mat resultat;
        Image* image;
        std::vector<Offset> offsets;
        int tailleY;
        int tailleX;
};


#endif // IMAGECOULEUR_HPP_INCLUDED
