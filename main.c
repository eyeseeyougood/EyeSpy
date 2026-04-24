#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<stdlib.h>
#include<raylib.h>
#include<dirent.h>

#define WIDTH 640
#define HEIGHT 480

static char* currentDir;
static int scrollAmountLeft;

void HandleClick(struct dirent* de)
{
	if (de->d_type == DT_DIR)
	{
		char* dir = malloc(256);
		strcpy(dir, currentDir);
		dir = strcat(dir, "/");
		dir = strcat(dir, de->d_name);
		currentDir = dir;
	}
}

Rectangle GetLeftPanelRect()
{
	int panelX = 0;
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

struct dirent** SortFiles(struct dirent** iNodes, int count)
{
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < count; j++)
		{
			if (iNodes[i]->d_name[0] > iNodes[j]->d_name[0])
			{
				struct dirent* temp = iNodes[i];
				iNodes[i] = iNodes[j];
				iNodes[j] = temp;
			}
		}
	}

	return iNodes;
}

void DrawLeftPanel()
{
	int textOffsetX = 10;
	int textOffsetY = 10 - scrollAmountLeft;
	int textHeight = 20;
	int padding = 5;

	Color panelBG = {24, 15, 24, 255};
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

    struct dirent* unsortedFiles[10000];
    int fileCount = 0;

    int i = 0;
    while ((de = readdir(dr)) != NULL)
    {
    	if (strcmp(de->d_name, ".") == 0)
    		continue;

    	unsortedFiles[i] = de;
    	fileCount++;
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

    	bool mouseOver = CheckCollisionPointRec(GetMousePosition(), rect);

    	Color normalBG = de->d_type == DT_DIR ? folderBG : fileBG;
    	Color hoverBG = de->d_type == DT_DIR ? folderHoverBG : fileHoverBG;
    	Color clickBG = de->d_type == DT_DIR ? folderClickBG : fileClickBG;

    	Color bg = mouseOver ? hoverBG : normalBG;
    	Color col = mouseOver ? fileTextHoverCol : fileTextCol;

    	bool mouseDown = IsMouseButtonDown(0) && mouseOver;

    	bg = mouseDown ? clickBG : bg;
    	col = mouseDown ? fileTextClickCol : col;

    	DrawRectangle(rect.x, rect.y, rect.width, rect.height, bg);
    	DrawText(de->d_name, rect.x+5, rect.y, textHeight, col);

    	if (IsMouseButtonReleased(0) && mouseOver)
    	{
    		HandleClick(de);
    	}
    	i++;
    }

    closedir(dr);
}

int main(void)
{
	InitWindow(WIDTH, HEIGHT, "The best file browser in the world!!!!");

	currentDir = ".";

	while (!WindowShouldClose())
	{
		if (CheckCollisionPointRec(GetMousePosition(), GetLeftPanelRect()))
			scrollAmountLeft -= GetMouseWheelMove()*14;

		BeginDrawing();
		ClearBackground(BLACK);

		DrawLeftPanel();

		EndDrawing();
	}

	return 0;
}