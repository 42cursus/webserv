/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLexer.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:00:58 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:00:58 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGLEXER_HPP
#define CONFIGLEXER_HPP

#include "Parser.hpp"

class ConfigLexer {
public:
	static void loadKeyDatabase(std::vector<std::string> &key_database);
	static void tokenizeFile(const std::string &config_root,
							 const std::vector<std::string> &key_database,
							 std::vector<t_token> &tokens,
							 unsigned int &current_line);
	static std::string getExecutablePath();
	static std::string getDirname(const std::string &path);
	static std::string joinPath(const std::string &a, const std::string &b);
	static t_token		makeToken(e_token key, std::string word, unsigned int linecount);
	static bool		isKnownKey(const std::vector<std::string> &db, const std::string &key);
	static void		readLogFormatString(std::ifstream		 &file,
									std::istringstream	 &iss,
									std::string			 &word,
									std::vector<t_token> &tokens,
									unsigned int		  current_line);
};

#endif//CONFIGLEXER_HPP
