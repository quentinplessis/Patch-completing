#ifndef OFFSET_HPP_INCLUDED
#define OFFSET_HPP_INCLUDED

class Offset {
    public:
        Offset();
        Offset(int x, int y);
        bool augmenteSiEgal(int x, int y);
        void affiche() const;

    private:
        int x, y;
        int nombre;
};

#endif // OFFSET_HPP_INCLUDED
