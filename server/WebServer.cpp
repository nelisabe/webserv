/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nelisabe <nelisabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/07/16 14:00:27 by nelisabe          #+#    #+#             */
/*   Updated: 2021/07/26 11:57:27 by nelisabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "WebServer.hpp"

WebServer::WebServer() {}

WebServer::WebServer(const WebServer &) {}

WebServer::~WebServer()
{
	if (!_servers.empty())
		for (std::vector<Server*>::iterator it = _servers.begin();\
			it < _servers.end(); it++)
			delete *it;
}

WebServer	&WebServer::operator=(const WebServer &) { return *this; }

bool		WebServer::Setup(const config::WebserverConf &config)
{
	Server	*server;

	for (std::map<int, std::string>::const_iterator it = config.getPorts().begin();\
		it != config.getPorts().end(); it++)
	{
		server = new Server();
		
		const std::map<std::string, config::tServer> server_conf = \
			config.getServerMap().at((*it).first);
		if (server->Setup((*it).first, (*it).second, server_conf, \
			&_fd_controller) == FAILURE)
		{
			delete server;
			return FAILURE;
		}
		_servers.push_back(server);
	}
	return SUCCESS;
}

bool		WebServer::Connection(void)
{
	size_t	timeout_seconds = 180; // 3 minutes

	InitFdSets();
	if (_fd_controller.Wait(timeout_seconds) == FAILURE)
		return Error("select");
	for (std::vector<Server*>::iterator it = _servers.begin();\
		it < _servers.end(); it++)
		(*it)->HandleClients(timeout_seconds);
	return SUCCESS;
}

void		WebServer::InitFdSets(void)
{
	_fd_controller.Clear();
	for (std::vector<Server*>::iterator it = _servers.begin();\
		it < _servers.end(); it++)
		_fd_controller.AddFDToWatch((*it)->getSocket(), SelectController::READ);
	for (std::vector<Server*>::iterator it = _servers.begin();\
		it < _servers.end(); it++)
		(*it)->AddClientsSockets();
}
