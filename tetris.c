#include "tetris.h"

static struct sigaction act, oact;

int main(){
	int exit=0;

	initscr();
	noecho();
	keypad(stdscr, TRUE);	

	srand((unsigned int)time(NULL));
	createRankList();
	recRoot = (RecNode*)calloc(1, sizeof(RecNode));
	recRoot->lv = -1;
	recRoot->score = 0;
	recRoot->f = field;
	while(!exit){
		clear();
		switch(menu()){
		case MENU_PLAY: play(); break;
		case MENU_EXIT: exit=1; break;
		case MENU_RANK: rank(); break;
		case MENU_REC_PLAY: recommendedPlay(); break;
		default: break;
		}
	}
	writeRankFile();
	endwin();
	system("clear");
	return 0;
}

void InitTetris(){
	int i,j;
	for(j=0;j<HEIGHT;j++)
		for(i=0;i<WIDTH;i++)
			field[j][i]=0;

	for (i = 0; i < VISIBLE_BLOCKS; i++)
	{
		nextBlock[i] = rand() % 7;
	}
	blockRotate=0;
	blockY=-1;
	blockX=WIDTH/2-2;
	score=0;	
	gameOver=0;
	timed_out=0;

	DrawOutline();
	DrawField();
	modified_recommend(recRoot);
	//recommend(recRoot);
	//DrawBlockWithFeatures(blockY,blockX,nextBlock[0],blockRotate);
	DrawNextBlock(nextBlock);
	PrintScore(score);
}

void DrawOutline(){	
	int i,j;
	/* 블럭이 떨어지는 공간의 태두리를 그린다.*/
	DrawBox(0,0,HEIGHT,WIDTH);

	/* next block을 보여주는 공간의 태두리를 그린다.*/
	move(2,WIDTH+10);
	printw("NEXT BLOCK 1");
	DrawBox(3,WIDTH+10,4,8);

	/* next block 2을 보여주는 공간의 태두리를 그린다.*/
	move(9, WIDTH + 10);
	printw("NEXT BLOCK 2");
	DrawBox(10, WIDTH + 10, 4, 8);

	/* score를 보여주는 공간의 태두리를 그린다.*/
	move(16,WIDTH+10);
	printw("SCORE");
	DrawBox(17,WIDTH+10,1,8);
}

int GetCommand(){
	int command;
	command = wgetch(stdscr);
	switch(command){
	case KEY_UP:
		break;
	case KEY_DOWN:
		break;
	case KEY_LEFT:
		break;
	case KEY_RIGHT:
		break;
	case ' ':	/* space key*/
		/*fall block*/
		break;
	case 'q':
	case 'Q':
		command = QUIT;
		break;
	default:
		command = NOTHING;
		break;
	}
	return command;
}

int ProcessCommand(int command){
	int ret=1;
	int drawFlag=0;
	switch(command){
	case QUIT:
		ret = QUIT;
		break;
	case KEY_UP:
		if((drawFlag = CheckToMove(field,nextBlock[0],(blockRotate+1)%4,blockY,blockX)))
			blockRotate=(blockRotate+1)%4;
		break;
	case KEY_DOWN:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)))
			blockY++;
		break;
	case KEY_RIGHT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX+1)))
			blockX++;
		break;
	case KEY_LEFT:
		if((drawFlag = CheckToMove(field,nextBlock[0],blockRotate,blockY,blockX-1)))
			blockX--;
		break;
	default:
		break;
	}
	if(drawFlag) DrawChange(field,command,nextBlock[0],blockRotate,blockY,blockX);
	return ret;	
}

void DrawField(){
	int i,j;
	for(j=0;j<HEIGHT;j++){
		move(j+1,1);
		for(i=0;i<WIDTH;i++){
			if(field[j][i]==1){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(".");
		}
	}
}


void PrintScore(int score){
	move(18,WIDTH+11);
	printw("%8d",score);
}

void DrawNextBlock(int *nextBlock){
	int i, j;
	for( i = 0; i < 4; i++ ){
		move(4+i,WIDTH+13);
		for( j = 0; j < 4; j++ ){
			if( block[nextBlock[1]][0][i][j] == 1 ){
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
	for (i = 0; i < 4; i++) {
		move(11 + i, WIDTH + 13);
		for (j = 0; j < 4; j++) {
			if (block[nextBlock[2]][0][i][j] == 1) {
				attron(A_REVERSE);
				printw(" ");
				attroff(A_REVERSE);
			}
			else printw(" ");
		}
	}
}

void DrawBlock(int y, int x, int blockID,int blockRotate,char tile){
	int i,j;
	for(i=0;i<4;i++)
		for(j=0;j<4;j++){
			if(block[blockID][blockRotate][i][j]==1 && i+y>=0){
				move(i+y+1,j+x+1);
				attron(A_REVERSE);
				printw("%c",tile);
				attroff(A_REVERSE);
			}
		}

	move(HEIGHT,WIDTH+10);
}

void DrawBox(int y,int x, int height, int width){
	int i,j;
	move(y,x);
	addch(ACS_ULCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_URCORNER);
	for(j=0;j<height;j++){
		move(y+j+1,x);
		addch(ACS_VLINE);
		move(y+j+1,x+width+1);
		addch(ACS_VLINE);
	}
	move(y+j+1,x);
	addch(ACS_LLCORNER);
	for(i=0;i<width;i++)
		addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void play(){
	int command;
	clear();
	act.sa_handler = BlockDown;
	sigaction(SIGALRM,&act,&oact);
	InitTetris();
	DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	do{
		if(timed_out==0){
			alarm(1);
			timed_out=1;
		}

		command = GetCommand();
		if(ProcessCommand(command)==QUIT){
			alarm(0);
			DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
			move(HEIGHT/2,WIDTH/2-4);
			printw("Good-bye!!");
			refresh();
			getch();

			return;
		}
	}while(!gameOver);

	alarm(0);
	getch();
	DrawBox(HEIGHT/2-1,WIDTH/2-5,1,10);
	move(HEIGHT/2,WIDTH/2-4);
	printw("GameOver!!");
	refresh();
	getch();
	newRank(score);
}

char menu(){
	printw("1. play\n");
	printw("2. rank\n");
	printw("3. recommended play\n");
	printw("4. exit\n");
	return wgetch(stdscr);
}

int CheckToMove(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	
	int i, j;
	for(i=0;i<4;i++){	
		for(j=0;j<4;j++){
			if(block[currentBlock][blockRotate][i][j]==1){
				if(f[blockY+i][blockX+j]) return 0;
				if(blockY+i>=HEIGHT) return 0;
				if(blockX+j>=WIDTH) return 0;
				if(blockX+j<0) return 0;
			}
		}
	}
	return 1;
	// user code
}

void DrawChange(char f[HEIGHT][WIDTH],int command,int currentBlock,int blockRotate, int blockY, int blockX){
	
	int i, j, shadY;
	int rotate=blockRotate, y=blockY, x=blockX;
	switch(command){
	case KEY_UP: rotate=(rotate+3)%4; break;
	case KEY_DOWN: y--; break;
	case KEY_RIGHT: x--; break;
	case KEY_LEFT: x++; break;
	}
	shadY = y;
	while (CheckToMove(f, currentBlock, rotate, shadY + 1, x))
	{
		shadY++;
	}
	for(i=0;i<4;i++){
		for(j=0;j<4;j++){
			if(block[currentBlock][rotate][i][j]==1){
				if(i+y>=0){
				move(y+i+1, x+j+1);
				printw(".");
				}
				if (i + shadY >= 0) {
					move(shadY + i + 1, x + j + 1);
					printw(".");
				}
			}
		}
	}
	
	DrawBlockWithFeatures(blockY, blockX, currentBlock,blockRotate);
	move(HEIGHT+10,WIDTH+50);
	return;
	// user code
}

void BlockDown(int sig){
	int i;
	if(CheckToMove(field,nextBlock[0],blockRotate,blockY+1,blockX)){
		blockY++;
		DrawChange(field,KEY_DOWN,nextBlock[0], blockRotate, blockY, blockX);
	}
	else{
		if(blockY==-1) gameOver=1;
		score= score+ AddBlockToField(field, nextBlock[0], blockRotate, blockY, blockX);
		score = score + DeleteLine(field);
		
		for (i = 0; i < VISIBLE_BLOCKS - 1; i++)
		{
			nextBlock[i] = nextBlock[i+1];
		}
		nextBlock[VISIBLE_BLOCKS - 1] = rand() % 7;
		blockRotate=0;
		blockY=-1;
		blockX=WIDTH/2-2;
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		
		recommend(recRoot);
		DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
	}
	timed_out=0;
	return;
	// user code
}

int AddBlockToField(char f[HEIGHT][WIDTH],int currentBlock,int blockRotate, int blockY, int blockX){
	
	int i, j, touched=0;
	for(i=0;i<4;i++){
		for(j=0;j<4;j++)
		{
			if (block[currentBlock][blockRotate][i][j] == 1) {
				f[blockY + i][blockX + j] = 1;
				if (blockY + 1 + i == HEIGHT || f[blockY + i + 1][blockX + j] == 1)
					touched++;
			}
		}
	}
	return touched*10;
	// user code
}

int DeleteLine(char f[HEIGHT][WIDTH]){
	
	int i, j, k;
	int flag;
	int count=0;
	for(i=0;i<HEIGHT;i++){
		flag=1;
		for(j=0;j<WIDTH;j++){
			if(f[i][j]==0){
				flag=0; break;}
		}
		if(flag==1){
			count++;
			for(k=i;k>0;k--){
				for(j=0;j<WIDTH;j++){
					f[k][j]=f[k-1][j];
				}
			}
		}
	}
	return count*count*100;
	// user code
}

void DrawShadow(int y, int x, int blockID,int blockRotate){
	int shadY = blockY;
	while (CheckToMove(field, blockID, blockRotate, shadY + 1, blockX))
	{
		shadY++;
	}
	DrawBlock(shadY, x, blockID, blockRotate, '/');
	return;
	// user code
}

void DrawBlockWithFeatures(int y, int x, int blockID, int blockRotate) {
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	DrawBlock(y, x, blockID, blockRotate, ' ');
	DrawShadow(y, x, blockID, blockRotate);
	return;
}

void createRankList(){
	FILE* fp;
	
	int i;
	if ((fp = fopen("rank.txt", "r")) == NULL)
	{
		root = NULL;
		return;
	}
	fscanf(fp, "%d", &rank_num);
	ranks=(Node*)malloc(sizeof(Node)*rank_num);
	for (i = 0; i < rank_num; i++)
	{
		fscanf(fp, "%s %d", ranks[i].name, &ranks[i].score);
	}
	fclose(fp);
	root=makeBST(ranks, 0, rank_num-1);
	free(ranks);
	return;
	// user code
}

Node* makeBST(Node* curr_ranks, int first, int last){
	int mid=(last+first)/2;
	Node* temp;
	if(last<first)
		return NULL;
	temp=(Node*)malloc(sizeof(Node));
	*temp=curr_ranks[mid];
	temp->left=makeBST(curr_ranks, first, mid-1);
	temp->right=makeBST(curr_ranks, mid+1, last);
	return temp;
}

void rank(){
	int x=-1, y=-1;
	ranking=0;
	int delRank;
	searchFlag = 0;
	delFlag = 0;
	char name[NAMELEN];
	clear();
	printw("1. List ranks from X to Y\n");
	printw("2. List ranks by a specific name\n");
	printw("3. Delete a specific rank\n");
	switch (wgetch(stdscr)) {
	case '1':
		echo();
		printw("X: ");
		scanw("%d", &x);
		printw("Y: ");
		scanw("%d", &y);
		noecho();
		printw("rank | name             | score\n");
		printw("----------------------------------\n");
		if (x == -1) x = 1;
		if (y == -1) y = x+4; 
		if (x > y){
			printw("search failure: no rank in the list");
			break;
		}
		showRanks(root, x, y);
		break;
	case '2':
		echo();
		printw("Input the name: ");
		getstr(name);
		noecho();
		printw("rank | name             | score\n");
		printw("----------------------------------\n");
		searchByName(root, name);
		if (searchFlag == 0)
		{
			printw("search failure: no name in the list\n");
		}
		break;
	case '3':
		echo();
		printw("Input the rank: ");
		scanw("%d", &delRank);
		noecho();
		if (delRank > rank_num)
		{
			printw("search failure: the rank not in the list\n");
			break;
		}
		root = delRanks(root, delRank);
		if (delFlag == 0)
		{
			printw("search failure: the rank not in the list\n");
		}
		else {
			printw("result: the rank deleted\n");
			rank_num--;
		}
		break;

	default: break;
	}
	getch();
	// user code
}

void showRanks(Node* node, int x, int y){
	if(node==NULL)
		return;
	showRanks(node->left, x, y);
	ranking++;
	if(ranking>y) return;
	if(ranking>=x && ranking<=y)
		printw("%4d | %-16s | %d \n", ranking, node->name, node->score);
	showRanks(node->right, x, y);
	return;
}

Node* delRanks(Node* node, int delRank) {
	if (node == NULL) return node;
	if (delFlag == 1) return node;

	node->left = delRanks(node->left, delRank);
	ranking++;
	if (ranking == delRank)
	{
		delFlag = 1;
		if (node->left == NULL && node->right == NULL)
		{
			free(node);
			node = NULL;
			return node;
		}
		else if (node->left == NULL)
		{
			Node* temp = node->right;
			free(node);
			node = NULL;
			return temp;
		}
		else if (node->right == NULL)
		{
			Node* temp = node->left;
			free(node);
			node = NULL;
			return temp;
		}
		else {
			Node* succParent = node->right;
			Node* succ = node->right;
			while (succ->left != NULL) {
				succParent = succ;
				succ = succ->left;
			}
			if (succParent == succ)
			{
				succParent->left = node->left;
				free(node);
				node = NULL;
				return succParent;
			}
			else {
				succParent->left = succ->right;
				strncpy(node->name, succ->name, NAMELEN);
				node->score = succ->score;
				succ = NULL;
				free(succ);
				return node;
			}
		}
	}
	node->right = delRanks(node->right, delRank);
	return node;

}

void searchByName(Node* node, char name[]) {
	if (node == NULL) return;
	searchByName(node->left, name);
	ranking++;
	if (strcmp(node->name, name)==0)
	{
		printw("%4d | %-16s | %d \n", ranking, node->name, node->score);
		searchFlag = 1;
	}
	searchByName(node->right, name);
	return;

}

void writeRankFile(){
	FILE* fp;
	fp = fopen("rank.txt", "w");
	fprintf(fp, "%d\n", rank_num);
	printRank(fp, root);
	fclose(fp);
	return;
	// user code
}

void printRank(FILE* fp, Node* node){

	if(node==NULL) return;
	printRank(fp, node->left);
	fprintf(fp, "%s %d\n", node->name, node->score);
	printRank(fp, node->right);
	return;

}

void newRank(int score){
	char name[NAMELEN];
	Node* newNode;
	clear();
	newNode = (Node*)malloc(sizeof(Node));
	printw("your name: ");
	echo();
	getstr(newNode->name);
	noecho();
	newNode->score = score;
	newNode->left=newNode->right=NULL;
	root = InsertNode(root, newNode);
	rank_num++;
	return;

	// user code
}
Node* InsertNode(Node* roots, Node* newNode){
	
	if(roots==NULL) return newNode;
	if(newNode->score > roots->score)
		roots->left = InsertNode(roots->left, newNode);
	else roots->right = InsertNode(roots->right, newNode);

		return roots;
}

void DrawRecommend(int y, int x, int blockID,int blockRotate){
	if (CheckToMove(field, blockID, blockRotate, y, x))
		DrawBlock(y, x, blockID, blockRotate, 'R');
	// user code
}

int recommend(RecNode *root){
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int r, i = 0, totalR = NUM_OF_ROTATE;
	int h, w, eval = 0;
	int recommended = 0, bestR, bestY, bestX;
	int y = -1;
	int x = 5;
	int lv = root->lv + 1;
	RecNode **c = root->c;

	if (nextBlock[lv] == 0 || nextBlock[lv] == 5 || nextBlock[lv] == 6)
		totalR = 2;
	else if (nextBlock[lv] == 4)
		totalR = 1;

	for (r = 0; r < totalR; r++)
	{
		while (CheckToMove(root->f, nextBlock[lv], r, y, x - 1))
		{
			x--;
		}

		do {
			if (c[i] == NULL) {
				c[i] = (RecNode*)calloc(1, sizeof(RecNode));
				c[i]->lv = lv;
				c[i]->f = (char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);

			}

			c[i]->score = 0;

			for (h = 0; h < HEIGHT; h++) {
				for (w = 0; w < WIDTH; w++) {
					c[i]->f[h][w] = root->f[h][w];
				}
			}

			while (CheckToMove(c[i]->f, nextBlock[lv], r, y + 1, x))
			{
				y++;
			}


			c[i]->score = root->score + AddBlockToField(c[i]->f, nextBlock[lv], r, y, x) + DeleteLine(c[i]->f);
			if (lv < VISIBLE_BLOCKS - 1) {
				eval = recommend(c[i]);
			}
			else {
				eval = c[i]->score;
			}
			if (max <= eval) {
				recommended = 1;
				max = eval;
				bestR = r;
				bestY = y;
				bestX = x;
			}

			x++;
			y = -1;
			i++;
		} while (CheckToMove(root->f, nextBlock[lv], r, y, x));
	}
	if (lv == 0 && recommended) {
		recommendR = bestR;
		recommendY = bestY;
		recommendX = bestX;
	}
	// user code

	return max;
}

int modified_recommend(RecNode *root) {
	int max = 0; // 미리 보이는 블럭의 추천 배치까지 고려했을 때 얻을 수 있는 최대 점수
	int r, i = 0, totalR = NUM_OF_ROTATE;
	int h, w, eval = 0;
	int recommended = 0, bestR, bestY = -1, bestX;
	int y = -1;
	int x = WIDTH / 2 - 2;
	int lv = root->lv + 1;
	RecNode **c = root->c;

	if (nextBlock[lv] == 0 || nextBlock[lv] == 5 || nextBlock[lv] == 6)
		totalR = 2;
	else if (nextBlock[lv] == 4)
		totalR = 1;

	for (r = 0; r < totalR; r++)
	{
		x = WIDTH / 2 - 2;
		while (CheckToMove(root->f, nextBlock[lv], r, y, x - 1))
		{
			x--;
		}

		do {
			if (c[i] == NULL) {
				c[i] = (RecNode*)calloc(1, sizeof(RecNode));
				c[i]->lv = lv;
				c[i]->f = (char(*)[WIDTH])malloc(sizeof(char)*HEIGHT*WIDTH);

			}

			c[i]->score = 0;

			

			while (CheckToMove(root->f, nextBlock[lv], r, y + 1, x))
			{
				y++;
			}
			if (bestY > y)
			{
				x++;
				y = -1;
				continue;
			}
			for (h = 0; h < HEIGHT; h++) {
				for (w = 0; w < WIDTH; w++) {
					c[i]->f[h][w] = root->f[h][w];
				}
			}

			c[i]->score = root->score + AddBlockToField(c[i]->f, nextBlock[lv], r, y, x) + DeleteLine(c[i]->f);
			if (lv < VISIBLE_BLOCKS - 1) {
				eval = recommend(c[i]);
			}
			else {
				eval = c[i]->score;
			}
			if (max <= eval && bestY <= y) {
			//if(max <= eval){
				recommended = 1;
				max = eval;
				bestR = r;
				bestY = y;
				bestX = x;
			}

			x++;
			y = -1;
			i++;
		} while (CheckToMove(root->f, nextBlock[lv], r, y, x));
	}
	if (lv == 0 && recommended) {
		recommendR = bestR;
		recommendY = bestY;
		recommendX = bestX;
	}
	// user code

	return max;
}

void recField(int sig) {
	int i;
	if (!CheckToMove(field, nextBlock[0], blockRotate, blockY+1, blockX)) gameOver = 1;
	else {
		score = score + AddBlockToField(field, nextBlock[0], recommendR, recommendY, recommendX);
		score = score + DeleteLine(field);
		blockY = -1;
		blockX = (WIDTH / 2) - 2;
		blockRotate = 0;
		for (i = 0; i < VISIBLE_BLOCKS - 1; i++) {
			nextBlock[i] = nextBlock[i + 1];
		}
		nextBlock[VISIBLE_BLOCKS - 1] = rand() % 7;
		modified_recommend(recRoot);
		//recommend(recRoot);
		DrawNextBlock(nextBlock);
		PrintScore(score);
		DrawField();
		DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
		DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
		//DrawBlockWithFeatures(blockY, blockX, nextBlock[0], blockRotate);
		timed_out = 0;
	}

}

void recommendedPlay(){
	int command;
	clear();
	act.sa_handler = recField;
	sigaction(SIGALRM, &act, &oact);
	start = time(NULL);
	InitTetris();
	DrawBlock(blockY, blockX, nextBlock[0], blockRotate, ' ');
	DrawRecommend(recommendY, recommendX, nextBlock[0], recommendR);
	do {
		if (timed_out == 0) {
			alarm(1);
			timed_out = 1;
		}
		
		command = GetCommand();
		if (command == 'q' || command == 'Q') {
			stop = time(NULL);
			duration = (double)difftime(stop, start);
			alarm(0);
			DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
			move(HEIGHT / 2, WIDTH / 2 - 4);
			printw("Good-bye!!");
			move(HEIGHT, WIDTH + 10);
			printw("The total time of play: %lf seconds", duration);
			move(HEIGHT + 1, WIDTH + 10);
			printw("score(t)/time(t) = %lf", score / duration);
			move(HEIGHT + 2, WIDTH + 10);
			printw("size of RecNode: %ld", sizeof(RecNode));
			move(HEIGHT + 3, WIDTH + 10);
			nodeCount = 0;
			printw("size of tree: %ld", evalSize(recRoot));
			move(HEIGHT + 4, WIDTH + 10);
			printw("number of nodes: %d", nodeCount);
			refresh();
			getch();
			return;
		}
	} while (!gameOver);
	stop = time(NULL);
	duration = (double)difftime(stop, start);
	alarm(0);
	getch();
	DrawBox(HEIGHT / 2 - 1, WIDTH / 2 - 5, 1, 10);
	move(HEIGHT / 2, WIDTH / 2 - 4);
	printw("GameOver!!");
	move(HEIGHT, WIDTH + 10);
	printw("The total time of play: %lf seconds", duration);
	move(HEIGHT + 1, WIDTH + 10);
	printw("score(t)/time(t) = %lf", score / duration);
	move(HEIGHT + 2, WIDTH + 10);
	printw("size of RecNode: %ld", sizeof(RecNode));
	move(HEIGHT + 3, WIDTH + 10);
	nodeCount = 0;
	printw("size of tree: %ld", evalSize(recRoot));
	move(HEIGHT + 4, WIDTH + 10);
	printw("number of nodes: %d", nodeCount);
	refresh();
	getch();
	//newRank(score);
	// user code
	return;
}

long evalSize(RecNode* root) {
	int i = 0;
	long eval = 0;
	if (root == NULL) return 0;
	nodeCount++;
	while (root->c[i] != NULL) {
		eval = eval + evalSize(root->c[i]);
		i++;
	}
	return sizeof(root) + eval;
}
