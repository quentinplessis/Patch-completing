#ifndef RENDU3D_HPP_INCLUDED
#define RENDU3D_HPP_INCLUDED

#include <string>
#include <windows.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.hpp"

class Rendu3D {
    public:
        Rendu3D(Image* img, const char* nomFenetre);
        void rendu();
        void changeTaille(GLsizei w, GLsizei h);

    private:
        Image* image;
        GLfloat sideRange;

        void repere();

        void initialSetup();
        void setupCallbackRendu();
        void setupCallbackTaille();
};


#endif // RENDU3D_HPP_INCLUDED
