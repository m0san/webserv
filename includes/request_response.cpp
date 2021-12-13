/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestResponse.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mbani <mbani@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/11/07 13:00:35 by mbani             #+#    #+#             */
/*   Updated: 2021/11/23 09:05:22 by mbani            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "requestresponse.hpp"

RequestResponse::RequestResponse():max_fd(-1)
{
	FD_ZERO(&(this->read_fd));
	FD_ZERO(&(this->write_fd));
	FD_ZERO(&(this->tmp_read));
	FD_ZERO(&(this->tmp_write));
}

void		RequestResponse::reset(int fd)
{
	(req_fd[fd]).resetRequest();
}

bool		RequestResponse::receive(int fd, Server &server) // return false if connection is closed 
{
	long long status;
	char buffer[BUFFER_SIZE];

	bzero(buffer, BUFFER_SIZE);
	status = recv(fd, buffer, BUFFER_SIZE, 0);
	if (status == 0 || status == -1) // Closed connection or Invalid fd
	{
		remove_fd(fd, true, true); // remove from read set
		server.socketFree(fd);
		return false;
	}
	req_fd[fd].append(buffer, status);
	return true;
}

void	RequestResponse::send_all(int fd, std::string res)
{
	size_t total = 0;
	size_t left = res.length();
	int sent = 0;

	// while(total < res.length())
	// {
		sent = send(fd, res.c_str(), left, 0);
		if (sent == 0) // connection closed
		{
			// remove_fd(fd, true, true); // remove from read set
		}
		// if (sent < 0)
		// 	break;
		// total += sent;
		// left -= sent;
	// }
}

bool	RequestResponse::req_completed(int fd)
{
	return (req_fd[fd]).is_completed();
}

const std::stringstream&		RequestResponse::get_req(int fd)
{
	return this->req_fd[fd].get_req();
}

int 	RequestResponse::is_ready(int fd, bool to_read)
{
	if (to_read)
		return FD_ISSET(fd, &(this->tmp_read));
	else
		return FD_ISSET(fd, &(this->tmp_write));
}

void	RequestResponse::update_set()
{
	this->tmp_read = this->read_fd;
	this->tmp_write = this->write_fd;
}

void RequestResponse::set_fd(int fd, bool to_read, bool is_client) // set fd to read or write sets (1 for read 0 for write) && update max_fd
{
	if (to_read)
	{
		FD_SET(fd, &read_fd);
		if (is_client)
			req_fd.insert(std::make_pair(fd, Request()));
	}
	else 
		FD_SET(fd, &write_fd);
	if (fd > max_fd) // update max_fd
			max_fd = fd;
}

void RequestResponse::remove_fd(int fd, bool to_read, bool is_client, bool _close)
{
	if (to_read)
	{
		FD_CLR(fd, &read_fd);
		if (is_client && _close)
		{
			req_fd.erase(fd);
		}
	}
	else
		FD_CLR(fd, &write_fd);
	
}

bool RequestResponse::select_fd()
{
	if (select(max_fd + 1, &(this->tmp_read), &(this->tmp_write), NULL, NULL) < 0)
	{
		// std::cout << "Select Failed !" << std::endl;
		// perror("select");
		return false;
	}
	return true;
}

int RequestResponse::get_maxfd()const
{
	return this->max_fd;
}

void RequestResponse::close_connection(int fd) const
{
	shutdown(fd, 2); // Further sends and receives are disallowed
	close(fd);
}

std::map<int , Request>& RequestResponse::getMap()
{
	return this->req_fd;
}

RequestResponse::~RequestResponse()
{}