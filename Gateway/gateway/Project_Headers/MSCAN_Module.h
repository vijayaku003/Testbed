/*
 * MSCAN_Module.h
 *
 *  Created on: Apr 22, 2014
 *      Author: Kitty
 */

#ifndef MSCAN_MODULE_H_
#define MSCAN_MODULE_H_


void MSCAN_ModuleEn(void);
void MSCAN_Rx_IRQHandler(void);
void MSCAN_Tx_IRQHandler(void);

#endif /* MSCAN_MODULE_H_ */
