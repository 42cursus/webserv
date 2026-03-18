/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigCompiler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:00:47 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:00:47 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */



#ifndef CONFIGCOMPILER_HPP
#define CONFIGCOMPILER_HPP

#include "State.hpp"
#include "webserv.hpp"

typedef const std::map<std::string, std::string> ConfigMap;
class ConfigCompiler {
public:
	static void compileServer(Server &server, struct Config &cfg);
	static TrieNode   *build_cgi_trie(std::vector<CGI> &cgis);
	static void		overwrite_error_pages(Config &cfg, const ConfigMap &error_pages);
};

#endif//CONFIGCOMPILER_HPP
