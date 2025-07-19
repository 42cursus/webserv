/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/18 21:41:03 by abelov            #+#    #+#             */
/*   Updated: 2025/07/18 21:41:03 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>
#include <sstream>
#include "HttpRequest.hpp"

HttpRequest::HttpRequest(const std::string &path) : path(path)
{}

void HttpRequest::parseRequest(const std::string &rawRequest)
{

	int currindex = 0;

	while (currindex < rawRequest.length())
	{
		if (rawRequest[currindex] == ' ')
		{
			break;
		}
		method += rawRequest[currindex];
		currindex++;
	}

	headers["method"] = method;

	currindex++;
	while (currindex < rawRequest.length())
	{
		if (rawRequest[currindex] == ' ')
		{
			break;
		}
		path += rawRequest[currindex];
		currindex++;
	}

	headers["path"] = path;
}

std::string HttpRequest::readHtmlFile(const std::string &path)
{
	int flag = 0;
	std::string filename = path.substr(1, path.length());

	std::string str = "../" + filename;
	std::ifstream file(str.c_str());

	if (!file) {
		flag = 1;
		std::cerr << "File not found." << std::endl;
	}

	if (flag == 0) {
		std::stringstream buffer;
		buffer << file.rdbuf();
		return buffer.str();
	}

	return "";
}

std::string HttpRequest::getMimeType(const std::string &path)
{
	std::map<std::string, std::string> mimeTypes;

	mimeTypes.insert(std::make_pair("html", "text/html"));
	mimeTypes.insert(std::make_pair("css", "text/css"));
	mimeTypes.insert(std::make_pair("js", "text/javascript"));
	mimeTypes.insert(std::make_pair("jpg", "image/jpeg"));
	mimeTypes.insert(std::make_pair("png", "image/png"));

	std::string fileExtension = path.substr(path.find_last_of(".") + 1);

	return mimeTypes[fileExtension];
}

HttpRequest::HttpRequest()
{

}
