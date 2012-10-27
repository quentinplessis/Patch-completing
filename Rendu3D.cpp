#include "Rendu3D.hpp"
#include "Offset.hpp"

using namespace std;

Rendu3D* g_instance;

Rendu3D::Rendu3D(Image* img, const char* nomFenetre) {
    image = img;
    sideRange = image->getTaille();

    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutCreateWindow(nomFenetre);

    setupCallbackRendu();
    setupCallbackTaille();
    initialSetup();
    glutMainLoop();
}

void Rendu3D::changeTaille(GLsizei w, GLsizei h) {
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-sideRange, sideRange, -sideRange, sideRange, -sideRange, sideRange);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Rendu3D::rendu() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
    glRotatef(70,1,0,0); // 70
    glRotatef(-15, 0, 0, 1); // -10
    repere();

    vector<Offset> offsets = image->getOffsets();
    vector<Offset>::iterator i;
    int nMax = 0;
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
        if (i->getN() > nMax)
            nMax = i->getN();
    }

    glBegin(GL_LINES);
    for (i = offsets.begin() ; i != offsets.end() ; ++i) {
        float proportion = ((float) i->getN())/nMax;
        glColor3f(proportion,0,1-proportion);
        glVertex3f(i->getX(), i->getY(), 0);
        glVertex3f(i->getX(), i->getY(), -(i->getN()));
    }
    glEnd();

    glPopMatrix();
    glFlush();
}

void Rendu3D::repere() {
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_LINES);
    glVertex3f(-0,0,0);
    glVertex3f(sideRange, 0, 0);
    //glColor3f(1,0,0);
    glVertex3f(0,-0,0);
    glVertex3f(0, sideRange, 0);
    //glColor3f(0,1,0);
    glVertex3f(0,0,-0);
    glVertex3f(0, 0, -sideRange);
    //glColor3f(0,0,0);
    glVertex3f(0,-sideRange,0);
    glVertex3f(0, 0, 0);
    glVertex3f(-sideRange,0,0);
    glVertex3f(0, 0, 0);
    glEnd();
}


void Rendu3D::initialSetup() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
}


extern "C"
void renderer() {
  g_instance->rendu();
}

void changeSize(GLsizei w, GLsizei h) {
  g_instance->changeTaille(w, h);
}

void Rendu3D::setupCallbackRendu() {
  ::g_instance = this;
  ::glutDisplayFunc(::renderer);
}

void Rendu3D::setupCallbackTaille() {
  ::g_instance = this;
  ::glutReshapeFunc(::changeSize);
}

