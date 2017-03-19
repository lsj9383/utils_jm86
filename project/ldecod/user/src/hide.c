#include "hide.h"
#include "util.h"
#include "math.h"

int macLevel[4][4][16];
int macRun[4][4][16];
int blocksMatrix[4][4][4][4];

const static int maxModifyEleNumber = 8;
const static int isSkipNoDC = 0;
const static int right_block_mode[] = { 0, 3, 7 };
const static int under_block_mode[] = { 1, 8 };
const static int under_left_block_mode[] = { 0, 1, 2, 4, 5, 6, 8 };
const static int under_right_block_mode[] = { 0, 1, 2, 3, 7, 8 };
const static int pairSets[2][3][4] = {
	{ { 3, 0, 3, 2 }, { 2, 2, 2, 0 }, { 1, 2, 1, 0 } },		//保证向下方零漂移的配对系数集合
	{ { 0, 3, 2, 3 }, { 2, 2, 0, 2 }, { 2, 1, 0, 1 } } };	//保证向右边零漂移的配对系数集合

static void LoadMode(int mode[4][4]);
static int filter4x4Block(int mode[4][4], int buffer[]);
static int filterEles(int ele_pos[2][50], int buffer[], int blockNumber);

int get_feature_value(int *g, int n)
{
	int feature_value = 0;
	int max_value = pow(3, n);
	for (int i = 0; i < n; i++)
	{
		feature_value += pow(3, i) * g[i];
	}
	feature_value = mymod(feature_value, max_value);
	return feature_value;
}


void hide2(struct img_par *img)
{
	static int bits = 0;
	if (img->type == I_SLICE && img->mb_data[img->current_mb_nr].mb_type == I4MB)
	{
		Macroblock *currentMb = &img->mb_data[img->current_mb_nr];
		int mode[4][4] = {-1};
		LoadMode(mode);

		//筛选出可以嵌入数据的4x4块
		int block_buffer[50] = { -1 };
		int ele_pos[2][50];
		int modifyEleNumber = filterEles(ele_pos, block_buffer, filter4x4Block(currentMb, mode, block_buffer));

		int eles[16] = { 0 };
		for (int i = 0; i < modifyEleNumber; i++)
		{
			int b8 = ele_pos[0][i] / 1000;
			int b4 = (ele_pos[0][i] % 1000) / 100;
			int m = (ele_pos[0][i] % 100) / 10;
			int n = ele_pos[0][i] % 10;
			int cofmatrix[4][4];

			Lr2Matrix(macLevel[b8][b4], macRun[b8][b4], cofmatrix);
			eles[i] = cofmatrix[m][n];
		}

		int feature_value = get_feature_value(eles, modifyEleNumber);
		feature_value = feature_value;
		//***************************************
		FILE *fp = fopen("extract.txt", "a");
		fprintf(fp, ", %d\n", feature_value);
		fclose(fp);
		//***************************************
	}
	printf("bits = %d\n", bits);
}

static int filterEles(int ele_pos[2][50], int buffer[], int blockNumber)
{
	int eleNumber = 0;

	for (int pairNum = 0; pairNum < 3; pairNum++)
	{
		for (int i = 0; i < blockNumber; i++)
		{
			int b8 = buffer[i] / 100;
			int b4 = (buffer[i] % 100) / 10;
			int conditions = buffer[i] % 10;
			int mn[2][2] = { 0 };


			mn[0][0] = pairSets[conditions][pairNum][0];
			mn[0][1] = pairSets[conditions][pairNum][1];
			mn[1][0] = pairSets[conditions][pairNum][2];
			mn[1][1] = pairSets[conditions][pairNum][3];

			ele_pos[0][eleNumber] = b8 * 1000 + b4 * 100 + mn[0][0] * 10 + mn[0][1];
			ele_pos[1][eleNumber] = b8 * 1000 + b4 * 100 + mn[1][0] * 10 + mn[1][1];
			eleNumber += 1;
			if (eleNumber >= maxModifyEleNumber)
			{
				return eleNumber;
			}
		}
	}

	return eleNumber;
}

//筛选出当前宏块中可以修改的4x4块，以及其满足的嵌入条件。
static int filter4x4Block(Macroblock *currentMb, int mode[4][4], int buffer[])
{
	int canModifyNumber = 0;
	int cbp = currentMb->cbp;

	for (int b8 = 0; b8 < 4; b8++)
	{
		if (!(cbp & (1 << b8))){ continue; }
		for (int b4 = 0; b4 < 4; b4++)
		{
			int cofmatrix[4][4];
			int m = B8b42M(b8, b4);		//块所位于的【行】
			int n = B8b42N(b8, b4);		//块所位于的【列】

			Lr2Matrix(macLevel[b8][b4], macRun[b8][b4], cofmatrix);

			if ((cofmatrix[0][0] == 0) && isSkipNoDC){ continue; }				//跳过

			if (m < 3 && (n == 1 || n == 2))
			{//part 1
				if (ArrayContains(mode[m][n + 1], right_block_mode, 3) == 1)
				{	//满足条件1
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 0;
					canModifyNumber++;
				}
				else if ((ArrayContains(mode[m + 1][n], under_block_mode, 2) == 1)
					&& (ArrayContains(mode[m + 1][n - 1], under_left_block_mode, 7) == 1))
				{	//满足条件2;
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 1;
					canModifyNumber++;
				}
			}
			else if (n == 0 & m != 3)
			{//part 2
				if (ArrayContains(mode[m][n + 1], right_block_mode, 3) == 1)
				{	//满足条件1
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 0;
					canModifyNumber++;
				}
				else if (ArrayContains(mode[m + 1][n], under_block_mode, 2) == 1)
				{	//满足条件2，不用考察左下的块，因为是肯定不会用到当前4x4块的(编码左下块时，当前4x4块还没编码的，所以肯定不会用的)
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 1;
					canModifyNumber++;
				}
			}
			else if (n == 3 && m != 3)
			{//part 3
				if ((ArrayContains(mode[m + 1][n], under_block_mode, 2) == 1)
					&& (ArrayContains(mode[m + 1][n - 1], under_left_block_mode, 7) == 1))
				{	//满足条件2
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 1;
					canModifyNumber++;
				}
			}
			else if (m != 3)
			{
				if (ArrayContains(mode[m][n + 1], right_block_mode, 3) == 1)
				{	//满足条件1
					buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 0;
					canModifyNumber++;
				}
			}
			else if (m == 3 && n == 3)
			{
				//buffer[canModifyNumber] = b8 * 100 + b4 * 10 + 0;
				///canModifyNumber++;
			}
		}
	}

	return canModifyNumber;
}

static void LoadMode(int mode[4][4])
{
	int **ipredmode4 = img->ipredmode;
	for (int b8 = 0; b8 < 4; b8++)
	{
		for (int b4 = 0; b4 < 4; b4++)
		{
			int block4_x = (8 * (b8 % 2) + 4 * (b4 % 2));//++ 当前4*4块左上角像素在本宏块内的横坐标（以像素为单位）
			int block4_y = (8 * (b8 / 2) + 4 * (b4 / 2));//++ 当前4*4块左上角像素在本宏块内的纵坐标（以像素为单位）
			int pic_pix_x = img->pix_x + block4_x;
			int pic_pix_y = img->pix_y + block4_y;
			int pic_block4_x = pic_pix_x / 4;
			int pic_block4_y = pic_pix_y / 4;

			mode[block4_y / 4][block4_x / 4] = ipredmode4[pic_block4_x][pic_block4_y];
		}
	}
}