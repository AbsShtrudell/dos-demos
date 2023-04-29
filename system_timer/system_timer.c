#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>

#define TIME_CLOCK 1193180
#define PORT61 0x61
#define PORT40 0x40
#define PORT41 0x41
#define PORT42 0x42
#define PORT43 0x43

// Структура, содержащая частоту и длительность звука
typedef struct
{
	int frequency;
	int time;
} Note;

void DynamicOn(void);
void DynamicOff(void);
void ShowDynamicStatus(void);
void ReadStatusWords(void);
void PlaySound(Note note);
char* ToBinary(int value, char* resultValue, int bitCount);
char* ToHexadecimal(int value, char* resultValue, int bitCount);
char FirstBytesToHex(int value);

int main()
{
	Note notes[] =
	{
		{82, 142}, {0, 991},
		{147, 142}, {0, 142},
		{139, 142}, {0, 142},
		{131, 1698},

		{82, 140}, {10, 2},
		{82, 142},
		{165, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{156, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{147, 142}, {0, 142},
		{139, 142}, {0, 142},
		{131, 566},
		{82, 140}, {10, 2},
		{82, 142},
		{123, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{117, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{110, 142},
		{82, 142},
		{104, 142},
		{82, 142},
		{98, 142},
		{82, 142},
		{93, 142},
		{87, 142},

		{82, 140}, {10, 2},
		{82, 142},
		{165, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{156, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{147, 142}, {0, 142},
		{139, 142}, {0, 142},
		{131, 566},
		{82, 140}, {10, 2},
		{82, 142},
		{123, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{117, 142},
		{82, 140}, {10, 2},
		{82, 142},
		{110, 142},
		{82, 142},
		{104, 142},
		{82, 142},
		{98, 142},
		{82, 142},
		{93, 142},
		{87, 142},

		{82, 142},
		{87, 142},
		{123, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{82, 142},
		{87, 142},
		{139, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{82, 142},
		{87, 142},
		{123, 140}, {10, 2},
		{123, 142},
		{82, 142},
		{87, 142},
		{123, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{87, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142},

		{82, 142},
		{87, 142},
		{123, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{82, 142},
		{87, 142},
		{139, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{82, 142},
		{87, 142},
		{123, 140}, {10, 2},
		{123, 142},
		{82, 142},
		{87, 142},
		{123, 142},
		{82, 142},
		{87, 142},
		{131, 142},
		{87, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142},
		{82, 142},
		{98, 71}, {95, 71},
		{93, 142}

	};

	int notesCount = sizeof(notes) / sizeof(Note);
	int i;
	char choice;

	while (1)
	{
		system("cls");

		printf("Системный таймер персонального компьютера\n\n");

		printf("1. Проиграть мелодию\n");
		printf("2. Показать регистры каналов таймера\n");
		printf("3. Выход\n");
		choice = getch();

		system("cls");

		switch (choice)
		{
			case '1':
				DynamicOn(); // Включить динамик
				ShowDynamicStatus(); // Показать состояние порта 61
				ReadStatusWords();
				DynamicOff();

				system("pause");

				DynamicOn();

				for (i = 0; i < notesCount; i++)
				{
					if (notes[i].frequency == 0)
					{
						DynamicOff(); // Отключить динамик на длительность ноты
						delay(notes[i].time);
						DynamicOn();
					}
					else
					{
						PlaySound(notes[i]); // Проиграть ноту
					}
				}

				printf("\n");
				DynamicOff(); // Отключить динамик
				ShowDynamicStatus(); // Показать состояние порта 61
				ReadStatusWords();

				system("pause");

				break;
			case '2':
				ReadStatusWords(); // Показать статус каналов и регистров

				system("pause");

				break;
			case '3':
				system("cls");
				return 0;
		}
	}
}

// Функция включения динамика
void DynamicOn()
{
	int dwResult = inp(PORT61); // Чтение состояния из порта 61
	dwResult |= 0x03; // Для включения динамика нужно установить первые два бита в 1 с помощью побитовой операции ИЛИ
	outp(PORT61, dwResult); // Запись нового значения в порт 61
}

// Функция отключения динамика
void DynamicOff()
{
	int dwResult = inp(PORT61); // Чтение состояния из порта 61
	dwResult &= 0xFC; // Для выключения динамика нужно установить первые два бита в 0 с помощью побитовой операции И
	outp(PORT61, dwResult); // Запись нового значения в порт 61
}

// Функция показа состояния порта 61
void ShowDynamicStatus()
{
	int dwResult = inp(PORT61); // Чтение состояния из порта 61
	char dwString[9];
	dwString[8] = '\0';

	printf("Порт 61h\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 8));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 2));
}

// Функция воспроизведения звука
void PlaySound(Note note)
{
	int value = TIME_CLOCK / note.frequency; // Значения делителя частоты

	printf("%d-%d | ", note.frequency, note.time);
	outp(PORT42, (char)value); // Запись младшего байта
	outp(PORT42, (char)(value >> 8)); // Запись старшего байта

	delay(note.time); // Пауза на длительность ноты
}

// Функция показа состояния каналов и регистров
void ReadStatusWords()
{
	int dwResult;
	char dwString[17];
	dwString[16] = '\0';

	outp(PORT43, 0xe2);
	dwResult = inp(PORT40);
	printf("Порт 40h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 8));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 2));

	outp(PORT43, 0x00);
	dwResult = inp(PORT40) | (inp(PORT40) << 8);
	printf("Регистр счётчика 40h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 16));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 4));

	outp(PORT43, 0xe4);
	dwResult = inp(PORT41);
	printf("Порт 41h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 8));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 2));

	outp(PORT43, 0x40);
	dwResult = inp(PORT41) | (inp(PORT41) << 8);
	printf("Регистр счётчика 41h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 16));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 4));

	outp(PORT43, 0xe8);
	dwResult = inp(PORT42);
	printf("Порт 42h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 8));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 2));

	outp(PORT43, 0x80);
	dwResult = inp(PORT42) | (inp(PORT42) << 8);
	printf("Регистр счётчика 42h:\n");
	printf("\tдвоичный: %s\n", ToBinary(dwResult, dwString, 16));
	printf("\tшестнадцатеричный: %s\n", ToHexadecimal(dwResult, dwString, 4));
}

// Функция перевода числа из десятичной системы счисления в двоичную
char* ToBinary(int value, char* resultValue, int bitCount)
{
	int i;
	int mask = 1 << (bitCount - 1); // Маска, где старший бит 1, а остальные 0

	for (i = 0; i < bitCount; i++)
	{
		resultValue[i] = value & mask ? '1' : '0'; // Если результат наложения маски нулевой, то записывается 0, иначе 1
		value <<= 1;
	}
	resultValue[bitCount] = '\0';

	return resultValue;
}

// Функция перевода числа из десятичной системы счисления в
char* ToHexadecimal(int value, char* resultValue, int bitCount)
{
	resultValue[bitCount] = '\0';

	while (bitCount)
	{
		bitCount--;
		resultValue[bitCount] = FirstBytesToHex(value); // Получение шестнадцатеричной цифры из первых четырёх бит числа
		value >>= 4;
	}

	return resultValue;
}
// Функция получения шестнадцатеричной цифры
char FirstBytesToHex(int value)
{
	value &= 0x0F; // Избавление от всех бит числа кроме первых четырёх
	return (value < 10) ? '0' + value : 'A' - 10 + value; // Если число является десятичной цифрой, оно и возвращается, иначе буква от A до F
}
