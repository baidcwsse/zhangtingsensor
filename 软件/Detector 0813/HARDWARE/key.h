#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"




#define VCC_EN PCout(13)






//////////////////////////////////////////////////////////////////////////////////	 
//本扫描键盘模块的特点：
//一、使用灵活：一体实现按键的普通、单击、双击、长按、保持以及组合等功能，无须为每种键值单独进行宏定义，只须选择所用键值编写相应的响应代码即可。
//             可以灵活选择按键的处理实时性，能够使强实时性的紧急按键优先得到处理，可自由选择中断处理及查询处理或二者混合方式，
//             从而兼具按键的强实时性要求以及超长（主循环执行一遍的时间长达1秒以上的）程序适应性。
//二、通用性强：适应几乎所有按键需求，从少到2-3个按键直到最大32个按键（包括行列式键盘、矩阵加直联端口混合）都可适用。
//三、稳定可靠：后台抖动消除、按键干扰杂波滤除措施有力，按键稳定可靠，不会产生重复按键，也不会漏失按键。	   
//四、移植简便：所有可调整参数（数量不多）均以宏定义列出，除与硬件相关（按键个数及连接端口）的部分须根据具体系统修改外，其它均无须变化，很容易移植。
//             程序可读性强，注释详尽丰富，其中包括函数调用关系及详细运用修改说明，如有未尽事宜，可提出，本人尽量解答修改。
//五、高效节能：消抖无须延时等待，同时采取自适应变频扫键、键盘闲置检测、消抖读键双进程周期差异等多项智能措施尽量减少占用CPU的计算资源。 

//根据需求可修改定义如下参数.
#define TICKS_INTERVAL    	5				//ms,　后台调用间隔时间
#define DEBOUNCE_TICKS    	4				//消除抖动次数，即时长至少为5ms*4
#define NORMAL_SCAN_FREQ    6   //正常情况下扫键频率因子，如为6则表示稳定后扫键周期为6*TICKS_INTERVAL＝30ms
#define SHORT_TICKS       	(300 /TICKS_INTERVAL)  //短按时间定义300ms
#define LONG_TICKS        	(1200 /TICKS_INTERVAL) //长按时间定义1200ms
#define KEYBUFFSIZE					16   //按键缓存FIFO深度，定义保存16个键值

//***************** 以下与具体系统的硬件相关 ********************************************
#define 	KeyNumMax					4			//硬件实体按键数量
typedef		u16 			KeyS_Type;    //键态字类型定义（根据按键总数定义足够长度的数据类型，
//本例程只有4个键，用u8足矣，但为扩充方便这里用了u16，最大可满足16键,大于16键时请定义为u32）
//按键硬件读端口位置
#define KEY0_IN  	PBin(5)			//按键0输入端口
#define KEY1_IN  	PBin(6)			//按键1输入端口
#define KEY2_IN  	PBin(7)			//按键2输入端口 
#define WKUP_IN 	PBin(12)			//按键3输入端口(WK_UP)  

//硬件实体按键编号，键态字KeyS_Type依此顺序按位组合，每BIT位对应一个实体按键
#define KB_KEY0  		0
#define KB_KEY1  		1
#define KB_KEY2  		2 
#define KB_WKUP 		3



//************ 以下基本与硬件无关（除增删组合键码定义外一般无须修改） *********************

//定义一个特殊值用于复位状态机
#define KB_CLR			44


//这里可以定义一些特殊键码（如组合键等）
#define WKUP_PLUSKEY0_PRES	1							//示例：WKUP+KEY0组合按键（先按下WKUP再按下KEY0）


//功能键值输出：
#define KEYOUT_BASE_DEF					10*(KeyNum+2) //为保留组合键值等空间，这里定义为20以上，可视情修改）

#define KEY_EVENT(m,n)					(u8)(10*(m+2)+n) 		//按键事件(即键值)宏定义
//有了上述宏定义后，无须再为各个按键单独写宏定义，使用KEY_EVENT(键编号,键值事件)就可以代表特定按键事件了。
//例如：用KEY_EVENT(KB_WKUP,DOUBLE_CLICK)就表示了WKUP键双击的键值（或称事件值）


//状态机键值事件宏定义如下：
#define NONE_PRESS				0
#define PRESS_UP					1
#define PRESS_DOWN				2
#define SINGLE_CLICK			3
#define DOUBLE_CLICK			4
#define LONG_RRESS_START	5
#define LONG_PRESS_HOLD		6


//按键已经全部释放
#define KEY_RELEASED 				(Cont==0)  //简化的条件
//#define KEY_RELEASED 				(Cont==0 && Trg==0) //严格的条件



/**************数据和函数接口声明*******************/
extern KeyS_Type Trg;
extern KeyS_Type Cont;
extern u16 KeyTime;
extern u8 Flag300ms;

void power_key_floating(void);
void power_vcc_en_init(void);
void KEY_Init(void);//键硬件IO端口初始化，由主函数调用
void GetAndSaveKey(void);//本函数由SYSTICK调用，在后台读键，如果有键值则存入按键缓冲区
u8 Read_A_Key(void);//读取按键值：由系统主循环调用。
void Key_Scan_Stick(void);//本函数由SYSTICK调用，在后台扫描按键获取消除抖动后的稳定键值
u8 Get_Key_State(u8 KeyNum);//按键状态机，本函数一般由Get_Key()内部调用
void key_run_main_task(void);
//函数的调用关系：由SYSTICK自动启动两个进程：
//进程一：SysTick_Handler ->自动调用Key_Scan_Stick()；该进程比较简单，扫描按键获取消除抖动后的稳定键值，无须修改。
//进程二：SysTick_Handler ->自动调用GetAndSaveKey() ->调用Key_PrePro() ->调用Get_Key() ->调用Get_Key_State()
//       该进程中间的两个函数，可根据具体需求进行修改实现：
// 1是Get_Key()函数，定义系统中起作用的键事件，大部分键已通过调用Get_Key_State()自动完成，如有需要，可增加定义一些组合键事件；
// 2是Key_PrePro()函数，键预处理，实质是截获部分要求强实时性的紧急按键优先处理；剩下的键则由GetAndSaveKey()自动存入FIFO队列待主循环查询处理。
//
//系统主循环 ->调用Read_A_Key()：系统主循环中通过调用Read_A_Key()，对FIFO队列中的按键进行查询处理。
//总之，运用本代码模块的主要工作是三件事：
//一：硬件接口初始化：包括必要的GPIO初始化，实体硬按键的排序等。
//二：键值（或称事件）生成：对Get_Key()函数进行改写，使之能输出所有有效的键值。所有按键的单击、双击、长按、保持等键值输出已经实现，如果有需要则在该函数中增加组合键等键值。
//三：键值（或称事件）处理：对键值的具体响应处理可在两个地方实现。如有强实时性的紧急按键需要优先处理的键值请在Key_PrePro()函数编写代码，
//                         其它按键的响应请在系统主循环调用Read_A_Key()得到键值后编写代码。
//                         根据具体项目需求，也可全部响应代码都在主循环中编写，也可以全部响应代码都在Key_PrePro()函数中编写。
//                         如果全部响应代码都在Key_PrePro()函数中编写，则主循环中无须再处理按键（也无须调用Read_A_Key函数）。



		 				    
#endif


















