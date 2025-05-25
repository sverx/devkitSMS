#include "SGlib.h"

void bresenham(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char color) {
    int dx = x2 - x1;
    int dy = y2 - y1;

    int x = x1;
    int y = y1;

    int dx1 = (dx < 0) ? -dx : dx;
    int dy1 = (dy < 0) ? -dy : dy;
    int px = (dy1 << 1) - dx1;
    int py = (dx1 << 1) - dy1;

    int xEnd, yEnd;

    if (dy1 <= dx1) {
        int twoDy = dy1 << 1;
        int twoDyDx = (dy1 - dx1) << 1;
        int p = (dy1 << 1) - dx1;

        if (x1 > x2) {
            x = x2;
            y = y2;
            xEnd = x1;
        } else {
            x = x1;
            y = y1;
            xEnd = x2;
        }

        SG_putPixel(x, y, 1, color);

        while (x < xEnd) {
            x++;
            if (p < 0) {
                p += twoDy;
            } else {
                y += (dy < 0) ? -1 : 1;
                p += twoDyDx;
            }
            SG_putPixel(x, y, 1, color);
        }
    } else {
        int twoDx = dx1 << 1;
        int twoDxDy = (dx1 - dy1) << 1;
        int p = (dx1 << 1) - dy1;

        if (y1 > y2) {
            x = x2;
            y = y2;
            yEnd = y1;
        } else {
            x = x1;
            y = y1;
            yEnd = y2;
        }

        SG_putPixel(x, y, 1, color);

        while (y < yEnd) {
            y++;
            if (p < 0) {
                p += twoDx;
            } else {
                x += (dx < 0) ? -1 : 1;
                p += twoDxDy;
            }
            SG_putPixel(x, y, 1, color);
        }
    }
}

void line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned color) {
  
  if (x1 > x2) {
    unsigned char temp;
    temp =x1;
    x1 = x2;
    x2 = temp;
    temp = y1;
    y1 = y2;
    y2 = temp;
  }
  bresenham(x1, y1, x2, y2, color);
}

void main(void) {
    char blank[8] = { 0, 0, 0, 0,
                    0, 0, 0, 0};
    
    SG_displayOn();
    for(unsigned int i=0; i < 256 * 3; ++i)
    {
      SG_loadTilePatterns(blank, i, 8);
      SG_loadTileColours(blank, i, 8);
    }

    unsigned int tileno=0;
    for(unsigned char y=0; y < (192 >> 3); ++y)
      for(unsigned char x=0; x < (256 >> 3); ++x)
        SG_setTileatXY(x, y, tileno++);

    line(100, 100, 150, 100, 2); 
    line(150, 100, 150, 50,  3);  
    line(150, 50, 100, 50,   4);  
    line(100, 50, 125, 25,   5);    
    line(125, 25, 150, 50,   6);
    line(150, 50, 100, 100,  7);
    line(100, 100, 100, 50,  8);
    line(100, 50, 150, 100,  9);
}

