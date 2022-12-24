#ifndef MINECRAFTALTER_CUBE_H
#define MINECRAFTALTER_CUBE_H
#include "Quad.h"

namespace MinecraftAlter
{
    class Cube {
    public:
        unsigned int VAOs[6] = {0};
        unsigned int VBOs[6] = {0};
        Quad XPos;
        Quad XNeg;
        Quad YPos;
        Quad YNeg;
        Quad ZPos;
        Quad ZNeg;
        Cube() {
            XPos = Quad{VAOs[xPos], VBOs[xPos], xPos};
            XNeg = Quad{VAOs[xNeg], VBOs[xNeg], xNeg};
            YPos = Quad{VAOs[yPos], VBOs[yPos], yPos};
            YNeg = Quad{VAOs[yNeg], VBOs[yNeg], yNeg};
            ZPos = Quad{VAOs[zPos], VBOs[zPos], zPos};
            ZNeg = Quad{VAOs[zNeg], VBOs[zNeg], zNeg};
        }
        Cube(bool cornerAtOrigin) {
            XPos = Quad{VAOs[xPos], VBOs[xPos], xPos};
            XNeg = Quad{VAOs[xNeg], VBOs[xNeg], xNeg};
            YPos = Quad{VAOs[yPos], VBOs[yPos], yPos};
            YNeg = Quad{VAOs[yNeg], VBOs[yNeg], yNeg};
            ZPos = Quad{VAOs[zPos], VBOs[zPos], zPos};
            ZNeg = Quad{VAOs[zNeg], VBOs[zNeg], zNeg};
            if (cornerAtOrigin) {
                XPos.removeNegPosition();
                XNeg.removeNegPosition();
                YPos.removeNegPosition();
                YNeg.removeNegPosition();
                ZPos.removeNegPosition();
                ZNeg.removeNegPosition();
            } else {
                XPos.scale(0.5);
                XNeg.scale(0.5);
                YPos.scale(0.5);
                YNeg.scale(0.5);
                ZPos.scale(0.5);
                ZNeg.scale(0.5);
            }
        }
    };
}

#endif
