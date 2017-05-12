/*!
 * @file
 * @brief Interface for M66 core functionality.
 *
 * Contains functions for enabling and disabling modem
 * and establishing IP connection
 *
 * @author Niranjan Rao
 * @date 2017-02-09
 *
 * @copyright &copy; 2015, 2016, 2017 ubirch GmbH (https://ubirch.com)
 *
 * ```
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ```
 */

#ifndef QUECTEL_M66_INTERFACE_H
#define QUECTEL_M66_INTERFACE_H

#define QUECTEL_M66_READ_BUFFER_SIZE					1024
#define QUECTEL_M66_PPP_READ_START_SIGNAL				0x01
#define QUECTEL_M66_PPP_READ_STOP_SIGNAL				0x02
//#define QUECTEL_M66_PPP_READ_DATA_SIGNAL				0x04

#include <stdint.h>
#include "mbed.h"
#include "QuectelM66CommandCoordinator.h"
#include "SerialStreamAdapter.h"

/** M66Interface class
 *  Implementation of the NetworkInterface for the Quectel M66 GSM modem
 */
class QuectelM66Interface : public NetworkInterface
{
public:
	
    /** M66Interface
     * @param tx        TX pin
     * @param rx        RX pin
     * @param pwrKey    POWERKEY pin
     * @param vdd_ext   VDD_EXT pin
     */
	QuectelM66Interface(PinName tx, PinName rx, PinName pwrKey, PinName vdd_ext, const char *apn, const char *username, const char *password);
	
	virtual ~QuectelM66Interface();

	    /** Get the local MAC address
	     *
	     *  Provided MAC address is intended for info or debug purposes and
	     *  may not be provided if the underlying network interface does not
	     *  provide a MAC address
	     *  
	     *  @return         Null-terminated representation of the local MAC address
	     *                  or null if no MAC address is available
	     */
	virtual const char *get_mac_address();

	    /** Get the local IP address
	     *
	     *  @return         Null-terminated representation of the local IP address
	     *                  or null if no IP address has been recieved
	     */
	virtual const char *get_ip_address();

	    /** Get the local network mask
	     *
	     *  @return         Null-terminated representation of the local network mask 
	     *                  or null if no network mask has been recieved
	     */
	virtual const char *get_netmask();

	    /** Get the local gateway
	     *
	     *  @return         Null-terminated representation of the local gateway
	     *                  or null if no network mask has been recieved
	     */
	virtual const char *get_gateway();

	    /** Set a static IP address
	     *
	     *  Configures this network interface to use a static IP address.
	     *  Implicitly disables DHCP, which can be enabled in set_dhcp.
	     *  Requires that the network is disconnected.
	     *
	     *  @param address  Null-terminated representation of the local IP address
	     *  @param netmask  Null-terminated representation of the local network mask
	     *  @param gateway  Null-terminated representation of the local gateway
	     *  @return         0 on success, negative error code on failure
	     */
	virtual nsapi_error_t set_network(
	        const char *ip_address, const char *netmask, const char *gateway);

	            /** Enable or disable DHCP on the network
	             *
	             *  Enables DHCP on connecting the network. Defaults to enabled unless
	             *  a static IP address has been assigned. Requires that the network is
	             *  disconnected.
	             *
	             *  @param dhcp     True to enable DHCP
	             *  @return         0 on success, negative error code on failure
	             */
	virtual nsapi_error_t set_dhcp(bool dhcp);

	    /** Start the interface
	     *
	     *  @return     0 on success, negative error code on failure
	     */
	virtual nsapi_error_t connect();

	    /** Stop the interface
	     *
	     *  @return     0 on success, negative error code on failure
	     */
	virtual nsapi_error_t disconnect();
	

protected:
	
	QuectelM66Interface(SerialStreamAdapter *serialStreamAdapter, PinName pwrKey, PinName vdd_ext, const char *apn, const char *userName, const char *passPhrase);
	
	friend class Socket;
	friend class UDPSocket;
	friend class TCPSocket;
	friend class TCPServer;
	friend class SocketAddress;
	template <typename IF>
		friend NetworkStack *nsapi_create_stack(IF *iface);

		    /** Provide access to the NetworkStack object
		     *
		     *  @return The underlying NetworkStack object
		     */
	virtual NetworkStack *get_stack();
	

private:
	
	QuectelM66CommandCoordinator _commandCoordinator;
	SerialStreamAdapter* _serialStreamAdapter;
	
	Thread _readProcessingThread;
	Queue <QuectelM66Interface, 10> _readNotificationQueue;
	
	bool _dhcp;
	char _ip_address[NSAPI_IPv4_SIZE];
	char _netmask[NSAPI_IPv4_SIZE];
	char _gateway[NSAPI_IPv4_SIZE];
	
	const char* _apn;
	const char* _username;
	const char* _password;
	
	uint8_t _serialBuffer[QUECTEL_M66_READ_BUFFER_SIZE];
	
	//void serial_read_notify();
	void serial_read_thread_entry();

};

#endif
