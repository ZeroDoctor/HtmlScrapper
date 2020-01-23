// HtmlImporter.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <array>
#include <stack>

#include "inireader.h"
#include "readfile.h"
#include "threadpool.h"
#include "prettyconsole.h"
#include "parser.h"

using namespace prettycon;


struct Rules {
	bool path = false;
	bool name = false;
	
	const char* list[2] = { "path", "name" }; // if it gets to big just sort it and go from there
};

Dom* parse_create_dom(std::string path) 
{

	std::vector<std::string> lines;
	FileReader fr;
	std::cout << "reading file: " << path << "\n\n";
	bool opened = fr.readFile(&lines, fr.getCurrentDir(path.c_str()));
	return create_dom(lines);
}

int readini() 
{
	FileReader fr;
	CSimpleIniA ini;
	ini.SetUnicode();
	SI_Error rc = ini.LoadFile(fr.getCurrentDir("\\web\\rules.ini").c_str());
	if (rc < SI_Error::SI_OK)
	{
		std::cout << "could not load ini file...\n";
		return -1;
	}

	CSimpleIniA::TNamesDepend sections;
	CSimpleIniA::TNamesDepend keys;
	ini.GetAllSections(sections);
	
	Rules* rules = new Rules;

	for (auto sec : sections)
	{
		std::cout << sec.pItem << std::endl;
		ini.GetAllKeys(sec.pItem, keys);
		for (auto k : keys)
		{

			for (auto r : rules->list)
			{
				if (std::strcmp(r, k.pItem) == 0) {
					std::cout << "check" << std::endl;
				}
			}
			std::cout << "  " << k.pItem << "=";
			const char* value = ini.GetValue(sec.pItem, k.pItem, NULL);
			std::cout << value << std::endl;
		}
	}

	return 0;
}


int main()
{
	//if (readini() < 0) return -1;

	ThreadPool pool(4);

	pool.init();
	auto future = std::array<std::future<Dom*>, 5*3>();

	future[0] = pool.submit(parse_create_dom, "\\web\\index.html");
	future[1] = pool.submit(parse_create_dom, "\\web\\resume.html");

	ccout.setColor(Color::black, Color::lgreen, Attribute::mydefault);

	// index.html
	std::vector<Dom*> atag = future[0].get()->find_all("script");
	if (atag.size() > 0)
	{
		for (auto tag : atag)
		{
			std::vector<attribute> v_attr;
			tag->get_attribute(v_attr);
			for (auto attr : v_attr)
			{
				std::cout << attr.name << "=" << attr.value << std::endl;
			}
		}
	}

	std::cout << "----------------------" << std::endl;
	
	// resume.html
	std::vector<Dom*> btag = future[1].get()->find_all("div");
	if (btag.size() > 0)
	{
		for (auto tag : btag)
		{
			std::vector<attribute> v_attr;
			
			std::cout << "tag name: " << tag->get_name() << std::endl;
			tag->get_attribute(v_attr);
			for (auto attr : v_attr)
			{
				std::cout << "\t" << attr.name << "=" << attr.value << std::endl;
			}
		}
	}

	pool.shutdown();

	std::cout << "Done!" << std::endl;

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu
