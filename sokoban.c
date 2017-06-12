#include <stdio.h>
#include <termio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

char name[11]; // 이름
char p_map[6][31][31] = {0}; // map파일 저장
char p_Ospot[6][31][31] = {0}; // O 좌표
char p_spot = '@'; // 플레이어
char umap[6][6][31][31] = {0}; // UNDO 저장
char re_map[6][31][31] = {0}; // REPLAY
char top_name[6][6][11]; // 랭킹에 등록되어 있는  이름 저장
int top_cnt[6]; // top 실행 시 등록되어있는 사람 수 저장
int p_x[6] = {0}; // player x좌표
int p_y[6] = {0}; // player y좌표
int O_num[6] = {0}; // 각 스테이지의 O 개수
int dollar_num[6] = {0};
int stage = 1; // 현재 스테이지 단계
int i = 0; // 세로
int j = 0; // 가로
int count_undo = 0; // undo는 최대 5번
int num_undo = 0; // undo하고 또 undo
clock_t gap_time[6]; // endTime - startTime
clock_t startTime = 0; // 시작 시간
clock_t endTime = 0; // 종료 시간
clock_t plustime = 0; // save 시에 현재 시간을 저장해 둔 것을 받음. 클리어 시 합산하여 시간 계산
float top_time[6][6]; // 스테이지 별 시간
FILE * map; // 맵 읽기, map.txt
FILE * load; // 파일 로드, sokoban.txt
FILE * rank; // 랭킹 저장, ranking.txt

// 함수 원형
int getch(void); //  OK
void scan_undo(void); // 움직이기 이전 저장 OK
void print_undo(void); // option undo OK
void scan_map(void); // map파일을 모두 읽어 stage별로 저장 OK
void print_map(void); // 맵 출력 OK
void f_move(void); // 움직이기 OK
void f_name(void); // 이름 받기 OK 
void dhelp(void); // d입력 시 도움말 출력 OK
void new(void); // stage 1부터 다시 시작. OK
void replay(void); // 현재 스테이지 다시 시작. OK
void fexit(void); // 프로그램 종료 OK
void stage_check(void); // 클리어 체크 OK
void save(void);// 저장 기능 OK
void f_load(void); // 불러오기 OK
void saving_top(void); // ranking.txt 파일 읽어서 저장 OK
void sort_top(void); // 기존 랭킹과 비교, 정렬 OK
void write_top(void); // ranking.txt 파일에 쓰기 OK

//------------------------------------------------------------------------------------------------------------------------------------

// main 함수
int main(void){
	f_name();
	int i,j,k;
	for (k=0;k<6;k++)
		for(i=0;i<32;i++)
			for(j=0;j<32;j++)
				p_Ospot[k][i][j]=' ';

	scan_map();
	saving_top();
	system("clear");
	print_map();
	for(int c=0;c<6;c++)
		scan_undo();
	f_move();
	return 0;
}

// 출력 안하고 키 받기 
int getch(void){
	int ch;

	struct termios buf;
	struct termios save;

	tcgetattr(0, &save);
	buf = save;

	buf.c_lflag&=~(ICANON|ECHO);
	buf.c_cc[VMIN] = 1;
	buf.c_cc[VTIME] = 0;

	tcsetattr(0, TCSAFLUSH, &buf);
	ch = getchar();
	tcsetattr(0, TCSAFLUSH, &save);

	return ch;
}

// map 파일을 읽어서 저장, 양성인
void scan_map(void){

	map = fopen("map.txt", "rt");
	i = 0;
	j = 0;
	int scan_stage = 0;

	while(1){

		fscanf(map, "%c", &p_map[scan_stage][i][j]);

		if(p_map[scan_stage][i][j] == 'm' && dollar_num[scan_stage] != O_num[scan_stage]){
			printf("맵 오류 발생.\n 프로그램을 종료합니다.");
			exit(0);
		}

		if(p_map[scan_stage][i][j] == 'm'){
			scan_stage++;
			i=0;
			j=0;
			continue;
		}

		else if(p_map[scan_stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		else if(p_map[scan_stage][i][j] == '@'){
			p_y[scan_stage] = i;
			p_x[scan_stage] = j;
		}

		else if(p_map[scan_stage][i][j]=='O'){
			p_Ospot[scan_stage][i][j]='O';
			O_num[scan_stage]++;
		}

		else if(p_map[scan_stage][i][j]=='$')
			dollar_num[scan_stage]++;

		else if(p_map[scan_stage][i][j]=='e')
			break;
		j++;
	}
	fclose(map);

	i=0;
	j=0;
	scan_stage=0;

	// New와 replay 를 위한 맵 저장.
	while(1){

		re_map[scan_stage][i][j] = p_map[scan_stage][i][j];

		if(re_map[scan_stage][i][j] == 'm'){
			scan_stage++;
			i=0;
			j=0;
			continue;
		}

		else if(re_map[scan_stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		else if(re_map[scan_stage][i][j] == 'e')
			break;

		j++;
	}
}

// 맵 출력, 양성인
void print_map(void)
{
	i = 0;
	j = 0;
	int k = 0;
	printf("   Hello %s\n", name);
	while (1){

		if((p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e') && p_map[stage][i-1][k] == '\n')
			break;

		if(p_map[stage][i][j] != 'm' && p_map[stage][i][j] != 'a' && p_map[stage][i][j] != 'p')
			printf("%c", p_map[stage][i][j]);

		if(p_map[stage][i][j] == '\n'){
			i++;
			k=j;
			j=0;
			continue;
		}
		j++;
	}
	printf("(Command)");
}

// 새로 시작, 양성인
void new(void){

	int cnt = 0;
	i=0;
	j=0;
	stage=1;
	count_undo = 0;
	system("clear");
	printf("   Hello %s\n", name);
	while(1){

		p_map[1][i][j] = re_map[1][i][j];

		for(int c=0;c<6;c++)
			umap[c][stage][i][j] = p_map[stage][i][j];	

		if(p_map[1][i][j] == 'm')
			break;

		else if(p_map[1][i][j] == '@'){
			p_y[1] = i;
			p_x[1] = j;
		}

		else if(p_map[1][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	}

	i=0;
	j=0;

	while(1){
		if(p_map[1][i][j] != 'm' && p_map[1][i][j] != 'a' && p_map[1][i][j] != 'p')
			printf("%c", p_map[1][i][j]);

		else if(p_map[1][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		else if(p_map[1][i][j] == 'm')
			break;
		j++;
	}
	printf("(Command)");


	i=0;
	j=0;

	while(1){

		umap[cnt][stage][i][j] = p_map[stage][i][j];

		if(cnt == 6)
			break;

		else if(umap[cnt][stage][i][j] == 'm' || umap[cnt][stage][i][j] == 'e'){
			cnt++;
			i=0;
			j=0;
			continue;
		}

		else if(umap[0][stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	}
	startTime = clock();
	plustime = 0;
}



// 현재 스테이지 다시 시작, 양성인
void replay(void){

	i=0;
	j=0;
	count_undo = 0;
	system("clear");
	printf("   Hello %s\n", name);
	while(1){

		p_map[stage][i][j] = re_map[stage][i][j];

		for(int c=0;c<6;c++)
			umap[c][stage][i][j] = p_map[stage][i][j];

		if(p_map[stage][i][j] != 'm' && p_map[stage][i][j] != 'a' && p_map[stage][i][j] != 'p' && p_map[stage][i][j] != 'e' && p_map[stage][i][j] != 'n' && p_map[stage][i][j] != 'd')
			printf("%c", p_map[stage][i][j]);

		else if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
			break;

		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		if(p_map[stage][i][j] == '@'){
			p_y[stage] = i;
			p_x[stage] = j;
		}

		j++;
	}
	printf("(Command)");
}

// 움직이기, 박영준
void f_move(void){
	startTime = clock(); // 시간 측정 시작
	while(1){
		char move[3];
		move[0] = getch();

		if(move[0] == 'h')
		{
			if(p_map[stage][p_y[stage]][p_x[stage]-1] =='$' && p_map[stage][p_y[stage]][p_x[stage]-2] == p_Ospot[stage][p_y[stage]][p_x[stage]-2])
			{
				scan_undo();
				p_map[stage][p_y[stage]][p_x[stage]-2] = '$';
				p_map[stage][p_y[stage]][p_x[stage]-1] = '@';
				p_map[stage][p_y[stage]][p_x[stage]] = ' ';
				p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
				p_x[stage]--;
				system("clear");
				print_map();
				stage_check();
				continue;
			}
			if(p_map[stage][p_y[stage]][p_x[stage]-1] == '#')
				continue;
			if(p_map[stage][p_y[stage]][p_x[stage]-1] == '$' && p_map[stage][p_y[stage]][p_x[stage]-2] == '$')
				continue;
			if(p_map[stage][p_y[stage]][p_x[stage]-1] == '$' && p_map[stage][p_y[stage]][p_x[stage]-2] == '#')
				continue;

			scan_undo();
			p_map[stage][p_y[stage]][p_x[stage]-1] = '@';
			p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
			p_x[stage]--;
			system("clear");
			print_map();
			stage_check();
			continue;
		}



		else if(move[0] == 'k')
		{
			{
				if(p_map[stage][p_y[stage]-1][p_x[stage]] == '$' && p_map[stage][p_y[stage]-2][p_x[stage]] == p_Ospot[stage][p_y[stage]-2][p_x[stage]])
				{
					scan_undo();
					p_map[stage][p_y[stage]-2][p_x[stage]] = '$';
					p_map[stage][p_y[stage]-1][p_x[stage]] = '@';
					p_map[stage][p_y[stage]][p_x[stage]] = ' ';
					p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
					p_y[stage]--;
					system("clear");
					print_map();
					stage_check();
					continue;
				}
				else if(p_map[stage][p_y[stage]-1][p_x[stage]] == '#')
					continue;
				else if(p_map[stage][p_y[stage]-1][p_x[stage]] == '$' && p_map[stage][p_y[stage]-2][p_x[stage]] == '$')
					continue;
				else if(p_map[stage][p_y[stage]-1][p_x[stage]] == '$' && p_map[stage][p_y[stage]-2][p_x[stage]] == '#')
					continue;
			}
			scan_undo();
			p_map[stage][p_y[stage]-1][p_x[stage]] = '@';
			p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
			p_y[stage]--;
			system("clear");
			print_map();
			stage_check();
			continue;
		}
		else if(move[0] == 'j')
		{
			{
				if(p_map[stage][p_y[stage]+1][p_x[stage]] == '$' && p_map[stage][p_y[stage]+2][p_x[stage]] == p_Ospot[stage][p_y[stage]+2][p_x[stage]])
				{
					scan_undo();
					p_map[stage][p_y[stage]+2][p_x[stage]] = '$';
					p_map[stage][p_y[stage]+1][p_x[stage]] = '@';
					p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
					p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
					p_y[stage]++;
					system("clear");
					print_map();
					stage_check();
					continue;
				}
				else if(p_map[stage][p_y[stage]+1][p_x[stage]] == '#')
					continue;
				else if(p_map[stage][p_y[stage]+1][p_x[stage]] == '$' && p_map[stage][p_y[stage]+2][p_x[stage]] == '$')
					continue;
				else if(p_map[stage][p_y[stage]+1][p_x[stage]] == '$' && p_map[stage][p_y[stage]+2][p_x[stage]] == '#')
					continue;
			}
			scan_undo();
			p_map[stage][p_y[stage]+1][p_x[stage]] = '@';
			p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
			p_y[stage]++;
			system("clear");
			print_map();
			stage_check();
			continue;

		}
		else if(move[0] == 'l')
		{
			{
				if(p_map[stage][p_y[stage]][p_x[stage]+1] == '$' && p_map[stage][p_y[stage]][p_x[stage]+2] == p_Ospot[stage][p_y[stage]][p_x[stage]+2])
				{
					scan_undo();
					p_map[stage][p_y[stage]][p_x[stage]+2] = '$';
					p_map[stage][p_y[stage]][p_x[stage]+1] = '@';
					p_map[stage][p_y[stage]][p_x[stage]] = ' ';
					p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
					p_x[stage]++;
					system("clear");
					print_map();
					stage_check();
					continue;
				}
				else if(p_map[stage][p_y[stage]][p_x[stage]+1] =='#')
					continue;
				else if(p_map[stage][p_y[stage]][p_x[stage]+1] == '$' && p_map[stage][p_y[stage]][p_x[stage]+2] == '$')
					continue;
				else if(p_map[stage][p_y[stage]][p_x[stage]+1] == '$' && p_map[stage][p_y[stage]][p_x[stage]+2] == '#')
					continue;
			}
			scan_undo();
			p_map[stage][p_y[stage]][p_x[stage]+1] = '@';
			p_map[stage][p_y[stage]][p_x[stage]] = p_Ospot[stage][p_y[stage]][p_x[stage]];
			p_x[stage]++;
			system("clear");
			print_map();
			stage_check();
			continue;
		}

		else if (move[0] == 't'){
			printf("%c", move[0]);
			move[1] = getch();

			if(move[1] == 10){
				system("clear");
				printf("   Hello %s\n\n", name);
				for(int a=1;a<6;a++){
					if(top_cnt[a] == 0)
						continue;
					printf("map %d\n", a);
					for(int b=1;b<6;b++){
						if(top_time[a][b] == 0.0){
							continue;
						}
						printf("%s %.1fsec\n", top_name[a][b], top_time[a][b]);
					}
				}	
				printf("\n(Command) t");
				getch();
				system("clear");
				print_map();
				continue;
			}

			else if(move[1] >= '1' && move[1] <= '5'){ 
				printf("%c", move[1]);
				move[2] = getch();
				if(move[2] == 10){
					system("clear");
					printf("   Hello %s\n\n", name);
					printf("map %c\n", move[1]);
					for(int a=1;a<=top_cnt[move[1]-'0'];a++)
						printf("%s %.1f\n", top_name[move[1]-'0'][a], top_time[move[1]-'0'][a]);  // 아스키코드 값으로 인식하므로 -'0'
					printf("\n(Command)%c %c\n",move[0], move[1]);
					getch();
					system("clear");
					print_map();
				}
			}
			continue;
		}

		else if(move[0] != 'u' && move[0] != 'd' && move[0] != 's' && move[0] != 'n' && move[0] != 'f' && move[0] != 'e' && move[0] != 'r'){
			system("clear");
			print_map();
			continue;
		}

		printf("%c", move[0]);

		move[1] = getch();

		if(move[0] == 'u' && move[1] == 10){
			if(count_undo == 5){
				printf("이미 5회 사용하셨습니다.");
				continue;
			}
			count_undo++;
			print_undo();
			continue;
		}

		else if(move[0] == 'd' && move[1] == 10){
			dhelp();
			continue;
		}

		else if(move[0] == 's' && move[1] == 10){
			save();
			continue;
		}
		else if(move[0] == 'n' && move[1] == 10){
			endTime = 0;
			new();
			continue;
		}

		else if(move[0] == 'f' && move[1] == 10){
			f_load();
			continue;
		}
		else if(move[0] == 'e' && move[1] == 10){
			fexit();
			continue;
		}
		else if(move[0] == 'r' && move[1] == 10){
			replay();
			continue;
		}

	}
}

// umap 배열에 undo내용 저장, 박영준
void scan_undo(void){

	i = 0;
	j = 0;
	num_undo = 0;

	while(1){

		umap[num_undo+5][stage][i][j] = umap[num_undo+4][stage][i][j];
		umap[num_undo+4][stage][i][j] = umap[num_undo+3][stage][i][j];
		umap[num_undo+3][stage][i][j] = umap[num_undo+2][stage][i][j];
		umap[num_undo+2][stage][i][j] = umap[num_undo+1][stage][i][j];
		umap[num_undo+1][stage][i][j] = umap[num_undo][stage][i][j];
		umap[num_undo][stage][i][j] = p_map[stage][i][j];

		if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
			break;

		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	} 
}

// undo 시 출력, 박영준
void print_undo(void){

	i = 0;
	j = 0;
	system("clear");

	printf("   Hello %s\n", &name);
	while(1){
		if(umap[num_undo][stage][i][j] != 'm' && umap[num_undo][stage][i][j] != 'a' && umap[num_undo][stage][i][j] != 'p' && umap[num_undo][stage][i][j] != 'e' && umap[num_undo][stage][i][j] != 'n' && umap[num_undo][stage][i][j] != 'd')
			printf("%c", umap[num_undo][stage][i][j]);

		if(umap[num_undo][stage][i][j] == 'm' || umap[num_undo][stage][i][j] == 'e')
			break;

		if(umap[num_undo][stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	}
	printf("(Command)");

	i = 0;
	j = 0;

	while(1){

		p_map[stage][i][j] = umap[num_undo][stage][i][j];

		if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
			break;

		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		if(p_map[stage][i][j] == '@'){
			p_y[stage] = i;
			p_x[stage] = j;
		}

		j++;
	}
	num_undo++;
}


//프로그램 종료, 정경륜
void fexit(void){
	system("clear");
	printf("S E E  Y O U  %s . . . \n", name);
	printf("\n(Command) e");
	exit(0);
}

// 도움말 출력, 정경륜
void dhelp(void){
	system("clear");
	printf("   Hello %s\n\n", name);
	printf("h(����), j(�Ʒ�), k(��), l(������)\n");
	printf("u(undo)\n");
	printf("r(replay)\n");
	printf("n(new)\n");
	printf("e(exit)\n");
	printf("s(save)\n");
	printf("f(file load)\n");
	printf("d(display help)\n");
	printf("t(top)\n\n");
	printf("(Command) d");
	getch();
	system("clear");
	print_map();
}

// 이름 받기, 이서영
void f_name(){
	int i = 0;
	char c;
	system("clear");
	printf("Start....\n");
	printf("input name : ");

	while(1){
		c = getchar();
		
		if((c=='\n')||(c==EOF)){
			name[i++] = '\0';
			break;
		}

		if((c==' ') || (i==10)){
			while((c=getchar())!='\n') ;
			printf("\n10자 이하의 영문으로 입력해주세요.\ninput name : ");
			i = 0;
			continue;
		}

		if((c>='a' && c<='z') || (c>='A' && c<='Z')){
		}

		else{
			while((c=getchar())!='\n') ;
			i=0;
			printf("\n10자 이하의 영문으로 입력해주세요.\ninput name : ");
			continue;
		}
		name[i++] = c;
	}
}

// 스테이지 클리어 체크, 이서영
void stage_check(void){

	int check = 0;
	i=0;
	j=0;
	while(1){
		if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] =='e'){
			i=0;
			j=0;
			break;
		}
		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		if(p_Ospot[stage][i][j]== 'O' && p_map[stage][i][j] == '$')
			check++;

		if(check == O_num[stage]){
			endTime = clock(); // 스테이지 클리어 시 측정 종료
			gap_time[stage] = (float)((endTime - startTime + plustime)/(CLOCKS_PER_SEC));
			saving_top();
			sort_top();
			write_top();
			stage++;

			if(stage == 6){
				printf("\n");
				system("clear");
				printf("Congraturation ! You clear all stage.");
				exit(0);
			}

			for(int c=0;c<6;c++){
				i=0;
				j=0;
				while(1){

					umap[c][stage][i][j] = p_map[stage][i][j];

					if(p_map[stage][i][j] == '\n'){
						i++;
						j=0;
						continue;
					}
					if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
						break;
					j++;
				}
			}
			i=0;
			j=0;

			count_undo = 0;
			printf("\n");
			system("clear");
			print_map();
			startTime = clock(); // 다음 스테이지 측정 시작
			break;
		}
		j++;
	}
}


// 현재 상태 저장, 이서영
void save(void){ 

	int nu = 0;
	i=0;
	j=0;
	FILE * sokoban;
	endTime = clock();
	plustime = endTime - startTime;
	sokoban = fopen("sokoban.txt", "wt");
	fprintf(sokoban, "%s %d %d %d", name, plustime, stage, count_undo);
	fprintf(sokoban, "\n");

	while(1){

		fprintf(sokoban, "%c", p_map[stage][i][j]);

		if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
			break;

		else if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	}

	i=0;
	j=0;

	while(nu <= 5){

		fprintf(sokoban, "%c", umap[nu][stage][i][j]);

		if(umap[nu][stage][i][j] == 'm' || umap[nu][stage][i][j] == 'e'){
			nu++;
			i=0;
			j=0;
			continue;
		}
		else if(umap[nu][stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}
		j++;
	}

	system("clear");
	print_map();
	fclose(sokoban);
}


// 불러오기, 이서영
void f_load(void){

	int nu = 0;

	load = fopen("sokoban.txt", "rt");
	fscanf(load,"%s %d %d %d\n", name, &plustime, &stage, &count_undo);
	i=0;
	j=0;
	system("clear");

	while(1){

		fscanf(load, "%c", &p_map[stage][i][j]);

		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		if(p_map[stage][i][j]== 'm' || p_map[stage][i][j] == 'e')	
			break;

		if(p_map[stage][i][j]== '@'){
			p_x[stage] = j;
			p_y[stage] = i;
		}

		if(p_map[stage][i][j]== 'O'){
			p_Ospot[stage][i][j]='O';
		}
		j++;
	}
	i=0;
	j=0;

	while(nu<6){

		fscanf(load, "%c", &umap[nu][stage][i][j]);

		if(umap[nu][stage][i][j] == 'm' || umap[nu][stage][i][j] == 'e'){
			nu++;
			i=0;
			j=0;
			continue;
		}

		if(umap[nu][stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		j++;
	}

	fclose(load);

	printf("   Hello %s\n", name);

	while(1){

		if(p_map[stage][i][j] != 'm' && p_map[stage][i][j] != 'a' && p_map[stage][i][j] != 'p' && p_map[stage][i][j] != 'e' && p_map[stage][i][j] != 'n' && p_map[stage][i][j] != 'd')
			printf("%c", p_map[stage][i][j]);

		if(p_map[stage][i][j] == '\n'){
			i++;
			j=0;
			continue;
		}

		if(p_map[stage][i][j] == 'm' || p_map[stage][i][j] == 'e')
			break;

		j++;
	}
	startTime = clock();
	printf("(Command)");
}


// ranking.txt 파일의 내용을 저장, 정경륜
void saving_top(void){ 

	char del[5];
	int check = 1;
	int cnt_stage;
	rank = fopen("ranking.txt","r");

	while(1){

		if(check == 6)
			break;

		fscanf(rank,"%s", del);
		fscanf(rank,"%d", &cnt_stage);
		fscanf(rank,"%d", &top_cnt[cnt_stage]);

		for(int a=1;a<=top_cnt[cnt_stage];a++){
			fscanf(rank,"%s", top_name[cnt_stage][a]);
			fscanf(rank,"%f", &top_time[cnt_stage][a]);
		}	
		check++;
	}
	fclose(rank);	
}

// 기존 랭킹과 현재 플레이어의 기록을 비교, 정렬. 정경륜
void sort_top(void){

	char tmp_name[10][11];
	float tmp_time[10];
	int cnt_stage = stage;
	cnt_stage = stage;
	int x = 1;

	if(top_cnt[cnt_stage] == 0){
		top_time[cnt_stage][1] = gap_time[stage];
		strcpy(top_name[cnt_stage][1], name);
	}

	else if(top_cnt[cnt_stage] == 1){

		while(1){
			if(top_time[cnt_stage][x] >= gap_time[stage]){
				tmp_time[0] = top_time[cnt_stage][x];
				top_time[cnt_stage][x] = gap_time[stage];
				top_time[cnt_stage][x+1] = tmp_time[0];

				strcpy(tmp_name[0], top_name[cnt_stage][x]);
				strcpy(top_name[cnt_stage][x], name);
				strcpy(top_name[cnt_stage][x+1], tmp_name[0]);
				break;
			}

			else {
				top_time[cnt_stage][2] = gap_time[stage];
				strcpy(top_name[cnt_stage][2], name);
			}
			x++;

			if(x == top_cnt[cnt_stage]+1)
				break;
		}
	}

	else if(top_cnt[cnt_stage] == 2){
		while(1){
			if(top_time[cnt_stage][x] >= gap_time[stage]){
				tmp_time[0] = top_time[cnt_stage][x];
				tmp_time[1] = top_time[cnt_stage][x+1];
				top_time[cnt_stage][x] = gap_time[stage];
				top_time[cnt_stage][x+1] = tmp_time[0];
				top_time[cnt_stage][x+2] = tmp_time[1];

				strcpy(tmp_name[0], top_name[cnt_stage][x]);
				strcpy(tmp_name[1], top_name[cnt_stage][x+1]);
				strcpy(top_name[cnt_stage][x], name);
				strcpy(top_name[cnt_stage][x+1], tmp_name[0]);
				strcpy(top_name[cnt_stage][x+2], tmp_name[1]);
				break;
			}

			else {
				top_time[cnt_stage][3] = gap_time[stage];
				strcpy(top_name[cnt_stage][3], name);
			}
			x++;

			if(x == top_cnt[cnt_stage]+1)
				break;
		}
	}


	else if(top_cnt[cnt_stage] == 3){

		while(1){
			if(top_time[cnt_stage][x] >= gap_time[stage]){
				
				tmp_time[0] = top_time[cnt_stage][x];
				tmp_time[1] = top_time[cnt_stage][x+1];
				top_time[cnt_stage][x] = gap_time[stage];
				top_time[cnt_stage][x+1] = tmp_time[0];
				top_time[cnt_stage][x+2] = tmp_time[1];

				strcpy(tmp_name[0], top_name[cnt_stage][x]);
				strcpy(tmp_name[1], top_name[cnt_stage][x+1]);
				strcpy(top_name[cnt_stage][x], name);
				strcpy(top_name[cnt_stage][x+1], tmp_name[0]);
				strcpy(top_name[cnt_stage][x+2], tmp_name[1]);
				break;
				
			}

			else {
				top_time[cnt_stage][4] = gap_time[stage];
				strcpy(top_name[cnt_stage][4], name);
			}

			x++;

			if(x == top_cnt[cnt_stage]+1)
				break;
		}
	}

	else if(top_cnt[cnt_stage] == 4){

		while(1){
			if(top_time[cnt_stage][x] >= gap_time[stage]){
				tmp_time[0] = top_time[cnt_stage][x];
				tmp_time[1] = top_time[cnt_stage][x+1];
				tmp_time[2] = top_time[cnt_stage][x+2];
				top_time[cnt_stage][x] = gap_time[stage];
				top_time[cnt_stage][x+1] = tmp_time[0];
				top_time[cnt_stage][x+2] = tmp_time[1];
				top_time[cnt_stage][x+3] = tmp_time[2];

				strcpy(tmp_name[0], top_name[cnt_stage][x]);
				strcpy(tmp_name[1], top_name[cnt_stage][x+1]);
				strcpy(tmp_name[2], top_name[cnt_stage][x+2]);
				strcpy(top_name[cnt_stage][x], name);
				strcpy(top_name[cnt_stage][x+1], tmp_name[0]);
				strcpy(top_name[cnt_stage][x+2], tmp_name[1]);
				strcpy(top_name[cnt_stage][x+3], tmp_name[2]);
				break;
				
			}
			else {
				top_time[cnt_stage][5] = gap_time[stage];
				strcpy(top_name[cnt_stage][5], name);
			}

			x++;

			if(x == top_cnt[cnt_stage]+1)
				break;
		}
	}

	else if(top_cnt[cnt_stage] == 5){

		while(1){
			if(top_time[cnt_stage][x] >= gap_time[stage]){
					tmp_time[0] = top_time[cnt_stage][x];
					tmp_time[1] = top_time[cnt_stage][x+1];
					tmp_time[2] = top_time[cnt_stage][x+2];
					tmp_time[3] = top_time[cnt_stage][x+3];
					top_time[cnt_stage][x] = gap_time[stage];
					top_time[cnt_stage][x+1] = tmp_time[0];
					top_time[cnt_stage][x+2] = tmp_time[1];
					top_time[cnt_stage][x+3] = tmp_time[2];
					top_time[cnt_stage][x+4] = tmp_time[3];

					strcpy(tmp_name[0], top_name[cnt_stage][x]);
					strcpy(tmp_name[1], top_name[cnt_stage][x+1]);
					strcpy(tmp_name[2], top_name[cnt_stage][x+2]);
					strcpy(tmp_name[3], top_name[cnt_stage][x+3]);
					strcpy(top_name[cnt_stage][x], name);
					strcpy(top_name[cnt_stage][x+1], tmp_name[0]);
					strcpy(top_name[cnt_stage][x+2], tmp_name[1]);
					strcpy(top_name[cnt_stage][x+3], tmp_name[2]);
					strcpy(top_name[cnt_stage][x+4], tmp_name[3]);
					
				break;
			}
			x++;

			if(x == top_cnt[cnt_stage]+1)
				break;
		}
	}
}

// ranking.txt 파일에 내용 쓰기, 정경륜
void write_top(void){
	int num = 1;
	int top_stage = 1;
	rank = fopen("ranking.txt", "w");

	while(1){

		if(top_stage == 6)
			break;

		if(top_cnt[num] < 5 && top_stage == stage){
			top_cnt[num]++;
			fprintf(rank,"map %d %d\n", num, top_cnt[num]);
		}

		else 
		{
			fprintf(rank,"map %d %d\n", num, top_cnt[num]);
		}

		for(int a=1;a<=top_cnt[num];a++)
			fprintf(rank,"%s %.1f\n", top_name[top_stage][a], top_time[top_stage][a]);

		num++;
		top_stage++;
	}
	fclose(rank);
}
