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

        SG_setPixel(x, y, color);

        while (x < xEnd) {
            x++;
            if (p < 0) {
                p += twoDy;
            } else {
                y += (dy < 0) ? -1 : 1;
                p += twoDyDx;
            }
            SG_setPixel(x, y, color);
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

        SG_setPixel(x, y, color);

        while (y < yEnd) {
            y++;
            if (p < 0) {
                p += twoDx;
            } else {
                x += (dx < 0) ? -1 : 1;
                p += twoDxDy;
            }
            SG_setPixel(x, y, color);
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


unsigned char color(unsigned char color, _Bool bgmode) {

    return bgmode ? SG_BgColor(color) : color;
}

void draw_house(unsigned char x_offset, unsigned char y_offset, _Bool extra_color, _Bool bgmode) {

    line(50+x_offset,    100+y_offset,   100+x_offset,   100+y_offset,   color((extra_color ? 2 : NO_COLOR_UPDATE), bgmode)); 
    line(100+x_offset,   100+y_offset,   100+x_offset,   50+y_offset,    color((extra_color ? 3 : NO_COLOR_UPDATE), bgmode));  
    line(100+x_offset,   50+y_offset,    50+x_offset,    50+y_offset,    color((extra_color ? 4 : NO_COLOR_UPDATE), bgmode));  
    line(50+x_offset,    50+y_offset,    75+x_offset,    25+y_offset,    color((extra_color ? 5 : NO_COLOR_UPDATE), bgmode));    
    line(75+x_offset,    25+y_offset,    100+x_offset,   50+y_offset,    color((extra_color ? 6 : NO_COLOR_UPDATE), bgmode));
    line(100+x_offset,   50+y_offset,    50+x_offset,    100+y_offset,   color((extra_color ? 7 : NO_COLOR_UPDATE), bgmode));
    line(50+x_offset,    100+y_offset,   50+x_offset,    50+y_offset,    color((extra_color ? 8 : NO_COLOR_UPDATE), bgmode));
    line(50+x_offset,    50+y_offset,    100+x_offset,   100+y_offset,   color((extra_color ? 9 : NO_COLOR_UPDATE), bgmode));
}

void main(void) {
    SG_initBMPmode(SG_COLOR_WHITE, SG_COLOR_LIGHT_GREEN);
    SG_displayOn();
    
    draw_house(0,   0,  1, 0);
    draw_house(75, 75,  0, 0);
    draw_house(75,  0,  1, 1);
    while(1) SG_waitForVBlank();
}