#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <math.h>

unsigned int COLORMAP[3] = {6, 4, 8};

void WriteXY(char* str, unsigned int Xpos, unsigned int Ypos, unsigned int color);
void Clear(int Ypos);
void CGA(void);
void EGA(void);
void DrawPoint(int x, int y,  unsigned char color);
void DrawLine(int Xpos, int Ypos, int Xpos1, int Ypos1, unsigned char color);
void DrawCircle(int Xpos, int Ypos, int radius, unsigned char color);
void SetBackGroundColor(unsigned char color);
void SetVideoMode(int color);

void main(void)
{
    char input;
	while (1)
	{
        SetVideoMode(0x03);
        Clear(0);
	    WriteXY("Работа видеоподсистемы", 0, 1, COLORMAP[2]);
        WriteXY("Текстовый режим (AH = 0x00, AL = 0x03) (80х25) ", 0, 3, COLORMAP[1]);
	    WriteXY("[1] CGA (AH = 0x00, AL = 0x0D) (640x200) ", 0, 4, COLORMAP[0]);
        WriteXY("[2] EGA (AH = 0x00, AL = 0x04) (640x350) ", 0, 5, COLORMAP[0]);
	    WriteXY("[3] Выход ", 0, 6, COLORMAP[0]);
		input = getch();
		switch (input) {
		case '1': 
            CGA(); 
            break;
		case '2': 
            EGA(); 
            break;
		case '3': 
            clrscr();
            return 0;
		default: 
            break;
		}
	}
}

void WriteXY(char* str, unsigned int Xpos, unsigned int Ypos, unsigned int color)
{
    union REGS inregs, outregs;
	int i;

    for (i = 0; str[i] != '\0'; i = i + 1)
	{
        inregs.h.ah = 0x02;
        inregs.h.bh = 0;
        inregs.h.dh = Ypos;
        inregs.h.dl = Xpos + i;
        int86(0x10, &inregs, &outregs);

		inregs.h.ah = 0x09;
        inregs.h.al = (int)str[i];
        inregs.h.bl = color;
        inregs.x.cx = 1;
        int86(0x10, &inregs, &outregs);
	}

    inregs.h.ah = 0x02;
    inregs.h.bh = 0;
    inregs.h.dh = 0;
    inregs.h.dl = 0;
    int86(0x10, &inregs, &outregs);
}

void Clear(int Ypos)
{
    union REGS inregs, outregs;
	int i, j;

	for (i = Ypos; i < 25; i = i + 1)
	{
		for (j = 0; j < 80; j = j + 1)
		{
		inregs.h.ah = 0x02;
        inregs.h.bh = 0;
        inregs.h.dh = i;
        inregs.h.dl = 0 + j;
        int86(0x10, &inregs, &outregs);

		inregs.h.ah = 0x09;
        inregs.h.al = ' ';
        inregs.h.bl = 7;
        inregs.x.cx = 1;
        int86(0x10, &inregs, &outregs);
		}
	}
    inregs.h.ah = 0x02;
    inregs.h.bh = 0;
    inregs.h.dh = 0;
    inregs.h.dl = 0;
    int86(0x10, &inregs, &outregs);
}

void SetBackGroundColor(unsigned char color)
{
    union REGS inregs, outregs;

    inregs.h.ah = 0x0B;
	inregs.h.bh = 0x00;
	inregs.h.bl = color;
	int86(0x10, &inregs, &outregs);
}

void SetVideoMode(int mode)
{
    union REGS inregs, outregs;

    inregs.h.ah = 0x00;
	inregs.h.al = mode;
	int86(0x10, &inregs, &outregs);
}

void DrawPoint(int x, int y, unsigned char color)
{
    union REGS inregs, outregs;
    
    inregs.h.ah = 0x0C;
	inregs.h.al = (unsigned char)(color);
    inregs.x.cx = x;
	inregs.x.dx = y;
	int86(0x10, &inregs, &outregs);
}

void DrawLine(int Xpos, int Ypos, int Xpos1, int Ypos1, unsigned char color)
{
    int deltaX = abs(Xpos1 - Xpos);
    int deltaY = abs(Ypos1 - Ypos);

    const int signX = Xpos < Xpos1 ? 1 : -1;
    const int signY = Ypos < Ypos1 ? 1 : -1;
    int error = deltaX - deltaY;
    int error2;

    DrawPoint(Xpos1, Ypos1, color);

    while(Xpos != Xpos1 || Ypos != Ypos1) 
    {
        DrawPoint(Xpos, Ypos, color);
        error2 = error * 2;
        if(error2 > -deltaY) 
        {
            error -= deltaY;
            Xpos += signX;
        }
        if(error2 < deltaX) 
        {
            error += deltaX;
            Ypos += signY;
        }
    }
}

void DrawCircle(int Xpos, int Ypos, int radius,  unsigned char color)
{
	int x = 0;
	int y = radius;

	int delta = -99;
	int error = 0;

	while(y >= x) 
    {
        DrawPoint(Xpos + x, Ypos + y, color);
		DrawPoint(Xpos + x, Ypos - y, color);
        DrawPoint(Xpos - x, Ypos + y, color);
        DrawPoint(Xpos - x, Ypos - y, color);
        DrawPoint(Xpos + y, Ypos + x, color);
        DrawPoint(Xpos + y, Ypos - x, color);
        DrawPoint(Xpos - y, Ypos + x, color);
        DrawPoint(Xpos - y, Ypos - x, color);
		
		error = 2 * (delta + y) - 1;
		if((delta < 0) && (error <= 0))
		{
			delta += 2 * ++x + 1;
			continue;
		}
		if((delta >0) && (error > 0))
		{
			delta -= 2 * --y + 1;
			continue;
		}
		delta += 2 * (++x - --y);
	}
}

void CGA(void)
{
    union REGS inregs, outregs;

	unsigned char color = 4, backColor = 1;

	SetVideoMode(0x04);

	SetBackGroundColor(backColor);

	while (!kbhit()) {

        WriteXY("Нажмите любую клавишу, чтобы выйти", 1, 23, color);

		DrawCircle(100, 100, 80, 3);

		delay(500);
		color++;
		if(color == 9) color = 1;
        if(color | backColor) color ++;
	}
}

void EGA(void) 
{ 
    union REGS inregs, outregs;
	unsigned char color = 12, backColor = 3;

	SetVideoMode(0x0D);

    SetBackGroundColor(backColor);

    while (!kbhit()) {

        WriteXY("Нажмите любую клавишу, чтобы выйти", 1, 23, color);

		DrawCircle(60, 60, 50, 3);
        DrawLine(0, 0, 120, 120, 3);
		delay(200);

        color++;
        if(color == 15) color = 1;
        if(color | backColor) color ++;
	}
}