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
#include "kernel/dev/arch/cortexm/stm32f4xx/driverlib/stm32f4xx.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/types.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/target.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/gpio.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/uart.h"
#include "kernel/dev/arch/cortexm/stm32f4xx/spi.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/*******************************************************************************
* GPIO
*******************************************************************************/
const _Gpio_Descriptor Gpio_Descriptor[] = {
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_8,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD3
  {GPIO_TYPE_STD, GPIOD,  GPIO_Pin_9,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD3
//   {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_10,   0,  GPIO_FCT_IN,  0},   // GPIO_TXD3
//   {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_11,   0,  GPIO_FCT_IN,  0},   // GPIO_RXD3

  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_6,   0,  GPIO_MODE_IN,       0},   // GPIO_TXD6
  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_7,   0,  GPIO_MODE_IN,       0},   // GPIO_RXD6

  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_11,  0,  GPIO_MODE_IN,       0},   // GPIO_ID_MISO
  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_12,  0,  GPIO_MODE_IN,       0},   // GPIO_ID_MOSI
  {GPIO_TYPE_STD, GPIOC,  GPIO_Pin_10,  0,  GPIO_MODE_IN,       0},   // GPIO_ID_SCK

  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_8,   1,  GPIO_MODE_OUT,      1},   // GPIO_FLASH_CS

  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_6,   1,  GPIO_MODE_OUT,      1},   // GPIO_LED1
  {GPIO_TYPE_STD, GPIOF,  GPIO_Pin_7,   1,  GPIO_MODE_OUT,      1},   // GPIO_LED2

  {GPIO_TYPE_STD, GPIOA,  GPIO_Pin_0,   0,  GPIO_MODE_IN,       0}    // GPIO_WKUP

  };

/*******************************************************************************
* UART
*******************************************************************************/
