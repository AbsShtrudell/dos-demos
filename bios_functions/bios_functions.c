#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdlib.h>

unsigned int NameXpos = 0;
unsigned int NameYpos = 0;
unsigned int HeaderXpos = 0;
unsigned int HeaderYpos = 3;
unsigned int COLORMAP[3] = {6, 4, 8};

volatile int close = 0;
union REGS rg;
union REGS cl;

typedef struct _HDWCFG
{
	unsigned HddPresent : 1; //0 - Дисковод
	unsigned NpuPresent : 1; //1 - Математический сопроцессор
	unsigned AmountOfRAM : 2; //2-3 - Размер ОЗУ
	unsigned VideoMode : 2; //4-5 - Активный видеорежим
	unsigned NumberOfFdd : 2; //6-7 - Число обнаруженных НГМД
	unsigned DmaPresent : 1; //8 - Наличие контроллера DMA
	unsigned NumberOfCom : 3; //9-11 - Число COM-портов
	unsigned GamePresent : 1; //12 - Игровой адаптер
	unsigned JrComPresent : 1; //13 - Резерв
	unsigned NumberOfLpt : 2; //14-15 - Число LPT-портов
} HDWCFG;

HDWCFG HdwCfg;

char videoModes[4][40] = { 	
	"Не используется", 
	"ЧБ 40х25 c цветной платой", 
	"ЧБ 80х25 c цветной платой",
	"ЧБ 80х25 c монохромной платой" 
	};

void MainMenu(void); //главное менб
void Name(void); //выводит данные по лабораторной
void Header(char* str);

void WriteXY(char* str, unsigned int Xpos, unsigned int Ypos, unsigned int color); //выводит текст на x,y
void Clear(unsigned int Xpos, unsigned int Ypos, unsigned int count); // чистит экран
int Getch(void);
int IsKeyPressed(void);

void WriteConfig(void);
void Input(void);
void InChar(void);
void InStr(void);
void InCharWaiting(void);
void InStrDelay(void);
int InDigit(int Xpos, int Ypos, char* limits, int left, int right);
void TimeOperations(void);
void ShowTime(void);
void SetTimeValue(void);
void SetDateValue(void);
char* DateInputField(char* str, unsigned int Xpos, unsigned int Ypos, char delim);

char* B2Bin(unsigned char bt, char* str);
char* B2H2(unsigned char bt, char* str);
unsigned char B4to1H(unsigned char bt);
char* IntToStr(int n);// Функция преобразования числа в строку
int bcdtoint(char* k);//Переводит число, заданное в двоично-десятичном коде, в тип int, 2бита
int bcdtoint2(char* k); //Переводит число, заданное в двоично-десятичном коде, в тип int, 4 бита
void itodec(int i, char* c, int j); //Перевод двухзначного числа в десятичную систему счисления
void itodec2(int i, char* c, int j);//Перевод четрыёх значного числа в десятичную систему счисления
void inttobcd(int i, char* c);
int AtoI(char* str); // E,HFNM

void main(void)
{
	int key;
	Clear(0, 0, 25 * 80);
	Name();
	while(!close)
	{
		MainMenu();
		switch(Getch())
		{
		case 5:
			close = 1;
			break;
		case 2:
			WriteConfig();
			break;
		case 3:
            Input();
			break;
		case 4:
			TimeOperations();
			break;
		default:
			break;
		}
	}
	clrscr();
	return;
}

void MainMenu(void)
{
	Clear(0, 2, 23 * 80);
	Header("ГЛАВНОЕ МЕНЮ");
	WriteXY("[1] Конфигурация ПК (INT 11h, INT 12h)", 0, 4, COLORMAP[0]);
	WriteXY("[2] Ввод с клавиатуры (INT 16h)", 0, 5, COLORMAP[0]);
	WriteXY("[3] Дата и время (INT 1Ah)", 0, 6, COLORMAP[0]);
	WriteXY("[4] Выход", 0, 7, COLORMAP[1]);
}

void Name(void)
{
	WriteXY("Функции BIOS", NameXpos, NameYpos + 1, COLORMAP[2]);
}

void Header(char* str)
{
	WriteXY(str, HeaderXpos, HeaderYpos, COLORMAP[1]);
}

void WriteXY(char* str, unsigned int Xpos, unsigned int Ypos, unsigned int color)
{
	int i = 0;
	char far* StartMemPos = (char* far)0xb8000000;
	char far* MemPos;

	for (i = 0; str[i] != '\0'; i++)
	{
		MemPos = StartMemPos + Ypos * 160 + Xpos * 2;
		*MemPos = str[i];
		Xpos++;
		MemPos++;
		*MemPos = color;
	}
}

void Clear(unsigned int Xpos, unsigned int Ypos, unsigned int count)
{
	int i;
	char far* StartMemPos = (char far*)0xb8000000;
	char far* MemPos;
	for (i = 0; i < count; i++)
	{
		MemPos = StartMemPos + Ypos * 160 + Xpos * 2;
		*MemPos = ' ';
		MemPos++;
		*MemPos = 7;
		Xpos++;
	}
}

int Getch(void)
{
	rg.h.ah = 0;//Помещаем в регистр AH значение 0
	int86(0x16, &rg, &rg);//Выполняем прерывание 16H
	return rg.h.ah;
}

int IsKeyPressed(void)
{
	rg.h.ah = 0x11;//Помещаем в регистр AH значение 0
	int86(0x16, &rg, &rg);//Выполняем прерывание 16H
	return rg.h.ah;
}

void WriteConfig(void)
{
	char buf[9];
	char c[2];
	unsigned uword = 0;
	
	Clear(0, 2, 23 * 80);
	Header("КОНФИГУРАЦИЯ ПК");

	rg.h.ah = 0;
	int86(0x11, &rg, &rg);

	WriteXY("[Int 11h]", 0, 4, COLORMAP[1]);
	WriteXY("Регистр состояния конфигурации", 0, 5, COLORMAP[0]);
	WriteXY(B2Bin(rg.h.ah, buf), 32, 5, COLORMAP[1]);
	WriteXY(".", 40, 5, COLORMAP[1]);
	WriteXY(B2Bin(rg.h.al, buf), 41, 5, COLORMAP[1]);

	uword = (unsigned int)rg.x.ax;
	memcpy(&HdwCfg, &uword, 2);

	if (HdwCfg.HddPresent)
		WriteXY("Накопитель на магнитном жестком диске установлен", 0, 6, COLORMAP[0]);
	else
		WriteXY("Накопитель на магнитном жестком диске не установлен", 0, 6, COLORMAP[0]);
	if (HdwCfg.NpuPresent)
		WriteXY("Арифметический процессор установлен", 0, 7, COLORMAP[0]);
	else
		WriteXY("Арифметический процессор не установлен", 0, 7, COLORMAP[0]);
	c[0] = HdwCfg.AmountOfRAM + '0';
	WriteXY("Количество банков оперативной памяти на системной плате:", 0, 8, COLORMAP[0]);
	WriteXY(c, 59, 8, COLORMAP[1]);
	WriteXY("         ", 60, 8, COLORMAP[0]);
	WriteXY("Начальный режим видеоадаптера:", 0, 9, COLORMAP[0]);
	WriteXY(videoModes[HdwCfg.VideoMode], 32, 9, COLORMAP[1]);
	c[0] = (HdwCfg.NumberOfFdd + 1) + '0';
	WriteXY("Количество установленных накопителей на гибких магнитных дисках:", 0, 10,COLORMAP[0]);
	WriteXY(c, 66, 10, COLORMAP[1]);
	WriteXY("         ", 67, 10, COLORMAP[0]);
	if (HdwCfg.DmaPresent)
		WriteXY("Контроллер прямого доступа к памяти DMA установлен", 0, 11, COLORMAP[0]);
	else
		WriteXY("Контроллер прямого доступа к памяти DMA не установлен", 0, 11,COLORMAP[0]);
	c[0] = HdwCfg.NumberOfCom + '0';
	WriteXY("Количество установленных асинхронных последвательных портов:", 0, 12, COLORMAP[0]);
	WriteXY(c, 67, 12, COLORMAP[1]);
	WriteXY("         ", 68, 12, COLORMAP[0]);
	if (HdwCfg.GamePresent)
		WriteXY("Игровой порт установлен", 0, 13, COLORMAP[0]);
	WriteXY("Игровой порт не установлен", 0, 13, COLORMAP[0]);
	if (HdwCfg.JrComPresent)
		WriteXY("Последовательный порт установлен", 0, 14, COLORMAP[0]);
	else
		WriteXY("Последовательный порт не установлен", 0, 14, COLORMAP[0]);
	c[0] = HdwCfg.NumberOfLpt + '0';
	WriteXY("Количество установленных параллельных адаптеров:", 0, 15, COLORMAP[0]);
	WriteXY(c, 50, 15, COLORMAP[1]);
	WriteXY("         ", 51, 15, COLORMAP[0]);

	rg.h.ah = 0x88;
	int86(0x12, &rg, &rg);
	WriteXY("[Int 12h]", 0, 17, COLORMAP[1]);
	WriteXY("Объем оперативной памяти компьютера:", 0, 18, COLORMAP[0]);
	WriteXY(IntToStr(rg.x.ax),47,18, COLORMAP[1]);
	WriteXY("KБайт", 51, 18, COLORMAP[1]);
	WriteXY("Нажмите любую клавишу чтобы выйти", 0, 20, COLORMAP[1]);
	Getch();
	return;
}

void Input(void)
{
    int back = 0;
    while(!back)
    {
        Clear(0, 2, 23 * 80);
	    Header("ВВОД С КЛАВИАТУРЫ");
        WriteXY("[1] Ввод символа", 0, 4, COLORMAP[0]);
	    WriteXY("[2] Ввод строки", 0, 5, COLORMAP[0]);
	    WriteXY("[3] Ввод символа с ожиданием", 0, 6, COLORMAP[0]);
	    WriteXY("[4] Ввод строки с ожиданием", 0, 7, COLORMAP[0]);
        WriteXY("[5] Вернуться в меню", 0, 8, COLORMAP[1]);

        switch (Getch()) 
        {
	    case 2:
		    InChar();
		    break;
	    case 3:
		    InStr();
		    break;
	    case 4:
            InCharWaiting();
		    break;
	    case 5:
		    InStrDelay();
		    break;
	    case 6:
            back = 1;
		    break;
        default:
			break;
        }
    }
}

void InChar(void)
{
	char c[2];
    int input;
	Clear(0, 2, 23 * 80);
    Header("ВВОД СИМВОЛА");

	c[1] = '\0';
	WriteXY("Введите символ", 0, 4, COLORMAP[0]);
    WriteXY("----------------- ", 0, 5, COLORMAP[2]);
	input = Getch();
    c[0] = (char)rg.h.al;
	WriteXY("Символ ", 0, 6, COLORMAP[0]);
	WriteXY(c, 17, 6, COLORMAP[1]);
	WriteXY("Скан-код ", 0, 7, COLORMAP[0]);
	WriteXY(IntToStr(input), 17, 7, COLORMAP[1]);
	WriteXY("Нажмите любую клавишу чтобы выйти", 0, 9, COLORMAP[1]);
	Getch();
}

void InStr(void)
{
    char str[80];
	int i = 0;
    int input;
    int back = 0;
	Clear(0, 2, 23 * 80);
    Header("ВВОД СТРОКИ");
	WriteXY("Введите строку", 0, 4, COLORMAP[0]);
    WriteXY("----------------------", 0, 5, COLORMAP[2]);
    WriteXY("Нажмите ENTER чтобы выйти", 0, 8, COLORMAP[1]);
	while (!back)
	{
		input = Getch();
		if (input == 14 && i > 0)//backspace
		{
			i--;
			str[i] = '\0';
			Clear(i, 6, 1);
		}
		else if (input == 28)
		{
			back = 1;
		}
		else if (i < 80 && input != 28)
		{
			str[i] = (char)rg.h.al;
			str[i + 1] = '\0';
			WriteXY(str, 0, 6, 15);
			i++;
		}
	}
}

void InCharWaiting(void)
{
	int input, inp = 0;
    int code;
    int back = 0;
	char c[2];
    c[1] = '\0';

	Clear(0, 2, 23 * 80);
    Header("ВВОД СИМВОЛА С ЗАДЕРДКОЙ");
	WriteXY("Введите символ", 0, 4, COLORMAP[0]);
    WriteXY("----------------------", 0, 5, COLORMAP[2]);
    WriteXY("Символ ", 0, 6, COLORMAP[0]);
    WriteXY("Скан-код", 0, 7, COLORMAP[0]);
    WriteXY("Нажмите ENTER чтобы выйти", 0, 9, COLORMAP[1]);
	while (!back)
	{
		input = Getch();
		if (input == 14)//backspace
		{
			Clear(17, 6, 1);
            Clear(17, 7, 2);
		}
		else if (input == 28) {
			back = 1;
		}
		else {
			c[0] = (char)rg.h.al;
			WriteXY(c, 17, 6, COLORMAP[1]);
            WriteXY(IntToStr(input), 17, 7, COLORMAP[1]);
		}
	}
}

void InStrDelay(void)
{
    char str[80];
	int i = 0;
    int input;
    int back = 0;
    int del;

	Clear(0, 2, 23 * 80);
    Header("ВВОД СТРОКИ С ЗАДЕРДКОЙ");

    WriteXY("Введите задержку", 0, 4, COLORMAP[0]);
	del = InDigit(0 , 5, "[1500-10000]", 1500, 10000);

    Clear(0, 2, 23 * 80);

    Header("ВВОД СТРОКИ С ЗАДЕРДКОЙ");
	WriteXY("Введите строку", 0, 4, COLORMAP[0]);
    WriteXY("----------------------", 0, 5, COLORMAP[2]);
    WriteXY("Нажмите ENTER чтобы выйти", 0, 8, COLORMAP[1]);
	while (!back)
	{
		input = Getch();
		if (input == 14 && i > 0)//backspace
		{
			i--;
			str[i] = '\0';
			Clear(i, 6, 1);
		}
		else if (input == 28)
		{
			back = 1;
		}
		else if (i < 80 && input != 28)
		{
			str[i] = (char)rg.h.al;
			str[i + 1] = '\0';
            delay(del);
			WriteXY(str, 0, 6, 15);
			i++;
		}
	}
}

int InDigit(int Xpos, int Ypos, char* limits, int left, int right)
{
    int inp;
	char input[80];
	int i = 0, k = 0, digit = 0;
	WriteXY("Введите число ", Xpos, Ypos, COLORMAP[0]);
	WriteXY(limits, Xpos + 14, Ypos, COLORMAP[1]);
	Ypos++;
	do
	{
		digit = 0;
		i = 0;
		input[i] = '\0';
		Clear(0, Ypos, 80);
		while (1)
		{
			inp = Getch();
			if (inp == 14 && i > 0)//backspace
			{
				i--;
				input[i] = '\0';
				Clear(i, Ypos, 1);
			}
			else if (inp == 28 && i > 0)//enter
			{
				break;
			}
			else if (i < 80 && (inp >= 2 && inp <= 11))
			{
				input[i] = (char)rg.h.al;
				input[i + 1] = '\0';
				WriteXY(input, 0, Ypos, COLORMAP[1]);
				i++;
			}
		}
		for (k = 0; k < i; k++)//преобразование в int
		{
			digit = digit * 10 + (input[k] - '0');
		}
	} while (digit < left || digit > right);
	return digit;
}

void TimeOperations(void)
{
	int k = 0, j = 0;
	int back = 0;
	Clear(0, 2, 23 * 80);

	while(!back)
	{
		Header("ДАТА И ВРЕМЯ");
    	WriteXY("[1] Установить дату", 0, 4, COLORMAP[0]);
		WriteXY("[2] Установить время", 0, 5, COLORMAP[0]);
 		WriteXY("[3] Вернуться в меню", 0, 6, COLORMAP[1]);
		ShowTime();
		
		if(kbhit()){
		switch (Getch())
		{
		case 2:
			SetDateValue();
			break;
		case 3:
			SetTimeValue();
			break;
		case 4:
			back = 1;
			break;
		}
		}
	}
}

void ShowTime(void)
{
	char str[20];
	int year = 0, month = 0, day = 0, secs = 0, mins = 0, hrs = 0;

	cl.h.ah = 0x04;//Считывание даты с часов реального времени
	int86(0x1a, &cl, &cl);
	day = bcdtoint(&(cl.h.dl));
	month = bcdtoint(&(cl.h.dh));
	year = bcdtoint2(&(cl.h.cl));

	cl.h.ah = 0x02;
	int86(0x1a, &cl, &cl);
	hrs = bcdtoint(&(cl.h.ch));
	mins = bcdtoint(&(cl.h.cl));
	secs = bcdtoint(&(cl.h.dh));

	itodec2(year, str, 0);
	str[4] = '.';
	itodec(month, str, 5);
	str[7] = '.';
	itodec(day, str, 8);
	str[10] = '\0';
	WriteXY("Текущая дата:", 40, 4, COLORMAP[2]);
	WriteXY(str, 55, 4, COLORMAP[0]);

	itodec(hrs, str, 0);
	str[2] = ':';
	itodec(mins, str, 3);
	str[5] = ':';
	itodec(secs, str, 6);
	str[8] = '\0';
	WriteXY("Текущее время:", 40, 5, COLORMAP[2]);
	WriteXY(str, 56, 5, COLORMAP[0]);
}

void SetTimeValue()
{
	char str[9];
	int values[3];

	Clear(0, 2, 23 * 80);
	Header("УСТАНОВКА ВРЕМЕНИ");

	while (1)
	{
		WriteXY("Введите время в формате ЧЧ:ММ:СС", 0, 4, COLORMAP[0]);
		DateInputField(str, 0, 5, ':');

		if (str[0] == '\0') 
			return;

		str[2] = '\0';
		str[5] = '\0';

		values[0] = AtoI(str);
		values[1] = AtoI(str + 3);
		values[2] = AtoI(str + 6);

		if (values[0] < 0 || values[0] > 23)
		{
			WriteXY("Неверное значение часа!", 0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else if (values[1] < 0 || values[1] > 59)
		{
			WriteXY("Неверное значение минут!", 0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else if (values[2] < 0 || values[2] > 59)
		{
			WriteXY("Неверное значение секунд!", 0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else
			break;
	}

	cl.h.ah = 0x03;
	inttobcd(values[0], &(cl.h.ch));//часы
	inttobcd(values[1], &(cl.h.cl));//минуты
	inttobcd(values[2], &(cl.h.dh));//секунды
	int86(0x1a, &cl, &cl);

	Clear(0, 2, 23 * 80);
}

void SetDateValue()
{
	char str[9];
	int values[3];
	int DaysInMonth[12] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

	Clear(0, 2, 23 * 80);
	Header("УСТАНОВКА ДАТЫ");

	while (1)
	{
		WriteXY("Введите дату в формате ГГГГ.ММ.ДД: ", 0, 4, COLORMAP[0]);
		WriteXY("20", 0, 5, COLORMAP[2]);
		DateInputField(str, 2, 5, '.');
		if (str[0] == '\0') 
			return;

		str[2] = '\0';
		str[5] = '\0';

		values[0] = AtoI(str);

		if (values[0] % 4 == 0) DaysInMonth[1] = 29;

		values[1] = AtoI(str + 3);
		values[2] = AtoI(str + 6);

		if (values[0] < 0 || values[0] > 30)
		{
			WriteXY("Неверное значение года!",  0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else if (values[1] < 1 || values[1] > 12)
		{
			WriteXY("Неверное значение месяца!",  0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else if (values[2] < 1 || values[2] > DaysInMonth[values[1] - 1])
		{
			WriteXY("Неверное значение дня месяца!",  0, 6, COLORMAP[1]);
			delay(1000);
			Clear(0, 6, 79);
		}
		else
			break;

		DaysInMonth[1] = 28;
	}

	cl.h.ah = 0x05;
	inttobcd(values[2], &(cl.h.dl));//день
	inttobcd(values[1], &(cl.h.dh));//месяц
	inttobcd(values[0], &(cl.h.cl));//год
	int86(0x1a, &cl, &cl);

	Clear(0, 2, 23 * 80);
}

char* DateInputField(char* str, unsigned int Xpos, unsigned int Ypos, char delim)
{
	int i = 0;
	char c;
	char delimStr[2];
	delimStr[0] = delim;
	delimStr[1] = '\0';

	str[0] = '\0'; str[1] = '\0'; str[2] = '\0'; str[3] = '\0'; str[4] = '\0'; str[5] = '\0'; str[6] = '\0'; str[7] = '\0';

	WriteXY("__ __ __", Xpos, Ypos, COLORMAP[1]);
	WriteXY(delimStr, Xpos + 2, Ypos, COLORMAP[1]);
	WriteXY(delimStr, Xpos + 5, Ypos, COLORMAP[1]);

	while (1)
	{
		c = Getch(); // зaпоминaние вводa
		if (c == 1) // если ESC
		{
			Clear(0, Ypos, 79); // очисткa поля
			str[0] = '\0';
			return str; // устaновлен флaг выходa

		}
		else if (c == 14) // если BACKSPACE
		{
			if (i > 0) // если строкa не нулевaя
			{
				i--; // смещение нa позицию влево
				
				if (i == 2 || i == 5)
				{
					WriteXY(delimStr, Xpos  + i, Ypos, COLORMAP[1]);
					i--;
				}

				WriteXY("_", Xpos  + i, Ypos, COLORMAP[1]); // стирaние последнего символa
			}
			str[i] = '\0'; // обрезaние строки

		}
		else if (c == 28 && str[0] != '\0' && str[1] != '\0' && str[2] != '\0' && str[3] != '\0'
			&& str[4] != '\0' && str[5] != '\0' && str[6] != '\0' && str[7] != '\0') // если ENTER и строкa зaполненa
		{
			str[8] = '\0'; // обрезaние строки
			return str;
		}
		else if (c < 2 || c > 11 || i > 7) // если выход зa пределы мaссивa или пользовaтель ввел не цифру
		{
			continue; // игнорировaние вводa
		}
		else // если пользовaтель ввел цифру
		{
			str[i] = (char)rg.h.al; // зaпись в строку цифры
			i++; // переход впрaво нa одну позицию

			if (i == 2 || i == 5)
			{
				str[i] = delim;
				i++;
			}

			str[i] = '\0'; // обрезaние строки
			WriteXY(str, Xpos, Ypos, COLORMAP[1]); // вывод числa нa экрaн
		}

	}
}

char* B2Bin(unsigned char bt, char* str)//перевод в 2ичную систему
{
    int i;
    for (i = 0; i < 8; i++)
    {
        str[i] = (bt & 0x80) ? '1' : '0';
        bt <<= 1;
    }
    str[8] = '\0';
    return str;
}

char* B2H2(unsigned char bt, char* str)//перевод в 16ричную
{
    str[0] = B4to1H(bt >> 4);
    str[1] = B4to1H(bt);
    str[2] = '\0';
    return str;
}

unsigned char B4to1H(unsigned char bt)//перевод тетрады в 16-ричную систему
{
    bt &= 0x0F;
    return (bt <= 9) ? '0' + bt : 'A' + (bt - 10);
}

char* IntToStr(int n)
{
	char s[40], t, * temp;
	int i, k;
	int sign = 0;
	i = 0;
	k = n;
	if (k < 0)
	{
		sign = 1;
		k = -k;
	}
	do {
		t = k % 10;
		k = k / 10;
		s[i] = t | 0x30;
		i++;
	} while (k > 0);
	if (sign == 1)
	{
		s[i] = '-';
		i++;
	}
	temp = malloc(sizeof(char) * i);
	k = 0;
	i--;
	while (i >= 0) {
		temp[k] = s[i];
		i--; k++;
	}
	temp[k] = '\0';
	return(temp);
}

int bcdtoint(char* k)
{
	return (*k) % 16 + (*k) / 16 * 10;
}

int bcdtoint2(char* k)
{
	return bcdtoint(k + 1) * 100 + bcdtoint(k);
}

void itodec(int i, char* c, int j)
{
	c[j] = '0' + ((i % 100) / 10);
	c[j + 1] = '0' + (i % 10);
}

void itodec2(int i, char* c, int j)
{
	c[j] = '0' + ((i % 10000) / 1000);
	c[j + 1] = '0' + ((i % 1000) / 100);
	c[j + 2] = '0' + ((i % 100) / 10);
	c[j + 3] = '0' + (i % 10);
}

void inttobcd(int i, char* c)
{
	*c = (i % 10) + ((i / 10) << 4);
}

int AtoI(char* str)
{
	int  i = 0, value = 0, j = 1;
	for (i = 0; str[i] != '\0'; i++); 
	i--;
	for (i, j = 1; i >= 0; i--, j *= 10) 
	{
		if (str[i] <= 57 && str[i] >= 48) 
		{
			value += (str[i] - 48) * j;
		}
		else j /= 10;
	}
	return value; 
}