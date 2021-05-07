/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nelisabe <nelisabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/05/06 12:56:25 by nelisabe          #+#    #+#             */
/*   Updated: 2021/05/07 14:42:21 by nelisabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <vector>

# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <unistd.h>
# include <errno.h>

// -- server settings --
# define SERVER_PORT 8080
# define MAX_CONNECTIONS 5
// ---------------------

typedef	struct sockaddr_in	t_sockaddr_in;
typedef	struct sockaddr		t_sockaddr;
typedef struct addrinfo 	t_addrinfo;
typedef unsigned int		uint;
typedef unsigned short		ushort;
typedef unsigned char		uchar;

class Server
{
	public:
		Server(void);
		virtual ~Server();

		bool	setup(int port);
		bool	connection(void);
		bool	readData(uchar **buffer);
		bool	sendData(char const *buffer) const;
	private:
		Server(Server const &);

		Server &operator=(Server const &);

		bool	_error(std::string const error) const;
		bool	_create_socket(void);

		ushort			_serverPort;
		int				_max_connections;
		int				_socket_ID;
		// struct containing address information for socket
		t_sockaddr_in	_socket_address;
		int				_client_socket_ID;

		int	const		_read_buffer_size;
};

#endif
