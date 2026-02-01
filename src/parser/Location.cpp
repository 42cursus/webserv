/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/31 23:56:59 by abelov            #+#    #+#             */
/*   Updated: 2026/01/31 23:56:59 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"
#include "Prefix_suffix.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ostream>

Location::Location(const Location &copy) :
    IBlock(copy),
    _root(copy._root),
    _path(copy._path),
    _index(copy._index),
    _methods(copy._methods),
    _max_body_size(copy._max_body_size),
    _autoindex(copy._autoindex),
    _cgi(copy._cgi),
    cgi_trie(NULL)
{
}

Location::Location() :
    IBlock(LOCATION_),
    _autoindex(false),
    cgi_trie(NULL)
{
    setInBlock(false);
}

Location::~Location()
{
    free_trie(cgi_trie);
}

Location&   Location::operator=(const Location& copy)
{
    if (this != &copy)
    {
        IBlock::operator=(copy);
        _root = copy._root;
        _path = copy._path;
        _index.clear();
        for (size_t i = 0; i < copy._index.size(); i++)
            _index.push_back(copy._index[i]);
        _methods.clear();
        for (size_t i = 0; i < copy._methods.size(); i++)
            _methods.push_back(copy._methods[i]);
        _max_body_size = copy._max_body_size;
        _autoindex = copy._autoindex;
        _cgi.clear();
        for (size_t i = 0; i < copy._cgi.size(); i++)
            _cgi.push_back(copy._cgi[i]);
    }
    return *this;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
    os << "[";

    std::size_t i = 0;
    while (i < vec.size()) {
        os << vec[i];
        if (i + 1 < vec.size()) {
            os << ", ";
        }
        ++i;
    }

    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const sockaddr_in& a)
{
    char ip[INET_ADDRSTRLEN] = {0};

    const char* ok = inet_ntop(AF_INET, &a.sin_addr, ip, sizeof(ip));
    os << "sockaddr_in{family=" << a.sin_family;

    if (ok) os << ", addr=" << ip;
    else    os << ", addr=<invalid>";

    os << ", port=" << ntohs(a.sin_port) << "}";
    return os;
}



std::ostream &operator<<(std::ostream &os, const Location &location) {
    os << "{"
       << " _root: " << location._root
       << " _path: " << location._path
       << " _index: " << location._index
       << " _methods: " << location._methods
       << " _max_body_size: " << location._max_body_size
       << " _autoindex: " << location._autoindex
       << " _cgi: " << location._cgi
       << " cgi_trie: " << location.cgi_trie
       << std::endl;
    return os;
}

__attribute__((used))
void debug_print_tcp_sockaddr_in(const sockaddr_in& x) { std::cerr << x << "\n"; }

__attribute__((used))
void debug_print_location(const Location& x) { std::cerr << x << "\n"; }
