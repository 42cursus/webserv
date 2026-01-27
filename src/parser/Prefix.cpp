/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Prefix.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fsmyth <fsmyth@student.42london.com>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/23 12:53:33 by fsmyth            #+#    #+#             */
/*   Updated: 2026/01/23 15:25:20 by fsmyth           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Prefix.hpp"
#include "State.hpp"
#include "webserv.hpp"
#include <cstddef>

TrieNode::TrieNode(void) : location(NULL), children() {}

void	loc_trie_insert(TrieNode *head, Location *location)
{
	TrieNode		*current = head;
	std::string&	path = location->_path;

	for (uint64_t i = 0; i < path.length(); i++)
	{
		size_t idx = static_cast<size_t>(path[i]);

		if (current->children[idx] == NULL)
			current->children[idx] = new TrieNode();
		current = current->children[idx];
	}
	current->location = location;
}

Location	*loc_trie_search(TrieNode *head, std::string& path)
{
	TrieNode						*current = head;
	Location	*last_loc = NULL;

	for (uint64_t i = 0; i < path.length() && current != NULL; i++)
	{
		size_t idx = static_cast<size_t>(path[i]);

		if (current->location != NULL)
			last_loc = current->location;
		current = current->children[idx];
	}

	return last_loc;
}

void	free_loc_trie(TrieNode *node)
{
	if (node == NULL)
		return ;

	for (uint64_t i = 0; i < 128; i++)
	{
		free_loc_trie(node->children[i]);
	}

	delete node;
}

void	test_trie_match(TrieNode *head, std::string path)
{
	Location	*match;

	std::cout << "Searching for '" << path << "'" << std::endl;
	match = loc_trie_search(head, path);
	std::cout << "Matches to: '" << match->_root << "'" << std::endl << std::endl;
}

// void test_trie(void)
// {
// 	TrieNode	*head = new TrieNode();
// 	Config::Http::Server::Location root = {
// 		.path = "/",
// 		.config = {
// 			.root = "/usr/web/",
// 		},
// 	};
// 	Config::Http::Server::Location loc1 = {
// 		.path = "/dir/",
// 		.config = {
// 			.root = "/usr/web/resources/",
// 		},
// 	};
// 	Config::Http::Server::Location loc2 = {
// 		.path = "/storage/",
// 		.config = {
// 			.root = "/usr/storage/",
// 		},
// 	};
//
// 	loc_trie_insert(head, &root);
// 	loc_trie_insert(head, &loc1);
// 	loc_trie_insert(head, &loc2);
//
// 	test_trie_match(head, "/hello");
// 	test_trie_match(head, "/di");
// 	test_trie_match(head, "/dir/weeee");
// 	test_trie_match(head, "/storage/waaaaa");
//
// 	free_loc_trie(head);
// }
