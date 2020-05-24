#pragma once
#include<cstdio>
struct xml_data
{
	char *buf;
	char *version;
	char *encoding;
	private:
		void parse_data();
	public:
	struct xml_element;
	struct xml_element_base
	{
		unsigned elem_count=0;
		xml_element *elements;
	protected:
		void parse_el(xml_element *&cel, char *&cur);
		friend void xml_data::parse_data();
	}tree;
	struct xml_element : xml_element_base
	{
		void *custom_struct;
		xml_element *parent;
		xml_element *next_elem;
		char *name;
		unsigned param_count=0;
		struct xml_param
		{
			xml_param *next_param;
			char *name;
			char *val;
		}*params;
		void write_xml(FILE *f, bool spaced, unsigned tT);
	private:
		void parse_attr(char *&cur);
		void parse_elend(char *&cur);
		friend void xml_element_base::parse_el(xml_element *&, char *&);
	};
	xml_data(FILE *f);
	~xml_data();
	/*saves the XML document at the specified path. Spaced:adds newlines, tabs, etc.*/
	int save_data(const char *path, bool spaced);
};

typedef void (*processElem_t)(xml_data::xml_element *xe);
