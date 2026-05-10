#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<raylib.h>
#include<dirent.h>
#include"StringFuncs.h"

#define WIDTH 4*250
#define HEIGHT 3*250

#define ADDRESSCHARLIMIT 37
#define FILECHARLIMIT 37
#define EXTENSIONCHARLIMIT 4

#define COL(r,g,b) ((Color){r,g,b,255})

#define PALETTEPURPLE COL(75, 50, 100)
#define PALETTELIGHTPURPLE COL(95, 70, 120)
#define PALETTEDARKPURPLE COL(90, 60, 100)
#define PALETTEDEEPPURPLE COL(24, 15, 24)
#define PALETTEPINK COL(100, 50, 75)
#define PALETTEBRIGHTPINK COL(200, 75, 125)
#define PALETTELIGHTPINK COL(120, 70, 95)
#define PALETTEDARKPINK COL(100, 60, 90)
#define PALETTEDEEPPINK COL(100, 50, 75)

#define PALETTEPURPLEBUTTON PALETTEPURPLE, PALETTEDARKPURPLE, PALETTELIGHTPURPLE
#define PALETTEPINKBUTTON PALETTEPINK, PALETTEDARKPINK, PALETTELIGHTPINK

#define ADDRESSBGCOL PALETTEDARKPINK
#define ADDRESSFGCOL PALETTEBRIGHTPINK
#define LEFTPANELBGCOL PALETTEDEEPPURPLE
#define RIGHTPANELBGCOL COL(30, 21, 30)
#define SELECTEDBG COL(20, 100, 165)
#define SELECTEDFG COL(0, 0, 40)

static char** selectedPaths;
static int selectedPathCount;
static int selectionAllocation;

static char* currentDir;
static int scrollAmountLeft = 0;
static int addressScrollAmount = 0;

bool IsPathSelected(char* path)
{
	for (int i = 0; i < selectedPathCount; i++)
	{
		if (EquateStrings(selectedPaths[i], path))
		{
			return true;
		}
	}

	return false;
}

void SelectPath(char* path)
{
	// check if path is already in selection
	if (IsPathSelected(path))
		return;
	
	// if not already in selection, check if it is full;
	if (selectedPathCount == selectionAllocation)
	{
		// if full we expand the allocation of the array by 1.5x;
		selectionAllocation *= 1.5;

		char** biggerMemory = realloc(selectedPaths, selectionAllocation * sizeof(char*));
		selectedPaths = biggerMemory;
	}

	selectedPaths[selectedPathCount] = CopyString(path);
	selectedPathCount++;
}

void DeselectPath(char* path)
{
	// check if selection is empty;
	if (selectedPathCount == 0)
		return;

	for (int i = 0; i < selectedPathCount; i++)
	{
		if (EquateStrings(selectedPaths[i], path))
		{
			int remainingItems = selectedPathCount - i;

			char* temp[remainingItems];
			memcpy(temp, selectedPaths+i+1, remainingItems * sizeof(char*));
			memcpy(selectedPaths+i, temp, remainingItems * sizeof(char*));
			selectedPathCount--;
		}
	}
}

void BinPath(char* path)
{
	const char* filename = GetFileName(path);

    char* homeDir = getenv("HOME");

    char* temp = CopyString(homeDir);
    char* temp1 = AddData(temp, "/.local/share/Trash/files/");
    char* trashPath = AddData(temp1, filename);

	int errCode = rename(path, trashPath);
	if (errCode != 0)
	{
    	printf("Failed to bin: '%s'\n", path);
    	printf("Tried to move to: '%s'\n", trashPath);
        return;
    }
}

void DeleteButtonClicked()
{
	int count = selectedPathCount;
	for (int i = 0; i < count; i++)
	{
		BinPath(selectedPaths[0]);
		DeselectPath(selectedPaths[0]);
	}
}

void HandleClick(struct dirent* de)
{
	if (de->d_type == DT_DIR)
	{
		char* dir = malloc(256 + strlen(currentDir));
		strcpy(dir, currentDir);
		strcat(dir, "/");
		strcat(dir, de->d_name);
		realpath(dir, dir);

		free(currentDir);
		currentDir = dir;

		scrollAmountLeft = 0;
	}
}

Rectangle GetRightPanelRect()
{
	int panelX = WIDTH * 0.5;
	int panelY = 0;
	int panelWidth = WIDTH * 0.5;
	int panelHeight = HEIGHT - panelY;

	Rectangle rect = {
		panelX,
		panelY,
		panelWidth,
		panelHeight,
	};

	return rect;
}

void TextLabel(int x, int y, int width, int height, char* text, Color bg, Color fg)
{
	DrawRectangle(x, y, width, height, bg);

	int textWidth = MeasureText(text, height-10);
	DrawText(text, x+width/2 - textWidth/2+5, y+5, height-10, fg);
}

bool Button(int x, int y, int width, int height, char* text, Color bg, Color pressed, Color hover, void(*action)(void))
{
	bool isOver = CheckCollisionPointRec(GetMousePosition(), (Rectangle){x,y,width,height});
	bool isDown = isOver && IsMouseButtonDown(0);
	bool isJustPressed = isOver && IsMouseButtonPressed(0);

	Color col = isDown ? pressed : (isOver ? hover : bg);

	TextLabel(x, y, width, height, text, col, BLACK);

	if (isDown && action != NULL)
	{
		action();
	}

	return isJustPressed;
}

void DrawRightPanel()
{
	Rectangle rect = GetRightPanelRect();

	DrawRectangle(rect.x, rect.y, rect.width, rect.height, RIGHTPANELBGCOL);

	TextLabel(rect.x + 5, rect.y + 5, rect.width - 10, 40, "Eye Spy - by eyeseeyougood", PALETTEBRIGHTPINK, BLACK);

	Button(rect.x + 5, rect.y + 60, rect.width/2-10, 40, "Delete", PALETTEPURPLEBUTTON, &DeleteButtonClicked);
	Button(rect.x + rect.width/2, rect.y + 60, rect.width/2-5, 40, "New File", PALETTEPINKBUTTON, NULL);
}

Rectangle GetLeftPanelRect()
{
	int panelX = 0;
	int panelY = 50;
	int panelWidth = WIDTH * 0.5;
	int panelHeight = HEIGHT - panelY;

	Rectangle rect = {
		panelX,
		panelY,
		panelWidth,
		panelHeight,
	};

	return rect;
}

struct dirent** SortFiles(struct dirent** iNodes, int count)
{
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count-1; j++)
		{
			int val = iNodes[j]->d_name[0];
			if ((char)val == '.')
			{
				val = 0;
			}
		
			if (val > iNodes[j+1]->d_name[0])
			{
				struct dirent* temp = iNodes[j];
				iNodes[j] = iNodes[j+1];
				iNodes[j+1] = temp;
			}
		}
	}

	return iNodes;
}

char* TrimAddress(char* address)
{
	char* result = malloc((strlen(address)+1)*sizeof(char));

	int addressLength = strlen(address);

	if (addressLength <= ADDRESSCHARLIMIT)
	{
		strcpy(result, address);
		return result;
	}

	int addressOffset = addressLength - ADDRESSCHARLIMIT;
	if (addressOffset < 0)
	{
		addressScrollAmount -= addressOffset;
		addressOffset = 0;
	}
	if (-addressScrollAmount > addressOffset)
	{
		addressScrollAmount = -addressOffset;
	}
	if (addressScrollAmount > 0)
	{
		addressScrollAmount = 0;
	}

	strcpy(result, address+addressScrollAmount + addressOffset);
	memset(result + (ADDRESSCHARLIMIT+1)*sizeof(char), 0, sizeof(char));

	return result;
}

Rectangle GetAddressPanelRect()
{
	Rectangle leftRect = GetLeftPanelRect();

	Rectangle rect = {
		0,
		0,
		leftRect.width,
		50,
	};

	return rect;
}

void DrawAddressBar()
{
	Rectangle panelRect = GetAddressPanelRect();

	int inset = 5;
	int fontSize = 20;

	DrawRectangle(panelRect.x, panelRect.y, panelRect.width, panelRect.height, ADDRESSBGCOL);
	DrawRectangle(panelRect.x+inset, panelRect.y+inset, panelRect.width-inset*2, panelRect.height-inset*2, BLACK);

	char* trimmedAddress = TrimAddress(currentDir);

	DrawText(trimmedAddress, panelRect.x+8, 5+fontSize, fontSize, ADDRESSFGCOL);

	free(trimmedAddress);
}

char* TrimFileName(char* name)
{
	int finalTextSize = (FILECHARLIMIT+EXTENSIONCHARLIMIT+2+1)*sizeof(char);
	char* finalText = malloc(finalTextSize);
	memset(finalText, 0, finalTextSize);

	memcpy(finalText, name, FILECHARLIMIT * sizeof(char));

	int nameLength = strlen(name);
	int lastDotPosition = 0;
	for (int i = 0; i < nameLength; i++)
	{
		if (name[i] == '.')
			lastDotPosition = i;
	}

	if (lastDotPosition == 0)
	{
		return finalText;
	}

	if (nameLength <= FILECHARLIMIT)
	{
		return finalText;
	}

	int extSize = EXTENSIONCHARLIMIT + 1;
	char extension[extSize];
	memset(extension, 0, extSize);
	memcpy(extension, name+lastDotPosition, EXTENSIONCHARLIMIT);

	strcat(finalText, "..");
	strcat(finalText, extension);

	return finalText;
}

struct dirent** GetFilesFromDirectory(DIR* dir, char* directoryName, int* outFileCount)
{
	struct dirent *de;

    if (dir == NULL)
    {
        printf("Could not open current directory");
        return NULL;
    }

    uint fileCount = GetDirectoryFileCount(directoryName);
    outFileCount[0] = fileCount;

    struct dirent** unsortedFiles = malloc((fileCount+2) * sizeof(struct dirent*));
    memset(unsortedFiles, 0, fileCount * sizeof(struct dirent*));

    int i = 0;
    while ((de = readdir(dir)) != NULL)
    {
    	if (strcmp(de->d_name, ".") == 0)
    		continue;

    	unsortedFiles[i] = de;
    	i++;
	}

	struct dirent** sortedFiles = SortFiles(unsortedFiles, fileCount);

	return sortedFiles;
}

void DrawLeftPanel()
{
	int textOffsetX = 10;
	int textOffsetY = 10 - scrollAmountLeft;
	int textHeight = 20;
	int padding = 5;

	Color panelBG = LEFTPANELBGCOL;
	Color fileBG = {100, 50, 75, 255};
	Color fileHoverBG = {120, 70, 95, 255};
	Color fileClickBG = {100, 60, 90, 255};

	Color folderBG = {75, 50, 100, 255};
	Color folderHoverBG = {95, 70, 120, 255};
	Color folderClickBG = {90, 60, 100, 255};

	Color fileTextCol = {200, 75, 125, 255};
	Color fileTextHoverCol = {220, 95, 145, 255};
	Color fileTextClickCol = {180, 55, 105, 255};

	Rectangle panelRect = GetLeftPanelRect();
	DrawRectangle(panelRect.x, panelRect.y, panelRect.width, panelRect.height, panelBG);

	int fileCount = 0;
	DIR* directory = opendir(currentDir);
	struct dirent** sortedFiles = GetFilesFromDirectory(directory, currentDir, &fileCount);

	for (int i = 0; i < fileCount; i++)
	{
		struct dirent* de = sortedFiles[i];

		char* cD = CopyString(currentDir);
		char* cDWithSlash = AddData(cD, "/");
		char* fullpath = AddData(cDWithSlash, de->d_name);

    	Rectangle rect = {
    		panelRect.x+textOffsetX,
    		panelRect.y+textOffsetY + i*(textHeight+padding),
    		panelRect.width - textOffsetX * 2,
    		textHeight,
    	};

    	if (rect.y < panelRect.y)
    		continue;

    	bool mouseOver = CheckCollisionPointRec(GetMousePosition(), rect);

    	Color normalBG = de->d_type == DT_DIR ? folderBG : fileBG;
    	Color hoverBG = de->d_type == DT_DIR ? folderHoverBG : fileHoverBG;
    	Color clickBG = de->d_type == DT_DIR ? folderClickBG : fileClickBG;


    	Color bg = mouseOver ? hoverBG : normalBG;
    	Color col = mouseOver ? fileTextHoverCol : fileTextCol;

    	bool mouseDown = IsMouseButtonDown(0) && mouseOver;

    	bg = mouseDown ? clickBG : bg;
    	col = mouseDown ? fileTextClickCol : col;

    	if (IsPathSelected(fullpath))
    	{
    		bg = SELECTEDBG;
    		col = SELECTEDFG;
    	}


    	DrawRectangle(rect.x, rect.y, rect.width, rect.height, bg);

    	char* finalText = TrimFileName(de->d_name);

    	DrawText(finalText, rect.x+5, rect.y, textHeight, col);

    	free(finalText);

    	if (IsMouseButtonReleased(0) && mouseOver)
    	{
    		HandleClick(de);
    	}
    	else if (IsKeyPressed(KEY_ESCAPE) && EquateStrings(de->d_name, ".."))
    	{
    		HandleClick(de);
    	}
    	else if (IsMouseButtonPressed(1) && mouseOver && !EquateStrings(de->d_name, ".."))
    	{
    		if (IsKeyDown(KEY_LEFT_SHIFT))
    		{
    			DeselectPath(fullpath);
    		}
    		else
    		{
    			SelectPath(fullpath);
    		}
    	}

    	free(fullpath);
    }

    closedir(directory);
    free(sortedFiles);
}

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "The best file browser in the world!!!!");

	SetExitKey(KEY_NULL);

	char* startPath = "/home/eyeseeyougood/Documents";
	currentDir = malloc(strlen(startPath)+1);
	strcpy(currentDir, startPath);

	selectionAllocation = 2;
	selectedPathCount = 0;
	selectedPaths = malloc(selectionAllocation * sizeof(char*));

	while (!WindowShouldClose())
	{
		if (CheckCollisionPointRec(GetMousePosition(), GetLeftPanelRect()))
			scrollAmountLeft -= GetMouseWheelMove()*14;
		if (CheckCollisionPointRec(GetMousePosition(), GetAddressPanelRect()))
			addressScrollAmount -= GetMouseWheelMove();

		BeginDrawing();
		ClearBackground(BLACK);

		DrawAddressBar();
		DrawLeftPanel();
		DrawRightPanel();

		EndDrawing();
	}

	return 0;
}