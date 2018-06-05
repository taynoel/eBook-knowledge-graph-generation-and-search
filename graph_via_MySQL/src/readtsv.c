#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <string>

int main()
{
	int MODEE=4;
	if(MODEE==0)
	{
	
		char str[999];
		FILE * file;
		file = fopen( "/home/noel/dataset/yagoFacts.tsv" , "r");
		if (file) 
		{
			int Count=0;
			while (fscanf(file, "%s", str)!=EOF)
			{
				//if(Count%40000000==0)
				
				Count++;
				//if(Count==100)break;
			
				if(Count>50)
				{
					int triple=(Count-52)%4;
					if(triple==1)
						printf("%d %s ",Count,str);
					else if(triple==2)
						printf("\t\t%s ",str);
					else if(triple==3)
						printf("\t\t%s \n",str);
				}
			
			
			}
		
			fclose(file);
			printf("%d\n",Count);
		}
	}
	else if(MODEE==1)
	{
		char str[999];
		FILE * file;
		file = fopen( "/home/noel/dataset/yagoTaxonomy.tsv" , "r");
		char trp1[100];
		char trp2[100];
		char trp3[100];
		if (file) 
		{
			int Count=0;
			while (fscanf(file, "%s", str)!=EOF)
			{
				//if(Count%40000000==0)
				
				Count++;
				//if(Count==200)break;
				
				char* strr=NULL;
				int flagg=1;
				//strr=strstr(str,"wikicat_Operating_systems_by_architecture");
				//if(strr!=NULL)flagg=1;
				
				if(Count>59)
				{
					int triple=(Count-60)%4;
					if(triple==1)
						strcpy(trp1,str);
						//printf("%d %s ",Count,str);
					else if(triple==2)
						strcpy(trp2,str);
						//printf("\t\t%s ",str);
					else if(triple==3)
					{
						strcpy(trp3,str);
						if(flagg==1)
						printf("%d: %s %s %s \n",Count,trp1,trp2,trp3);
					}
				}
			
			
			}
		
			fclose(file);
			printf("%d\n",Count);
		}
	}
	else if(MODEE==2)//send to mysql yagoTaxonomy.tsv
	{
		std::string STRG;
		
		MYSQL *con=mysql_init(NULL);
	    if(con==NULL)exit(-1);
	    if(mysql_real_connect(con,"localhost","monty","some_pass","robot",0,NULL,0)==NULL)exit(-2);
		
		char str[999];
		FILE * file;
		file = fopen( "/home/noel/dataset/yagoTaxonomy.tsv" , "r");
		char trp1[100];
		char trp2[100];
		char trp3[100];
		
		FILE *file2=fopen("/home/noel/CProg/EBook_KWS/KBgen/failedQuery.txt","w+");
		
		if (file) 
		{
			STRG="INSERT INTO yagoTaxonomy (subject,property,object) VALUES";
			int Count=0;
			int REstart=1;
			int queryCount=0;
			int TRIPLECOUNT=0;
			while (fscanf(file, "%s", str)!=EOF)
			{
				if(REstart==1)
				{
					STRG="INSERT INTO yagoTaxonomy (subject,property,object) VALUES";
					REstart=0;
					queryCount=0;
				}
				
				Count++;
				//if(Count==10000)break;
				if(Count>59)
				{
					int triple=(Count-60)%4;
					if(triple==1) strcpy(trp1,str);
					else if(triple==2) strcpy(trp2,str);
					else if(triple==3)
					{
						char buffer[200];
						strcpy(trp3,str);
						sprintf(buffer,"(\"%s\",\"%s\",\"%s\"),",trp1,trp2,trp3);
						STRG.append(buffer);
						queryCount++;
						TRIPLECOUNT++;
						
						if(queryCount>400)
						{
							REstart=1;
							STRG.erase(STRG.size()-1);
							int res=mysql_query(con,STRG.c_str());
							printf("TRIPLECOUNT:%d   Count:%d\n",TRIPLECOUNT,Count);
							
							if(res==1){printf("FAIL!\n");fputs(STRG.c_str(),file2);}
							//printf("%d: %s \n",Count,STRG.c_str());
						}
						//printf("%d: %s %s %s \n",Count,trp1,trp2,trp3);
					}
				}
			}
			
			STRG.erase(STRG.size()-1);
			mysql_query(con,STRG.c_str());
			printf("%s \n",STRG.c_str());
			
			fclose(file);
			printf("%d\n",Count);
		}
		mysql_close(con);
	}
	else if(MODEE==3)//send to mysql yagoFacts.tsv
	{
		std::string STRG;
		
		MYSQL *con=mysql_init(NULL);
	    if(con==NULL)exit(-1);
	    if(mysql_real_connect(con,"localhost","monty","some_pass","robot",0,NULL,0)==NULL)exit(-2);
		
		char str[999];
		FILE * file;
		file = fopen( "/home/noel/dataset/yagoFacts.tsv" , "r");
		char trp1[100];
		char trp2[100];
		char trp3[100];
		
		FILE *file2=fopen("/home/noel/CProg/EBook_KWS/KBgen/failedQuery.txt","w+");
		
		if (file) 
		{
			STRG="INSERT INTO  yagoFacts(subject,property,object) VALUES";
			int Count=0;
			int REstart=1;
			int queryCount=0;
			int TRIPLECOUNT=0;
			while (fscanf(file, "%s", str)!=EOF)
			{
				if(REstart==1)
				{
					STRG="INSERT INTO  yagoFacts(subject,property,object) VALUES";
					REstart=0;
					queryCount=0;
				}
				
				Count++;
				//if(Count%100000==0)printf("AAA %d\n",Count);
				if(Count>52)
				{
					int triple=(Count-60)%4;
					if(triple==1) strcpy(trp1,str);
					else if(triple==2) strcpy(trp2,str);
					else if(triple==3)
					{
						char buffer[200];
						strcpy(trp3,str);
						sprintf(buffer,"(\"%s\",\"%s\",\"%s\"),",trp1,trp2,trp3);
						STRG.append(buffer);
						queryCount++;
						TRIPLECOUNT++;
						//printf("%s\n",buffer);
						
						if(queryCount>100000)
						{
							REstart=1;
							STRG.erase(STRG.size()-1);
							int res=mysql_query(con,STRG.c_str());
							printf("TRIPLECOUNT:%d   Count:%d\n",TRIPLECOUNT,Count);
							
							if(res==1){printf("FAIL!\n");fputs(STRG.c_str(),file2);}
							//printf("%d: %s \n",Count,STRG.c_str());
						}
						//printf("%d: %s %s %s \n",Count,trp1,trp2,trp3);
					}
				}
			}
			
			STRG.erase(STRG.size()-1);
			mysql_query(con,STRG.c_str());
			printf("%s \n",STRG.c_str());
			
			fclose(file);
			printf("%d\n",Count);
		}
		mysql_close(con);
		fclose(file2);
	}
	else if(MODEE==4)//send to mysql yagoTypes.tsv
	{
		std::string STRG;
		
		MYSQL *con=mysql_init(NULL);
	    if(con==NULL)exit(-1);
	    if(mysql_real_connect(con,"localhost","monty","some_pass","robot",0,NULL,0)==NULL)exit(-2);
		
		char str[999];
		FILE * file;
		file = fopen( "/home/noel/dataset/yagoTypes.tsv" , "r");
		char trp1[100];
		char trp2[100];
		char trp3[100];
		
		FILE *file2=fopen("/home/noel/CProg/EBook_KWS/KBgen/failedQuery.txt","w+");
		
		if (file) 
		{
			STRG="INSERT INTO  yagoTypes(subject,property,object) VALUES";
			int Count=0;
			int REstart=1;
			int queryCount=0;
			int TRIPLECOUNT=0;
			while (fscanf(file, "%s", str)!=EOF)
			{
				if(REstart==1)
				{
					STRG="INSERT INTO  yagoTypes(subject,property,object) VALUES";
					REstart=0;
					queryCount=0;
				}
				
				Count++;
				//if(Count%1000000==0)printf("AAA %d\n",Count);
			
				if(Count>50)
				{
					int triple=(Count-51)%4;
					if(triple==1) strcpy(trp1,str);
					else if(triple==2) strcpy(trp2,str);
					else if(triple==3)
					{
						char buffer[200];
						strcpy(trp3,str);
						sprintf(buffer,"(\"%s\",\"%s\",\"%s\"),",trp1,trp2,trp3);
						STRG.append(buffer);
						queryCount++;
						TRIPLECOUNT++;
						//printf("%s\n",buffer);
						
						if(queryCount>10000)
						{
							REstart=1;
							STRG.erase(STRG.size()-1);
							int res=mysql_query(con,STRG.c_str());
							printf("TRIPLECOUNT:%d   Count:%d\n",TRIPLECOUNT,Count);
							
							if(res==1){printf("FAIL!\n");fputs(STRG.c_str(),file2);}
							//printf("%d: %s \n",Count,STRG.c_str());
						}
						//printf("%d: %s %s %s \n",Count,trp1,trp2,trp3);
					}
				}
			}
			
			STRG.erase(STRG.size()-1);
			mysql_query(con,STRG.c_str());
			printf("%s \n",STRG.c_str());
			
			fclose(file);
			printf("%d\n",Count);
		}
		mysql_close(con);
		fclose(file2);
	}
	

}
