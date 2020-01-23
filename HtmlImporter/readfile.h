#pragma once
#include <Windows.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <string>

#include "dirent.h"


class FileReader {

private:
	


public:

	std::string getCurrentDir(const char* path)
	{
		std::string result = path;

		if (path[0] != 'C')
		{
			char buffer[MAX_PATH];
			GetModuleFileNameA(NULL, buffer, MAX_PATH);
			std::string::size_type pos = std::string(buffer).find_last_of("\\//");
			result = std::string((char*)buffer).substr(0, pos) + path;
		}

		return result;
	}


	bool getAllFiles(std::string* out_str, const char* path)
	{
		std::string newPath = getCurrentDir(path);

		DIR *dir;
		struct dirent *ent;
		int i = 0;

		if ((dir = opendir(newPath.c_str())) != NULL)
		{
			while ((ent = readdir(dir)) != NULL)
			{
				out_str[i] = std::string(ent->d_name);
				i++;
			}

			closedir(dir);
			return true;
		}
		else {
			perror("readfile : could not open directory");
			return false;
		}
	}


	bool readFile(std::vector<std::string>* out_str, std::string file)
	{
		std::string line;
		std::ifstream infile;
		infile.open(file);

		if (!infile) return false;
		
		while (std::getline(infile, line))
		{
			out_str->push_back(line);
		}

		infile.close();

		return true;
	}

};