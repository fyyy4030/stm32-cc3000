
#include "platform_config.h"
#include "debug.h"
#include <stm32f10x_usart.h>
#include <stm32f10x_misc.h>

#define TO_HEX(i) ( (((i) & 0xf) <= 9) ? ('0' + ((i) & 0xf)) : ('A' - 10 + ((i) & 0xf)) )

void debug_setup() {
  USART_InitTypeDef usartInitStructure;
  GPIO_InitTypeDef gpioInitStructure;
  NVIC_InitTypeDef nvicInitStructure;

  /* Enable the USART1 Interrupt */
  nvicInitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  nvicInitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  nvicInitStructure.NVIC_IRQChannelSubPriority = 3;
  nvicInitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvicInitStructure);

  usartInitStructure.USART_BaudRate = DEBUG_USART_BAUD;
  usartInitStructure.USART_WordLength = USART_WordLength_8b;
  usartInitStructure.USART_Parity = USART_Parity_No;
  usartInitStructure.USART_StopBits = USART_StopBits_1;
  usartInitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  usartInitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Enable clocks */
  RCC_APB2PeriphClockCmd(DEBUG_USART_RCC, ENABLE);

  /* Configure USART Tx as alternate function push-pull */
  gpioInitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  gpioInitStructure.GPIO_Pin = DEBUG_USART_TX_PIN;
  gpioInitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DEBUG_USART_TX, &gpioInitStructure);

  /* Configure USART Rx as input floating */
  gpioInitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  gpioInitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
  GPIO_Init(DEBUG_USART_RX, &gpioInitStructure);

  /* USART configuration */
  USART_Init(DEBUG_USART, &usartInitStructure);

  /* Enable USART */
  USART_Cmd(DEBUG_USART, ENABLE);

  /* Enable the USART interrupts */
  USART_ITConfig(DEBUG_USART, USART_IT_RXNE, ENABLE);
  USART_ITConfig(DEBUG_USART, USART_IT_TXE, DISABLE);
}

/* !!! Interrupt handler - Don't change this function name !!! */
void USART1_IRQHandler(void) {
  if (USART_GetITStatus(DEBUG_USART, USART_IT_RXNE) != RESET) {
    uint8_t b[1];
    b[0] = USART_ReceiveData(DEBUG_USART);
    debug_on_rx(b, 1);
  }
}

void debug_write_line(const char* str) {
  debug_write(str);
  debug_write_ch('\n');
}

void debug_write_bytes(const uint8_t *data, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    debug_write_ch((char) data[i]);
  }
}

void debug_write(const char* str) {
  const char *p = str;
  while (*p) {
    debug_write_ch(*p);
    p++;
  }
}

void debug_write_ch(char ch) {
  USART_SendData(DEBUG_USART, ch);
  while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);
}

void debug_write_u16(uint8_t val, uint8_t base) {
  if (base == 16) {
    debug_write_ch(TO_HEX(val >> 12));
    debug_write_ch(TO_HEX(val >> 8));
    debug_write_ch(TO_HEX(val >> 4));
    debug_write_ch(TO_HEX(val >> 0));
  } else {
    debug_write("NOT IMPLEMENTED");
  }
}

void debug_write_u8(uint8_t val, uint8_t base) {
  if (base == 16) {
    debug_write_ch(TO_HEX(val >> 4));
    debug_write_ch(TO_HEX(val >> 0));
  } else {
    debug_write("NOT IMPLEMENTED");
  }
}
