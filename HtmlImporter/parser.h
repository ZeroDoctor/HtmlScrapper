#pragma once

#include "dom.h"

// parses an html file into potential start, end, or single tags
bool parse_html(std::vector<std::string>& result, std::string str, char delim, char excpt)
{
	result.clear();
	std::size_t starttag = str.find('<');

	if ((str[starttag + 1] == ' ' || starttag == std::string::npos) && str.size() > 2) {
		result.push_back(str);
		return false;
	}


	int excpt_found = 0;
	int prev = 0;

	for (std::size_t i = 0; i < str.size(); i++) {

		if (str[i] == excpt) excpt_found++;

		if (str[i] == delim && excpt_found % 2 == 0) {
			std::string test = str.substr(prev, (i - prev) + 1);
			result.push_back(test); // we want that '>' back
			prev = i + 1;
			excpt_found = 0;
		}
	}

	return true;
}

// checks if we are at the end of a tag
bool valid_end(std::string potential_end)
{
	int found = 0;
	std::size_t end_pos = 0;
	std::size_t quote_pos = 0;

	for (std::size_t i = 0; i < potential_end.size(); i++)
	{
		if (potential_end[i] == '\"')
		{
			found++;
			quote_pos = i;
		}

		if (potential_end[i] == '>') end_pos = i;
	}

	if (found % 2 == 0 && end_pos > quote_pos) return true;
	return false;
}

// parses tag name, attributes, or values
bool parse_tag_str(std::size_t& i, std::string& name, std::string tag, char* delim, int length)
{
	while (i < tag.size()) // the file may not be in html format if i < tag.size() is false
	{
		for (int j = 0; j < length; j++)
			if (tag[i] == delim[j]) return true;

		name += tag[i];
		i++;
	}

	return true;
}

// parses a tag
genericTag parse_tag(std::string tag)
{
	bool found_tag = 0;
	bool is_start = false;
	bool found_name = false;

	struct genericTag generic_tag;

	std::string name = "";
	char end_delim[] = ">";
	char start_delim[] = " >";
	char attr_delim[] = "=/>";
	char value_delim[] = "\"";

	for (std::size_t i = 0; i < tag.size(); i++)
	{

		if (tag[i] == '>') found_tag = false; // for the \t\t  <div> </div>

		if (found_tag) // start reading tag
		{
			if (!is_start && tag[i] == '/') // start reading end tag
			{
				name = "";
				generic_tag.content += tag.substr(0, i - 1); // minus the </

				parse_tag_str(i, name, tag, end_delim, 1); // get end tag name
				generic_tag.name = name;
			}
			else // start reading start tag
			{
				if (tag[i] == ' ') continue; // for <div   class = ""    value=""  >
				if (!found_name)
				{
					is_start = true;
					generic_tag.is_start = is_start;

					std::string start_name = "";
					found_name = parse_tag_str(i, start_name, tag, start_delim, 2); // get start tag name
					generic_tag.name = start_name;
					i++;
				}

				name = "";
				struct attribute attr;
				parse_tag_str(i, name, tag, attr_delim, 1); // get attribute 

				if (valid_end(name)) // checking if we reached the end of attribute
				{
					if (name == "/>") generic_tag.is_single = true;
					continue; 
				}

				attr.name = name;
				name = "";

				if (i < tag.size() && tag[i] == '=')
				{
					i = tag.find('\"', i) + 1;
					parse_tag_str(i, name, tag, value_delim, 1); // get value

					attr.value = name;
					generic_tag.attrs.push_back(attr);

				}
			}
		}

		if (i < tag.size() && tag[i] == '<') found_tag = true;
	}

	return generic_tag;
}

// creates dom tree

/**
	Limitation:
		- every void element needs the '/' character ex. <img />, <meta />, etc.
		- will not work if '>' is found on the next line. ex:
			<div
			class="something"
			id="anotherthing"
			>

	TODO:
		- create a list of void elements
		- check if potential tags match a void element
		- continue until '>' is found even on the next line or if another '<' is occurs
*/
Dom* create_dom(std::vector<std::string> lines)
{
	std::vector<std::string> parse_line;

	bool is_tag;
	std::string content = "";

	std::stack<Dom*> dom;
	Dom* current_dom = NULL;
	Dom* last_dom = NULL;
	Dom* result = NULL;

	dom.push(NULL);

	for (std::size_t i = 0; i < lines.size(); i++) {
		is_tag = parse_html(parse_line, lines[i], '>', '"');
		for (auto j : parse_line)
		{
			if (is_tag) {
				genericTag tag = parse_tag(j);
				if (tag.name[0] == '!') continue; // could have an array or set of character / words to ignore
				last_dom = dom.top();

				if (tag.is_start)
				{
					//std::cout << "start: " << tag.name << std::endl;
					current_dom = new Dom(tag, last_dom);
					current_dom->start_linenum = i;
					if (last_dom != NULL)
						last_dom->add_child(current_dom);
					if (!tag.is_single)
						dom.push(current_dom);
					else
						result = current_dom;
				}
				else if (last_dom != NULL && ("/" + last_dom->get_name()) == tag.name)
				{
					//std::cout << "end: " << tag.name << std::endl;
					last_dom->end_linenum = i;
					tag.content += content;
					content = "";
					result = dom.top();
					dom.pop();
				}
			}
			else
			{
				content += j;
			}
		}
	}

	return result;
}
