/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: margo <margo@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/17 22:51:57 by margo             #+#    #+#             */
/*   Updated: 2025/07/17 22:52:21 by margo            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <fstream>


int main(int argc, char **argv)
{
	try {
		std::cout << "Wello horld!" << std::endl;

		std::ifstream file(argv[1]);
		if (!file.is_open())
			std::cerr << "Error: File doesn't exist or can't be opened." << std::endl;
		else
		{
			std::string line;
			while(std::getline(file, line))
				std::cout << line << std::endl;
		}
		file.close();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

    return 0;
	(void)argc;
}
