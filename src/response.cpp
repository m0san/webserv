#include "../includes/response.hpp"
#include "../includes/location.hpp"


Response::Response(void): _response(""), _loc()
, _root(""), _uri(""), _error_pages(""){}

Response::Response(std::string const& root, Location const& loc
, std::string const& uri, std::string const& error_pages): _loc(loc), _root(root)
, _uri(uri), _error_pages(error_pages)
{
	_type.insert(std::make_pair("json", "application"));
	_type.insert(std::make_pair("html", "text"));
}

Response::Response(Response const& x) { *this = x;	}
Response::~Response(void) { _file.close(); }
Response& Response::operator=(Response const& x)
{
	_response = x._response;
	return *this;
}

void Response::Get_request(void)
{
	std::vector<std::string> allowed = _loc.getAllowedMethods();

	//lets first check for alowed methods in this location
	if (find(allowed.begin(), allowed.end(), "GET") == allowed.end())
	{
		_fill_response(_error_pages + '/' + "403.html", 403, "Forbiden");
		return;
	}
	// first we have to check if the location is a dir or just a file
	if (_loc.getPath() == "/")
		_process_as_dir();
	else if (_is_dir(_root + '/' + _loc.getPath()))
		_process_as_dir();
	else	
		_process_as_file();
}

void Response::_process_as_dir(void)
{
	std::vector<std::string> const	index = _loc.getIndex();
	bool							found(false);
	
	_root += '/' + _uri;
	if (_uri.empty() || _is_dir(_root))
	{
		for (size_t i = 0; i < index.size(); ++i)
		{
			_file_path = _root + '/' + index[i];
			if (!(found = _file_is_good(false)) && errno != 2) // if the file exists but we don't have the permissions to read from it
			{
				_fill_response(_error_pages + '/' + "403.html", 403, "Forbiden");
				return;
			}
			else if (found)
			{
				if (!_file_is_good(true))
					return;
				_fill_response(_file_path, 200, "OK");
				break;
			}
		}
		if (!found && _loc.getAutoIndex() != "on")
		{
			_fill_response(_error_pages + '/' + "404.html", 404, "Not found");
			return;
		}	
	}
	else
	{
		_file_path = _root;
		if (!_file_is_good(true))
			return;
		_fill_response(_file_path, 200, "OK");
		return;
	}
	// if we are here than we didn't found the file we are seaching on, and we have a intoindex set to on, so we should fill the template for 
	// autoindex on to list all the files in the dir
}

void Response::_process_as_file(void)
{
	_file_path = _root + '/' + _uri;
	if (!_file_is_good(true))
		return;
	_fill_response(_file_path, 200, "OK");
	return;
}

bool Response::_is_dir(std::string const& path) const
{
	struct stat s;

	if (!lstat(path.c_str(), &s))
	{
		if (S_ISDIR(s.st_mode))
			return true;
		else
			return false;
	}
	return false;
}

void Response::_set_headers(size_t status_code, std::string const& message, size_t content_length, std::string const& path)
{
	time_t rawtime;

	time (&rawtime);
	_response += "HTTP/1.1 " +  std::to_string(status_code) + " " + message + '\n';
	_response += "Date: " + std::string(ctime(&rawtime));
	_response += "Server: webserver\n";
	_response += "Content-Length: " + std::to_string(content_length) + '\n';
	_response += "Content-Type: " + _type[path.substr(path.find_last_of(".") + 1)] + '/'
	+ path.substr(path.find_last_of(".") + 1) + '\n';
	_response += "Connection: close\n";
	_response += '\n';
}

void Response::_fill_response(std::string const& path, size_t status_code, std::string const& message)
{
	std::string 	line;
	std::string		tmp_resp;
	size_t			content_counter(0);
	
	_file.open(path);
	while (!_file.eof())
	{
		std::getline(_file, line);
		content_counter += line.size();
		if (!_file.eof())
		{
			line += '\n';
			content_counter++;
		}
		tmp_resp += line;
	}
	// set all the needed response header
	_set_headers(status_code, message, content_counter, path);
	_response += tmp_resp;
}

bool Response::_file_is_good(bool fill_resp)
{
	if (_file_path.empty())
		_file_path = _root + '/' + _uri;
	if (open(_file_path.c_str(), O_RDONLY) < 0)
	{
		if (errno == 2 && fill_resp)
			_fill_response(_error_pages + '/' + "404.html", 404, "Not Found");
		else if (fill_resp)
			_fill_response(_error_pages + '/' + "403.html", 403, "Forbidden");
		return false;
	}
	return true;
}
std::string const& 	Response::get_response(void) const	{ return _response; }