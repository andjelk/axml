#include<cstdio>
#include"xml_d.hpp"
using namespace std;

void newl(FILE *f, unsigned tT)
{
	fputc('\n', f);
	for(unsigned i=0;i<tT;i++)fputc('\t', f);
}
void xml_data::xml_element::write_xml(FILE *f, bool spaced, unsigned tT)
{
	if(spaced)newl(f, tT);
	if(name[0]=='\0')
		fputs(name+1, f);
	else {
		fprintf(f, "<%s", name);
		if(param_count>0) {
			xml_param *cur = params;
			do {
				fprintf(f, " %s=\"%s\"", cur->name, cur->val);
			} while((cur=cur->next_param)!=nullptr);
		}
		if (elem_count > 0) {
			fputc('>', f);
			xml_element *cur = elements;
			do {
				cur->write_xml(f, spaced, tT+1);
			} while((cur=cur->next_elem)!=nullptr);
			if(spaced)newl(f, tT);
			fprintf(f, "</%s", name);
		} else fputc('/', f);
		fputc('>', f);
	}
}
int xml_data::save_data(const char *path, bool spaced)
{
	FILE *f = fopen(path, "wb");
	if(!f)return 1;
	fputs("<?xml", f);
	if(version!=nullptr) fprintf(f, " version=\"%s\"", version);
	if(encoding!=nullptr) fprintf(f, " encoding=\"%s\"", encoding);
	fputs("?>", f);
	if(tree.elem_count==0) {if(spaced)fputs("<!-- Empty document -->", f);}
	else {
		xml_element *cur = tree.elements;
		do {
			cur->write_xml(f, spaced, 0);
		} while((cur=cur->next_elem)!=nullptr);
	}
	fclose(f);
	return 0;
}

extern "C" {
_declspec(dllexport) int saveXml(xml_data* xd, const char *path, bool spaced)
{
	return xd->save_data(path, spaced);
}
}
