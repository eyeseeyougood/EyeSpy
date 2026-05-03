#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<raylib.h>
#include<dirent.h>
#include"StringViews.h"

#define WIDTH 4*250
#define HEIGHT 3*250

#define ADDRESSCHARLIMIT 37
#define FILECHARLIMIT 37
#define EXTENSIONCHARLIMIT 4

#define COL(r,g,b) ((Color){r,g,b,255})
#define ADDRESSBGCOL (Color){70, 30, 140, 255}
#define ADDRESSFGCOL (Color){200, 50, 150, 255}
#define LEFTPANELBGCOL (Color){24, 15, 24, 255}
#define RIGHTPANELBGCOL (Color){30, 21, 30, 255}
#define SELECTEDBG (Color){20, 100, 165, 255}
#define SELECTEDFG (Color){0, 0, 40, 255}

static char* currentDir;
static int scrollAmountLeft = 0;
static int addressScrollAmount = 0;

static String* selectedPaths;
static int selectedPathCounts;
static int allocatedPathSpace;

void BinPath(String* path)
{
	const char* filename = GetFileName(path->data);

    String* homeDir = NewString(getenv("HOME"));

    String* trashPath = NewString(GetData(homeDir));
    AddData(trashPath, "/.local/share/Trash/files/");
    AddData(trashPath, filename);

	int errCode = rename(GetData(path), GetData(trashPath));
	if (errCode != 0)
	{
    	printf("Failed to bin: '%s'\n", GetData(path));
    	printf("Tried to move to: '%s'\n", GetData(trashPath));
        return;
    }
}

void HandleClick(struct dirent* de)
{
	if (de->d_type == DT_DIR)
	{
		char* dir = malloc(256 + strlen(currentDir) * sizeof(char));
		strcpy(dir, currentDir);
		dir = strcat(dir, "/");
		dir = strcat(dir, de->d_name);
		realpath(dir, dir);
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

bool Button(int x, int y, int width, int height, char* text, Color bg, Color pressed, Color hover)
{
	bool isOver = CheckCollisionPointRec(GetMousePosition(), (Rectangle){x,y,width,height});
	bool isDown = isOver && IsMouseButtonDown(0);
	bool isJustPressed = isOver && IsMouseButtonPressed(0);

	Color col = isDown ? pressed : (isOver ? hover : bg);

	TextLabel(x, y, width, height, text, col, BLACK);

	return isJustPressed;
}

void DrawRightPanel()
{
	Rectangle rect = GetRightPanelRect();

	DrawRectangle(rect.x, rect.y, rect.width, rect.height, RIGHTPANELBGCOL);

	TextLabel(rect.x + 5, rect.y + 5, rect.width - 10, 40, "Eye Spy - by eyeseeyougood", COL(180, 50, 120), BLACK);

	Button(rect.x + 5, rect.y + 60, rect.width/2-10, 40, "Delete", COL(140, 24, 25), COL(60, 10, 10), COL(160, 30, 15));
	Button(rect.x + rect.width/2, rect.y + 60, rect.width/2-5, 40, "New File", COL(140, 24, 90), COL(130, 20, 80), COL(160, 30, 110));
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
	char* result = malloc(strlen(address)*sizeof(char));

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

void DeleteButton()
{
	for (int i = 0; i < selectedPathCounts; i++)
	{
		BinPath(&selectedPaths[i]);
	}
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

	DrawText(TrimAddress(currentDir), panelRect.x+8, 5+fontSize, fontSize, ADDRESSFGCOL);
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

	int extSize = (EXTENSIONCHARLIMIT+1)*sizeof(char);
	char extension[extSize];
	memset(extension, 0, extSize);
	memcpy(extension, name+lastDotPosition, EXTENSIONCHARLIMIT * sizeof(char));

	strcat(finalText, "..");
	strcat(finalText, extension);

	return finalText;
}

void SelectPath(String* path)
{
	if (selectedPathCounts == allocatedPathSpace)
	{
		allocatedPathSpace *= 1.5;
		String* newMemorySpace = realloc(selectedPaths, allocatedPathSpace * sizeof(String));
		free(selectedPaths);
		selectedPaths = newMemorySpace;
	}

	selectedPaths[selectedPathCounts] = *path;
	selectedPathCounts++;
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

    struct dirent *de;

    DIR *dr = opendir(currentDir);

    if (dr == NULL)
    {
        printf("Could not open current directory");
        return;
    }

    struct dirent* unsortedFiles[10000*sizeof(struct dirent*)];
    int fileCount = 0;
    int i = 0;
    while ((de = readdir(dr)) != NULL)
    {
    	if (strcmp(de->d_name, ".") == 0)
    		continue;

    	unsortedFiles[i] = de;
    	fileCount++;
    	i++;
	}

	struct dirent** sortedFiles = SortFiles(unsortedFiles, fileCount);

	for (i = 0; i < fileCount; i++)
	{
		struct dirent* de = sortedFiles[i];
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

    	String* fullpath = NewString(currentDir);
    	AddData(fullpath, "/");
    	AddData(fullpath, de->d_name);


    	for (int i = 0; i < selectedPathCounts; i++)
    	{
    		if (EquateStrings(fullpath, &(selectedPaths[i])))
    		{
    			bg = SELECTEDBG;
    			col = SELECTEDFG;
    		}
    	}

    	DrawRectangle(rect.x, rect.y, rect.width, rect.height, bg);

    	char* finalText = TrimFileName(de->d_name);

    	DrawText(finalText, rect.x+5, rect.y, textHeight, col);

    	free(finalText);

    	if (IsMouseButtonReleased(0) && mouseOver)
    	{
    		HandleClick(de);
    	}

    	if (IsMouseButtonReleased(1) && mouseOver)
    	{
    		SelectPath(fullpath);
    	}

    	DisposeString(fullpath);
    }

    closedir(dr);
}

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "The best file browser in the world!!!!");

	currentDir = "/home/eyeseeyougood/Documents";
	allocatedPathSpace = 20;
	selectedPaths = malloc(allocatedPathSpace);
	selectedPathCounts = 0;

	SelectPath(NewString("/home/eyeseeyougood/Documents/BG.png"));

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