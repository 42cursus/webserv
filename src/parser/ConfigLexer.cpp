/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLexer.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelov <abelov@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/16 17:00:54 by abelov            #+#    #+#             */
/*   Updated: 2026/03/16 17:00:54 by abelov           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLexer.hpp"
#include <climits>
#include <unistd.h>

std::string ConfigLexer::getExecutablePath()
{
	char	buf[PATH_MAX] = {0x00};
	ssize_t len			  = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);

	if (len <= 0)
		throw std::runtime_error("readlink(/proc/self/exe) failed");

	buf[len] = '\0';
	return std::string(buf);
}

std::string ConfigLexer::getDirname(const std::string &path)
{
	std::string::size_type pos = path.find_last_of('/');

	if (pos == std::string::npos)
		return ".";
	if (pos == 0)
		return "/";
	return path.substr(0, pos);
}

std::string ConfigLexer::joinPath(const std::string &a, const std::string &b)
{
	std::string ret;

	if (a.empty() || (!b.empty() && b[0] == '/'))
		ret = b;
	else if (a[a.size() - 1] == '/')
		ret = a + b;
	else
		ret = a + "/" + b;
	return ret;
}

t_token ConfigLexer::makeToken(e_token key, std::string word, unsigned int linecount)
{
	t_token token;
	token.type = key;
	token.literal = word;
	token.line = linecount;
	return token;
}

bool ConfigLexer::isKnownKey(const std::vector<std::string> &db, const std::string &key)
{
	return std::find(db.begin(), db.end(), key) != db.end();
}

void ConfigLexer::readLogFormatString(std::ifstream &file, std::istringstream &iss, std::string &word,
						 std::vector<t_token> &tokens, unsigned int current_line)
{
	tokens.push_back(makeToken(KEY, word, current_line));
	iss >> word;
	tokens.push_back(makeToken(EQUAL, word, current_line));
	std::string token_literal;
	std::string line;

	while (iss >> word) {
		token_literal += word;
		token_literal += " ";
	}
	while (std::getline(file, line)) {
		if (line == ";") {
			break;
		}
		if (!line.empty() && line[line.length() - 1] == ';') {
			line = line.substr(0, line.length() - 1);
			token_literal += line;
			break;
		}
		token_literal += line;
	}
	tokens.push_back(makeToken(KEY, token_literal, current_line));
	tokens.push_back(makeToken(SEMICOLON, ";", current_line));
}

void ConfigLexer::loadKeyDatabase(std::vector<std::string> &key_database)
{
	std::string line;

	std::string executablePath = getExecutablePath();
	std::string currentDir	   = getDirname(executablePath);
	std::string keywordsPath   = joinPath(currentDir, "resources/keywords.txt");

	std::fstream fin;
	fin.open(keywordsPath.c_str(), std::ios::in);
	if (!fin.is_open())
		throw std::runtime_error("Cannot open keywords file: " + keywordsPath);

	while (std::getline(fin, line)) {
		std::stringstream iss(line);
		std::string		  key;

		iss >> key;
		key_database.push_back(key);
	}
	fin.close();
}

void ConfigLexer::tokenizeFile(const std::string &config_root,
							   const std::vector<std::string> &key_database,
							   std::vector<t_token> &tokens,
							   unsigned int &current_line)
{
	std::string	  line;
	std::ifstream file(config_root.c_str());

	if (!file.is_open())
		throw Parser::Error("Error: file doesn't exit/can't open file");

	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string		   word;

		current_line++;
		if (line.empty())
			continue;

		while (iss >> word) {
			if (word[0] == '#')
				break;
			else if (isKnownKey(key_database, word)) {
				if (word == "log_format")
					readLogFormatString(file, iss, word, tokens, current_line);
				else
					tokens.push_back(makeToken(KEY, word, current_line));
			} else if (word == "{")
				tokens.push_back(makeToken(BLOCK_START, word, current_line));
			else if (word == "}")
				tokens.push_back(makeToken(BLOCK_END, word, current_line));
			else if (word == "=")
				tokens.push_back(makeToken(EQUAL, word, current_line));
			else if (word.find('/') != std::string::npos) {
				if (word[word.length() - 1] == ';') {
					tokens.push_back(makeToken(REGEX, word.substr(0, word.length() - 1), current_line));
					tokens.push_back(makeToken(SEMICOLON, ";", current_line));
				} else
					tokens.push_back(makeToken(REGEX, word, current_line));
			} else if (word == ";" || word[word.length() - 1] == ';') {
				if (word != ";")
					tokens.push_back(makeToken(KEY, word.substr(0, word.length() - 1), current_line));
				tokens.push_back(makeToken(SEMICOLON, ";", current_line));
			} else
				tokens.push_back(makeToken(ILLEGAL, word, current_line));
		}
	}
	file.close();
}
