#include<dos.h>
#define PORT20 0x20
#define PORT60 0x60
#define PORT61 0x61
#define PORT64 0x64

//Флаг завершения программы
int exitFlag = 0;
//Флаг мигания
int flickFlag = 0;
//Флаг ошибки для повторной отправки данных
int resendFlag = 1;
//Положение курсора для вывода функцией Write()
int x = 0, y = 6;
//Счет, сколько записано символов функцией Write()
unsigned int count = 0;
//Собственный бработчик прерывания 9
void interrupt newInt9h(void);
//Старый обработчик прерывания 9
void interrupt(*oldInt9h)(void);

//Прокрутка экрана
void ScrollScreen(void);
//Очистить в определённой строке n символов
void ClearSymbols(int, int);
//Вывод в строки в указанные координаты определённым цветом
void WriteXY(char* str, unsigned int, unsigned int, unsigned int);
//Вывод строки по глобальным координатам
void Write(char*, unsigned int);
//Чтение и вывод значений регистров
void RegistersWork(void);
//Помигать индикаторами
void Flick(void);
//Включает нужный индикатор клавиатуры
void Indicator(unsigned char);
char* Byte2Bin(unsigned char, char*);
char* Byte2Hex(unsigned char, char*);
char Tetrad2Hex(unsigned char);
void ISRStatus(void);

//Устанавливаем для  char far* синоним address (adds)
typedef char far* adds;

int main()
{
	clrscr();
	WriteXY("Контроллер клавиатуры", 1, 26, 7);
	//Для красивого оформления
	WriteXY("1", 5, 11, 14);
	WriteXY("- Выход  |", 5, 15, 6);
	WriteXY("Space", 5, 26, 14);
	WriteXY("- Индикаторы  |", 5, 32, 6);
	WriteXY("Ctrl", 5, 49, 14);
	WriteXY("- Очистить экран", 5, 55, 6);
	//Запоминаем вектор обработчика прерывания 9
	oldInt9h = getvect(9);
	//Ставим собственный обработчик прерывания 9
	setvect(9, newInt9h);
	//Выводим регистры
	RegistersWork();
	ISRStatus();
	//Пока флаг выхода установлен в 0
	while (!exitFlag)
	{
		//Мигаем, если флаг мигания установлен в 1
		if (flickFlag)
			Flick();
		delay(10);
	}
	//Восстанавливаем старый обработчик
	setvect(9, oldInt9h);
	clrscr();
	return 0;
}

//Прокрутка экрана
void ScrollScreen(void)
{
	int i, j;
	char str;
	//Адрес начала видобуфера B800:0000
	adds beg = (adds)0xB8000000;
	//Для сохранения положения в видеобуфере
	adds value;
	//Начинаем с 6 строки, так как 0-5 заняты
	int strok = 6;
	for (i = strok; i < 24; i++)
	{
		//Всего 80 символов в строке, обрабатываем каждый код символа
		for (j = 0; j < 80; j++)
		{
			//Текущее положение в видеобуфере
			//Прыгаем через символ, забирая лишь старший байт
			value = beg + (i + 1) * 160 + j * 2;
			//Старший байт - код символа. Его и запоминаем
			str = *value;
			//Вставляем символ в тот же столбец, но на строчку выше
			value = beg + i * 160 + j * 2;
			*value = str;
		}
	}
	//Очищаем 80 символов последней строчки
	ClearSymbols(24, 80);
}

//Обработчик прерывания 9
void interrupt newInt9h(void)
{
	//Скан-код клавиши
	unsigned char value;
	//Буфер для вывода данных
	char str[9];
	//Читаем содержимое регистров
	RegistersWork();
	ISRStatus();
	if (count >= 1519)//1520/80 = 25-6 - Экран полностью заполнен
	{
		//Перемещаем курсор на начало последней строчки
		y = 24; x = 0;
		//Прокручиваем экран
		ScrollScreen();
		//Ставим счет без учета последней строки (1519-80=1439)
		count = 1439;
	}
	//Читаем содержимое регистра управления (скан-код)
	value = inp(PORT60);
	//esc - 0х01, выходим
	if (value == 0x02) exitFlag = 1;
	//space - 0х39, тогда мигаем
	else if (value == 0x39)
		flickFlag = !flickFlag;
	//ctrl - 0x1D, очищаем всё
	else if (value == 0x1D) 
	{
		ClearSymbols(6, count);
		y = 6; x = 0; count = 0; 
	}
	//Если нет подтверждения успешного выполнения команды,
	//то устанавливаем флаг передечи байта resendFlag
	if (value != 0xFA && flickFlag == 1) resendFlag = 1;
	else resendFlag = 0;
	//Переводим скан-код в hex и выводим на экран
	Byte2Hex(value, str);
	Write(str, 15);
	//Посылаем сигнал "конец прерывания" контроллеру прерываний 8259
	outp(PORT20, 0x20);
	delay(50);
	ISRStatus();
}

//Очистить count символов начиная с строки strok
void ClearSymbols(int strok, int _count)
{
	int i;
	int stolb = 0;
	//Адрес начала видеобуфера
	adds beg = (adds)0xB8000000;
	//Текущее положение в видеобуфере
	adds value;
	for (i = 0; i < _count; i++, stolb++)
	{
		//Заполняем пробелами
		value = beg + strok * 160 + stolb * 2;
		*value = ' ';
	}
}

//Вывод в строки в указанные координаты определённым цветом
void WriteXY(char* str, unsigned int strok, unsigned int stolb, unsigned int attrib)
{
	int i;
	//Адрес на начало видеобуфера B800:0000
	adds beg = (adds)0xB8000000;
	//Текущее положение в видеобуфере
	adds value;
	for (i = 0; str[i]; i++)
	{
		//Преобразовываем координаты
		value = beg + strok * 160 + stolb * 2;
		//Помещаем символ в видеобуфер
		*value = str[i];
		stolb++;
		value++;
		//Ставим соответствующий атрибут для цвета
		*value = attrib;
	}
}

//Вывод строки по глобальным координатам
void Write(char* str, unsigned int attrib)
{
	unsigned int i;
	//Адрес начала видеобуфера B800:0000
	adds beg = (adds)0xB8000000;
	//Текущее положение в видеобуфере
	adds value;

	for (i = 0; str[i]; i++)
	{
		//Преобразовываем координаты x и y
		value = beg + y * 160 + x * 2;
		//Выводим в видеобуфер символ
		*value = str[i];
		x++;
		value++;
		//Задаем атрибут цвета символу
		*value = attrib;
		//Увеличиваем выведенное количество символов
		count++;
	}
	//Если переходим на новую строку, задаем соответствующее
	//положение курсора в начале следующей строки
	if (x >= 80)
	{
		y++; x = 0;
	}
	//Иначе ставим пробел для разделения
	else
	{
		value = beg + y * 160 + x * 2;
		x++;
		*value = ' ';
		count++;
	}
}

//Чтение и вывод значений регистров
void RegistersWork()
{
	unsigned char t;
	char str[9];

	//Запоминаем содержимое порта 64h
	t = inp(PORT64);
	Byte2Hex(t, str);
	//Выводим на экран в HEX
	WriteXY("64h: ", 2, 0, 3);
	WriteXY(str, 2, 5, 3);
	//Переводим в bin и тоже выводим на экран
	Byte2Bin(t, str);
	WriteXY(str, 2, 9, 3);
	//Так же с портом 61h и с портом 60h
	t = inp(PORT61);
	Byte2Hex(t, str);
	WriteXY("61h: ", 3, 0, 3);
	WriteXY(str, 3, 5, 3);
	Byte2Bin(t, str);
	WriteXY(str, 3, 9, 3);

	t = inp(PORT60);
	Byte2Hex(t, str);
	WriteXY("60h: ", 4, 0, 3);
	WriteXY(str, 4, 5, 3);
	Byte2Bin(t, str);
	WriteXY(str, 4, 9, 3);
}

//Мигание
void Flick()
{
	//Включить Num Lock 00000010
	Indicator(0x02);
	delay(150);
	//Включить Caps Lock 00000100
	Indicator(0x04);
	delay(150);
	//Включить Num + Caps 00000110
	Indicator(0x06);
	delay(200);
	//Выключить все 00000000
	Indicator(0x00);
}

//Включение нужного индикатора клавиатуры
void Indicator(unsigned char mask)
{
	resendFlag = 1;
	//Пока нет подтверждения успешного выполнения команды
	//пересылаем повторно
	while (resendFlag)
	{
		//Ожидаем освобождения входного буфера клавиатуры
		//проверяя бит 1 порта 64h
		while ((inp(PORT64) & 0x02) != 0x00);
		//Записываем в порт команду управления индикаторами 0xED
		outp(PORT60, 0xED);
		delay(50);
	}
	resendFlag = 1;

	//Ожидаем освобождения входного буфера клавиатуры
	//проверяя бит 1 порта 64h
	while (inp(PORT64) & 0x02 != 0x00);
	//Записываем в порт битовую маску для настройки индикаторов
	outp(PORT60, mask);
	delay(50);
}

//Байт в бинарную (строку)
char* Byte2Bin(unsigned char value, char* str)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		//Проверяем старший бит
		str[i] = (value & 0x80) ? '1' : '0';
		//Сдвигаем на 1 бит влево
		value <<= 1;
	}
	str[8] = '\0';
	return str;
}

char* Byte2Hex(unsigned char value, char* str)
{
	str[0] = Tetrad2Hex(value >> 4);
	str[1] = Tetrad2Hex(value);
	str[2] = '\0';
	return str;
}

char Tetrad2Hex(unsigned char _4bitValue)
{
	_4bitValue &= 0x0F;
	return (_4bitValue < 10) ? '0' + _4bitValue : 'A' - 10 + _4bitValue;
}

void ISRStatus()
{
	unsigned char t;
	char str[9];

	outp(PORT20, 0x0B);
	t = inp(PORT20);
	Byte2Hex(t, str);
	WriteXY("ISR:", 4, 63, 2);
	WriteXY(str, 4, 68, 2);
	Byte2Bin(t, str);
	WriteXY(str, 4, 72, 2);
}