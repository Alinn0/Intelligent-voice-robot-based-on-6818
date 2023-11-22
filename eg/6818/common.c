#include "common.h"

/********************* 通用函数列表 *******************/

ssize_t Write(int fildes, const void *buf, size_t nbyte)
{
	ssize_t n;
	size_t total = 0;

	char *tmp = (char *)buf;
	while(nbyte > 0)
	{
		while((n=write(fildes, tmp, nbyte)) == -1 &&
				errno == EINTR);

		if(n == -1)
		{
			perror("write() error");
			exit(0);
		}

		nbyte -= n;
		tmp += n;
		total += n;
	}

	return total;
}

ssize_t Read(int fildes, void *buf, size_t nbyte)
{
	ssize_t n;
	while((n=read(fildes, buf, nbyte)) == -1
			&& errno == EINTR);

	if(n == -1)
	{
		perror("read() failed");
		exit(0);
	}

	return n;
}





xmlChar *wait4id(int sockfd)
{
	char *xml = calloc(1, XMLSIZE);

	// 从ubuntu接收XML结果
	int n = Read(sockfd, xml, XMLSIZE);
	printf("%d bytes has been recv from ubuntu.\n", n);

	// 将XML写入本地文件 XMLFILE 中
	FILE *fp = fopen(XMLFILE, "w");
	if(fp == NULL)
	{
		perror("fopen() failed");
		exit(0);
	}

	size_t m = fwrite(xml, 1, n, fp);
	if(m != n)
	{
		perror("fwrite() failed");
		exit(0);
	}

	fflush(fp);

	return parse_xml(XMLFILE);
}


/********************* XML函数列表 *******************/

xmlChar *__get_cmd_id(xmlDocPtr doc, xmlNodePtr cur)
{
	xmlChar *key, *id;
	cur = cur->xmlChildrenNode;
	while (cur != NULL)
	{
	    if ((!xmlStrcmp(cur->name, (const xmlChar *)"cmd")))
	    {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    printf("cmd: %s\n", key);
		    xmlFree(key);

		    id = xmlGetProp(cur, (const xmlChar *)"id");
		    printf("id: %s\n", id);

		    xmlFree(doc);
		    return id;
 	    }
		cur = cur->next;
	}

	xmlFree(doc);
    return NULL;
}

xmlChar *parse_xml(char *xmlfile)
{
	xmlDocPtr doc;
	xmlNodePtr cur1, cur2;

	doc = xmlParseFile(xmlfile);
	if (doc == NULL)
	{
		fprintf(stderr,"Document not parsed successfully. \n");
		return NULL;
	}
	
	cur1 = xmlDocGetRootElement(doc);
	if(cur1 == NULL)
	{
		fprintf(stderr,"empty document\n");
		xmlFreeDoc(doc);
		return NULL;
	}

	if(xmlStrcmp(cur1->name, (const xmlChar *)"nlp"))
	{
		fprintf(stderr,"document of the wrong type, root node != nlp");
		xmlFreeDoc(doc);
		return NULL;
	}
	
	cur1 = cur1->xmlChildrenNode;

	while (cur1 != NULL)
	{
		if ((!xmlStrcmp(cur1->name, (const xmlChar *)"result")))
		{
			cur2 = cur1->xmlChildrenNode;
			while(cur2 != NULL)
			{
				if((!xmlStrcmp(cur2->name, (const xmlChar *)"confidence")))
				{
					xmlChar *key = xmlNodeListGetString(doc, cur2->xmlChildrenNode, 1);
					if(atoi((char *)key) < 30)
					{
						xmlFree(doc);
						fprintf(stderr, "sorry, I'm NOT sure what you say.\n");
						return NULL;
					}
				}
				
				if((!xmlStrcmp(cur2->name, (const xmlChar *)"object")))
				{
					return __get_cmd_id(doc, cur2);
				}
				cur2 = cur2->next;
			}
		}
		cur1 = cur1->next;
	}

	xmlFreeDoc(doc);
	return NULL;
}


/********************* 显示函数列表 *******************/






