#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <time.h>
#include <iomanip>
#include <string>

using namespace std;

int width = 8;
int height = 10;
int mineChance = 35;
bool allowCounting = false;
bool custom = false;

vector<vector<bool>> *lastBoard = new vector<vector<bool>>(height);
vector<vector<int>> *lastNumbers = new vector<vector<int>>(height);
int lastStartX = 0;
int lastStartY = 0;
int lastPlacedMines = 0;

vector<int> generateEquation(int x, int y, int value);
void gaussianElimination(vector<vector<int>> *matrix);
bool runItteration();

void obfuscate_bitmap(unsigned char *bmp, int bits, int decode);

string translation[] = {":zero:", ":one:", ":two:", ":three:", ":four:", ":five:", ":six:", ":seven:", ":eight:"};

bool ansiCodes = false;

int main()
{
	cout << "Defaults: " << to_string(width) << "x" << to_string(height) << " | " << to_string(mineChance) << "% Mine Chance | No Counting | No ANSI Code Coloring (y/n)";
	string initInput = "";
	while (initInput.size() == 0 || (initInput[0] != 'y' && initInput[0] != 'n' && initInput[0] != 'Y' && initInput[0] != 'N'))
	{
		cin >> initInput;
	}

	if (initInput[0] == 'n' || initInput[0] == 'N')
	{
		custom = true;
		cout << "Board Width: ";
		cin >> initInput;
		width = stoi(initInput);
		cout << "Board Height: ";
		cin >> initInput;
		height = stoi(initInput);
		cout << "Mine Chance (out of 100): ";
		cin >> initInput;
		mineChance = stoi(initInput);
		cout << "Allow Counting(y/n): ";
		cin >> initInput;
		allowCounting = initInput[0] == 'y' || initInput[0] == 'Y';
		cout << "ANSI Code Coloring(y/n): ";
		cin >> initInput;
		ansiCodes = initInput[0] == 'y' || initInput[0] == 'Y';
		cout << "Chosen Settings: " << to_string(width) << "x" << to_string(height) << " | " << to_string(mineChance) << "% Mine Chance | " << (allowCounting ? "Counting Allowed" : "No Counting") << " | " << (ansiCodes ? "" : "No ") << "ANSI Code Coloring\n\n";
	}

	srand(time(NULL));

	// for (int i = 0; i < 10; i += runItteration())
	//	;

	while (true)
	{
		while (!runItteration())
			;

		bool waitingForInput = true;
		while (waitingForInput)
		{
			string input = "";
			cin >> input;
			switch (input[0])
			{
			case 'n':
				waitingForInput = false;
				break;
			case 'c':
				return 0;
			case 'o':
				cout << "\n\n";
				cout << "This puzzle's :bomb: bomb count is " << to_string(lastPlacedMines) << "\n";
				for (int i = 0; i < height; i++)
				{
					for (int j = 0; j < width; j++)
					{
						if (i != lastStartY || j != lastStartX)
							cout << "||";
						if (lastBoard->at(i).at(j))
						{
							cout << ":bomb:";
						}
						else
						{
							cout << translation[lastNumbers->at(i).at(j)];
						}
						if (i != lastStartY || j != lastStartX)
							cout << "||";
					}
					cout << "\n";
				}
				cin >> input;
				return 0;
			}
		}
	}
}

vector<int> generateEquation(int x, int y, int value)
{
	vector<int> equation = vector<int>(width * height);
	equation.push_back(value);
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (x + i >= 0 && y + j >= 0 && x + i < width && y + j < height)
				equation.at((y + j) * width + (x + i)) = 1;
		}
	}
	return equation;
}

void gaussianElimination(vector<vector<int>> *matrix)
{
	vector<int> indecies;
	indecies.reserve(width * height * 0.1);

	for (int i = 0; i < width * height + 1; i++)
	{
		for (int j = 0; j < matrix->size(); j++)
		{
			if (matrix->at(j).at(i) != 0)
			{
				indecies.push_back(i);
				break;
			}
		}
	}

	int swapIndex = 0;
	for (int I = 0; I < int(indecies.size()) - 1; I++)
	{
		int i = indecies.at(I);
		int j = swapIndex;
		for (; j < matrix->size(); j++)
		{
			if (matrix->at(j).at(i) != 0)
				break;
		}
		if (j == matrix->size())
			continue;
		int J = j;
		if (j >= swapIndex)
		{
			vector<int> temp = matrix->at(swapIndex);
			matrix->at(swapIndex) = matrix->at(j);
			matrix->at(j) = temp;
			J = swapIndex;
			swapIndex++;
		}
		for (j = 0; j < matrix->size(); j++)
		{
			if (j == J || matrix->at(j).at(i) == 0)
				continue;
			int multiplier = matrix->at(J).at(i) / matrix->at(j).at(i);
			for (int k = 0; k < indecies.size(); k++)
			{
				matrix->at(j).at(indecies.at(k)) -= matrix->at(J).at(indecies.at(k)) * multiplier;
			}
		}
	}
}

bool runItteration()
{
	vector<vector<bool>> *board = new vector<vector<bool>>(height, vector<bool>(width));
	vector<vector<int>> *numbers = new vector<vector<int>>(height);
	vector<int> knownValues = vector<int>(width * height, -1); //[width * height]{};
	// fill(knownValues, knownValues + sizeof(int) * width * height, -1);
	vector<vector<int>> equations;

	int placedMines = 0;

	int startX = rand() % width;
	int startY = rand() % height;

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (max(abs(i - startY), abs(j - startX)) <= 1 || rand() % 100 > mineChance)
				continue;
			board->at(i).at(j) = true;
			placedMines++;
		}
	}

	if (!custom && placedMines < 21)
	{
		delete (board);
		delete (numbers);
		return false;
	}

	int zeros = 0;

	for (int i = 0; i < height; i++)
	{
		numbers->at(i) = vector<int>(width);
		for (int j = 0; j < width; j++)
		{
			// bitset<width> mask(0b111);
			int count = 0;
			if (i > 0)
			{
				count += j > 0 && board->at(i - 1).at(j - 1);
				count += board->at(i - 1).at(j);
				count += j < width - 1 && board->at(i - 1).at(j + 1);
			}
			//	count += (board->at(i - 1) & (j > 0 ? (mask << j - 1) : (mask >> 1))).count();
			//	count += (board->at(i) & (j > 0 ? (mask << j - 1) : (mask >> 1))).count();
			count += j > 0 && board->at(i).at(j - 1);
			count += board->at(i).at(j);
			count += j < width - 1 && board->at(i).at(j + 1);
			if (i < height - 1)
			{
				count += j > 0 && board->at(i + 1).at(j - 1);
				count += board->at(i + 1).at(j);
				count += j < width - 1 && board->at(i + 1).at(j + 1);
			}
			//	count += (board->at(i + 1) & (j > 0 ? (mask << j - 1) : (mask >> 1))).count();
			numbers->at(i).at(j) = count;
			zeros += numbers->at(i).at(j) == 0;
		}
	}

	if (!custom && zeros > 5)
	{
		delete (board);
		delete (numbers);
		return false;
	}

	if (allowCounting)
	{
		vector<int> equation = vector<int>(width * height, 1);
		equation.push_back(placedMines);
		equations.push_back(equation);
	}

	equations.push_back(generateEquation(startX, startY, numbers->at(startY).at(startX)));

	bool change = true;
	float score = 0;

	while (change)
	{
		int changesThisItteration = 0;
		change = false;
		gaussianElimination(&equations);
		bool recentChange = true;
		while (recentChange)
		{
			recentChange = false;
			bool countingEquationChanges = false;
			for (int i = equations.size() - 1; i > -1; i--)
			{
				changesThisItteration += countingEquationChanges;
				countingEquationChanges = false;
				int max = 0;
				int min = 0;
				for (int j = 0; j < width * height; j++)
				{
					if (equations.at(i).at(j) == 0)
						continue;
					if (knownValues[j] != -1)
					{
						equations.at(i).at(equations.at(i).size() - 1) -= knownValues[j] * equations.at(i).at(j);
						equations.at(i).at(j) = 0;
						change = true;
						recentChange = true;
						countingEquationChanges = true;
					}
					if (equations.at(i).at(j) > 0)
						max += equations.at(i).at(j);
					if (equations.at(i).at(j) < 0)
						min += equations.at(i).at(j);
				}
				if (equations.at(i).at(equations.at(i).size() - 1) == max)
				{
					for (int j = 0; j < width * height; j++)
					{
						if (equations.at(i).at(j) > 0)
						{
							knownValues[j] = 1;
							change = true;
							recentChange = true;
							countingEquationChanges = true;
						}
						else if (equations.at(i).at(j) < 0)
						{
							knownValues[j] = 0;
							equations.push_back(generateEquation(j % width, j / width, numbers->at(j / width).at(j % width)));
							change = true;
							recentChange = true;
							countingEquationChanges = true;
						}
					}
					equations.erase(equations.begin() + i);
				}
				else if (equations.at(i).at(equations.at(i).size() - 1) == min)
				{
					for (int j = 0; j < width * height; j++)
					{
						if (equations.at(i).at(j) > 0)
						{
							knownValues[j] = 0;
							equations.push_back(generateEquation(j % width, j / width, numbers->at(j / width).at(j % width)));
							change = true;
							recentChange = true;
							countingEquationChanges = true;
						}
						else if (equations.at(i).at(j) < 0)
						{
							knownValues[j] = 1;
							change = true;
							recentChange = true;
							countingEquationChanges = true;
						}
					}
					equations.erase(equations.begin() + i);
				}
			}
			changesThisItteration += countingEquationChanges;
		}
		if (changesThisItteration > 0)
			score += 1 / float(changesThisItteration);
	}

	if (!custom && score < 1)
	{
		delete (board);
		delete (numbers);
		return false;
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (knownValues[i * width + j] == -1)
			{
				delete (board);
				delete (numbers);
				return false;
			}
		}
	}

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if (ansiCodes)
			{
				if (knownValues[i * width + j] == 0)
					cout << "\x1b[38;5;28m";
				if (knownValues[i * width + j] == 1)
					cout << "\x1b[38;5;125m";
				if (i == startY && j == startX)
					cout << "\x1b[38;5;33m";
			}
			if (!ansiCodes && i == startY && j == startX)
			{
				cout << "#";
			}
			else
			{
				cout << char(board->at(i).at(j) ? '*' : '0' + numbers->at(i).at(j));
			}
			if (ansiCodes)
			{
				cout << "\x1b[0m";
			}
		}
		cout << "\n";
	}
	cout << "Mine Count: " << to_string(placedMines) << "\n";
	cout << "Score: " << to_string(score) << "\n";

	string encodedBoard = "";

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			if ((i * width + j) % 4 == 0)
			{
				encodedBoard += char(0);
			}
			if (board->at(i).at(j))
			{
				encodedBoard[(i * width + j) / 4] += 1 << 3 - ((i * width + j) % 4);
			}
		}
	}

	for (int i = 0; i < encodedBoard.size(); i++)
	{
		encodedBoard[i] = "0123456789abcdef"[encodedBoard[i]];
	}

	cout << "https://www.chiark.greenend.org.uk/~sgtatham/puzzles/js/mines.html#" << to_string(width) << "x" << to_string(height) << ":" << to_string(startX) << "," << to_string(startY) << ",u" << encodedBoard << "\n";

	delete (lastBoard);
	delete (lastNumbers);
	lastBoard = board;
	lastNumbers = numbers;
	lastStartX = startX;
	lastStartY = startY;
	lastPlacedMines = placedMines;
	return true;
}