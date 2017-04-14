/* Copyright (C) 2015 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LWIPOPTS_CONF_H
#define LWIPOPTS_CONF_H

#define LWIP_TRANSPORT_ETHERNET       1

//#define MEM_SIZE                      (1600 * 16)

#define ENET_RX_RING_LEN              (16)
#define ENET_TX_RING_LEN              (8)
#define ENET_ETH_MAX_FLEN             (1522)	// recommended size for a VLAN frame
#define ENET_BUFF_ALIGNMENT			  16U		/*!< Ethernet buffer alignment. */

#define MEM_SIZE                      (ENET_RX_RING_LEN * (ENET_ETH_MAX_FLEN + ENET_BUFF_ALIGNMENT) + ENET_TX_RING_LEN * ENET_ETH_MAX_FLEN)


/* LwIP Stack Parameters (modified compared to initialization value in opt.h) -*/
/* Parameters set in STM32CubeMX LwIP Configuration GUI -*/
///*----- Value in opt.h for LWIP_DHCP: 0 -----*/
//#define LWIP_DHCP 1
///*----- Value in opt.h for NO_SYS: 0 -----*/
//#define NO_SYS 1
///*----- Value in opt.h for SYS_LIGHTWEIGHT_PROT: 1 -----*/
//#define SYS_LIGHTWEIGHT_PROT 0
///*----- Value in opt.h for MEM_ALIGNMENT: 1 -----*/
//#define MEM_ALIGNMENT 4
///*----- Default Value for MEMP_NUM_SYS_TIMEOUT: 5 ---*/
//#define MEMP_NUM_SYS_TIMEOUT 4
///*----- Value in opt.h for LWIP_ETHERNET: LWIP_ARP || PPPOE_SUPPORT -*/
//#define LWIP_ETHERNET 1
///*----- Value in opt.h for LWIP_DNS_SECURE: (LWIP_DNS_SECURE_RAND_XID | LWIP_DNS_SECURE_NO_MULTIPLE_OUTSTANDING | LWIP_DNS_SECURE_RAND_SRC_PORT) -*/
//#define LWIP_DNS_SECURE 7
///*----- Default Value for LWIP_TCP: 1 ---*/
//#define LWIP_TCP 0
///*----- Default Value for TCP_QUEUE_OOSEQ: 1 ---*/
//#define TCP_QUEUE_OOSEQ 0
///*----- Value in opt.h for TCP_SND_QUEUELEN: (4*TCP_SND_BUF + (TCP_MSS - 1))/TCP_MSS -----*/
//#define TCP_SND_QUEUELEN 9
///*----- Value in opt.h for TCP_SNDLOWAT: LWIP_MIN(LWIP_MAX(((TCP_SND_BUF)/2), (2 * TCP_MSS) + 1), (TCP_SND_BUF) - 1) -*/
//#define TCP_SNDLOWAT 1071
///*----- Value in opt.h for TCP_SNDQUEUELOWAT: LWIP_MAX(TCP_SND_QUEUELEN)/2, 5) -*/
//#define TCP_SNDQUEUELOWAT 5
///*----- Value in opt.h for TCP_WND_UPDATE_THRESHOLD: LWIP_MIN(TCP_WND/4, TCP_MSS*4) -----*/
//#define TCP_WND_UPDATE_THRESHOLD 536
///*----- Default Value for LWIP_CALLBACK_API: 1 ---*/
//#define LWIP_CALLBACK_API 0
///*----- Default Value for LWIP_NETIF_STATUS_CALLBACK: 0 ---*/
//#define LWIP_NETIF_STATUS_CALLBACK 1
///*----- Default Value for LWIP_NETIF_LINK_CALLBACK: 0 ---*/
//#define LWIP_NETIF_LINK_CALLBACK 1
///*----- Value in opt.h for LWIP_NETCONN: 1 -----*/
//#define LWIP_NETCONN 0
///*----- Value in opt.h for LWIP_SOCKET: 1 -----*/
//#define LWIP_SOCKET 0
///*----- Value in opt.h for LWIP_STATS: 1 -----*/
//#define LWIP_STATS 0
///*----- Value in opt.h for CHECKSUM_GEN_IP: 1 -----*/
//#define CHECKSUM_GEN_IP 0
///*----- Value in opt.h for CHECKSUM_GEN_UDP: 1 -----*/
//#define CHECKSUM_GEN_UDP 0
///*----- Value in opt.h for CHECKSUM_GEN_TCP: 1 -----*/
//#define CHECKSUM_GEN_TCP 0
///*----- Value in opt.h for CHECKSUM_GEN_ICMP: 1 -----*/
//#define CHECKSUM_GEN_ICMP 0
///*----- Value in opt.h for CHECKSUM_GEN_ICMP6: 1 -----*/
//#define CHECKSUM_GEN_ICMP6 0
///*----- Value in opt.h for CHECKSUM_CHECK_IP: 1 -----*/
//#define CHECKSUM_CHECK_IP 0
///*----- Value in opt.h for CHECKSUM_CHECK_UDP: 1 -----*/
//#define CHECKSUM_CHECK_UDP 0
///*----- Value in opt.h for CHECKSUM_CHECK_TCP: 1 -----*/
//#define CHECKSUM_CHECK_TCP 0
///*----- Value in opt.h for CHECKSUM_CHECK_ICMP: 1 -----*/
//#define CHECKSUM_CHECK_ICMP 0
///*----- Value in opt.h for CHECKSUM_CHECK_ICMP6: 1 -----*/
//#define CHECKSUM_CHECK_ICMP6 0
///*-----------------------------------------------------------------------------*/ 

#endif
