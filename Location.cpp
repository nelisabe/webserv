/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bbenny <bbenny@student.21-school.ru>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/04/23 12:40:02 by bbenny            #+#    #+#             */
/*   Updated: 2021/04/23 12:40:04 by bbenny           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

namespace config{

Location::Location()
{
	//autoindex = "off";
	accepted_methods.insert(std::make_pair("GET", 0));//!
	//method.insert(std::make_pair("HEAD", 0));
	accepted_methods.insert(std::make_pair("POST", 0));//!
	//method.insert(std::make_pair("PUT", 0));
	accepted_methods.insert(std::make_pair("DELETE", 0));//!
	//method.insert(std::make_pair("CONNECT", 0));
	///method.insert(std::make_pair("OPTIONS", 0));
	//method.insert(std::make_pair("TRACE", 0));
	//method.insert(std::make_pair("PATCH", 0));
}
Location::~Location()
{}

Location::Location(Location const &copy){*this = copy;}
Location &Location::operator=(Location const &eq)
{
	locationMask = eq.locationMask;
	accepted_methods = eq.accepted_methods;
	root = eq.root;
	autoindex = eq.autoindex;
	return (*this);
}
		

void	Location::fillAll(list<string>::iterator &itList, list<std::string> tokenList)
{
	map<string, void (Location::*)(list<string>::iterator &itList, list<std::string> tokenList)> funMap;
	map<string, void (Location::*)(list<string>::iterator &itList, list<std::string> tokenList)>::iterator it;

	funMap.insert(std::make_pair("autoindex", &Location::setAutoindex));
	funMap.insert(std::make_pair("root", &Location::setRoot));
	funMap.insert(std::make_pair("alias", &Location::setAlias));
	funMap.insert(std::make_pair("method", &Location::setMethod));

	itList++;

	locationMask = *itList;
	//std::cout << "*itList = " << *itList << std::endl;
	if (locationMask.empty() ||  locationMask == "{" \
		|| locationMask == "}" || locationMask == ";")
		throw "location there is not mask location";
	itList++;
	//std::cout << "*itList = " << *itList << std::endl;
	if ((*itList).empty() || *itList != "{")
		throw "location there is not {";
	itList++;
	while (itList != tokenList.end() && *itList != "{" && *itList != "}")
	{
		it = funMap.find(*itList);
		//std::cout <<"hello	" << *itList << std::endl;
		//std::cout <<"*itList = 	" << *itList << std::endl;
		if (it != funMap.end())
			(this->*(it->second))(itList, tokenList);
		else
			throw "unknown directive";
		itList++;
	}
	if ((*itList).empty() || *itList != "}")
		throw "location there is not }";
}

void	Location::setAutoindex(list<string>::iterator &itList, list<std::string> tokenList)
{
	//itList != tokenList.end()
	itList++;

	if ((*itList).compare(0, 3, "on") != 0 && (*itList).compare(0, 4, "off"))
		throw "something wrong after autoindex";
	autoindex = *itList == "on" ? 1 : 0;
	itList++;
	if ((*itList).compare(0, 2, ";") != 0)
		throw "there is not ; after autoindex";
}

void	Location::setRoot(list<string>::iterator &itList, list<std::string> tokenList)
{
	//itList != tokenList.end()
	itList++;

	if ((*itList).empty() || (*itList).compare(0, 2, ";") == 0 || (*itList).compare(0, 2, "}") == 0 \
	|| (*itList).compare(0, 2, "{") == 0)
		throw "something wrong after root";
	root = *itList;
	itList++;
	if ((*itList).compare(0, 2, ";") != 0)
		throw "there is not ; after root";

	//check root if is directory
}



void	Location::setAlias(list<string>::iterator &itList, list<std::string> tokenList)
{
	//itList != tokenList.end()
	itList++;

	if ((*itList).empty() || (*itList).compare(0, 2, ";") == 0 || (*itList).compare(0, 2, "}") == 0 \
	|| (*itList).compare(0, 2, "{") == 0)
		throw "something wrong after alias";
	alias = *itList;
	itList++;
	if ((*itList).compare(0, 2, ";") != 0)
		throw "there is not ; after alias";


	//check root if is directory
}

void	Location::setMethod(list<string>::iterator &itList, list<std::string> tokenList)
{
	map<string, int>::iterator it;
	itList++;

	if (itList == tokenList.end() || (*itList).empty() || \
		(*itList).compare(0, 2, ";") == 0 || (*itList).compare(0, 2, "}") == 0 \
		|| (*itList).compare(0, 2, "{") == 0)
		throw "something wrong after method";
	it = accepted_methods.find(*itList);
	if (it == accepted_methods.end())
		throw "there are not this method";
	it->second = 1;
	itList++;
	if ((*itList).compare(0, 2, ";") != 0)
		throw "there is not ; after method";
}

}