/*
The contents of this file are subject to the Mozilla Public License Version 1.1
(the "License"); you may not use this file except in compliance with the License.
You may obtain a copy of the License at http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for the
specific language governing rights and limitations under the License.

The Original Code is Lepton.

The Initial Developer of the Original Code is Philippe Le Boulanger.
Portions created by Philippe Le Boulanger are Copyright (C) 2014 <lepton.phlb@gmail.com>.
All Rights Reserved.

Contributor(s): Jean-Jacques Pitrolle <lepton.jjp@gmail.com>.

Alternatively, the contents of this file may be used under the terms of the eCos GPL license
(the  [eCos GPL] License), in which case the provisions of [eCos GPL] License are applicable
instead of those above. If you wish to allow use of your version of this file only under the
terms of the [eCos GPL] License and not to allow others to use your version of this file under
the MPL, indicate your decision by deleting  the provisions above and replace
them with the notice and other provisions required by the [eCos GPL] License.
If you do not delete the provisions above, a recipient may use your version of this file under
either the MPL or the [eCos GPL] License."
*/

/* Includes ------------------------------------------------------------------*/
#include "kernel/core/kernelconf.h"
#include "kernel/core/types.h"
#include "kernel/core/interrupt.h"
#include "kernel/core/kernel.h"
#include "kernel/core/system.h"
#include "kernel/core/fcntl.h"
#include "kernel/core/cpu.h"
#include "kernel/fs/vfs/vfsdev.h"
#include "kernel/core/malloc.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/target.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/dma.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/uart.h"

#include "kernel/dev/arch/cortexm/stm32f4xx/dev_stm32f4xx/dev_stm32f4xx_uart_x.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define uart_is_rx_hw_fc(Uart)      gpio_read_output((*Uart->Ctrl)->Gpio)
#define uart_set_rx_hw_fc(Uart)     gpio_set((*Uart->Ctrl)->Gpio)
#define uart_reset_rx_hw_fc(Uart)   gpio_reset((*Uart->Ctrl)->Gpio)
#define uart_tx_enable(Uart)        gpio_set((*Uart->Ctrl)->Gpio)
#define uart_tx_disable(Uart)       gpio_reset((*Uart->Ctrl)->Gpio)

//#define _malloc   malloc
//#define _free     free

/* Private variables & constants ---------------------------------------------*/
_Uart_Ctrl *Uart_Ctrl[UART_NB];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : uart_open
* Description    : Open the UART port communication
* Input          : - Uart: Select the USART or the UART peripheral
*                : - BaudRate: Baud rate configuration
*                : - DmaBufSize: DMA buffer size
*                : - RxBufSize: Receive buffer size
*                : - TxBufSize: Transmit buffer size
*                : - HwCtrl: Hardware control options
*                : - Gpio: GPIO used for hardware control
* Output         : None
* Return         : 0 if OK, -1 in case of error
*******************************************************************************/
int uart_open (const _Uart_Descriptor *Uart, u32 BaudRate, u8 DmaBufSize, u16 RxBufSize, u16 TxBufSize, u8 HwCtrl, const _Gpio_Descriptor *Gpio)
{
  USART_InitTypeDef usart_init_structure;
  DMA_InitTypeDef dma_init_structure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Init control variables and bufers */
  if (*Uart->Ctrl) _sys_free(*Uart->Ctrl);
  if ((*Uart->Ctrl = _sys_malloc(sizeof(_Uart_Ctrl) + DmaBufSize + RxBufSize + TxBufSize)) == 0) return(-1);
  memset(*Uart->Ctrl, 0, sizeof(_Uart_Ctrl));
  (*Uart->Ctrl)->DmaBufPtr = (char *)*Uart->Ctrl + sizeof(_Uart_Ctrl);
  (*Uart->Ctrl)->DmaBufSize = DmaBufSize;
  (*Uart->Ctrl)->iDma = DmaBufSize;
  (*Uart->Ctrl)->RxBufSize = RxBufSize;
  (*Uart->Ctrl)->RxBufPtr = (*Uart->Ctrl)->DmaBufPtr + (*Uart->Ctrl)->DmaBufSize;
  (*Uart->Ctrl)->TxBufSize = TxBufSize;
  (*Uart->Ctrl)->TxBufPtr = (*Uart->Ctrl)->RxBufPtr + (*Uart->Ctrl)->RxBufSize;
  (*Uart->Ctrl)->HwCtrl = HwCtrl;
  (*Uart->Ctrl)->Gpio = Gpio;
  #ifdef _UART_OS_SUPPORT
    (*Uart->Ctrl)->Event = SYS_EVT_INCOMING_DATA;
    (*Uart->Ctrl)->Task = sys_task_self();
  #endif

  /* Enable peripheral clock */
  (*Uart->RCC_APBxPeriphClockCmd)(Uart->RCC_APBxPeriph, ENABLE);

  /* Init GPIO */
  gpio_set_function(Uart->TxGpio, Uart->GPIO_AF);
  gpio_set_function(Uart->RxGpio, Uart->GPIO_AF);
  gpio_set_mode(Uart->TxGpio, GPIO_MODE_AF, 0);
  gpio_set_mode(Uart->RxGpio, GPIO_MODE_AF, 0);
  if ((*Uart->Ctrl)->HwCtrl & (UART_HW_FLOW_CTRL_RX | UART_HALF_DUPLEX)) gpio_set_mode((*Uart->Ctrl)->Gpio, GPIO_FCT_OUT, 0);

  /* Init UART peripheral */
  USART_DeInit(Uart->UARTx);
  USART_StructInit(&usart_init_structure);
  usart_init_structure.USART_BaudRate = BaudRate;
  if ((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_TX) usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_CTS;
  USART_Init(Uart->UARTx, &usart_init_structure);

  /* Configure DMA (if used) */
  if ((*Uart->Ctrl)->DmaBufSize)
  {
    DMA_DeInit(Uart->DMAy_Streamx);
    DMA_StructInit(&dma_init_structure);
    dma_init_structure.DMA_Channel = Uart->DMA_Channel;
    dma_init_structure.DMA_PeripheralBaseAddr = (u32)&Uart->UARTx->DR;
    dma_init_structure.DMA_Memory0BaseAddr = (u32)(*Uart->Ctrl)->DmaBufPtr;
    dma_init_structure.DMA_BufferSize = (*Uart->Ctrl)->DmaBufSize;
    dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_Mode = DMA_Mode_Circular;
    dma_init_structure.DMA_Priority = DMA_Priority_Medium;
    dma_init_structure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_Init(Uart->DMAy_Streamx, &dma_init_structure);
    DMA_ITConfig(Uart->DMAy_Streamx, DMA_IT_TC | DMA_IT_HT, ENABLE);
    DMA_Cmd(Uart->DMAy_Streamx, ENABLE);
    NVIC_EnableIRQ(Uart->DMAx_IRQn);
    //
    //NVIC_SetPriority((IRQn_Type)Uart->DMAx_IRQn, (uint32_t)129);
    NVIC_SetPriority((IRQn_Type)Uart->DMAx_IRQn, (1 << __NVIC_PRIO_BITS) -3);
    //
    USART_DMACmd(Uart->UARTx, USART_DMAReq_Rx, ENABLE);
    USART_ITConfig(Uart->UARTx, USART_IT_IDLE, ENABLE);
  }
  else USART_ITConfig(Uart->UARTx, USART_IT_RXNE, ENABLE);

  /* Enable IT and start peripheral */
  //NVIC_InitStructure.NVIC_IRQChannel = Uart->IRQn;		 // we want to configure the USART1 interrupts
  //NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;// this sets the priority group of the USART1 interrupts
  //NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		 // this sets the subpriority inside the group
  //NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			 // the USART1 interrupts are globally enabled
  //NVIC_Init(&NVIC_InitStructure);	
  //
  //NVIC_SetPriority((IRQn_Type)Uart->IRQn, (uint32_t)129);
  NVIC_SetPriority((IRQn_Type)Uart->IRQn, (1 << __NVIC_PRIO_BITS) -4);
  //
   
  USART_ITConfig(Uart->UARTx, USART_IT_TC, ENABLE);
  NVIC_EnableIRQ(Uart->IRQn);
  USART_Cmd(Uart->UARTx, ENABLE);
  
  
  while (!gpio_read(Uart->TxGpio));

  return(0);
}

/*******************************************************************************
* Function Name  : uart_close
* Description    : Close the USART port communication
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : 0
*******************************************************************************/
int uart_close(const _Uart_Descriptor *Uart)
{
  /* Disable IT */
  NVIC_DisableIRQ(Uart->IRQn);

  /* Free control variables and buffers */
  if (*Uart->Ctrl)
  {
    if ((*Uart->Ctrl)->DmaBufSize)
    {
      USART_DMACmd(Uart->UARTx, USART_DMAReq_Rx, DISABLE);
      NVIC_DisableIRQ(Uart->DMAx_IRQn);
      DMA_DeInit(Uart->DMAy_Streamx);
    }
    if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) || ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX)) gpio_init((*Uart->Ctrl)->Gpio);
    _sys_free(*Uart->Ctrl);
    *Uart->Ctrl = 0;
  }

  /* Deactivate UART peripheral */
  USART_DeInit(Uart->UARTx);

  /* Disable peripheral clock */
  (*Uart->RCC_APBxPeriphClockCmd)(Uart->RCC_APBxPeriph, DISABLE);

  /* Configure GPIO to default state */
  gpio_init(Uart->TxGpio);
  gpio_init(Uart->RxGpio);

  return(0);
}

/*******************************************************************************
* Function Name  : uart_irq_handler
* Description    : Interrupt handle function
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : None
*******************************************************************************/
void uart_irq_handler(const _Uart_Descriptor *Uart)
{
  //
  __hw_enter_interrupt();
  //
  if (*Uart->Ctrl && (*Uart->Ctrl)->DmaBufSize)
  {
    if (USART_GetITStatus(Uart->UARTx, USART_IT_IDLE) != RESET)                          // Idle line
    {
      NVIC_SetPendingIRQ(Uart->DMAx_IRQn);
      USART_ReceiveData(Uart->UARTx);
    }
  }
  else if (USART_GetITStatus(Uart->UARTx, USART_IT_RXNE) != RESET)                       // Received Data Ready to be Read
  {
    if (*Uart->Ctrl && ((*Uart->Ctrl)->RxCnt < (*Uart->Ctrl)->RxBufSize))
    {
      (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiPut++] = (char)USART_ReceiveData(Uart->UARTx);
      //lepton
      if(!(*Uart->Ctrl)->RxCnt){
        if(Uart->board_uart_info && Uart->board_uart_info->desc_r!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_r].owner_pthread_ptr_read);
      }
      //lepton
      (*Uart->Ctrl)->RxCnt++;
      if ((*Uart->Ctrl)->RxiPut >= (*Uart->Ctrl)->RxBufSize) (*Uart->Ctrl)->RxiPut = 0;
      if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && ((*Uart->Ctrl)->RxCnt > ((*Uart->Ctrl)->RxBufSize - (*Uart->Ctrl)->DmaBufSize))) uart_set_rx_hw_fc(Uart);
    }
    else USART_ClearITPendingBit(Uart->UARTx, USART_IT_RXNE);
    #ifdef _UART_OS_SUPPORT
      isr_evt_set((*Uart->Ctrl)->Event, (*Uart->Ctrl)->Task);
    #endif
  }

  if (USART_GetITStatus(Uart->UARTx, USART_IT_TXE) != RESET)                             // Transmit Data Register Empty
  {
    if (*Uart->Ctrl && (*Uart->Ctrl)->TxCnt)
    {
      USART_SendData(Uart->UARTx, (*Uart->Ctrl)->TxBufPtr[(*Uart->Ctrl)->TxiGet++]);
      (*Uart->Ctrl)->TxCnt--;
      if ((*Uart->Ctrl)->TxiGet >= (*Uart->Ctrl)->TxBufSize) (*Uart->Ctrl)->TxiGet = 0;
    }
    else{
      USART_ITConfig(Uart->UARTx, USART_IT_TXE, DISABLE);
      //lepton
        if(Uart->board_uart_info && Uart->board_uart_info->desc_w!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_w].owner_pthread_ptr_write);
      //lepton
    }
  }

  if (USART_GetITStatus(Uart->UARTx, USART_IT_TC) != RESET)                              // Transmission complete
  {
    if (*Uart->Ctrl && ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX)) 
      uart_tx_disable(Uart);
    USART_ClearITPendingBit(Uart->UARTx, USART_IT_TC);
  }
  //
  __hw_leave_interrupt();
}

/*******************************************************************************
* Function Name  : uart_dma_irq_handler
* Description    : DMA interrupt handle function
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : None
*******************************************************************************/
void uart_dma_irq_handler(const _Uart_Descriptor *Uart)
{
  //
  __hw_enter_interrupt();
  //
  if (*Uart->Ctrl)
  {
    while ((DMA_GetCurrDataCounter(Uart->DMAy_Streamx) != (*Uart->Ctrl)->iDma) && ((*Uart->Ctrl)->RxCnt < (*Uart->Ctrl)->RxBufSize))
    {
      (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiPut++] = (*Uart->Ctrl)->DmaBufPtr[(*Uart->Ctrl)->DmaBufSize - (*Uart->Ctrl)->iDma--];
      if (!(*Uart->Ctrl)->iDma) (*Uart->Ctrl)->iDma = (*Uart->Ctrl)->DmaBufSize;
      //lepton
      if(!(*Uart->Ctrl)->RxCnt){
        if(Uart->board_uart_info && Uart->board_uart_info->desc_r!=-1)
           __fire_io_int(ofile_lst[Uart->board_uart_info->desc_r].owner_pthread_ptr_read);
      }
      //lepton
      (*Uart->Ctrl)->RxCnt++;
      if ((*Uart->Ctrl)->RxiPut >= (*Uart->Ctrl)->RxBufSize) (*Uart->Ctrl)->RxiPut = 0;
      if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && ((*Uart->Ctrl)->RxCnt > ((*Uart->Ctrl)->RxBufSize - (*Uart->Ctrl)->DmaBufSize))) uart_set_rx_hw_fc(Uart);
    }
  }
  #ifdef _UART_OS_SUPPORT
    isr_evt_set((*Uart->Ctrl)->Event, (*Uart->Ctrl)->Task);
  #endif
  //
  DMA_ClearITPendingBit(Uart->DMAy_Streamx, DMA_IT_ALL_1);
  //
  __hw_leave_interrupt();
  //
}

/*******************************************************************************
* Function Name  : uart_get_char
* Description    : Get a single data byte from the USARTx RxBuffer.
* Input          : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : The data read from the buffer (-1 if the RxBuffer is empty)
*******************************************************************************/
int uart_get_char(const _Uart_Descriptor *Uart)
{
  char data;

  #ifdef _UART_OS_SUPPORT
    if (!(*Uart->Ctrl) || (sys_task_self() != (*Uart->Ctrl)->Task) || !(*Uart->Ctrl)->RxCnt) return(-1);
  #else
    if (!(*Uart->Ctrl) || !(*Uart->Ctrl)->RxCnt) return(-1);
  #endif
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_DisableIRQ(Uart->DMAx_IRQn);
  else 
    USART_ITConfig(Uart->UARTx, USART_IT_RXNE, DISABLE);
  //
  data = (*Uart->Ctrl)->RxBufPtr[(*Uart->Ctrl)->RxiGet++];
  (*Uart->Ctrl)->RxCnt--;
  //
  if ((*Uart->Ctrl)->RxiGet >= (*Uart->Ctrl)->RxBufSize) 
    (*Uart->Ctrl)->RxiGet = 0;
  //
  if (((*Uart->Ctrl)->HwCtrl & UART_HW_FLOW_CTRL_RX) && (uart_is_rx_hw_fc(Uart)) && ((*Uart->Ctrl)->RxCnt < ((*Uart->Ctrl)->RxBufSize / 2))) 
    uart_reset_rx_hw_fc(Uart);
  //
  if ((*Uart->Ctrl)->DmaBufSize) 
    NVIC_EnableIRQ(Uart->DMAx_IRQn);
  else 
    USART_ITConfig(Uart->UARTx, USART_IT_RXNE, ENABLE);
  
  return((int)data);
}

/*******************************************************************************
* Function Name  : uart_put_char
* Description    : Put a single data byte to the USARTx TxBuffer.
* Input          : - Data: the data to transmit.
*                  - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : 0 if OK, -1 if ERROR
*******************************************************************************/
int uart_put_char(char Data, const _Uart_Descriptor *Uart)
{
  if (!(*Uart->Ctrl) || !(*Uart->Ctrl)->TxBufPtr) 
    return(-1);
  //
  while ((*Uart->Ctrl)->TxCnt == (*Uart->Ctrl)->TxBufSize)
  {
    #ifdef _UART_OS_SUPPORT
      sys_wait(1);
    #endif
  }
  //
  //USART_ITConfig(Uart->UARTx, USART_IT_TXE, DISABLE);
  //
  (*Uart->Ctrl)->TxBufPtr[(*Uart->Ctrl)->TxiPut++] = Data;
  (*Uart->Ctrl)->TxCnt++;
  //
  if ((*Uart->Ctrl)->TxiPut >= (*Uart->Ctrl)->TxBufSize) 
    (*Uart->Ctrl)->TxiPut = 0;
  //
  //if ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX) 
  //  uart_tx_enable(Uart);
  //
  //USART_ITConfig(Uart->UARTx, USART_IT_TXE, ENABLE);
  return(0);
}

/*******************************************************************************
* Function Name  : uart_read
* Description    : Read data bytes from the USARTx RxBuffer.
* Input          : - WrPtr: Location where the first byte is to be stored
*                : - Size: Number of bytes to be read
*                : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : Number of bytes read from the UARTx RxBuffer (-1 if the number
*                  of bytes to be read superior to the number of bytes available)
*******************************************************************************/
int uart_read(void *WrPtr, u16 Size, const _Uart_Descriptor *Uart)
{
  u16 i;
  char *ptr;
  int ch;
  int cb;

  //
  if (!(*Uart->Ctrl)) 
    return(-1);
  //
  if((*Uart->Ctrl)->RxCnt>Size)
    cb=Size;
  else
    cb=(*Uart->Ctrl)->RxCnt;
  //
  ptr = (char *)WrPtr;
  //
  for (i = 0 ; i < cb ; i++)
  {
    if ((ch = uart_get_char(Uart)) == -1) 
      break;
    *ptr++ = (char)ch;
  }
  return((int)(i));
}

/*******************************************************************************
* Function Name  : uart_write
* Description    : Write data bytes to the USARTx TxBuffer.
* Input          : - RdPtr: Location where the first byte is to be read
*                : - Size: Number of bytes to be written
*                : - Uart: Select the USART or the UART peripheral.
* Output         : None
* Return         : -1 if ERROR, Number of char not written (0 if OK)
*******************************************************************************/
int uart_write(const void *RdPtr, u16 Size, const _Uart_Descriptor *Uart)
{
  u16 i;
  char *ptr = (char *)RdPtr;
  
  if(Size>(*Uart->Ctrl)->TxBufSize)
     Size=(*Uart->Ctrl)->TxBufSize;

  //
  USART_ITConfig(Uart->UARTx, USART_IT_TXE, DISABLE);
  //
  for (i = 0 ; i < Size ; i++){ 
    if (uart_put_char(*ptr++, Uart) == -1) 
      break;
  }
  //
  if ((*Uart->Ctrl)->HwCtrl & UART_HALF_DUPLEX) 
    uart_tx_enable(Uart);
  //
  USART_ITConfig(Uart->UARTx, USART_IT_TXE, ENABLE);
  //
  return((int)(i));
}
