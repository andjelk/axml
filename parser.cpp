#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<cctype>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include"xml_d.hpp"
using namespace std;
const char invl_decl[]="Invalid XML declaration";
const char invl_elem[]="Invalid element";
const char invl_elem_end[]="Invalid element end";
const char invl_doc_end[]="Unexpected end of document";
//Checks is specified character valid for names in XML
bool isnmvld(char c)
{
	return isalnum(c)||c=='_'||c=='-';
}
HANDLE heap;
void xml_data::xml_element::parse_attr(char *& cur)
{
	xml_param *cpar = (xml_param*)HeapAlloc(heap, 0, sizeof(xml_param));
	params = cpar;
	loop:
		cpar->name=cur;
		cur++;
		while(isnmvld(*cur))cur++;
		char *end=cur;
		if(isspace(*cur))cur++;
		if(*cur!='=')throw invl_elem;
		cur++;
		*end='\0';
		if(isspace(*cur))cur++;
		if(*cur!='"')throw invl_elem;
		cur++;
		if(isspace(*cur))cur++;
		cpar->val=cur;
		while(*cur!='"'&&isprint(*cur))cur++;
		char *vend=cur;
		if(isspace(*cur))cur++;
		if(*cur!='"')throw invl_elem;
		cur++;
		if(isspace(*cur))cur++;
		*vend='\0';
		param_count++;
		if(!isnmvld(*cur))goto loopend;
		cpar->next_param=(xml_param*)HeapAlloc(heap, 0, sizeof(xml_param));
		cpar=cpar->next_param;
	goto loop; loopend:
	cpar->next_param = nullptr;
}
processElem_t pe;
void xml_data::xml_element_base::parse_el(xml_data::xml_element *& cel, char *& cur)
{
	cel->name=cur;
	cur++;
	while(isnmvld(*cur))cur++;
	char *end=cur;
	if(isspace(*cur)) {
		cur++;
		if(isnmvld(*cur))cel->parse_attr(cur);
		else goto na;
	}
	else {
		na: cel->params=nullptr;
	}
	if (*cur == '/')
	{
		cur++;
		if(*cur == '>')
		{
			cur++;
			*end='\0';
			cel->elements=nullptr;
			if(pe!=nullptr)pe(cel);
		}
		else goto t;
	}
	else if (*cur == '>') {
		*end='\0';
		if(pe!=nullptr)pe(cel);
		cur++;
		if(isspace(*cur))cur++;
		bool el=*(cur++)=='<';
		if (el)
		{
			if(!isnmvld(*cur)) {
				if(*cur=='/')
				{
					cel->parse_elend(cur);
					cel->elements=nullptr;
					goto end;
				}
				else throw invl_elem;
			}
		}
		xml_element *celc = (xml_element*)HeapAlloc(heap, 0, sizeof(xml_element));
		cel->elements=celc;
		loop:
			celc->elem_count=0;
			celc->parent=cel;
			if (el) {
				cel->parse_el(celc, cur);
			}
			else {
				celc->name=cur-2;
				*(cur-2)='\0';
				while(*cur!='<') if(*(cur++)=='\0') throw invl_doc_end;
				*(cur++)='\0';
				el=true;
				celc->param_count=0;
				celc->params=nullptr;
				celc->elements=nullptr;
				if(pe!=nullptr)pe(celc);
				goto sk;
			}
			el=*(cur++)=='<';
			if (el) {
				sk:
				if(!isnmvld(*cur)) {
					if(*cur=='/')
					{
						cel->parse_elend(cur);
						celc->next_elem=nullptr;
						goto end;
					}
					else throw invl_elem;
				}
			}
			celc->next_elem=(xml_element*)HeapAlloc(heap, 0, sizeof(xml_element));
			celc = celc->next_elem;
		goto loop;
	}
	else {
	t:
		throw invl_elem;
	}
	end:
	if(isspace(*cur))cur++;
	elem_count++;
}
void xml_data::xml_element::parse_elend(char*& cur)
{
	cur++;
	const unsigned strl = strlen(name);
	if (!memcmp(name, cur, strlen(name)))
	{
		cur+=strl;
		if(isspace(*cur))cur++;
		if(*cur!='>') throw invl_elem_end;
		cur++;
		return;
	}
	else throw "Elements can not intersect (or bad element end).";
}
void xml_data::parse_data()
{
	char *cur=buf;
	//Check declaration
	{
		const char c[6]={'<', '?', 'x', 'm', 'l', ' '};
		version=nullptr;
		encoding=nullptr;
		if(!memcmp(cur, c, sizeof(c))) {
			cur+=sizeof(c);
			if(isspace(*cur))cur++;
			const char v[]={'v', 'e', 'r', 's', 'i', 'o', 'n'};
			const char e[]={'e', 'n', 'c', 'o', 'd', 'i', 'n', 'g'};
			const int sz[]={sizeof(v), sizeof(e)};
			const char * const t[]={(const char*)v, (const char*)e};
			char ** const b[]={&version, &encoding};
			for(unsigned i=0;i<2;i++)
			{
				if(!memcmp(cur, t[i], sz[i])) {
					cur+=sz[i];
					if(isspace(*cur))cur++;
					if(*cur!='=')throw invl_decl;
					cur++;
					if(isspace(*cur))cur++;
					if(*cur!='"')throw invl_decl;
					cur++;
					if(isspace(*cur))cur++;
					*b[i]=cur;
					while(*cur!='"'&&isgraph(*cur))cur++;
					char *vend=cur;
					if(isspace(*cur))cur++;
					if(*cur!='"')throw invl_decl;
					cur++;
					if(isspace(*cur))cur++;
					*vend='\0';
				}
				else {
					throw invl_decl;
				}
				if(cur[0]=='?'&&cur[1]=='>'){cur+=2;goto valid;}
			}
			throw invl_decl;
			valid:;
		}
		if(isspace(*cur))cur++;
	}
	//Parse elements
	if(*cur!='<')throw invl_elem;
	else cur++;
	xml_element *cel = (xml_element*)HeapAlloc(heap, 0, sizeof(xml_element));
	tree.elements=cel;
	loop:
		cel->elem_count=0;
		cel->parent=nullptr;
		tree.parse_el(cel, cur);
		if(*cur=='\0') {cel->next_elem=nullptr; goto loopend;}
		else if(*cur!='<') throw invl_elem;
		else 
		{
			cur++;
			if(!isnmvld(*cur)) {
				throw invl_elem;
			}
			cel->next_elem = (xml_element*)HeapAlloc(heap, 0, sizeof(xml_element));
			cel = cel->next_elem;
		}
	goto loop; loopend: return;
}
xml_data::xml_data(FILE *f)
{
	fseek(f, 0, SEEK_END);
	const int len = ftell(f);
	fseek(f, 0, SEEK_SET);
	unsigned char *const temp_buf = (unsigned char *const)HeapAlloc(heap, 0, len);
	if(fread(temp_buf, len, 1, f)!=1)
	throw "Could not read XML data.";
	buf = (char *const)HeapAlloc(heap, 0, len+1);
	{
		unsigned j = 0;
		bool flag=false;
		for (int i = 0;;i++)
		{
		b:if(i==len)break;
			if (isspace(temp_buf[i]))
			{
				if(!flag) {
					buf[j]=' ';
					flag=1;
				} else continue;
			}
			else {
				if(temp_buf[i] == '<') {
					if(i>len-4) throw invl_doc_end;
					if (temp_buf[i+1] == '!')
					{
						i+=2;
						if(temp_buf[i] == '-'&&temp_buf[i+1] == '-')
						{
							i+=2;
							while (i<len-3)
							{
								if (temp_buf[i] == '-'&&temp_buf[i + 1] == '-'&&temp_buf[i + 2] == '>')
								{
									i+=3;
									goto b;
								}
								i++;
							}
							throw "Unclosed comment";
						}
						else {while(temp_buf[i]!='>') if(++i==len) throw invl_doc_end; continue;}
					}
				}	
				buf[j]=temp_buf[i];
				flag=0;
			}
			j++;
		}
		HeapFree(heap, 0, temp_buf);
		if(!j) throw "Empty file";
		buf[j]='\0';
		/*j must be <= len so it will not move the buffer.*/
		HeapReAlloc(heap, 0, buf, j+1);
	}
	parse_data();
}
xml_data::~xml_data()
{
	HeapFree(heap, 0, buf);
}

extern "C" {
_declspec(dllexport) xml_data* parseXml(const char *path, processElem_t pel, HANDLE hHeap)
{
	FILE *f = fopen(path, "rb");
	if(!f)return nullptr;
	pe = pel;
	heap = hHeap;
	auto *d = new ((xml_data*)HeapAlloc(heap, 0, sizeof(xml_data)))xml_data(f);
	fclose(f);
	return d;
}
}
