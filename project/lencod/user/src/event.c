#include "event.h"
#include "util.h"
#include "util264.h"
#include "hide.h"

void Event_Start()
{

}

void Event_End()
{
	printf("\n-------------------------------------------------End-------------------------------------------------\n");
	while (1);
}

void Event_WriteCoeff4x4_CAVLC(int block_type, int *pLevel, int *pRun)
{
	//只针对亮度4x4块做
	if (block_type == LUMA)
	{
		/*
		int coeff[16];
		Lr2Zz(pLevel, pRun, coeff);
		StatisticsLNZIndex(coeff);
		StatisticsLNZ(coeff, 0);
		*/
	}
}

extern InputParameters *input;			//输入参数
/*
完成一个搜寻块的搜索，会触发一次
需要注意的是，这里修改的运动矢量只会对非skip的块起作用哟
在最后所有模式中选择最优模式时，是会涉及skip模式的哟
*/
void Event_MvSearch(int block_x,		//搜寻块在当前宏块的起始位置，4x4单位
	int block_y,		//搜寻看在当前宏块的起始位置
	int list,
	int ref,
	int blocktype,
	int****** all_mv)
{
	int       bsx = input->blc_size[blocktype][0];	//搜寻块大小, 像素单位
	int       bsy = input->blc_size[blocktype][1];	//搜寻块大小, 像素单位

	if (blocktype != 7)
	{
		return ;
	}
	/*
	for (int i = 0; i < (bsx >> 2); i++)
	{
		for (int j = 0; j < (bsy >> 2); j++)
		{
			int mv_x = all_mv[block_x + i][block_y + j][list][ref][blocktype][0];
			int mv_y = all_mv[block_x + i][block_y + j][list][ref][blocktype][1];
		}
	}
	*/
}

void Event_FinishOneMac()
{
	truehide();
	//hide2();
}