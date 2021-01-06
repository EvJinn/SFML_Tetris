#pragma warning(disable : 4996)

#include <SFML/Graphics.hpp>
#include <time.h>
#include <sstream>
#include <fstream>
#include <string>

using namespace sf;

const float scale = 0.5; //Масштаб
const int startSpeed = 16;
//Пути до файлов с ресурсами игры
const std::string	PathToBlock = "TetrisBlock.png",
					PathToBackground = "TetrisBackground.png",
					PathToIcon = "TetrisIcon.png",
					PathToRectangle = "TetrisRectangle.png",
					PathToHelpBackground = "TetrisHelpBackground.png",
					PathToFont = "lcdm2n.ttf",
					PathToFontCyrillic = "lcdnov_9.ttf";


const int M = 20, N = 10;	//Высота и ширина поля
int field[M][N] = { 0 };	//Само игровое поле
int figures[7][4] =			//Массив фигур
{
	1,3,5,7, // I
	2,4,5,7, // S
	3,5,4,6, // Z
	3,5,4,7, // T
	2,3,5,7, // L
	3,5,7,6, // J
	2,3,4,5, // O
};

struct Point //Дополнительная вспомогательная структура для хранения координат
{
	int x, y;
}
a[4], b[4], c[4];

//Основные функции игры
bool borderCheck() //Проверка границ поля
{
	for (int i = 0; i < 4; i++)
	{
		if (a[i].x < 0 || a[i].x >= N || a[i].y >= M) return 0; //Проверка границ слева || справа || снизу
		else if (field[a[i].y][a[i].x]) return 0;				//Проверка ячейки на занятость
	}
	return 1;
};

int randomN[3];
int randomize(bool* beginGame, int prevn) //Рандомизатор с сохранением истории. Описан в курсовой.
{
	randomN[0] = prevn; //Предыдущая фигура
	if (*beginGame == true) //Проверка начала игры
	{
		randomN[1] = rand() % 7;
		randomN[2] = rand() % 7;
		if (randomN[2] == randomN[1]) randomN[2] = rand() % 7; //Сравниваниваем новую фигуру с историей
		return randomN[1];
	}
	if (*beginGame == false)
	{
		randomN[1] = randomN[2]; //Перемещаем третий элемент во вторую ячейку
		randomN[2] = rand() % 7; //Генерируем новую третью фигуру (для истории)
		if (randomN[2] == randomN[1]) randomN[2] = rand() % 7;
		return randomN[1]; //Возвращаем центральную полученную фигуру
	}
};

void beginGameCheck(bool* beginGame, int* n)
{
	if (*beginGame) //Проверка начала игры. По сути костыль, чтобы избежать бага с 1 клеткой вместо цельной фигуры вначале.
	{
		//n = rand() % 7; //Отладка
		*n = randomize(*&beginGame, 0);
		*beginGame = false;
		for (int i = 0; i < 4; i++)
		{ //Связь локальных координат массива фигур с глобальными координатами массива поля
			a[i].x = 4 + figures[*n][i] % 2;
			a[i].y = figures[*n][i] / 2;
		}
	}
};

bool endGame() //Проверка конца игры
{
	for (int i = 0; i < M; i++) //Проверяем первую (если считать сверху) строку игрового поля
		if (field[0][i]) return 0; //Если заполнена хоть одна из ячеек, то возвращаем ноль
	return 1;
};

void horizontalMoving(short* dx) //Горизонтальное перемещение
{
	for (int i = 0; i < 4; i++) //Перемещение фигуры по полю
	{
		b[i] = a[i];	//Запоминаем первоначальные координаты фигуры
		a[i].x += *dx;	//Горизонтальное
		//a[i].y += dy; //Вертикальное. Использовалось для отладки
	}
	if (!borderCheck())
		for (int i = 0; i < 4; i++) a[i] = b[i]; //Если вышли за границу, то возвращаем старые координаты
};

void verticalMoving(float* timer, float* delay, bool* beginGame, int* n)
{
	if (*timer > *delay) //Движение фигур вниз по тику таймера
	{
		for (int i = 0; i < 4; i++)
		{
			b[i] = a[i]; //Запоминаем старые координаты
			a[i].y += 1; //Сдвигаем фигуру вниз
		}
		if (!borderCheck())
		{
			for (int i = 0; i < 4; i++) field[b[i].y][b[i].x] = 1; //Заполняем ячейки игрового поля фигурой
			//n = rand() % 7; //Отладка
			*n = randomize(beginGame, *n);
			for (int i = 0; i < 4; i++)
			{ //Связь локальных координат массива фигур с глобальными координатами массива поля
				a[i].x = 4 + figures[*n][i] % 2;
				a[i].y = figures[*n][i] / 2;
			}
		}
		*timer = 0; //Сбрасываем таймер
	}
}

void rotateFigure(bool* rotate) //Поворот фигур
{
	if (*rotate == true) //Проверка нажатия клавиши поворота
	{
		Point p = a[1]; //Точка вращения в центре фигуры
		for (int i = 0; i < 4; i++)
		{
			//Вращение вокруг заданной точки
			//x = x0 + (x - x0) * cos(a) - (y - y0) * sin(a)
			//y = y0 + (y - y0) * cos(a) + (x - x0) * sina(a)
			//sin(90°) = 1, cos(90°) = 0 следовательно получаем:
			//x = x0 - (y - y0)
			//y = y0 + (x - x0)

			int x = a[i].y - p.y; //y - y0
			int y = a[i].x - p.x; //x - x0
			a[i].x = p.x - x;
			a[i].y = p.y + y;
		}
		if (!borderCheck())
			for (int i = 0; i < 4; i++) a[i] = b[i]; //Если вышли за границу, то возвращаем старые координаты
	}
};

void lineCheck(int* score, int* level, int* speed) //Проверка линий
{
	int k = M - 1;
	for (int i = M - 1; i > 0; i--)
	{
		int count = 0; //Счётчик занятых ячеек
		for (int j = 0; j < N; j++)
		{
			if (field[i][j]) count++; //Проверяем, занята ли ячейка
			field[k][j] = field[i][j]; //Присваиваем элемент самому себе
		}
		if (count < N) k--; //Сравниваем количество занятых ячеек с количеством ячеек в строке. Если есть пробелы в строке, то идём вниз
		if (count == N) //Если нет пробелов в строке, то при следующем проходе по строкам произойдёт сдвиг на одну строку вниз. Ниже описан код для подсчёта очков при такой ситуации
		{
			*score += 100;
			*level += 1;
			*speed += 1;
		}
	}
};

void helpWindow(Font fontCyrillic, Image icon)
{
	RenderWindow help(VideoMode(1024 * scale, 1035 * scale), "Help", Style::Close); //Создание окна справки
	help.setIcon(128, 128, icon.getPixelsPtr()); //Установка иконки с нужным разрешением
	help.setPosition(sf::Vector2i(10, 10)); //Устанавливаем позицию окна справки

	Texture textureHelpBackground; //Текстура фона
	textureHelpBackground.loadFromFile(PathToHelpBackground);
	
	Sprite helpBackground(textureHelpBackground); //Спрайт фона
	helpBackground.scale(scale, scale);

	Text helpText; 
	helpText.setFont(fontCyrillic);
	helpText.setCharacterSize(40 * scale);
	helpText.setColor(Color::Black);
	helpText.setPosition(4 * scale, 4 * scale);

	std::string text =	"Справка\n\n\nУправление:\n\nУправление в игре производится с помощью клавиш\n\nсо стрелками или клавиш W, A, D и пробел. При этом\n\nстрелки влево и вправо (клавиши A и D) отвечают за\n\nперемещение фигуры по полю влево и вправо\n\nсоответственно. Стрелка вверх и клавиша W\n\nотвечают за поворот фигуры в пространстве.\n\nСтрелка вниз и пробел отвечают за ускорение\n\nпадения фигуры вниз.\n\n\nНачисление очков:\n\nЗа каждую уничтоженную линию вам начисляется\n\n100 очков. Если было уничтожено 4 линии за один\n\nраз, вам дополнительно начислятся 1000 очков.\n\n\nЧтобы закрыть это окно нажмите клавишу ESC.";
	
	helpText.setString(text);
	while (help.isOpen())
	{
		//Отрисовываем элементы окна
		help.draw(helpBackground);
		help.draw(helpText);

		//Обрабатываем события в цикле
		Event event;
		while (help.pollEvent(event))
		{
			//Если пользователь захотел закрыть окно
			if (event.type == Event::Closed) help.close(); //Закрываем его (логично)
			if (event.type == Event::KeyPressed)
				if (event.key.code == Keyboard::Escape) help.close();
		}
		//Отображаем окно
		help.display();
	}
}

int main()
{
	setlocale(LC_ALL, "Russian");
	srand(time(0));
	//Объект окна с нужными нам характеристиками - размером окна и его заголовком
	RenderWindow window(VideoMode(1024 * scale, 1035 * scale), "Tetris", Style::Close | Style::Titlebar);

	Texture textureBlock, textureBackground, textureRectangle;	//Объект текстур
	textureBlock.loadFromFile(PathToBlock);		//Загрузка из файла
	textureBackground.loadFromFile(PathToBackground);
	textureRectangle.loadFromFile(PathToRectangle);

	Sprite block(textureBlock), background(textureBackground), rectangle(textureRectangle); //Объект спрайта с привязанным к нему объектом текстуры
	
	//Устанавливаем масштаб для спрайтов, потому что оригинальное разрешение слишком большое окно рисует.
	background.scale(scale, scale);
	block.scale(scale, scale);
	rectangle.scale(scale, scale);
	
	rectangle.setPosition(20 * scale, 408 * scale);

	Image icon;										//Объект изображения. Здесь будет иконка приложения
	icon.loadFromFile(PathToIcon);					//Загрузка из файла
	window.setIcon(128, 128, icon.getPixelsPtr());	//Установка с нужным разрешением

	Font font, fontCyrillic; //Объект шрифта для отображения текста
	font.loadFromFile(PathToFont);
	fontCyrillic.loadFromFile(PathToFontCyrillic); //Кириллический шрифт

	Text highScoreLabel;
	highScoreLabel.setFont(fontCyrillic);
	highScoreLabel.setString("High score");
	highScoreLabel.setCharacterSize(48 * scale);
	highScoreLabel.setColor(Color::Black);
	highScoreLabel.setPosition(550 * scale, 2 * scale);

	Text displayHighScore;
	displayHighScore.setFont(font);
	displayHighScore.setCharacterSize(48 * scale);
	displayHighScore.setColor(Color::Black);
	displayHighScore.setPosition(800 * scale, 2 * scale);

	Text scoreLabel;
	scoreLabel.setFont(fontCyrillic);
	scoreLabel.setString("Your score");
	scoreLabel.setCharacterSize(48 * scale);
	scoreLabel.setColor(Color::Black);
	scoreLabel.setPosition(550 * scale, 56 * scale);

	Text displayScore;
	displayScore.setFont(font);
	displayScore.setCharacterSize(48 * scale);
	displayScore.setColor(Color::Black);
	displayScore.setPosition(800 * scale, 56 * scale);

	Text levelLabel;
	levelLabel.setFont(fontCyrillic);
	levelLabel.setString("Level");
	levelLabel.setCharacterSize(48 * scale);
	levelLabel.setColor(Color::Black);
	levelLabel.setPosition(550 * scale, 110 * scale);

	Text displayLevel;
	displayLevel.setFont(font);
	displayLevel.setCharacterSize(48 * scale);
	displayLevel.setColor(Color::Black);
	displayLevel.setPosition(800 * scale, 110 * scale);

	Text pauseLabel;
	pauseLabel.setFont(fontCyrillic);
	pauseLabel.setString("Pause");
	pauseLabel.setCharacterSize(100 * scale);
	pauseLabel.setColor(Color::Black);
	pauseLabel.setPosition(125 * scale, 450 * scale);

	Text gameOverLabel;
	gameOverLabel.setFont(fontCyrillic);
	gameOverLabel.setString("Game over!");
	gameOverLabel.setCharacterSize(90 * scale);
	gameOverLabel.setColor(Color::Black);
	gameOverLabel.setPosition(50 * scale, 460 * scale);

	Text startLabel;
	startLabel.setFont(fontCyrillic);
	startLabel.setString("      Start \n\n   new game!");
	startLabel.setCharacterSize(70 * scale);
	startLabel.setColor(Color::Black);
	startLabel.setPosition(50 * scale, 424 * scale);

	Text helpLabel;
	helpLabel.setFont(fontCyrillic);
	helpLabel.setString("F1 - Help \n\nEnter - Start new game \n\nEsc - Pause");
	helpLabel.setCharacterSize(44 * scale);
	helpLabel.setColor(Color::Black);
	helpLabel.setPosition(540 * scale, 840 * scale);

	Text nextFigureLabel;
	nextFigureLabel.setFont(fontCyrillic);
	nextFigureLabel.setString("Next figure:");
	nextFigureLabel.setCharacterSize(48 * scale);
	nextFigureLabel.setColor(Color::Black);
	nextFigureLabel.setPosition(624 * scale, 300 * scale);

	float timer = 0, delay = 1;
	Clock clock;

	short dx = 0/*, dy = 0*/; //переменные перемещения спрайтов по полю. dy использовалась для отладки
	//Флаги:
	bool rotate = false;	//Поворот фигуры
	bool beginGame = true;	//Начало игры
	bool pauseGame = false; //Пауза
	bool startGame = false; //Начало новой игры
	bool gameOver = false;	//Проигрыш
	//int n = rand() % 7; //Отладка
	int n;					//Переменная фигуры
	int score = 0;			//Набираемые очки
	int level = 0;			//Уровень
	int speed = startSpeed; //Скорость

	//Считываем из файла последний сохранённый лучший результат
	int highScore = 0;
	std::ifstream highScoreRead("highScore.txt"); //Открываем файл для чтения
	if (highScoreRead.is_open()) //Проверка открытия файла
	{
		highScoreRead >> highScore; //Если файл открылся, то считываем из него число с лучшим результатом
		highScoreRead.close();		//Закрываем файл для чтения
	}
	else
	{
		highScoreRead.close();							//Закрываем файл для чтения
		std::ofstream highScoreWrite("highScore.txt");	//Открываем файл для записи
		highScoreWrite << score;						//Записываем в него текущий результат (поскольку игра только запущена будет записан ноль)
		highScoreWrite.close();							//Закрываем файл для записи
		highScore = score;
	}

	// Главный цикл приложения. Выполняется, пока открыто окно
	while (window.isOpen())
	{
		float time = clock.getElapsedTime().asSeconds(); //Получаем время с таймера
		clock.restart(); //Сбрасываем таймер
		timer += time; //Прибавляем полученное время

		//Отрисовка текстов и спрайта фона
		window.draw(background);
		window.draw(highScoreLabel);
		window.draw(displayHighScore);
		window.draw(scoreLabel); 
		window.draw(displayScore);
		window.draw(levelLabel);
		window.draw(displayLevel);
		window.draw(helpLabel);
		window.draw(nextFigureLabel);

		bool DownKeyPressed = false; //Флаг проверки нажатия клавиши ускорения фигуры

		// Обрабатываем события в цикле
		Event event;
		while (window.pollEvent(event))
		{
			//Если пользователь захотел закрыть окно
			if (event.type == Event::Closed) window.close(); //Закрываем его (логично)
			//Обработка нажатий клавиш на клавиатуре
			if (event.type == Event::KeyPressed)
			{
				if (event.key.code == Keyboard::Up || event.key.code == Keyboard::W) rotate = true; //Стрелка вверх
				if (event.key.code == Keyboard::Left || event.key.code == Keyboard::A) dx = -1;		//Стрелка влево
				if (event.key.code == Keyboard::Right || event.key.code == Keyboard::D) dx = 1;		//Стрелка вправо
				if (event.key.code == Keyboard::F1) helpWindow(fontCyrillic, icon);					//F1. Вызывает справку и передаёт ей фон и иконку
				if (event.key.code == Keyboard::Escape)												//ESC. 
					if (!gameOver) //Если игра не закончена
					{
						if (!pauseGame) pauseGame = true;	//И если не пауза, то ставим игру на паузу
						else pauseGame = false;				//Иначе снимаем игру с паузы
					}
				if (event.key.code == Keyboard::Enter) //Enter
				{
					if (gameOver) //Если игра окончена
					{
						gameOver = false; //Снимаем флаг окончания игры
						//Очищаем поле и обнуляем очки, уровень и скорость
						for (int i = 0; i < M; i++)
							for (int j = 0; j < N; j++)
								field[i][j] = 0;
						score = 0;
						level = 0;
						speed = startSpeed;
					}
					startGame = true; //Поднимаем флаг начала новой игры
					
				}
				if (event.key.code == Keyboard::Space) //Пробел. Из-за некоторых особенностей идёт проверка одиночного нажатия, а не зажатия клавиши, хотя принцип работы такой же как у проверки стрелки вниз
				{
					DownKeyPressed = true;	//Поднимаем флаг нажатия клавиши вниз
					delay *= 0.05;			//Уменьшаем задержку таймера для ускорения падения фигуры вниз
				}
				//Отладка
				//if (event.key.code == Keyboard::Down) dy = 1;
				//if (event.key.code == Keyboard::Up) dy = -1;
			}
			if (Keyboard::isKeyPressed(Keyboard::Down)) //Стрелка вниз
			{ //Логика такая же как у пробела
				DownKeyPressed = true;
				delay *= 0.05;
			}
		}

		if (!startGame && !gameOver) //Если у нас первый запуск, то отображаем окно с предложением начать новую игру
		{
			window.draw(rectangle);
			window.draw(startLabel);
		}

		if (startGame && !gameOver) //Если игра началась, но не проиграна, то выполняем игровую логику
		{
			if (!pauseGame)
			{
				horizontalMoving(&dx);							//Горизонтальное движение
				rotateFigure(&rotate);							//Поворот фигуры
				verticalMoving(&timer, &delay, &beginGame, &n);	//Вертикальное движение
				int templevel = level;							//Сохраняем текущий уровень
				lineCheck(&score, &level, &speed);				//Проверка линий для удаления заполненных
				if (level - templevel == 4) score += 1000;		//Если удалилось сразу 4 линии, то начисляем дополнительные очки
				beginGameCheck(&beginGame, &n);					//Костыль проверки начала игры (суть описана в самой функции)

				dx = 0;				//Обнуляем направление движения
				//dy = 0; //Отладка
				rotate = false;		//Убираем флаг поворота
				if (DownKeyPressed == true) delay /= 0.05;	//Если было ускорение, то возвращаем задержку таймера на прежний уровень
				delay = 1. / (0.5 * (float)sqrt(speed));	//Рассчитываем скорость согласно изменению уровня (если оно вообще было)
				if (!endGame()) //Проверка верхней границы
				{
					startGame = false; //Опускаем флаг начала новой игры
					if (score > highScore) //Если набранные очки больше лучшего результата
					{
						std::ofstream highScoreWrite("highScore.txt");	//Открываем файл для записи
						highScoreWrite << score;						//Записываем в него результат
						highScoreWrite.close();							//Закрываем файл
						highScore = score;								//Записываем в переменную лучшего результата текущий
					}
					gameOver = true; //Поднимаем флаг окончания игры
				}
			}
		}

		//Отрисовка всего того, что понаписано выше
		for (int i = 0; i < M; i++)
		{
			for (int j = 0; j < N; j++)
			{
				if (field[i][j] == 0) continue;
				block.setPosition(j * 51 * scale, i * 51 * scale);
				block.move(6 * scale, 6 * scale);
				window.draw(block);
			}
		}
		for (int i = 0; i < 4; i++)
		{
			block.setPosition(a[i].x * 51 * scale, a[i].y * 51 * scale);
			block.move(6 * scale, -44 * scale);
			window.draw(block);
			//Отрисовка спрайтов, указывающих на следующую фигуру
			for (int j = 0; j < 4; j++)
			{ //Связь локальных координат массива фигур с глобальными координатами массива поля
				c[j].x = figures[randomN[2]][j] % 2;
				c[j].y = figures[randomN[2]][j] / 2;
			}
			block.setPosition(c[i].x * 51 * scale, c[i].y * 51 * scale);
			block.move(684 * scale, 380 * scale);
			window.draw(block);
		}

		//Если игра окончена или стоит на паузе, то отображаем соответствующие сообщения
		if (gameOver)
		{
			window.draw(rectangle);
			window.draw(gameOverLabel);
		}
		if (pauseGame)
		{
			window.draw(rectangle);
			window.draw(pauseLabel);
		}
		

		//Преобразование чисел в строки, для того чтобы добавить их в отображаемые текстовые поля
		std::ostringstream playerScoreString;
		playerScoreString << (score);
		displayScore.setString(playerScoreString.str());
		
		std::ostringstream playerLevelString;
		playerLevelString << (level);
		displayLevel.setString(playerLevelString.str());

		std::ostringstream highScoreString;
		highScoreString << (highScore);
		displayHighScore.setString(highScoreString.str());

		window.display(); //Отрисовка самого окна
	}
	return 0;
}