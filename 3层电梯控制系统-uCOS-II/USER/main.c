#include "sys.h" 	
#include "delay.h"	
#include "led.h"
#include "includes.h"
#include "usart.h"

/////////////////////////UCOSII��������///////////////////////////////////

// KEYSCANͨ����Ϣ����
#define KEYSCAN_MSG_SIZE                10
void *KeyScanMessagePtr[KEYSCAN_MSG_SIZE];
OS_EVENT *KeyScan_Msg_Event;

// ELEVͨ����Ϣ����
#define ELEV_MSG_SIZE                   10
void *ELEVMessagePtr[ELEV_MSG_SIZE];
OS_EVENT *ELEV_Msg_Event;

// CTLͨ����Ϣ����
#define CTL_MSG_SIZE                    10
void *CTLMessagePtr[CTL_MSG_SIZE];
OS_EVENT *CTL_Msg_Event;

//KEYSCAN����
//�����������ȼ�
#define KEYSCAN_TASK_PRIO               3
//���������ջ��С
#define KEYSCAN_STK_SIZE                64
//�����ջ	
OS_STK KEYSCAN_TASK_STK[KEYSCAN_STK_SIZE];
//������
void KEYSCAN_task(void *pdata);


//CTL����
//�����������ȼ�
#define CTL_TASK_PRIO       			4
//���������ջ��С
#define CTL_STK_SIZE  					64
//�����ջ
OS_STK CTL_TASK_STK[CTL_STK_SIZE];
//������
void CTL_task(void *pdata);

//ELEVATOR����
//�����������ȼ�
#define ELEVATOR_TASK_PRIO              5
//���������ջ��С
#define ELEVATOR_STK_SIZE               64
//�����ջ
OS_STK ELEVATOR_TASK_STK[ELEVATOR_STK_SIZE];
//������
void ELEVATOR_task(void *pdata);


struct ELEVATOR_STATUS{
    uint8_t ELEVATOR_FLOOR;
    uint8_t ELEVATOR_DESTNATION;
};

#define ELEVATOR_CTL_OPEN_DOOR          0
#define ELEVATOR_CTL_GOTO_FLOOR(x)      x
#define ELEVATOR_CTL_GOTO_FLOOR_RECV(x) x

//#define UART_FOR_DEBUG

int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();	    //��ʱ������ʼ��	  
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
    
	OSInit();
    KeyScan_Msg_Event = OSQCreate(&KeyScanMessagePtr[0], KEYSCAN_MSG_SIZE);
    ELEV_Msg_Event = OSQCreate(&ELEVMessagePtr[0], ELEV_MSG_SIZE);
    CTL_Msg_Event = OSQCreate(&CTLMessagePtr[0], CTL_MSG_SIZE);
 	OSTaskCreate(KEYSCAN_task,(void *)0,(OS_STK*)&KEYSCAN_TASK_STK[KEYSCAN_STK_SIZE-1], KEYSCAN_TASK_PRIO);
 	OSTaskCreate(CTL_task,(void *)0,(OS_STK*)&CTL_TASK_STK[CTL_STK_SIZE-1], CTL_TASK_PRIO);
 	OSTaskCreate(ELEVATOR_task,(void *)0,(OS_STK*)&ELEVATOR_TASK_STK[ELEVATOR_STK_SIZE-1], ELEVATOR_TASK_PRIO);
    OSStart();
}

//KEYSCAN����:��ȡ���ݰ���,�����ȼ�
void KEYSCAN_task(void *pdata)
{
    uint8_t key_down = 0;
	while(1)
	{
        key_down = KEY_Scan();
        if(key_down != 0)
        {   
            uint8_t * p = (uint8_t *)malloc(sizeof(key_down));
            *p = key_down;
            OSQPost(KeyScan_Msg_Event, p);    // ������Ϣ����������
        }
        else
            delay_ms(10);
	}
}

//CTL����:�ƻ���������
void CTL_task(void *pdata)
{	  
    uint8_t err = 0;
    uint8_t * key_down_recv;
    struct ELEVATOR_STATUS * recv_tmp;
    struct ELEVATOR_STATUS elevator_status_recv;
    uint8_t * p;
    
    uint8_t elevator_up[3] = {0};       // �����ⰴ�� ���� [3FUP,2FUP,1FUP]
    uint8_t elevator_down[3] = {0};     // �����ⰴ�� ���� [3FDN,2FDN,1FDN]
    uint8_t elevator_in[3] = {0};       // �����ڰ��� ¥�� [3FFL,2FFL,1FFL]
    
    uint8_t req_floor_last = 0;
    
    while(1)
    {
        key_down_recv = (uint8_t *)OSQPend(KeyScan_Msg_Event, 1, &err); // ���������հ�����Ϣ
        if(err == OS_ERR_NONE)
        {
            // ��������Ϣ
            switch(*key_down_recv)
            {
                case KEY_F3_DOWN        :   elevator_down[2] = 1;   printf("\'DOWN\' ON F3 PRESSED.\r\n");      break;
                case KEY_F2_UP          :   elevator_up[1] = 1;     printf("\'UP\' ON F2 PRESSED.\r\n");        break;
                case KEY_F2_DOWN        :   elevator_down[1] = 1;   printf("\'DOWN\' ON F2 PRESSED.\r\n");      break;
                case KEY_F1_UP          :   elevator_up[0] = 1;     printf("\'UP\' ON F1 PRESSED.\r\n");        break;
                case KEY_ELEVATOR_F3    :   elevator_in[2] = 1;     printf("\'F3\' IN ELEVATOR PRESSED.\r\n");  break;
                case KEY_ELEVATOR_F2    :   elevator_in[1] = 1;     printf("\'F2\' IN ELEVATOR PRESSED.\r\n");  break;
                case KEY_ELEVATOR_F1    :   elevator_in[0] = 1;     printf("\'F1\' IN ELEVATOR PRESSED.\r\n");  break;
            }
            #ifdef UART_FOR_DEBUG
            printf("KeyScan_Msg_Event = %d\r\n", *key_down_recv);
            printf("ELEV UP = %d%d%d, DN = %d%d%d, IN = %d%d%d\r\n",
                elevator_up[0],elevator_up[1],elevator_up[2],
                elevator_down[0],elevator_down[1],elevator_down[2],
                elevator_in[0],elevator_in[1],elevator_in[2]);
            #endif
        }
        free(key_down_recv);
        
        recv_tmp = (struct ELEVATOR_STATUS *)OSQPend(ELEV_Msg_Event, 1, &err); // ���������յ���״̬
        if(err == OS_ERR_NONE)
        {
            elevator_status_recv = * recv_tmp;  // ��ȡ����״̬
        }
        
        if(elevator_up[0]|elevator_up[1]|elevator_up[2]
            |elevator_down[0]|elevator_down[1]|elevator_down[2]
            |elevator_in[0]|elevator_in[1]|elevator_in[2])      // ��δ������
        {   
            if(elevator_in[elevator_status_recv.ELEVATOR_FLOOR - 1]
                |elevator_up[elevator_status_recv.ELEVATOR_FLOOR - 1]
                |elevator_down[elevator_status_recv.ELEVATOR_FLOOR - 1])   // ���ﰴ�����ڲ�
            {
                elevator_in[elevator_status_recv.ELEVATOR_FLOOR - 1] = 0;
                elevator_up[elevator_status_recv.ELEVATOR_FLOOR - 1] = 0;
                elevator_down[elevator_status_recv.ELEVATOR_FLOOR - 1] = 0;
                // ����
                #ifdef UART_FOR_DEBUG
                printf("CTL_Msg_Event :OPEN DOOR F%d\r\n", elevator_status_recv.ELEVATOR_FLOOR);
                #endif
                p = (uint8_t *)malloc(sizeof(uint8_t));
                *p = ELEVATOR_CTL_OPEN_DOOR;
                OSQPost(CTL_Msg_Event, p);      // ���͵��ݿ���ָ��
            }
            else 
            {
                uint8_t elev_to_in = 0;
                uint8_t elev_to_out = 0;
                uint8_t elev_to_ctl = 0;
                
                if(elevator_in[0]|elevator_in[1]|elevator_in[2])    // �����ڰ���������(����)
                {
                    if(elevator_in[1] == 1)     // (TODO)ע��:�����ȼ�
                        elev_to_in = 2;
                    else if(elevator_in[0] == 1)
                        elev_to_in = 1;
                    else if(elevator_in[2] == 1)
                        elev_to_in = 3;
                }
                else if(elevator_up[0]|elevator_down[0]
                    |elevator_up[1]|elevator_down[1]
                    |elevator_up[2]|elevator_down[2])    // �����ⲿ��������
                {
                    if(elevator_up[1]|elevator_down[1])     // (TODO)ע��:�����ȼ�
                        elev_to_out = 2;
                    else if(elevator_up[0]|elevator_down[0])
                        elev_to_out = 1;
                    else if(elevator_up[2]|elevator_down[2])
                        elev_to_out = 3;
                }
                
                if(elev_to_in == 0 && elev_to_out == 0)         // �ް���
                    elev_to_ctl = 0;
                else if(elev_to_in == 0 && elev_to_out != 0)    // �ⲿ��������
                    elev_to_ctl = elev_to_out;
                else if(elev_to_in != 0 && elev_to_out == 0)    // �ڲ���������/��������°��������ȴ����ڲ�����
                    elev_to_ctl = elev_to_in;
                
                if(req_floor_last != elev_to_ctl)
                {
                    #ifdef UART_FOR_DEBUG
                    printf("CTL_Msg_Event :TO %d\r\n", elev_to_ctl);
                    #endif
                    // Ŀ��elev_to_ctl��
                    p = (uint8_t *)malloc(sizeof(uint8_t));
                    *p = ELEVATOR_CTL_GOTO_FLOOR(elev_to_ctl);
                    OSQPost(CTL_Msg_Event, p);      // ���͵��ݿ���ָ��
                }
                req_floor_last = elev_to_ctl;
            }
        }
        delay_ms(10);
    }
}


//ELEVATOR����:���ݿ���
void ELEVATOR_task(void *pdata)
{
    struct ELEVATOR_STATUS elevator_status = {1, 1};
    uint8_t err = 0;
    uint8_t * elevator_ctl_recv;
    F1_FLOOR_LED = 1;
    while(1)
    {
        OSQPost(ELEV_Msg_Event, &elevator_status);      // ����״̬����������
        
        elevator_ctl_recv = OSQPend(CTL_Msg_Event, 1, &err);      // ���յ��ݿ���ָ��
        if(err == OS_ERR_NONE)
        {
            #ifdef UART_FOR_DEBUG
            printf("ELEV CTL:%d\r\n", *elevator_ctl_recv);
            #endif
            switch(*elevator_ctl_recv)
            {
                case ELEVATOR_CTL_OPEN_DOOR:
                    printf("ELEVATOR OPEN DOOR.\r\n");
                    ALARM_LED = 1;
                    delay_ms(200);
                    ALARM_LED = 0;
                    delay_ms(200);
                    ALARM_LED = 1;
                    delay_ms(200);
                    ALARM_LED = 0;
                    delay_ms(200);
                    printf("ELEVATOR CLOSE DOOR.\r\n");
                break;
                case ELEVATOR_CTL_GOTO_FLOOR_RECV(1):
                    printf("ELEVATOR MOVING TO F1.\r\n");
                    delay_ms(500);
                    elevator_status.ELEVATOR_FLOOR = 1;
                    F1_FLOOR_LED = 1;
                    F2_FLOOR_LED = 0;
                    F3_FLOOR_LED = 0;
                    printf("ELEVATOR MOVED TO F1.\r\n");
                break;
                case ELEVATOR_CTL_GOTO_FLOOR_RECV(2):
                    printf("ELEVATOR MOVING TO F2.\r\n");
                    delay_ms(500);
                    elevator_status.ELEVATOR_FLOOR = 2;
                    F1_FLOOR_LED = 0;
                    F2_FLOOR_LED = 1;
                    F3_FLOOR_LED = 0;
                    printf("ELEVATOR MOVED TO F2.\r\n");
                break;
                case ELEVATOR_CTL_GOTO_FLOOR_RECV(3):
                    printf("ELEVATOR MOVING TO F3.\r\n");
                    delay_ms(500);
                    elevator_status.ELEVATOR_FLOOR = 3;
                    F1_FLOOR_LED = 0;
                    F2_FLOOR_LED = 0;
                    F3_FLOOR_LED = 1;
                    printf("ELEVATOR MOVED TO F3.\r\n");
                break;
            }
        }
        free(elevator_ctl_recv);
        delay_ms(25);
    }
}
