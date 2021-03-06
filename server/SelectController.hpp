/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SelectController.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nelisabe <nelisabe@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/07/21 19:37:23 by nelisabe          #+#    #+#             */
/*   Updated: 2021/07/26 11:42:50 by nelisabe         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SELECTCTRL_HPP
# define SELECTCTRL_HPP

# include "IIOController.hpp"
# include "Errors.hpp"
# include <sys/select.h>
# include <sys/time.h>
# include <unistd.h>

class SelectController : public IIOController
{
	public:
		SelectController();
		virtual ~SelectController();

		bool	Wait(size_t timeout_seconds);
		void	AddFDToWatch(int fd, IOMode mode);
		bool	CheckIfFDReady(int fd, IOMode mode);
		void	Clear(void);
	private:
		SelectController(const SelectController &);
		
		SelectController	&operator=(const SelectController &);

		fd_set	_read_set;
		fd_set	_write_set;
		int		_max_fd;

		timeval	_timeout;
};

#endif
