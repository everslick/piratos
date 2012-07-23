#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include <cli.h>

// Gloal Values
static int stage[21][12] = {0};
static int block[4][4] = {
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0},
	{0,0,0,0}
};
static int field[21][12] = {0};
static int y, x = 4;
static int gameover, oneline, twoline, threeline, fourline;

static int iterations_before_drop=200000; //default level

static CLI *cli;

/* block data */
static int block_list[7][4][4] = { {
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0},
	{0,1,0,0}
}, {
	{0,0,0,0},
	{0,1,1,0},
	{0,1,0,0},
	{0,1,0,0}
}, {
	{0,0,1,0},
	{0,1,1,0},
	{0,1,0,0},
	{0,0,0,0}
}, {
	{0,1,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,0,0}
}, {
	{0,0,0,0},
	{0,1,0,0},
	{1,1,1,0},
	{0,0,0,0}
}, {
	{0,0,0,0},
	{0,1,1,0},
	{0,1,1,0},
	{0,0,0,0}
}, {
	{0,0,0,0},
	{0,1,1,0},
	{0,0,1,0},
	{0,0,1,0}
} };

void level_change();
void game_over();
 int title_show();
void game_start();
void field_display();
 int blocks_create();
void first_clean();
void block_move(int, int);
void block_land();
 int check_over(int, int);
void keyboard_read();
 int block_rotate();
void lines_check();
void delete_blocks();

int
getkey(int blocking) {
	if (blocking) {
		int k = -1;

		while (k == -1) {
			//k = getc(stdin);
			usleep(10000);
		}

		return (k);
	}

	//return (getc(stdin));

	return (0);
}

void
cls(void) {
	printf("\033[J");
}

void
cmd_tetris_help(CLI *cli) {
   printf("'tetris' classic tetris game for the text console.\n");
}

int
cmd_tetris_exec(CLI *_cli, char *argv) {
	int quit = 0;

	cli = _cli;

	while (!quit) {
		int menu = title_show();

		switch (menu){
			case 49:  game_start();   break;
			case 50:  level_change(); break;
			case 51:  quit = 1;       break;
			default: printf("Choose 1-3\n");
		}
	}

	return (0);
}

void
game_over() {
	printf(" #####     #    #     # ####### ####### #     # ####### ######\n");
	printf("#     #   # #   ##   ## #       #     # #     # #       #     #\n");
	printf("#        #   #  # # # # #       #     # #     # #       #     #\n");
	printf("#  #### #     # #  #  # #####   #     # #     # #####   ######\n");
	printf("#     # ####### #     # #       #     #  #   #  #       #   #\n");
	printf("#     # #     # #     # #       #     #   # #   #       #    #\n");
	printf(" #####  #     # #     # ####### #######    #    ####### #     #\n");
	printf("\n\nPress any key\n");

	getkey(1);
}

void
level_change() {
	int n = 1, ch_level, ch_num, ans;

	cls();

	printf("LEVEL\n");
	printf("\n\n");
	printf("1: Easy   2: Normal   3: Hard   4: Very Hard   5: Impossible\n");

	ch_level = getkey(1);

	switch (ch_level) {
		case 49: ch_num = 270000; break;
		case 50: ch_num = 200000; break;
		case 51: ch_num = 100000; break;
		case 52: ch_num =  50000; break;
		case 53: ch_num =   5000; break;
	}
	
	iterations_before_drop = ch_num;
}

void
game_start() {
	int iteration = 0; 

	first_clean(); 

	gameover = 0;

	while (!gameover) {
		keyboard_read();

		if (iteration < iterations_before_drop){
			iteration++;
		} else {
			delete_blocks();    
			iteration = 0;
		}
	}
}

int
title_show() {
	cls();

	printf("####### ####### ####### ######    ###    #####\n");
	printf("   #    #          #    #     #    #    #     #\n");
	printf("   #    #          #    #     #    #    #\n");
	printf("   #    #####      #    ######     #     #####\n");
	printf("   #    #          #    #   #      #          #\n");
	printf("   #    #          #    #    #     #    #     #\n");
	printf("   #    #######    #    #     #   ###    #####\n");
	printf("\n\n");
	printf("1: Start Game   2: Level Change   3: Quit\n");

	return (getkey(1));
}
 
void
field_display() {
	cls();

	for (int i=0; i<21; i++) {
		for (int j=0; j<12; j++) {
			switch (field[i][j]) {
				case 0:  printf(" "); break;
				case 9:  printf("."); break;
				default: printf("#"); break;
			}
		}

		printf("\n");
	}

	printf("\nScore: %i\t%i\t%i\t%i\n", oneline, twoline, threeline, fourline);

	if (gameover) {
		cls();

		game_over();
	}
}

void
first_clean() {
	for (int i=0; i<=20; i++) {
		for (int j=0; j<=11; j++) {
			if ((j == 0) || (j == 11) || (i == 20)) {
				field[i][j] = stage[i][j] = 9;
			} else {
				field[i][j] = stage[i][j] = 0;
			}
		}
	}

	blocks_create();
	field_display();
}

int
blocks_create() {
	int i,j;
	int block_type;
  
	x = 4;
	y = 0;
 
	srand((unsigned)time(NULL));
	block_type = rand() % 7;
 
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			block[i][j] = 0;
			block[i][j] = block_list[block_type][i][j];
		}
	}
  
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			field[i][j+4] = stage[i][j+4] + block[i][j];
			if (field[i][j+4] > 1) {
				gameover = 1;

				return (1);
			}
		}
	}

	return (0);
}

void
block_move(int x2, int y2) {
	int i, j; 

	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			field[y+i][x+j] -= block[i][j];
		}
	}

	x = x2;
	y = y2;

	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			field[y+i][x+j] += block[i][j];
		}
	}

	field_display();
}

void
block_land() {
	int i, j;

	for (i=0; i<21; i++) {
		for (j=0; j<12; j++) {
			stage[i][j] = field[i][j];
		}
	}

	lines_check();

	for (i=0; i<21; i++) {
		for (j=0; j<12; j++) {
			field[i][j] = stage[i][j];
		}
	}
}

int
check_over(int x2, int y2) {
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			if (block[i][j]) {
				if (stage[y2 + i][x2 + j] != 0) {
					return (1);
				}
			}
		}
	}

	return (0);
}

void
keyboard_read() {
	char key = getkey(0);

	switch (key) {
		case 67: if (!check_over(x + 1, y)) block_move(x + 1, y); break;
		case 68: if (!check_over(x - 1, y)) block_move(x - 1, y); break;
		case 66: if (!check_over(x, y + 1)) block_move(x, y + 1); break;
		case 65: block_rotate(); break;
	}
}

int
block_rotate() {
	int i, j, tmp[4][4]={0};
 
	for (i=0; i<4; i++) {
   	for (j=0; j<4; j++) {
      	tmp[i][j] = block[i][j];
		}
	}
 
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			block[i][j] = tmp[3-j][i];
		}
	}

	if (check_over(x,y)) {
		for (i=0; i<4; i++) {
			for (j=0; j<4; j++) {
				block[i][j] = tmp[i][j];
			}
		}

		return (1);
	}
 
	for (i=0; i<4; i++) {
		for (j=0; j<4; j++) {
			field[y+i][x+j] -= tmp[i][j];
			field[y+i][x+j] += block[i][j];
		}
	}

	field_display();

	return (0);
}

void
lines_check() {
	int i, j, k, comp, lines = 0;
 
	while (1) {
		for (i=0; i<20; i++) {
			comp = 1;

			for (j=1; j<11; j++) {
				if (stage[i][j] == 0) {
					comp = 0;
				}
			}

			if (comp == 1) break;
		}

		if (comp == 0) break;
	
		lines++;
	
		for (j=1; j<11; j++) {
			stage[i][j] = 0;
		}
	
		for (k=i; k>0; k--) {
			for (j=1; j<11; j++) {
				stage[k][j] = stage[k-1][j];
			}
		}
	}

	switch (lines) {
		case 1: oneline++;   break;
		case 2: twoline++;   break;
		case 3: threeline++; break;
		case 4: fourline++;  break;
		default: break;
	}
}

void
delete_blocks() {
	if (!check_over(x, y + 1)) {
		block_move(x, y + 1);
	} else {
		block_land();
		blocks_create();
		field_display();
	}
}
