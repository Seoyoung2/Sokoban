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

//------------------------------------------------------------------------------------------------------------------------------------

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
