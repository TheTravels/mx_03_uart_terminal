/**
  ******************************************************************************
  * File Name          : terminal.c
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <string.h>
#include "terminal.h"
#include "terminal_8266.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"
#define WIFI_RX_BUFFER_SIZE  128
static uint8_t wifi_pdata[WIFI_RX_BUFFER_SIZE+1];
static uint8_t send_buf[WIFI_RX_BUFFER_SIZE+1];
static uint16_t send_buf_size=0;
static uint16_t wifi_pdata_len=0;

#define CS_CLOSED    0
#define CS_CONNECT   1
#define CS_SIZE      5
//static uint8_t connection_link[CS_SIZE]={0};
static uint8_t connection_status[CS_SIZE]={CS_SIZE};

static int receive_terminal_data(const uint32_t delay)
{
		uint32_t delay_tmp=0;
		int ret = -1;
		uint8_t _byte;
		for(delay_tmp=0; delay_tmp<delay; delay_tmp++)
		{
				//osDelay(2);
				osDelay(1);
				while(0==cache_queue_read_byte(&cache1_terminal, &_byte))
				{
						wifi_pdata[wifi_pdata_len] = _byte;
						wifi_pdata_len++;
						HAL_UART_Transmit(&huart2, &_byte, 1, 10);
						if(wifi_pdata_len>=(sizeof(wifi_pdata)))
						{
								ret = 0; // receive down
								break;
						}
				}
				if(0==terminal_rx_flag)
				{
						ret = 0; // receive down
						break;
				}
				terminal_rx_flag = 0;
		}
#if 0
				cache_queue_read_byte(&cache1_terminal, &_byte);
				{
						HAL_UART_Transmit(&huart2, &_byte, 1, 10);
				}
#endif
		__nop();
		return ret;
}
static int receive_terminal_data_status(const uint32_t delay, const uint8_t flag)
{
		uint32_t delay_tmp=0;
		int ret = -1;
		uint8_t _byte;
		for(delay_tmp=0; delay_tmp<delay; delay_tmp++)
		{
				//osDelay(2);
				osDelay(1);
				while(0==cache_queue_read_byte(&cache1_terminal, &_byte))
				{
						wifi_pdata[wifi_pdata_len] = _byte;
						if(wifi_pdata_len>0) wifi_pdata_len++;
						if(0==wifi_pdata_len)
						{
								if(flag==_byte) wifi_pdata_len++;
						}
						HAL_UART_Transmit(&huart2, &_byte, 1, 10);
						if(wifi_pdata_len>=(sizeof(wifi_pdata)))
						{
								ret = 0; // receive down
								break;
						}
				}
				if(0==terminal_rx_flag)
				{
						ret = 0; // receive down
						break;
				}
				terminal_rx_flag = 0;
		}
#if 0
				cache_queue_read_byte(&cache1_terminal, &_byte);
				{
						HAL_UART_Transmit(&huart2, &_byte, 1, 10);
				}
#endif
		__nop();
		return ret;
}

static void _wifi_module_enable(void)
{
	//HAL_GPIO_WritePin(WIFI_EN_GPIO_Port,WIFI_EN_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_RESET);
	osDelay(100);
	HAL_GPIO_WritePin(WIFI_RST_GPIO_Port,WIFI_RST_Pin,GPIO_PIN_SET);
//	at_delay(5000);
}

static int32_t _wait_mode_ready(const uint32_t delay)
{
		uint32_t delay_tmp=0;
		//uint32_t tickStart=0;
		terminal_rx_flag = 0;
		char* p=NULL;
		wifi_pdata_len=0;
		for(delay_tmp=0; delay_tmp<delay; delay_tmp+=2)
		{
				if(receive_terminal_data(2)) // 超时
				{
						continue;
				}
				p=strstr((const char*)wifi_pdata,(const char*)"ready");
				if(NULL!=p) return 0;
				wifi_pdata_len=0;
		}
		p=strstr((const char*)wifi_pdata,(const char*)"ready");
		if(NULL!=p) return 0;
		return -1;
}

static void _wifi_put_string(const char *str)
{
		HAL_UART_Transmit(&huart1,(uint8_t *)str,strlen((char*)str),50);
}
static void _wifi_put_data(const char *data, const uint16_t Size)
{
		HAL_UART_Transmit(&huart1,(uint8_t *)data, Size, 50);
}
//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
static int atk_8266_send_cmd_bak(const char *cmd, const char *ack, const uint16_t waittime)
{
		uint8_t *buf = wifi_pdata;
		uint16_t delay_tmp=0;
		char* p=NULL;
	
		sprintf((char*)buf, "%s\r\n",cmd);	//发送命令
		_wifi_put_string(buf);
		if((NULL==ack) || (0==waittime)) return 0; // OK
		terminal_rx_flag = 0;
		wifi_pdata_len=0;
		memset(wifi_pdata, 0, sizeof(wifi_pdata));
		for(delay_tmp=0; delay_tmp<waittime; delay_tmp+=2)
		{
				if(receive_terminal_data(2)) // 超时
				{
						continue;
				}
				p=strstr((const char*)wifi_pdata,(const char*)ack);
				if(NULL!=p) return 0;
				wifi_pdata_len=0;
				memset(wifi_pdata, 0, sizeof(wifi_pdata));
		}
		p=strstr((const char*)wifi_pdata,(const char*)ack);
		if(NULL!=p) return 0;
		return -1;
} 
static char* _atk_8266_send_cmd(const char *buf, const char *ack, const uint16_t waittime)
{
		//uint8_t *buf = wifi_pdata;
		uint16_t delay_tmp=0;
		char* p=NULL;
	
		//sprintf((char*)buf, "%s\r\n",cmd);	//发送命令
		_wifi_put_string((uint8_t *)buf);
		if((NULL==ack) || (0==waittime)) return 0; // OK
		terminal_rx_flag = 0;
		wifi_pdata_len=0;
		memset(wifi_pdata, 0, sizeof(wifi_pdata));
		for(delay_tmp=0; delay_tmp<waittime; delay_tmp+=2)
		{
				if(receive_terminal_data(2)) // 超时
				{
						continue;
				}
				p=strstr((const char*)wifi_pdata,(const char*)ack);
				if(NULL!=p) return p;
				wifi_pdata_len=0;
				memset(wifi_pdata, 0, sizeof(wifi_pdata));
		}
		p=strstr((const char*)wifi_pdata,(const char*)ack);
		if(NULL!=p) return p;
		printf("\r\n@%d %s  %s [%s]\r\n", __LINE__, __func__, buf, "FAIL");
		return NULL;
} 
static char* receive_atk_8266(const char *mark, uint16_t waittime)
{
		uint16_t delay_tmp=0;
		uint8_t link_id=0;
		char* p=NULL;
		if(NULL==mark) return 0; // OK
		if(0==waittime) waittime = 2;
		terminal_rx_flag = 0;
		wifi_pdata_len=0;
		memset(wifi_pdata, 0, sizeof(wifi_pdata));
		for(delay_tmp=0; delay_tmp<waittime; delay_tmp++)
		{
				//if(receive_terminal_data(1)) // 超时
				if(receive_terminal_data_status(1, mark[0]))
				{
						continue;
				}
				//connection_status=CS_CLOSED;
				p=strstr((const char*)wifi_pdata,",CLOSED\r\n");
				if(NULL!=p)
				{
						link_id = p[-1]-'0';
						printf("\r\n@%d %s [%d | %s]\r\n", __LINE__, __func__, link_id, "CLOSED");
						if(link_id>=CS_SIZE) return NULL;
						connection_status[link_id]=CS_CLOSED;
						return NULL;
				}
				//connection_status=CS_CONNECT;
				p=strstr((const char*)wifi_pdata,",CONNECT\r\n");
				if(NULL!=p)
				{
						link_id = p[-1]-'0';
						//printf("\r\n@%d %s [%s]\r\n", __LINE__, __func__, "CONNECT");
						printf("\r\n@%d %s [%d | %s]\r\n", __LINE__, __func__, link_id, "CONNECT");
						if(link_id>=CS_SIZE) return NULL;
						//connection_status=CS_CONNECT;
						connection_status[link_id]=CS_CONNECT;
						return NULL;
				}
				p=strstr((const char*)wifi_pdata,(const char*)mark);
				if(NULL!=p)
				{
						printf("\r\n@%d %s len:[%d]\r\n", __LINE__, __func__, wifi_pdata_len);
						return p;
				}
				wifi_pdata_len=0;
				memset(wifi_pdata, 0, sizeof(wifi_pdata));
		}
		p=strstr((const char*)wifi_pdata,(const char*)mark);
		if(NULL!=p) return p;
		return NULL;
} 
static char* atk_8266_send_cmd(const char *cmd, const char *ack, const uint16_t waittime)
{
		uint8_t *buf = wifi_pdata;	
		sprintf((char*)buf, "%s\r\n",cmd);	//发送命令
		return _atk_8266_send_cmd((const char *)buf, ack, waittime);
} 
//static void at_cwsap(char *ssid,char *pwd,uint8_t chl,uint8_t ecn,uint8_t max_conn)
//{
//		sprintf(at_cmd_buffer,"AT+CWSAP=\"%s\",\"%s\",%d,%d,%d\r\n",ssid,pwd,chl,ecn,max_conn);
//}
static int _setAP(const char *ssid, const char *pwd)
{
		char *buf = (char *)wifi_pdata;
		char *data=NULL;
		uint8_t chl;
		uint8_t ecn;
		uint8_t max_conn;
		int ret=0;
		//printf("SSID is %s", ssid);
		//printf("PWD is %s", pwd);
		if(NULL==ssid) return -1;
		printf("\r\n@%d%s  %s SSID is %s\r\n", __LINE__, __func__, __DATE__, ssid);
		if(NULL!=pwd) printf("\r\n@%d%s  %s PWD is %s\r\n", __LINE__, __func__, __DATE__, pwd);
	
		chl = 11;
		ecn = 0; // OPEN
		max_conn = 4;
		if(NULL != pwd) ecn = 4; // WPA_WPA2_PSK
#if 0
		sprintf((char*)buf,"AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d,%d\r\n",ssid,pwd,chl,ecn,max_conn);
		data = _atk_8266_send_cmd((const char *)buf, "OK", 20);
		if(NULL!=data)
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, data);
		}
#endif
	
		sprintf((char*)buf,"AT+CWSAP=\"%s\",\"%s\",%d,%d,%d\r\n",ssid,pwd,chl,ecn,max_conn);
#if 1
		data = _atk_8266_send_cmd((const char *)buf, "OK", 300);
		if(NULL!=data) return 0;
		printf("\r\n@%d %s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "SSID,PWD set fail");
		return -1;
#else
		ret = _atk_8266_send_cmd((const char *)buf, "OK", 20);
		if(0==ret) 
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "SSID,PWD set ok");
				return 0;
		}
		printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "SSID,PWD set fail");
		return -1;
#endif
}
static int _at_connect_ap(const char ssid[], const char pwd[])
{
		char *buf = (char *)wifi_pdata;
		char *data=NULL;
		sprintf(buf,"AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", ssid, pwd);
		//sprintf(at_cmd_buffer,"AT+CWJAP_DEF=\"%s\",\"%s\"\r\n", ssid, pwd);
		data = _atk_8266_send_cmd((const char *)buf, "WIFI GOT IP", 3000);
		if(NULL!=data) return 0;
		printf("\r\n@%d %s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "SSID,PWD set fail");
		return -1;
}
static void termina_wifi_init_bak(void)
{
//	uint32_t uwTick=0;
		int ret=0;
		char* p=0;
		char *buf = (char *)wifi_pdata;
		uint8_t data[] = "Hello World!";
		//uint32_t link_id_sync = -1;
		uint16_t* uid = (uint16_t*)(0x1FFFF7E8);
		uint16_t id=0;
		
		__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);	
		osDelay(100);
//		printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "SSID,PWD set check");
//		ret=_setAP("terminal", NULL);
//		if(0!=ret) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "SSID,PWD set fail");
//		sprintf((char*)buf,"AT+CWSAP_CUR=\"%s\",\"%s\",%d,%d,%d\r\n",ssid,pwd,chl,ecn,max_conn);
//		p = _atk_8266_send_cmd((const char *)buf, "OK", 20);
//		if(NULL!=p)
//		{
//				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, data);
//		}
		_wifi_module_enable();	
		//__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);	
		//wait_mode_ready(30000);//等待模块发送ready
		//_wait_mode_ready(3000);//等待模块发送ready
		if(0==_wait_mode_ready(3000))
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "start");
		}
		else
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "start...");
		}
		osDelay(500);//让模块进一步初始化
		//at_e(0);
		p=_atk_8266_send_cmd("ATE0\r\n","OK",20);
		//if(0==ret) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "OK");
		//else       printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		//osDelay(10);
		// at();
		p=_atk_8266_send_cmd("AT\r\n","OK",20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		osDelay(20);
		//if(setAP(data,"") == -1)
		ret=_setAP("terminal", NULL);
		if(0!=ret) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "SSID,PWD set fail");
		// at_cipmux(1);//多连接模式
		// sprintf(at_cmd_buffer,"AT+CIPMUX=%d\r\n",mode);
		p=_atk_8266_send_cmd("AT+CIPMUX=1\r\n","OK",20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		// at_cipmode(0);//设置普通传输模式
		// sprintf(at_cmd_buffer,"AT+CIPSERVER=%d\r\n",mode);
		p=_atk_8266_send_cmd("AT+CIPSERVER=0\r\n","OK",20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		// if(at_cwmode(3) != AT_OK)
		// sprintf(at_cmd_buffer,"AT+CWMODE=%d\r\n",mode);
		p=_atk_8266_send_cmd("AT+CWMODE=3\r\n","OK",20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		// if(at_cwautoconn(0) != AT_OK)
		// sprintf(at_cmd_buffer,"AT+CWAUTOCONN=%d\r\n",en);
		p=_atk_8266_send_cmd("AT+CWAUTOCONN=0\r\n","OK",20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		// at_cipap("192.168.2.3","","255.255.255.0");
		// sprintf(at_cmd_buffer,"AT+CIPAP=\"%s\",\"%s\",\"%s\"\r\n",ip,gateway,netmask);
		sprintf(buf,"AT+CIPAP=\"%s\",\"%s\",\"%s\"\r\n", "192.168.2.3", "", "255.255.255.0");
		p=_atk_8266_send_cmd(buf, "OK", 20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
		// at_cipserver(1,5000);//打开TCP服务器，端口好为5000
		// sprintf(at_cmd_buffer,"AT+CIPSERVER=%d,%d\r\n",mode,port);
		p=_atk_8266_send_cmd("AT+CIPSERVER=1,5000\r\n", "OK", 20);
		if(NULL==p) printf("\r\n@%d%s  %s [%s]\r\n", __LINE__, __func__, __DATE__, "FAIL");
}
static void termina_wifi_init(void)
{
		int ret=0;
		char* p=0;
		char *buf = (char *)wifi_pdata;
		uint8_t retry=0;
		uint8_t data[17] = {0};//"Hello World!";
		//uint16_t* uid = (uint16_t*)(0x1FFFF7E8);
		//uint16_t id=0;
		
		__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);	
wifi_init:
		osDelay(100);
		_wifi_module_enable();	
		//__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);	
		//_wait_mode_ready(3000);//等待模块发送ready
		if(0==_wait_mode_ready(3000))
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "start");
		}
		else
		{
				printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, "start...");
		}
		osDelay(500);//让模块进一步初始化
		_atk_8266_send_cmd("ATE0\r\n","OK",20);
		_atk_8266_send_cmd("AT\r\n","OK",20);
		osDelay(20);
		snprintf(buf, 16,"%s", __TIME__);
		//buf[8] = 0;
		//snprintf(data, 16,"terminal %s", &buf[5]); 
		ret=_setAP("terminal", NULL);
		//ret=_setAP(data, NULL);
		if(0!=ret)
		{
				//osDelay(200);
				retry++;
				if(retry<5) goto wifi_init;
		}
//		_at_connect_ap("server", "1234567890");
//		for(ret=0; ret<1000; ret++)
//		{
//				osDelay(20);
//				p=receive_atk_8266("WIFI GOT IP", 10);
//				if(NULL!=p) break;
//		}
//		osDelay(500);
//		_at_connect_ap("server", "1234567890");
		// at_cipmux(1);//多连接模式
		// sprintf(at_cmd_buffer,"AT+CIPMUX=%d\r\n",mode);
		//_atk_8266_send_cmd("AT+CIPMUX=1\r\n","OK",20);
		_atk_8266_send_cmd("AT+CIPMUX=1\r\n","OK",20);
		// at_cipmode(0);//设置普通传输模式
		// sprintf(at_cmd_buffer,"AT+CIPSERVER=%d\r\n",mode);
		_atk_8266_send_cmd("AT+CIPSERVER=0\r\n","OK",20);
		// if(at_cwmode(3) != AT_OK)
		// sprintf(at_cmd_buffer,"AT+CWMODE=%d\r\n",mode);
		_atk_8266_send_cmd("AT+CWMODE=3\r\n","OK",20);
		// if(at_cwautoconn(0) != AT_OK)
		// sprintf(at_cmd_buffer,"AT+CWAUTOCONN=%d\r\n",en);
		_atk_8266_send_cmd("AT+CWAUTOCONN=0\r\n","OK",20);
		// at_cipap("192.168.2.3","","255.255.255.0");
		// sprintf(at_cmd_buffer,"AT+CIPAP=\"%s\",\"%s\",\"%s\"\r\n",ip,gateway,netmask);
		sprintf(buf,"AT+CIPAP=\"%s\",\"%s\",\"%s\"\r\n", "192.168.2.3", "", "255.255.255.0");
		_atk_8266_send_cmd(buf, "OK", 20);
		// at_cipserver(1,5000);//打开TCP服务器，端口好为5000
		// sprintf(at_cmd_buffer,"AT+CIPSERVER=%d,%d\r\n",mode,port);
		_atk_8266_send_cmd("AT+CIPSERVER=1,60001\r\n", "OK", 20);
		//p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:APIP,", 20);
		//p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:OK", 20);
		p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:STAMAC,", 10);
		//if(NULL!=p) printf("\r\n@%d%s  APIP:%s\r\n", __LINE__, __func__, p);
#if 0
		p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:APMAC,", 10);
		if(NULL!=p) printf("\r\n@%d%s  APMAC:%s\r\n", __LINE__, __func__, p);
		p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:STAIP,", 10);
		if(NULL!=p) printf("\r\n@%d%s  STAIP:%s\r\n", __LINE__, __func__, p);
		p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:STAMAC,", 10);
		if(NULL!=p) printf("\r\n@%d%s  STAMAC:%s\r\n", __LINE__, __func__, p);
#endif
		//p=strstr((const char*)wifi_pdata,"+CIFSR:APIP,");
		//if(NULL!=p) printf("\r\n@%d%s  APIP:%s\r\n", __LINE__, __func__, p+strlen("+CIFSR:APIP,"));
		//p=strstr((const char*)wifi_pdata,"+CIFSR:APMAC,");
		//if(NULL!=p) printf("\r\n@%d%s  APMAC:%s\r\n", __LINE__, __func__, p+strlen("+CIFSR:APMAC,"));
		//p=_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:STAMAC,", 20);
		//p=strstr((const char*)wifi_pdata,"+CIFSR:STAIP,");
		//if(NULL!=p) printf("\r\n@%d%s  STAIP:%s\r\n", __LINE__, __func__, p+strlen("+CIFSR:STAIP,"));
		//p=strstr((const char*)wifi_pdata,"+CIFSR:STAMAC,");
		//if(NULL!=p) printf("\r\n@%d%s  STAMAC:%s\r\n", __LINE__, __func__, p+strlen("+CIFSR:STAMAC,"));
}

static void termina_handle_bak(const char *data, const uint16_t len)
{
		uint8_t link_id=0;
		//char *buf = (char *)wifi_pdata;
		char buf[64]={0};
		char send[WIFI_RX_BUFFER_SIZE+1]={0};
		char *p=NULL;
		memset(send, 0, sizeof(send)); // save data
		memcpy(send, data, len);
		//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
		printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, data);
		//for(link_id=0; link_id<CS_SIZE; link_id++)
		for(link_id=0; link_id<1; link_id++)
		{
				//if(CS_CONNECT!=connection_status[link_id]) continue;
				sprintf(buf,"AT+CIPSENDBUF=%d,%d\r\n",link_id,len);
				//printf("%s", buf);
				printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, buf);
				//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
				//_wifi_put_string(buf);
				//receive_atk_8266(">\r\n", 1);
				//p = _atk_8266_send_cmd(buf, "OK", 10);
				p = _atk_8266_send_cmd(buf, ">", 10);
				//if(NULL==p) continue;
				osDelay(10);
				printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, "SEND");
				//_wifi_put_data(data, len);
				//_wifi_put_string(data);
				//receive_atk_8266("SEND OK", 10);
				p = _atk_8266_send_cmd(send, "SEND OK", 10);
				//if(NULL==p) _atk_8266_send_cmd(data, "SEND OK", 10);
				printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, data);
		}
	//uart_put_data(data,len);
	//return at_cmd_process_frame();
}

static char* termina_handle(const char *data, const uint16_t len)
{
		uint8_t link_id=0;
		uint8_t count=0;
		//char *buf = (char *)wifi_pdata;
		char buf[64]={0};
		//char send[WIFI_RX_BUFFER_SIZE+1]={0};
		char *p=NULL;
		//memset(send, 0, sizeof(send)); // save data
		//memcpy(send, data, len);
		//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
		//printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, data);
		//for(link_id=0; link_id<CS_SIZE; link_id++)
		for(link_id=0; link_id<1; link_id++)
		{
				//if(CS_CONNECT!=connection_status[link_id]) continue;
				//sprintf(buf,"AT+CIPSENDBUF=%d,%d\r\n",link_id,len);
				sprintf(buf,"AT+CIPSEND=%d,%d\r\n",link_id,len);
				//printf("%s", buf);
				//printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, buf);
				//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
				//_wifi_put_string(buf);
				//receive_atk_8266(">\r\n", 1);
				//p = _atk_8266_send_cmd(buf, "OK", 10);
				p = _atk_8266_send_cmd(buf, ">", 10);
				if(NULL==p) continue;
				_wifi_put_data(data, len);
				//_wifi_put_data("ACK\r\n", 5);
				//_wifi_put_string(data);
				for(count=0; count<100; count++) // 1m
				{
						osDelay(1);
						p=receive_atk_8266("SEND OK", 10);
						if(NULL!=p)
						{
								printf("\r\n@%d%s [%d]%s", __LINE__, __func__, count, "SEND OK");
								break;
						}
				}
				printf("\r\n@%d%s [%d]%s", __LINE__, __func__, count, "FAIL");
				//p = _atk_8266_send_cmd(send, "SEND OK", 10);
				//if(NULL==p) _atk_8266_send_cmd(data, "SEND OK", 10);
				//printf("\r\n@%d%s  %s\r\n", __LINE__, __func__, data);
		}
		p=strstr((const char*)wifi_pdata, "+IPD,");
		if(NULL!=p) return p;
		return NULL;
	//uart_put_data(data,len);
	//return at_cmd_process_frame();
}
static void termina_ack(void)
{
		termina_handle("ACK\r\n", 5);
}
void boot_send(const char *data, const uint16_t len)
{
		uint16_t delay=0;
		__asm("CPSID  I");
		memcpy(send_buf, data, len);
		send_buf_size=len;
		__asm("CPSIE  I");
		for(delay=0; delay<1000; delay++) // watting
		{
				osDelay(2);
				if(send_buf_size&0x8000) break;
		}
}
//void StartWiFiTask(void const * argument)
void StartTerminalTask(void const * argument)
{
  /* Infinite loop */
	char *data=NULL;
	char *len_end=NULL;
	uint16_t len=0;
	int8_t i=0,j=0;
	//uint8_t _byte=0;
	char recv[WIFI_RX_BUFFER_SIZE+1]={0};
	printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
	//printf("\r\n+IPD\r\n");
	//HAL_UART_Transmit(&huart1, "StartWiFiTask", 13,100);
	init_queue(cache);
	init_queue(cache1_terminal);
	init_queue(cache1_terminal_tx);
	init_queue(cache_board);
	termina_wifi_init();
		//osDelay(500);
#if 0
	if(0!=_at_connect_ap("server", "1234567890"))
	for(len=0; len<500; len++)
	{
			osDelay(20);
			data=receive_atk_8266("WIFI GOT IP", 10);
			if(NULL!=data) break;
	}
	osDelay(2000);
	_atk_8266_send_cmd("AT+CIFSR\r\n", "+CIFSR:STAMAC,", 10);
	//osDelay(2000);
	printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
	_atk_8266_send_cmd("AT+CIPSTART=0,\"TCP\",\"192.168.155.1\",60000\r\n", "OK", 50);
	osDelay(500);
	termina_handle("ACK\r\n", 5);
	termina_handle("ACK\r\n", 5);
#endif
  for(;;)
  {
			data=receive_atk_8266("+IPD,", 10);
			//receive_terminal_data(2);
#if 0
			if(NULL!=data)
			{
					printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, &data[5]);
					data=strstr((const char*)&data[5],",");
					printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, data);
					len_end=strstr((const char*)data,":");
					if(NULL==len_end) continue;
					j=len_end-data-1;
					printf("\r\n%s  %s %s[%d]\r\n", __func__, __DATE__, len_end, j);
					len=0;
					data++; // skip ','
					for(i=0; i<j; i++)
					{
							if((data[i]>='0') && (data[i]<='9'))
							{
									len = len*10+data[i]-'0';
									continue;
							}
							i=0;
							break;
					}
					if(0==i) continue;
					len_end++;  // skip ':'
					printf("\r\n%s  %s %s[%d]\r\n", __func__, __DATE__, len_end, len);
					termina_handle(len_end, len);
			}
#else
			handle:
			if(NULL!=data)
			{
					// eg
					// +IPD,0,22:http://www.cmsoft.cn
					//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, &data[5]);
					data=strstr((const char*)&data[5],",");
					//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, data);
					len_end=strstr((const char*)data,":");
					if(NULL==len_end) continue;
					j=len_end-data-1;
					//printf("\r\n%s  %s %s[%d]\r\n", __func__, __DATE__, len_end, j);
					len=0;
					data++; // skip ','
					for(i=0; i<j; i++)
					{
							if((data[i]>='0') && (data[i]<='9'))
							{
									len = len*10+data[i]-'0';
									continue;
							}
							i=0;
							break;
					}
					if(0==i) continue;
					len_end++;  // skip ':'
					printf("\r\n%s  %s %s[%d]\r\n", __func__, __DATE__, len_end, len);
					memset(recv, 0, sizeof(recv)); // save data
					memcpy(recv, len_end, len);
					//termina_handle("ACK\r\n", 5);
					//osDelay(10);
					data = termina_handle(recv, len);
					for(i=0; i<len; i++)
					{
							__asm("CPSID  I");
							macro_queue_write(recv[i], cache_board);
							__asm("CPSIE  I");
					}
					goto handle;
			}
#endif
			__asm("CPSID  I");
			len = send_buf_size;
			memset(recv, 0, sizeof(recv));
			memcpy(recv, send_buf, len);
			__asm("CPSIE  I");
			if( (len&0xFF))
			{
					termina_handle(recv, (len&0xFF));
					__asm("CPSID  I");
					send_buf_size = 0x8000;
					__asm("CPSIE  I");
			}
			osDelay(1);
  }
  /* USER CODE END StartWiFiTask */
}


